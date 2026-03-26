/*
 * test_placement.c — Tests for placement layer ordering.
 *
 * Uses two 1x1 sprites of different colours placed on a 1x1 canvas.
 * Whichever sprite is on top wins the pixel.
 */
#include "../animate.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    color_t red  = animate_color_argb(0xFF, 0xFF, 0, 0);
    color_t blue = animate_color_argb(0xFF, 0, 0, 0xFF);

    struct sprite *sr = animate_create_rectangle(1, 1, red,  1);
    struct sprite *sb = animate_create_rectangle(1, 1, blue, 1);
    assert(sr && sb);

    struct canvas *c = animate_create_canvas(1, 1, 0u);

    /* Place red first (bottom), blue second (top) */
    struct sprite_placement *pr = animate_place_sprite(c, sr, 0, 0);
    struct sprite_placement *pb = animate_place_sprite(c, sb, 0, 0);
    assert(pr && pb);

    color_t buf;
    animate_generate_frame(c, 0, 25, &buf);
    /* Blue should win (it's on top) */
    assert((buf & 0x00FFFFFFu) == (blue & 0x00FFFFFFu));

    /* Move red to top */
    animate_placement_top(pr);
    animate_generate_frame(c, 0, 25, &buf);
    assert((buf & 0x00FFFFFFu) == (red & 0x00FFFFFFu));

    /* Move red back to bottom */
    animate_placement_bottom(pr);
    animate_generate_frame(c, 0, 25, &buf);
    assert((buf & 0x00FFFFFFu) == (blue & 0x00FFFFFFu));

    /* Move blue down one step — red should win */
    animate_placement_down(pb);
    animate_generate_frame(c, 0, 25, &buf);
    assert((buf & 0x00FFFFFFu) == (red & 0x00FFFFFFu));

    /* Move blue up one step — blue should win again */
    animate_placement_up(pb);
    animate_generate_frame(c, 0, 25, &buf);
    assert((buf & 0x00FFFFFFu) == (blue & 0x00FFFFFFu));

    animate_destroy_canvas(c);
    assert(animate_destroy_sprite(sr) == true);
    assert(animate_destroy_sprite(sb) == true);

    fprintf(stdout, "test_placement: PASS\n");
    return 0;
}
