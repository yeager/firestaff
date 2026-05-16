#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_PC34_COMPAT_H

#include "memory_graphics_dat_main_loop_command_pc34_compat.h"

enum MemoryGraphicsDatMainLoopCommandLoopStage_Compat {
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_STAGE_NOT_READY = 0,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_STAGE_COMMAND_READY = 1,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_LOOP_STAGE_COMMAND_LOOP_COMPLETE = 2
};

struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat {
    struct MemoryGraphicsDatMainLoopCommandResult_Compat lastCommand;
    enum MemoryGraphicsDatMainLoopCommandLoopStage_Compat stage;
    enum MemoryGraphicsDatMainLoopCommand_Compat command;
    unsigned int requestedCommandCount;
    unsigned int completedCommandCount;
    unsigned int handledCommandCount;
    int commandLoopReady;
    int commandLoopCompleted;
};

int F0479_MEMORY_RunMainLoopCommandLoopMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    enum MemoryGraphicsDatMainLoopCommand_Compat command,
    unsigned int commandCount,
    struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat* outResult);

void F0479_MEMORY_FreeMainLoopCommandLoopMini_Compat(
    struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat* result);

#endif
