#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_bitmap_path_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned char read_nibble(const unsigned char* src, unsigned short nibbleOffset) {
    unsigned char packed = src[nibbleOffset >> 1];
    return (nibbleOffset & 1) ? (packed & 0x0F) : (packed >> 4);
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapResult;
    unsigned int graphicIndex;
    unsigned int rawBufferBytes;
    unsigned char* rawGraphic;
    unsigned char* ownedStorage;
    unsigned char* ownedBitmap;
    unsigned int i;

    if (argc < 3) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT graphic_index\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    graphicIndex = (unsigned int)strtoul(argv[2], 0, 10);

    memset(&runtimeState, 0, sizeof(runtimeState));
    memset(&bitmapResult, 0, sizeof(bitmapResult));
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(graphicsDatPath, &fileState, &runtimeState)) {
        fprintf(stderr, "failed: could not initialize runtime state from %s\n", graphicsDatPath);
        return 1;
    }
    if (graphicIndex >= runtimeState.graphicCount) {
        fprintf(stderr, "failed: graphic index %u out of range (graphicCount=%u)\n", graphicIndex, (unsigned int)runtimeState.graphicCount);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    rawBufferBytes = runtimeState.decompressedByteCounts[graphicIndex];
    if (rawBufferBytes < 4096U) rawBufferBytes = 4096U;
    rawGraphic = (unsigned char*)calloc((size_t)rawBufferBytes + 4096U, 1);
    ownedStorage = (unsigned char*)calloc((size_t)rawBufferBytes + 8192U, 1);
    if ((rawGraphic == 0) || (ownedStorage == 0)) {
        fprintf(stderr, "failed: allocation error\n");
        free(rawGraphic);
        free(ownedStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    ownedBitmap = ownedStorage + 4;
    if (F0489_MEMORY_LoadNativeBitmapByIndex_Compat(
            graphicsDatPath,
            &runtimeState,
            &fileState,
            graphicIndex,
            rawGraphic,
            ownedBitmap,
            &bitmapResult) != ownedBitmap) {
        fprintf(stderr, "failed: could not load graphic %u\n", graphicIndex);
        free(rawGraphic);
        free(ownedStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    printf("ok\n");
    printf("graphicIndex=%u\n", graphicIndex);
    printf("width=%u\n", (unsigned int)bitmapResult.selection.widthHeight.Width);
    printf("height=%u\n", (unsigned int)bitmapResult.selection.widthHeight.Height);
    printf("paletteNibbles=");
    for (i = 0; i < 6; ++i) {
        if (i) printf(",");
        printf("%u", (unsigned int)read_nibble(rawGraphic, (unsigned short)(8 + i)));
    }
    printf("\n");
    printf("headerBytes=%u,%u,%u,%u\n",
           (unsigned int)rawGraphic[0],
           (unsigned int)rawGraphic[1],
           (unsigned int)rawGraphic[2],
           (unsigned int)rawGraphic[3]);

    free(rawGraphic);
    free(ownedStorage);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 0;
}
