#ifndef REDMCSB_MEMORY_CACHE_REUSE_SPLIT_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_REUSE_SPLIT_PC34_COMPAT_H

#include "memory_cache_unused_blocks_pc34_compat.h"

struct MemoryCacheReuseSplitResult_Compat {
    struct MemoryCacheUnusedBlock_Compat* selectedBlock;
    unsigned long allocatedSize;
    int reusedExistingBlock;
    int exactFit;
    int splitPerformed;
};

int MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(
    unsigned long requestedSize,
    unsigned long minimumSplitSize,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryCacheReuseSplitResult_Compat* outResult);

#endif
