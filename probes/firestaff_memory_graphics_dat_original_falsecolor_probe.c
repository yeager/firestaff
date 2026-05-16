#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_bitmap_path_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"
#include "screen_bitmap_export_ppm_falsecolor_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    const char* outputPath;
    unsigned int graphicIndex = 0;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapResult;
    struct ScreenBitmapPresentResult_Compat presentResult;
    struct ScreenBitmapExportPpmFalsecolorResult_Compat exportResult;
    unsigned int rawBufferBytes;
    unsigned char* rawGraphic;
    unsigned char* ownedStorage;
    unsigned char* screenStorage;
    unsigned char* ownedBitmap;
    unsigned char* screenBitmap;

    if (argc < 4) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT /path/to/output.ppm graphic_index\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    outputPath = argv[2];
    graphicIndex = (unsigned int)strtoul(argv[3], 0, 10);

    memset(&runtimeState, 0, sizeof(runtimeState));
    memset(&bitmapResult, 0, sizeof(bitmapResult));
    memset(&presentResult, 0, sizeof(presentResult));
    memset(&exportResult, 0, sizeof(exportResult));
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
    screenStorage = (unsigned char*)calloc((size_t)rawBufferBytes + 8192U, 1);
    if ((rawGraphic == 0) || (ownedStorage == 0) || (screenStorage == 0)) {
        fprintf(stderr, "failed: allocation error\n");
        free(rawGraphic);
        free(ownedStorage);
        free(screenStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    ownedBitmap = ownedStorage + 4;
    screenBitmap = screenStorage + 4;
    if (F0489_MEMORY_LoadNativeBitmapByIndex_Compat(
            graphicsDatPath,
            &runtimeState,
            &fileState,
            graphicIndex,
            rawGraphic,
            ownedBitmap,
            &bitmapResult) != ownedBitmap) {
        fprintf(stderr, "failed: could not build native bitmap for graphic %u\n", graphicIndex);
        free(rawGraphic);
        free(ownedStorage);
        free(screenStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }
    if (!F9000_SCREEN_PresentBitmapToScreen_Compat(ownedBitmap, screenBitmap, &presentResult)) {
        fprintf(stderr, "failed: could not present bitmap to screen for graphic %u\n", graphicIndex);
        free(rawGraphic);
        free(ownedStorage);
        free(screenStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }
    if (!F9003_SCREEN_ExportBitmapToPpmFalsecolor_Compat(screenBitmap, outputPath, &exportResult)) {
        fprintf(stderr, "failed: could not export falsecolor bitmap to %s\n", outputPath);
        free(rawGraphic);
        free(ownedStorage);
        free(screenStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    printf("ok\n");
    printf("graphicIndex=%u\n", graphicIndex);
    printf("outputPath=%s\n", outputPath);
    printf("width=%u\n", (unsigned int)exportResult.width);
    printf("height=%u\n", (unsigned int)exportResult.height);

    free(rawGraphic);
    free(ownedStorage);
    free(screenStorage);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 0;
}
