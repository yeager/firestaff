#ifndef REDMCSB_MEMORY_LOAD_PIPELINE_PC34_COMPAT_H
#define REDMCSB_MEMORY_LOAD_PIPELINE_PC34_COMPAT_H

#include "memory_load_wrappers_transaction_pc34_compat.h"

#define MEMORY_LOAD_PIPELINE_MODE_TEMPORARY 1
#define MEMORY_LOAD_PIPELINE_MODE_ENDGAME_BITMAP 2

struct MemoryLoadPipelineResult_Compat {
    long byteCount;
    unsigned char* output;
    struct MemoryLoadTransactionResult_Compat transaction;
};

int MEMORY_LOAD_RunPipeline_Compat(
    int mode,
    int graphicIndex,
    const unsigned short* languageSpecificGraphicIndices,
    const unsigned short* graphicDecompressedByteCounts,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char** outGraphic,
    unsigned char* allocatedBuffer,
    const struct GraphicWidthHeight_Compat* sizeInfo,
    struct MemoryLoadPipelineResult_Compat* outResult);

#endif
