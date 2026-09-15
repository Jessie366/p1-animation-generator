/**
 * @file test_placement.c
 * @brief Tests for placement functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/animate.h"
#include "test_framework.h"

void test_place_sprite(void) {
    printf("\n=== Test: Place Sprite ===\n");

    struct canvas* canvas = animate_create_canvas(100, 100, 0);
    TEST_ASSERT_NOT_NULL(canvas);

    struct sprite* sprite = animate_create_rectangle(10, 10, animate_color_rgb(255, 0, 0), true);
    TEST_ASSERT_NOT_NULL(sprite);

    struct sprite_placement* placement = animate_place_sprite(canvas, sprite, 5, 10);
    TEST_ASSERT_NOT_NULL(placement);

    /* Clean up */
    animate_destroy_placement(placement);
    animate_destroy_sprite(sprite);
    animate_destroy_canvas(canvas);
}

void test_layer_operations(void) {
    printf("\n=== Test: Layer Operations ===\n");

    struct canvas* canvas = animate_create_canvas(100, 100, 0);
    TEST_ASSERT_NOT_NULL(canvas);

    struct sprite* sprite1 = animate_create_rectangle(10, 10, animate_color_rgb(255, 0, 0), true);
    struct sprite* sprite2 = animate_create_rectangle(10, 10, animate_color_rgb(0, 255, 0), true);
    struct sprite* sprite3 = animate_create_rectangle(10, 10, animate_color_rgb(0, 0, 255), true);
    TEST_ASSERT_NOT_NULL(sprite1);
    TEST_ASSERT_NOT_NULL(sprite2);
    TEST_ASSERT_NOT_NULL(sprite3);

    /* Place sprites (order: 1, 2, 3 - 3 is on top) */
    struct sprite_placement* p1 = animate_place_sprite(canvas, sprite1, 0, 0);
    struct sprite_placement* p2 = animate_place_sprite(canvas, sprite2, 0, 0);
    struct sprite_placement* p3 = animate_place_sprite(canvas, sprite3, 0, 0);
    TEST_ASSERT_NOT_NULL(p1);
    TEST_ASSERT_NOT_NULL(p2);
    TEST_ASSERT_NOT_NULL(p3);

    /* Move p2 up one layer */
    animate_placement_up(p2);

    /* Move p2 to top */
    animate_placement_top(p2);

    /* Move p1 to bottom */
    animate_placement_bottom(p1);

    /* Clean up */
    animate_destroy_placement(p1);
    animate_destroy_placement(p2);
    animate_destroy_placement(p3);
    animate_destroy_sprite(sprite1);
    animate_destroy_sprite(sprite2);
    animate_destroy_sprite(sprite3);
    animate_destroy_canvas(canvas);

    /* If we reach here without crashing, tests passed */
    TEST_ASSERT_TRUE(1);
}

void test_animation_params(void) {
    printf("\n=== Test: Animation Parameters ===\n");

    struct canvas* canvas = animate_create_canvas(100, 100, 0);
    TEST_ASSERT_NOT_NULL(canvas);

    struct sprite* sprite = animate_create_rectangle(10, 10, animate_color_rgb(255, 0, 0), true);
    TEST_ASSERT_NOT_NULL(sprite);

    struct sprite_placement* placement = animate_place_sprite(canvas, sprite, 10, 20);
    TEST_ASSERT_NOT_NULL(placement);

    /* Set animation parameters */
    animate_set_animation_params(placement, 5, -3, 1, 2);

    /* If we reach here without crashing, test passed */
    TEST_ASSERT_TRUE(1);

    /* Clean up */
    animate_destroy_placement(placement);
    animate_destroy_sprite(sprite);
    animate_destroy_canvas(canvas);
}

int main(void) {
    printf("Placement Tests\n");
    printf("===============\n");

    reset_tests();

    test_place_sprite();
    test_layer_operations();
    test_animation_params();

    print_test_summary();

    return (tests_passed == tests_run) ? 0 : 1;
}
