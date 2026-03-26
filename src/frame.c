/**
 * @file frame.c
 * @brief Frame generation implementation
 */

#include <stdlib.h>
#include <string.h>
#include <math.h>

#include "animate_internal.h"

/**
 * Calculate the current position of a sprite based on animation parameters.
 *
 * Formula: p = p0 + v*t + (a*t^2)/2
 *
 * @param placement The placement to calculate position for
 * @param frame The current frame number (starting from 0)
 * @param frame_rate The number of frames per second
 * @param out_x Output: calculated X position
 * @param out_y Output: calculated Y position
 */
static void calculate_position(struct spring_placement* placement,
                               size_t frame, size_t frame_rate,
                               ssize_t* out_x, ssize_t* out_y) {
    /* Calculate time in seconds */
    float t = (float)frame / (float)frame_rate;

    /* Use custom animation function if set */
    if (placement->anim_fn != NULL) {
        ssize_t x = placement->initial_x;
        ssize_t y = placement->initial_y;
        placement->anim_fn(placement->anim_priv, &x, &y, t);
        *out_x = x;
        *out_y = y;
        return;
    }

    /* Standard physics calculation */
    /* p = p0 + v*t + (a*t^2)/2 */
    float x = (float)placement->initial_x +
              (float)placement->vx * t +
              (float)placement->ax * t * t / 2.0f;

    float y = (float)placement->initial_y +
              (float)placement->vy * t +
              (float)placement->ay * t * t / 2.0f;

    *out_x = (ssize_t)roundf(x);
    *out_y = (ssize_t)roundf(y);
}

/**
 * Generate a single frame of the animation.
 *
 * Alpha handling:
 * - Alpha = 0: fully transparent, do not write pixel
 * - Alpha != 0: fully opaque, write pixel (force alpha to 0xFF)
 *
 * Rendering order:
 * - Fill with background color first
 * - Iterate placements from bottom to top (tail to head)
 * - Write non-transparent pixels
 */
void animate_generate_frame(struct canvas* canvas, size_t frame,
                             size_t frame_rate, void* buf) {
    if (canvas == NULL || buf == NULL) {
        return;
    }

    color_t* pixels = (color_t*)buf;
    size_t width = canvas->width;
    size_t height = canvas->height;

    /* Step 1: Fill with background color */
    for (size_t i = 0; i < width * height; i++) {
        pixels[i] = canvas->background_color;
    }

    /* Step 2: Render placements from bottom to top (tail to head) */
    struct spring_placement* current = canvas->tail;

    while (current != NULL) {
        struct spring* sprite = current->sprite;
        if (sprite == NULL || sprite->data == NULL) {
            current = current->prev;
            continue;
        }

        /* Calculate current position */
        ssize_t pos_x, pos_y;
        calculate_position(current, frame, frame_rate, &pos_x, &pos_y);

        /* Step 3: Copy sprite pixels to frame */
        for (size_t sy = 0; sy < sprite->height; sy++) {
            for (size_t sx = 0; sx < sprite->width; sx++) {
                /* Calculate frame coordinates */
                ssize_t fx = pos_x + (ssize_t)sx;
                ssize_t fy = pos_y + (ssize_t)sy;

                /* Boundary check */
                if (fx < 0 || fx >= (ssize_t)width ||
                    fy < 0 || fy >= (ssize_t)height) {
                    continue;
                }

                /* Get sprite pixel */
                color_t pixel = sprite->data[sy * sprite->width + sx];

                /* Check alpha channel (bits 24-31) */
                uint8_t alpha = (pixel >> 24) & 0xFF;

                /* Only write if not transparent */
                if (alpha != 0) {
                    /* Force alpha to 0xFF (fully opaque) */
                    pixels[(size_t)fy * width + (size_t)fx] = pixel | 0xFF000000;
                }
            }
        }

        current = current->prev;
    }
}
