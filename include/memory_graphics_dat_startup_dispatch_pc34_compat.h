#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_DISPATCH_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_STARTUP_DISPATCH_PC34_COMPAT_H

#include "memory_graphics_dat_first_frame_pc34_compat.h"

enum MemoryGraphicsDatDispatchStage_Compat {
    MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_NOT_READY = 0,
    MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_FIRST_FRAME_READY = 1,
    MEMORY_GRAPHICS_DAT_DISPATCH_STAGE_STARTUP_DISPATCHED = 2
};

struct MemoryGraphicsDatStartupDispatchResult_Compat {
    struct MemoryGraphicsDatFirstFrameResult_Compat firstFrame;
    enum MemoryGraphicsDatDispatchStage_Compat stage;
    int startupDispatchReady;
    int startupDispatched;
};

int F0479_MEMORY_RunStartupDispatchMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatStartupDispatchResult_Compat* outResult);

void F0479_MEMORY_FreeStartupDispatchMini_Compat(
    struct MemoryGraphicsDatStartupDispatchResult_Compat* result);

#endif
