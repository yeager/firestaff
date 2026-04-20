#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_COMPOSED_TRANSACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_COMPOSED_TRANSACTION_PC34_COMPAT_H

#include "memory_graphics_dat_select_pc34_compat.h"
#include "memory_graphics_dat_transaction_pc34_compat.h"

int F0490_MEMORY_RunSelectedGraphicsDatTransaction_Compat(
    const char* graphicsDatPath,
    const struct MemoryGraphicsDatHeader_Compat* header,
    unsigned int graphicIndex,
    int graphicIndexFlags,
    struct MemoryGraphicsDatState_Compat* state,
    unsigned char* viewportGraphicBuffer,
    unsigned char* destinationBitmap,
    struct MemoryGraphicsDatTransactionResult_Compat* outResult,
    struct MemoryGraphicsDatSelection_Compat* outSelection);

#endif
