#include "theron_v1_track02_experience_table.h"
#include <assert.h>
#include <stdio.h>

static void test_count(void)
{
    assert(THERON_TRACK02_EXPERIENCE_ENTRY_COUNT == 64);
    printf("  PASS: count\n");
}

static void test_first_entries(void)
{
    assert(theron_v1_track02_us_experience_threshold(0) == 0);
    assert(theron_v1_track02_us_experience_threshold(1) == 3);
    assert(theron_v1_track02_us_experience_threshold(2) == 8);
    assert(theron_v1_track02_us_experience_threshold(3) == 11);
    printf("  PASS: first_entries\n");
}

static void test_last_entry(void)
{
    assert(theron_v1_track02_us_experience_threshold(63) == 214);
    printf("  PASS: last_entry\n");
}

static void test_monotonic(void)
{
    for (unsigned int i = 1; i < THERON_TRACK02_EXPERIENCE_ENTRY_COUNT; i++) {
        unsigned int prev = theron_v1_track02_us_experience_threshold(i - 1);
        unsigned int curr = theron_v1_track02_us_experience_threshold(i);
        assert(curr >= prev);
    }
    printf("  PASS: monotonic\n");
}

static void test_bounds(void)
{
    assert(theron_v1_track02_us_experience_threshold(64) == 0);
    assert(theron_v1_track02_us_experience_threshold(255) == 0);
    printf("  PASS: bounds\n");
}

int main(void)
{
    printf("test_theron_v1_track02_experience_table:\n");
    test_count();
    test_first_entries();
    test_last_entry();
    test_monotonic();
    test_bounds();
    printf("All tests passed.\n");
    return 0;
}
