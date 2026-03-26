#include "animate_internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* Allocate a new sprite struct with zero-initialised fields. */
static struct sprite *alloc_sprite(void) {
    struct sprite *s = malloc(sizeof(*s));
    if (!s) return NULL;
    s->pixels   = NULL;
    s->width    = 0;
    s->height   = 0;
    s->refcount = 0;
    s->filename = NULL;
    s->loaded   = false;
    return s;
}

/*
 * Load pixel data from a BMP (ARGB32) file into s->pixels.
 * BMP rows are stored bottom-to-top; we store them top-to-bottom
 * so that index 0 corresponds to the top row of the image.
 * Sets s->loaded = true on return (even on error) to prevent retries.
 */
bool sprite_load_bmp(struct sprite *s) {
    s->loaded = true;

    FILE *fp = fopen(s->filename, "rb");
    if (!fp) return false;

    struct bitmap_header bh;
    if (fread(&bh, sizeof(bh), 1, fp) != 1) { fclose(fp); return false; }
    if (bh.magic[0] != 'B' || bh.magic[1] != 'M') { fclose(fp); return false; }

    struct bitmapv5_header v5h;
    if (fread(&v5h, sizeof(v5h), 1, fp) != 1) { fclose(fp); return false; }

    s->width  = (size_t)v5h.bV5Width;
    s->height = (size_t)v5h.bV5Height;

    size_t total = s->width * s->height;
    s->pixels = malloc(total * sizeof(color_t));
    if (!s->pixels) { fclose(fp); return false; }

    /* Seek to the start of pixel data. */
    if (fseek(fp, (long)bh.pixel_offset, SEEK_SET) != 0) {
        free(s->pixels); s->pixels = NULL;
        fclose(fp); return false;
    }

    /*
     * Read BMP rows sequentially (bottom row first) and store them in
     * reverse order so that row 0 of pixels[] = top row of image.
     * When row counts down from height-1 to 0 we store:
     *   file row 0 -> pixels[height-1]  (bottom of image)
     *   file row 1 -> pixels[height-2]
     *   ...
     *   file row height-1 -> pixels[0]  (top of image)
     */
    for (ssize_t row = (ssize_t)s->height - 1; row >= 0; row--) {
        size_t dest = (size_t)row * s->width;
        if (fread(s->pixels + dest, sizeof(color_t), s->width, fp) != s->width) {
            free(s->pixels); s->pixels = NULL;
            fclose(fp); return false;
        }
    }

    fclose(fp);
    return true;
}

/* Public: create a sprite backed by a BMP file (lazy load). */
struct sprite *animate_create_sprite(const char *file) {
    struct sprite *s = alloc_sprite();
    if (!s) return NULL;

    size_t len = strlen(file);
    s->filename = malloc(len + 1);
    if (!s->filename) { free(s); return NULL; }
    memcpy(s->filename, file, len + 1);

    return s;
}

/* Public: create a filled or unfilled rectangle sprite. */
struct sprite *animate_create_rectangle(size_t width, size_t height,
                                        color_t c, bool filled) {
    struct sprite *s = alloc_sprite();
    if (!s) return NULL;

    s->width  = width;
    s->height = height;
    s->loaded = true;

    size_t total = width * height;
    s->pixels = malloc(total * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    /* Force fully-opaque colour. */
    color_t opaque = (c & 0x00FFFFFFu) | 0xFF000000u;

    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            bool on_border = (row == 0 || row == height - 1 ||
                              col == 0 || col == width  - 1);
            if (filled || on_border) {
                s->pixels[row * width + col] = opaque;
            } else {
                s->pixels[row * width + col] = 0; /* alpha = 0 => transparent */
            }
        }
    }

    return s;
}

/* Public: create a filled circle sprite with bounding box (2r+1) x (2r+1). */
struct sprite *animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled; /* currently only filled circles are supported */

    struct sprite *s = alloc_sprite();
    if (!s) return NULL;

    size_t dim    = 2 * radius + 1;
    s->width      = dim;
    s->height     = dim;
    s->loaded     = true;

    s->pixels = malloc(dim * dim * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    color_t opaque = (c & 0x00FFFFFFu) | 0xFF000000u;
    ssize_t r      = (ssize_t)radius;
    ssize_t r2     = r * r;

    for (size_t row = 0; row < dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            ssize_t dx = (ssize_t)col - r;
            ssize_t dy = (ssize_t)row - r;
            if (dx * dx + dy * dy <= r2) {
                s->pixels[row * dim + col] = opaque;
            } else {
                s->pixels[row * dim + col] = 0;
            }
        }
    }

    return s;
}

/*
 * Public: free a sprite.
 * Returns true on success, false if the sprite is still in use (refcount > 0).
 */
bool animate_destroy_sprite(struct sprite *sprite) {
    if (!sprite) return false;
    if (sprite->refcount > 0) return false;
    free(sprite->pixels);
    free(sprite->filename);
    free(sprite);
    return true;
}
