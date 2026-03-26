#include <stdbool.h>
#include <stdlib.h>

#include "animate_internal.h"

bool animate_destroy_sprite(struct sprite *sprite) {
    if (sprite == NULL) {
        return false;
    }

    if (sprite->ref_count != 0) {
        return false;
    }

    free(sprite->pixels);
    free(sprite);
    return true;
}
