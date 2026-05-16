#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_main_loop_command_pc34_compat.h"

void F0479_MEMORY_FreeMainLoopCommandMini_Compat(
struct MemoryGraphicsDatMainLoopCommandResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeStartupTickMini_Compat(&result->startupTick);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMainLoopCommandMini_Compat(
const char*                                              graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                    fileState             SEPARATOR
unsigned int                                             dialogGraphicIndex    SEPARATOR
unsigned int                                             viewportGraphicIndex  SEPARATOR
unsigned char*                                           viewportGraphicBuffer SEPARATOR
unsigned char*                                           viewportBitmap        SEPARATOR
enum MemoryGraphicsDatMainLoopCommand_Compat            command               SEPARATOR
struct MemoryGraphicsDatMainLoopCommandResult_Compat*    outResult             FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_STAGE_NOT_READY;
        outResult->command = command;
        if (!F0479_MEMORY_RunStartupTickMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                &outResult->startupTick)) {
                return 0;
        }
        outResult->commandReady =
            outResult->startupTick.startupTickCompleted &&
            (outResult->startupTick.stage == MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_STARTUP_TICK_COMPLETE);
        if (!outResult->commandReady) {
                return 0;
        }
        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_STAGE_STARTUP_TICK_READY;
        if (command != MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_NONE) {
                outResult->commandHandled = 1;
                outResult->processedCommandCount = 1;
                outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_STAGE_COMMAND_HANDLED;
        }
        return 1;
}
