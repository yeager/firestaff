#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_allocator_boundary_pc34_compat.h"

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithAllocatorBoundary_Compat(
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
struct MemoryGraphicsDatBitmapPathResult_Compat* outResult                FINAL_SEPARATOR
{
        unsigned short blockIndex;


        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if ((blockIndex != MEMORY_CACHE_INDEX_NONE) && (blockIndex < (unsigned short)blockCapacity) && (blocks[blockIndex] != 0)) {
                return F0489_MEMORY_GetOrLoadBitmapWithUsage_Compat(
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
                    outResult);
        }
        if (MEMORY_CACHE_FindFirstFreeBlockSlot_Compat(blocks, blockCapacity) == MEMORY_CACHE_ALLOCATION_NO_SLOT) {
                if (outResult != 0) {
                        memset(outResult, 0, sizeof(*outResult));
                }
                return 0;
        }
        return F0489_MEMORY_GetOrLoadBitmapWithUsage_Compat(
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
            outResult);
}
