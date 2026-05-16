#ifndef REDMCSB_MEMORY_LOAD_TRANSACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_LOAD_TRANSACTION_PC34_COMPAT_H

#include "memory_load_session_pc34_compat.h"

struct GraphicWidthHeight_Compat;

struct MemoryLoadTransactionResult_Compat {
    struct MemoryLoadSessionResult_Compat session;
    int musicStopped;
    int graphicsOpened;
    int graphicsClosed;
};

void F0490_MEMORY_RunLoadGraphicTransaction_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* viewportGraphicBuffer,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo,
    struct MemoryLoadTransactionResult_Compat* outResult);

#endif
