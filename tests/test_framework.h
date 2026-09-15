#ifndef TEST_FRAMEWORK_H
#define TEST_FRAMEWORK_H

#include <stdio.h>

static int tests_run = 0;
static int tests_passed = 0;

#define TEST_ASSERT_TRUE(expr)                                                   \
    do {                                                                         \
        tests_run++;                                                             \
        if (expr) {                                                              \
            tests_passed++;                                                      \
            printf("  PASS: %s\n", #expr);                                      \
        } else {                                                                 \
            printf("  FAIL: %s (%s:%d)\n", #expr, __FILE__, __LINE__);          \
        }                                                                        \
    } while (0)

#define TEST_ASSERT_FALSE(expr) TEST_ASSERT_TRUE(!(expr))
#define TEST_ASSERT_NULL(ptr) TEST_ASSERT_TRUE((ptr) == NULL)
#define TEST_ASSERT_NOT_NULL(ptr) TEST_ASSERT_TRUE((ptr) != NULL)

#define TEST_ASSERT_EQUAL_UINT(expected, actual)                                  \
    do {                                                                         \
        unsigned long long test_expected_ = (unsigned long long)(expected);       \
        unsigned long long test_actual_ = (unsigned long long)(actual);           \
        tests_run++;                                                             \
        if (test_expected_ == test_actual_) {                                     \
            tests_passed++;                                                      \
            printf("  PASS: %s == %s\n", #expected, #actual);                   \
        } else {                                                                 \
            printf("  FAIL: %s == %s (%s:%d): expected %llu, got %llu\n",        \
                   #expected, #actual, __FILE__, __LINE__,                       \
                   test_expected_, test_actual_);                                \
        }                                                                        \
    } while (0)

static inline void reset_tests(void) {
    tests_run = 0;
    tests_passed = 0;
}

static inline void print_test_summary(void) {
    printf("\n%d/%d tests passed\n", tests_passed, tests_run);
}

#endif
