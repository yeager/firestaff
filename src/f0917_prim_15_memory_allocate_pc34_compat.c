#include "f0917_prim_15_memory_allocate_pc34_compat.h"

void *f0917_prim_15_memory_allocate_pc34_compat(
    const f0917_prim_15_memory_allocator_pc34_compat *allocator,
    int32_t byte_count)
{
    if (allocator == NULL || allocator->allocate == NULL || byte_count < 0) {
        return NULL;
    }

    return allocator->allocate(allocator->context, (size_t)byte_count);
}

const char *f0917_prim_15_memory_allocate_source_evidence_pc34(void)
{
    return "ReDMCSB PRIM1.C:10-16 F0917_PRIM_15_Memory_Allocate: "
           "forwards its signed-long size to the PC allocator and returns "
           "that result; PRIM.H:279 declares the PRIM-15 entry point.";
}
