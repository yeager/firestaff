#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_PC34_COMPAT_H

#include "memory_graphics_dat_startup_tick_pc34_compat.h"

enum MemoryGraphicsDatMainLoopCommand_Compat {
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_NONE = 0,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_TICK = 1
};

enum MemoryGraphicsDatMainLoopCommandStage_Compat {
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_STAGE_NOT_READY = 0,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_STAGE_STARTUP_TICK_READY = 1,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_STAGE_COMMAND_HANDLED = 2
};

struct MemoryGraphicsDatMainLoopCommandResult_Compat {
    struct MemoryGraphicsDatStartupTickResult_Compat startupTick;
    enum MemoryGraphicsDatMainLoopCommandStage_Compat stage;
    enum MemoryGraphicsDatMainLoopCommand_Compat command;
    int commandReady;
    int commandHandled;
    unsigned int processedCommandCount;
};

int F0479_MEMORY_RunMainLoopCommandMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    enum MemoryGraphicsDatMainLoopCommand_Compat command,
    struct MemoryGraphicsDatMainLoopCommandResult_Compat* outResult);

void F0479_MEMORY_FreeMainLoopCommandMini_Compat(
    struct MemoryGraphicsDatMainLoopCommandResult_Compat* result);

#endif
