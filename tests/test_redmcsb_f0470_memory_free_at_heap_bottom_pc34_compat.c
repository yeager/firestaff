#include <stdio.h>

#include "redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat.h"

static int failures;

#define CHECK(expression) do { \
    if (!(expression)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #expression, __FILE__, __LINE__); \
        ++failures; \
    } \
} while (0)

static ReDMCSBF0470MemoryHeapBoundsPc34Compat valid_bounds(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = {100U, 160U, 240U, 256U};
    return bounds;
}

int main(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = valid_bounds();
    ReDMCSBF0470MemoryHeapBoundsPc34Compat unchanged;

    CHECK(F0470_MEMORY_FreeAtHeapBottom_PC34(&bounds, 24U));
    CHECK(bounds.permanent_end == 136U);
    CHECK(bounds.temporary_top == 240U);

    CHECK(F0470_MEMORY_FreeAtHeapBottom_PC34(&bounds, 0U));
    CHECK(bounds.permanent_end == 136U);

    bounds = valid_bounds();
    CHECK(F0470_MEMORY_FreeAtHeapBottom_PC34(&bounds, 60U));
    CHECK(bounds.permanent_end == bounds.heap_begin);

    bounds = valid_bounds();
    unchanged = bounds;
    CHECK(!F0470_MEMORY_FreeAtHeapBottom_PC34(&bounds, 61U));
    CHECK(bounds.heap_begin == unchanged.heap_begin);
    CHECK(bounds.permanent_end == unchanged.permanent_end);

    bounds = valid_bounds();
    bounds.permanent_end = 241U;
    unchanged = bounds;
    CHECK(!F0470_MEMORY_FreeAtHeapBottom_PC34(&bounds, 1U));
    CHECK(bounds.permanent_end == unchanged.permanent_end);

    CHECK(!F0470_MEMORY_FreeAtHeapBottom_PC34(NULL, 1U));

    return failures != 0;
}
