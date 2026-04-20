#ifndef REDMCSB_BOOT_PLAN_SCRIPT_PC34_COMPAT_H
#define REDMCSB_BOOT_PLAN_SCRIPT_PC34_COMPAT_H

#include "boot_plan_runtime_pc34_compat.h"

struct BootPlanScript_Compat {
    const char* name;
    const struct BootProgramStep_Compat* steps;
    unsigned int stepCount;
    unsigned int expectedCompletedStepCount;
    unsigned int expectedSkippedStepCount;
    unsigned int expectedSkippedPlaceholderStepCount;
    unsigned int expectedSkippedSpecialStepCount;
    unsigned int expectedPublishedFrameCount;
};

unsigned int F9010_RUNTIME_GetBootPlanScriptCount_Compat(void);

const struct BootPlanScript_Compat* F9010_RUNTIME_GetBootPlanScriptByIndex_Compat(
    unsigned int scriptIndex);

const struct BootPlanScript_Compat* F9010_RUNTIME_GetBootPlanScript_Compat(
    const char* scriptName);

int F9011_RUNTIME_RunBootPlanScript_Compat(
    const char* graphicsDatPath,
    const char* outputPrefix,
    unsigned int dialogGraphicIndex,
    unsigned int firstFrameNumber,
    const char* scriptName,
    struct BootPlanRuntimeResult_Compat* outResult);

#endif
