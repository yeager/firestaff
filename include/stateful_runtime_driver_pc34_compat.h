#ifndef REDMCSB_STATEFUL_RUNTIME_DRIVER_PC34_COMPAT_H
#define REDMCSB_STATEFUL_RUNTIME_DRIVER_PC34_COMPAT_H

#include "multi_frame_runtime_driver_pc34_compat.h"

enum StatefulStartupPhase_Compat {
    STATEFUL_STARTUP_PHASE_NOT_STARTED = 0,
    STATEFUL_STARTUP_PHASE_BOOTSTRAP = 1,
    STATEFUL_STARTUP_PHASE_FIRST_FRAME = 2,
    STATEFUL_STARTUP_PHASE_STEADY_STARTUP = 3
};

struct StatefulRuntimeDriverResult_Compat {
    struct MultiFrameRuntimeDriverResult_Compat sequence;
    enum StatefulStartupPhase_Compat phase;
    unsigned int advancedPhaseCount;
    unsigned int skippedFrameCount;
    unsigned int skippedPlaceholderFrameCount;
    unsigned int skippedSpecialFrameCount;
    unsigned int publishedFrameCount;
    int reachedSteadyStartup;
};

int F9005_RUNTIME_RunStatefulStartupSequence_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int firstFrameNumber,
    unsigned int frameCount,
    struct StatefulRuntimeDriverResult_Compat* outResult);

#endif
