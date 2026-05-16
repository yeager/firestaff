#ifndef REDMCSB_MEMORY_CACHE_USAGE_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_USAGE_PC34_COMPAT_H

#include "memory_cache_block_pc34_compat.h"

#define MEMORY_CACHE_INDEX_NONE 0xFFFF

struct MemoryCacheUsageState_Compat {
    struct NativeBitmapBlock_Compat* firstUsedBlock;
    struct NativeBitmapBlock_Compat* lastUsedBlock;
    struct NativeBitmapBlock_Compat* firstReferencedUsedBlock;
};

void F0485_CACHE_ResetUsageCounts_Compat(
    struct MemoryCacheUsageState_Compat* state,
    struct NativeBitmapBlock_Compat** blocks);

void F0486_MEMORY_AddBlockToUsedList_Compat(
    unsigned short blockIndex,
    struct MemoryCacheUsageState_Compat* state,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity);

struct NativeBitmapBlock_Compat* F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(
    unsigned short blockIndex,
    struct MemoryCacheUsageState_Compat* state,
    struct NativeBitmapBlock_Compat** blocks,
    int blockCapacity);

#endif
