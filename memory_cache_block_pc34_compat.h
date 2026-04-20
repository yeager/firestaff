#ifndef REDMCSB_MEMORY_CACHE_BLOCK_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_BLOCK_PC34_COMPAT_H

struct GraphicWidthHeight_Compat;
struct NativeBitmapBlock_Compat {
    unsigned short usageCount;
    unsigned short previousIndex;
    unsigned short nextIndex;
    unsigned short bitmapIndex;
    unsigned short width;
    unsigned short height;
    unsigned char bitmap[];
};

unsigned char* F0489_MEMORY_PrepareNativeBitmapBlock_Compat(struct NativeBitmapBlock_Compat* block, unsigned short bitmapIndex, const struct GraphicWidthHeight_Compat* sizeInfo);

#endif
