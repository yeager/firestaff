#include "redmcsb_f0473_memory_sort_values_pc34_compat.h"

#include <stdio.h>

static int check(int condition, const char *label)
{
    if (condition) {
        return 1;
    }
    fprintf(stderr, "FAIL: %s\n", label);
    return 0;
}

static int equal_values(const uint16_t *actual, const uint16_t *expected,
                        size_t count)
{
    size_t index;

    for (index = 0U; index < count; index++) {
        if (actual[index] != expected[index]) {
            return 0;
        }
    }
    return 1;
}

int main(void)
{
    uint16_t values[] = { 5U, 65535U, 3U, 5U, 0U, 32768U, 1U };
    const uint16_t expected[] = { 0U, 1U, 3U, 5U, 5U, 32768U, 65535U };
    uint16_t rejected[] = { 9U, 4U, 7U };
    const uint16_t rejected_before[] = { 9U, 4U, 7U };
    int ok = 1;

    ok &= check(F0473_MEMORY_SortValues_PC34(
                    values, sizeof(values) / sizeof(values[0]),
                    sizeof(values) / sizeof(values[0])),
                "sort succeeds within the supplied bound");
    ok &= check(equal_values(values, expected,
                             sizeof(values) / sizeof(values[0])),
                "sorts ascending as unsigned 16-bit values");

    ok &= check(F0473_MEMORY_SortValues_PC34(NULL, 0U, 0U),
                "empty range is a valid no-op");
    ok &= check(!F0473_MEMORY_SortValues_PC34(NULL, 1U, 1U),
                "nonempty null range rejects");

    ok &= check(!F0473_MEMORY_SortValues_PC34(rejected, 3U, 2U),
                "range beyond capacity rejects");
    ok &= check(equal_values(rejected, rejected_before,
                             sizeof(rejected) / sizeof(rejected[0])),
                "rejection preserves caller values");

    if (!ok) {
        return 1;
    }
    puts("PASS redmcsb_f0473_memory_sort_values_pc34_compat");
    return 0;
}
