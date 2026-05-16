#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_startup_tick_pc34_compat.h"

void F0479_MEMORY_FreeStartupTickMini_Compat(
struct MemoryGraphicsDatStartupTickResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMainLoopEntryMini_Compat(&result->mainLoop);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunStartupTickMini_Compat(
const char*                                          graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                fileState             SEPARATOR
unsigned int                                         dialogGraphicIndex    SEPARATOR
unsigned int                                         viewportGraphicIndex  SEPARATOR
unsigned char*                                       viewportGraphicBuffer SEPARATOR
unsigned char*                                       viewportBitmap        SEPARATOR
struct MemoryGraphicsDatStartupTickResult_Compat*    outResult             FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_NOT_READY;
        if (!F0479_MEMORY_RunMainLoopEntryMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->mainLoop)) {
                return 0;
        }
        outResult->startupTickReady =
            outResult->mainLoop.mainLoopEntered &&
            (outResult->mainLoop.stage == MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_MAIN_LOOP_ENTERED);
        if (!outResult->startupTickReady) {
                return 0;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_MAIN_LOOP_READY;
        outResult->startupTickCompleted = 1;
        outResult->stage = MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_STARTUP_TICK_COMPLETE;
        return 1;
}
