#include <stdio.h>
#include <stdlib.h>

#include "boot_program_runtime_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPrefix;
    unsigned int dialogGraphicIndex = 1;
    unsigned int firstFrameNumber = 1;
    struct BootProgramStep_Compat steps[3] = {
        {0},
        {1},
        {2}
    };
    struct BootProgramRuntimeResult_Compat result;

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

    if (!F9007_RUNTIME_RunBootProgram_Compat(
            graphicsDatPath,
            outputPrefix,
            dialogGraphicIndex,
            firstFrameNumber,
            steps,
            3,
            &result)) {
        fprintf(stderr, "failed: boot program did not complete\n");
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("outputPrefix=%s\n", outputPrefix);
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("firstFrameNumber=%u\n", result.firstFrameNumber);
    printf("requestedStepCount=%u\n", result.requestedStepCount);
    printf("completedStepCount=%u\n", result.completedStepCount);
    printf("bootProgramCompleted=%d\n", result.bootProgramCompleted);
    return 0;
}
