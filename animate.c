/*
 * animate.c — PointerPro Animate: complete implementation.
 *
 * Internal struct definitions are in animate_internal.h.
 * All required API functions declared in animate.h are implemented here.
 */

#include "animate.h"
#include "animate_internal.h"

#include <stdlib.h>
#include <string.h>
#include <stdio.h>

/* =========================================================================
 * SPRITE — creation and destruction
 * ========================================================================= */

/* Allocate and zero-initialise a sprite struct. */
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
 * Load pixel data from an ARGB32 BMP file into s->pixels.
 * BMP rows are stored bottom-to-top; we reverse them so pixels[0] is the
 * top row of the image.  Sets s->loaded = true to prevent repeated attempts.
 */
static void load_bmp_pixels(struct sprite *s) {
    s->loaded = true;

    FILE *fp = fopen(s->filename, "rb");
    if (!fp) return;

    struct bitmap_header bh;
    if (fread(&bh, sizeof(bh), 1, fp) != 1) { fclose(fp); return; }
    if (bh.magic[0] != 'B' || bh.magic[1] != 'M') { fclose(fp); return; }

    struct bitmapv5_header v5h;
    if (fread(&v5h, sizeof(v5h), 1, fp) != 1) { fclose(fp); return; }

    s->width  = (size_t)v5h.bV5Width;
    s->height = (size_t)v5h.bV5Height;

    s->pixels = malloc(s->width * s->height * sizeof(color_t));
    if (!s->pixels) { fclose(fp); return; }

    if (fseek(fp, (long)bh.pixel_offset, SEEK_SET) != 0) {
        free(s->pixels); s->pixels = NULL;
        fclose(fp); return;
    }

    /*
     * Read BMP rows sequentially (bottom row first from file) and store in
     * reverse so that pixels[0] corresponds to the top row of the image:
     *   file row 0  -> pixels[(height-1) * width]  (bottom of image)
     *   ...
     *   file row h-1 -> pixels[0]                  (top of image)
     */
    for (ssize_t row = (ssize_t)s->height - 1; row >= 0; row--) {
        size_t dest = (size_t)row * s->width;
        if (fread(s->pixels + dest, sizeof(color_t), s->width, fp) != s->width) {
            free(s->pixels); s->pixels = NULL;
            fclose(fp); return;
        }
    }

    fclose(fp);
}

/* Public: create a sprite backed by a BMP file (lazy-loaded). */
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

    s->pixels = malloc(width * height * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    color_t opaque = (c & 0x00FFFFFFu) | 0xFF000000u;

    for (size_t row = 0; row < height; row++) {
        for (size_t col = 0; col < width; col++) {
            bool border = (row == 0 || row == height - 1 ||
                           col == 0 || col == width  - 1);
            s->pixels[row * width + col] = (filled || border) ? opaque : 0u;
        }
    }

    return s;
}

/*
 * Public: create a filled circle sprite.
 * Bounding box is (2r+1) x (2r+1); a pixel at offset (dx,dy) from the centre
 * is opaque if dx*dx + dy*dy <= r*r.
 */
struct sprite *animate_create_circle(size_t radius, color_t c, bool filled) {
    (void)filled; /* only filled circles are currently supported */

    struct sprite *s = alloc_sprite();
    if (!s) return NULL;

    size_t dim = 2 * radius + 1;
    s->width   = dim;
    s->height  = dim;
    s->loaded  = true;

    s->pixels = malloc(dim * dim * sizeof(color_t));
    if (!s->pixels) { free(s); return NULL; }

    color_t opaque = (c & 0x00FFFFFFu) | 0xFF000000u;
    ssize_t r  = (ssize_t)radius;
    ssize_t r2 = r * r;

    for (size_t row = 0; row < dim; row++) {
        for (size_t col = 0; col < dim; col++) {
            ssize_t dx = (ssize_t)col - r;
            ssize_t dy = (ssize_t)row - r;
            s->pixels[row * dim + col] =
                (dx * dx + dy * dy <= r2) ? opaque : 0u;
        }
    }

