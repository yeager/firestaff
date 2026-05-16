#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_defrag_orchestrator_pc34_compat.h"

int F0482_CACHE_DefragmentMini_Compat(
const struct MemoryCacheRawBlockHeader_Compat* headers        SEPARATOR
unsigned int                                   headerCount    SEPARATOR
struct MemoryCacheDefragSegment_Compat*         segments       SEPARATOR
unsigned int                                   maxSegments    SEPARATOR
unsigned short*                                nativeBitmapBlockIndices SEPARATOR
unsigned short*                                derivedBitmapBlockIndices SEPARATOR
unsigned long*                                 blockOffsets   SEPARATOR
struct MemoryCacheUsageState_Compat*           usageState     SEPARATOR
unsigned short                                 nativeIndexCount SEPARATOR
unsigned short                                 derivedIndexCount SEPARATOR
struct MemoryCacheDefragResultState_Compat*    defragState    SEPARATOR
struct MemoryCacheDefragOrchestratorResult_Compat* outResult   FINAL_SEPARATOR
{
        struct MemoryCacheDefragApplyResult_Compat applyResult;
        unsigned int segmentCount;


        segmentCount = MEMORY_CACHE_ScanSegments_Compat(headers, headerCount, segments, maxSegments);
        outResult->segmentCount = segmentCount;
        if (!F0482_CACHE_Defragment_Compat(
                segments,
                segmentCount,
                nativeBitmapBlockIndices,
                derivedBitmapBlockIndices,
                blockOffsets,
                usageState,
                nativeIndexCount,
                derivedIndexCount,
                defragState,
                &applyResult)) {
                return 0;
        }
        outResult->movedBlockCount = applyResult.movedBlockCount;
        outResult->compactedTopOffset = applyResult.compactedTopOffset;
        return 1;
}
