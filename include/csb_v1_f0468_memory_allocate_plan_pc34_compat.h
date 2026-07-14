#ifndef CSB_V1_F0468_MEMORY_ALLOCATE_PLAN_PC34_COMPAT_H
#define CSB_V1_F0468_MEMORY_ALLOCATE_PLAN_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0468_MEMORY_BITMAP_ALLOCATION_MASK_PC34 = 0x8000U,
    CSB_V1_F0468_MEMORY_BITMAP_HEADER_WORD_COUNT_PC34 = 2U
};

typedef enum {
    CSB_V1_F0468_MEMORY_ALLOCATION_PERMANENT_PC34,
    CSB_V1_F0468_MEMORY_ALLOCATION_TEMPORARY_PC34
} CsbV1F0468MemoryAllocationKindPc34Compat;

/* All values are offsets in the caller-owned heap address space. */
typedef struct {
    size_t heap_begin;
    size_t heap_limit;
    size_t permanent_end;
    size_t temporary_top;
} CsbV1F0468MemoryHeapBoundsPc34Compat;

typedef struct {
    CsbV1F0468MemoryAllocationKindPc34Compat kind;
    unsigned int allocation_type;
    size_t requested_byte_count;
} CsbV1F0468MemoryAllocateRequestPc34Compat;

typedef struct {
    size_t allocation_begin;
    size_t allocation_end;
    size_t usable_offset;
    size_t requested_byte_count;
    size_t reserved_byte_count;
    size_t bitmap_header_byte_count;
    CsbV1F0468MemoryAllocationKindPc34Compat kind;
} CsbV1F0468MemoryAllocatePlanPc34Compat;

/*
 * Builds one F0468 allocation layout without allocating memory or changing
 * caller-owned heap bounds. Returns false if the request would overflow or
 * let the permanent and temporary regions overlap.
 */
bool csb_v1_f0468_memory_allocate_plan_pc34_compat(
    const CsbV1F0468MemoryHeapBoundsPc34Compat *bounds,
    const CsbV1F0468MemoryAllocateRequestPc34Compat *request,
    CsbV1F0468MemoryAllocatePlanPc34Compat *out_plan);

const char *csb_v1_f0468_memory_allocate_plan_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
