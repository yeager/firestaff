#ifndef REDMCSB_MEMORY_CACHE_ALLOCATOR_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_ALLOCATOR_PC34_COMPAT_H

#include "memory_cache_defrag_branch_pc34_compat.h"

struct MemoryCacheAllocatorResult_Compat {
    unsigned long blockOffset;
    unsigned long allocatedSize;
    int allocatedFromTop;
    int reusedExistingBlock;
    int splitPerformed;
    int defragmentRequested;
};

int F0483_CACHE_GetNewBlock_Compat(
    unsigned long requestedSize,
    unsigned long minimumSplitSize,
    struct MemoryCacheAllocateOrReuseState_Compat* state,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryCacheAllocatorResult_Compat* outResult);

#endif
