#ifndef REDMCSB_MEMORY_NATIVE_OR_GRAPHIC_PC34_COMPAT_H
#define REDMCSB_MEMORY_NATIVE_OR_GRAPHIC_PC34_COMPAT_H

#include "memory_cache_gated_access_pc34_compat.h"

#define MEMORY_NATIVE_OR_GRAPHIC_FLAG_NOT_EXPANDED 0x8000
#define MEMORY_NATIVE_OR_GRAPHIC_INDEX_MASK 0x7FFF

struct GraphicWidthHeight_Compat;

unsigned char* F0489_MEMORY_GetNativeBitmapOrGraphicTimeGated_Compat(
    unsigned long gameTime,
    unsigned long* lastResetGameTime,
    int nativeBitmapOrGraphicIndex,
    const unsigned char** graphics,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryCacheUsageState_Compat* usageState,
    const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
