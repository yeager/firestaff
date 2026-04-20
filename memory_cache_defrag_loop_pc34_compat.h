#ifndef REDMCSB_MEMORY_CACHE_DEFRAG_LOOP_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_DEFRAG_LOOP_PC34_COMPAT_H

#include "memory_cache_compact_used_pc34_compat.h"
#include "memory_cache_defrag_result_pc34_compat.h"

struct MemoryCacheDefragSegment_Compat {
    int isUsed;
    int bitmapIndex;
    int isDerivedBitmap;
    unsigned long blockSize;
};

struct MemoryCacheDefragLoopResult_Compat {
    unsigned long compactedTopOffset;
    unsigned int movedBlockCount;
};

int MEMORY_CACHE_RunDefragLoop_Compat(
    const struct MemoryCacheDefragSegment_Compat* segments,
    unsigned int segmentCount,
    unsigned short* nativeBitmapBlockIndices,
    unsigned short* derivedBitmapBlockIndices,
    unsigned long* blockOffsets,
    struct MemoryCacheUsageState_Compat* usageState,
    unsigned short nativeIndexCount,
    unsigned short derivedIndexCount,
    struct MemoryCacheDefragLoopResult_Compat* outResult);

#endif
