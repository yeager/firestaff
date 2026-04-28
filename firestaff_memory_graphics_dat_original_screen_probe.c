#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_bitmap_path_pc34_compat.h"
#include "screen_bitmap_present_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned short read_u16_le(const unsigned char* p) {
    return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static unsigned long storage_byte_count_from_size(unsigned short width, unsigned short height) {
    unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    return (unsigned long)bytesPerRow * (unsigned long)height;
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    unsigned int graphicIndex = 0;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat bitmapResult;
    struct ScreenBitmapPresentResult_Compat presentResult;
    unsigned int rawBufferBytes;
    unsigned long bitmapBufferBytes;
    unsigned char* rawGraphic;
    unsigned char* ownedStorage;
    unsigned char* screenStorage;
    unsigned char* ownedBitmap;
    unsigned char* screenBitmap;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [graphic_index]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    if (argc >= 3) {
        graphicIndex = (unsigned int)strtoul(argv[2], 0, 10);
    }

    memset(&runtimeState, 0, sizeof(runtimeState));
    memset(&bitmapResult, 0, sizeof(bitmapResult));
    memset(&presentResult, 0, sizeof(presentResult));
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
    if (rawBufferBytes < 4096U) {
        rawBufferBytes = 4096U;
    }
    bitmapBufferBytes = storage_byte_count_from_size(
        runtimeState.widthHeight[graphicIndex].Width,
        runtimeState.widthHeight[graphicIndex].Height);
    rawGraphic = (unsigned char*)calloc((size_t)rawBufferBytes + 4096U, 1);
    ownedStorage = (unsigned char*)calloc((size_t)bitmapBufferBytes + 8192U, 1);
    screenStorage = (unsigned char*)calloc((size_t)bitmapBufferBytes + 8192U, 1);
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
    if (memcmp(ownedBitmap - 4, screenBitmap - 4, (size_t)presentResult.copiedByteCount + 4U) != 0) {
        fprintf(stderr, "failed: screen surface mismatch for graphic %u\n", graphicIndex);
        free(rawGraphic);
        free(ownedStorage);
        free(screenStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("graphicIndex=%u\n", graphicIndex);
    printf("graphicCount=%u\n", (unsigned int)runtimeState.graphicCount);
    printf("width=%u\n", (unsigned int)read_u16_le(screenBitmap - 4));
    printf("height=%u\n", (unsigned int)read_u16_le(screenBitmap - 2));
    printf("screenCopiedBytes=%lu\n", presentResult.copiedByteCount);
    printf("compressedBytes=%u\n", (unsigned int)bitmapResult.selection.compressedByteCount);
    printf("offset=%ld\n", bitmapResult.selection.offset);

    free(rawGraphic);
    free(ownedStorage);
    free(screenStorage);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 0;
}
