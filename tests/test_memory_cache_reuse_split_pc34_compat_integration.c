#include <stdio.h>

#include "memory_cache_reuse_split_pc34_compat.h"

static int test_returns_failure_when_no_unused_block_exists(void) {
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheReuseSplitResult_Compat result = {0};
    return MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(24, 24, &first, &remainder, &result) == 0;
}

static int test_exact_fit_selects_matching_block(void) {
    struct MemoryCacheUnusedBlock_Compat a = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {24, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheReuseSplitResult_Compat result = {0};
    F0472_CACHE_AddUnusedBlock_Compat(&a, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&b, &first);
    return MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(24, 24, &first, &remainder, &result) == 1 &&
           result.selectedBlock == &b && result.reusedExistingBlock == 1 && result.exactFit == 1 && result.splitPerformed == 0 &&
           first == &a && a.nextUnusedBlock == 0;
}

static int test_fallback_to_largest_block_and_split_when_needed(void) {
    struct MemoryCacheUnusedBlock_Compat a = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {32, 0, 0};
    struct MemoryCacheUnusedBlock_Compat c = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheReuseSplitResult_Compat result = {0};
    F0472_CACHE_AddUnusedBlock_Compat(&a, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&b, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&c, &first);
    return MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(24, 24, &first, &remainder, &result) == 1 &&
           result.selectedBlock == &a && result.exactFit == 0 && result.splitPerformed == 0 && result.allocatedSize == 40 &&
           first == &b && b.nextUnusedBlock == &c;
}

static int test_split_remainder_registered_when_large_enough(void) {
    struct MemoryCacheUnusedBlock_Compat a = {48, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheReuseSplitResult_Compat result = {0};
    F0472_CACHE_AddUnusedBlock_Compat(&a, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&b, &first);
    return MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(24, 24, &first, &remainder, &result) == 1 &&
           result.selectedBlock == &a && result.splitPerformed == 1 && result.allocatedSize == 24 && remainder.blockSize == 24 &&
           first == &remainder && remainder.nextUnusedBlock == &b && b.previousUnusedBlock == &remainder;
}

int main(void) {
    if (!test_returns_failure_when_no_unused_block_exists()) {
        fprintf(stderr, "test_returns_failure_when_no_unused_block_exists failed\n");
        return 1;
    }
    if (!test_exact_fit_selects_matching_block()) {
        fprintf(stderr, "test_exact_fit_selects_matching_block failed\n");
        return 1;
    }
    if (!test_fallback_to_largest_block_and_split_when_needed()) {
        fprintf(stderr, "test_fallback_to_largest_block_and_split_when_needed failed\n");
        return 1;
    }
    if (!test_split_remainder_registered_when_large_enough()) {
        fprintf(stderr, "test_split_remainder_registered_when_large_enough failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
