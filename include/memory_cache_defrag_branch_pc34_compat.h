#ifndef REDMCSB_MEMORY_CACHE_DEFRAG_BRANCH_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_DEFRAG_BRANCH_PC34_COMPAT_H

#include "memory_cache_allocate_or_reuse_pc34_compat.h"
#include "memory_cache_unused_blocks_pc34_compat.h"

struct MemoryCacheDefragBranchResult_Compat {
    int shouldDefragment;
    int shouldAllocateFromTopAfterDefrag;
    int shouldReuseUnusedBlock;
};

struct MemoryCacheDefragBranchResult_Compat MEMORY_CACHE_CheckDefragBranch_Compat(
    unsigned long requestedSize,
    unsigned long cacheMemorySpan,
    struct MemoryCacheUnusedBlock_Compat* firstUnusedBlock);

#endif
