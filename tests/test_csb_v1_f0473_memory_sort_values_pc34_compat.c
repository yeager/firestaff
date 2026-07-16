#include "redmcsb_f0473_memory_sort_values_pc34_compat.h"

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

static int check_values(
    const uint16_t *actual,
    const uint16_t *expected,
    size_t count)
{
    size_t index;

    for (index = 0U; index < count; index++) {
        CHECK(actual[index] == expected[index]);
    }
    return 0;
}

static int test_source_named_wrapper_sorts_ascending(void)
{
    uint16_t values[] = { 9u, 1u, 7u, 3u, 5u, 0u };
    const uint16_t expected[] = { 0u, 1u, 3u, 5u, 7u, 9u };

    CHECK(F0473_MEMORY_SortValues(values, 6u, 6u) == true);
    CHECK(check_values(values, expected, 6u) == 0);
    return 0;
}

static int test_duplicates_and_uint16_max_are_sorted(void)
{
    uint16_t values[] = { UINT16_MAX, 4u, 4u, 0u, 65534u, 4u };
    const uint16_t expected[] = { 0u, 4u, 4u, 4u, 65534u, UINT16_MAX };

    CHECK(F0473_MEMORY_SortValues(values, 6u, 6u) == true);
    CHECK(check_values(values, expected, 6u) == 0);
    return 0;
}

static int test_only_active_prefix_is_sorted(void)
{
    uint16_t values[] = { 6u, 2u, 5u, 99u, 77u };
    const uint16_t expected[] = { 2u, 5u, 6u, 99u, 77u };

    CHECK(F0473_MEMORY_SortValues(values, 3u, 5u) == true);
    CHECK(check_values(values, expected, 5u) == 0);
    return 0;
}

static int test_zero_count_accepts_null_pointer(void)
{
    CHECK(F0473_MEMORY_SortValues(NULL, 0u, 0u) == true);
    return 0;
}

static int test_invalid_capacity_rejects_without_mutation(void)
{
    uint16_t values[] = { 3u, 1u, 2u };
    const uint16_t before[] = { 3u, 1u, 2u };

    CHECK(F0473_MEMORY_SortValues(values, 4u, 3u) == false);
    CHECK(check_values(values, before, 3u) == 0);
    return 0;
}

static int test_null_nonzero_count_is_rejected(void)
{
    CHECK(F0473_MEMORY_SortValues(NULL, 1u, 1u) == false);
    return 0;
}

static int test_wrapper_matches_pc34_entrypoint(void)
{
    uint16_t wrapper_values[] = { 8u, 3u, 8u, 1u, 5u };
    uint16_t pc34_values[] = { 8u, 3u, 8u, 1u, 5u };

    CHECK(F0473_MEMORY_SortValues(wrapper_values, 5u, 5u) == true);
    CHECK(F0473_MEMORY_SortValues_PC34(pc34_values, 5u, 5u) == true);
    CHECK(memcmp(wrapper_values, pc34_values, sizeof(wrapper_values)) == 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_sorts_ascending() == 0);
    CHECK(test_duplicates_and_uint16_max_are_sorted() == 0);
    CHECK(test_only_active_prefix_is_sorted() == 0);
    CHECK(test_zero_count_accepts_null_pointer() == 0);
    CHECK(test_invalid_capacity_rejects_without_mutation() == 0);
    CHECK(test_null_nonzero_count_is_rejected() == 0);
    CHECK(test_wrapper_matches_pc34_entrypoint() == 0);
    return 0;
}
