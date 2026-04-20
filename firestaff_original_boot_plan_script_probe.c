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
    const char* scriptName = "mixed_warmup";
    unsigned int dialogGraphicIndex = 1;
    unsigned int firstFrameNumber = 1;
    unsigned int i;
    const struct BootPlanScript_Compat* script;
    struct BootPlanRuntimeResult_Compat result;

    if ((argc == 2) && (strcmp(argv[1], "--list") == 0)) {
        printf("ok\n");
        printf("scriptCount=%u\n", F9010_RUNTIME_GetBootPlanScriptCount_Compat());
        for (i = 0; i < F9010_RUNTIME_GetBootPlanScriptCount_Compat(); ++i) {
            script = F9010_RUNTIME_GetBootPlanScriptByIndex_Compat(i);
            printf("script[%u]=%s steps=%u expected_completed=%u expected_skipped=%u expected_placeholder=%u expected_special=%u expected_published=%u\n",
                   i,
                   script->name,
                   script->stepCount,
                   script->expectedCompletedStepCount,
                   script->expectedSkippedStepCount,
                   script->expectedSkippedPlaceholderStepCount,
                   script->expectedSkippedSpecialStepCount,
                   script->expectedPublishedFrameCount);
        }
        return 0;
    }

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output_prefix [script_name] [dialog_index] [first_frame_number]\n", argv[0]);
        fprintf(stderr, "   or: %s --list\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPrefix = argv[2];
    if (argc >= 4) {
        scriptName = argv[3];
    }
    if (argc >= 5) {
        dialogGraphicIndex = (unsigned int)strtoul(argv[4], 0, 10);
    }
    if (argc >= 6) {
        firstFrameNumber = (unsigned int)strtoul(argv[5], 0, 10);
    }

    script = F9010_RUNTIME_GetBootPlanScript_Compat(scriptName);
    if (script == 0) {
        fprintf(stderr, "failed: unknown script %s\n", scriptName);
        return 1;
    }
    if (!F9011_RUNTIME_RunBootPlanScript_Compat(
            graphicsDatPath,
            outputPrefix,
            dialogGraphicIndex,
            firstFrameNumber,
            scriptName,
            &result)) {
        fprintf(stderr, "failed: boot plan script did not complete\n");
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("outputPrefix=%s\n", outputPrefix);
    printf("scriptName=%s\n", script->name);
    printf("scriptStepCount=%u\n", script->stepCount);
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("firstFrameNumber=%u\n", result.program.firstFrameNumber);
    printf("completedStepCount=%u\n", result.program.completedStepCount);
    printf("skippedStepCount=%u\n", result.skippedStepCount);
    printf("skippedPlaceholderStepCount=%u\n", result.skippedPlaceholderStepCount);
    printf("skippedSpecialStepCount=%u\n", result.skippedSpecialStepCount);
    printf("publishedFrameCount=%u\n", result.publishedFrameCount);
    printf("phase=%d\n", (int)result.phase);
    printf("bootPlanCompleted=%d\n", result.bootPlanCompleted);
    return 0;
}
