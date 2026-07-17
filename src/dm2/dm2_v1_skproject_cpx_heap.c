#include "dm2_v1_skproject_cpx_heap.h"

#include <string.h>

static uint16_t dm2_v1_cpx_alloc_node(DM2_V1_SkprojectCpxHeap *heap)
{
    uint16_t i;

    for (i = 0; i < DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT; ++i) {
        if (!heap->nodes[i].used) {
            heap->nodes[i].used = 1u;
            heap->nodes[i].next = DM2_V1_SKPROJECT_CPX_NONE;
            heap->nodes[i].offset = 0u;
            heap->nodes[i].size = 0u;
            return i;
        }
    }
    return DM2_V1_SKPROJECT_CPX_NONE;
}

static void dm2_v1_cpx_release_node(
    DM2_V1_SkprojectCpxHeap *heap,
    uint16_t node)
{
    if (!heap || node >= DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT) return;
    memset(&heap->nodes[node], 0, sizeof(heap->nodes[node]));
}

void dm2_v1_skproject_cpx_heap_init(
    DM2_V1_SkprojectCpxHeap *heap,
    uint32_t heap_size)
{
    uint16_t node;

    if (!heap) return;
    memset(heap, 0, sizeof(*heap));
    heap->free_head = DM2_V1_SKPROJECT_CPX_NONE;
    heap->heap_size = heap_size;
    if (heap_size == 0u) return;
    node = dm2_v1_cpx_alloc_node(heap);
    if (node == DM2_V1_SKPROJECT_CPX_NONE) return;
    heap->nodes[node].offset = 0u;
    heap->nodes[node].size = heap_size;
    heap->free_head = node;
}

int dm2_v1_skproject_cpx_alloc_lower(
    DM2_V1_SkprojectCpxHeap *heap,
    uint32_t size,
    DM2_V1_SkprojectCpxAllocReceipt *out_receipt)
{
    uint16_t prev = DM2_V1_SKPROJECT_CPX_NONE;
    uint16_t node;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt) return 0;
    out_receipt->requested_size = size;
    if (!heap) {
        out_receipt->rejected_null_state = 1;
        return 0;
    }
    out_receipt->free_head_before = heap->free_head;
    if (size == 0u) {
        out_receipt->rejected_zero_size = 1;
        out_receipt->free_head_after = heap->free_head;
        return 0;
    }

    node = heap->free_head;
    while (node != DM2_V1_SKPROJECT_CPX_NONE) {
        DM2_V1_SkprojectCpxFreeNode *free_node;

        if (node >= DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT ||
            !heap->nodes[node].used) {
            break;
        }
        free_node = &heap->nodes[node];
        if (free_node->size >= size) {
            out_receipt->valid = 1;
            out_receipt->selected_node = node;
            out_receipt->previous_node = prev;
            out_receipt->allocated_offset = free_node->offset;
            out_receipt->selected_offset_before = free_node->offset;
            out_receipt->selected_size_before = free_node->size;
            if (free_node->size == size) {
                out_receipt->exact_fit = 1;
                if (prev == DM2_V1_SKPROJECT_CPX_NONE)
                    heap->free_head = free_node->next;
                else
                    heap->nodes[prev].next = free_node->next;
                dm2_v1_cpx_release_node(heap, node);
            } else {
                out_receipt->split_block = 1;
                free_node->offset += size;
                free_node->size -= size;
                out_receipt->selected_size_after = free_node->size;
            }
            out_receipt->free_head_after = heap->free_head;
            return 1;
        }
        prev = node;
        node = free_node->next;
    }

    out_receipt->rejected_no_block = 1;
    out_receipt->free_head_after = heap->free_head;
    return 0;
}

