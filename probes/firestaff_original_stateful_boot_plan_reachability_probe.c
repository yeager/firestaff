#include <stdio.h>
#include <stdlib.h>

#include "stateful_boot_plan_reachability_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPrefix;
    const char* scriptName = "m7_reachability_b";
    unsigned int dialogGraphicIndex = 1;
    unsigned int firstFrameNumber = 1;
    unsigned int backdropStepCount = 24;
    unsigned int titleStepCount = 16;
    unsigned int menuStepCount = 16;
    unsigned int holdStepCount = 8;
    unsigned int holdCycleSize = 8;
    struct BootPlanReachabilityResult_Compat result;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output_prefix [script_name] [dialog_index] [first_frame_number] [backdrop_steps] [title_steps] [menu_steps] [hold_steps] [hold_cycle_size]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPrefix = argv[2];
    if (argc >= 4) scriptName = argv[3];
    if (argc >= 5) dialogGraphicIndex = (unsigned int)strtoul(argv[4], 0, 10);
    if (argc >= 6) firstFrameNumber = (unsigned int)strtoul(argv[5], 0, 10);
    if (argc >= 7) backdropStepCount = (unsigned int)strtoul(argv[6], 0, 10);
    if (argc >= 8) titleStepCount = (unsigned int)strtoul(argv[7], 0, 10);
    if (argc >= 9) menuStepCount = (unsigned int)strtoul(argv[8], 0, 10);
    if (argc >= 10) holdStepCount = (unsigned int)strtoul(argv[9], 0, 10);
    if (argc >= 11) holdCycleSize = (unsigned int)strtoul(argv[10], 0, 10);

    if (!F9012_RUNTIME_RunStatefulBootPlanReachabilityScript_Compat(
            graphicsDatPath,
            outputPrefix,
            dialogGraphicIndex,
            firstFrameNumber,
            scriptName,
            backdropStepCount,
            titleStepCount,
            menuStepCount,
            holdStepCount,
            holdCycleSize,
            &result)) {
        fprintf(stderr, "failed: stateful boot-plan reachability probe did not complete\n");
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("outputPrefix=%s\n", outputPrefix);
    printf("scriptName=%s\n", scriptName);
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("firstFrameNumber=%u\n", firstFrameNumber);
    printf("completedStepCount=%u\n", result.run.program.completedStepCount);
    printf("publishedFrameCount=%u\n", result.run.publishedFrameCount);
    printf("phase=%d\n", (int)result.phase);
    printf("advancedPhaseCount=%u\n", result.advancedPhaseCount);
    printf("reachedMenuEstablished=%d\n", result.reachedMenuEstablished);
    printf("reachedMenuHeld=%d\n", result.reachedMenuHeld);
    printf("backdropCompletedStepCount=%u\n", result.backdropCompletedStepCount);
    printf("titleCompletedStepCount=%u\n", result.titleCompletedStepCount);
    printf("menuCompletedStepCount=%u\n", result.menuCompletedStepCount);
    printf("holdCompletedStepCount=%u\n", result.holdCompletedStepCount);
    printf("holdCycleSize=%u\n", result.holdCycleSize);
    printf("holdCycleCount=%u\n", result.holdCycleCount);
    return 0;
}
