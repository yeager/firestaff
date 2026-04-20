#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_TICK_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_TICK_PC34_COMPAT_H

#include "memory_graphics_dat_main_loop_entry_pc34_compat.h"

enum MemoryGraphicsDatStartupTickStage_Compat {
    MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_NOT_READY = 0,
    MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_MAIN_LOOP_READY = 1,
    MEMORY_GRAPHICS_DAT_STARTUP_TICK_STAGE_STARTUP_TICK_COMPLETE = 2
};

struct MemoryGraphicsDatStartupTickResult_Compat {
    struct MemoryGraphicsDatMainLoopEntryResult_Compat mainLoop;
    enum MemoryGraphicsDatStartupTickStage_Compat stage;
    int startupTickReady;
    int startupTickCompleted;
};

int F0479_MEMORY_RunStartupTickMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatStartupTickResult_Compat* outResult);

void F0479_MEMORY_FreeStartupTickMini_Compat(
    struct MemoryGraphicsDatStartupTickResult_Compat* result);

#endif
