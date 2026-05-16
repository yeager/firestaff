#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_access_pc34_compat.h"
#include "memory_cache_index_pc34_compat.h"
#include "derived_bitmap_cache_pc34_compat.h"

unsigned char* F0489_MEMORY_GetNativeBitmapByIndexAndTouch_Compat(
int                                     graphicIndex             SEPARATOR
const unsigned char*                    graphic                  SEPARATOR
unsigned short*                         nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**       blocks                   SEPARATOR
int                                     blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*        newBlock                 SEPARATOR
struct MemoryCacheUsageState_Compat*    usageState               SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo                FINAL_SEPARATOR
{
        unsigned short blockIndex;
        struct NativeBitmapBlock_Compat* block;


        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if (blockIndex != MEMORY_CACHE_INDEX_NONE && blockIndex < (unsigned short)blockCapacity && blocks[blockIndex] != 0) {
                block = F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(blockIndex, usageState, blocks, blockCapacity);
                return block->bitmap;
        }
        if (F0489_MEMORY_GetNativeBitmapByIndex_Compat(
            graphicIndex,
            graphic,
            nativeBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock,
            sizeInfo) == 0) {
                return 0;
        }
        F0486_MEMORY_AddBlockToUsedList_Compat(nativeBitmapBlockIndices[graphicIndex], usageState, blocks, blockCapacity);
        return blocks[nativeBitmapBlockIndices[graphicIndex]]->bitmap;
}

int F0491_CACHE_IsDerivedBitmapInCacheAndTouch_Compat(
int                                  derivedBitmapIndex        SEPARATOR
unsigned short*                      derivedBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**    blocks                    SEPARATOR
int                                  blockCapacity             SEPARATOR
struct NativeBitmapBlock_Compat*     newBlock                  SEPARATOR
struct MemoryCacheUsageState_Compat* usageState                FINAL_SEPARATOR
{
        unsigned short blockIndex;


        blockIndex = derivedBitmapBlockIndices[derivedBitmapIndex];
        if (blockIndex != MEMORY_CACHE_INDEX_NONE && blockIndex < (unsigned short)blockCapacity && blocks[blockIndex] != 0) {
                F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(blockIndex, usageState, blocks, blockCapacity);
                return 1;
        }
        return F0491_CACHE_IsDerivedBitmapInCache_Compat(
            derivedBitmapIndex,
            derivedBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock);
}

void F0493_CACHE_AddDerivedBitmap_Compat(
int                                  derivedBitmapIndex        SEPARATOR
unsigned short*                      derivedBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**    blocks                    SEPARATOR
int                                  blockCapacity             SEPARATOR
struct MemoryCacheUsageState_Compat* usageState                FINAL_SEPARATOR
{
        unsigned short blockIndex;


        blockIndex = derivedBitmapBlockIndices[derivedBitmapIndex];
        if (blockIndex != MEMORY_CACHE_INDEX_NONE && blockIndex < (unsigned short)blockCapacity && blocks[blockIndex] != 0) {
                F0486_MEMORY_AddBlockToUsedList_Compat(blockIndex, usageState, blocks, blockCapacity);
        }
}
