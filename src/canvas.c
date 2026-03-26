#include <stdlib.h>

#include "animate_internal.h"

/* canvas owns placements, not sprites.
   placement holds one counted reference to sprite. */

struct canvas *animate_create_canvas(size_t height, size_t width,
                                     color_t background_color) {
    struct canvas *canvas = malloc(sizeof(*canvas));
    if (canvas == NULL) {
        return NULL;
    }

    canvas->height = height;
    canvas->width = width;
    canvas->background_color = background_color;
    canvas->head = NULL;
    canvas->tail = NULL;

    return canvas;
}

void animate_destroy_canvas(struct canvas *canvas) {
    struct sprite_placement *curr;
    struct sprite_placement *next;

    if (canvas == NULL) {
        return;
    }

    curr = canvas->head;
    while (curr != NULL) {
        next = curr->next;
        animate_destroy_placement(curr);
        curr = next;
    }

    free(canvas);
}
