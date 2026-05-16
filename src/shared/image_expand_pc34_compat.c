#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "image_expand_pc34_compat.h"
#include "image_backend_pc34_compat.h"

void F0689_IMG_ExpandGraphicToBitmap_Compat(
const unsigned char* src SEPARATOR
unsigned char*       dst FINAL_SEPARATOR
{
        IMG3_Compat_ExpandFromSource(src, dst);
}
