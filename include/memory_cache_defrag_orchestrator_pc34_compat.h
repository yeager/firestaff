#ifndef REDMCSB_MEMORY_CACHE_DEFRAG_ORCHESTRATOR_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_DEFRAG_ORCHESTRATOR_PC34_COMPAT_H

#include "memory_cache_segment_scan_pc34_compat.h"
#include "memory_cache_defrag_apply_pc34_compat.h"

struct MemoryCacheDefragOrchestratorResult_Compat {
    unsigned int segmentCount;
    unsigned int movedBlockCount;
    unsigned long compactedTopOffset;
};

int F0482_CACHE_DefragmentMini_Compat(
    const struct MemoryCacheRawBlockHeader_Compat* headers,
    unsigned int headerCount,
    struct MemoryCacheDefragSegment_Compat* segments,
    unsigned int maxSegments,
    unsigned short* nativeBitmapBlockIndices,
    unsigned short* derivedBitmapBlockIndices,
    unsigned long* blockOffsets,
    struct MemoryCacheUsageState_Compat* usageState,
    unsigned short nativeIndexCount,
    unsigned short derivedIndexCount,
    struct MemoryCacheDefragResultState_Compat* defragState,
    struct MemoryCacheDefragOrchestratorResult_Compat* outResult);

#endif
