#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_load_transaction_pc34_compat.h"

void F0490_MEMORY_RunLoadGraphicTransaction_Compat(
int                                 graphicIndexFlags     SEPARATOR
const unsigned char*                loadedGraphic         SEPARATOR
unsigned long                       loadedByteCount       SEPARATOR
unsigned char*                      viewportGraphicBuffer SEPARATOR
unsigned char*                      destinationBitmap     SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo          SEPARATOR
struct MemoryLoadTransactionResult_Compat* outResult      FINAL_SEPARATOR
{
        outResult->musicStopped = 1;
        outResult->graphicsOpened = 1;
        F0490_MEMORY_LoadGraphicSession_Compat(
            graphicIndexFlags,
            loadedGraphic,
            loadedByteCount,
            viewportGraphicBuffer,
            destinationBitmap,
            sizeInfo,
            &outResult->session);
        outResult->graphicsClosed = 1;
}
