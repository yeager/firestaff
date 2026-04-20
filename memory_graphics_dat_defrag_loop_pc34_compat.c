#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_defrag_loop_pc34_compat.h"

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithDefragLoop_Compat(
const char*                                      graphicsDatPath          SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState           SEPARATOR
struct MemoryGraphicsDatState_Compat*            fileState                SEPARATOR
unsigned int                                     graphicIndex             SEPARATOR
unsigned char*                                   loadedGraphicBuffer      SEPARATOR
unsigned short*                                  nativeBitmapBlockIndices SEPARATOR
unsigned short*                                  derivedBitmapBlockIndices SEPARATOR
unsigned long*                                   blockOffsets             SEPARATOR
struct NativeBitmapBlock_Compat**                blocks                   SEPARATOR
int                                              blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*                 newBlock                 SEPARATOR
struct MemoryCacheUsageState_Compat*             usageState               SEPARATOR
const struct MemoryCacheRawBlockHeader_Compat*   headers                  SEPARATOR
unsigned int                                     headerCount              SEPARATOR
struct MemoryCacheDefragSegment_Compat*          segments                 SEPARATOR
unsigned int                                     maxSegments              SEPARATOR
unsigned long                                    compactedTopOffset       SEPARATOR
struct MemoryCacheAllocateOrReuseState_Compat*   allocatorState           SEPARATOR
struct MemoryCacheDefragResultState_Compat*      defragState              SEPARATOR
struct MemoryCacheUnusedBlock_Compat**           firstUnusedBlock         SEPARATOR
struct MemoryCacheUnusedBlock_Compat*            splitRemainder           SEPARATOR
struct MemoryGraphicsDatDefragLoopResult_Compat* outResult               FINAL_SEPARATOR
{
        unsigned short blockIndex;
        struct MemoryGraphicsDatDefragEntryResult_Compat entryResult;
        struct MemoryCacheDefragOrchestratorResult_Compat orchestrator;
        unsigned char* bitmap;


        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if ((blockIndex != MEMORY_CACHE_INDEX_NONE) && (blockIndex < (unsigned short)blockCapacity) && (blocks[blockIndex] != 0)) {
                bitmap = F0489_MEMORY_GetOrLoadBitmapWithDefragEntry_Compat(
                    graphicsDatPath,
                    runtimeState,
                    fileState,
                    graphicIndex,
                    loadedGraphicBuffer,
                    nativeBitmapBlockIndices,
                    blocks,
                    blockCapacity,
                    newBlock,
                    usageState,
                    compactedTopOffset,
                    allocatorState,
                    defragState,
                    firstUnusedBlock,
                    splitRemainder,
                    &entryResult);
                if ((bitmap != 0) && (outResult != 0)) {
                        memset(outResult, 0, sizeof(*outResult));
                        outResult->bitmapPath = entryResult.bitmapPath;
                        outResult->allocator = entryResult.allocator;
                        outResult->normalizedRequestedSize = entryResult.normalizedRequestedSize;
                        outResult->defragApplied = entryResult.defragApplied;
                }
                return bitmap;
        }
        memset(&orchestrator, 0, sizeof(orchestrator));
        if (!F0482_CACHE_DefragmentMini_Compat(
                headers,
                headerCount,
                segments,
                maxSegments,
                nativeBitmapBlockIndices,
                derivedBitmapBlockIndices,
                blockOffsets,
                usageState,
                runtimeState->graphicCount,
                0,
                defragState,
                &orchestrator)) {
                return 0;
        }
        bitmap = F0489_MEMORY_GetOrLoadBitmapWithDefragEntry_Compat(
            graphicsDatPath,
            runtimeState,
            fileState,
            graphicIndex,
            loadedGraphicBuffer,
            nativeBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock,
            usageState,
            orchestrator.compactedTopOffset,
            allocatorState,
            defragState,
            firstUnusedBlock,
            splitRemainder,
            &entryResult);
        if ((bitmap != 0) && (outResult != 0)) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->bitmapPath = entryResult.bitmapPath;
                outResult->allocator = entryResult.allocator;
                outResult->orchestrator = orchestrator;
                outResult->normalizedRequestedSize = entryResult.normalizedRequestedSize;
                outResult->defragApplied = entryResult.defragApplied;
        }
        return bitmap;
}
