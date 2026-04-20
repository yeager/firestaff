#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_main_loop_command_queue_pc34_compat.h"

void F0479_MEMORY_FreeMainLoopCommandQueueMini_Compat(
struct MemoryGraphicsDatMainLoopCommandQueueResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMainLoopCommandLoopMini_Compat(&result->lastLoop);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMainLoopCommandQueueMini_Compat(
const char*                                                   graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                         fileState             SEPARATOR
unsigned int                                                  dialogGraphicIndex    SEPARATOR
unsigned int                                                  viewportGraphicIndex  SEPARATOR
unsigned char*                                                viewportGraphicBuffer SEPARATOR
unsigned char*                                                viewportBitmap        SEPARATOR
const enum MemoryGraphicsDatMainLoopCommand_Compat*           commands              SEPARATOR
unsigned int                                                  commandCount          SEPARATOR
struct MemoryGraphicsDatMainLoopCommandQueueResult_Compat*    outResult             FINAL_SEPARATOR
{
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        outResult->requestedQueueLength = commandCount;
        if ((commands == 0) && (commandCount != 0)) {
                return 0;
        }
        for (i = 0; i < commandCount; ++i) {
                F0479_MEMORY_FreeMainLoopCommandLoopMini_Compat(&outResult->lastLoop);
                if (!F0479_MEMORY_RunMainLoopCommandLoopMini_Compat(
                        graphicsDatPath,
                        fileState,
                        dialogGraphicIndex,
                        viewportGraphicIndex,
                        viewportGraphicBuffer,
                        viewportBitmap,
                        commands[i],
                        1,
                        &outResult->lastLoop)) {
                        return 0;
                }
                outResult->completedQueueLength++;
                outResult->handledCommandCount += outResult->lastLoop.handledCommandCount;
                if (outResult->lastLoop.commandLoopReady) {
                        outResult->queueReady = 1;
                }
        }
        outResult->queueCompleted = (outResult->completedQueueLength == commandCount);
        return outResult->queueCompleted;
}
