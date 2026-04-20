#ifndef REDMCSB_STARTUP_RUNTIME_DRIVER_PC34_COMPAT_H
#define REDMCSB_STARTUP_RUNTIME_DRIVER_PC34_COMPAT_H

#include "memory_graphics_dat_startup_tick_pc34_compat.h"
#include "host_video_pgm_backend_pc34_compat.h"
#include "graphics_dat_entry_classify_pc34_compat.h"

struct StartupRuntimeDriverResult_Compat {
    struct MemoryGraphicsDatStartupTickResult_Compat startupTick;
    struct HostVideoPgmBackendResult_Compat hostFrame;
    struct GraphicsDatEntryClassificationResult_Compat entryClassification;
    int framePrepared;
    int framePublished;
    int frameSkippedByDispatcher;
};

int F9003_RUNTIME_RunStartupFrameDriver_Compat(
    const char* graphicsDatPath,
    const char* outputPath,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int frameNumber,
    struct StartupRuntimeDriverResult_Compat* outResult);

void F9003_RUNTIME_FreeStartupFrameDriver_Compat(
    struct StartupRuntimeDriverResult_Compat* result);

#endif
