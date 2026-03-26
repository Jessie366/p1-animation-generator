#include <stdlib.h>

#include "animate_internal.h"

static void detach_placement(struct sprite_placement *placement) {
    struct canvas *canvas;

    if (placement == NULL || placement->owner == NULL) {
        return;
    }

    canvas = placement->owner;

    if (placement->prev != NULL) {
        placement->prev->next = placement->next;
    } else {
        canvas->front = placement->next;
    }

    if (placement->next != NULL) {
        placement->next->prev = placement->prev;
    } else {
        canvas->back = placement->prev;
    }

    placement->prev = NULL;
    placement->next = NULL;
    placement->owner = NULL;
}

static void insert_placement_at_top(struct canvas *canvas,
                                    struct sprite_placement *placement) {
    if (canvas == NULL || placement == NULL) {
        return;
    }

    placement->owner = canvas;
    placement->prev = canvas->back;
    placement->next = NULL;

    if (canvas->back != NULL) {
        canvas->back->next = placement;
    } else {
        canvas->front = placement;
    }

    canvas->back = placement;
}

struct sprite_placement *animate_place_sprite(struct canvas *canvas,
                                              struct sprite *sprite,
                                              ssize_t x, ssize_t y) {
    struct sprite_placement *placement;

    if (canvas == NULL || sprite == NULL) {
        return NULL;
    }

    placement = malloc(sizeof(*placement));
    if (placement == NULL) {
        return NULL;
    }

    placement->sprite = sprite;
    placement->owner = NULL;

    placement->x = x;
    placement->y = y;

    placement->vx = 0;
    placement->vy = 0;
    placement->ax = 0;
    placement->ay = 0;

    placement->prev = NULL;
    placement->next = NULL;

    insert_placement_at_top(canvas, placement);
    sprite->ref_count++;

    return placement;
}

void animate_destroy_placement(struct sprite_placement *sprite_placement) {
    if (sprite_placement == NULL) {
        return;
    }

    detach_placement(sprite_placement);

    if (sprite_placement->sprite != NULL &&
        sprite_placement->sprite->ref_count > 0) {
        sprite_placement->sprite->ref_count--;
    }

    free(sprite_placement);
}

void animate_set_animation_params(struct sprite_placement *sprite_placement,
                                  ssize_t vx, ssize_t vy,
                                  ssize_t ax, ssize_t ay) {
    if (sprite_placement == NULL) {
        return;
    }

    sprite_placement->vx = vx;
    sprite_placement->vy = vy;
    sprite_placement->ax = ax;
    sprite_placement->ay = ay;
}
