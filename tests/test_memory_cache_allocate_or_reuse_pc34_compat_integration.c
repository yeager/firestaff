#include <stdio.h>

#include "memory_cache_allocate_or_reuse_pc34_compat.h"

static int test_allocates_from_top_when_span_is_enough(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 64, 12};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocateOrReuseResult_Compat result = {0};
    return MEMORY_CACHE_AllocateOrReuseBlock_Compat(24, 24, &state, &first, &remainder, &result) == 1 &&
           result.allocatedFromTop == 1 && result.blockOffset == 12 && result.allocatedSize == 24 &&
           state.cacheMemoryTopOffset == 36 && state.cacheMemorySpan == 40 && state.cacheBytesAvailable == 76;
}

static int test_reuses_when_top_span_is_too_small(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 8, 20};
    struct MemoryCacheUnusedBlock_Compat a = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocateOrReuseResult_Compat result = {0};
    F0472_CACHE_AddUnusedBlock_Compat(&a, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&b, &first);
    return MEMORY_CACHE_AllocateOrReuseBlock_Compat(24, 24, &state, &first, &remainder, &result) == 1 &&
           result.allocatedFromTop == 0 && result.reusedExistingBlock == 1 && result.allocatedSize == 40 &&
           state.cacheMemoryTopOffset == 20 && state.cacheMemorySpan == 8 && state.cacheBytesAvailable == 60;
}

static int test_reuse_can_split_remainder(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {120, 4, 20};
    struct MemoryCacheUnusedBlock_Compat a = {48, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocateOrReuseResult_Compat result = {0};
    F0472_CACHE_AddUnusedBlock_Compat(&a, &first);
    return MEMORY_CACHE_AllocateOrReuseBlock_Compat(24, 24, &state, &first, &remainder, &result) == 1 &&
           result.allocatedFromTop == 0 && result.splitPerformed == 1 && result.allocatedSize == 24 &&
           remainder.blockSize == 24 && first == &remainder && state.cacheBytesAvailable == 96;
}

static int test_fails_when_no_top_space_and_no_unused_block(void) {
    struct MemoryCacheAllocateOrReuseState_Compat state = {100, 8, 20};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheAllocateOrReuseResult_Compat result = {0};
    return MEMORY_CACHE_AllocateOrReuseBlock_Compat(24, 24, &state, &first, &remainder, &result) == 0;
}

int main(void) {
    if (!test_allocates_from_top_when_span_is_enough()) {
        fprintf(stderr, "test_allocates_from_top_when_span_is_enough failed\n");
        return 1;
    }
    if (!test_reuses_when_top_span_is_too_small()) {
        fprintf(stderr, "test_reuses_when_top_span_is_too_small failed\n");
        return 1;
    }
    if (!test_reuse_can_split_remainder()) {
        fprintf(stderr, "test_reuse_can_split_remainder failed\n");
        return 1;
    }
    if (!test_fails_when_no_top_space_and_no_unused_block()) {
        fprintf(stderr, "test_fails_when_no_top_space_and_no_unused_block failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
