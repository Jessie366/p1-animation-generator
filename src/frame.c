#include <stddef.h>
#include <stdint.h>

#include "animate_internal.h"

static unsigned alpha_of(color_t c) {
    return (unsigned)((c >> 24) & 0xFFu);
}

static color_t force_opaque(color_t c) {
    return (c & 0x00FFFFFFu) | 0xFF000000u;
}

static size_t pixel_index(size_t width, size_t x, size_t y) {
    return y * width + x;
}

size_t animate_frame_size_bytes(struct canvas *canvas) {
    if (canvas == NULL) {
        return 0;
    }

    return canvas->height * canvas->width * sizeof(color_t);
}
