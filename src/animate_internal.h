/**
 * @file animate_internal.h
 * @brief Internal structures for PointerPro Animate
 *
 * This header defines the internal data structures used in the implementation.
 * These are not part of the public API.
 */

#ifndef ANIMATE_INTERNAL_H
#define ANIMATE_INTERNAL_H

#include "animate.h"

struct sprite {
    size_t width;
    size_t height;
    color_t *pixels;
    size_t ref_count;
};

struct canvas;

struct sprite_placement {
    struct sprite *sprite;
    struct canvas *owner;

    ssize_t x;
    ssize_t y;

    ssize_t vx;
    ssize_t vy;
    ssize_t ax;
    ssize_t ay;

    struct sprite_placement *prev;
    struct sprite_placement *next;
};

struct canvas {
    size_t height;
    size_t width;
    color_t background_color;

    struct sprite_placement *head; /* bottom-most */
    struct sprite_placement *tail; /* top-most */
};

#endif /* ANIMATE_INTERNAL_H */
