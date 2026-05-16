#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_block_index_pc34_compat.h"

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

unsigned char* F0489_MEMORY_GetOrLoadBitmapByBlockIndex_Compat(
const char*                                      graphicsDatPath          SEPARATOR
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState           SEPARATOR
struct MemoryGraphicsDatState_Compat*            fileState                SEPARATOR
unsigned int                                     graphicIndex             SEPARATOR
unsigned char*                                   loadedGraphicBuffer      SEPARATOR
unsigned short*                                  nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**                blocks                   SEPARATOR
int                                              blockCapacity            SEPARATOR
struct NativeBitmapBlock_Compat*                 newBlock                 SEPARATOR
struct MemoryGraphicsDatBitmapPathResult_Compat* outResult                FINAL_SEPARATOR
{
        struct MemoryGraphicsDatSelection_Compat selection;
        unsigned short blockIndex;
        int i;
        unsigned char* targetBitmap;


        if (!select_from_runtime_state(runtimeState, graphicIndex, &selection)) {
                return 0;
        }
        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if ((blockIndex != 0xFFFF) && (blockIndex < (unsigned short)blockCapacity) && (blocks[blockIndex] != 0)) {
            if (outResult != 0) {
                memset(outResult, 0, sizeof(*outResult));
                outResult->selection = selection;
                outResult->bitmap = blocks[blockIndex]->bitmap;
                outResult->loadedGraphic = loadedGraphicBuffer;
            }
            return blocks[blockIndex]->bitmap;
        }
        for (i = 0; i < blockCapacity; i++) {
                if (blocks[i] == 0) {
                        blocks[i] = newBlock;
                        nativeBitmapBlockIndices[graphicIndex] = (unsigned short)i;
                        targetBitmap = F0489_MEMORY_PrepareNativeBitmapBlock_Compat(
                            newBlock,
                            (unsigned short)graphicIndex,
                            &selection.widthHeight);
                        return F0489_MEMORY_GetOrLoadNativeBitmapByIndex_Compat(
                            graphicsDatPath,
                            runtimeState,
                            fileState,
                            graphicIndex,
                            loadedGraphicBuffer,
                            0,
                            targetBitmap,
                            outResult);
                }
        }
        return 0;
}
