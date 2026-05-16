#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_BITMAP_REUSE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_BITMAP_REUSE_PC34_COMPAT_H

#include "memory_graphics_dat_bitmap_path_pc34_compat.h"
#include "memory_cache_reuse_pc34_compat.h"

unsigned char* F0489_MEMORY_GetOrLoadNativeBitmapByIndex_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    unsigned char* loadedGraphicBuffer,
    unsigned char* cachedBitmap,
    unsigned char* targetBitmap,
    struct MemoryGraphicsDatBitmapPathResult_Compat* outResult);

#endif
