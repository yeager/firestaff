#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_allocation_boundary_pc34_compat.h"

unsigned long MEMORY_CACHE_NormalizeBlockSize_Compat(
unsigned long byteCount FINAL_SEPARATOR
{
        if ((byteCount & 1UL) != 0) {
                byteCount++;
        }
        return byteCount;
}

unsigned short MEMORY_CACHE_FindFirstFreeBlockSlot_Compat(
struct NativeBitmapBlock_Compat** blocks       SEPARATOR
int                               blockCapacity FINAL_SEPARATOR
{
        int i;


        for (i = 0; i < blockCapacity; i++) {
                if (blocks[i] == 0) {
                        return (unsigned short)i;
                }
        }
        return MEMORY_CACHE_ALLOCATION_NO_SLOT;
}

int MEMORY_CACHE_RegisterNativeBlock_Compat(
int                               graphicIndex             SEPARATOR
struct NativeBitmapBlock_Compat*   block                    SEPARATOR
unsigned short*                    nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**  blocks                   SEPARATOR
int                                blockCapacity            SEPARATOR
unsigned short*                    outBlockIndex            FINAL_SEPARATOR
{
        unsigned short blockIndex;


        blockIndex = MEMORY_CACHE_FindFirstFreeBlockSlot_Compat(blocks, blockCapacity);
        if (blockIndex == MEMORY_CACHE_ALLOCATION_NO_SLOT) {
                return 0;
        }
        blocks[blockIndex] = block;
        nativeBitmapBlockIndices[graphicIndex] = blockIndex;
        block->bitmapIndex = (unsigned short)graphicIndex;
        if (outBlockIndex != 0) {
                *outBlockIndex = blockIndex;
        }
        return 1;
}

int MEMORY_CACHE_RegisterDerivedBlock_Compat(
int                               derivedBitmapIndex        SEPARATOR
struct NativeBitmapBlock_Compat*   block                    SEPARATOR
unsigned short*                    derivedBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**  blocks                   SEPARATOR
int                                blockCapacity            SEPARATOR
unsigned short*                    outBlockIndex            FINAL_SEPARATOR
{
        unsigned short blockIndex;


        blockIndex = MEMORY_CACHE_FindFirstFreeBlockSlot_Compat(blocks, blockCapacity);
        if (blockIndex == MEMORY_CACHE_ALLOCATION_NO_SLOT) {
                return 0;
        }
        blocks[blockIndex] = block;
        derivedBitmapBlockIndices[derivedBitmapIndex] = blockIndex;
        block->bitmapIndex = (unsigned short)(derivedBitmapIndex | 0x8000);
        if (outBlockIndex != 0) {
                *outBlockIndex = blockIndex;
        }
        return 1;
}
