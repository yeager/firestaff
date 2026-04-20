#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "selector_frontend_pc34_compat.h"
#include "bitmap_call_pc34_compat.h"

void F8347_Compat(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        IMG_Compat_ExpandToBitmapIfPresent(graphic, bitmap);
}
