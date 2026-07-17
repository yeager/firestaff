#include "dm2_v1_skproject_cpx_heap.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(cond, msg) do { \
    if (!(cond)) { \
        ++failed; \
        printf("FAIL: %s\n", msg); \
    } else { \
        printf("PASS: %s\n", msg); \
    } \
} while (0)

static void test_lower_alloc_splits_from_low_address(void)
{
    DM2_V1_SkprojectCpxHeap heap;
    DM2_V1_SkprojectCpxAllocReceipt receipt;

    dm2_v1_skproject_cpx_heap_init(&heap, 100u);
    CHECK(heap.free_head != DM2_V1_SKPROJECT_CPX_NONE &&
              heap.nodes[heap.free_head].offset == 0u &&
              heap.nodes[heap.free_head].size == 100u,
          "CPX heap init publishes one lower free span");
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 24u, &receipt) == 1 &&
              receipt.valid && receipt.split_block &&
              receipt.allocated_offset == 0u &&
              heap.nodes[heap.free_head].offset == 24u &&
              heap.nodes[heap.free_head].size == 76u,
          "ALLOC_LOWER_CPXHEAP splits from the low edge of the free span");
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 16u, &receipt) == 1 &&
              receipt.allocated_offset == 24u &&
              heap.nodes[heap.free_head].offset == 40u &&
              heap.nodes[heap.free_head].size == 60u,
          "second lower allocation preserves ascending address order");
    CHECK(dm2_v1_skproject_cpx_free_list_is_ordered(&heap),
          "free list remains ordered after lower allocations");
}

static void test_free_inserts_ordered_and_coalesces(void)
{
    DM2_V1_SkprojectCpxHeap heap;
    DM2_V1_SkprojectCpxAllocReceipt alloc_receipt;
    DM2_V1_SkprojectCpxFreeReceipt free_receipt;
    uint16_t head;

    dm2_v1_skproject_cpx_heap_init(&heap, 128u);
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 16u, &alloc_receipt) == 1,
          "allocated first CPX span");
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 32u, &alloc_receipt) == 1,
          "allocated second CPX span");
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 16u, &alloc_receipt) == 1,
          "allocated third CPX span");

    CHECK(dm2_v1_skproject_cpx_free_lower(&heap, 16u, 32u,
                                          &free_receipt) == 1 &&
              free_receipt.valid &&
              heap.nodes[heap.free_head].offset == 16u &&
              heap.nodes[heap.free_head].size == 32u,
          "free inserts middle span before the existing high free span");
    CHECK(dm2_v1_skproject_cpx_free_lower(&heap, 0u, 16u,
                                          &free_receipt) == 1 &&
              free_receipt.coalesced_next &&
              heap.nodes[heap.free_head].offset == 0u &&
              heap.nodes[heap.free_head].size == 48u,
          "free coalesces with the next lower-ordered span");
    CHECK(dm2_v1_skproject_cpx_free_lower(&heap, 48u, 16u,
                                          &free_receipt) == 1 &&
              free_receipt.coalesced_previous &&
              free_receipt.coalesced_next,
          "free bridges previous and next spans into one ordered block");

    head = heap.free_head;
    CHECK(heap.nodes[head].offset == 0u &&
              heap.nodes[head].size == 128u &&
              heap.nodes[head].next == DM2_V1_SKPROJECT_CPX_NONE,
          "coalesced CPX free list recovers the whole heap");
    CHECK(dm2_v1_skproject_cpx_free_list_is_ordered(&heap),
          "coalesced free list is still ordered");
}

