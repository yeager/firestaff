#ifndef REDMCSB_MEMORY_LOAD_WRAPPERS_PC34_COMPAT_H
#define REDMCSB_MEMORY_LOAD_WRAPPERS_PC34_COMPAT_H

struct GraphicWidthHeight_Compat;

unsigned short F0494_MEMORY_GetGraphicDecompressedByteCount_Compat(
    int graphicIndex,
    const unsigned short* languageSpecificGraphicIndices,
    const unsigned short* graphicDecompressedByteCounts);

long F0440_STARTEND_LoadTemporaryGraphic_Compat(
    int graphicIndex,
    const unsigned short* languageSpecificGraphicIndices,
    const unsigned short* graphicDecompressedByteCounts,
    const unsigned char* loadedGraphic,
    unsigned char** outGraphic,
    unsigned char* allocatedGraphic,
    const struct GraphicWidthHeight_Compat* sizeInfo);

unsigned char* F0763_LoadEndgameBitmapExpanded_Compat(
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* allocatedBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
