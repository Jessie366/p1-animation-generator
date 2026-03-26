#include "animate_internal.h"

#include <stdlib.h>
#include <string.h>

/* Public: allocate and initialise an empty canvas. */
struct canvas *animate_create_canvas(size_t height, size_t width,
                                     color_t background_color) {
    struct canvas *c = malloc(sizeof(*c));
    if (!c) return NULL;

    c->height           = height;
    c->width            = width;
    c->background_color = background_color;
    c->bottom           = NULL;
    c->top              = NULL;

    return c;
}

/*
 * Public: destroy canvas and all its placements (but NOT the sprites).
 * Decrements the refcount of each sprite whose placement is removed.
 */
void animate_destroy_canvas(struct canvas *canvas) {
    if (!canvas) return;

    struct sprite_placement *p = canvas->bottom;
    while (p) {
        struct sprite_placement *next = p->next;
        p->sprite->refcount--;
        free(p);
        p = next;
    }

    free(canvas);
}

/* Public: return the byte size of one frame buffer for this canvas. */
size_t animate_frame_size_bytes(struct canvas *canvas) {
    return canvas->height * canvas->width * sizeof(color_t);
}
