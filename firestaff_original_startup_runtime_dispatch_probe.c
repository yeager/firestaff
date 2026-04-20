#include <stdio.h>
#include <stdlib.h>

#include "startup_runtime_driver_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static const char* kind_name(enum GraphicsDatEntryKind_Compat kind) {
    switch (kind) {
        case GRAPHICS_DAT_ENTRY_BITMAP_SAFE: return "BITMAP_SAFE";
        case GRAPHICS_DAT_ENTRY_BITMAP_SUSPICIOUS: return "BITMAP_SUSPICIOUS";
        case GRAPHICS_DAT_ENTRY_SPECIAL_NON_BITMAP: return "SPECIAL_NON_BITMAP";
        case GRAPHICS_DAT_ENTRY_EMPTY: return "EMPTY";
        case GRAPHICS_DAT_ENTRY_ZERO_SIZED_PLACEHOLDER: return "ZERO_SIZED_PLACEHOLDER";
    }
    return "UNKNOWN";
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPath;
    unsigned int dialogGraphicIndex = 1;
    unsigned int viewportGraphicIndex = 693;
    unsigned int frameNumber = 1;
    struct StartupRuntimeDriverResult_Compat result;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output.pgm [dialog_index] [viewport_index] [frame_number]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPath = argv[2];
    if (argc >= 4) {
        dialogGraphicIndex = (unsigned int)strtoul(argv[3], 0, 10);
    }
    if (argc >= 5) {
        viewportGraphicIndex = (unsigned int)strtoul(argv[4], 0, 10);
    }
    if (argc >= 6) {
        frameNumber = (unsigned int)strtoul(argv[5], 0, 10);
    }

    if (!F9003_RUNTIME_RunStartupFrameDriver_Compat(
            graphicsDatPath,
            outputPath,
            dialogGraphicIndex,
            viewportGraphicIndex,
            frameNumber,
            &result)) {
        fprintf(stderr, "failed: startup runtime driver failed\n");
        return 1;
    }

    if (result.frameSkippedByDispatcher) {
        printf("skipped\n");
    } else {
        printf("ok\n");
    }
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("viewportGraphicIndex=%u\n", viewportGraphicIndex);
    printf("frameNumber=%u\n", frameNumber);
    printf("kind=%s\n", kind_name(result.entryClassification.kind));
    printf("framePrepared=%d\n", result.framePrepared);
    printf("framePublished=%d\n", result.framePublished);
    printf("frameSkippedByDispatcher=%d\n", result.frameSkippedByDispatcher);

    F9003_RUNTIME_FreeStartupFrameDriver_Compat(&result);
    return 0;
}
