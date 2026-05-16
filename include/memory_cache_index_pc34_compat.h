#ifndef REDMCSB_MEMORY_CACHE_INDEX_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_INDEX_PC34_COMPAT_H

struct NativeBitmapBlock_Compat;
struct GraphicWidthHeight_Compat;

unsigned char* F0489_MEMORY_GetNativeBitmapByIndex_Compat(
    int graphicIndex,
    const unsigned char* graphic,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock,
    const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
