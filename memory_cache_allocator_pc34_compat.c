#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_allocator_pc34_compat.h"

int F0483_CACHE_GetNewBlock_Compat(
unsigned long                              requestedSize    SEPARATOR
unsigned long                              minimumSplitSize SEPARATOR
struct MemoryCacheAllocateOrReuseState_Compat* state        SEPARATOR
struct MemoryCacheUnusedBlock_Compat**     firstUnusedBlock SEPARATOR
struct MemoryCacheUnusedBlock_Compat*      splitRemainder   SEPARATOR
struct MemoryCacheAllocatorResult_Compat*  outResult        FINAL_SEPARATOR
{
        struct MemoryCacheDefragBranchResult_Compat branch;
        struct MemoryCacheAllocateOrReuseResult_Compat allocResult;


        branch = MEMORY_CACHE_CheckDefragBranch_Compat(
            requestedSize,
            state->cacheMemorySpan,
            *firstUnusedBlock);
        outResult->defragmentRequested = branch.shouldDefragment;
        if (branch.shouldDefragment) {
                if (state->cacheBytesAvailable < requestedSize) {
                        return 0;
                }
                outResult->blockOffset = state->cacheMemoryTopOffset;
                outResult->allocatedSize = requestedSize;
                outResult->allocatedFromTop = 1;
                outResult->reusedExistingBlock = 0;
                outResult->splitPerformed = 0;
                state->cacheMemoryTopOffset += requestedSize;
                state->cacheBytesAvailable -= requestedSize;
                state->cacheMemorySpan = 0;
                return 1;
        }
        if (!MEMORY_CACHE_AllocateOrReuseBlock_Compat(
                requestedSize,
                minimumSplitSize,
                state,
                firstUnusedBlock,
                splitRemainder,
                &allocResult)) {
                return 0;
        }
        outResult->blockOffset = allocResult.blockOffset;
        outResult->allocatedSize = allocResult.allocatedSize;
        outResult->allocatedFromTop = allocResult.allocatedFromTop;
        outResult->reusedExistingBlock = allocResult.reusedExistingBlock;
        outResult->splitPerformed = allocResult.splitPerformed;
        return 1;
}
