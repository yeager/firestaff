#include "csb_v1_f0468_memory_allocate_pc34_compat.h"

#include <stdint.h>

bool csb_v1_f0468_memory_allocate_pc34_compat(
    CsbV1F0468MemoryStatePc34Compat *state,
    const CsbV1F0468MemoryAllocateRequestPc34Compat *request,
    CsbV1F0468MemoryAllocateResultPc34Compat *out_result)
{
    CsbV1F0468MemoryStatePc34Compat next_state;
    CsbV1F0468MemoryAllocateResultPc34Compat result;
    size_t reserved_byte_count;
    size_t header_byte_count;

    if (state == NULL || request == NULL || out_result == NULL ||
        state->heap_begin > state->heap_end ||
        state->heap_end > state->heap_top_of_temporary ||
        state->heap_top_of_temporary > state->heap_limit ||
        state->available_heap_byte_count !=
            state->heap_top_of_temporary - state->heap_end ||
        (request->allocation_type !=
             CSB_V1_F0468_MEMORY_TEMPORARY_ON_TOP_OF_HEAP_PC34 &&
         request->allocation_type != CSB_V1_F0468_MEMORY_PERMANENT_PC34 &&
         request->allocation_type !=
             CSB_V1_F0468_MEMORY_TEMPORARY_ON_BOTTOM_OF_HEAP_PC34) ||
        ((request->byte_count & 1U) != 0U && request->byte_count == SIZE_MAX)) {
        return false;
    }

    reserved_byte_count = request->byte_count + (request->byte_count & 1U);
    header_byte_count =
        (request->memory_request & CSB_V1_F0468_MEMORY_REQUEST_BITMAP_PC34) !=
                0U
            ? CSB_V1_F0468_MEMORY_BITMAP_HEADER_BYTE_COUNT_PC34
            : 0U;
    if (reserved_byte_count > SIZE_MAX - header_byte_count) {
        return false;
    }
    reserved_byte_count += header_byte_count;
    if (reserved_byte_count > state->available_heap_byte_count) {
        return false;
    }

    next_state = *state;
    result.reserved_byte_count = reserved_byte_count;
    if (request->allocation_type ==
        CSB_V1_F0468_MEMORY_TEMPORARY_ON_TOP_OF_HEAP_PC34) {
        next_state.heap_top_of_temporary -= reserved_byte_count;
        result.usable_offset =
            next_state.heap_top_of_temporary + header_byte_count;
    } else {
        result.usable_offset = next_state.heap_end + header_byte_count;
        next_state.heap_end += reserved_byte_count;
    }
    next_state.available_heap_byte_count -= reserved_byte_count;

    *state = next_state;
    *out_result = result;
    return true;
}

const char *csb_v1_f0468_memory_allocate_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:304-397 F0468_MEMORY_Allocate: rounds "
           "requests to even byte counts, reserves two int16_t words for "
           "MASK0x8000, returns the usable address after that header, and "
           "rejects HeapEnd/HeapTopOfTemporary overlap. PC 3.4 call sites "
           "use C0 temporary-top, C1 permanent, and C2 temporary-bottom; "
           "COORD.C F0640 pairs C2 with F0470_MEMORY_FreeAtHeapBottom.";
}
