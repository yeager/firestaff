#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_defrag_branch_pc34_compat.h"

struct MemoryCacheDefragBranchResult_Compat MEMORY_CACHE_CheckDefragBranch_Compat(
unsigned long                         requestedSize    SEPARATOR
unsigned long                         cacheMemorySpan  SEPARATOR
struct MemoryCacheUnusedBlock_Compat* firstUnusedBlock FINAL_SEPARATOR
{
        struct MemoryCacheDefragBranchResult_Compat result;


        result.shouldDefragment = 0;
        result.shouldAllocateFromTopAfterDefrag = 0;
        result.shouldReuseUnusedBlock = 0;
        if (cacheMemorySpan >= requestedSize) {
                result.shouldAllocateFromTopAfterDefrag = 1;
                return result;
        }
        if (firstUnusedBlock == 0) {
                return result;
        }
        if (firstUnusedBlock->blockSize < requestedSize) {
                result.shouldDefragment = 1;
                result.shouldAllocateFromTopAfterDefrag = 1;
                return result;
        }
        result.shouldReuseUnusedBlock = 1;
        return result;
}
