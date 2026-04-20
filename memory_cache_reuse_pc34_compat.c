#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_reuse_pc34_compat.h"
#include "memory_cache_frontend_pc34_compat.h"

unsigned char* F0489_MEMORY_GetNativeBitmapOrGraphic_Compat(
const unsigned char*                    graphic      SEPARATOR
unsigned char*                          cachedBitmap SEPARATOR
unsigned char*                          targetBitmap SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo     FINAL_SEPARATOR
{
        if (cachedBitmap != 0) {
                return cachedBitmap;
        }
        return F0489_MEMORY_BuildNativeBitmapInPlace_Compat(graphic, targetBitmap, sizeInfo);
}
