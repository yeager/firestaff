#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_INPUT_COMMAND_QUEUE_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_INPUT_COMMAND_QUEUE_PC34_COMPAT_H

#include "memory_graphics_dat_main_loop_typed_command_queue_pc34_compat.h"

enum MemoryGraphicsDatInputSignal_Compat {
    MEMORY_GRAPHICS_DAT_INPUT_SIGNAL_NONE = 0,
    MEMORY_GRAPHICS_DAT_INPUT_SIGNAL_ADVANCE = 1
};

struct MemoryGraphicsDatInputCommandQueueResult_Compat {
    struct MemoryGraphicsDatMainLoopTypedCommandQueueResult_Compat typedQueue;
    unsigned int requestedInputCount;
    unsigned int mappedInputCount;
    unsigned int advanceInputCount;
    unsigned int idleInputCount;
};

int F0479_MEMORY_RunInputCommandQueueMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    const enum MemoryGraphicsDatInputSignal_Compat* inputs,
    unsigned int inputCount,
    struct MemoryGraphicsDatInputCommandQueueResult_Compat* outResult);

void F0479_MEMORY_FreeInputCommandQueueMini_Compat(
    struct MemoryGraphicsDatInputCommandQueueResult_Compat* result);

#endif
