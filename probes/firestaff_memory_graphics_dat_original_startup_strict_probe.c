#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_startup_tick_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    unsigned int dialogGraphicIndex = 1;
    unsigned int viewportGraphicIndex = 0;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatHeader_Compat header = {0};
    struct MemoryGraphicsDatStartupTickResult_Compat tickResult;
    unsigned int viewportBytes;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportBitmap;
    int ok;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [dialog_index] [viewport_index]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    if (argc >= 3) {
        dialogGraphicIndex = (unsigned int)strtoul(argv[2], 0, 10);
    }
    if (argc >= 4) {
        viewportGraphicIndex = (unsigned int)strtoul(argv[3], 0, 10);
    }

    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsDatPath, &fileState, &header)) {
        fprintf(stderr, "failed: could not load GRAPHICS.DAT header from %s\n", graphicsDatPath);
        return 1;
    }
    if ((dialogGraphicIndex >= header.graphicCount) || (viewportGraphicIndex >= header.graphicCount)) {
        fprintf(stderr, "failed: candidate out of range (graphicCount=%u, dialog=%u, viewport=%u)\n",
            (unsigned int)header.graphicCount,
            dialogGraphicIndex,
            viewportGraphicIndex);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    viewportBytes = header.decompressedByteCounts[viewportGraphicIndex];
    if (viewportBytes < 4096U) {
        viewportBytes = 4096U;
    }
    viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes + 4096U, 1);
    viewportBitmap = (unsigned char*)calloc((size_t)viewportBytes + 4096U, 1);
    if ((viewportGraphicBuffer == 0) || (viewportBitmap == 0)) {
        fprintf(stderr, "failed: could not allocate probe buffers\n");
        free(viewportGraphicBuffer);
        free(viewportBitmap);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    memset(&tickResult, 0, sizeof(tickResult));
    ok = F0479_MEMORY_RunStartupTickMini_Compat(
        graphicsDatPath,
        &fileState,
        dialogGraphicIndex,
        viewportGraphicIndex,
        viewportGraphicBuffer,
        viewportBitmap,
        &tickResult);

    if (!ok) {
        fprintf(stderr, "failed: startup tick returned 0 for dialog=%u viewport=%u\n",
            dialogGraphicIndex,
            viewportGraphicIndex);
        free(viewportGraphicBuffer);
        free(viewportBitmap);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("graphicCount=%u\n", (unsigned int)header.graphicCount);
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("viewportGraphicIndex=%u\n", viewportGraphicIndex);
    printf("viewportCompressedBytes=%u\n", (unsigned int)header.compressedByteCounts[viewportGraphicIndex]);
    printf("viewportDecompressedBytes=%u\n", (unsigned int)header.decompressedByteCounts[viewportGraphicIndex]);
    printf("startupTickStage=%d\n", (int)tickResult.stage);
    printf("mainLoopStage=%d\n", (int)tickResult.mainLoop.stage);
    printf("dispatchStage=%d\n", (int)tickResult.mainLoop.dispatch.stage);
    printf("frameStage=%d\n", (int)tickResult.mainLoop.dispatch.firstFrame.stage);
    printf("bootStage=%d\n", (int)tickResult.mainLoop.dispatch.firstFrame.startup.boot.stage);
    printf("dialogPreloaded=%d\n", tickResult.mainLoop.dispatch.firstFrame.startup.boot.startup.dialogPreloaded);
    printf("viewportLoaded=%d\n", tickResult.mainLoop.dispatch.firstFrame.startup.boot.startup.viewportLoaded);
    printf("startupTickCompleted=%d\n", tickResult.startupTickCompleted);

    F0479_MEMORY_FreeStartupTickMini_Compat(&tickResult);
    free(viewportGraphicBuffer);
    free(viewportBitmap);
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 0;
}
