#include <limits.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

#include "animate_internal.h"

static bool read_u16_le(FILE *fp, uint16_t *out) {
    unsigned char buf[2];

    if (fread(buf, 1, 2, fp) != 2) {
        return false;
    }

    *out = (uint16_t)buf[0]
         | ((uint16_t)buf[1] << 8);
    return true;
}

static bool read_u32_le(FILE *fp, uint32_t *out) {
    unsigned char buf[4];

    if (fread(buf, 1, 4, fp) != 4) {
        return false;
    }

    *out = (uint32_t)buf[0]
         | ((uint32_t)buf[1] << 8)
         | ((uint32_t)buf[2] << 16)
         | ((uint32_t)buf[3] << 24);
    return true;
}

static bool read_i32_le(FILE *fp, int32_t *out) {
    uint32_t bits;

    if (!read_u32_le(fp, &bits)) {
        return false;
    }

    *out = (int32_t)bits;
    return true;
}

struct sprite *animate_create_sprite(const char *file) {
    FILE *fp;
    struct sprite *sprite;
    uint16_t bf_type;
    uint32_t bf_size;
    uint16_t bf_reserved1;
    uint16_t bf_reserved2;
    uint32_t bf_off_bits;

    uint32_t bi_size;
    int32_t bi_width;
    int32_t bi_height;
    uint16_t bi_planes;
    uint16_t bi_bit_count;
    uint32_t bi_compression;
    uint32_t bi_size_image;
    int32_t bi_x_pels_per_meter;
    int32_t bi_y_pels_per_meter;
    uint32_t bi_clr_used;
    uint32_t bi_clr_important;

    size_t width;
    size_t height;
    bool bottom_up;
    size_t row;
    size_t col;

    (void)bf_size;
    (void)bf_reserved1;
    (void)bf_reserved2;
    (void)bi_size_image;
    (void)bi_x_pels_per_meter;
    (void)bi_y_pels_per_meter;
    (void)bi_clr_used;
    (void)bi_clr_important;

    fp = fopen(file, "rb");
    if (fp == NULL) {
        return NULL;
    }

    if (!read_u16_le(fp, &bf_type) ||
        !read_u32_le(fp, &bf_size) ||
        !read_u16_le(fp, &bf_reserved1) ||
        !read_u16_le(fp, &bf_reserved2) ||
        !read_u32_le(fp, &bf_off_bits) ||
        !read_u32_le(fp, &bi_size) ||
        !read_i32_le(fp, &bi_width) ||
        !read_i32_le(fp, &bi_height) ||
        !read_u16_le(fp, &bi_planes) ||
        !read_u16_le(fp, &bi_bit_count) ||
        !read_u32_le(fp, &bi_compression) ||
        !read_u32_le(fp, &bi_size_image) ||
        !read_i32_le(fp, &bi_x_pels_per_meter) ||
        !read_i32_le(fp, &bi_y_pels_per_meter) ||
        !read_u32_le(fp, &bi_clr_used) ||
        !read_u32_le(fp, &bi_clr_important)) {
        fclose(fp);
        return NULL;
    }

    if (bf_type != 0x4D42u) {
        fclose(fp);
        return NULL;
    }

    if (bi_size < 40 || bi_planes != 1 || bi_bit_count != 32) {
        fclose(fp);
        return NULL;
    }

    if (bi_width <= 0 || bi_height == 0) {
        fclose(fp);
        return NULL;
    }

    if (bi_compression != 0) {
        fclose(fp);
        return NULL;
    }

    width = (size_t)bi_width;
    if (bi_height > 0) {
        height = (size_t)bi_height;
        bottom_up = true;
    } else {
        height = (size_t)(-bi_height);
        bottom_up = false;
    }

    sprite = malloc(sizeof(*sprite));
    if (sprite == NULL) {
        fclose(fp);
        return NULL;
    }

    sprite->pixels = malloc(width * height * sizeof(*(sprite->pixels)));
    if (sprite->pixels == NULL) {
        free(sprite);
        fclose(fp);
        return NULL;
    }

    sprite->width = width;
    sprite->height = height;
    sprite->ref_count = 0;

    if (fseek(fp, (long)bf_off_bits, SEEK_SET) != 0) {
        free(sprite->pixels);
        free(sprite);
        fclose(fp);
        return NULL;
    }

    for (row = 0; row < height; row++) {
        size_t dest_row = bottom_up ? (height - 1 - row) : row;

        for (col = 0; col < width; col++) {
            uint32_t pixel;

            if (!read_u32_le(fp, &pixel)) {
                free(sprite->pixels);
                free(sprite);
                fclose(fp);
                return NULL;
            }

            sprite->pixels[dest_row * width + col] = (color_t)pixel;
        }
    }

    fclose(fp);
    return sprite;
}

struct sprite *animate_create_rectangle(size_t width, size_t height, color_t c,
                                        bool filled) {
    struct sprite *sprite;
    size_t x;
    size_t y;

    if (width == 0 || height == 0) {
        return NULL;
    }

    sprite = malloc(sizeof(*sprite));
    if (sprite == NULL) {
        return NULL;
    }

    sprite->pixels = malloc(width * height * sizeof(*(sprite->pixels)));
    if (sprite->pixels == NULL) {
        free(sprite);
        return NULL;
    }

    sprite->width = width;
    sprite->height = height;
    sprite->ref_count = 0;

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            size_t idx = y * width + x;
            bool is_border = (x == 0 || x + 1 == width || y == 0 || y + 1 == height);

            if (filled || is_border) {
                sprite->pixels[idx] = c;
            } else {
                sprite->pixels[idx] = 0;
            }
        }
    }

    return sprite;
}

struct sprite *animate_create_circle(size_t radius, color_t c, bool filled) {
    struct sprite *sprite;
    size_t diameter;
    size_t x;
    size_t y;
    size_t center;
    size_t rr;

    (void)filled;

    /* Guard: 2*radius+1 and radius*radius must not overflow size_t. */
    if (radius > SIZE_MAX / 2 - 1) {
        return NULL;
    }
    if (radius != 0 && radius > SIZE_MAX / radius) {
        return NULL;
    }

    diameter = 2 * radius + 1;
    center = radius;
    rr = radius * radius;

    sprite = malloc(sizeof(*sprite));
    if (sprite == NULL) {
        return NULL;
    }

    sprite->pixels = malloc(diameter * diameter * sizeof(*(sprite->pixels)));
    if (sprite->pixels == NULL) {
        free(sprite);
        return NULL;
    }

    sprite->width = diameter;
    sprite->height = diameter;
    sprite->ref_count = 0;

    for (y = 0; y < diameter; y++) {
        for (x = 0; x < diameter; x++) {
            size_t idx = y * diameter + x;
            ssize_t dx = (ssize_t)x - (ssize_t)center;
            ssize_t dy = (ssize_t)y - (ssize_t)center;
            ssize_t dist2 = dx * dx + dy * dy;

            if ((size_t)dist2 <= rr) {
                sprite->pixels[idx] = c;
            } else {
                sprite->pixels[idx] = 0;
            }
        }
    }

    return sprite;
}

bool animate_destroy_sprite(struct sprite *sprite) {
    if (sprite == NULL) {
        return true;   // error
    }

    if (sprite->ref_count != 0) {
        return true;   // error: still in use
    }

    free(sprite->pixels);
    free(sprite);
    return false;      // 0 = success
}
