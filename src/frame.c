#include <stddef.h>
#include <stdint.h>
#include <sys/types.h>

#include "animate_internal.h"

static unsigned alpha_of(color_t c) {
    return (unsigned)((c >> 24) & 0xFFu);
}

static color_t force_opaque(color_t c) {
    return (c & 0x00FFFFFFu) | 0xFF000000u;
}

static size_t pixel_index(size_t width, size_t x, size_t y) {
    return y * width + x;
}

size_t animate_frame_size_bytes(struct canvas *canvas) {
    if (canvas == NULL) {
        return 0;
    }

    return canvas->height * canvas->width * sizeof(color_t);
}

void animate_generate_frame(struct canvas *canvas,
                            size_t frame, size_t frame_rate,
                            void *buf) {
    color_t *out;
    color_t bg;
    size_t x;
    size_t y;
    float t;
    struct sprite_placement *placement;

    if (canvas == NULL || buf == NULL) {
        return;
    }

    out = buf;
    bg = force_opaque(canvas->background_color);

    for (y = 0; y < canvas->height; y++) {
        for (x = 0; x < canvas->width; x++) {
            out[pixel_index(canvas->width, x, y)] = bg;
        }
    }

    if (frame_rate == 0) {
        t = 0.0f;
    } else {
        t = (float)frame / (float)frame_rate;
    }

    placement = canvas->head;
    while (placement != NULL) {
        const struct sprite *sprite = placement->sprite;
        ssize_t base_x;
        ssize_t base_y;
        size_t sx;
        size_t sy;

        if (sprite != NULL) {
            base_x = (ssize_t)(
                (float)placement->x +
                (float)placement->vx * t +
                ((float)placement->ax * t * t) / 2.0f
            );
            base_y = (ssize_t)(
                (float)placement->y +
                (float)placement->vy * t +
                ((float)placement->ay * t * t) / 2.0f
            );

            for (sy = 0; sy < sprite->height; sy++) {
                for (sx = 0; sx < sprite->width; sx++) {
                    ssize_t dst_x = base_x + (ssize_t)sx;
                    ssize_t dst_y = base_y + (ssize_t)sy;
                    color_t src;

                    if (dst_x < 0 || dst_y < 0) {
                        continue;
                    }
                    if ((size_t)dst_x >= canvas->width ||
                        (size_t)dst_y >= canvas->height) {
                        continue;
                    }

                    src = sprite->pixels[pixel_index(sprite->width, sx, sy)];
                    if (alpha_of(src) == 0) {
                        continue;
                    }

                    out[pixel_index(canvas->width, (size_t)dst_x, (size_t)dst_y)] =
                        force_opaque(src);
                }
            }
        }

        placement = placement->next;
    }
}