int dm2_v1_skproject_cpx_free_lower(
    DM2_V1_SkprojectCpxHeap *heap,
    uint32_t offset,
    uint32_t size,
    DM2_V1_SkprojectCpxFreeReceipt *out_receipt)
{
    uint16_t prev = DM2_V1_SKPROJECT_CPX_NONE;
    uint16_t next;
    uint16_t node;
    uint64_t end;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt) return 0;
    out_receipt->freed_offset = offset;
    out_receipt->freed_size = size;
    if (!heap) {
        out_receipt->rejected_null_state = 1;
        return 0;
    }
    out_receipt->free_head_before = heap->free_head;
    if (size == 0u) {
        out_receipt->rejected_zero_size = 1;
        out_receipt->free_head_after = heap->free_head;
        return 0;
    }
    end = (uint64_t)offset + (uint64_t)size;
    if (end > (uint64_t)heap->heap_size) {
        out_receipt->rejected_out_of_range = 1;
        out_receipt->free_head_after = heap->free_head;
        return 0;
    }

    next = heap->free_head;
    while (next != DM2_V1_SKPROJECT_CPX_NONE &&
           next < DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT &&
           heap->nodes[next].used &&
           heap->nodes[next].offset < offset) {
        prev = next;
        next = heap->nodes[next].next;
    }

    node = dm2_v1_cpx_alloc_node(heap);
    if (node == DM2_V1_SKPROJECT_CPX_NONE) {
        out_receipt->rejected_node_exhausted = 1;
        out_receipt->free_head_after = heap->free_head;
        return 0;
    }

    heap->nodes[node].offset = offset;
    heap->nodes[node].size = size;
    heap->nodes[node].next = next;
    if (prev == DM2_V1_SKPROJECT_CPX_NONE)
        heap->free_head = node;
    else
        heap->nodes[prev].next = node;

    out_receipt->valid = 1;
    out_receipt->inserted_node = node;
    out_receipt->previous_node = prev;
    out_receipt->next_node = next;

    if (prev != DM2_V1_SKPROJECT_CPX_NONE &&
        heap->nodes[prev].offset + heap->nodes[prev].size ==
            heap->nodes[node].offset) {
        heap->nodes[prev].size += heap->nodes[node].size;
        heap->nodes[prev].next = heap->nodes[node].next;
        dm2_v1_cpx_release_node(heap, node);
        node = prev;
        out_receipt->coalesced_previous = 1;
        out_receipt->inserted_node = node;
    }
    next = heap->nodes[node].next;
    if (next != DM2_V1_SKPROJECT_CPX_NONE &&
        next < DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT &&
        heap->nodes[next].used &&
        heap->nodes[node].offset + heap->nodes[node].size ==
            heap->nodes[next].offset) {
        heap->nodes[node].size += heap->nodes[next].size;
        heap->nodes[node].next = heap->nodes[next].next;
        dm2_v1_cpx_release_node(heap, next);
        out_receipt->coalesced_next = 1;
    }
    out_receipt->free_head_after = heap->free_head;
    return 1;
}

int dm2_v1_skproject_cpx_free_list_is_ordered(
    const DM2_V1_SkprojectCpxHeap *heap)
{
    uint16_t node;
    uint32_t previous_end = 0u;
    int have_previous = 0;

    if (!heap) return 0;
    node = heap->free_head;
    while (node != DM2_V1_SKPROJECT_CPX_NONE) {
        const DM2_V1_SkprojectCpxFreeNode *free_node;

        if (node >= DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT ||
            !heap->nodes[node].used) {
            return 0;
        }
        free_node = &heap->nodes[node];
        if (free_node->size == 0u ||
            (uint64_t)free_node->offset + (uint64_t)free_node->size >
                (uint64_t)heap->heap_size) {
            return 0;
        }
        if (have_previous && free_node->offset < previous_end)
            return 0;
        previous_end = free_node->offset + free_node->size;
        have_previous = 1;
        node = free_node->next;
    }
    return 1;
}

static void dm2_v1_cpx_reuse_link_node(
    DM2_V1_SkprojectCpxReuseList *list,
    uint16_t node)
{
    uint16_t current;

    if (list->head == DM2_V1_SKPROJECT_CPX_NONE) {
        list->head = node;
        return;
    }
    current = list->head;
    if (list->nodes[node].amount >= list->nodes[current].amount) {
        list->nodes[node].next = current;
        list->nodes[current].previous = node;
        list->head = node;
        return;
    }
    while (list->nodes[current].next != DM2_V1_SKPROJECT_CPX_NONE) {
        uint16_t next = list->nodes[current].next;
        if (list->nodes[node].amount >= list->nodes[next].amount) {
            list->nodes[node].next = next;
            list->nodes[node].previous = current;
            list->nodes[next].previous = node;
            list->nodes[current].next = node;
            return;
        }
        current = next;
    }
    list->nodes[node].previous = current;
    list->nodes[current].next = node;
}

static void dm2_v1_cpx_reuse_unlink_node(
    DM2_V1_SkprojectCpxReuseList *list,
    uint16_t node)
{
    uint16_t previous = list->nodes[node].previous;
    uint16_t next = list->nodes[node].next;

    if (previous != DM2_V1_SKPROJECT_CPX_NONE)
        list->nodes[previous].next = next;
    else
        list->head = next;
    if (next != DM2_V1_SKPROJECT_CPX_NONE)
        list->nodes[next].previous = previous;
    list->nodes[node].next = DM2_V1_SKPROJECT_CPX_NONE;
    list->nodes[node].previous = DM2_V1_SKPROJECT_CPX_NONE;
}

