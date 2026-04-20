#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include "endgame_frontend_pc34_compat.h"
#include "bitmap_call_pc34_compat.h"

void ENDGAME_Compat_ExpandCreditsToScreenBitmap(
const unsigned char* graphic SEPARATOR
unsigned char*       screenBitmap FINAL_SEPARATOR
{
        IMG_Compat_ExpandToBitmapRequired(graphic, screenBitmap);
}
