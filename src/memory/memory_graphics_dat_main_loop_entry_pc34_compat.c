#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_main_loop_entry_pc34_compat.h"

void F0479_MEMORY_FreeMainLoopEntryMini_Compat(
struct MemoryGraphicsDatMainLoopEntryResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeStartupDispatchMini_Compat(&result->dispatch);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMainLoopEntryMini_Compat(
const char*                                           graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                 fileState             SEPARATOR
unsigned int                                          dialogGraphicIndex    SEPARATOR
unsigned int                                          viewportGraphicIndex  SEPARATOR
unsigned char*                                        viewportGraphicBuffer SEPARATOR
unsigned char*                                        viewportBitmap        SEPARATOR
struct MemoryGraphicsDatMainLoopEntryResult_Compat*   outResult             FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_NOT_READY;
        if (!F0479_MEMORY_RunStartupDispatchMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->dispatch)) {
                return 0;
        }
        outResult->mainLoopReady =
            outResult->dispatch.startupDispatched &&
            (outResult->dispatch.stage == MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_STARTUP_DISPATCHED);
        if (!outResult->mainLoopReady) {
                return 0;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_STARTUP_READY;
        outResult->mainLoopEntered = 1;
        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_MAIN_LOOP_ENTERED;
        return 1;
}
