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
        canvas->head = placement->next;
    }

    if (placement->next != NULL) {
        placement->next->prev = placement->prev;
    } else {
        canvas->tail = placement->prev;
    }

    placement->prev = NULL;
    placement->next = NULL;
    placement->owner = NULL;
}

static void insert_at_top(struct canvas *canvas,
                          struct sprite_placement *placement) {
    if (canvas == NULL || placement == NULL) {
        return;
    }

    placement->owner = canvas;
    placement->prev = canvas->tail;
    placement->next = NULL;

    if (canvas->tail != NULL) {
        canvas->tail->next = placement;
    } else {
        canvas->head = placement;
    }

    canvas->tail = placement;
}

static void insert_at_bottom(struct canvas *canvas,
                             struct sprite_placement *placement) {
    if (canvas == NULL || placement == NULL) {
        return;
    }

    placement->owner = canvas;
    placement->prev = NULL;
    placement->next = canvas->head;

    if (canvas->head != NULL) {
        canvas->head->prev = placement;
    } else {
        canvas->tail = placement;
    }

    canvas->head = placement;
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

    insert_at_top(canvas, placement);
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

void animate_placement_top(struct sprite_placement *sprite_placement) {
    struct canvas *canvas;

    if (sprite_placement == NULL || sprite_placement->owner == NULL) {
        return;
    }

    canvas = sprite_placement->owner;
    if (canvas->tail == sprite_placement) {
        return;
    }

    detach_placement(sprite_placement);
    insert_at_top(canvas, sprite_placement);
}

void animate_placement_bottom(struct sprite_placement *sprite_placement) {
    struct canvas *canvas;

    if (sprite_placement == NULL || sprite_placement->owner == NULL) {
        return;
    }

    canvas = sprite_placement->owner;
    if (canvas->head == sprite_placement) {
        return;
    }

    detach_placement(sprite_placement);
    insert_at_bottom(canvas, sprite_placement);
}

void animate_placement_up(struct sprite_placement *sprite_placement) {
    struct canvas *canvas;
    struct sprite_placement *left;
    struct sprite_placement *right;
    struct sprite_placement *right_right;

    if (sprite_placement == NULL || sprite_placement->owner == NULL) {
        return;
    }

    canvas = sprite_placement->owner;
    right = sprite_placement->next;

    if (right == NULL) {
        return;
    }

    left = sprite_placement->prev;
    right_right = right->next;

    if (left != NULL) {
        left->next = right;
    } else {
        canvas->head = right;
    }

    right->prev = left;
    right->next = sprite_placement;

    sprite_placement->prev = right;
    sprite_placement->next = right_right;

    if (right_right != NULL) {
        right_right->prev = sprite_placement;
    } else {
        canvas->tail = sprite_placement;
    }
}

void animate_placement_down(struct sprite_placement *sprite_placement) {
    struct canvas *canvas;
    struct sprite_placement *left_left;
    struct sprite_placement *left;
    struct sprite_placement *right;

    if (sprite_placement == NULL || sprite_placement->owner == NULL) {
        return;
    }

    canvas = sprite_placement->owner;
    left = sprite_placement->prev;

    if (left == NULL) {
        return;
    }

    left_left = left->prev;
    right = sprite_placement->next;

    if (left_left != NULL) {
        left_left->next = sprite_placement;
    } else {
        canvas->head = sprite_placement;
    }

    sprite_placement->prev = left_left;
    sprite_placement->next = left;

    left->prev = sprite_placement;
    left->next = right;

    if (right != NULL) {
        right->prev = left;
    } else {
        canvas->tail = left;
    }
}
