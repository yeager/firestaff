#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "derived_bitmap_cache_pc34_compat.h"
#include "memory_cache_block_pc34_compat.h"

int F0491_CACHE_IsDerivedBitmapInCache_Compat(
int                                    derivedBitmapIndex       SEPARATOR
unsigned short*                        derivedBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**      blocks                   SEPARATOR
int                                    blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*       newBlock                 FINAL_SEPARATOR
{
        unsigned short blockIndex;
        int i;


        blockIndex = derivedBitmapBlockIndices[derivedBitmapIndex];
        if (blockIndex != 0xFFFF && blockIndex < (unsigned short)blockCapacity && blocks[blockIndex] != 0) {
                return 1;
        }
        for (i = 0; i < blockCapacity; i++) {
                if (blocks[i] == 0) {
                        blocks[i] = newBlock;
                        derivedBitmapBlockIndices[derivedBitmapIndex] = (unsigned short)i;
                        newBlock->bitmapIndex = (unsigned short)(derivedBitmapIndex | 0x8000);
                        newBlock->width = 0;
                        newBlock->height = 0;
                        return 0;
                }
        }
        return 0;
}

unsigned char* F0492_CACHE_GetDerivedBitmap_Compat(
int                               derivedBitmapIndex       SEPARATOR
unsigned short*                   derivedBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat** blocks FINAL_SEPARATOR
{
        return blocks[derivedBitmapBlockIndices[derivedBitmapIndex]]->bitmap;
}
