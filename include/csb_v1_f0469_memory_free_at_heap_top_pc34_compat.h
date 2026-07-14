#ifndef CSB_V1_F0469_MEMORY_FREE_AT_HEAP_TOP_PC34_COMPAT_H
#define CSB_V1_F0469_MEMORY_FREE_AT_HEAP_TOP_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/* All values are offsets in the caller-owned heap address space. */
typedef struct {
    size_t heap_begin;
    size_t heap_limit;
    size_t available_heap_top;
    size_t available_heap_byte_count;
} CsbV1F0469MemoryHeapAccountingPc34Compat;

/*
 * Applies F0469's heap-top release accounting. Odd byte counts are rounded up
 * to the next even byte count. Returns false without changing the state when
 * the state is invalid, rounding would overflow, or the release exceeds the
 * bounded heap.
 */
bool csb_v1_f0469_memory_free_at_heap_top_pc34_compat(
    CsbV1F0469MemoryHeapAccountingPc34Compat *heap,
    size_t byte_count);

const char *csb_v1_f0469_memory_free_at_heap_top_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
