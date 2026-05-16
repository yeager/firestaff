#ifndef REDMCSB_DERIVED_BITMAP_CACHE_PC34_COMPAT_H
#define REDMCSB_DERIVED_BITMAP_CACHE_PC34_COMPAT_H

struct NativeBitmapBlock_Compat;
int F0491_CACHE_IsDerivedBitmapInCache_Compat(
    int derivedBitmapIndex,
    unsigned short* derivedBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    struct NativeBitmapBlock_Compat* newBlock);

unsigned char* F0492_CACHE_GetDerivedBitmap_Compat(
    int derivedBitmapIndex,
    unsigned short* derivedBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks);

#endif
