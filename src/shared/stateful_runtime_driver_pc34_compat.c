#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "stateful_runtime_driver_pc34_compat.h"

int F9005_RUNTIME_RunStatefulStartupSequence_Compat(
const char*                               graphicsDatPath      SEPARATOR
const char*                               outputPrefix         SEPARATOR
unsigned int                              dialogGraphicIndex   SEPARATOR
unsigned int                              viewportGraphicIndex SEPARATOR
unsigned int                              firstFrameNumber     SEPARATOR
unsigned int                              frameCount           SEPARATOR
struct StatefulRuntimeDriverResult_Compat* outResult          FINAL_SEPARATOR
{
        unsigned int i;


        memset(outResult, 0, sizeof(*outResult));
        outResult->phase = STATEFUL_STARTUP_PHASE_NOT_STARTED;
        if (frameCount > 0) {
                outResult->phase = STATEFUL_STARTUP_PHASE_BOOTSTRAP;
                outResult->advancedPhaseCount = 1;
        }
        if (!F9004_RUNTIME_RunStartupFrameSequence_Compat(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                viewportGraphicIndex,
                firstFrameNumber,
                frameCount,
                &outResult->sequence)) {
                return 0;
        }
        outResult->skippedFrameCount = outResult->sequence.skippedFrameCount;
        outResult->skippedPlaceholderFrameCount = outResult->sequence.skippedPlaceholderFrameCount;
        outResult->skippedSpecialFrameCount = outResult->sequence.skippedSpecialFrameCount;
        outResult->publishedFrameCount = outResult->sequence.publishedFrameCount;
        for (i = 0; i < outResult->sequence.completedFrameCount; ++i) {
                if (i == 0 && outResult->phase < STATEFUL_STARTUP_PHASE_FIRST_FRAME) {
                        outResult->phase = STATEFUL_STARTUP_PHASE_FIRST_FRAME;
                        outResult->advancedPhaseCount++;
                } else if (i >= 1 && outResult->phase < STATEFUL_STARTUP_PHASE_STEADY_STARTUP) {
                        outResult->phase = STATEFUL_STARTUP_PHASE_STEADY_STARTUP;
                        outResult->advancedPhaseCount++;
                }
        }
        outResult->reachedSteadyStartup = (outResult->phase == STATEFUL_STARTUP_PHASE_STEADY_STARTUP);
        return outResult->sequence.completedFrameCount == frameCount;
}
