#ifndef ANIMATE_INTERNAL_H
#define ANIMATE_INTERNAL_H

#include "animate.h"
#include <stddef.h>
#include <stdint.h>
#include <stdbool.h>

/* Pixel data loaded either immediately (rectangle/circle) or lazily (BMP). */
struct sprite {
    color_t        *pixels;    /* row-major, row 0 = top of image */
    size_t          width;
    size_t          height;
    int             refcount;  /* number of active placements */
    char           *filename;  /* non-NULL for BMP sprites (lazy load) */
    bool            loaded;    /* true once pixel data has been populated */
};

/*
 * Doubly-linked list node ordered bottom-to-top.
 *   prev -> toward bottom (drawn first)
 *   next -> toward top    (drawn last / on top)
 */
struct sprite_placement {
    struct sprite          *sprite;
    ssize_t                 x0, y0;       /* initial position */
    ssize_t                 vx, vy;       /* velocity (pixels/s) */
    ssize_t                 ax, ay;       /* acceleration (pixels/s^2) */
    struct sprite_placement *prev;        /* toward bottom */
    struct sprite_placement *next;        /* toward top */
    struct canvas           *canvas;      /* owning canvas */
    animate_fn               anim_fn;     /* optional custom animation fn */
    void                    *anim_priv;   /* private data for anim_fn */
};

struct canvas {
    size_t                   height;
    size_t                   width;
    color_t                  background_color;
    struct sprite_placement *bottom;  /* first drawn (lowest layer) */
    struct sprite_placement *top;     /* last drawn (highest layer) */
};

/* BMP file header (14 bytes, packed). */
struct bitmap_header {
    uint8_t  magic[2];       /* {'B', 'M'} */
    uint32_t size_bytes;     /* file size in bytes */
    uint16_t reserved[2];
    uint32_t pixel_offset;  /* byte offset to pixel data */
} __attribute__((packed));

/* BITMAPV5HEADER immediately follows bitmap_header. */
struct bitmapv5_header {
    uint32_t bV5Size;
    uint32_t bV5Width;
    uint32_t bV5Height;
    uint16_t bV5Planes;
    uint16_t bV5BitCount;
    uint32_t bV5Compression;
    uint32_t bV5SizeImage;
    uint32_t bV5XPelsPerMeter;
    uint32_t bV5YPelsPerMeter;
    uint32_t bV5ClrUsed;
    uint32_t bV5ClrImportant;
    uint32_t bV5RedMask;
    uint32_t bV5GreenMask;
    uint32_t bV5BlueMask;
    uint32_t bV5AlphaMask;
    uint32_t bV5CSType;
    uint8_t  bV5Endpoints[36];
    uint32_t bV5GammaRed;
    uint32_t bV5GammaGreen;
    uint32_t bV5GammaBlue;
    uint32_t bV5Intent;
    uint32_t bV5ProfileData;
    uint32_t bV5ProfileSize;
    uint32_t bV5Reserved;
};

#endif /* ANIMATE_INTERNAL_H */
