#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_cache_frontend_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

unsigned char* F0489_MEMORY_BuildNativeBitmapInPlace_Compat(
const unsigned char*                    graphic  SEPARATOR
unsigned char*                          bitmap   SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo FINAL_SEPARATOR
{
        F0488_MEMORY_ExpandGraphicToBitmap_Compat(graphic, bitmap, sizeInfo);
        return bitmap;
}
