#ifndef REDMCSB_MEMORY_CACHE_ALLOCATOR_DEFRAG_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_ALLOCATOR_DEFRAG_PC34_COMPAT_H

#include "memory_cache_allocator_pc34_compat.h"
#include "memory_cache_defrag_result_pc34_compat.h"

int F0483_CACHE_GetNewBlockWithDefrag_Compat(
    unsigned long requestedSize,
    unsigned long minimumSplitSize,
    unsigned long compactedTopOffset,
    struct MemoryCacheAllocateOrReuseState_Compat* allocatorState,
    struct MemoryCacheDefragResultState_Compat* defragState,
    struct MemoryCacheUnusedBlock_Compat** firstUnusedBlock,
    struct MemoryCacheUnusedBlock_Compat* splitRemainder,
    struct MemoryCacheAllocatorResult_Compat* outResult);

#endif
