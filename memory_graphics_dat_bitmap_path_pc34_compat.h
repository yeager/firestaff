#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_BITMAP_PATH_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_BITMAP_PATH_PC34_COMPAT_H

#include "memory_graphics_dat_viewport_path_pc34_compat.h"
#include "memory_cache_frontend_pc34_compat.h"

struct MemoryGraphicsDatBitmapPathResult_Compat {
    struct MemoryGraphicsDatTransactionResult_Compat transaction;
    struct MemoryGraphicsDatSelection_Compat selection;
    unsigned char* bitmap;
    unsigned char* loadedGraphic;
};

unsigned char* F0489_MEMORY_LoadNativeBitmapByIndex_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    unsigned char* loadedGraphicBuffer,
    unsigned char* ownedBitmap,
    struct MemoryGraphicsDatBitmapPathResult_Compat* outResult);

#endif
