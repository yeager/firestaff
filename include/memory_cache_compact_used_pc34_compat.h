#ifndef REDMCSB_MEMORY_CACHE_COMPACT_USED_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_COMPACT_USED_PC34_COMPAT_H

#include "memory_cache_usage_pc34_compat.h"

struct MemoryCacheCompactUsedResult_Compat {
    unsigned short blockIndex;
    unsigned long movedFromOffset;
    unsigned long movedToOffset;
    int moved;
};

int MEMORY_CACHE_CompactUsedBlock_Compat(
    int bitmapIndex,
    int isDerivedBitmap,
    unsigned long fromOffset,
    unsigned long toOffset,
    unsigned short* nativeBitmapBlockIndices,
    unsigned short* derivedBitmapBlockIndices,
    unsigned long* blockOffsets,
    struct MemoryCacheUsageState_Compat* usageState,
    unsigned short nativeIndexCount,
    unsigned short derivedIndexCount,
    struct MemoryCacheCompactUsedResult_Compat* outResult);

#endif
