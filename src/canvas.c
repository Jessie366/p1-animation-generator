/**
 * @file canvas.c
 * @brief Canvas implementation
 */

#include <stdlib.h>
#include <string.h>

#include "animate_internal.h"

/**
 * Create a new canvas with the specified dimensions and background color.
 */
struct canvas* animate_create_canvas(size_t height, size_t width, color_t background_color) {
    struct canvas* canvas = malloc(sizeof(struct canvas));
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

/**
 * Destroy a canvas and all its placements.
 * Note: This does NOT destroy the sprites themselves, as they may be shared.
 */
void animate_destroy_canvas(struct canvas* canvas) {
    if (canvas == NULL) {
        return;
    }

    /* Destroy all placements */
    struct spring_placement* current = canvas->head;
    while (current != NULL) {
        struct spring_placement* next = current->next;
        free(current);
        current = next;
    }

    free(canvas);
}

/**
 * Return the size in bytes of a frame for this canvas.
 */
size_t animate_frame_size_bytes(struct canvas* canvas) {
    if (canvas == NULL) {
        return 0;
    }
    return canvas->height * canvas->width * sizeof(color_t);
}
