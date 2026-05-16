#ifndef REDMCSB_MEMORY_CACHE_ALLOCATION_BOUNDARY_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_ALLOCATION_BOUNDARY_PC34_COMPAT_H

#include "memory_cache_block_pc34_compat.h"

#define MEMORY_CACHE_ALLOCATION_NO_SLOT 0xFFFF

unsigned long MEMORY_CACHE_NormalizeBlockSize_Compat(unsigned long byteCount);

unsigned short MEMORY_CACHE_FindFirstFreeBlockSlot_Compat(
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity);

int MEMORY_CACHE_RegisterNativeBlock_Compat(
    int graphicIndex,
    struct NativeBitmapBlock_Compat* block,
    unsigned short* nativeBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    unsigned short* outBlockIndex);

int MEMORY_CACHE_RegisterDerivedBlock_Compat(
    int derivedBitmapIndex,
    struct NativeBitmapBlock_Compat* block,
    unsigned short* derivedBitmapBlockIndices,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity,
    unsigned short* outBlockIndex);

#endif
