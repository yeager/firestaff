#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_main_loop_command_loop_pc34_compat.h"

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
    enum MemoryGraphicsDatMainLoopCommand_Compat command,
    unsigned int commandCount,
    struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat* loopResult,
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
    ok = F0479_MEMORY_RunMainLoopCommandLoopMini_Compat(
        graphicsDatPath,
        &fileState,
        dialogGraphicIndex,
        viewportGraphicIndex,
        viewportGraphicBuffer,
        viewportBitmap,
        command,
        commandCount,
        loopResult);
    free(viewportGraphicBuffer);
    free(viewportBitmap);
    return ok;
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatHeader_Compat header = {0};
    struct MemoryGraphicsDatMainLoopCommandLoopResult_Compat loopResult;
    unsigned int dialogStart;
    unsigned int dialogLimit;
    unsigned int viewportLimit;
    unsigned int dialogIndex;
    unsigned int viewportIndex;
    unsigned int attempts;
    unsigned int commandCount = 2;
    enum MemoryGraphicsDatMainLoopCommand_Compat command = MEMORY_GRAPHICS_DAT_MAIN_LOOP_COMMAND_TICK;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [max_indices] [command_count]\n", argv[0]);
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
    if (argc >= 4) {
        commandCount = (unsigned int)strtoul(argv[3], 0, 10);
        if (commandCount == 0U) commandCount = 1U;
    }
    dialogStart = (header.graphicCount > 1U) ? 1U : 0U;
    dialogLimit = header.graphicCount;
    if (dialogLimit > 4U) dialogLimit = 4U;

    attempts = 0;
    for (dialogIndex = dialogStart; dialogIndex < dialogLimit; ++dialogIndex) {
        for (viewportIndex = 0; viewportIndex < viewportLimit; ++viewportIndex) {
            memset(&loopResult, 0, sizeof(loopResult));
            attempts++;
            if (!try_candidate(graphicsDatPath, dialogIndex, viewportIndex, command, commandCount, &loopResult, &header)) {
                continue;
            }
            printf("ok\n");
            printf("graphicsDatPath=%s\n", graphicsDatPath);
            printf("graphicCount=%u\n", (unsigned int)header.graphicCount);
            printf("attempts=%u\n", attempts);
            printf("dialogGraphicIndex=%u\n", dialogIndex);
            printf("viewportGraphicIndex=%u\n", viewportIndex);
            printf("command=%d\n", (int)loopResult.command);
            printf("commandLoopStage=%d\n", (int)loopResult.stage);
            printf("requestedCommandCount=%u\n", loopResult.requestedCommandCount);
            printf("completedCommandCount=%u\n", loopResult.completedCommandCount);
            printf("handledCommandCount=%u\n", loopResult.handledCommandCount);
            printf("commandLoopReady=%d\n", loopResult.commandLoopReady);
            printf("commandLoopCompleted=%d\n", loopResult.commandLoopCompleted);
            printf("lastCommandHandled=%d\n", loopResult.lastCommand.commandHandled);
            F0479_MEMORY_FreeMainLoopCommandLoopMini_Compat(&loopResult);
            F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
            return 0;
        }
    }

    fprintf(stderr, "failed: no main-loop command-loop candidate succeeded in %u attempts\n", attempts);
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 1;
}
