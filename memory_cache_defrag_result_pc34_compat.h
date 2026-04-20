#ifndef REDMCSB_MEMORY_CACHE_DEFRAG_RESULT_PC34_COMPAT_H
#define REDMCSB_MEMORY_CACHE_DEFRAG_RESULT_PC34_COMPAT_H

#include "memory_cache_unused_blocks_pc34_compat.h"
#include "memory_cache_usage_pc34_compat.h"

struct MemoryCacheDefragResultState_Compat {
    unsigned long cacheMemoryTopOffset;
    struct MemoryCacheUnusedBlock_Compat* firstUnusedBlock;
};

void MEMORY_CACHE_ApplyDefragResult_Compat(
    unsigned long compactedTopOffset,
    struct MemoryCacheDefragResultState_Compat* state);

#endif
