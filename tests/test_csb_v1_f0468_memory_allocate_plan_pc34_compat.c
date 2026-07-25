#include "csb_v1_f0468_memory_allocate_plan_pc34_compat.h"

#include <assert.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

int main(void)
{
    const CsbV1F0468MemoryHeapBoundsPc34Compat bounds = {0U, 64U, 16U, 48U};
    (void)bounds;
    CsbV1F0468MemoryAllocateRequestPc34Compat request;
    CsbV1F0468MemoryAllocatePlanPc34Compat plan;
    CsbV1F0468MemoryAllocatePlanPc34Compat unchanged;
    (void)unchanged;

    request.kind = CSB_V1_F0468_MEMORY_ALLOCATION_PERMANENT_PC34;
    request.allocation_type = 0U;
    request.requested_byte_count = 12U;
    assert(csb_v1_f0468_memory_allocate_plan_pc34_compat(
        &bounds, &request, &plan));
    assert(plan.allocation_begin == 16U);
    assert(plan.allocation_end == 28U);
    assert(plan.usable_offset == 16U);
    assert(plan.reserved_byte_count == 12U);
    assert(plan.bitmap_header_byte_count == 0U);

    request.kind = CSB_V1_F0468_MEMORY_ALLOCATION_TEMPORARY_PC34;
    request.allocation_type = CSB_V1_F0468_MEMORY_BITMAP_ALLOCATION_MASK_PC34;
    request.requested_byte_count = 8U;
    assert(csb_v1_f0468_memory_allocate_plan_pc34_compat(
        &bounds, &request, &plan));
    assert(plan.allocation_begin == 36U);
    assert(plan.allocation_end == 48U);
    assert(plan.usable_offset == 36U + 2U * sizeof(int16_t));
    assert(plan.reserved_byte_count == 8U + 2U * sizeof(int16_t));
    assert(plan.bitmap_header_byte_count == 2U * sizeof(int16_t));

    memset(&plan, 0xA5, sizeof(plan));
    unchanged = plan;
    request.kind = CSB_V1_F0468_MEMORY_ALLOCATION_PERMANENT_PC34;
    request.allocation_type = 0U;
    request.requested_byte_count = 33U;
    assert(!csb_v1_f0468_memory_allocate_plan_pc34_compat(
        &bounds, &request, &plan));
    assert(memcmp(&plan, &unchanged, sizeof(plan)) == 0);

    request.requested_byte_count = SIZE_MAX;
    request.allocation_type = CSB_V1_F0468_MEMORY_BITMAP_ALLOCATION_MASK_PC34;
    assert(!csb_v1_f0468_memory_allocate_plan_pc34_compat(
        &bounds, &request, &plan));
    assert(memcmp(&plan, &unchanged, sizeof(plan)) == 0);
    assert(strstr(csb_v1_f0468_memory_allocate_plan_source_evidence_pc34(),
                  "F0468_MEMORY_Allocate") != NULL);

    puts("PASS csb_v1_f0468_memory_allocate_plan_pc34_compat");
    return 0;
}
