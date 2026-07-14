#ifndef FIRESTAFF_REDMCSB_F0470_MEMORY_FREE_AT_HEAP_BOTTOM_PC34_COMPAT_H
#define FIRESTAFF_REDMCSB_F0470_MEMORY_FREE_AT_HEAP_BOTTOM_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

/*
 * ReDMCSB MEMORY.C F0470_MEMORY_FreeAtHeapBottom, PC34/I34E family.
 * The permanent heap grows upward from heap_begin; this bounded adapter
 * accounts for a permanent release by moving permanent_end downward.
 * All members are offsets in the caller-owned heap address space.
 */
typedef struct {
    size_t heap_begin;
    size_t permanent_end;
    size_t temporary_top;
    size_t heap_limit;
} ReDMCSBF0470MemoryHeapBoundsPc34Compat;

/*
 * Applies the original permanent-heap boundary adjustment. Returns false
 * without changing bounds when the heap state is invalid or byte_count
 * exceeds the accounted permanent allocation.
 */
bool F0470_MEMORY_FreeAtHeapBottom_PC34(
    ReDMCSBF0470MemoryHeapBoundsPc34Compat *bounds,
    size_t byte_count);

#endif