    return s;
}

/*
 * Public: free a sprite.
 * Returns true on success; false if the sprite is still referenced by a
 * placement (refcount > 0).
 */
bool animate_destroy_sprite(struct sprite *sprite) {
    if (!sprite) return false;
    if (sprite->refcount > 0) return false;
    free(sprite->pixels);
    free(sprite->filename);
    free(sprite);
    return true;
}

/* =========================================================================
 * CANVAS — creation and destruction
 * ========================================================================= */

/* Public: allocate and initialise an empty canvas. */
struct canvas *animate_create_canvas(size_t height, size_t width,
                                     color_t background_color) {
    struct canvas *c = malloc(sizeof(*c));
    if (!c) return NULL;
    c->height           = height;
    c->width            = width;
    c->background_color = background_color;
    c->bottom           = NULL;
    c->top              = NULL;
    return c;
}

/*
 * Public: destroy canvas and all its placements (NOT the sprites).
 * Decrements each sprite's refcount as placements are removed.
 */
void animate_destroy_canvas(struct canvas *canvas) {
    if (!canvas) return;
    struct sprite_placement *p = canvas->bottom;
    while (p) {
        struct sprite_placement *next = p->next;
        p->sprite->refcount--;
        free(p);
        p = next;
    }
    free(canvas);
}

/* Public: return the byte size of one frame buffer for this canvas. */
size_t animate_frame_size_bytes(struct canvas *canvas) {
    return canvas->height * canvas->width * sizeof(color_t);
}

/* =========================================================================
 * PLACEMENT — layer management
 * ========================================================================= */

/*
 * Detach a placement from the doubly-linked list without freeing it.
 * After this call p->prev and p->next are both NULL.
 */
static void list_remove(struct sprite_placement *p) {
    struct canvas *c = p->canvas;
    if (p->prev) p->prev->next = p->next; else c->bottom = p->next;
    if (p->next) p->next->prev = p->prev; else c->top    = p->prev;
    p->prev = NULL;
    p->next = NULL;
}

/*
 * Swap placement p with its immediate successor p->next in the list.
 * List before: ... <-> prev <-> p <-> q <-> next <-> ...
 * List after:  ... <-> prev <-> q <-> p <-> next <-> ...
 */
static void swap_with_next(struct sprite_placement *p) {
    if (!p || !p->next) return;

    struct canvas          *c   = p->canvas;
    struct sprite_placement *q  = p->next;
    struct sprite_placement *pre = p->prev;
    struct sprite_placement *nxt = q->next;

    if (pre) pre->next = q; else c->bottom = q;
    q->prev = pre;
    q->next = p;
    p->prev = q;
    p->next = nxt;
    if (nxt) nxt->prev = p; else c->top = p;
}

/* Public: place sprite at the top of the canvas layer stack. O(1). */
struct sprite_placement *animate_place_sprite(struct canvas *canvas,
                                              struct sprite *sprite,
                                              ssize_t x, ssize_t y) {
    struct sprite_placement *p = malloc(sizeof(*p));
    if (!p) return NULL;

    p->sprite    = sprite;
    p->x0        = x;   p->y0 = y;
    p->vx        = 0;   p->vy = 0;
    p->ax        = 0;   p->ay = 0;
    p->canvas    = canvas;
    p->anim_fn   = NULL;
    p->anim_priv = NULL;

    p->prev = canvas->top;
    p->next = NULL;
    if (canvas->top) canvas->top->next = p; else canvas->bottom = p;
    canvas->top = p;

    sprite->refcount++;
    return p;
}

/* Public: raise placement one level toward the top. O(1). */
void animate_placement_up(struct sprite_placement *p) {
    swap_with_next(p);
}

/* Public: lower placement one level toward the bottom. O(1). */
void animate_placement_down(struct sprite_placement *p) {
    if (p && p->prev) swap_with_next(p->prev);
}

