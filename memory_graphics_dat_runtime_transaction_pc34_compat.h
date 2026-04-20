#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_RUNTIME_TRANSACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_RUNTIME_TRANSACTION_PC34_COMPAT_H

#include "memory_graphics_dat_state_pc34_compat.h"
#include "memory_graphics_dat_composed_transaction_pc34_compat.h"

int F0490_MEMORY_RunRuntimeGraphicsDatTransaction_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState,
    unsigned int graphicIndex,
    int graphicIndexFlags,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned char* viewportGraphicBuffer,
    unsigned char* destinationBitmap,
    struct MemoryGraphicsDatTransactionResult_Compat* outResult,
    struct MemoryGraphicsDatSelection_Compat* outSelection);

#endif
