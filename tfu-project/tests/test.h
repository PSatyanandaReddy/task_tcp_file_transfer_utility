#ifndef TFU_TEST_H
#define TFU_TEST_H

#include <cstdio>
#include <cstring>
#include <string>

static int g_pass = 0, g_fail = 0;

#define ASSERT_TRUE(expr) do { \
    if (expr) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "  FAIL %s:%d: %s\n", __FILE__, __LINE__, #expr); } \
} while(0)

#define ASSERT_FALSE(expr) ASSERT_TRUE(!(expr))

#define ASSERT_EQ(a, b) do { \
    if ((a) == (b)) { g_pass++; } \
    else { g_fail++; fprintf(stderr, "  FAIL %s:%d: %s != %s\n", __FILE__, __LINE__, #a, #b); } \
} while(0)

#define RUN_TEST(fn) do { \
    printf("  %s ... ", #fn); fn(); printf("ok\n"); \
} while(0)

#define TEST_REPORT() do { \
    printf("\n%d passed, %d failed\n", g_pass, g_fail); \
    return g_fail > 0 ? 1 : 0; \
} while(0)

#endif
