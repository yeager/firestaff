#include <stdio.h>
#include <stdlib.h>

#include "stateful_runtime_driver_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPrefix;
    unsigned int dialogGraphicIndex = 1;
    unsigned int viewportGraphicIndex = 0;
    unsigned int firstFrameNumber = 1;
    unsigned int frameCount = 3;
    struct StatefulRuntimeDriverResult_Compat result;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output_prefix [dialog_index] [viewport_index] [first_frame_number] [frame_count]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPrefix = argv[2];
    if (argc >= 4) {
        dialogGraphicIndex = (unsigned int)strtoul(argv[3], 0, 10);
    }
    if (argc >= 5) {
        viewportGraphicIndex = (unsigned int)strtoul(argv[4], 0, 10);
    }
    if (argc >= 6) {
        firstFrameNumber = (unsigned int)strtoul(argv[5], 0, 10);
    }
    if (argc >= 7) {
        frameCount = (unsigned int)strtoul(argv[6], 0, 10);
    }

    if (!F9005_RUNTIME_RunStatefulStartupSequence_Compat(
            graphicsDatPath,
            outputPrefix,
            dialogGraphicIndex,
            viewportGraphicIndex,
            firstFrameNumber,
            frameCount,
            &result)) {
        fprintf(stderr, "failed: stateful runtime sequence did not complete\n");
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("outputPrefix=%s\n", outputPrefix);
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("viewportGraphicIndex=%u\n", viewportGraphicIndex);
    printf("firstFrameNumber=%u\n", result.sequence.firstFrameNumber);
    printf("requestedFrameCount=%u\n", result.sequence.requestedFrameCount);
    printf("skippedFrameCount=%u\n", result.skippedFrameCount);
    printf("skippedPlaceholderFrameCount=%u\n", result.skippedPlaceholderFrameCount);
    printf("skippedSpecialFrameCount=%u\n", result.skippedSpecialFrameCount);
    printf("publishedFrameCount=%u\n", result.publishedFrameCount);
    printf("phase=%d\n", (int)result.phase);
    printf("advancedPhaseCount=%u\n", result.advancedPhaseCount);
    printf("reachedSteadyStartup=%d\n", result.reachedSteadyStartup);
    return 0;
}
