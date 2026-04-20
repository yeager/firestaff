#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_BLOCK_INDEX_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_BLOCK_INDEX_PC34_COMPAT_H

#include "memory_graphics_dat_bitmap_reuse_pc34_compat.h"
#include "memory_cache_block_pc34_compat.h"

unsigned char* F0489_MEMORY_GetOrLoadBitmapByBlockIndex_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    unsigned char* loadedGraphicBuffer,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    struct MemoryGraphicsDatBitmapPathResult_Compat* outResult);

#endif
