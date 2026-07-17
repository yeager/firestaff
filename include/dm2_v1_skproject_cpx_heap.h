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

/* SKULLWIN/c_dballoc.cpp keeps reusable CPX blocks in descending amount
 * order. This model deliberately owns only that allocator metadata, never
 * decoded GDAT bytes or CPX backing storage. */
typedef struct {
    uint32_t amount;
    uint16_t next;
    uint16_t previous;
    uint8_t used;
} DM2_V1_SkprojectCpxReuseNode;

typedef struct {
    DM2_V1_SkprojectCpxReuseNode nodes[DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT];
    uint16_t head;
} DM2_V1_SkprojectCpxReuseList;

typedef struct {
    int valid;
    int rejected_null_state;
    int rejected_zero_size;
    int rejected_empty_list;
    int exact_match;
    int used_largest_block;
    int retained_remainder;
    uint32_t requested_amount;
    uint32_t allocated_amount;
    uint32_t selected_amount;
    uint32_t remainder_amount;
    uint16_t selected_node;
    uint16_t remainder_node;
    uint16_t head_before;
    uint16_t head_after;
} DM2_V1_SkprojectCpxReuseReceipt;

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
int dm2_v1_skproject_cpx_reuse_list_init(
    DM2_V1_SkprojectCpxReuseList *list,
    const uint32_t *amounts,
    uint16_t amount_count);
int dm2_v1_skproject_cpx_reuse_list_is_descending(
    const DM2_V1_SkprojectCpxReuseList *list);
int dm2_v1_skproject_cpx_alloc_cpx1_receipt(
    DM2_V1_SkprojectCpxReuseList *list,
    uint32_t requested_amount,
    DM2_V1_SkprojectCpxReuseReceipt *out_receipt);
const char *dm2_v1_skproject_cpx_heap_source_evidence(void);

#endif
