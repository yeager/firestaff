#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "memory_frontend_pc34_compat.h"
#include "expand_frontend_pc34_compat.h"

#define M100_PIXEL_WIDTH(bitmap)  (*((unsigned short*)(bitmap) - 2))
#define M101_PIXEL_HEIGHT(bitmap) (*((unsigned short*)(bitmap) - 1))

void F0488_MEMORY_ExpandGraphicToBitmap_Compat(
const unsigned char*                      graphic   SEPARATOR
unsigned char*                            bitmap    SEPARATOR
const struct GraphicWidthHeight_Compat*   sizeInfo  FINAL_SEPARATOR
{
        F0466_EXPAND_GraphicToBitmap_Compat(graphic, bitmap);
        if ((bitmap != 0) && (sizeInfo != 0)) {
                M100_PIXEL_WIDTH(bitmap) = sizeInfo->Width;
                M101_PIXEL_HEIGHT(bitmap) = sizeInfo->Height;
        }
}
