#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "graphics_dat_entry_classify_pc34_compat.h"

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
    unsigned int startIndex;
    unsigned int count;
    unsigned int i;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct GraphicsDatEntryClassificationResult_Compat result;

    if (argc < 4) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT start count\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    startIndex = (unsigned int)strtoul(argv[2], 0, 10);
    count = (unsigned int)strtoul(argv[3], 0, 10);

    memset(&runtimeState, 0, sizeof(runtimeState));
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(graphicsDatPath, &fileState, &runtimeState)) {
        fprintf(stderr, "failed: initialize runtime state\n");
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    for (i = 0; i < count; ++i) {
        unsigned int index = startIndex + i;
        if (index >= runtimeState.graphicCount) {
            break;
        }
        memset(&result, 0, sizeof(result));
        if (!F9012_RUNTIME_ClassifyGraphicsDatEntry_Compat(&runtimeState, index, &result)) {
            printf("graphic=%u classify_failed\n", index);
            continue;
        }
        printf("graphic=%u kind=%s compressed=%u decompressed=%u width=%u height=%u useBitmap=%d skipExport=%d\n",
               index,
               kind_name(result.kind),
               (unsigned int)runtimeState.compressedByteCounts[index],
               (unsigned int)runtimeState.decompressedByteCounts[index],
               (unsigned int)runtimeState.widthHeight[index].Width,
               (unsigned int)runtimeState.widthHeight[index].Height,
               result.shouldUseBitmapPath,
               result.shouldSkipBitmapExport);
    }

    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 0;
}
