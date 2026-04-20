#ifndef REDMCSB_MEMORY_CACHE_ACCESS_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_ACCESS_PC34_COMPAT_H

#include "memory_cache_block_pc34_compat.h"
#include "memory_cache_usage_pc34_compat.h"

struct GraphicWidthHeight_Compat;

unsigned char* F0489_MEMORY_GetNativeBitmapByIndexAndTouch_Compat(
    int graphicIndex,
    const unsigned char* graphic,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState,
    const struct GraphicWidthHeight_Compat* sizeInfo);

int F0491_CACHE_IsDerivedBitmapInCacheAndTouch_Compat(
    int derivedBitmapIndex,
    unsigned short* derivedBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState);

void F0493_CACHE_AddDerivedBitmap_Compat(
    int derivedBitmapIndex,
    unsigned short* derivedBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct MemoryCacheUsageState_Compat* usageState);

#endif
