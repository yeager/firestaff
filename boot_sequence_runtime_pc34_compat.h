#ifndef REDMCSB_BOOT_SEQUENCE_RUNTIME_PC34_COMPAT_H
#define REDMCSB_BOOT_SEQUENCE_RUNTIME_PC34_COMPAT_H

#include "stateful_runtime_driver_pc34_compat.h"

enum BootSequencePhase_Compat {
    BOOT_SEQUENCE_PHASE_NOT_STARTED = 0,
    BOOT_SEQUENCE_PHASE_RUNTIME_BOOTSTRAP = 1,
    BOOT_SEQUENCE_PHASE_RUNTIME_STEADY = 2,
    BOOT_SEQUENCE_PHASE_BOOT_SEQUENCE_COMPLETE = 3
};

struct BootSequenceRuntimeResult_Compat {
    struct StatefulRuntimeDriverResult_Compat runtime;
    enum BootSequencePhase_Compat phase;
    unsigned int completedStepCount;
    unsigned int skippedFrameCount;
    unsigned int skippedPlaceholderFrameCount;
    unsigned int skippedSpecialFrameCount;
    unsigned int publishedFrameCount;
    int bootSequenceCompleted;
};

int F9006_RUNTIME_RunBootSequence_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    unsigned int firstFrameNumber,
    unsigned int frameCount,
    struct BootSequenceRuntimeResult_Compat* outResult);

#endif
