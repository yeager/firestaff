#ifndef FIRESTAFF_REDMCSB_F0470_MEMORY_FREE_AT_HEAP_BOTTOM_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0470_MEMORY_FREE_AT_HEAP_BOTTOM_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

/*
 * ReDMCSB MEMORY.C F0470_MEMORY_FreeAtHeapBottom, PC34/I34E family.
 * This adapter records the released capacity only.  It deliberately does
 * not move either permanent or temporary allocation boundary.
 * All members are offsets in the caller-owned heap address space.
 */
typedef struct {
    size_t heap_begin;
    size_t permanent_end;
    size_t temporary_top;
    size_t heap_limit;
    size_t available_heap_byte_count;
} ReDMCSBF0470MemoryHeapBoundsPc34Compat;

/*
 * Increases available_heap_byte_count by byte_count rounded up to an even
 * value. Returns false without changing bounds when the heap state is
 * invalid, rounding would overflow, or the increased count exceeds the
 * bounded heap capacity. Allocation boundary members are never modified.
 */
bool F0470_MEMORY_FreeAtHeapBottom_PC34(
    ReDMCSBF0470MemoryHeapBoundsPc34Compat *bounds,
    size_t byte_count);

#endif
