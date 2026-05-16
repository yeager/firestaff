#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_TRANSACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_TRANSACTION_PC34_COMPAT_H

#include "memory_graphics_dat_pc34_compat.h"
#include "memory_load_session_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

struct MemoryGraphicsDatTransactionResult_Compat {
    struct MemoryLoadSessionResult_Compat session;
    int graphicsOpened;
    int graphicsClosed;
};

int F0490_MEMORY_RunGraphicsDatTransaction_Compat(
    const char* graphicsDatPath,
    long graphicOffset,
    int compressedByteCount,
    int graphicIndexFlags,
    struct MemoryGraphicsDatState_Compat* state,
    unsigned char* viewportGraphicBuffer,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo,
    struct MemoryGraphicsDatTransactionResult_Compat* outResult);

#endif
