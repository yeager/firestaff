#include <stdio.h>
#include <stdint.h>

#include "memory_cache_defrag_apply_pc34_compat.h"

static int test_defrag_apply_runs_loop_and_updates_defrag_state(void) {
    struct MemoryCacheDefragSegment_Compat segments[3] = {
        {1, 0, 0, 20},
        {0, 0, 0, 10},
        {1, 1, 0, 30}
    };
    unsigned short nativeIndices[2] = {0, 1};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[2] = {0, 30};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {60, (struct MemoryCacheUnusedBlock_Compat*)(uintptr_t)1};
    struct MemoryCacheDefragApplyResult_Compat result = {0};
    usageState.lastUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)30;
    return F0482_CACHE_Defragment_Compat(segments, 3, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &defragState, &result) == 1 &&
           result.compactedTopOffset == 50 && result.movedBlockCount == 1 && defragState.cacheMemoryTopOffset == 50 && defragState.firstUnusedBlock == 0 &&
           blockOffsets[1] == 20 && (uintptr_t)usageState.lastUsedBlock == 20;
}

static int test_defrag_apply_is_noop_when_unused_list_is_empty(void) {
    struct MemoryCacheDefragSegment_Compat segments[1] = {
        {1, 0, 0, 20}
    };
    unsigned short nativeIndices[1] = {0};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {20, 0};
    struct MemoryCacheDefragApplyResult_Compat result = {0};
    return F0482_CACHE_Defragment_Compat(segments, 1, nativeIndices, derivedIndices, blockOffsets, &usageState, 1, 1, &defragState, &result) == 1 &&
           result.compactedTopOffset == 20 && result.movedBlockCount == 0 && defragState.cacheMemoryTopOffset == 20;
}

static int test_defrag_apply_fails_when_loop_hits_unregistered_block(void) {
    struct MemoryCacheDefragSegment_Compat segments[1] = {
        {1, 1, 0, 20}
    };
    unsigned short nativeIndices[2] = {0xFFFF, 0xFFFF};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {20, (struct MemoryCacheUnusedBlock_Compat*)(uintptr_t)1};
    struct MemoryCacheDefragApplyResult_Compat result = {0};
    return F0482_CACHE_Defragment_Compat(segments, 1, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &defragState, &result) == 0;
}

int main(void) {
    if (!test_defrag_apply_runs_loop_and_updates_defrag_state()) {
        fprintf(stderr, "test_defrag_apply_runs_loop_and_updates_defrag_state failed\n");
        return 1;
    }
    if (!test_defrag_apply_is_noop_when_unused_list_is_empty()) {
        fprintf(stderr, "test_defrag_apply_is_noop_when_unused_list_is_empty failed\n");
        return 1;
    }
    if (!test_defrag_apply_fails_when_loop_hits_unregistered_block()) {
        fprintf(stderr, "test_defrag_apply_fails_when_loop_hits_unregistered_block failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
