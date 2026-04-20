#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_allocator_orchestrator_pc34_compat.h"
#include "memory_graphics_dat_bitmap_path_pc34_compat.h"

static int select_from_runtime_state(
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState SEPARATOR
unsigned int                                       graphicIndex SEPARATOR
struct MemoryGraphicsDatSelection_Compat*          selection    FINAL_SEPARATOR
{
        struct MemoryGraphicsDatHeader_Compat header;


        memset(&header, 0, sizeof(header));
        header.format = runtimeState->format;
        header.graphicCount = runtimeState->graphicCount;
        header.compressedByteCounts = runtimeState->compressedByteCounts;
        header.decompressedByteCounts = runtimeState->decompressedByteCounts;
        header.widthHeight = runtimeState->widthHeight;
        header.fileSize = runtimeState->fileSize;
        return F0490_MEMORY_SelectGraphicFromHeader_Compat(&header, graphicIndex, selection);
}

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithAllocatorOrchestrator_Compat(
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
struct MemoryCacheAllocateOrReuseState_Compat*   allocatorState           SEPARATOR
struct MemoryCacheDefragResultState_Compat*      defragState              SEPARATOR
struct MemoryCacheUnusedBlock_Compat**           firstUnusedBlock         SEPARATOR
struct MemoryCacheUnusedBlock_Compat*            splitRemainder           SEPARATOR
struct MemoryGraphicsDatAllocatorOrchestratorResult_Compat* outResult    FINAL_SEPARATOR
{
        unsigned short blockIndex;
        struct MemoryGraphicsDatSelection_Compat selection;
        struct MemoryCacheDefragOrchestratorResult_Compat orchestrator;
        struct MemoryCacheAllocatorResult_Compat allocator;
        unsigned long requestedSize;
        unsigned char* bitmap;


        memset(&selection, 0, sizeof(selection));
        memset(&orchestrator, 0, sizeof(orchestrator));
        memset(&allocator, 0, sizeof(allocator));
        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if ((blockIndex != MEMORY_CACHE_INDEX_NONE) && (blockIndex < (unsigned short)blockCapacity) && (blocks[blockIndex] != 0)) {
                bitmap = F0489_MEMORY_GetOrLoadBitmapWithUsage_Compat(
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
                    outResult != 0 ? &outResult->bitmapPath : 0);
                if ((bitmap != 0) && (outResult != 0)) {
                        memset(outResult, 0, sizeof(*outResult));
                        outResult->bitmapPath.bitmap = bitmap;
                }
                return bitmap;
        }
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
        if (!select_from_runtime_state(runtimeState, graphicIndex, &selection)) {
                return 0;
        }
        requestedSize = MEMORY_CACHE_NormalizeBlockSize_Compat((unsigned long)selection.decompressedByteCount);
        if (!F0483_CACHE_GetNewBlockWithDefrag_Compat(
                requestedSize,
                2,
                orchestrator.compactedTopOffset,
                allocatorState,
                defragState,
                firstUnusedBlock,
                splitRemainder,
                &allocator)) {
                if (outResult != 0) {
                        memset(outResult, 0, sizeof(*outResult));
                        outResult->normalizedRequestedSize = requestedSize;
                        outResult->orchestrator = orchestrator;
                }
                return 0;
        }
        bitmap = F0489_MEMORY_GetOrLoadBitmapWithUsage_Compat(
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
            outResult != 0 ? &outResult->bitmapPath : 0);
        if ((bitmap != 0) && (outResult != 0)) {
                outResult->allocator = allocator;
                outResult->orchestrator = orchestrator;
                outResult->normalizedRequestedSize = requestedSize;
                outResult->defragApplied = allocator.defragmentRequested;
        }
        return bitmap;
}
