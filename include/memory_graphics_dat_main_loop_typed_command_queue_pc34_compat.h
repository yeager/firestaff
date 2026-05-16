#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_TYPED_COMMAND_QUEUE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_TYPED_COMMAND_QUEUE_PC34_COMPAT_H

#include "memory_graphics_dat_main_loop_command_queue_pc34_compat.h"

struct MemoryGraphicsDatMainLoopTypedCommandQueueResult_Compat {
    struct MemoryGraphicsDatMainLoopCommandQueueResult_Compat queue;
    unsigned int noopCommandCount;
    unsigned int tickCommandCount;
    unsigned int handledTickCommandCount;
};

int F0479_MEMORY_RunMainLoopTypedCommandQueueMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    const enum MemoryGraphicsDatMainLoopCommand_Compat* commands,
    unsigned int commandCount,
    struct MemoryGraphicsDatMainLoopTypedCommandQueueResult_Compat* outResult);

void F0479_MEMORY_FreeMainLoopTypedCommandQueueMini_Compat(
    struct MemoryGraphicsDatMainLoopTypedCommandQueueResult_Compat* result);

#endif
