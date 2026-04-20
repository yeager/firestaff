#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "dialog_frontend_pc34_compat.h"
#include "memory_frontend_pc34_compat.h"

void F0427_DIALOG_DrawBackdrop_Compat(
const unsigned char*                    graphic        SEPARATOR
unsigned char*                          viewportBitmap SEPARATOR
const struct GraphicWidthHeight_Compat* sizeInfo       FINAL_SEPARATOR
{
        F0488_MEMORY_ExpandGraphicToBitmap_Compat(graphic, viewportBitmap, sizeInfo);
}
