#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_block_index_pc34_compat.h"

#define TEST_BLOCK_STORAGE_SIZE (sizeof(struct NativeBitmapBlock_Compat) + 32)

static int gBuildCallCount;
static int gApplyCallCount;
static int gPrepareCallCount;
static unsigned short gPreparedBitmapIndex;
static struct GraphicWidthHeight_Compat gPreparedSizeInfo;

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

unsigned char* F0489_MEMORY_PrepareNativeBitmapBlock_Compat(
    struct NativeBitmapBlock_Compat* block,
    unsigned short bitmapIndex,
    const struct GraphicWidthHeight_Compat* sizeInfo) {
    gPrepareCallCount++;
    gPreparedBitmapIndex = bitmapIndex;
    gPreparedSizeInfo = *sizeInfo;
    block->bitmapIndex = bitmapIndex;
    block->width = sizeInfo->Width;
    block->height = sizeInfo->Height;
    return block->bitmap;
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

static int test_block_index_returns_existing_bitmap_on_hit(void) {
    const char* path = "./test_graphics_dat_block_index.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    unsigned short indices[2] = {0xFFFF, 0};
    unsigned char loadedGraphic[32] = {0};
    unsigned char blockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    struct NativeBitmapBlock_Compat* blockStorage = (struct NativeBitmapBlock_Compat*)blockBytes;
    struct NativeBitmapBlock_Compat* blocks[2] = {blockStorage, 0};
    blockStorage->bitmap[0] = 0x7E;

    gBuildCallCount = 0;
    gApplyCallCount = 0;
    gPrepareCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (F0489_MEMORY_GetOrLoadBitmapByBlockIndex_Compat(
            path,
            &runtimeState,
            &fileState,
            1,
            loadedGraphic,
            indices,
            blocks,
            2,
            blockStorage,
            &result) != blockStorage->bitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gBuildCallCount != 0 || gApplyCallCount != 0 || gPrepareCallCount != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (result.bitmap != blockStorage->bitmap || result.selection.offset != 23) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

static int test_block_index_prepares_new_block_and_loads_on_miss(void) {
    const char* path = "./test_graphics_dat_block_index.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    unsigned short indices[2] = {0xFFFF, 0xFFFF};
    unsigned char loadedGraphic[32] = {0};
    unsigned char blockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    struct NativeBitmapBlock_Compat* blockStorage = (struct NativeBitmapBlock_Compat*)blockBytes;
    struct NativeBitmapBlock_Compat* blocks[2] = {0, 0};
    int i;

    gBuildCallCount = 0;
    gApplyCallCount = 0;
    gPrepareCallCount = 0;
    memset(&gPreparedSizeInfo, 0, sizeof(gPreparedSizeInfo));
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (F0489_MEMORY_GetOrLoadBitmapByBlockIndex_Compat(
            path,
            &runtimeState,
            &fileState,
            1,
            loadedGraphic,
            indices,
            blocks,
            2,
            blockStorage,
            &result) != blockStorage->bitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (indices[1] != 0 || blocks[0] != blockStorage) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gPrepareCallCount != 1 || gPreparedBitmapIndex != 1 || gPreparedSizeInfo.Width != 9 || gPreparedSizeInfo.Height != 10) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gBuildCallCount != 1 || gApplyCallCount != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 5; i++) {
        if (loadedGraphic[i] != (unsigned char)(0x20 + i) || blockStorage->bitmap[i] != loadedGraphic[i]) {
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_block_index_returns_existing_bitmap_on_hit()) {
        fprintf(stderr, "test_block_index_returns_existing_bitmap_on_hit failed\n");
        return 1;
    }
    if (!test_block_index_prepares_new_block_and_loads_on_miss()) {
        fprintf(stderr, "test_block_index_prepares_new_block_and_loads_on_miss failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
