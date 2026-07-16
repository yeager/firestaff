#include "redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static ReDMCSBF0470MemoryHeapBoundsPc34Compat make_bounds(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = {
        100u,
        140u,
        180u,
        220u,
        32u
    };
    return bounds;
}

static int test_source_named_wrapper_rounds_odd_byte_counts(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = make_bounds();

    CHECK(F0470_MEMORY_FreeAtHeapBottom(&bounds, 5u) == true);
    CHECK(bounds.available_heap_byte_count == 38u);
    CHECK(bounds.heap_begin == 100u);
    CHECK(bounds.permanent_end == 140u);
    CHECK(bounds.temporary_top == 180u);
    CHECK(bounds.heap_limit == 220u);
    return 0;
}

static int test_even_byte_counts_are_released_exactly(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = make_bounds();

    CHECK(F0470_MEMORY_FreeAtHeapBottom(&bounds, 8u) == true);
    CHECK(bounds.available_heap_byte_count == 40u);
    CHECK(bounds.heap_begin == 100u);
    CHECK(bounds.permanent_end == 140u);
    CHECK(bounds.temporary_top == 180u);
    CHECK(bounds.heap_limit == 220u);
    return 0;
}

static int test_invalid_state_does_not_mutate(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = make_bounds();
    ReDMCSBF0470MemoryHeapBoundsPc34Compat before;

    bounds.permanent_end = 230u;
    before = bounds;

    CHECK(F0470_MEMORY_FreeAtHeapBottom(&bounds, 2u) == false);
    CHECK(memcmp(&bounds, &before, sizeof(bounds)) == 0);
    return 0;
}

static int test_release_beyond_heap_capacity_is_rejected(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat bounds = make_bounds();
    ReDMCSBF0470MemoryHeapBoundsPc34Compat before;

    bounds.available_heap_byte_count = 118u;
    before = bounds;

    CHECK(F0470_MEMORY_FreeAtHeapBottom(&bounds, 3u) == false);
    CHECK(memcmp(&bounds, &before, sizeof(bounds)) == 0);
    return 0;
}

static int test_null_bounds_are_rejected(void)
{
    CHECK(F0470_MEMORY_FreeAtHeapBottom(NULL, 2u) == false);
    return 0;
}

static int test_wrapper_matches_pc34_entrypoint(void)
{
    ReDMCSBF0470MemoryHeapBoundsPc34Compat wrapper_bounds = make_bounds();
    ReDMCSBF0470MemoryHeapBoundsPc34Compat pc34_bounds = make_bounds();

    CHECK(F0470_MEMORY_FreeAtHeapBottom(&wrapper_bounds, 7u) == true);
    CHECK(F0470_MEMORY_FreeAtHeapBottom_PC34(&pc34_bounds, 7u) == true);
    CHECK(memcmp(&wrapper_bounds, &pc34_bounds, sizeof(wrapper_bounds)) == 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_rounds_odd_byte_counts() == 0);
    CHECK(test_even_byte_counts_are_released_exactly() == 0);
    CHECK(test_invalid_state_does_not_mutate() == 0);
    CHECK(test_release_beyond_heap_capacity_is_rejected() == 0);
    CHECK(test_null_bounds_are_rejected() == 0);
    CHECK(test_wrapper_matches_pc34_entrypoint() == 0);
    return 0;
}
