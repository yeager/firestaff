#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "graphics_dat_entry_classify_pc34_compat.h"
#include "late_special_dispatch_pc34_compat.h"

int F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(
const struct MemoryGraphicsDatRuntimeState_Compat* runtimeState SEPARATOR
unsigned int                                       graphicIndex  SEPARATOR
struct GraphicsDatEntryClassificationResult_Compat* outResult    FINAL_SEPARATOR
{
        struct LateSpecialDispatchResult_Compat late;


        memset(outResult, 0, sizeof(*outResult));
        outResult->graphicIndex = graphicIndex;
        outResult->kind = GRAPHICS_DAT_ENTRY_BITMAP_SAFE;
        outResult->shouldUseBitmapPath = 1;
        outResult->shouldSkipBitmapExport = 0;
        if (runtimeState == 0) {
                return 0;
        }
        if (graphicIndex >= runtimeState->graphicCount) {
                return 0;
        }
        if (!F9008_RUNTIME_ClassifyLateGraphicsDatEntry_Compat(graphicIndex, &late)) {
                return 0;
        }
        if (late.kind == LATE_GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP) {
                outResult->kind = GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP;
                outResult->shouldUseBitmapPath = 0;
                outResult->shouldSkipBitmapExport = 1;
                return 1;
        }
        if (late.kind == LATE_GRAPHICS_DAT_ENTRY_EMPTY) {
                outResult->kind = GRAPHICS_DAT_ENTRY_EMPTY;
                outResult->shouldUseBitmapPath = 0;
                outResult->shouldSkipBitmapExport = 1;
                return 1;
        }
        if ((runtimeState->compressedByteCounts[graphicIndex] == 0)
         && (runtimeState->decompressedByteCounts[graphicIndex] == 0)
         && (runtimeState->widthHeight[graphicIndex].Width == 0)
         && (runtimeState->widthHeight[graphicIndex].Height == 0)) {
                outResult->kind = GRAPHICS_DAT_ENTRY_ZERO_SIZED_PLACEHOLDER;
                outResult->shouldUseBitmapPath = 0;
                outResult->shouldSkipBitmapExport = 1;
                return 1;
        }
        if (late.kind == LATE_GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS) {
                outResult->kind = GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS;
                outResult->shouldUseBitmapPath = 1;
                outResult->shouldSkipBitmapExport = 0;
                return 1;
        }
        return 1;
}
