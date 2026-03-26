/**
 * @file placement.c
 * @brief Sprite placement implementation
 */

#include <stdlib.h>
#include <string.h>

#include "animate_internal.h"

/* ============================================================================
 * Placement Creation and Destruction
 * ============================================================================ */

/**
 * Place a sprite on the canvas at the specified position.
 * The sprite is placed on the top layer.
 *
 * This function has O(1) time complexity.
 */
struct spring_placement* animate_place_sprite(struct canvas* canvas,
                                              struct spring* sprite,
                                              ssize_t x, ssize_t y) {
    if (canvas == NULL || sprite == NULL) {
        return NULL;
    }

    /* Create placement structure */
    struct spring_placement* placement = malloc(sizeof(struct spring_placement));
    if (placement == NULL) {
        return NULL;
    }

    /* Initialize placement */
    placement->sprite = sprite;
    placement->canvas = canvas;
    placement->initial_x = x;
    placement->initial_y = y;
    placement->vx = 0;
    placement->vy = 0;
    placement->ax = 0;
    placement->ay = 0;
    placement->anim_fn = NULL;
    placement->anim_priv = NULL;
    placement->prev = NULL;

    /* Insert at head of list (top layer) - O(1) */
    if (canvas->head != NULL) {
        canvas->head->prev = placement;
    }
    placement->next = canvas->head;
    canvas->head = placement;

    /* If this is the first placement, also set as tail */
    if (canvas->tail == NULL) {
        canvas->tail = placement;
    }

    /* Increment sprite reference count */
    sprite->ref_count++;

    return placement;
}

/**
 * Destroy a placement and remove it from the canvas.
 * Decrements the sprite's reference count.
 */
void animate_destroy_placement(struct spring_placement* placement) {
    if (placement == NULL) {
        return;
    }

    struct canvas* canvas = placement->canvas;

    /* Remove from linked list */
    if (placement->prev != NULL) {
        placement->prev->next = placement->next;
    } else {
        /* This was the head */
        canvas->head = placement->next;
    }

    if (placement->next != NULL) {
        placement->next->prev = placement->prev;
    } else {
        /* This was the tail */
        canvas->tail = placement->prev;
    }

    /* Decrement sprite reference count */
    if (placement->sprite != NULL) {
        placement->sprite->ref_count--;
    }

    free(placement);
}

/* ============================================================================
 * Layer Movement Operations (all O(1) time complexity)
 * ============================================================================ */

/**
 * Move placement up one layer (towards top).
 * O(1) time complexity.
 */
void animate_placement_up(struct spring_placement* placement) {
    if (placement == NULL || placement->prev == NULL) {
        /* Already at top or invalid */
        return;
    }

    /* Swap with previous node */
    struct spring_placement* prev = placement->prev;

    /* Update pointers */
    placement->prev = prev->prev;
    if (prev->prev != NULL) {
        prev->prev->next = placement;
    } else {
        /* prev was the head, now placement is the head */
        placement->canvas->head = placement;
    }

    prev->next = placement->next;
    if (placement->next != NULL) {
        placement->next->prev = prev;
    } else {
        /* placement was the tail, now prev is the tail */
        placement->canvas->tail = prev;
    }

    placement->next = prev;
    prev->prev = placement;
}

/**
 * Move placement down one layer (towards bottom).
 * O(1) time complexity.
 */
void animate_placement_down(struct spring_placement* placement) {
    if (placement == NULL || placement->next == NULL) {
        /* Already at bottom or invalid */
        return;
    }

    /* Swap with next node (same as previous operation in reverse) */
    animate_placement_up(placement->next);
}

/**
 * Move placement to the top layer.
 * O(1) time complexity.
 */
void animate_placement_top(struct spring_placement* placement) {
    if (placement == NULL || placement->prev == NULL) {
        /* Already at top or invalid */
        return;
    }

    /* Remove from current position */
    if (placement->prev != NULL) {
        placement->prev->next = placement->next;
    }
    if (placement->next != NULL) {
        placement->next->prev = placement->prev;
    } else {
        /* Was tail */
        placement->canvas->tail = placement->prev;
    }

    /* Insert at head */
    placement->prev = NULL;
    placement->next = placement->canvas->head;
    if (placement->canvas->head != NULL) {
        placement->canvas->head->prev = placement;
    }
    placement->canvas->head = placement;

    /* If list was empty (shouldn't happen here), set tail */
    if (placement->canvas->tail == NULL) {
        placement->canvas->tail = placement;
    }
}

/**
 * Move placement to the bottom layer.
 * O(1) time complexity.
 */
void animate_placement_bottom(struct spring_placement* placement) {
    if (placement == NULL || placement->next == NULL) {
        /* Already at bottom or invalid */
        return;
    }

    /* Remove from current position */
    if (placement->prev != NULL) {
        placement->prev->next = placement->next;
    } else {
        /* Was head */
        placement->canvas->head = placement->next;
    }
    placement->next->prev = placement->prev;

    /* Insert at tail */
    placement->next = NULL;
    placement->prev = placement->canvas->tail;
    if (placement->canvas->tail != NULL) {
        placement->canvas->tail->next = placement;
    }
    placement->canvas->tail = placement;

    /* If list was empty (shouldn't happen here), set head */
    if (placement->canvas->head == NULL) {
        placement->canvas->head = placement;
    }
}

/* ============================================================================
 * Animation Parameters
 * ============================================================================ */

/**
 * Set animation parameters (velocity and acceleration).
 */
void animate_set_animation_params(struct spring_placement* placement,
                                    ssize_t vx, ssize_t vy,
                                    ssize_t ax, ssize_t ay) {
    if (placement == NULL) {
        return;
    }

    placement->vx = vx;
    placement->vy = vy;
    placement->ax = ax;
    placement->ay = ay;
}

/**
 * Set a custom animation function.
 */
void animate_set_animation_function(struct spring_placement* placement,
                                     animate_fn fn, void* priv) {
    if (placement == NULL) {
        return;
    }

    placement->anim_fn = fn;
    placement->anim_priv = priv;
}
