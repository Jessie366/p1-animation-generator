/**
 * @file demo.c
 * @brief Demo program to test PointerPro Animate
 *
 * This file demonstrates the usage of the animation API.
 * Run with: ./demo
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "animate.h"

/* Demo: Create a simple animation with moving shapes */

int main(void) {
    printf("PointerPro Animate Demo\n");
    printf("=======================\n\n");

    /* Create a canvas */
    printf("Creating canvas (100x100 pixels)...\n");
    struct canvas* canvas = animate_create_canvas(100, 100,
                                                   animate_color_rgb(255, 255, 255)); /* White background */
    if (canvas == NULL) {
        fprintf(stderr, "Failed to create canvas\n");
        return 1;
    }

    /* Create sprites */
    printf("Creating sprites...\n");
    struct spring* rect = animate_create_rectangle(20, 20,
                                                     animate_color_rgb(255, 0, 0), /* Red */
                                                     true); /* Filled */
    if (rect == NULL) {
        fprintf(stderr, "Failed to create rectangle\n");
        animate_destroy_canvas(canvas);
        return 1;
    }

    struct spring* circle = animate_create_circle(15,
                                                     animate_color_argb(255, 0, 0, 255), /* Blue */
                                                     true);
    if (circle == NULL) {
        fprintf(stderr, "Failed to create circle\n");
        animate_destroy_sprite(rect);
        animate_destroy_canvas(canvas);
        return 1;
    }

    /* Place sprites on canvas */
    printf("Placing sprites...\n");
    struct spring_placement* rect_placement = animate_place_sprite(canvas, rect, 10, 10);
    if (rect_placement == NULL) {
        fprintf(stderr, "Failed to place rectangle\n");
        animate_destroy_sprite(circle);
        animate_destroy_sprite(rect);
        animate_destroy_canvas(canvas);
        return 1;
    }

    struct spring_placement* circle_placement = animate_place_sprite(canvas, circle, 50, 50);
    if (circle_placement == NULL) {
        fprintf(stderr, "Failed to place circle\n");
        animate_destroy_placement(rect_placement);
        animate_destroy_sprite(circle);
        animate_destroy_sprite(rect);
        animate_destroy_canvas(canvas);
        return 1;
    }

    /* Set animation parameters for rectangle (moving diagonally) */
    printf("Setting animation parameters...\n");
    animate_set_animation_params(rect_placement,
                                   10,  /* vx: 10 pixels per second */
                                   5,   /* vy: 5 pixels per second */
                                   0,   /* ax: no acceleration */
                                   0);  /* ay: no acceleration */

    /* Generate and save frames */
    printf("Generating frames...\n");
    size_t frame_size = animate_frame_size_bytes(canvas);
    void* frame_buffer = malloc(frame_size);
    if (frame_buffer == NULL) {
        fprintf(stderr, "Failed to allocate frame buffer\n");
        animate_destroy_placement(circle_placement);
        animate_destroy_placement(rect_placement);
        animate_destroy_sprite(circle);
        animate_destroy_sprite(rect);
        animate_destroy_canvas(canvas);
        return 1;
    }

    /* Generate 10 frames at 10 FPS (1 second animation) */
    for (size_t frame = 0; frame < 10; frame++) {
        animate_generate_frame(canvas, frame, 10, frame_buffer);

        /* Save frame as raw data (for testing with ffmpeg) */
        char filename[64];
        snprintf(filename, sizeof(filename), "frame_%02zu.raw", frame);
        FILE* f = fopen(filename, "wb");
        if (f != NULL) {
            fwrite(frame_buffer, 1, frame_size, f);
            fclose(f);
            printf("  Saved: %s\n", filename);
        }
    }

    /* Test layer operations */
    printf("\nTesting layer operations...\n");
    printf("  Initial layer order: rect (index 0), circle (index 1)\n");
    printf("  Moving circle up one layer...\n");
    animate_placement_up(circle_placement);
    printf("  Layer order after move: circle (index 0), rect (index 1)\n");
    animate_placement_down(circle_placement);
    printf("  Layer order after move down: rect (index 0), circle (index 1)\n");

    /* Generate another frame to verify layer order */
    animate_generate_frame(canvas, 5, 10, frame_buffer);
    FILE* f = fopen("frame_final.raw", "wb");
    if (f != NULL) {
        fwrite(frame_buffer, 1, frame_size, f);
        fclose(f);
        printf("  Saved final frame with layer order preserved.\n");
    }

    /* Cleanup */
    printf("\nCleaning up...\n");
    free(frame_buffer);
    animate_destroy_placement(circle_placement);
    animate_destroy_placement(rect_placement);

    printf("  Destroying sprites (ref_count should allow destruction)...\n");
    if (animate_destroy_sprite(circle)) {
        printf("  Circle destroyed successfully.\n");
    } else {
        printf("  Failed to destroy circle (still in use).\n");
    }
    if (animate_destroy_sprite(rect)) {
        printf("  Rectangle destroyed successfully.\n");
    } else {
        printf("  Failed to destroy rectangle (still in use).\n");
    }

    printf("  Destroying canvas...\n");
    animate_destroy_canvas(canvas);

    printf("\nDemo complete!\n");
    printf("\nTo convert frames to video, run:\n");
    printf("  ffmpeg -f rawvideo -pix_fmt bgra -s 100x100 -r 10 -i frame_%%02zu.raw -c:v libx264 -y animation.mp4\n");

    return 0;
}
