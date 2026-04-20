#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_allocate_or_reuse_pc34_compat.h"

int MEMORY_CACHE_AllocateOrReuseBlock_Compat(
unsigned long                              requestedSize    SEPARATOR
unsigned long                              minimumSplitSize SEPARATOR
struct MemoryCacheAllocateOrReuseState_Compat* state        SEPARATOR
struct MemoryCacheUnusedBlock_Compat**     firstUnusedBlock SEPARATOR
struct MemoryCacheUnusedBlock_Compat*      splitRemainder   SEPARATOR
struct MemoryCacheAllocateOrReuseResult_Compat* outResult   FINAL_SEPARATOR
{
        struct MemoryCacheReuseSplitResult_Compat reuseResult;


        if (state->cacheMemorySpan >= requestedSize) {
                outResult->blockOffset = state->cacheMemoryTopOffset;
                outResult->allocatedSize = requestedSize;
                outResult->allocatedFromTop = 1;
                outResult->reusedExistingBlock = 0;
                outResult->splitPerformed = 0;
                state->cacheMemoryTopOffset += requestedSize;
        } else if (!MEMORY_CACHE_ReuseOrSplitUnusedBlock_Compat(
                requestedSize,
                minimumSplitSize,
                firstUnusedBlock,
                splitRemainder,
                &reuseResult)) {
                return 0;
        } else {
                outResult->blockOffset = 0;
                outResult->allocatedSize = reuseResult.allocatedSize;
                outResult->allocatedFromTop = 0;
                outResult->reusedExistingBlock = reuseResult.reusedExistingBlock;
                outResult->splitPerformed = reuseResult.splitPerformed;
        }
        state->cacheBytesAvailable -= outResult->allocatedSize;
        if (state->cacheMemorySpan >= requestedSize) {
                state->cacheMemorySpan -= requestedSize;
        }
        return 1;
}
