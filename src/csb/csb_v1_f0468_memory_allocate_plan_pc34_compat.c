#include "csb_v1_f0468_memory_allocate_plan_pc34_compat.h"

#include <stdint.h>

bool csb_v1_f0468_memory_allocate_plan_pc34_compat(
    const CsbV1F0468MemoryHeapBoundsPc34Compat *bounds,
    const CsbV1F0468MemoryAllocateRequestPc34Compat *request,
    CsbV1F0468MemoryAllocatePlanPc34Compat *out_plan)
{
    CsbV1F0468MemoryAllocatePlanPc34Compat plan;
    size_t header_byte_count;
    size_t reserved_byte_count;

    if (bounds == NULL || request == NULL || out_plan == NULL ||
        bounds->heap_begin > bounds->permanent_end ||
        bounds->permanent_end > bounds->temporary_top ||
        bounds->temporary_top > bounds->heap_limit ||
        (request->kind != CSB_V1_F0468_MEMORY_ALLOCATION_PERMANENT_PC34 &&
         request->kind != CSB_V1_F0468_MEMORY_ALLOCATION_TEMPORARY_PC34)) {
        return false;
    }

    header_byte_count =
        (request->allocation_type & CSB_V1_F0468_MEMORY_BITMAP_ALLOCATION_MASK_PC34) != 0U
            ? CSB_V1_F0468_MEMORY_BITMAP_HEADER_WORD_COUNT_PC34 * sizeof(int16_t)
            : 0U;
    if (request->requested_byte_count > SIZE_MAX - header_byte_count) {
        return false;
    }
    reserved_byte_count = request->requested_byte_count + header_byte_count;

    plan.requested_byte_count = request->requested_byte_count;
    plan.reserved_byte_count = reserved_byte_count;
    plan.bitmap_header_byte_count = header_byte_count;
    plan.kind = request->kind;
    if (request->kind == CSB_V1_F0468_MEMORY_ALLOCATION_PERMANENT_PC34) {
        if (reserved_byte_count > bounds->temporary_top - bounds->permanent_end) {
            return false;
        }
        plan.allocation_begin = bounds->permanent_end;
        plan.allocation_end = bounds->permanent_end + reserved_byte_count;
    } else {
        if (reserved_byte_count > bounds->temporary_top - bounds->permanent_end) {
            return false;
        }
        plan.allocation_begin = bounds->temporary_top - reserved_byte_count;
        plan.allocation_end = bounds->temporary_top;
    }
    plan.usable_offset = plan.allocation_begin + header_byte_count;
    *out_plan = plan;
    return true;
}

const char *csb_v1_f0468_memory_allocate_plan_source_evidence_pc34(void)
{
    return "ReDMCSB MEMORY.C:304-397 F0468_MEMORY_Allocate: permanent "
           "allocations advance HeapEnd, temporary allocations retreat "
           "HeapTopOfTemporary, and overlap is rejected at line 358; "
           "MEMORY.C:339 reserves two int16_t words when MASK0x8000 is set. "
           "ACTIDRAW.C:240 calls F0468_MEMORY_Allocate.";
}
