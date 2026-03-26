/**
 * @file test_sprite.c
 * @brief Tests for sprite functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/animate.h"
#include "test_framework.h"

void test_create_rectangle_filled(void) {
    printf("\n=== Test: Create Filled Rectangle ===\n");

    color_t color = animate_color_rgb(255, 0, 0); /* Red */
    struct spring* sprite = animate_create_rectangle(10, 10, color, true);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Check dimensions */
    /* Note: We can't directly access internal fields, so we test behavior */

    /* Clean up */
    animate_destroy_sprite(sprite);
}

void test_create_rectangle_border(void) {
    printf("\n=== Test: Create Border Rectangle ===\n");

    color_t color = animate_color_rgb(0, 255, 0); /* Green */
    struct spring* sprite = animate_create_rectangle(5, 5, color, false);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Clean up */
    animate_destroy_sprite(sprite);
}

void test_create_circle(void) {
    printf("\n=== Test: Create Circle ===\n");

    color_t color = animate_color_rgb(0, 0, 255); /* Blue */
    struct spring* sprite = animate_create_circle(5, color, true);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Clean up */
    animate_destroy_sprite(sprite);
}

void test_destroy_sprite_with_refcount(void) {
    printf("\n=== Test: Destroy Sprite with Placement ===\n");

    struct canvas* canvas = animate_create_canvas(100, 100, 0);
    TEST_ASSERT_NOT_NULL(canvas);

    struct spring* sprite = animate_create_rectangle(10, 10, animate_color_rgb(255, 0, 0), true);
    TEST_ASSERT_NOT_NULL(sprite);

    /* Place sprite on canvas (increments ref_count) */
    struct spring_placement* placement = animate_place_sprite(canvas, sprite, 0, 0);
    TEST_ASSERT_NOT_NULL(placement);

    /* Try to destroy sprite while it's in use - should fail */
    bool result = animate_destroy_sprite(sprite);
    TEST_ASSERT_FALSE(result);

    /* Destroy placement first */
    animate_destroy_placement(placement);

    /* Now sprite should be destroyable */
    result = animate_destroy_sprite(sprite);
    TEST_ASSERT_TRUE(result);

    /* Clean up */
    animate_destroy_canvas(canvas);
}

void test_null_sprite_creation(void) {
    printf("\n=== Test: NULL Sprite Creation ===\n");

    /* Creating sprite from NULL file should return NULL */
    struct spring* sprite = animate_create_sprite(NULL);
    TEST_ASSERT_NULL(sprite);

    /* This test always passes if we reach here without crashing */
}

int main(void) {
    printf("Sprite Tests\n");
    printf("=============\n");

    reset_tests();

    test_create_rectangle_filled();
    test_create_rectangle_border();
    test_create_circle();
    test_destroy_sprite_with_refcount();
    test_null_sprite_creation();

    print_test_summary();

    return (tests_passed == tests_run) ? 0 : 1;
}
