#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_first_frame_pc34_compat.h"

void F0479_MEMORY_FreeFirstFrameMini_Compat(
struct MemoryGraphicsDatFirstFrameResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeStartupWiringMini_Compat(&result->startup);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunFirstFrameMini_Compat(
const char*                                     graphicsDatPath      SEPARATOR
struct MemoryGraphicsDatState_Compat*           fileState            SEPARATOR
unsigned int                                    dialogGraphicIndex   SEPARATOR
unsigned int                                    viewportGraphicIndex SEPARATOR
unsigned char*                                  viewportGraphicBuffer SEPARATOR
unsigned char*                                  viewportBitmap       SEPARATOR
struct MemoryGraphicsDatFirstFrameResult_Compat* outResult          FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_FRAME_STAGE_NOT_DISPATCHED;
        if (!F0479_MEMORY_RunStartupWiringMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->startup)) {
                return 0;
        }
        outResult->dispatchReady = outResult->startup.firstFramePrerequisitesReady;
        if (!outResult->dispatchReady) {
                return 0;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_FRAME_STAGE_PREREQUISITES_READY;
        outResult->firstFrameAttempted = 1;
        outResult->stage = MEMORY_GRAPHICS_DAT_FRAME_STAGE_FIRST_FRAME_ATTEMPTED;
        return 1;
}
