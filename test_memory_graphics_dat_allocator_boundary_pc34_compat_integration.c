#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_allocator_boundary_pc34_compat.h"

#define TEST_BLOCK_STORAGE_SIZE (sizeof(struct NativeBitmapBlock_Compat) + 32)

static int gBuildCallCount;
static int gApplyCallCount;
static int gPrepareCallCount;

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

static int test_allocator_boundary_allows_hit_without_free_slot(void) {
    const char* path = "./test_graphics_dat_allocator_boundary.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    struct MemoryCacheUsageState_Compat usageState = {0};
    unsigned short indices[2] = {0xFFFF, 0};
    unsigned char loadedGraphic[32] = {0};
    unsigned char existingBlockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    unsigned char spareBlockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    struct NativeBitmapBlock_Compat* existingBlock = (struct NativeBitmapBlock_Compat*)existingBlockBytes;
    struct NativeBitmapBlock_Compat* spareBlock = (struct NativeBitmapBlock_Compat*)spareBlockBytes;
    struct NativeBitmapBlock_Compat* blocks[1] = {existingBlock};

    existingBlock->bitmap[0] = 0x7E;
    usageState.firstUsedBlock = existingBlock;
    usageState.lastUsedBlock = existingBlock;
    gBuildCallCount = 0;
    gApplyCallCount = 0;
    gPrepareCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (F0489_MEMORY_GetOrLoadBitmapWithAllocatorBoundary_Compat(
            path,
            &runtimeState,
            &fileState,
            1,
            loadedGraphic,
            indices,
            blocks,
            1,
            spareBlock,
            &usageState,
            &result) != existingBlock->bitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (existingBlock->usageCount != 1 || gBuildCallCount != 0 || gApplyCallCount != 0 || gPrepareCallCount != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

static int test_allocator_boundary_blocks_miss_without_free_slot(void) {
    const char* path = "./test_graphics_dat_allocator_boundary.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatBitmapPathResult_Compat result;
    struct MemoryCacheUsageState_Compat usageState = {0};
    unsigned short indices[2] = {0xFFFF, 0xFFFF};
    unsigned char loadedGraphic[32] = {0};
    unsigned char existingBlockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    unsigned char spareBlockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    struct NativeBitmapBlock_Compat* existingBlock = (struct NativeBitmapBlock_Compat*)existingBlockBytes;
    struct NativeBitmapBlock_Compat* spareBlock = (struct NativeBitmapBlock_Compat*)spareBlockBytes;
    struct NativeBitmapBlock_Compat* blocks[1] = {existingBlock};

    gBuildCallCount = 0;
    gApplyCallCount = 0;
    gPrepareCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (F0489_MEMORY_GetOrLoadBitmapWithAllocatorBoundary_Compat(
            path,
            &runtimeState,
            &fileState,
            1,
            loadedGraphic,
            indices,
            blocks,
            1,
            spareBlock,
            &usageState,
            &result) != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gBuildCallCount != 0 || gApplyCallCount != 0 || gPrepareCallCount != 0 || indices[1] != 0xFFFF) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_allocator_boundary_allows_hit_without_free_slot()) {
        fprintf(stderr, "test_allocator_boundary_allows_hit_without_free_slot failed\n");
        return 1;
    }
    if (!test_allocator_boundary_blocks_miss_without_free_slot()) {
        fprintf(stderr, "test_allocator_boundary_blocks_miss_without_free_slot failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
