#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_VIEWPORT_PATH_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_VIEWPORT_PATH_PC34_COMPAT_H

#include "memory_graphics_dat_runtime_transaction_pc34_compat.h"

int F0490_MEMORY_LoadViewportGraphicByIndex_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int graphicIndex,
    int graphicIndexFlags,
    unsigned char* viewportGraphicBuffer,
    unsigned char* destinationBitmap,
    struct MemoryGraphicsDatTransactionResult_Compat* outResult,
    struct MemoryGraphicsDatSelection_Compat* outSelection);

#endif
