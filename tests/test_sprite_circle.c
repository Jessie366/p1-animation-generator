/* test_sprite_circle.c — Tests for circle sprite creation. */
#include "../animate.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    /*
     * Radius-1 circle: bounding box 3x3, centre at (1,1).
     * Pixels inside circle (dx^2 + dy^2 <= 1): (1,0),(0,1),(1,1),(2,1),(1,2)
     * Corners (0,0),(2,0),(0,2),(2,2) are outside.
     */
    color_t blue = animate_color_rgb(0, 0, 255);
    struct sprite *s = animate_create_circle(1, blue, 1);
    assert(s != NULL);

    struct canvas *c = animate_create_canvas(3, 3, 0u);
    struct sprite_placement *p = animate_place_sprite(c, s, 0, 0);
    assert(p != NULL);

    color_t buf[9];
    animate_generate_frame(c, 0, 25, buf);

    /* Centre pixel must be opaque */
    assert((buf[1 * 3 + 1] >> 24) == 0xFF);
    /* Cardinal neighbours must be opaque */
    assert((buf[0 * 3 + 1] >> 24) == 0xFF);
    assert((buf[2 * 3 + 1] >> 24) == 0xFF);
    assert((buf[1 * 3 + 0] >> 24) == 0xFF);
    assert((buf[1 * 3 + 2] >> 24) == 0xFF);
    /* Corners must be transparent (background = 0) */
    assert((buf[0 * 3 + 0] >> 24) == 0x00);
    assert((buf[0 * 3 + 2] >> 24) == 0x00);
    assert((buf[2 * 3 + 0] >> 24) == 0x00);
    assert((buf[2 * 3 + 2] >> 24) == 0x00);

    animate_destroy_canvas(c);
    assert(animate_destroy_sprite(s) == true);

    fprintf(stdout, "test_sprite_circle: PASS\n");
    return 0;
}
