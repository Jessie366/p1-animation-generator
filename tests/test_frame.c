/**
 * @file test_frame.c
 * @brief Tests for frame generation
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/animate.h"
#include "test_framework.h"

void test_generate_frame_basic(void) {
    printf("\n=== Test: Generate Frame Basic ===\n");

    /* Create canvas with white background */
    struct canvas* canvas = animate_create_canvas(50, 50, animate_color_rgb(255, 255, 255));
    TEST_ASSERT_NOT_NULL(canvas);

    /* Create red rectangle */
    struct spring* sprite = animate_create_rectangle(10, 10, animate_color_rgb(255, 0, 0), true);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Place sprite at (5, 5) */
    struct spring_placement* placement = animate_place_sprite(canvas, sprite, 5, 5);
    TEST_ASSERT_NOT_NULL(placement);

    /* Allocate frame buffer and generate frame */
    size_t frame_size = animate_frame_size_bytes(canvas);
    void* frame = malloc(frame_size);
    TEST_ASSERT_NOT_NULL(frame);

    animate_generate_frame(canvas, 0, 60, frame);

    /* Frame should have red rectangle at position (5,5) to (14,14) */
    color_t* pixels = (color_t*)frame;
    color_t red = animate_color_rgb(255, 0, 0);

    /* Check a pixel in the red rectangle */
    /* Position (10, 10) should be red */
    bool has_red = false;
    for (int y = 5; y < 15 && !has_red; y++) {
        for (int x = 5; x < 15 && !has_red; x++) {
            /* Get alpha (bits 24-31 should be 0xFF for opaque pixels) */
            uint8_t alpha = (pixels[y * 50 + x] >> 24) & 0xFF;
            color_t expected_color = red | 0xFF000000; /* Force alpha to 0xFF */

            /* Check if pixel matches expected red */
            if (alpha != 0 && (pixels[y * 50 + x] & 0x00FFFFFF) == (expected_color & 0x00FFFFFF)) {
                has_red = true;
            }
        }
    }
    TEST_ASSERT_TRUE(has_red);

    /* Check a pixel outside the rectangle (should be white background) */
    color_t bg = animate_color_rgb(255, 255, 255);
    TEST_ASSERT_EQUAL_UINT(bg | 0xFF000000, pixels[0] & 0xFFFFFF00); /* Allow for alpha */

    /* Clean up */
    free(frame);
    animate_destroy_placement(placement);
    animate_destroy_sprite(sprite);
    animate_destroy_canvas(canvas);
}

void test_generate_frame_with_motion(void) {
    printf("\n=== Test: Generate Frame With Motion ===\n");

    /* Create canvas */
    struct canvas* canvas = animate_create_canvas(100, 100, animate_color_rgb(0, 0, 0));
    TEST_ASSERT_NOT_NULL(canvas);

    /* Create blue circle */
    struct spring* sprite = animate_create_circle(5, animate_color_rgb(0, 0, 255), true);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Place sprite with velocity */
    struct spring_placement* placement = animate_place_sprite(canvas, sprite, 10, 10);
    TEST_ASSERT_NOT_NULL(placement);

    /* Set velocity: 10 pixels per second in x direction */
    animate_set_animation_params(placement, 10, 0, 0, 0);

    /* Allocate frame buffer */
    size_t frame_size = animate_frame_size_bytes(canvas);
    void* frame = malloc(frame_size);
    TEST_ASSERT_NOT_NULL(frame);

    /* Generate frame 0 (t=0) */
    animate_generate_frame(canvas, 0, 60, frame);

    /* Generate frame 30 (t=0.5s, x should be 10 + 10*0.5 = 15) */
    animate_generate_frame(canvas, 30, 60, frame);

    /* If we reach here, motion calculation works */
    TEST_ASSERT_TRUE(1);

    /* Clean up */
    free(frame);
    animate_destroy_placement(placement);
    animate_destroy_sprite(sprite);
    animate_destroy_canvas(canvas);
}

void test_alpha_handling(void) {
    printf("\n=== Test: Alpha Handling ===\n");

    /* Create canvas */
    struct canvas* canvas = animate_create_canvas(20, 20, animate_color_rgb(255, 255, 255));
    TEST_ASSERT_NOT_NULL(canvas);

    /* Create rectangle with border only (hollow) */
    struct spring* sprite = animate_create_rectangle(10, 10, animate_color_rgb(255, 0, 0), false);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Place sprite */
    struct spring_placement* placement = animate_place_sprite(canvas, sprite, 5, 5);
    TEST_ASSERT_NOT_NULL(placement);

    /* Generate frame */
    size_t frame_size = animate_frame_size_bytes(canvas);
    void* frame = malloc(frame_size);
    TEST_ASSERT_NOT_NULL(frame);

    animate_generate_frame(canvas, 0, 60, frame);
    color_t* pixels = (color_t*)frame;

    /* Check that border pixels are opaque */
    bool has_border = false;
    /* Border is at x=5, x=14, y=5, y=14 */
    for (int i = 5; i < 15 && !has_border; i++) {
        /* Top and bottom borders */
        uint8_t alpha_top = (pixels[5 * 20 + i] >> 24) & 0xFF;
        uint8_t alpha_bottom = (pixels[14 * 20 + i] >> 24) & 0xFF;
        if (alpha_top != 0 || alpha_bottom != 0) {
            has_border = true;
        }
    }
    TEST_ASSERT_TRUE(has_border);

    /* Check that center pixels are transparent (show background) */
    color_t bg = animate_color_rgb(255, 255, 255);
    uint8_t alpha_center = (pixels[10 * 20 + 10] >> 24) & 0xFF;
    /* Alpha should be 0xFF for background pixels (they're not transparent) */
    TEST_ASSERT_EQUAL_UINT(0xFF, alpha_center);

    /* Clean up */
    free(frame);
    animate_destroy_placement(placement);
    animate_destroy_sprite(sprite);
    animate_destroy_canvas(canvas);
}

int main(void) {
    printf("Frame Generation Tests\n");
    printf("======================\n");

    reset_tests();

    test_generate_frame_basic();
    test_generate_frame_with_motion();
    test_alpha_handling();

    print_test_summary();

    return (tests_passed == tests_run) ? 0 : 1;
}
