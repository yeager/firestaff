#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "boot_plan_runtime_pc34_compat.h"

int F9009_RUNTIME_RunBootPlan_Compat(
const char*                               graphicsDatPath    SEPARATOR
const char*                               outputPrefix       SEPARATOR
unsigned int                              dialogGraphicIndex SEPARATOR
unsigned int                              firstFrameNumber   SEPARATOR
const struct BootProgramStep_Compat*      steps              SEPARATOR
unsigned int                              stepCount          SEPARATOR
struct BootPlanRuntimeResult_Compat*      outResult          FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->phase = BOOT_PLAN_PHASE_NOT_STARTED;
        if (stepCount > 0) {
                outResult->phase = BOOT_PLAN_PHASE_PROGRAM_BOOTSTRAP;
                outResult->completedPhaseCount = 1;
        }
        if (!F9007_RUNTIME_RunBootProgram_Compat(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                firstFrameNumber,
                steps,
                stepCount,
                &outResult->program)) {
                return 0;
        }
        outResult->skippedStepCount = outResult->program.skippedStepCount;
        outResult->skippedPlaceholderStepCount = outResult->program.skippedPlaceholderStepCount;
        outResult->skippedSpecialStepCount = outResult->program.skippedSpecialStepCount;
        outResult->publishedFrameCount = outResult->program.publishedFrameCount;
        if (outResult->program.completedStepCount > 0) {
                outResult->phase = BOOT_PLAN_PHASE_PROGRAM_RUNNING;
                outResult->completedPhaseCount = 2;
        }
        if (outResult->program.bootProgramCompleted) {
                outResult->phase = BOOT_PLAN_PHASE_PROGRAM_COMPLETE;
                outResult->completedPhaseCount = 3;
                outResult->bootPlanCompleted = 1;
        }
        return outResult->bootPlanCompleted;
}
