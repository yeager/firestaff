#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_input_command_queue_pc34_compat.h"

void F0479_MEMORY_FreeInputCommandQueueMini_Compat(
struct MemoryGraphicsDatInputCommandQueueResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMainLoopTypedCommandQueueMini_Compat(&result->typedQueue);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunInputCommandQueueMini_Compat(
const char*                                                 graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                       fileState             SEPARATOR
unsigned int                                                dialogGraphicIndex    SEPARATOR
unsigned int                                                viewportGraphicIndex  SEPARATOR
unsigned char*                                              viewportGraphicBuffer SEPARATOR
unsigned char*                                              viewportBitmap        SEPARATOR
const enum MemoryGraphicsDatInputSignal_Compat*             inputs                SEPARATOR
unsigned int                                                inputCount            SEPARATOR
struct MemoryGraphicsDatInputCommandQueueResult_Compat*     outResult             FINAL_SEPARATOR
{
        enum MemoryGraphicsDatMainLoopCommand_Compat commands[32];
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        outResult->requestedInputCount = inputCount;
        if ((inputs == 0) && (inputCount != 0)) {
                return 0;
        }
        if (inputCount > 32U) {
                return 0;
        }
        for (i = 0; i < inputCount; ++i) {
                if (inputs[i] == MEMORY_GRAPHICS_DAT_INPUT_SIGNAL_ADVANCE) {
                        commands[i] = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_TICK;
                        outResult->advanceInputCount++;
                } else {
                        commands[i] = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_NONE;
                        outResult->idleInputCount++;
                }
                outResult->mappedInputCount++;
        }
        if (!F0479_MEMORY_RunMainLoopTypedCommandQueueMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                commands,
                inputCount,
                &outResult->typedQueue)) {
                return 0;
        }
        return outResult->typedQueue.queue.queueCompleted;
}
