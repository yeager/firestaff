#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_allocator_defrag_pc34_compat.h"

int F0483_CACHE_GetNewBlockWithDefrag_Compat(
unsigned long                              requestedSize     SEPARATOR
unsigned long                              minimumSplitSize  SEPARATOR
unsigned long                              compactedTopOffset SEPARATOR
struct MemoryCacheAllocateOrReuseState_Compat* allocatorState SEPARATOR
struct MemoryCacheDefragResultState_Compat* defragState      SEPARATOR
struct MemoryCacheUnusedBlock_Compat**     firstUnusedBlock  SEPARATOR
struct MemoryCacheUnusedBlock_Compat*      splitRemainder    SEPARATOR
struct MemoryCacheAllocatorResult_Compat*  outResult         FINAL_SEPARATOR
{
        if (!F0483_CACHE_GetNewBlock_Compat(
                requestedSize,
                minimumSplitSize,
                allocatorState,
                firstUnusedBlock,
                splitRemainder,
                outResult)) {
                return 0;
        }
        if (outResult->defragmentRequested) {
                MEMORY_CACHE_ApplyDefragResult_Compat(compactedTopOffset, defragState);
                allocatorState->cacheMemoryTopOffset = defragState->cacheMemoryTopOffset;
                *firstUnusedBlock = defragState->firstUnusedBlock;
        }
        return 1;
}
