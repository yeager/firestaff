#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_QUEUE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_QUEUE_PC34_COMPAT_H

#include "memory_graphics_dat_main_loop_command_loop_pc34_compat.h"

struct MemoryGraphicsDatMainLoopCommandQueueResult_Compat {
    struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat lastLoop;
    unsigned int requestedQueueLength;
    unsigned int completedQueueLength;
    unsigned int handledCommandCount;
    int queueReady;
    int queueCompleted;
};

int F0479_MEMORY_RunMainLoopCommandQueueMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    const enum MemoryGraphicsDatMainLoopCommand_Compat* commands,
    unsigned int commandCount,
    struct MemoryGraphicsDatMainLoopCommandQueueResult_Compat* outResult);

void F0479_MEMORY_FreeMainLoopCommandQueueMini_Compat(
    struct MemoryGraphicsDatMainLoopCommandQueueResult_Compat* result);

#endif
