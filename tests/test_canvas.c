/**
 * @file test_canvas.c
 * @brief Tests for canvas functions
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "../include/animate.h"
#include "test_framework.h"

void test_create_canvas(void) {
    printf("\n=== Test: Create Canvas ===\n");

    struct canvas* canvas = animate_create_canvas(100, 200, animate_color_rgb(255, 0, 0));
    TEST_ASSERT_NOT_NULL(canvas);

    /* Clean up */
    animate_destroy_canvas(canvas);
}

void test_frame_size_bytes(void) {
    printf("\n=== Test: Frame Size Bytes ===\n");

    struct canvas* canvas = animate_create_canvas(100, 200, 0);
    TEST_ASSERT_NOT_NULL(canvas);

    /* Frame size should be width * height * sizeof(color_t) */
    size_t expected = 100 * 200 * sizeof(color_t);
    size_t actual = animate_frame_size_bytes(canvas);
    TEST_ASSERT_EQUAL_UINT(expected, actual);

    /* Clean up */
    animate_destroy_canvas(canvas);
}

void test_destroy_null_canvas(void) {
    printf("\n=== Test: Destroy NULL Canvas ===\n");

    /* Should not crash */
    animate_destroy_canvas(NULL);
    TEST_ASSERT_TRUE(1); /* Test passes if we reach here */
}

int main(void) {
    printf("Canvas Tests\n");
    printf("===========\n");

    reset_tests();

    test_create_canvas();
    test_frame_size_bytes();
    test_destroy_null_canvas();

    print_test_summary();

    return (tests_passed == tests_run) ? 0 : 1;
}
