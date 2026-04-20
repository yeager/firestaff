#ifndef REDMCSB_MEMORY_CACHE_ALLOCATE_OR_REUSE_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_ALLOCATE_OR_REUSE_PC34_COMPAT_H

#include "memory_cache_reuse_split_pc34_compat.h"

struct MemoryCacheAllocateOrReuseState_Compat {
    unsigned long cacheBytesAvailable;
    unsigned long cacheMemorySpan;
    unsigned long cacheMemoryTopOffset;
};

struct MemoryCacheAllocateOrReuseResult_Compat {
    unsigned long blockOffset;
    unsigned long allocatedSize;
    int allocatedFromTop;
    int reusedExistingBlock;
    int splitPerformed;
};

int MEMORY_CACHE_AllocateOrReuseBlock_Compat(
    unsigned long requestedSize,
    unsigned long minimumSplitSize,
    struct MemoryCacheAllocateOrReuseState_Compat* state,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryCacheAllocateOrReuseResult_Compat* outResult);

#endif
