#include <stdio.h>

#include "memory_cache_defrag_branch_pc34_compat.h"

static int test_uses_top_when_span_already_sufficient(void) {
    struct MemoryCacheDefragBranchResult_Compat result = MEMORY_CACHE_CheckDefragBranch_Compat(24, 32, 0);
    return result.shouldDefragment == 0 && result.shouldAllocateFromTopAfterDefrag == 1 && result.shouldReuseUnusedBlock == 0;
}

static int test_requests_defrag_when_largest_unused_block_too_small(void) {
    struct MemoryCacheUnusedBlock_Compat first = {16, 0, 0};
    struct MemoryCacheDefragBranchResult_Compat result = MEMORY_CACHE_CheckDefragBranch_Compat(24, 8, &first);
    return result.shouldDefragment == 1 && result.shouldAllocateFromTopAfterDefrag == 1 && result.shouldReuseUnusedBlock == 0;
}

static int test_reuses_when_largest_unused_block_is_large_enough(void) {
    struct MemoryCacheUnusedBlock_Compat first = {40, 0, 0};
    struct MemoryCacheDefragBranchResult_Compat result = MEMORY_CACHE_CheckDefragBranch_Compat(24, 8, &first);
    return result.shouldDefragment == 0 && result.shouldAllocateFromTopAfterDefrag == 0 && result.shouldReuseUnusedBlock == 1;
}

static int test_no_action_when_no_span_and_no_unused_block(void) {
    struct MemoryCacheDefragBranchResult_Compat result = MEMORY_CACHE_CheckDefragBranch_Compat(24, 8, 0);
    return result.shouldDefragment == 0 && result.shouldAllocateFromTopAfterDefrag == 0 && result.shouldReuseUnusedBlock == 0;
}

int main(void) {
    if (!test_uses_top_when_span_already_sufficient()) {
        fprintf(stderr, "test_uses_top_when_span_already_sufficient failed\n");
        return 1;
    }
    if (!test_requests_defrag_when_largest_unused_block_too_small()) {
        fprintf(stderr, "test_requests_defrag_when_largest_unused_block_too_small failed\n");
        return 1;
    }
    if (!test_reuses_when_largest_unused_block_is_large_enough()) {
        fprintf(stderr, "test_reuses_when_largest_unused_block_is_large_enough failed\n");
        return 1;
    }
    if (!test_no_action_when_no_span_and_no_unused_block()) {
        fprintf(stderr, "test_no_action_when_no_span_and_no_unused_block failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
