#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_graphics_dat_used_list_pc34_compat.h"

unsigned char* F0489_MEMORY_GetOrLoadBitmapWithUsage_Compat(
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
        unsigned char* bitmap;


        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if ((blockIndex != MEMORY_CACHE_INDEX_NONE) && (blockIndex < (unsigned short)blockCapacity) && (blocks[blockIndex] != 0)) {
                bitmap = F0487_CACHE_GetBlockAndIncrementUsageCount_Compat(
                    blockIndex,
                    usageState,
                    blocks,
                    blockCapacity)->bitmap;
                if (outResult != 0) {
                        outResult->bitmap = bitmap;
                }
                return bitmap;
        }
        bitmap = F0489_MEMORY_GetOrLoadBitmapByBlockIndex_Compat(
            graphicsDatPath,
            runtimeState,
            fileState,
            graphicIndex,
            loadedGraphicBuffer,
            nativeBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock,
            outResult);
        if (bitmap == 0) {
                return 0;
        }
        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if ((blockIndex != MEMORY_CACHE_INDEX_NONE) && (blockIndex < (unsigned short)blockCapacity) && (blocks[blockIndex] != 0)) {
                F0486_MEMORY_AddBlockToUsedList_Compat(
                    blockIndex,
                    usageState,
                    blocks,
                    blockCapacity);
        }
        return bitmap;
}
