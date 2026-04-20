#ifndef REDMCSB_BOOT_PLAN_RUNTIME_PC34_COMPAT_H
#define REDMCSB_BOOT_PLAN_RUNTIME_PC34_COMPAT_H

#include "boot_program_runtime_pc34_compat.h"

enum BootPlanPhase_Compat {
    BOOT_PLAN_PHASE_NOT_STARTED = 0,
    BOOT_PLAN_PHASE_PROGRAM_BOOTSTRAP = 1,
    BOOT_PLAN_PHASE_PROGRAM_RUNNING = 2,
    BOOT_PLAN_PHASE_PROGRAM_COMPLETE = 3
};

struct BootPlanRuntimeResult_Compat {
    struct BootProgramRuntimeResult_Compat program;
    enum BootPlanPhase_Compat phase;
    unsigned int completedPhaseCount;
    unsigned int skippedStepCount;
    unsigned int skippedPlaceholderStepCount;
    unsigned int skippedSpecialStepCount;
    unsigned int publishedFrameCount;
    int bootPlanCompleted;
};

int F9009_RUNTIME_RunBootPlan_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int firstFrameNumber,
    const struct BootProgramStep_Compat* steps,
    unsigned int stepCount,
    struct BootPlanRuntimeResult_Compat* outResult);

#endif
