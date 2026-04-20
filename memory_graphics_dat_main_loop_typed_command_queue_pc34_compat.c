#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "memory_graphics_dat_main_loop_typed_command_queue_pc34_compat.h"

void F0479_MEMORY_FreeMainLoopTypedCommandQueueMini_Compat(
struct MemoryGraphicsDatMainLoopTypedCommandQueueResult_Compat* result FINAL_SEPARATOR
{
        F0479_MEMORY_FreeMainLoopCommandQueueMini_Compat(&result->queue);
        memset(result, 0, sizeof(*result));
}

int F0479_MEMORY_RunMainLoopTypedCommandQueueMini_Compat(
const char*                                                        graphicsDatPath       SEPARATOR
struct MemoryGraphicsDatState_Compat*                              fileState             SEPARATOR
unsigned int                                                       dialogGraphicIndex    SEPARATOR
unsigned int                                                       viewportGraphicIndex  SEPARATOR
unsigned char*                                                     viewportGraphicBuffer SEPARATOR
unsigned char*                                                     viewportBitmap        SEPARATOR
const enum MemoryGraphicsDatMainLoopCommand_Compat*                commands              SEPARATOR
unsigned int                                                       commandCount          SEPARATOR
struct MemoryGraphicsDatMainLoopTypedCommandQueueResult_Compat*    outResult             FINAL_SEPARATOR
{
        unsigned int i;

        memset(outResult, 0, sizeof(*outResult));
        if (!F0479_MEMORY_RunMainLoopCommandQueueMini_Compat(
                graphicsDatPath,
                fileState,
                dialogGraphicIndex,
                viewportGraphicIndex,
                viewportGraphicBuffer,
                viewportBitmap,
                commands,
                commandCount,
                &outResult->queue)) {
                return 0;
        }
        for (i = 0; i < commandCount; ++i) {
                if (commands[i] == MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_NONE) {
                        outResult->noopCommandCount++;
                } else if (commands[i] == MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_TICK) {
                        outResult->tickCommandCount++;
                }
        }
        outResult->handledTickCommandCount = outResult->queue.handledCommandCount;
        return outResult->queue.queueCompleted;
}
