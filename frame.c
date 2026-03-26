#include "animate_internal.h"

#include <string.h>

/* Declared in sprite.c; used here for lazy BMP loading. */
bool sprite_load_bmp(struct sprite *s);

/*
 * Public: render one animation frame into buf.
 *
 * Algorithm:
 *  1. Fill entire buffer with background colour.
 *  2. Compute t = frame / frame_rate (seconds).
 *  3. Walk placements from bottom to top (bottom-most drawn first).
 *  4. For each placement compute position from physics or custom fn.
 *  5. Blit non-transparent sprite pixels onto the frame, clipping to bounds.
 *     All written pixels get alpha forced to 0xFF.
 */
void animate_generate_frame(const struct canvas *canvas,
                            size_t frame, size_t frame_rate,
                            void *buf) {
    color_t *fb     = (color_t *)buf;
    size_t   total  = canvas->width * canvas->height;

    /* Step 1: fill background. */
    for (size_t i = 0; i < total; i++) {
        fb[i] = canvas->background_color;
    }

    /* Step 2: time in seconds. */
    float t = (frame_rate > 0) ? ((float)frame / (float)frame_rate) : 0.0f;

    /* Step 3: traverse list bottom → top. */
    struct sprite_placement *p = canvas->bottom;
    while (p) {
        struct sprite *s = p->sprite;

        /* Lazy-load BMP pixel data on first access. */
        if (!s->loaded) {
            sprite_load_bmp(s);
        }

        /* Skip sprites with no pixel data (e.g. failed BMP load). */
        if (!s->pixels) {
            p = p->next;
            continue;
        }

        /* Step 4: compute canvas position. */
        ssize_t px, py;
        if (p->anim_fn) {
            px = p->x0;
            py = p->y0;
            p->anim_fn(p->anim_priv, &px, &py, t);
        } else {
            px = p->x0 + (ssize_t)((float)p->vx * t
                                   + (float)p->ax * t * t / 2.0f);
            py = p->y0 + (ssize_t)((float)p->vy * t
                                   + (float)p->ay * t * t / 2.0f);
        }

        /* Step 5: blit sprite pixels. */
        for (size_t row = 0; row < s->height; row++) {
            ssize_t dest_row = py + (ssize_t)row;
            if (dest_row < 0 || dest_row >= (ssize_t)canvas->height) continue;

            for (size_t col = 0; col < s->width; col++) {
                color_t pixel = s->pixels[row * s->width + col];

                /* Zero alpha means fully transparent — skip. */
                if ((pixel >> 24) == 0) continue;

                ssize_t dest_col = px + (ssize_t)col;
                if (dest_col < 0 || dest_col >= (ssize_t)canvas->width) continue;

                /* Write pixel with alpha forced to 0xFF. */
                fb[(size_t)dest_row * canvas->width + (size_t)dest_col] =
                    (pixel & 0x00FFFFFFu) | 0xFF000000u;
            }
        }

        p = p->next;
    }
}
