#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_bitmap_reuse_pc34_compat.h"

static int gBuildCallCount;
static int gApplyCallCount;

void F0490_MEMORY_ApplyLoadedGraphic_Compat(
    int graphicIndexFlags,
    const unsigned char* loadedGraphic,
    unsigned long loadedByteCount,
    unsigned char* destinationBitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    (void)graphicIndexFlags;
    (void)sizeInfo;
    gApplyCallCount++;
    memcpy(destinationBitmap, loadedGraphic, (size_t)loadedByteCount);
}

unsigned char* F0489_MEMORY_BuildNativeBitmapInPlace_Compat(
    const unsigned char* graphic,
    unsigned char* bitmap,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    (void)sizeInfo;
    gBuildCallCount++;
    memcpy(bitmap, graphic, 5);
    return bitmap;
}

static int write_u16(FILE* f, unsigned short value) {
    unsigned char bytes[2];
    bytes[0] = (unsigned char)(value & 0xFF);
    bytes[1] = (unsigned char)((value >> 8) & 0xFF);
    return fwrite(bytes, 1, 2, f) == 2;
}

static int write_format1_file(const char* path) {
    FILE* f = fopen(path, "wb");
    unsigned char payloadA[3] = {0x10, 0x11, 0x12};
    unsigned char payloadB[5] = {0x20, 0x21, 0x22, 0x23, 0x24};
    if (f == 0) {
        return 0;
    }
    if (!write_u16(f, 0x8001)
     || !write_u16(f, 2)
     || !write_u16(f, 3)
     || !write_u16(f, 5)
     || !write_u16(f, 13)
     || !write_u16(f, 25)
     || !write_u16(f, 7)
     || !write_u16(f, 8)
     || !write_u16(f, 9)
     || !write_u16(f, 10)
     || fwrite(payloadA, 1, sizeof(payloadA), f) != sizeof(payloadA)
     || fwrite(payloadB, 1, sizeof(payloadB), f) != sizeof(payloadB)) {
        fclose(f);
        return 0;
    }
    fclose(f);
    return 1;
}

static int test_bitmap_reuse_returns_cached_bitmap_without_rebuild(void) {
    const char* path = "./test_graphics_dat_bitmap_reuse.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    unsigned char loadedGraphic[32] = {0};
    unsigned char cachedBitmap[32] = {0xAA, 0xBB, 0xCC};
    unsigned char targetBitmap[32] = {0};
    unsigned char* returned;

    gBuildCallCount = 0;
    gApplyCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    returned = F0489_MEMORY_GetOrLoadNativeBitmapByIndex_Compat(
        path,
        &runtimeState,
        &fileState,
        1,
        loadedGraphic,
        cachedBitmap,
        targetBitmap,
        &result);
    if (returned != cachedBitmap || result.bitmap != cachedBitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gBuildCallCount != 0 || gApplyCallCount != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (result.selection.offset != 23 || result.selection.compressedByteCount != 5 || result.selection.widthHeight.Width != 9) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

static int test_bitmap_reuse_loads_and_builds_when_cache_missing(void) {
    const char* path = "./test_graphics_dat_bitmap_reuse.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    unsigned char loadedGraphic[32] = {0};
    unsigned char targetBitmap[32] = {0};
    unsigned char* returned;
    int i;

    gBuildCallCount = 0;
    gApplyCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    returned = F0489_MEMORY_GetOrLoadNativeBitmapByIndex_Compat(
        path,
        &runtimeState,
        &fileState,
        1,
        loadedGraphic,
        0,
        targetBitmap,
        &result);
    if (returned != targetBitmap || result.bitmap != targetBitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gBuildCallCount != 1 || gApplyCallCount != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (loadedGraphic[i] != (unsigned char)(0x20 + i) || targetBitmap[i] != loadedGraphic[i]) {
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_bitmap_reuse_returns_cached_bitmap_without_rebuild()) {
        fprintf(stderr, "test_bitmap_reuse_returns_cached_bitmap_without_rebuild failed\n");
        return 1;
    }
    if (!test_bitmap_reuse_loads_and_builds_when_cache_missing()) {
        fprintf(stderr, "test_bitmap_reuse_loads_and_builds_when_cache_missing failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
