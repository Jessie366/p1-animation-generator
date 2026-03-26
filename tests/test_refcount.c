/*
 * test_refcount.c — Verify that a sprite in use cannot be freed,
 * and that destroy_placement correctly decrements the refcount.
 */
#include "../animate.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    color_t red = animate_color_argb(0xFF, 0xFF, 0, 0);
    struct sprite *s = animate_create_rectangle(2, 2, red, 1);
    assert(s != NULL);

    struct canvas *c = animate_create_canvas(4, 4, 0u);

    struct sprite_placement *p1 = animate_place_sprite(c, s, 0, 0);
    struct sprite_placement *p2 = animate_place_sprite(c, s, 1, 1);
    assert(p1 && p2);

    /* Sprite has refcount 2 — cannot be freed */
    assert(animate_destroy_sprite(s) == false);

    /* Remove one placement */
    animate_destroy_placement(p1);

    /* Still refcount 1 — cannot be freed */
    assert(animate_destroy_sprite(s) == false);

    /* Remove last placement */
    animate_destroy_placement(p2);

    /* Now refcount 0 — can be freed */
    assert(animate_destroy_sprite(s) == true);

    animate_destroy_canvas(c);

    fprintf(stdout, "test_refcount: PASS\n");
    return 0;
}
