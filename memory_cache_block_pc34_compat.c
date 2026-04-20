#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_block_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned char* F0489_MEMORY_PrepareNativeBitmapBlock_Compat(
struct NativeBitmapBlock_Compat*        block       SEPARATOR
unsigned short                          bitmapIndex SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo    FINAL_SEPARATOR
{
        block->bitmapIndex = bitmapIndex;
        if (sizeInfo != 0) {
                block->width = sizeInfo->Width;
                block->height = sizeInfo->Height;
        } else {
                block->width = 0;
                block->height = 0;
        }
        return block->bitmap;
}
