#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_startup_tick_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

void F0490_MEMORY_ApplyLoadedGraphic_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    (void)graphicIndexFlags;
    (void)sizeInfo;
    memcpy(destinationBitmap, loadedGraphic, (size_t)loadedByteCount);
}

static int try_candidate(
    const char* graphicsDatPath,
    unsigned int dialogGraphicIndex,
    unsigned int viewportGraphicIndex,
    struct MemoryGraphicsDatStartupTickResult_Compat* tickResult,
    struct MemoryGraphicsDatHeader_Compat* header) {
    struct MemoryGraphicsDatState_Compat fileState = {0};
    unsigned int viewportBytes;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportBitmap;
    int ok;

    viewportBytes = header->compressedByteCounts[viewportGraphicIndex];
    if (viewportBytes < 16U) {
        viewportBytes = 16U;
    }
    viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes + 16U, 1);
    viewportBitmap = (unsigned char*)calloc((size_t)viewportBytes + 64U, 1);
    if ((viewportGraphicBuffer == 0) || (viewportBitmap == 0)) {
        free(viewportGraphicBuffer);
        free(viewportBitmap);
        return 0;
    }
    ok = F0479_MEMORY_RunStartupTickMini_Compat(
        graphicsDatPath,
        &fileState,
        dialogGraphicIndex,
        viewportGraphicIndex,
        viewportGraphicBuffer,
        viewportBitmap,
        tickResult);
    free(viewportGraphicBuffer);
    free(viewportBitmap);
    return ok;
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatHeader_Compat header = {0};
    struct MemoryGraphicsDatStartupTickResult_Compat tickResult;
    unsigned int dialogStart;
    unsigned int dialogLimit;
    unsigned int viewportLimit;
    unsigned int dialogIndex;
    unsigned int viewportIndex;
    unsigned int attempts;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [max_indices]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsDatPath, &fileState, &header)) {
        fprintf(stderr, "failed: could not load GRAPHICS.DAT header from %s\n", graphicsDatPath);
        return 1;
    }
    if (header.graphicCount == 0) {
        fprintf(stderr, "failed: GRAPHICS.DAT contains zero graphics\n");
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    viewportLimit = header.graphicCount;
    if (argc >= 3) {
        unsigned long parsed = strtoul(argv[2], 0, 10);
        if ((parsed > 0UL) && (parsed < viewportLimit)) {
            viewportLimit = (unsigned int)parsed;
        }
    } else if (viewportLimit > 32U) {
        viewportLimit = 32U;
    }
    dialogStart = (header.graphicCount > 1U) ? 1U : 0U;
    dialogLimit = header.graphicCount;
    if (dialogLimit > 4U) {
        dialogLimit = 4U;
    }

    attempts = 0;
    for (dialogIndex = dialogStart; dialogIndex < dialogLimit; ++dialogIndex) {
        for (viewportIndex = 0; viewportIndex < viewportLimit; ++viewportIndex) {
            memset(&tickResult, 0, sizeof(tickResult));
            attempts++;
            if (!try_candidate(graphicsDatPath, dialogIndex, viewportIndex, &tickResult, &header)) {
                continue;
            }
            printf("ok\n");
            printf("graphicsDatPath=%s\n", graphicsDatPath);
            printf("graphicCount=%u\n", (unsigned int)header.graphicCount);
            printf("attempts=%u\n", attempts);
            printf("dialogGraphicIndex=%u\n", dialogIndex);
            printf("viewportGraphicIndex=%u\n", viewportIndex);
            printf("startupTickStage=%d\n", (int)tickResult.stage);
            printf("mainLoopStage=%d\n", (int)tickResult.mainLoop.stage);
            printf("dispatchStage=%d\n", (int)tickResult.mainLoop.dispatch.stage);
            printf("frameStage=%d\n", (int)tickResult.mainLoop.dispatch.firstFrame.stage);
            printf("bootStage=%d\n", (int)tickResult.mainLoop.dispatch.firstFrame.startup.boot.stage);
            printf("runtimeInitialized=%d\n", tickResult.mainLoop.dispatch.firstFrame.startup.boot.startup.runtimeInitialized);
            printf("dialogPreloaded=%d\n", tickResult.mainLoop.dispatch.firstFrame.startup.boot.startup.dialogPreloaded);
            printf("viewportLoaded=%d\n", tickResult.mainLoop.dispatch.firstFrame.startup.boot.startup.viewportLoaded);
            printf("firstFramePrerequisitesReady=%d\n", tickResult.mainLoop.dispatch.firstFrame.startup.firstFramePrerequisitesReady);
            printf("startupDispatched=%d\n", tickResult.mainLoop.dispatch.startupDispatched);
            printf("mainLoopEntered=%d\n", tickResult.mainLoop.mainLoopEntered);
            printf("startupTickCompleted=%d\n", tickResult.startupTickCompleted);
            F0479_MEMORY_FreeStartupTickMini_Compat(&tickResult);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            return 0;
        }
    }

    fprintf(stderr, "failed: no startup candidate succeeded in %u attempts (dialog range %u..%u, viewport range 0..%u)\n",
        attempts,
        dialogStart,
        (dialogLimit == 0U) ? 0U : (dialogLimit - 1U),
        (viewportLimit == 0U) ? 0U : (viewportLimit - 1U));
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 1;
}
