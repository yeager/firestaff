#include "redmcsb_f0473_memory_sort_values_pc34_compat.h"

static void f0473_sift_down(uint16_t *values, size_t root, size_t end)
{
    while (root < end / 2U) {
        size_t child = root * 2U + 1U;

        if (child + 1U < end && values[child + 1U] >= values[child]) {
            child++;
        }
        if (values[root] >= values[child]) {
            break;
        }

        {
            uint16_t value = values[root];
            values[root] = values[child];
            values[child] = value;
        }
        root = child;
    }
}

bool F0473_MEMORY_SortValues_PC34(
    uint16_t *values,
    size_t value_count,
    size_t value_capacity)
{
    size_t index;

    if (value_count > value_capacity ||
        (value_count != 0U && values == NULL)) {
        return false;
    }

    for (index = value_count / 2U; index != 0U;) {
        index--;
        f0473_sift_down(values, index, value_count);
    }

    for (index = value_count; index > 1U;) {
        uint16_t value;

        index--;
        value = values[index];
        values[index] = values[0];
        values[0] = value;
        f0473_sift_down(values, 0U, index);
    }

    return true;
}

bool F0473_MEMORY_SortValues(
    uint16_t *values,
    size_t value_count,
    size_t value_capacity)
{
    return F0473_MEMORY_SortValues_PC34(
        values,
        value_count,
        value_capacity);
}
