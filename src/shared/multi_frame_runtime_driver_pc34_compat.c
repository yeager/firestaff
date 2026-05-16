#ifndef COMPILE_H
#include "COMPILE.H"
#endif
#include <stdio.h>
#include <string.h>
#include "multi_frame_runtime_driver_pc34_compat.h"
#include "startup_runtime_driver_pc34_compat.h"

int F9004_RUNTIME_RunStartupFrameSequence_Compat(
const char*                                  graphicsDatPath      SEPARATOR
const char*                                  outputPrefix         SEPARATOR
unsigned int                                 dialogGraphicIndex   SEPARATOR
unsigned int                                 viewportGraphicIndex SEPARATOR
unsigned int                                 firstFrameNumber     SEPARATOR
unsigned int                                 frameCount           SEPARATOR
struct MultiFrameRuntimeDriverResult_Compat* outResult           FINAL_SEPARATOR
{
        unsigned int i;
        char outputPath[1024];
        struct StartupRuntimeDriverResult_Compat frameResult;


        memset(outResult, 0, sizeof(*outResult));
        outResult->requestedFrameCount = frameCount;
        outResult->firstFrameNumber = firstFrameNumber;
        outResult->outputPrefix = outputPrefix;
        for (i = 0; i < frameCount; ++i) {
                memset(&frameResult, 0, sizeof(frameResult));
                snprintf(outputPath, sizeof(outputPath), "%s_%04u.pgm", outputPrefix, firstFrameNumber + i);
                if (!F9003_RUNTIME_RunStartupFrameDriver_Compat(
                        graphicsDatPath,
                        outputPath,
                        dialogGraphicIndex,
                        viewportGraphicIndex,
                        firstFrameNumber + i,
                        &frameResult)) {
                        return 0;
                }
                outResult->completedFrameCount++;
                if (frameResult.frameSkippedByDispatcher) {
                        outResult->skippedFrameCount++;
                        if (frameResult.entryClassification.kind == GRAPHICS_DAT_ENTRY_ZERO_SIZED_PLACEHOLDER) {
                                outResult->skippedPlaceholderFrameCount++;
                        } else {
                                outResult->skippedSpecialFrameCount++;
                        }
                }
                if (frameResult.framePublished) {
                        outResult->publishedFrameCount++;
                }
                F9003_RUNTIME_FreeStartupFrameDriver_Compat(&frameResult);
        }
        return outResult->completedFrameCount == frameCount;
}
