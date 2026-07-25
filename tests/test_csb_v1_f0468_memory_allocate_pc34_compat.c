#include "csb_v1_f0468_memory_allocate_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    CsbV1F0468MemoryStatePc34Compat state = {0U, 64U, 16U, 48U, 32U};
    CsbV1F0468MemoryAllocateRequestPc34Compat request;
    CsbV1F0468MemoryAllocateResultPc34Compat result;
    CsbV1F0468MemoryStatePc34Compat unchanged_state;
    (void)unchanged_state;
    CsbV1F0468MemoryAllocateResultPc34Compat unchanged_result;
    (void)unchanged_result;

    request.byte_count = 5U;
    request.allocation_type = CSB_V1_F0468_MEMORY_PERMANENT_PC34;
    request.memory_request = 0U;
    assert(csb_v1_f0468_memory_allocate_pc34_compat(&state, &request, &result));
    assert(result.usable_offset == 16U);
    assert(result.reserved_byte_count == 6U);
    assert(state.heap_end == 22U);
    assert(state.heap_top_of_temporary == 48U);
    assert(state.available_heap_byte_count == 26U);

    request.byte_count = 7U;
    request.allocation_type =
        CSB_V1_F0468_MEMORY_TEMPORARY_ON_TOP_OF_HEAP_PC34;
    request.memory_request = CSB_V1_F0468_MEMORY_REQUEST_BITMAP_PC34;
    assert(csb_v1_f0468_memory_allocate_pc34_compat(&state, &request, &result));
    assert(result.usable_offset == 40U);
    assert(result.reserved_byte_count == 12U);
    assert(state.heap_end == 22U);
    assert(state.heap_top_of_temporary == 36U);
    assert(state.available_heap_byte_count == 14U);

    request.byte_count = 8U;
    request.allocation_type =
        CSB_V1_F0468_MEMORY_TEMPORARY_ON_BOTTOM_OF_HEAP_PC34;
    request.memory_request = 0U;
    assert(csb_v1_f0468_memory_allocate_pc34_compat(&state, &request, &result));
    assert(result.usable_offset == 22U);
    assert(result.reserved_byte_count == 8U);
    assert(state.heap_end == 30U);
    assert(state.heap_top_of_temporary == 36U);
    assert(state.available_heap_byte_count == 6U);

    unchanged_state = state;
    memset(&result, 0xA5, sizeof(result));
    unchanged_result = result;
    request.byte_count = 8U;
    assert(!csb_v1_f0468_memory_allocate_pc34_compat(&state, &request, &result));
    assert(memcmp(&state, &unchanged_state, sizeof(state)) == 0);
    assert(memcmp(&result, &unchanged_result, sizeof(result)) == 0);

    request.byte_count = 1U;
    request.allocation_type = (CsbV1F0468MemoryAllocationTypePc34Compat)3;
    assert(!csb_v1_f0468_memory_allocate_pc34_compat(&state, &request, &result));
    assert(memcmp(&state, &unchanged_state, sizeof(state)) == 0);
    assert(memcmp(&result, &unchanged_result, sizeof(result)) == 0);

    request.byte_count = SIZE_MAX;
    assert(!csb_v1_f0468_memory_allocate_pc34_compat(&state, &request, &result));
    assert(memcmp(&state, &unchanged_state, sizeof(state)) == 0);
    assert(memcmp(&result, &unchanged_result, sizeof(result)) == 0);

    assert(strstr(csb_v1_f0468_memory_allocate_source_evidence_pc34(),
                  "F0468_MEMORY_Allocate") != NULL);
    puts("PASS csb_v1_f0468_memory_allocate_pc34_compat");
    return 0;
}
