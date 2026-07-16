#include "csb_v1_f0469_memory_free_at_heap_top_pc34_compat.h"

#include <stdint.h>

bool csb_v1_f0469_memory_free_at_heap_top_pc34_compat(
    CsbV1F0469MemoryHeapAccountingPc34Compat *heap,
    size_t byte_count)
{
    size_t released_byte_count;
    size_t heap_byte_count;

    if (heap == NULL || heap->heap_begin > heap->available_heap_top ||
        heap->available_heap_top > heap->heap_limit) {
        return false;
    }

    heap_byte_count = heap->heap_limit - heap->heap_begin;
    if (heap->available_heap_byte_count > heap_byte_count ||
        ((byte_count & 1U) != 0U && byte_count == SIZE_MAX)) {
        return false;
    }
    released_byte_count = byte_count + (byte_count & 1U);

    if (released_byte_count > heap->heap_limit - heap->available_heap_top ||
        released_byte_count > heap_byte_count - heap->available_heap_byte_count) {
        return false;
    }

    heap->available_heap_top += released_byte_count;
    heap->available_heap_byte_count += released_byte_count;
    return true;
}

bool F0469_MEMORY_FreeAtHeapTop(
    CsbV1F0469MemoryHeapAccountingPc34Compat *heap,
    size_t byte_count)
{
    return csb_v1_f0469_memory_free_at_heap_top_pc34_compat(heap, byte_count);
}

const char *csb_v1_f0469_memory_free_at_heap_top_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:100-108 F469_rzzz_MEMORY_FreeAtHeapTop: "
           "odd byte counts are incremented before both "
           "AvailableHeapMemoryByteCount and AvailableHeapMemoryTop are "
           "increased.";
}