/* Public: raise placement to the very top of the stack. O(1). */
void animate_placement_top(struct sprite_placement *p) {
    struct canvas *c = p->canvas;
    if (c->top == p) return;
    list_remove(p);
    p->prev = c->top;
    p->next = NULL;
    if (c->top) c->top->next = p; else c->bottom = p;
    c->top = p;
}

/* Public: lower placement to the very bottom of the stack. O(1). */
void animate_placement_bottom(struct sprite_placement *p) {
    struct canvas *c = p->canvas;
    if (c->bottom == p) return;
    list_remove(p);
    p->next = c->bottom;
    p->prev = NULL;
    if (c->bottom) c->bottom->prev = p; else c->top = p;
    c->bottom = p;
}

/* Public: remove placement from canvas and free it. */
void animate_destroy_placement(struct sprite_placement *p) {
    if (!p) return;
    list_remove(p);
    p->sprite->refcount--;
    free(p);
}

/* Public: store physics animation parameters. */
void animate_set_animation_params(struct sprite_placement *p,
                                  ssize_t vx, ssize_t vy,
                                  ssize_t ax, ssize_t ay) {
    p->vx = vx; p->vy = vy;
    p->ax = ax; p->ay = ay;
}

/* Optional extension: register a custom animation callback. */
void animate_set_animation_function(struct sprite_placement *p,
                                    animate_fn fn, void *priv) {
    p->anim_fn   = fn;
    p->anim_priv = priv;
}

/* =========================================================================
 * FRAME GENERATION
 * ========================================================================= */

/*
 * Public: render one animation frame into buf.
 *
 * 1. Fill buf with background_color.
 * 2. Compute t = frame / frame_rate (seconds).
 * 3. Walk placements bottom → top (bottom layer drawn first).
 * 4. For each placement compute position:
 *      x = x0 + vx*t + ax*t*t/2
 *      y = y0 + vy*t + ay*t*t/2
 *    (or delegate to the custom anim_fn if set).
 * 5. Blit opaque sprite pixels onto the frame, clipping to canvas bounds.
 *    All written pixels have alpha forced to 0xFF.
 */
void animate_generate_frame(const struct canvas *canvas,
                            size_t frame, size_t frame_rate,
                            void *buf) {
    color_t *fb    = (color_t *)buf;
    size_t   total = canvas->width * canvas->height;

    for (size_t i = 0; i < total; i++) {
        fb[i] = canvas->background_color;
    }

    float t = (frame_rate > 0) ? ((float)frame / (float)frame_rate) : 0.0f;

    struct sprite_placement *p = canvas->bottom;
    while (p) {
        struct sprite *s = p->sprite;

        /* Lazy-load BMP pixel data on first access. */
        if (!s->loaded) load_bmp_pixels(s);

        /* Skip sprites with no pixel data (e.g. failed BMP load). */
        if (!s->pixels) { p = p->next; continue; }

        /* Compute canvas position for this frame. */
        ssize_t px, py;
        if (p->anim_fn) {
            px = p->x0; py = p->y0;
            p->anim_fn(p->anim_priv, &px, &py, t);
        } else {
            px = p->x0 + (ssize_t)((float)p->vx * t
                                   + (float)p->ax * t * t / 2.0f);
            py = p->y0 + (ssize_t)((float)p->vy * t
                                   + (float)p->ay * t * t / 2.0f);
        }

        /* Blit sprite pixels, skipping transparent ones and out-of-bounds. */
        for (size_t row = 0; row < s->height; row++) {
            ssize_t dr = py + (ssize_t)row;
            if (dr < 0 || dr >= (ssize_t)canvas->height) continue;

            for (size_t col = 0; col < s->width; col++) {
                color_t pixel = s->pixels[row * s->width + col];
                if ((pixel >> 24) == 0) continue; /* fully transparent */

                ssize_t dc = px + (ssize_t)col;
                if (dc < 0 || dc >= (ssize_t)canvas->width) continue;

                fb[(size_t)dr * canvas->width + (size_t)dc] =
                    (pixel & 0x00FFFFFFu) | 0xFF000000u;
            }
        }

        p = p->next;
    }
}
