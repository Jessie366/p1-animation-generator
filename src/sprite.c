#include <stdbool.h>
#include <stddef.h>
#include <stdlib.h>

#include "animate_internal.h"

struct sprite *animate_create_rectangle(size_t width, size_t height, color_t c,
                                        bool filled) {
    struct sprite *sprite;
    size_t x;
    size_t y;

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
        return false;
    }

    if (sprite->ref_count != 0) {
        return false;
    }

    free(sprite->pixels);
    free(sprite);
    return true;
}
