#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "boot_plan_script_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPrefix;
    unsigned int dialogGraphicIndex = 1;
    unsigned int firstFrameNumber = 1;
    unsigned int i;
    unsigned int successCount = 0;
    unsigned int failureCount = 0;
    unsigned int totalCompletedStepCount = 0;
    unsigned int totalSkippedStepCount = 0;
    unsigned int totalPublishedFrameCount = 0;
    unsigned int totalSkippedPlaceholderStepCount = 0;
    unsigned int totalSkippedSpecialStepCount = 0;
    unsigned int expectationFailureCount = 0;
    char scriptOutputPrefix[1024];
    const struct BootPlanScript_Compat* script;
    struct BootPlanRuntimeResult_Compat result;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output_prefix [dialog_index] [first_frame_number]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPrefix = argv[2];
    if (argc >= 4) {
        dialogGraphicIndex = (unsigned int)strtoul(argv[3], 0, 10);
    }
    if (argc >= 5) {
        firstFrameNumber = (unsigned int)strtoul(argv[4], 0, 10);
    }

    printf("suite_begin\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("outputPrefix=%s\n", outputPrefix);
    printf("scriptCount=%u\n", F9010_RUNTIME_GetBootPlanScriptCount_Compat());

    for (i = 0; i < F9010_RUNTIME_GetBootPlanScriptCount_Compat(); ++i) {
        memset(&result, 0, sizeof(result));
        script = F9010_RUNTIME_GetBootPlanScriptByIndex_Compat(i);
        snprintf(scriptOutputPrefix, sizeof(scriptOutputPrefix), "%s_%s", outputPrefix, script->name);
        if (!F9011_RUNTIME_RunBootPlanScript_Compat(
                graphicsDatPath,
                scriptOutputPrefix,
                dialogGraphicIndex,
                firstFrameNumber,
                script->name,
                &result)) {
            failureCount++;
            printf("script_failure name=%s\n", script->name);
            continue;
        }
        if ((result.program.completedStepCount != script->expectedCompletedStepCount)
         || (result.skippedStepCount != script->expectedSkippedStepCount)
         || (result.skippedPlaceholderStepCount != script->expectedSkippedPlaceholderStepCount)
         || (result.skippedSpecialStepCount != script->expectedSkippedSpecialStepCount)
         || (result.publishedFrameCount != script->expectedPublishedFrameCount)) {
            expectationFailureCount++;
            printf("script_mismatch name=%s expected_completed=%u actual_completed=%u expected_skipped=%u actual_skipped=%u expected_placeholder=%u actual_placeholder=%u expected_special=%u actual_special=%u expected_published=%u actual_published=%u\n",
                   script->name,
                   script->expectedCompletedStepCount,
                   result.program.completedStepCount,
                   script->expectedSkippedStepCount,
                   result.skippedStepCount,
                   script->expectedSkippedPlaceholderStepCount,
                   result.skippedPlaceholderStepCount,
                   script->expectedSkippedSpecialStepCount,
                   result.skippedSpecialStepCount,
                   script->expectedPublishedFrameCount,
                   result.publishedFrameCount);
            continue;
        }
        successCount++;
        totalCompletedStepCount += result.program.completedStepCount;
        totalSkippedStepCount += result.skippedStepCount;
        totalPublishedFrameCount += result.publishedFrameCount;
        totalSkippedPlaceholderStepCount += result.skippedPlaceholderStepCount;
        totalSkippedSpecialStepCount += result.skippedSpecialStepCount;
        printf("script_ok name=%s steps=%u completed=%u skipped=%u placeholder=%u special=%u published=%u phase=%d\n",
               script->name,
               script->stepCount,
               result.program.completedStepCount,
               result.skippedStepCount,
               result.skippedPlaceholderStepCount,
               result.skippedSpecialStepCount,
               result.publishedFrameCount,
               (int)result.phase);
    }

    printf("suite_complete\n");
    printf("successCount=%u\n", successCount);
    printf("failureCount=%u\n", failureCount);
    printf("expectationFailureCount=%u\n", expectationFailureCount);
    printf("totalCompletedStepCount=%u\n", totalCompletedStepCount);
    printf("totalSkippedStepCount=%u\n", totalSkippedStepCount);
    printf("totalPublishedFrameCount=%u\n", totalPublishedFrameCount);
    printf("totalSkippedPlaceholderStepCount=%u\n", totalSkippedPlaceholderStepCount);
    printf("totalSkippedSpecialStepCount=%u\n", totalSkippedSpecialStepCount);
    return ((failureCount == 0) && (expectationFailureCount == 0)) ? 0 : 1;
}
