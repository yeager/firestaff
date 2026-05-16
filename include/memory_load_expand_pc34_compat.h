#ifndef REDMCSB_MEMORY_LOAD_EXPAND_PC34_COMPAT_H
#define REDMCSB_MEMORY_LOAD_EXPAND_PC34_COMPAT_H

struct GraphicWidthHeight_Compat;

#define MEMORY_LOAD_EXPAND_FLAG_NOT_EXPANDED 0x8000
#define MEMORY_LOAD_EXPAND_FLAG_DO_NOT_COPY_DIMENSIONS 0x4000

void F0490_MEMORY_ApplyLoadedGraphic_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
