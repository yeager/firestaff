#ifndef FIRESTAFF_REDMCSB_F0473_MEMORY_SORT_VALUES_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0473_MEMORY_SORT_VALUES_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/*
 * ReDMCSB MEMORY.C F0473_MEMORY_SortValues, PC34/I34E family.
 * Sorts caller-owned unsigned 16-bit values from smallest to largest.
 * value_count is the active range and value_capacity bounds that range.
 */
bool F0473_MEMORY_SortValues_PC34(
    uint16_t *values,
    size_t value_count,
    size_t value_capacity);

static inline bool F0473_MEMORY_SortValues(
    uint16_t *values,
    size_t value_count,
    size_t value_capacity)
{
    return F0473_MEMORY_SortValues_PC34(values, value_count, value_capacity);
}

#endif
