#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_startup_dispatch_pc34_compat.h"

void F0479_MEMORY_FreeStartupDispatchMini_Compat(
struct MemoryGraphicsDatStartupDispatchResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeFirstFrameMini_Compat(&result->firstFrame);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunStartupDispatchMini_Compat(
const char*                                           graphicsDatPath      SEPARATOR
struct MemoryGraphicsDatState_Compat*                 fileState            SEPARATOR
unsigned int                                          dialogGraphicIndex   SEPARATOR
unsigned int                                          viewportGraphicIndex SEPARATOR
unsigned char*                                        viewportGraphicBuffer SEPARATOR
unsigned char*                                        viewportBitmap       SEPARATOR
struct MemoryGraphicsDatStartupDispatchResult_Compat* outResult           FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_NOT_READY;
        if (!F0479_MEMORY_RunFirstFrameMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->firstFrame)) {
                return 0;
        }
        outResult->startupDispatchReady =
            outResult->firstFrame.firstFrameAttempted &&
            (outResult->firstFrame.stage == MEMORY_GRAPHICS_DAT_FRAME_STAGE_FIRST_FRAME_ATTEMPTED);
        if (!outResult->startupDispatchReady) {
                return 0;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_FIRST_FRAME_READY;
        outResult->startupDispatched = 1;
        outResult->stage = MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_STARTUP_DISPATCHED;
        return 1;
}
