#include <stdio.h>

#include "memory_cache_allocator_pc34_compat.h"

static int test_allocator_takes_top_path_without_defrag(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 64, 10};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlock_Compat(24, 24, &state, &first, &remainder, &result) == 1 &&
           result.allocatedFromTop == 1 && result.reusedExistingBlock == 0 && result.defragmentRequested == 0 &&
           result.blockOffset == 10 && state.cacheMemoryTopOffset == 34;
}

static int test_allocator_reuses_when_unused_block_is_large_enough(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 8, 10};
    struct MemoryCacheUnusedBlock_Compat firstBlock = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &firstBlock;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlock_Compat(24, 24, &state, &first, &remainder, &result) == 1 &&
           result.allocatedFromTop == 0 && result.reusedExistingBlock == 1 && result.defragmentRequested == 0 &&
           result.allocatedSize == 40;
}

static int test_allocator_marks_defrag_when_largest_unused_is_too_small(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 8, 10};
    struct MemoryCacheUnusedBlock_Compat firstBlock = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &firstBlock;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlock_Compat(24, 24, &state, &first, &remainder, &result) == 0 ? 0 : result.defragmentRequested == 1;
}

static int test_allocator_fails_when_no_top_and_no_unused_path_exists(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 8, 10};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocatorResult_Compat result = {0};
    return F0483_CACHE_GetNewBlock_Compat(24, 24, &state, &first, &remainder, &result) == 0;
}

int main(void) {
    if (!test_allocator_takes_top_path_without_defrag()) {
        fprintf(stderr, "test_allocator_takes_top_path_without_defrag failed\n");
        return 1;
    }
    if (!test_allocator_reuses_when_unused_block_is_large_enough()) {
        fprintf(stderr, "test_allocator_reuses_when_unused_block_is_large_enough failed\n");
        return 1;
    }
    if (!test_allocator_marks_defrag_when_largest_unused_is_too_small()) {
        fprintf(stderr, "test_allocator_marks_defrag_when_largest_unused_is_too_small failed\n");
        return 1;
    }
    if (!test_allocator_fails_when_no_top_and_no_unused_path_exists()) {
        fprintf(stderr, "test_allocator_fails_when_no_top_and_no_unused_path_exists failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
