#ifndef REDMCSB_MEMORY_CACHE_GATED_ACCESS_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_GATED_ACCESS_PC34_COMPAT_H

#include "memory_cache_access_pc34_compat.h"
#include "memory_cache_timegate_pc34_compat.h"

struct GraphicWidthHeight_Compat;

unsigned char* F0489_MEMORY_GetNativeBitmapByIndexTimeGated_Compat(
    unsigned long gameTime,
    unsigned long* lastResetGameTime,
    int graphicIndex,
    const unsigned char* graphic,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState,
    const struct GraphicWidthHeight_Compat* sizeInfo);

int F0491_CACHE_IsDerivedBitmapInCacheTimeGated_Compat(
    unsigned long gameTime,
    unsigned long* lastResetGameTime,
    int derivedBitmapIndex,
    unsigned short* derivedBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState);

#endif
