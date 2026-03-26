#include <stdlib.h>

#include "animate_internal.h"

struct canvas *animate_create_canvas(uint16_t width, uint16_t height) {
    struct canvas *canvas = malloc(sizeof(*canvas));
    if (canvas == NULL) {
        return NULL;
    }

    canvas->width = width;
    canvas->height = height;
    canvas->front = NULL;
    canvas->back = NULL;
    canvas->placement_count = 0;

    return canvas;
}

void animate_destroy_canvas(struct canvas *canvas) {
    struct sprite_placement *curr;
    struct sprite_placement *next;

    if (canvas == NULL) {
        return;
    }

    curr = canvas->front;
    while (curr != NULL) {
        next = curr->next;

        if (curr->sprite != NULL && curr->sprite->ref_count > 0) {
            curr->sprite->ref_count--;
        }

        free(curr);
        curr = next;
    }

    free(canvas);
}
