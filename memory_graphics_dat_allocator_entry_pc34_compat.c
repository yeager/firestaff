#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_allocator_entry_pc34_compat.h"

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

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithAllocatorEntry_Compat(
const char*                                      graphicsDatPath          SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState           SEPARATOR
struct MemoryGraphicsDatState_Compat*            fileState                SEPARATOR
unsigned int                                     graphicIndex             SEPARATOR
unsigned char*                                   loadedGraphicBuffer      SEPARATOR
unsigned short*                                  nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**                blocks                   SEPARATOR
int                                              blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*                 newBlock                 SEPARATOR
struct MemoryCacheUsageState_Compat*             usageState               SEPARATOR
struct MemoryCacheAllocateOrReuseState_Compat*   allocatorState           SEPARATOR
struct MemoryCacheUnusedBlock_Compat**           firstUnusedBlock         SEPARATOR
struct MemoryCacheUnusedBlock_Compat*            splitRemainder           SEPARATOR
struct MemoryGraphicsDatAllocatorEntryResult_Compat* outResult           FINAL_SEPARATOR
{
        struct MemoryGraphicsDatSelection_Compat selection;
        unsigned short blockIndex;
        unsigned long requestedSize;
        struct MemoryCacheAllocatorResult_Compat allocator;
        unsigned char* bitmap;


        memset(&selection, 0, sizeof(selection));
        memset(&allocator, 0, sizeof(allocator));
        if (!select_from_runtime_state(runtimeState, graphicIndex, &selection)) {
                return 0;
        }
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
                        outResult->allocator = allocator;
                        outResult->normalizedRequestedSize = 0;
                }
                return bitmap;
        }
        requestedSize = MEMORY_CACHE_NormalizeBlockSize_Compat((unsigned long)selection.decompressedByteCount);
        if (!F0483_CACHE_GetNewBlock_Compat(
                requestedSize,
                2,
                allocatorState,
                firstUnusedBlock,
                splitRemainder,
                &allocator)) {
                if (outResult != 0) {
                        memset(outResult, 0, sizeof(*outResult));
                        outResult->normalizedRequestedSize = requestedSize;
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
                outResult->normalizedRequestedSize = requestedSize;
        }
        return bitmap;
}
