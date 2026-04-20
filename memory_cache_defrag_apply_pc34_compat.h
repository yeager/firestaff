#ifndef REDMCSB_MEMORY_CACHE_DEFRAG_APPLY_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_DEFRAG_APPLY_PC34_COMPAT_H

#include "memory_cache_defrag_loop_pc34_compat.h"
#include "memory_cache_defrag_result_pc34_compat.h"

struct MemoryCacheDefragApplyResult_Compat {
    unsigned long compactedTopOffset;
    unsigned int movedBlockCount;
};

int F0482_CACHE_Defragment_Compat(
    const struct MemoryCacheDefragSegment_Compat* segments,
    unsigned int segmentCount,
    unsigned short* nativeBitmapBlockIndices,
    unsigned short* derivedBitmapBlockIndices,
    unsigned long* blockOffsets,
    struct MemoryCacheUsageState_Compat* usageState,
    unsigned short nativeIndexCount,
    unsigned short derivedIndexCount,
    struct MemoryCacheDefragResultState_Compat* defragState,
    struct MemoryCacheDefragApplyResult_Compat* outResult);

#endif
