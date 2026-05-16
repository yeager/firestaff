#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "bitmap_call_pc34_compat.h"
#include "image_expand_pc34_compat.h"

void IMG_Compat_ExpandToBitmapIfPresent(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        if (bitmap != 0) {
                F0689_IMG_ExpandGraphicToBitmap_Compat(graphic, bitmap);
        }
}

void IMG_Compat_ExpandToBitmapRequired(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        F0689_IMG_ExpandGraphicToBitmap_Compat(graphic, bitmap);
}
