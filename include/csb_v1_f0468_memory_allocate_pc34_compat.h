#ifndef CSB_V1_F0468_MEMORY_ALLOCATE_PC34_COMPAT_H
#define CSB_V1_F0468_MEMORY_ALLOCATE_PC34_COMPAT_H

#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

#ifdef __cplusplus
extern "C" {
#endif

enum {
    CSB_V1_F0468_MEMORY_REQUEST_BITMAP_PC34 = 0x8000U,
    CSB_V1_F0468_MEMORY_BITMAP_HEADER_BYTE_COUNT_PC34 = 2U * sizeof(int16_t)
};

typedef enum {
    CSB_V1_F0468_MEMORY_TEMPORARY_ON_TOP_OF_HEAP_PC34 = 0,
    CSB_V1_F0468_MEMORY_PERMANENT_PC34 = 1,
    CSB_V1_F0468_MEMORY_TEMPORARY_ON_BOTTOM_OF_HEAP_PC34 = 2
} CsbV1F0468MemoryAllocationTypePc34Compat;

/* All members are offsets in a caller-owned heap address space. */
typedef struct {
    size_t heap_begin;
    size_t heap_limit;
    size_t heap_end;
    size_t heap_top_of_temporary;
    size_t available_heap_byte_count;
} CsbV1F0468MemoryStatePc34Compat;

typedef struct {
    size_t byte_count;
    CsbV1F0468MemoryAllocationTypePc34Compat allocation_type;
    uint16_t memory_request;
} CsbV1F0468MemoryAllocateRequestPc34Compat;

typedef struct {
    size_t usable_offset;
    size_t reserved_byte_count;
} CsbV1F0468MemoryAllocateResultPc34Compat;

/*
 * Maps the cursor and result behavior of PC 3.4 F0468_MEMORY_Allocate.
 * The requested count is rounded to an even byte count; a bitmap request
 * reserves its two-word header before the returned usable offset. Returns
 * false without changing state or result if the request cannot fit.
 */
bool csb_v1_f0468_memory_allocate_pc34_compat(
    CsbV1F0468MemoryStatePc34Compat *state,
    const CsbV1F0468MemoryAllocateRequestPc34Compat *request,
    CsbV1F0468MemoryAllocateResultPc34Compat *out_result);

const char *csb_v1_f0468_memory_allocate_source_evidence_pc34(void);

#ifdef __cplusplus
}
#endif

#endif
