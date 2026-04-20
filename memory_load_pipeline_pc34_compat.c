#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_load_pipeline_pc34_compat.h"

int MEMORY_LOAD_RunPipeline_Compat(
int                                 mode                          SEPARATOR
int                                 graphicIndex                  SEPARATOR
const unsigned short*               languageSpecificGraphicIndices SEPARATOR
const unsigned short*               graphicDecompressedByteCounts  SEPARATOR
const unsigned char*                loadedGraphic                  SEPARATOR
unsigned long                       loadedByteCount                SEPARATOR
unsigned char**                     outGraphic                     SEPARATOR
unsigned char*                      allocatedBuffer                SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo                   SEPARATOR
struct MemoryLoadPipelineResult_Compat* outResult                  FINAL_SEPARATOR
{
        if (mode == MEMORY_LOAD_PIPELINE_MODE_TEMPORARY) {
                outResult->byteCount = F0440_STARTEND_LoadTemporaryGraphicTransaction_Compat(
                    graphicIndex,
                    languageSpecificGraphicIndices,
                    graphicDecompressedByteCounts,
                    loadedGraphic,
                    outGraphic,
                    allocatedBuffer,
                    sizeInfo,
                    &outResult->transaction);
                outResult->output = *outGraphic;
                return 1;
        }
        if (mode == MEMORY_LOAD_PIPELINE_MODE_ENDGAME_BITMAP) {
                outResult->output = F0763_LoadEndgameBitmapExpandedTransaction_Compat(
                    loadedGraphic,
                    loadedByteCount,
                    allocatedBuffer,
                    sizeInfo,
                    &outResult->transaction);
                outResult->byteCount = (long)loadedByteCount;
                if (outGraphic != 0) {
                        *outGraphic = outResult->output;
                }
                return 1;
        }
        return 0;
}
