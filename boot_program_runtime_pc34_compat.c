#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <string.h>
#include "boot_program_runtime_pc34_compat.h"
#include "startup_runtime_driver_pc34_compat.h"

int F9007_RUNTIME_RunBootProgram_Compat(
const char*                                  graphicsDatPath    SEPARATOR
const char*                                  outputPrefix       SEPARATOR
unsigned int                                 dialogGraphicIndex SEPARATOR
unsigned int                                 firstFrameNumber   SEPARATOR
const struct BootProgramStep_Compat*         steps              SEPARATOR
unsigned int                                 stepCount          SEPARATOR
struct BootProgramRuntimeResult_Compat*      outResult          FINAL_SEPARATOR
{
        unsigned int i;
        char outputPath[1024];
        struct StartupRuntimeDriverResult_Compat frameResult;


        memset(outResult, 0, sizeof(*outResult));
        outResult->requestedStepCount = stepCount;
        outResult->firstFrameNumber = firstFrameNumber;
        if ((steps == 0) && (stepCount != 0)) {
                return 0;
        }
        for (i = 0; i < stepCount; ++i) {
                memset(&frameResult, 0, sizeof(frameResult));
                snprintf(outputPath, sizeof(outputPath), "%s_step_%04u.pgm", outputPrefix, firstFrameNumber + i);
                if (!F9003_RUNTIME_RunStartupFrameDriver_Compat(
                        graphicsDatPath,
                        outputPath,
                        dialogGraphicIndex,
                        steps[i].viewportGraphicIndex,
                        firstFrameNumber + i,
                        &frameResult)) {
                        return 0;
                }
                outResult->completedStepCount++;
                if (frameResult.frameSkippedByDispatcher) {
                        outResult->skippedStepCount++;
                        if (frameResult.entryClassification.kind == GRAPHICS_DAT_ENTRY_ZERO_SIZED_PLACEHOLDER) {
                                outResult->skippedPlaceholderStepCount++;
                        }
                        if (frameResult.entryClassification.kind == GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP) {
                                outResult->skippedSpecialStepCount++;
                        }
                }
                if (frameResult.framePublished) {
                        outResult->publishedFrameCount++;
                }
                F9003_RUNTIME_FreeStartupFrameDriver_Compat(&frameResult);
        }
        outResult->bootProgramCompleted = (outResult->completedStepCount == stepCount);
        return outResult->bootProgramCompleted;
}
