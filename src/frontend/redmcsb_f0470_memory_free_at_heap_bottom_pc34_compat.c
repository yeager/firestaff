#include "redmcsb_f0470_memory_free_at_heap_bottom_pc34_compat.h"

bool F0470_MEMORY_FreeAtHeapBottom_PC34(
    ReDMCSBF0470MemoryHeapBoundsPc34Compat *bounds,
    size_t byte_count)
{
    if (bounds == NULL ||
        bounds->heap_begin > bounds->permanent_end ||
        bounds->permanent_end > bounds->temporary_top ||
        bounds->temporary_top > bounds->heap_limit ||
        byte_count > bounds->permanent_end - bounds->heap_begin) {
        return false;
    }

    /* MEMORY.C:413-425 decrements G0461_puc_HeapEnd by byte_count. */
    bounds->permanent_end -= byte_count;
    return true;
}
