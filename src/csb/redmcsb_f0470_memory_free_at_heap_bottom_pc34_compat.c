#include "redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat.h"

#include <stdint.h>

bool F0470_MEMORY_FreeAtHeapBottom_PC34(
    ReDMCSBF0470MemoryHeapBoundsPc34Compat *bounds,
    size_t byte_count)
{
    size_t released_byte_count;
    size_t heap_byte_count;

    if (bounds == NULL || bounds->heap_begin > bounds->permanent_end ||
        bounds->permanent_end > bounds->temporary_top ||
        bounds->temporary_top > bounds->heap_limit) {
        return false;
    }

    heap_byte_count = bounds->heap_limit - bounds->heap_begin;
    if (bounds->available_heap_byte_count > heap_byte_count ||
        ((byte_count & 1U) != 0U && byte_count == SIZE_MAX)) {
        return false;
    }
    released_byte_count = byte_count + (byte_count & 1U);
    if (released_byte_count >
        heap_byte_count - bounds->available_heap_byte_count) {
        return false;
    }

    bounds->available_heap_byte_count += released_byte_count;
    return true;
}

bool F0470_MEMORY_FreeAtHeapBottom(
    ReDMCSBF0470MemoryHeapBoundsPc34Compat *bounds,
    size_t byte_count)
{
    return F0470_MEMORY_FreeAtHeapBottom_PC34(bounds, byte_count);
}
