#ifndef FIRESTAFF_DM2_V1_SKPROJECT_CPX_HEAP_H
#define FIRESTAFF_DM2_V1_SKPROJECT_CPX_HEAP_H

#include <stdint.h>

#define DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT 32u
#define DM2_V1_SKPROJECT_CPX_NONE 0xffffu

typedef struct {
    uint32_t offset;
    uint32_t size;
    uint16_t next;
    uint8_t used;
} DM2_V1_SkprojectCpxFreeNode;

typedef struct {
    DM2_V1_SkprojectCpxFreeNode nodes[DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT];
    uint16_t free_head;
    uint32_t heap_size;
} DM2_V1_SkprojectCpxHeap;

typedef struct {
    int valid;
    int rejected_null_state;
    int rejected_zero_size;
    int rejected_no_block;
    int exact_fit;
    int split_block;
    uint32_t requested_size;
    uint32_t allocated_offset;
    uint32_t selected_offset_before;
    uint32_t selected_size_before;
    uint32_t selected_size_after;
    uint16_t selected_node;
    uint16_t previous_node;
    uint16_t free_head_before;
    uint16_t free_head_after;
} DM2_V1_SkprojectCpxAllocReceipt;

typedef struct {
    int valid;
    int rejected_null_state;
    int rejected_zero_size;
    int rejected_out_of_range;
    int rejected_node_exhausted;
    int coalesced_previous;
    int coalesced_next;
    uint32_t freed_offset;
    uint32_t freed_size;
    uint16_t inserted_node;
    uint16_t previous_node;
    uint16_t next_node;
    uint16_t free_head_before;
    uint16_t free_head_after;
} DM2_V1_SkprojectCpxFreeReceipt;

void dm2_v1_skproject_cpx_heap_init(
    DM2_V1_SkprojectCpxHeap *heap,
    uint32_t heap_size);
int dm2_v1_skproject_cpx_alloc_lower(
    DM2_V1_SkprojectCpxHeap *heap,
    uint32_t size,
    DM2_V1_SkprojectCpxAllocReceipt *out_receipt);
int dm2_v1_skproject_cpx_free_lower(
    DM2_V1_SkprojectCpxHeap *heap,
    uint32_t offset,
    uint32_t size,
    DM2_V1_SkprojectCpxFreeReceipt *out_receipt);
int dm2_v1_skproject_cpx_free_list_is_ordered(
    const DM2_V1_SkprojectCpxHeap *heap);
const char *dm2_v1_skproject_cpx_heap_source_evidence(void);

#endif
