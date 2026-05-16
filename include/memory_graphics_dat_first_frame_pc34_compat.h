#ifndef REDMCSB_MEMORY_GRAPHICS_DAT_FIRST_FRAME_PC34_COMPAT_H
#define REDMCSB_MEMORY_GRAPHICS_DAT_FIRST_FRAME_PC34_COMPAT_H

#include "memory_graphics_dat_startup_wiring_pc34_compat.h"

enum MemoryGraphicsDatFrameStage_Compat {
    MEMORY_GRAPHICS_DAT_FRAME_STAGE_NOT_DISPATCHED = 0,
    MEMORY_GRAPHICS_DAT_FRAME_STAGE_PREREQUISITES_READY = 1,
    MEMORY_GRAPHICS_DAT_FRAME_STAGE_FIRST_FRAME_ATTEMPTED = 2
};

struct MemoryGraphicsDatFirstFrameResult_Compat {
    struct MemoryGraphicsDatStartupWiringResult_Compat startup;
    enum MemoryGraphicsDatFrameStage_Compat stage;
    int dispatchReady;
    int firstFrameAttempted;
};

int F0479_MEMORY_RunFirstFrameMini_Compat(
    const char* graphicsDatPath,
    struct MemoryGraphicsDatState_Compat* fileState,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned char* viewportGraphicBuffer,
    unsigned char* viewportBitmap,
    struct MemoryGraphicsDatFirstFrameResult_Compat* outResult);

void F0479_MEMORY_FreeFirstFrameMini_Compat(
    struct MemoryGraphicsDatFirstFrameResult_Compat* result);

#endif
