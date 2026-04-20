#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "boot_sequence_runtime_pc34_compat.h"

int F9006_RUNTIME_RunBootSequence_Compat(
const char*                              graphicsDatPath      SEPARATOR
const char*                              outputPrefix         SEPARATOR
unsigned int                             dialogGraphicIndex   SEPARATOR
unsigned int                             viewportGraphicIndex SEPARATOR
unsigned int                             firstFrameNumber     SEPARATOR
unsigned int                             frameCount           SEPARATOR
struct BootSequenceRuntimeResult_Compat* outResult           FINAL_SEPARATOR
{
        memset(outResult, 0, sizeof(*outResult));
        outResult->phase = BOOT_SEQUENCE_PHASE_NOT_STARTED;
        if (frameCount > 0) {
                outResult->phase = BOOT_SEQUENCE_PHASE_RUNTIME_BOOTSTRAP;
                outResult->completedStepCount = 1;
        }
        if (!F9005_RUNTIME_RunStatefulStartupSequence_Compat(
                graphicsDatPath,
                outputPrefix,
                dialogGraphicIndex,
                viewportGraphicIndex,
                firstFrameNumber,
                frameCount,
                &outResult->runtime)) {
                return 0;
        }
        outResult->skippedFrameCount = outResult->runtime.sequence.skippedFrameCount;
        outResult->skippedPlaceholderFrameCount = outResult->runtime.sequence.skippedPlaceholderFrameCount;
        outResult->skippedSpecialFrameCount = outResult->runtime.sequence.skippedSpecialFrameCount;
        outResult->publishedFrameCount = outResult->runtime.sequence.publishedFrameCount;
        if (outResult->runtime.reachedSteadyStartup) {
                outResult->phase = BOOT_SEQUENCE_PHASE_RUNTIME_STEADY;
                outResult->completedStepCount = 2;
        }
        if (outResult->runtime.sequence.completedFrameCount == frameCount) {
                outResult->phase = BOOT_SEQUENCE_PHASE_BOOT_SEQUENCE_COMPLETE;
                outResult->completedStepCount = 3;
                outResult->bootSequenceCompleted = 1;
        }
        return outResult->bootSequenceCompleted;
}
