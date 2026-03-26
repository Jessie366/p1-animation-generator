#include <stddef.h>

#include "animate_internal.h"

size_t animate_frame_size_bytes(struct canvas *canvas) {
    if (canvas == NULL) {
        return 0;
    }

    return canvas->height * canvas->width * sizeof(color_t);
}
