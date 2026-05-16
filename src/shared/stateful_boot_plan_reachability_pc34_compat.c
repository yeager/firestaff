#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "stateful_boot_plan_reachability_pc34_compat.h"

int F9012_RUNTIME_RunStatefulBootPlanReachabilityScript_Compat(
const char*                                  graphicsDatPath      SEPARATOR
const char*                                  outputPrefix         SEPARATOR
unsigned int                                 dialogGraphicIndex   SEPARATOR
unsigned int                                 firstFrameNumber     SEPARATOR
const char*                                  scriptName           SEPARATOR
unsigned int                                 backdropStepCount    SEPARATOR
unsigned int                                 titleStepCount       SEPARATOR
unsigned int                                 menuStepCount        SEPARATOR
unsigned int                                 holdStepCount        SEPARATOR
unsigned int                                 holdCycleSize        SEPARATOR
struct BootPlanReachabilityResult_Compat*    outResult           FINAL_SEPARATOR
{
        unsigned int completed;
        unsigned int threshold;
        unsigned int completedBeforeHold;

        memset(outResult, 0, sizeof(*outResult));
        outResult->phase = BOOT_PLAN_REACHABILITY_PHASE_NOT_STARTED;
        outResult->holdCycleSize = holdCycleSize;
        if (!F9011_RUNTIME_RunBootPlanScript_Compat(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                firstFrameNumber,
                scriptName,
                &outResult->run)) {
                return 0;
        }
        completed = outResult->run.program.completedStepCount;
        threshold = backdropStepCount;
        if (threshold > 0 && completed >= threshold) {
                outResult->phase = BOOT_PLAN_REACHABILITY_PHASE_BACKDROP_ESTABLISHED;
                outResult->advancedPhaseCount++;
                outResult->backdropCompletedStepCount = threshold;
        }
        threshold += titleStepCount;
        if (titleStepCount > 0 && completed >= threshold) {
                outResult->phase = BOOT_PLAN_REACHABILITY_PHASE_TITLE_ESTABLISHED;
                outResult->advancedPhaseCount++;
                outResult->titleCompletedStepCount = titleStepCount;
        }
        threshold += menuStepCount;
        completedBeforeHold = threshold;
        if (menuStepCount > 0 && completed >= threshold) {
                outResult->phase = BOOT_PLAN_REACHABILITY_PHASE_MENU_ESTABLISHED;
                outResult->advancedPhaseCount++;
                outResult->menuCompletedStepCount = menuStepCount;
                outResult->reachedMenuEstablished = 1;
        }
        threshold += holdStepCount;
        if (holdStepCount > 0 && completed >= completedBeforeHold) {
                outResult->holdCompletedStepCount = completed - completedBeforeHold;
                if (outResult->holdCompletedStepCount > holdStepCount) {
                        outResult->holdCompletedStepCount = holdStepCount;
                }
                if (holdCycleSize > 0) {
                        outResult->holdCycleCount = outResult->holdCompletedStepCount / holdCycleSize;
                }
        }
        if (holdStepCount > 0 && completed >= threshold) {
                outResult->phase = BOOT_PLAN_REACHABILITY_PHASE_MENU_HELD;
                outResult->advancedPhaseCount++;
                outResult->reachedMenuHeld = 1;
        }
        return outResult->run.bootPlanCompleted;
}
