#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_load_wrappers_pc34_compat.h"
#include "memory_load_expand_pc34_compat.h"

unsigned short F0494_MEMORY_GetGraphicDecompressedByteCount_Compat(
int                   graphicIndex                   SEPARATOR
const unsigned short* languageSpecificGraphicIndices SEPARATOR
const unsigned short* graphicDecompressedByteCounts  FINAL_SEPARATOR
{
        if (languageSpecificGraphicIndices != 0) {
                graphicIndex = languageSpecificGraphicIndices[graphicIndex];
        }
        return graphicDecompressedByteCounts[graphicIndex];
}

long F0440_STARTEND_LoadTemporaryGraphic_Compat(
int                                 graphicIndex                   SEPARATOR
const unsigned short*               languageSpecificGraphicIndices SEPARATOR
const unsigned short*               graphicDecompressedByteCounts  SEPARATOR
const unsigned char*                loadedGraphic                  SEPARATOR
unsigned char**                     outGraphic                     SEPARATOR
unsigned char*                      allocatedGraphic               SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo                   FINAL_SEPARATOR
{
        unsigned short byteCount;


        byteCount = F0494_MEMORY_GetGraphicDecompressedByteCount_Compat(
            graphicIndex,
            languageSpecificGraphicIndices,
            graphicDecompressedByteCounts);
        *outGraphic = allocatedGraphic;
        F0490_MEMORY_ApplyLoadedGraphic_Compat(
            graphicIndex | MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED | MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS,
            loadedGraphic,
            (unsigned long)byteCount,
            allocatedGraphic,
            sizeInfo);
        return (long)byteCount;
}

unsigned char* F0763_LoadEndgameBitmapExpanded_Compat(
const unsigned char*                loadedGraphic    SEPARATOR
unsigned long                       loadedByteCount  SEPARATOR
unsigned char*                      allocatedBitmap  SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo     FINAL_SEPARATOR
{
        F0490_MEMORY_ApplyLoadedGraphic_Compat(0, loadedGraphic, loadedByteCount, allocatedBitmap, sizeInfo);
        return allocatedBitmap;
}
