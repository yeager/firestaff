#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_main_loop_command_loop_pc34_compat.h"

void F0479_MEMORY_FreeMainLoopCommandLoopMini_Compat(
struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMainLoopCommandMini_Compat(&result->lastCommand);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMainLoopCommandLoopMini_Compat(
const char*                                                  graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                        fileState             SEPARATOR
unsigned int                                                 dialogGraphicIndex    SEPARATOR
unsigned int                                                 viewportGraphicIndex  SEPARATOR
unsigned char*                                               viewportGraphicBuffer SEPARATOR
unsigned char*                                               viewportBitmap        SEPARATOR
enum MemoryGraphicsDatMainLoopCommand_Compat                command               SEPARATOR
unsigned int                                                 commandCount          SEPARATOR
struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat*    outResult             FINAL_SEPARATOR
{
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_STAGE_NOT_READY;
        outResult->command = command;
        outResult->requestedCommandCount = commandCount;
        for (i = 0; i < commandCount; ++i) {
                F0479_MEMORY_FreeMainLoopCommandMini_Compat(&outResult->lastCommand);
                if (!F0479_MEMORY_RunMainLoopCommandMini_Compat(
                        graphicsDatPath,
                        fileState,
                        dialogGraphicIndex,
                        viewportGraphicIndex,
                        viewportGraphicBuffer,
                        viewportBitmap,
                        command,
                        &outResult->lastCommand)) {
                        return 0;
                }
                outResult->completedCommandCount++;
                if (outResult->lastCommand.commandReady) {
                        outResult->commandLoopReady = 1;
                        outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_STAGE_COMMAND_READY;
                }
                if (outResult->lastCommand.commandHandled) {
                        outResult->handledCommandCount++;
                }
        }
        outResult->commandLoopCompleted = (outResult->completedCommandCount == commandCount);
        if (outResult->commandLoopCompleted) {
                outResult->stage = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_STAGE_COMMAND_LOOP_COMPLETE;
        }
        return outResult->commandLoopCompleted;
}
