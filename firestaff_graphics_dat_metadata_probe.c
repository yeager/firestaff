#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_state_pc34_compat.h"
#include "memory_graphics_dat_select_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    unsigned int startIndex;
    unsigned int count;
    unsigned int i;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatHeader_Compat header;
    struct MemoryGraphicsDatSelection_Compat selection;

    if (argc < 4) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT start count\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    startIndex = (unsigned int)strtoul(argv[2], 0, 10);
    count = (unsigned int)strtoul(argv[3], 0, 10);

    memset(&runtimeState, 0, sizeof(runtimeState));
    memset(&header, 0, sizeof(header));
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(graphicsDatPath, &fileState, &runtimeState)) {
        fprintf(stderr, "failed: initialize runtime state\n");
        return 1;
    }
    if (!F0479_MEMORY_LoadGraphicsDatHeader_Compat(graphicsDatPath, &fileState, &header)) {
        fprintf(stderr, "failed: load header\n");
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("graphicCount=%u\n", (unsigned int)runtimeState.graphicCount);
    for (i = 0; i < count; ++i) {
        unsigned int index = startIndex + i;
        if (index >= runtimeState.graphicCount) {
            break;
        }
        memset(&selection, 0, sizeof(selection));
        if (!F0490_MEMORY_SelectGraphicFromHeader_Compat(&header, index, &selection)) {
            printf("graphic=%u select_failed\n", index);
            continue;
        }
        printf("graphic=%u offset=%ld compressed=%u decompressed=%u metaWidth=%u metaHeight=%u runtimeWidth=%u runtimeHeight=%u\n",
               index,
               selection.offset,
               (unsigned int)selection.compressedByteCount,
               (unsigned int)selection.decompressedByteCount,
               (unsigned int)selection.widthHeight.Width,
               (unsigned int)selection.widthHeight.Height,
               (unsigned int)runtimeState.widthHeight[index].Width,
               (unsigned int)runtimeState.widthHeight[index].Height);
    }

    F0479_MEMORY_FreeGraphicsDatHeader_Compat(&header);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 0;
}