static void test_exact_fit_and_rejections(void)
{
    DM2_V1_SkprojectCpxHeap heap;
    DM2_V1_SkprojectCpxAllocReceipt alloc_receipt;
    DM2_V1_SkprojectCpxFreeReceipt free_receipt;

    dm2_v1_skproject_cpx_heap_init(&heap, 12u);
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 12u, &alloc_receipt) == 1 &&
              alloc_receipt.exact_fit &&
              heap.free_head == DM2_V1_SKPROJECT_CPX_NONE,
          "exact-fit lower allocation unlinks the free node");
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 1u, &alloc_receipt) == 0 &&
              alloc_receipt.rejected_no_block,
          "allocation fails closed when no free block remains");
    CHECK(dm2_v1_skproject_cpx_alloc_lower(&heap, 0u, &alloc_receipt) == 0 &&
              alloc_receipt.rejected_zero_size,
          "allocation rejects zero-byte requests");
    CHECK(dm2_v1_skproject_cpx_free_lower(&heap, 8u, 8u,
                                          &free_receipt) == 0 &&
              free_receipt.rejected_out_of_range,
          "free rejects spans outside the CPX heap");
    CHECK(dm2_v1_skproject_cpx_free_lower(&heap, 0u, 12u,
                                          &free_receipt) == 1 &&
              heap.free_head != DM2_V1_SKPROJECT_CPX_NONE,
          "free accepts the exact heap span after exact-fit allocation");
}

static void test_skullwin_alloc_cpx1_reuses_size_sorted_blocks(void)
{
    const uint32_t amounts[] = { 40u, 100u, 70u };
    DM2_V1_SkprojectCpxReuseList list;
    DM2_V1_SkprojectCpxReuseReceipt receipt;

    CHECK(dm2_v1_skproject_cpx_reuse_list_init(&list, amounts, 3u) == 1 &&
              dm2_v1_skproject_cpx_reuse_list_is_descending(&list) &&
              list.nodes[list.head].amount == 100u,
          "CPX1 setup preserves SKULLWIN descending-size list order");
    CHECK(dm2_v1_skproject_cpx_alloc_cpx1_receipt(&list, 70u, &receipt) == 1 &&
              receipt.exact_match && !receipt.used_largest_block &&
              receipt.allocated_amount == 70u && !receipt.retained_remainder,
          "CPX1 reuses an exact-size free block");
    CHECK(dm2_v1_skproject_cpx_alloc_cpx1_receipt(&list, 60u, &receipt) == 1 &&
              receipt.used_largest_block && receipt.retained_remainder &&
              receipt.allocated_amount == 60u && receipt.remainder_amount == 40u &&
              dm2_v1_skproject_cpx_reuse_list_is_descending(&list),
          "CPX1 splits the largest block only when the source remainder is at least 30");
    CHECK(dm2_v1_skproject_cpx_alloc_cpx1_receipt(&list, 25u, &receipt) == 1 &&
              receipt.used_largest_block && !receipt.retained_remainder &&
              receipt.allocated_amount == 40u,
          "CPX1 absorbs a remainder smaller than 30 bytes");
    CHECK(dm2_v1_skproject_cpx_alloc_cpx1_receipt(NULL, 8u, &receipt) == 0 &&
              receipt.rejected_null_state,
          "CPX1 rejects absent allocator state without making a buffer");
}

int main(void)
{
    test_lower_alloc_splits_from_low_address();
    test_free_inserts_ordered_and_coalesces();
    test_exact_fit_and_rejections();
    test_skullwin_alloc_cpx1_reuses_size_sorted_blocks();
    CHECK(strstr(dm2_v1_skproject_cpx_heap_source_evidence(),
                 "ALLOC_LOWER_CPXHEAP") != 0,
          "source evidence names ALLOC_LOWER_CPXHEAP");
    CHECK(strstr(dm2_v1_skproject_cpx_heap_source_evidence(),
                 "DM2_ALLOC_CPX_LINK_NODE") != 0,
          "source evidence names c_dballoc CPX link helpers");
    CHECK(strstr(dm2_v1_skproject_cpx_heap_source_evidence(),
                 "DM2_ALLOC_CPX1") != 0,
          "source evidence names the SKULLWIN CPX1 allocator");

    if (failed) {
        printf("%d failure(s)\n", failed);
        return 1;
    }
    puts("all DM2 skproject CPX heap receipt checks passed");
    return 0;
}
