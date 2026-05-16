#include <stdio.h>
#include <stdint.h>

#include "memory_cache_defrag_loop_pc34_compat.h"

static int test_defrag_loop_skips_unused_and_moves_following_used_blocks(void) {
    struct MemoryCacheDefragSegment_Compat segments[3] = {
        {1, 0, 0, 20},
        {0, 0, 0, 10},
        {1, 1, 0, 30}
    };
    unsigned short nativeIndices[2] = {0, 1};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[2] = {0, 30};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragLoopResult_Compat result = {0};
    usageState.lastUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)30;
    return MEMORY_CACHE_RunDefragLoop_Compat(segments, 3, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &result) == 1 &&
           result.compactedTopOffset == 50 && result.movedBlockCount == 1 && blockOffsets[1] == 20 &&
           (uintptr_t)usageState.lastUsedBlock == 20;
}

static int test_defrag_loop_handles_derived_segment_move(void) {
    struct MemoryCacheDefragSegment_Compat segments[3] = {
        {0, 0, 0, 12},
        {1, 0, 1, 16},
        {1, 0, 0, 8}
    };
    unsigned short nativeIndices[1] = {1};
    unsigned short derivedIndices[1] = {0};
    unsigned long blockOffsets[2] = {12, 28};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragLoopResult_Compat result = {0};
    usageState.firstReferencedUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)12;
    return MEMORY_CACHE_RunDefragLoop_Compat(segments, 3, nativeIndices, derivedIndices, blockOffsets, &usageState, 1, 1, &result) == 1 &&
           result.compactedTopOffset == 24 && result.movedBlockCount == 2 && blockOffsets[0] == 0 && blockOffsets[1] == 16 &&
           (uintptr_t)usageState.firstReferencedUsedBlock == 0;
}

static int test_defrag_loop_fails_when_segment_bitmap_is_unregistered(void) {
    struct MemoryCacheDefragSegment_Compat segments[1] = {
        {1, 1, 0, 10}
    };
    unsigned short nativeIndices[2] = {0xFFFF, 0xFFFF};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragLoopResult_Compat result = {0};
    return MEMORY_CACHE_RunDefragLoop_Compat(segments, 1, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &result) == 0;
}

int main(void) {
    if (!test_defrag_loop_skips_unused_and_moves_following_used_blocks()) {
        fprintf(stderr, "test_defrag_loop_skips_unused_and_moves_following_used_blocks failed\n");
        return 1;
    }
    if (!test_defrag_loop_handles_derived_segment_move()) {
        fprintf(stderr, "test_defrag_loop_handles_derived_segment_move failed\n");
        return 1;
    }
    if (!test_defrag_loop_fails_when_segment_bitmap_is_unregistered()) {
        fprintf(stderr, "test_defrag_loop_fails_when_segment_bitmap_is_unregistered failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
