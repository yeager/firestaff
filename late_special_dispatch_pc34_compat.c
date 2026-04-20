#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "late_special_dispatch_pc34_compat.h"

int F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(
unsigned int                              graphicIndex SEPARATOR
struct LateSpecialDispatchResult_Compat*  outResult    FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->graphicIndex = graphicIndex;
        outResult->kind = LATE_GRAPHICS_DAT_ENTRY_BITMAP_SAFE;
        outResult->shouldUseBitmapPath = 1;
        outResult->shouldSkipBitmapExport = 0;

        if (graphicIndex == 694U) {
                outResult->kind = LATE_GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP;
                outResult->shouldUseBitmapPath = 0;
                outResult->shouldSkipBitmapExport = 1;
                return 1;
        }
        if ((graphicIndex >= 697U) && (graphicIndex <= 700U)) {
                outResult->kind = LATE_GRAPHICS_DAT_ENTRY_EMPTY;
                outResult->shouldUseBitmapPath = 0;
                outResult->shouldSkipBitmapExport = 1;
                return 1;
        }
        if ((graphicIndex == 696U)
         || ((graphicIndex >= 701U) && (graphicIndex <= 706U))
         || (graphicIndex == 708U)
         || ((graphicIndex >= 711U) && (graphicIndex <= 712U))) {
                outResult->kind = LATE_GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS;
                outResult->shouldUseBitmapPath = 1;
                outResult->shouldSkipBitmapExport = 0;
                return 1;
        }
        return 1;
}
