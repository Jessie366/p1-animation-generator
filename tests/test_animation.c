/*
 * test_animation.c — Tests for physics-based animation params and clipping.
 *
 * A 1x1 red sprite starts at (0,0) with velocity vx=4 pixels/s.
 * At frame 1 of 4 fps → t = 0.25 s → x = 1.
 * The canvas is 4x1, so we can verify the sprite moves correctly.
 */
#include "../animate.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>

int main(void) {
    color_t red  = animate_color_argb(0xFF, 0xFF, 0, 0);
    color_t bg   = 0u;

    struct sprite *sr = animate_create_rectangle(1, 1, red, 1);
    assert(sr != NULL);

    /* 1 row, 4 cols canvas */
    struct canvas *c = animate_create_canvas(1, 4, bg);
    struct sprite_placement *p = animate_place_sprite(c, sr, 0, 0);
    assert(p != NULL);

    /* vx = 4 px/s, vy = 0 */
    animate_set_animation_params(p, 4, 0, 0, 0);

    color_t buf[4];

    /* frame 0, fps 4 → t=0 → sprite at col 0 */
    animate_generate_frame(c, 0, 4, buf);
    assert((buf[0] & 0x00FFFFFFu) == (red & 0x00FFFFFFu));
    assert((buf[1] >> 24) == 0x00);

    /* frame 1, fps 4 → t=0.25 → x = 0 + 4*0.25 = 1 → sprite at col 1 */
    animate_generate_frame(c, 1, 4, buf);
    assert((buf[0] >> 24) == 0x00);
    assert((buf[1] & 0x00FFFFFFu) == (red & 0x00FFFFFFu));

    /* frame 4, fps 4 → t=1 → x = 4 → out of 4-wide canvas → background */
    animate_generate_frame(c, 4, 4, buf);
    for (int i = 0; i < 4; i++) assert((buf[i] >> 24) == 0x00);

    animate_destroy_canvas(c);
    assert(animate_destroy_sprite(sr) == true);

    fprintf(stdout, "test_animation: PASS\n");
    return 0;
}
