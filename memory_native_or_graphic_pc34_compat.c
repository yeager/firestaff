#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_native_or_graphic_pc34_compat.h"

unsigned char* F0489_MEMORY_GetNativeBitmapOrGraphicTimeGated_Compat(
unsigned long                        gameTime                    SEPARATOR
unsigned long*                       lastResetGameTime           SEPARATOR
int                                  nativeBitmapOrGraphicIndex SEPARATOR
const unsigned char**                graphics                    SEPARATOR
unsigned short*                      nativeBitmapBlockIndices    SEPARATOR
struct NativeBitmapBlock_Compat**    blocks                      SEPARATOR
int                                  blockCapacity               SEPARATOR
struct NativeBitmapBlock_Compat*     newBlock                    SEPARATOR
struct MemoryCacheUsageState_Compat* usageState                  SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo                 FINAL_SEPARATOR
{
        if ((nativeBitmapOrGraphicIndex & MEMORY_NATIVE_OR_GRAPHIC_FLAG_NOT_EXPANDED) != 0) {
                return (unsigned char*)graphics[nativeBitmapOrGraphicIndex & MEMORY_NATIVE_OR_GRAPHIC_INDEX_MASK];
        }
        return F0489_MEMORY_GetNativeBitmapByIndexTimeGated_Compat(
            gameTime,
            lastResetGameTime,
            nativeBitmapOrGraphicIndex,
            graphics[nativeBitmapOrGraphicIndex],
            nativeBitmapBlockIndices,
            blocks,
            blockCapacity,
            newBlock,
            usageState,
            sizeInfo);
}
