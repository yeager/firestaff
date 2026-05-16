#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_defrag_apply_pc34_compat.h"

int F0482_CACHE_Defragment_Compat(
const struct MemoryCacheDefragSegment_Compat* segments        SEPARATOR
unsigned int                                 segmentCount     SEPARATOR
unsigned short*                              nativeBitmapBlockIndices SEPARATOR
unsigned short*                              derivedBitmapBlockIndices SEPARATOR
unsigned long*                               blockOffsets     SEPARATOR
struct MemoryCacheUsageState_Compat*         usageState       SEPARATOR
unsigned short                               nativeIndexCount SEPARATOR
unsigned short                               derivedIndexCount SEPARATOR
struct MemoryCacheDefragResultState_Compat*  defragState      SEPARATOR
struct MemoryCacheDefragApplyResult_Compat*  outResult        FINAL_SEPARATOR
{
        struct MemoryCacheDefragLoopResult_Compat loopResult;


        if (defragState->firstUnusedBlock == 0) {
                outResult->compactedTopOffset = defragState->cacheMemoryTopOffset;
                outResult->movedBlockCount = 0;
                return 1;
        }
        if (!MEMORY_CACHE_RunDefragLoop_Compat(
                segments,
                segmentCount,
                nativeBitmapBlockIndices,
                derivedBitmapBlockIndices,
                blockOffsets,
                usageState,
                nativeIndexCount,
                derivedIndexCount,
                &loopResult)) {
                return 0;
        }
        MEMORY_CACHE_ApplyDefragResult_Compat(loopResult.compactedTopOffset, defragState);
        outResult->compactedTopOffset = loopResult.compactedTopOffset;
        outResult->movedBlockCount = loopResult.movedBlockCount;
        return 1;
}
