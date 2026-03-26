/**
 * @file animate_internal.h
 * @brief Internal structures for PointerPro Animate
 *
 * This header defines the internal data structures used in the implementation.
 * These are not part of the public API.
 */

#ifndef ANIMATE_INTERNAL_H
#define ANIMATE_INTERNAL_H

#include "animate.h"

/**
 * @brief Shape type for sprites
 */
typedef enum {
    SPRITE_TYPE_BITMAP,
    SPRITE_TYPE_RECTANGLE,
    SPRITE_TYPE_CIRCLE
} sprite_type_t;

/**
 * @brief Internal sprite structure
 *
 * Stores the pixel data of a sprite and maintains a reference count
 * to ensure memory efficiency (same sprite data shared by multiple placements).
 */
struct spring {
    size_t width;          /**< Width of the sprite in pixels */
    size_t height;         /**< Height of the sprite in pixels */
    color_t* data;         /**< Pixel data array (ARGB32 format) */
    sprite_type_t type;    /**< Type of the sprite */
    int ref_count;        /**< Reference count for memory management */
};

/**
 * @brief Internal canvas structure
 *
 * Contains all information needed to manage an animation canvas.
 */
struct canvas {
    size_t height;                         /**< Canvas height in pixels */
    size_t width;                          /**< Canvas width in pixels */
    color_t background_color;              /**< Background color */

    /* Doubly-linked list of placements (head = top layer) */
    struct spring_placement* head;         /**< Topmost layer */
    struct spring_placement* tail;         /**< Bottommost layer */
};

/**
 * @brief Internal placement structure
 *
 * Represents a single instance of a sprite placed on a canvas.
 */
struct spring_placement {
    struct spring* sprite;                 /**< Pointer to the sprite data */

    /* Canvas back-reference for easy access */
    struct canvas* canvas;

    /* Animation parameters */
    ssize_t initial_x;                     /**< Initial X position */
    ssize_t initial_y;                     /**< Initial Y position */
    ssize_t vx;                            /**< Velocity in X direction */
    ssize_t vy;                            /**< Velocity in Y direction */
    ssize_t ax;                            /**< Acceleration in X direction */
    ssize_t ay;                            /**< Acceleration in Y direction */

    /* Custom animation function (NULL if not set) */
    animate_fn anim_fn;
    void* anim_priv;

    /* Doubly-linked list pointers for layer management */
    struct spring_placement* prev;         /**< Previous in layer order */
    struct spring_placement* next;         /**< Next in layer order */
};

#endif /* ANIMATE_INTERNAL_H */
