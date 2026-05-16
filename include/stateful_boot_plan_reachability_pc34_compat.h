#ifndef REDMCSB_STATEFUL_BOOT_PLAN_REACHABILITY_PC34_COMPAT_H
#define REDMCSB_STATEFUL_BOOT_PLAN_REACHABILITY_PC34_COMPAT_H

#include "boot_plan_script_pc34_compat.h"

enum BootPlanReachabilityPhase_Compat {
    BOOT_PLAN_REACHABILITY_PHASE_NOT_STARTED = 0,
    BOOT_PLAN_REACHABILITY_PHASE_BACKDROP_ESTABLISHED = 1,
    BOOT_PLAN_REACHABILITY_PHASE_TITLE_ESTABLISHED = 2,
    BOOT_PLAN_REACHABILITY_PHASE_MENU_ESTABLISHED = 3,
    BOOT_PLAN_REACHABILITY_PHASE_MENU_HELD = 4
};

struct BootPlanReachabilityResult_Compat {
    struct BootPlanRuntimeResult_Compat run;
    enum BootPlanReachabilityPhase_Compat phase;
    unsigned int advancedPhaseCount;
    unsigned int backdropCompletedStepCount;
    unsigned int titleCompletedStepCount;
    unsigned int menuCompletedStepCount;
    unsigned int holdCompletedStepCount;
    unsigned int holdCycleSize;
    unsigned int holdCycleCount;
    int reachedMenuEstablished;
    int reachedMenuHeld;
};

int F9012_RUNTIME_RunStatefulBootPlanReachabilityScript_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int firstFrameNumber,
    const char* scriptName,
    unsigned int backdropStepCount,
    unsigned int titleStepCount,
    unsigned int menuStepCount,
    unsigned int holdStepCount,
    unsigned int holdCycleSize,
    struct BootPlanReachabilityResult_Compat* outResult);

#endif
