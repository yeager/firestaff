#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "swsh_frontend_pc34_compat.h"
#include "bitmap_call_pc34_compat.h"

void SWSH_Compat_ExpandLogoToBitmap(
const unsigned char* graphic SEPARATOR
unsigned char*       bitmap FINAL_SEPARATOR
{
        IMG_Compat_ExpandToBitmapRequired(graphic, bitmap);
}