int dm2_v1_skproject_cpx_reuse_list_init(
    DM2_V1_SkprojectCpxReuseList *list,
    const uint32_t *amounts,
    uint16_t amount_count)
{
    uint16_t i;

    if (!list || (amount_count != 0u && !amounts) ||
        amount_count > DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT)
        return 0;
    memset(list, 0, sizeof(*list));
    list->head = DM2_V1_SKPROJECT_CPX_NONE;
    for (i = 0u; i < amount_count; ++i) {
        if (amounts[i] == 0u) return 0;
        list->nodes[i].amount = amounts[i];
        list->nodes[i].next = DM2_V1_SKPROJECT_CPX_NONE;
        list->nodes[i].previous = DM2_V1_SKPROJECT_CPX_NONE;
        list->nodes[i].used = 1u;
        dm2_v1_cpx_reuse_link_node(list, i);
    }
    return 1;
}

int dm2_v1_skproject_cpx_reuse_list_is_descending(
    const DM2_V1_SkprojectCpxReuseList *list)
{
    uint16_t current;

    if (!list) return 0;
    current = list->head;
    while (current != DM2_V1_SKPROJECT_CPX_NONE) {
        uint16_t next;
        if (current >= DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT ||
            !list->nodes[current].used || list->nodes[current].amount == 0u)
            return 0;
        next = list->nodes[current].next;
        if (next != DM2_V1_SKPROJECT_CPX_NONE &&
            (next >= DM2_V1_SKPROJECT_CPX_FREE_NODE_LIMIT ||
             !list->nodes[next].used ||
             list->nodes[current].amount < list->nodes[next].amount ||
             list->nodes[next].previous != current))
            return 0;
        current = next;
    }
    return 1;
}

int dm2_v1_skproject_cpx_alloc_cpx1_receipt(
    DM2_V1_SkprojectCpxReuseList *list,
    uint32_t requested_amount,
    DM2_V1_SkprojectCpxReuseReceipt *out_receipt)
{
    uint16_t selected;
    uint32_t remainder;

    if (out_receipt) memset(out_receipt, 0, sizeof(*out_receipt));
    if (!out_receipt) return 0;
    out_receipt->requested_amount = requested_amount;
    if (!list) {
        out_receipt->rejected_null_state = 1;
        return 0;
    }
    out_receipt->head_before = list->head;
    if (requested_amount == 0u) {
        out_receipt->rejected_zero_size = 1;
        out_receipt->head_after = list->head;
        return 0;
    }
    if (list->head == DM2_V1_SKPROJECT_CPX_NONE ||
        !dm2_v1_skproject_cpx_reuse_list_is_descending(list)) {
        out_receipt->rejected_empty_list = 1;
        out_receipt->head_after = list->head;
        return 0;
    }

    selected = list->head;
    while (list->nodes[selected].amount != requested_amount) {
        if (requested_amount > list->nodes[selected].amount ||
            list->nodes[selected].next == DM2_V1_SKPROJECT_CPX_NONE) {
            selected = list->head;
            out_receipt->used_largest_block = 1;
            break;
        }
        selected = list->nodes[selected].next;
    }
    if (list->nodes[selected].amount == requested_amount)
        out_receipt->exact_match = 1;
    out_receipt->selected_node = selected;
    out_receipt->selected_amount = list->nodes[selected].amount;
    dm2_v1_cpx_reuse_unlink_node(list, selected);
    list->nodes[selected].used = 0u;

    remainder = out_receipt->selected_amount - requested_amount;
    if (remainder < 30u) {
        out_receipt->allocated_amount = out_receipt->selected_amount;
    } else {
        list->nodes[selected].amount = remainder;
        list->nodes[selected].used = 1u;
        dm2_v1_cpx_reuse_link_node(list, selected);
        out_receipt->allocated_amount = requested_amount;
        out_receipt->remainder_amount = remainder;
        out_receipt->remainder_node = selected;
        out_receipt->retained_remainder = 1;
    }
    out_receipt->head_after = list->head;
    out_receipt->valid = 1;
    return 1;
}

const char *dm2_v1_skproject_cpx_heap_source_evidence(void)
{
    return "skproject SKWIN/SkWinCore.cpp ALLOC_LOWER_CPXHEAP/"
           "ALLOC_CPXHEAP_MEM; "
           "SKULLWIN/c_dballoc.cpp DM2_ALLOC_CPX_LINK_NODE/"
           "DM2_ALLOC_CPX_UNLINK_NODE/DM2_ALLOC_CPX1/"
           "DM2_ALLOC_CPXHEAP_MEM";
}
