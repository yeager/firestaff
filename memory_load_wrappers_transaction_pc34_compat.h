#ifndef REDMCSB_MEMORY_LOAD_WRAPPERS_TRANSACTION_PC34_COMPAT_H
#define REDMCSB_MEMORY_LOAD_WRAPPERS_TRANSACTION_PC34_COMPAT_H

#include "memory_load_transaction_pc34_compat.h"

long F0440_STARTEND_LoadTemporaryGraphicTransaction_Compat(
    int graphicIndex,
    const unsigned short* languageSpecificGraphicIndices,
    const unsigned short* graphicDecompressedByteCounts,
    const unsigned char* loadedGraphic,
    unsigned char** outGraphic,
    unsigned char* allocatedGraphic,
    const struct GraphicWidthHeight_Compat* sizeInfo,
    struct MemoryLoadTransactionResult_Compat* outResult);

unsigned char* F0763_LoadEndgameBitmapExpandedTransaction_Compat(
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* allocatedBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo,
    struct MemoryLoadTransactionResult_Compat* outResult);

#endif
