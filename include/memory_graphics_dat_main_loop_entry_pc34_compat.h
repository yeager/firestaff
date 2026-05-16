#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_ENTRY_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_MAIN_LOOP_ENTRY_PC34_COMPAT_H

#include "memory_graphics_dat_startup_dispatch_pc34_compat.h"

enum MemoryGraphicsDatMainLoopStage_Compat {
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_NOT_READY = 0,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_STARTUP_READY = 1,
    MEMORY_GRAPHICS_DAT_MAIN_LOOP_STAGE_MAIN_LOOP_ENTERED = 2
};

struct MemoryGraphicsDatMainLoopEntryResult_Compat {
    struct MemoryGraphicsDatStartupDispatchResult_Compat dispatch;
    enum MemoryGraphicsDatMainLoopStage_Compat stage;
    int mainLoopReady;
    int mainLoopEntered;
};

int F0479_MEMORY_RunMainLoopEntryMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatMainLoopEntryResult_Compat* outResult);

void F0479_MEMORY_FreeMainLoopEntryMini_Compat(
    struct MemoryGraphicsDatMainLoopEntryResult_Compat* result);

#endif
