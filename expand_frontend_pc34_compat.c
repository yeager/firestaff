#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "expand_frontend_pc34_compat.h"
#include "bitmap_call_pc34_compat.h"

void F0466_EXPAND_GraphicToBitmap_Compat(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        IMG_Compat_ExpandToBitmapIfPresent(graphic, bitmap);
}
