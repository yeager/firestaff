#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_header_pc34_compat.h"
#include "memory_graphics_dat_menu_activate_consequence_pc34_compat.h"

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

static int parse_events(
    const char* text,
    enum MemoryGraphicsDatEvent_Compat* events,
    unsigned int capacity,
    unsigned int* outCount) {
    unsigned int count = 0;
    const unsigned char* p = (const unsigned char*)text;

    while (*p != '\0') {
        unsigned char c = (unsigned char)tolower(*p++);
        enum MemoryGraphicsDatEvent_Compat event;

        if ((c == ' ') || (c == '\t') || (c == '\n') || (c == '\r') || (c == ',') || (c == ';') || (c == '-')) {
            continue;
        }
        if (count >= capacity) {
            return 0;
        }
        switch (c) {
            case 'f': event = MEMORY_GRAPHICS_DAT_EVENT_FRAME; break;
            case 'n': event = MEMORY_GRAPHICS_DAT_EVENT_ADVANCE; break;
            case 'a': event = MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE; break;
            case 'i': event = MEMORY_GRAPHICS_DAT_EVENT_IDLE; break;
            default: return 0;
        }
        events[count++] = event;
    }
    *outCount = count;
    return (count > 0U);
}

static const char* event_name(enum MemoryGraphicsDatEvent_Compat event) {
    switch (event) {
        case MEMORY_GRAPHICS_DAT_EVENT_IDLE: return "IDLE";
        case MEMORY_GRAPHICS_DAT_EVENT_FRAME: return "FRAME";
        case MEMORY_GRAPHICS_DAT_EVENT_ADVANCE: return "ADVANCE";
        case MEMORY_GRAPHICS_DAT_EVENT_ACTIVATE: return "ACTIVATE";
        default: return "UNKNOWN";
    }
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    char inputBuffer[512];
    const char* eventSpec;
    unsigned int dialogGraphicIndex = 1;
    unsigned int viewportGraphicIndex = 0;
    unsigned int initialSelectionIndex = 0;
    unsigned int selectionCount = 3;
    unsigned int highlightBaseGraphicIndex = 1;
    unsigned int activateGraphicBaseIndex = 11;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatHeader_Compat header = {0};
    struct MemoryGraphicsDatMenuActivateConsequenceResult_Compat result;
    enum MemoryGraphicsDatEvent_Compat events[256];
    unsigned int eventCount = 0;
    unsigned int viewportBytes;
    unsigned char* viewportGraphicBuffer;
    unsigned char* viewportBitmap;
    unsigned int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [event_spec] [dialog_index] [viewport_index]\n", argv[0]);
        fprintf(stderr, "event chars: f=frame n=advance a=activate i=idle\n");
        return 2;
    }
    graphicsDatPath = argv[1];
    if (argc >= 3) {
        eventSpec = argv[2];
    } else {
        printf("enter event sequence (f=frame, n=advance, a=activate, i=idle), example: fnnaf\n> ");
        fflush(stdout);
        if (fgets(inputBuffer, sizeof(inputBuffer), stdin) == 0) {
            fprintf(stderr, "failed: no event sequence provided\n");
            return 1;
        }
        eventSpec = inputBuffer;
    }
    if (argc >= 4) dialogGraphicIndex = (unsigned int)strtoul(argv[3], 0, 10);
    if (argc >= 5) viewportGraphicIndex = (unsigned int)strtoul(argv[4], 0, 10);

    if (!parse_events(eventSpec, events, 256U, &eventCount)) {
        fprintf(stderr, "failed: could not parse event sequence '%s'\n", eventSpec);
        return 1;
    }
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsDatPath, &fileState, &header)) {
        fprintf(stderr, "failed: could not load GRAPHICS.DAT header from %s\n", graphicsDatPath);
        return 1;
    }
    if ((dialogGraphicIndex >= header.graphicCount) || (viewportGraphicIndex >= header.graphicCount)) {
        fprintf(stderr, "failed: dialog/viewport index out of range (graphicCount=%u)\n", (unsigned int)header.graphicCount);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    viewportBytes = header.compressedByteCounts[viewportGraphicIndex];
    if (viewportBytes < 16U) viewportBytes = 16U;
    viewportGraphicBuffer = (unsigned char*)calloc((size_t)viewportBytes + 16U, 1);
    viewportBitmap = (unsigned char*)calloc((size_t)viewportBytes + 64U, 1);
    if ((viewportGraphicBuffer == 0) || (viewportBitmap == 0)) {
        fprintf(stderr, "failed: out of memory allocating viewport buffers\n");
        free(viewportGraphicBuffer);
        free(viewportBitmap);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    memset(&result, 0, sizeof(result));
    if (!F0479_MEMORY_RunMenuActivateConsequenceMini_Compat(
            graphicsDatPath,
            &fileState,
            dialogGraphicIndex,
            viewportGraphicIndex,
            viewportGraphicBuffer,
            viewportBitmap,
            events,
            eventCount,
            initialSelectionIndex,
            selectionCount,
            highlightBaseGraphicIndex,
            activateGraphicBaseIndex,
            MEMORY_GRAPHICS_DAT_MENU_SCREEN_MENU,
            &result)) {
        fprintf(stderr, "failed: menu input shim run did not complete\n");
        free(viewportGraphicBuffer);
        free(viewportBitmap);
        F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("dialogGraphicIndex=%u\n", dialogGraphicIndex);
    printf("viewportGraphicIndex=%u\n", viewportGraphicIndex);
    printf("eventCount=%u\n", eventCount);
    for (i = 0; i < eventCount; ++i) {
        printf("event[%u]=%s\n", i, event_name(events[i]));
    }
    printf("initialSelectionIndex=%u\n", result.activate.render.menuState.initialSelectionIndex);
    printf("finalSelectionIndex=%u\n", result.activate.render.menuState.finalSelectionIndex);
    printf("advanceTransitionCount=%u\n", result.activate.render.menuState.advanceTransitionCount);
    printf("frameCount=%u\n", result.activate.render.menuState.frameCount);
    printf("selectedRenderVariant=%u\n", result.activate.render.selectedRenderVariant);
    printf("highlightedGraphicIndex=%u\n", result.activate.render.highlightedGraphicIndex);
    printf("activationTriggered=%d\n", result.activate.activationTriggered);
    printf("activatedSelectionIndex=%u\n", result.activate.activatedSelectionIndex);
    printf("activatedGraphicIndex=%u\n", result.activate.activatedGraphicIndex);
    printf("initialScreen=%u\n", (unsigned int)result.initialScreen);
    printf("finalScreen=%u\n", (unsigned int)result.finalScreen);
    printf("screenChanged=%d\n", result.screenChanged);

    F0479_MEMORY_FreeMenuActivateConsequenceMini_Compat(&result);
    free(viewportGraphicBuffer);
    free(viewportBitmap);
    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    return 0;
}
