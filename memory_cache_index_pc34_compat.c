#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_index_pc34_compat.h"
#include "memory_cache_block_pc34_compat.h"
#include "memory_cache_reuse_pc34_compat.h"

unsigned char* F0489_MEMORY_GetNativeBitmapByIndex_Compat(
int                                    graphicIndex            SEPARATOR
const unsigned char*                   graphic                 SEPARATOR
unsigned short*                        nativeBitmapBlockIndices SEPARATOR
struct NativeBitmapBlock_Compat**      blocks                  SEPARATOR
int                                    blockCapacity           SEPARATOR
struct NativeBitmapBlock_Compat*       newBlock                SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo              FINAL_SEPARATOR
{
        unsigned short blockIndex;
        int i;
        unsigned char* bitmap;


        blockIndex = nativeBitmapBlockIndices[graphicIndex];
        if (blockIndex != 0xFFFF && blockIndex < (unsigned short)blockCapacity && blocks[blockIndex] != 0) {
                return blocks[blockIndex]->bitmap;
        }
        for (i = 0; i < blockCapacity; i++) {
                if (blocks[i] == 0) {
                        blocks[i] = newBlock;
                        nativeBitmapBlockIndices[graphicIndex] = (unsigned short)i;
                        bitmap = F0489_MEMORY_PrepareNativeBitmapBlock_Compat(newBlock, (unsigned short)graphicIndex, sizeInfo);
                        return F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(graphic, 0, bitmap, sizeInfo);
                }
        }
        return 0;
}
