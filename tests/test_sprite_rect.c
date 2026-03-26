/* test_sprite_rect.c — Tests for rectangle sprite creation. */
#include "../animate.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

static color_t alpha_of(color_t c) { return c >> 24; }

int main(void) {
    /* Filled 2x2 — all pixels must be opaque with the given colour */
    color_t red = animate_color_rgb(255, 0, 0);
    struct sprite *s = animate_create_rectangle(2, 2, red, 1);
    assert(s != NULL);

    /* Generate a frame on a 2x2 canvas and inspect pixel data */
    struct canvas *c = animate_create_canvas(2, 2, 0u);
    struct sprite_placement *p = animate_place_sprite(c, s, 0, 0);
    assert(p != NULL);

    color_t buf[4];
    animate_generate_frame(c, 0, 25, buf);

    for (int i = 0; i < 4; i++) {
        assert(alpha_of(buf[i]) == 0xFF);
        /* RGB part must match red with alpha forced */
        assert((buf[i] & 0x00FFFFFFu) == (red & 0x00FFFFFFu));
    }

    animate_destroy_canvas(c);
    assert(animate_destroy_sprite(s) == true);

    /* Unfilled 4x4 — interior pixels transparent, border opaque */
    struct sprite *s2 = animate_create_rectangle(4, 4, red, 0);
    assert(s2 != NULL);

    struct canvas *c2 = animate_create_canvas(4, 4, 0u);
    struct sprite_placement *p2 = animate_place_sprite(c2, s2, 0, 0);
    assert(p2 != NULL);

    color_t buf2[16];
    animate_generate_frame(c2, 0, 25, buf2);

    /* Interior pixel at (row=1,col=1) must be transparent (background=0) */
    assert(alpha_of(buf2[1 * 4 + 1]) == 0x00);
    /* Border pixel at (row=0,col=0) must be opaque */
    assert(alpha_of(buf2[0 * 4 + 0]) == 0xFF);
    /* Border pixel at (row=3,col=3) must be opaque */
    assert(alpha_of(buf2[3 * 4 + 3]) == 0xFF);

    animate_destroy_canvas(c2);
    assert(animate_destroy_sprite(s2) == true);

    fprintf(stdout, "test_sprite_rect: PASS\n");
    return 0;
}
