#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_defrag_loop_pc34_compat.h"

int MEMORY_CACHE_RunDefragLoop_Compat(
const struct MemoryCacheDefragSegment_Compat* segments          SEPARATOR
unsigned int                                 segmentCount       SEPARATOR
unsigned short*                              nativeBitmapBlockIndices SEPARATOR
unsigned short*                              derivedBitmapBlockIndices SEPARATOR
unsigned long*                               blockOffsets       SEPARATOR
struct MemoryCacheUsageState_Compat*         usageState         SEPARATOR
unsigned short                               nativeIndexCount   SEPARATOR
unsigned short                               derivedIndexCount  SEPARATOR
struct MemoryCacheDefragLoopResult_Compat*   outResult          FINAL_SEPARATOR
{
        unsigned long sourceOffset;
        unsigned long targetOffset;
        unsigned int i;
        struct MemoryCacheCompactUsedResult_Compat compactResult;


        sourceOffset = 0;
        targetOffset = 0;
        outResult->movedBlockCount = 0;
        for (i = 0; i < segmentCount; i++) {
                if (!segments[i].isUsed) {
                        sourceOffset += segments[i].blockSize;
                        continue;
                }
                if (!MEMORY_CACHE_CompactUsedBlock_Compat(
                        segments[i].bitmapIndex,
                        segments[i].isDerivedBitmap,
                        sourceOffset,
                        targetOffset,
                        nativeBitmapBlockIndices,
                        derivedBitmapBlockIndices,
                        blockOffsets,
                        usageState,
                        nativeIndexCount,
                        derivedIndexCount,
                        &compactResult)) {
                        return 0;
                }
                if (compactResult.moved) {
                        outResult->movedBlockCount += 1;
                }
                sourceOffset += segments[i].blockSize;
                targetOffset += segments[i].blockSize;
        }
        outResult->compactedTopOffset = targetOffset;
        return 1;
}
