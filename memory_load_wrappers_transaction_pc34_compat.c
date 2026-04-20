#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_load_wrappers_transaction_pc34_compat.h"
#include "memory_load_wrappers_pc34_compat.h"

long F0440_STARTEND_LoadTemporaryGraphicTransaction_Compat(
int                                 graphicIndex                   SEPARATOR
const unsigned short*               languageSpecificGraphicIndices SEPARATOR
const unsigned short*               graphicDecompressedByteCounts  SEPARATOR
const unsigned char*                loadedGraphic                  SEPARATOR
unsigned char**                     outGraphic                     SEPARATOR
unsigned char*                      allocatedGraphic               SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo                   SEPARATOR
struct MemoryLoadTransactionResult_Compat* outResult               FINAL_SEPARATOR
{
        unsigned short byteCount;


        byteCount = F0494_MEMORY_GetGraphicDecompressedByteCount_Compat(
            graphicIndex,
            languageSpecificGraphicIndices,
            graphicDecompressedByteCounts);
        *outGraphic = allocatedGraphic;
        F0490_MEMORY_RunLoadGraphicTransaction_Compat(
            MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED | MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS,
            loadedGraphic,
            (unsigned long)byteCount,
            allocatedGraphic,
            allocatedGraphic,
            sizeInfo,
            outResult);
        return (long)byteCount;
}

unsigned char* F0763_LoadEndgameBitmapExpandedTransaction_Compat(
const unsigned char*                loadedGraphic    SEPARATOR
unsigned long                       loadedByteCount  SEPARATOR
unsigned char*                      allocatedBitmap  SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo     SEPARATOR
struct MemoryLoadTransactionResult_Compat* outResult FINAL_SEPARATOR
{
        F0490_MEMORY_RunLoadGraphicTransaction_Compat(
            0,
            loadedGraphic,
            loadedByteCount,
            allocatedBitmap,
            allocatedBitmap,
            sizeInfo,
            outResult);
        return allocatedBitmap;
}
