/* test_canvas.c — Tests for canvas create/destroy and frame size. */
#include "../animate.h"
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>

int main(void) {
    /* Basic creation */
    struct canvas *c = animate_create_canvas(4, 4, 0xFF000000u);
    assert(c != NULL);

    /* Frame size must equal height * width * 4 bytes */
    size_t sz = animate_frame_size_bytes(c);
    assert(sz == 4 * 4 * sizeof(uint32_t));

    animate_destroy_canvas(c);

    /* Zero-size canvas */
    struct canvas *c2 = animate_create_canvas(0, 0, 0);
    assert(c2 != NULL);
    assert(animate_frame_size_bytes(c2) == 0);
    animate_destroy_canvas(c2);

    fprintf(stdout, "test_canvas: PASS\n");
    return 0;
}
