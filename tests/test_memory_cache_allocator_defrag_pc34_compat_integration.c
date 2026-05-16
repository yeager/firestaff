#include <stdio.h>

#include "memory_cache_allocator_defrag_pc34_compat.h"

static int test_allocator_defrag_path_applies_compacted_top_and_clears_unused(void) {
    struct MemoryCacheAllocateOrReuseState_Compat allocatorState = {100, 8, 10};
    struct MemoryCacheDefragResultState_Compat defragState = {10, 0};
    struct MemoryCacheUnusedBlock_Compat firstBlock = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &firstBlock;
    struct MemoryCacheUnusedBlock_Compat splitRemainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlockWithDefrag_Compat(24, 24, 60, &allocatorState, &defragState, &first, &splitRemainder, &result) == 1 &&
           result.defragmentRequested == 1 && allocatorState.cacheMemoryTopOffset == 60 && defragState.cacheMemoryTopOffset == 60 && first == 0;
}

static int test_allocator_non_defrag_path_leaves_defrag_state_alone(void) {
    struct MemoryCacheAllocateOrReuseState_Compat allocatorState = {100, 64, 10};
    struct MemoryCacheDefragResultState_Compat defragState = {77, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat splitRemainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlockWithDefrag_Compat(24, 24, 60, &allocatorState, &defragState, &first, &splitRemainder, &result) == 1 &&
           result.defragmentRequested == 0 && allocatorState.cacheMemoryTopOffset == 34 && defragState.cacheMemoryTopOffset == 77;
}

static int test_allocator_defrag_path_fails_when_total_available_too_small(void) {
    struct MemoryCacheAllocateOrReuseState_Compat allocatorState = {20, 8, 10};
    struct MemoryCacheDefragResultState_Compat defragState = {10, 0};
    struct MemoryCacheUnusedBlock_Compat firstBlock = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &firstBlock;
    struct MemoryCacheUnusedBlock_Compat splitRemainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlockWithDefrag_Compat(24, 24, 60, &allocatorState, &defragState, &first, &splitRemainder, &result) == 0;
}

int main(void) {
    if (!test_allocator_defrag_path_applies_compacted_top_and_clears_unused()) {
        fprintf(stderr, "test_allocator_defrag_path_applies_compacted_top_and_clears_unused failed\n");
        return 1;
    }
    if (!test_allocator_non_defrag_path_leaves_defrag_state_alone()) {
        fprintf(stderr, "test_allocator_non_defrag_path_leaves_defrag_state_alone failed\n");
        return 1;
    }
    if (!test_allocator_defrag_path_fails_when_total_available_too_small()) {
        fprintf(stderr, "test_allocator_defrag_path_fails_when_total_available_too_small failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
