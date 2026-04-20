#include <stdio.h>
#include <stdint.h>

#include "memory_cache_defrag_orchestrator_pc34_compat.h"

static int test_orchestrator_scans_and_defrags_mixed_headers(void) {
    struct MemoryCacheRawBlockHeader_Compat headers[3] = {
        {-20, 0, 0},
        {10, 0, 0},
        {-30, 1, 0}
    };
    struct MemoryCacheDefragSegment_Compat segments[3] = {0};
    unsigned short nativeIndices[2] = {0, 1};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[2] = {0, 30};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {60, (struct MemoryCacheUnusedBlock_Compat*)(uintptr_t)1};
    struct MemoryCacheDefragOrchestratorResult_Compat result = {0};
    usageState.lastUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)30;
    return F0482_CACHE_DefragmentMini_Compat(headers, 3, segments, 3, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &defragState, &result) == 1 &&
           result.segmentCount == 3 && result.movedBlockCount == 1 && result.compactedTopOffset == 50 &&
           defragState.cacheMemoryTopOffset == 50 && defragState.firstUnusedBlock == 0 && blockOffsets[1] == 20;
}

static int test_orchestrator_respects_segment_capacity(void) {
    struct MemoryCacheRawBlockHeader_Compat headers[3] = {
        {-12, 0, 0},
        {8, 0, 0},
        {-16, 1, 1}
    };
    struct MemoryCacheDefragSegment_Compat segments[2] = {0};
    unsigned short nativeIndices[2] = {0, 0xFFFF};
    unsigned short derivedIndices[2] = {0xFFFF, 0xFFFF};
    unsigned long blockOffsets[1] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {12, (struct MemoryCacheUnusedBlock_Compat*)(uintptr_t)1};
    struct MemoryCacheDefragOrchestratorResult_Compat result = {0};
    return F0482_CACHE_DefragmentMini_Compat(headers, 3, segments, 2, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 2, &defragState, &result) == 1 &&
           result.segmentCount == 2;
}

static int test_orchestrator_fails_when_scanned_used_segment_is_unregistered(void) {
    struct MemoryCacheRawBlockHeader_Compat headers[1] = {
        {-20, 1, 0}
    };
    struct MemoryCacheDefragSegment_Compat segments[1] = {0};
    unsigned short nativeIndices[2] = {0xFFFF, 0xFFFF};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {20, (struct MemoryCacheUnusedBlock_Compat*)(uintptr_t)1};
    struct MemoryCacheDefragOrchestratorResult_Compat result = {0};
    return F0482_CACHE_DefragmentMini_Compat(headers, 1, segments, 1, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &defragState, &result) == 0;
}

int main(void) {
    if (!test_orchestrator_scans_and_defrags_mixed_headers()) {
        fprintf(stderr, "test_orchestrator_scans_and_defrags_mixed_headers failed\n");
        return 1;
    }
    if (!test_orchestrator_respects_segment_capacity()) {
        fprintf(stderr, "test_orchestrator_respects_segment_capacity failed\n");
        return 1;
    }
    if (!test_orchestrator_fails_when_scanned_used_segment_is_unregistered()) {
        fprintf(stderr, "test_orchestrator_fails_when_scanned_used_segment_is_unregistered failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
