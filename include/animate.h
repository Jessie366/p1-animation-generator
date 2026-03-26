/**
 * @file animate.h
 * @brief PointerPro Animate API
 *
 * This header defines the API for the PointerPro Animate animation generator.
 * Students must implement all functions declared in this header file.
 */

#ifndef ANIMATE_H
#define ANIMATE_H

#include <stdbool.h>
#include <stdint.h>
#include <sys/types.h>

/* Forward declarations - these are opaque pointers */
struct canvas;
struct sprite;
struct sprite_placement;

/**
 * @brief Representation of a pixel color in ARGB32 format.
 *
 * The format is: AA (bits 24-31), RR (bits 16-23), GG (bits 8-15), BB (bits 0-7)
 */
typedef uint32_t color_t;

/**
 * @brief Custom animation function type for optional extension.
 *
 * @param priv Private data provided when the function was registered
 * @param x Pointer to the X position (input: starting position, output: target position)
 * @param y Pointer to the Y position (input: starting position, output: target position)
 * @param t Current time since the start of the animation (in seconds)
 */
typedef void (*animate_fn)(void* priv, ssize_t* x, ssize_t* y, float t);

/**
 * @brief Create a color from RGB components (alpha set to 0xFF)
 *
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return color_t The combined color value
 */
static inline color_t animate_color_rgb(unsigned r, unsigned g, unsigned b) {
    return 0xFF000000 | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

/**
 * @brief Create a color from ARGB components
 *
 * @param a Alpha component (0-255)
 * @param r Red component (0-255)
 * @param g Green component (0-255)
 * @param b Blue component (0-255)
 * @return color_t The combined color value
 */
static inline color_t animate_color_argb(unsigned a, unsigned r, unsigned g, unsigned b) {
    return ((a & 0xFF) << 24) | ((r & 0xFF) << 16) | ((g & 0xFF) << 8) | (b & 0xFF);
}

/**
 * @brief Create the canvas on which our animation will be built.
 *
 * @param width Width of the drawing area in pixels
 * @param height Height of the drawing area in pixels
 * @return struct canvas* Abstract data type for the canvas.
 */
struct canvas* animate_create_canvas(uint16_t width, uint16_t height);

/**
 * @brief Clean up a canvas.
 *
 * Frees all memory associated with the canvas, including placements, but not sprites.
 *
 * @param canvas The canvas to clean up
 */
void animate_destroy_canvas(struct canvas* canvas);

/**
 * @brief Create a sprite from the provided input bitmap file.
 *
 * The file format can be assumed to be ARGB32 BMP.
 * NOTE: The order of image rows differs to what we expect.
 * The first pixel in the BMP file format is on the bottom left while the last pixel is the top right.
 * For simplicity, reverse the order of rows while loading the bitmap.
 *
 * @param file The name of the bitmap file to be loaded
 * @return struct sprite* A handle to the created sprite, or NULL on error
 */
struct sprite* animate_create_sprite(const char* file);

/**
 * @brief Create a sprite that represents a rectangle.
 *
 * @param width The width of the rectangle
 * @param height The height of the rectangle
 * @param c The colour of the rectangle
 * @param filled true if the rectangle should be filled, otherwise false (border only)
 * @return struct sprite* A handle to the created sprite
 */
struct sprite* animate_create_rectangle(size_t width, size_t height, color_t c, bool filled);

/**
 * @brief Create a sprite that represents a circle.
 *
 * We currently only support filled circles.
 * The circle pixels can be determined by the following formula:
 * x*x + y*y < r*r
 * Where x is the x offset from the center of the circle
 * y is the y offset from the center of the circle
 * r is the radius of the circle
 *
 * @param radius The radius of the circle
 * @param c The colour of the circle
 * @param filled Reserved for future use (must be true)
 * @return struct sprite* A handle to the created sprite
 */
struct sprite* animate_create_circle(size_t radius, color_t c, bool filled);

/**
 * @brief Frees all memory associated with a sprite.
 *
 * @param sprite The sprite to clean up
 * @return true on success, false if the sprite is still in use (has active placements)
 */
bool animate_destroy_sprite(struct sprite* sprite);

/**
 * @brief Places a sprite on the top layer of the canvas.
 *
 * @param canvas The canvas to place the sprite on
 * @param sprite The sprite to place
 * @param x The x coordinate of the canvas to which the top left pixel of the sprite should be placed
 * @param y The y coordinate of the canvas to which the top left pixel of the sprite should be placed
 * @return struct sprite_placement* A handle to the sprite placement
 */
struct sprite_placement* animate_place_sprite(struct canvas* canvas, struct sprite* sprite,
                                               ssize_t x, ssize_t y);

/**
 * @brief Raise a sprite one level towards the top layer.
 *
 * @param sprite_placement A handle to the placement
 */
void animate_placement_up(struct sprite_placement* sprite_placement);

/**
 * @brief Lower a sprite one level towards the bottom layer.
 *
 * @param sprite_placement A handle to the placement
 */
void animate_placement_down(struct sprite_placement* sprite_placement);

/**
 * @brief Raise the sprite to the top-most layer.
 *
 * @param sprite_placement A handle to the placement
 */
void animate_placement_top(struct sprite_placement* sprite_placement);

/**
 * @brief Lower the sprite to the bottom-most layer.
 *
 * @param sprite_placement A handle to the placement
 */
void animate_placement_bottom(struct sprite_placement* sprite_placement);

/**
 * @brief Removes the placement from its associated canvas and free resources.
 *
 * @param sprite_placement A handle to the placement
 */
void animate_destroy_placement(struct sprite_placement* sprite_placement);

/**
 * @brief Configure sprite animation.
 *
 * Currently, we support only physics simulation (velocity and acceleration).
 *
 * @param sprite_placement A handle to the placement
 * @param vx The initial velocity in the x direction
 * @param vy The initial velocity in the y direction
 * @param ax The acceleration in the x direction
 * @param ay The acceleration in the y direction
 */
void animate_set_animation_params(struct sprite_placement* sprite_placement,
                                  ssize_t vx, ssize_t vy,
                                  ssize_t ax, ssize_t ay);

/**
 * @brief [Optional extension] Define animation using the provided function pointer
 *
 * @param sprite_placement A handle to the placement
 * @param fn A custom function to call when determining the position of the placement
 * @param priv Private data that is passed to the function
 */
void animate_set_animation_function(struct sprite_placement* sprite_placement,
                                     animate_fn fn, void* priv);

/**
 * @brief Return the size in bytes of an animation frame.
 *
 * This is useful for the caller to determine how much memory to allocate
 * when combining sprites into a frame.
 *
 * @param canvas The canvas in question
 * @return size_t The size, in bytes, of a frame for this canvas
 */
size_t animate_frame_size_bytes(struct canvas* canvas);

/**
 * @brief Generate a frame of the animation.
 *
 * @param canvas The canvas describing the animation
 * @param frame The frame number (starting from 0)
 * @param frame_rate The number of frames per second in the animation
 * @param buf A buffer provided by the user that is large enough to hold a frame.
 */
void animate_generate_frame(struct canvas* canvas, size_t frame, size_t frame_rate, void* buf);

#endif /* ANIMATE_H */
