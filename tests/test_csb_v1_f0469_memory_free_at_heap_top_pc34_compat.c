#include "csb_v1_f0469_memory_free_at_heap_top_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    CsbV1F0469MemoryHeapAccountingPc34Compat heap = {0U, 64U, 40U, 24U};
    CsbV1F0469MemoryHeapAccountingPc34Compat unchanged;
    (void)unchanged;

    assert(csb_v1_f0469_memory_free_at_heap_top_pc34_compat(&heap, 7U));
    assert(heap.available_heap_top == 48U);
    assert(heap.available_heap_byte_count == 32U);

    heap.available_heap_top = 60U;
    heap.available_heap_byte_count = 60U;
    assert(csb_v1_f0469_memory_free_at_heap_top_pc34_compat(&heap, 4U));
    assert(heap.available_heap_top == 64U);
    assert(heap.available_heap_byte_count == 64U);

    unchanged = heap;
    assert(!csb_v1_f0469_memory_free_at_heap_top_pc34_compat(&heap, 1U));
    assert(memcmp(&heap, &unchanged, sizeof(heap)) == 0);

    heap.available_heap_top = 32U;
    heap.available_heap_byte_count = 32U;
    unchanged = heap;
    assert(!csb_v1_f0469_memory_free_at_heap_top_pc34_compat(&heap, SIZE_MAX));
    assert(memcmp(&heap, &unchanged, sizeof(heap)) == 0);
    assert(strstr(csb_v1_f0469_memory_free_at_heap_top_source_evidence_pc34(),
                  "F469_rzzz_MEMORY_FreeAtHeapTop") != NULL);

    puts("PASS csb_v1_f0469_memory_free_at_heap_top_pc34_compat");
    return 0;
}
