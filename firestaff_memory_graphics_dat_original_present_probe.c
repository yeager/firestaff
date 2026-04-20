#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "memory_graphics_dat_bitmap_path_pc34_compat.h"
#include "bitmap_copy_pc34_compat.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static unsigned short read_u16_le(const unsigned char* p) {
    return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static unsigned long bitmap_byte_count(const unsigned char* bitmap) {
    unsigned short width = read_u16_le(bitmap - 4);
    unsigned short height = read_u16_le(bitmap - 2);
    unsigned short bytesPerRow = (unsigned short)(((width + 1) & 0xFFFE) >> 1);
    return (unsigned long)bytesPerRow * (unsigned long)height;
}

int main(int argc, char** argv) {
    const char* graphicsDatPath;
    unsigned int graphicIndex = 0;
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    unsigned int rawBufferBytes;
    unsigned char* rawGraphic;
    unsigned char* ownedStorage;
    unsigned char* presentStorage;
    unsigned char* ownedBitmap;
    unsigned char* presentBitmap;
    unsigned long presentedBytes;
    int ok;

    if (argc < 2) {
        fprintf(stderr, "usage: %s /path/to/GRAPHICS.DAT [graphic_index]\n", argv[0]);
        return 2;
    }
    graphicsDatPath = argv[1];
    if (argc >= 3) {
        graphicIndex = (unsigned int)strtoul(argv[2], 0, 10);
    }

    memset(&runtimeState, 0, sizeof(runtimeState));
    memset(&result, 0, sizeof(result));
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
    rawGraphic = (unsigned char*)calloc((size_t)rawBufferBytes + 4096U, 1);
    ownedStorage = (unsigned char*)calloc((size_t)rawBufferBytes + 8192U, 1);
    presentStorage = (unsigned char*)calloc((size_t)rawBufferBytes + 8192U, 1);
    if ((rawGraphic == 0) || (ownedStorage == 0) || (presentStorage == 0)) {
        fprintf(stderr, "failed: allocation error\n");
        free(rawGraphic);
        free(ownedStorage);
        free(presentStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    ownedBitmap = ownedStorage + 4;
    presentBitmap = presentStorage + 4;
    ok = (F0489_MEMORY_LoadNativeBitmapByIndex_Compat(
            graphicsDatPath,
            &runtimeState,
            &fileState,
            graphicIndex,
            rawGraphic,
            ownedBitmap,
            &result) == ownedBitmap);
    if (!ok) {
        fprintf(stderr, "failed: could not build native bitmap for graphic %u\n", graphicIndex);
        free(rawGraphic);
        free(ownedStorage);
        free(presentStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    F0616_CopyBitmap_Compat(ownedBitmap, presentBitmap);
    presentedBytes = bitmap_byte_count(ownedBitmap);
    if (memcmp(ownedBitmap - 4, presentBitmap - 4, (size_t)presentedBytes + 4U) != 0) {
        fprintf(stderr, "failed: present surface copy mismatch for graphic %u\n", graphicIndex);
        free(rawGraphic);
        free(ownedStorage);
        free(presentStorage);
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 1;
    }

    printf("ok\n");
    printf("graphicsDatPath=%s\n", graphicsDatPath);
    printf("graphicIndex=%u\n", graphicIndex);
    printf("graphicCount=%u\n", (unsigned int)runtimeState.graphicCount);
    printf("width=%u\n", (unsigned int)read_u16_le(ownedBitmap - 4));
    printf("height=%u\n", (unsigned int)read_u16_le(ownedBitmap - 2));
    printf("presentedBytes=%lu\n", presentedBytes);
    printf("compressedBytes=%u\n", (unsigned int)result.selection.compressedByteCount);
    printf("offset=%ld\n", result.selection.offset);

    free(rawGraphic);
    free(ownedStorage);
    free(presentStorage);
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 0;
}
