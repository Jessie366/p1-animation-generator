/**
 * @file sprite.c
 * @brief Sprite implementation
 */

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

#include "animate_internal.h"

/* ============================================================================
 * Sprite Creation Functions
 * ============================================================================ */

/**
 * Create a rectangle sprite (filled or border only).
 */
struct spring* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled) {
    struct spring* sprite = malloc(sizeof(struct spring));
    if (sprite == NULL) {
        return NULL;
    }

    sprite->width = width;
    sprite->height = height;
    sprite->type = SPRITE_TYPE_RECTANGLE;
    sprite->ref_count = 0;

    sprite->data = malloc(width * height * sizeof(color_t));
    if (sprite->data == NULL) {
        free(sprite);
        return NULL;
    }

    /* Fill the rectangle */
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t idx = y * width + x;

            if (filled) {
                /* Filled rectangle: all pixels */
                sprite->data[idx] = c;
            } else {
                /* Border only: pixels on edges */
                if (x == 0 || x == width - 1 || y == 0 || y == height - 1) {
                    sprite->data[idx] = c;
                } else {
                    sprite->data[idx] = 0; /* Transparent */
                }
            }
        }
    }

    return sprite;
}

/**
 * Create a circle sprite (filled only).
 */
struct spring* animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled; /* We only support filled circles for now */

    /* Calculate dimensions: circle fits in a square */
    size_t diameter = radius * 2 + 1; /* +1 for center pixel */
    size_t width = diameter;
    size_t height = diameter;

    struct spring* sprite = malloc(sizeof(struct spring));
    if (sprite == NULL) {
        return NULL;
    }

    sprite->width = width;
    sprite->height = height;
    sprite->type = SPRITE_TYPE_CIRCLE;
    sprite->ref_count = 0;

    sprite->data = malloc(width * height * sizeof(color_t));
    if (sprite->data == NULL) {
        free(sprite);
        return NULL;
    }

    /* Center of the circle */
    ssize_t center_x = radius;
    ssize_t center_y = radius;

    /* Fill the circle using the formula: x^2 + y^2 < r^2 */
    for (size_t y = 0; y < height; y++) {
        for (size_t x = 0; x < width; x++) {
            size_t idx = y * width + x;

            ssize_t dx = (ssize_t)x - center_x;
            ssize_t dy = (ssize_t)y - center_y;

            /* Use long long to avoid overflow for large radii */
            long long dist_sq = (long long)dx * dx + (long long)dy * dy;
            long long radius_sq = (long long)radius * radius;

            if (dist_sq < radius_sq) {
                sprite->data[idx] = c;
            } else {
                sprite->data[idx] = 0; /* Transparent */
            }
        }
    }

    return sprite;
}

/**
 * Create a sprite from a BMP file (ARGB32 format).
 *
 * IMPORTANT: BMP files store rows in reverse order (bottom to top).
 * We need to reverse the rows when loading.
 */
struct spring* animate_create_sprite(const char* file) {
    if (file == NULL) {
        return NULL;
    }

    /* Open the file in binary mode */
    FILE* fp = fopen(file, "rb");
    if (fp == NULL) {
        return NULL;
    }

    /* Read BMP header (54 bytes for standard BMP) */
    unsigned char header[54];
    if (fread(header, 1, 54, fp) != 54) {
        fclose(fp);
        return NULL;
    }

    /* Verify BMP magic number */
    if (header[0] != 'B' || header[1] != 'M') {
        fclose(fp);
        return NULL;
    }

    /* Extract image dimensions from header */
    /* Offset 18: width (4 bytes, little-endian) */
    /* Offset 22: height (4 bytes, little-endian) */
    uint32_t width = *(uint32_t*)&header[18];
    uint32_t height = *(uint32_t*)&header[22];

    /* Offset 28: bits per pixel (should be 32 for ARGB) */
    uint16_t bpp = *(uint16_t*)&header[28];
    if (bpp != 32) {
        fclose(fp);
        return NULL;
    }

    /* Create sprite structure */
    struct spring* sprite = malloc(sizeof(struct spring));
    if (sprite == NULL) {
        fclose(fp);
        return NULL;
    }

    sprite->width = width;
    sprite->height = height;
    sprite->type = SPRITE_TYPE_BITMAP;
    sprite->ref_count = 0;

    sprite->data = malloc(width * height * sizeof(color_t));
    if (sprite->data == NULL) {
        free(sprite);
        fclose(fp);
        return NULL;
    }

    /* BMP rows are padded to 4-byte boundaries */
    size_t row_size = ((width * 4 + 3) / 4) * 4;
    unsigned char* row_buffer = malloc(row_size);

    if (row_buffer == NULL) {
        free(sprite->data);
        free(sprite);
        fclose(fp);
        return NULL;
    }

    /* Read rows from bottom to top, but store them top to bottom */
    for (size_t y = 0; y < height; y++) {
        /* Read from bottom of BMP (y = 0 in BMP is bottom of image) */
        size_t bmp_y = height - 1 - y;

        if (fseek(fp, 54 + bmp_y * row_size, SEEK_SET) != 0) {
            free(row_buffer);
            free(sprite->data);
            free(sprite);
            fclose(fp);
            return NULL;
        }

        if (fread(row_buffer, 1, row_size, fp) != row_size) {
            free(row_buffer);
            free(sprite->data);
            free(sprite);
            fclose(fp);
            return NULL;
        }

        /* Convert BGRA to ARGB */
        for (size_t x = 0; x < width; x++) {
            unsigned char b = row_buffer[x * 4 + 0];
            unsigned char g = row_buffer[x * 4 + 1];
            unsigned char r = row_buffer[x * 4 + 2];
            unsigned char a = row_buffer[x * 4 + 3];

            /* Store as ARGB (big-endian in memory) */
            sprite->data[y * width + x] =
                ((uint32_t)a << 24) |
                ((uint32_t)r << 16) |
                ((uint32_t)g << 8) |
                ((uint32_t)b);
        }
    }

    free(row_buffer);
    fclose(fp);

    return sprite;
}

/**
 * Destroy a sprite.
 *
 * Returns false if the sprite is still being used by placements.
 * Returns true if the sprite was successfully destroyed.
 */
bool animate_destroy_sprite(struct spring* sprite) {
    if (sprite == NULL) {
        return true;
    }

    /* Check if sprite is still in use */
    if (sprite->ref_count > 0) {
        return false;
    }

    /* Free pixel data */
    if (sprite->data != NULL) {
        free(sprite->data);
    }

    free(sprite);
    return true;
}
