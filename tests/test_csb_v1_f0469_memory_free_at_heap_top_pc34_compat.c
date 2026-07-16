#include "csb_v1_f0469_memory_free_at_heap_top_pc34_compat.h"

#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define CHECK(expr) do { \
    if (!(expr)) { \
        fprintf(stderr, "check failed at %s:%d: %s\n", __FILE__, __LINE__, #expr); \
        return 1; \
    } \
} while (0)

static int test_source_named_wrapper_rounds_odd_byte_counts(void)
{
    CsbV1F0469MemoryHeapAccountingPc34Compat heap = {
        100u,
        200u,
        140u,
        40u
    };

    CHECK(F0469_MEMORY_FreeAtHeapTop(&heap, 5u) == true);
    CHECK(heap.available_heap_top == 146u);
    CHECK(heap.available_heap_byte_count == 46u);
    return 0;
}

static int test_even_byte_counts_are_released_exactly(void)
{
    CsbV1F0469MemoryHeapAccountingPc34Compat heap = {
        100u,
        200u,
        140u,
        40u
    };

    CHECK(F0469_MEMORY_FreeAtHeapTop(&heap, 8u) == true);
    CHECK(heap.available_heap_top == 148u);
    CHECK(heap.available_heap_byte_count == 48u);
    return 0;
}

static int test_invalid_state_does_not_mutate(void)
{
    CsbV1F0469MemoryHeapAccountingPc34Compat heap = {
        100u,
        200u,
        220u,
        40u
    };
    const CsbV1F0469MemoryHeapAccountingPc34Compat before = heap;

    CHECK(F0469_MEMORY_FreeAtHeapTop(&heap, 2u) == false);
    CHECK(memcmp(&heap, &before, sizeof(heap)) == 0);
    return 0;
}

static int test_release_beyond_heap_is_rejected_without_mutation(void)
{
    CsbV1F0469MemoryHeapAccountingPc34Compat heap = {
        100u,
        200u,
        196u,
        96u
    };
    const CsbV1F0469MemoryHeapAccountingPc34Compat before = heap;

    CHECK(F0469_MEMORY_FreeAtHeapTop(&heap, 5u) == false);
    CHECK(memcmp(&heap, &before, sizeof(heap)) == 0);
    return 0;
}

static int test_wrapper_matches_csb_compat_entrypoint(void)
{
    CsbV1F0469MemoryHeapAccountingPc34Compat wrapper_heap = {
        0u,
        64u,
        16u,
        16u
    };
    CsbV1F0469MemoryHeapAccountingPc34Compat compat_heap = wrapper_heap;

    CHECK(F0469_MEMORY_FreeAtHeapTop(&wrapper_heap, 3u) == true);
    CHECK(csb_v1_f0469_memory_free_at_heap_top_pc34_compat(
        &compat_heap,
        3u) == true);
    CHECK(memcmp(&wrapper_heap, &compat_heap, sizeof(wrapper_heap)) == 0);
    return 0;
}

static int test_source_evidence_names_f0469(void)
{
    const char *evidence =
        csb_v1_f0469_memory_free_at_heap_top_source_evidence_pc34();

    CHECK(evidence != 0);
    CHECK(strstr(evidence, "F469") != 0);
    CHECK(strstr(evidence, "AvailableHeapMemoryTop") != 0);
    return 0;
}

int main(void)
{
    CHECK(test_source_named_wrapper_rounds_odd_byte_counts() == 0);
    CHECK(test_even_byte_counts_are_released_exactly() == 0);
    CHECK(test_invalid_state_does_not_mutate() == 0);
    CHECK(test_release_beyond_heap_is_rejected_without_mutation() == 0);
    CHECK(test_wrapper_matches_csb_compat_entrypoint() == 0);
    CHECK(test_source_evidence_names_f0469() == 0);
    return 0;
}
