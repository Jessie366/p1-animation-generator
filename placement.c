#include "animate_internal.h"

#include <stdlib.h>

/*
 * Unlink a placement from the canvas doubly-linked list without freeing it.
 * After this call p->prev and p->next are NULL.
 */
static void list_remove(struct sprite_placement *p) {
    struct canvas *c = p->canvas;

    if (p->prev) p->prev->next = p->next;
    else         c->bottom     = p->next;

    if (p->next) p->next->prev = p->prev;
    else         c->top        = p->prev;

    p->prev = NULL;
    p->next = NULL;
}

/*
 * Swap placement p with its successor p->next in the list.
 * Does nothing if p or p->next is NULL.
 */
static void swap_with_next(struct sprite_placement *p) {
    if (!p || !p->next) return;

    struct canvas          *c    = p->canvas;
    struct sprite_placement *q   = p->next;       /* the node above p */
    struct sprite_placement *pre = p->prev;       /* node below p */
    struct sprite_placement *nxt = q->next;       /* node above q */

    /* Attach node below p to q. */
    if (pre) pre->next = q;
    else     c->bottom = q;

    /* Relink: ... <-> pre <-> q <-> p <-> nxt <-> ... */
    q->prev = pre;
    q->next = p;
    p->prev = q;
    p->next = nxt;

    if (nxt) nxt->prev = p;
    else     c->top    = p;
}

/* Public: place sprite at top of the canvas layer stack. O(1). */
struct sprite_placement *animate_place_sprite(struct canvas *canvas,
                                              struct sprite *sprite,
                                              ssize_t x, ssize_t y) {
    struct sprite_placement *p = malloc(sizeof(*p));
    if (!p) return NULL;

    p->sprite    = sprite;
    p->x0        = x;
    p->y0        = y;
    p->vx        = 0;  p->vy = 0;
    p->ax        = 0;  p->ay = 0;
    p->canvas    = canvas;
    p->anim_fn   = NULL;
    p->anim_priv = NULL;

    /* Insert at the top of the list. */
    p->prev = canvas->top;
    p->next = NULL;
    if (canvas->top) canvas->top->next = p;
    else             canvas->bottom    = p;
    canvas->top = p;

    sprite->refcount++;
    return p;
}

/* Public: move placement one step toward the top. O(1). */
void animate_placement_up(struct sprite_placement *p) {
    swap_with_next(p);
}

/* Public: move placement one step toward the bottom. O(1). */
void animate_placement_down(struct sprite_placement *p) {
    if (p && p->prev) swap_with_next(p->prev);
}

/* Public: move placement to the very top of the stack. O(1). */
void animate_placement_top(struct sprite_placement *p) {
    struct canvas *c = p->canvas;
    if (c->top == p) return;

    list_remove(p);

    p->prev = c->top;
    p->next = NULL;
    if (c->top) c->top->next = p;
    else        c->bottom    = p;
    c->top = p;
}

/* Public: move placement to the very bottom of the stack. O(1). */
void animate_placement_bottom(struct sprite_placement *p) {
    struct canvas *c = p->canvas;
    if (c->bottom == p) return;

    list_remove(p);

    p->next = c->bottom;
    p->prev = NULL;
    if (c->bottom) c->bottom->prev = p;
    else           c->top          = p;
    c->bottom = p;
}

/* Public: remove placement from canvas and free it. */
void animate_destroy_placement(struct sprite_placement *p) {
    if (!p) return;
    list_remove(p);
    p->sprite->refcount--;
    free(p);
}

/* Public: store velocity and acceleration for physics animation. */
void animate_set_animation_params(struct sprite_placement *p,
                                  ssize_t vx, ssize_t vy,
                                  ssize_t ax, ssize_t ay) {
    p->vx = vx;  p->vy = vy;
    p->ax = ax;  p->ay = ay;
}

/* Optional extension: register a custom animation callback. */
void animate_set_animation_function(struct sprite_placement *p,
                                    animate_fn fn, void *priv) {
    p->anim_fn   = fn;
    p->anim_priv = priv;
}
