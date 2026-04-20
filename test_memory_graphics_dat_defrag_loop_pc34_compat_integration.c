#include <stdio.h>
#include <string.h>

#include "memory_graphics_dat_defrag_loop_pc34_compat.h"

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

static int test_defrag_loop_hit_bypasses_orchestrator(void) {
    const char* path = "./test_graphics_dat_defrag_loop.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatDefragLoopResult_Compat result;
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheAllocateOrReuseState_Compat allocatorState = {50, 50, 0};
    struct MemoryCacheDefragResultState_Compat defragState = {77, 0};
    struct MemoryCacheUnusedBlock_Compat* firstUnusedBlock = 0;
    struct MemoryCacheUnusedBlock_Compat splitRemainder = {0};
    struct MemoryCacheRawBlockHeader_Compat headers[1] = {{26, 1, 0}};
    struct MemoryCacheDefragSegment_Compat segments[2] = {{0}};
    unsigned short indices[2] = {0xFFFF, 0};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {0};
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
    if (F0489_MEMORY_GetOrLoadBitmapWithDefragLoop_Compat(
            path,
            &runtimeState,
            &fileState,
            1,
            loadedGraphic,
            indices,
            derivedIndices,
            blockOffsets,
            blocks,
            1,
            spareBlock,
            &usageState,
            headers,
            1,
            segments,
            2,
            123,
            &allocatorState,
            &defragState,
            &firstUnusedBlock,
            &splitRemainder,
            &result) != existingBlock->bitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (result.orchestrator.segmentCount != 0 || result.defragApplied != 0 || allocatorState.cacheBytesAvailable != 50) {
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

static int test_defrag_loop_miss_runs_orchestrator_then_loads(void) {
    const char* path = "./test_graphics_dat_defrag_loop.bin";
    struct MemoryGraphicsDatState_Compat fileState = {0};
    struct MemoryGraphicsDatRuntimeState_Compat runtimeState;
    struct MemoryGraphicsDatDefragLoopResult_Compat result;
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheAllocateOrReuseState_Compat allocatorState = {100, 0, 20};
    struct MemoryCacheUnusedBlock_Compat unusedBlock = {10, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* firstUnusedBlock = &unusedBlock;
    struct MemoryCacheUnusedBlock_Compat splitRemainder = {0};
    struct MemoryCacheDefragResultState_Compat defragState = {80, &unusedBlock};
    struct MemoryCacheRawBlockHeader_Compat headers[2] = {{10, 0, 0}, {-26, 1, 0}};
    struct MemoryCacheDefragSegment_Compat segments[3] = {{0}};
    unsigned short indices[2] = {0xFFFF, 0};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[2] = {10, 0};
    unsigned char loadedGraphic[32] = {0};
    unsigned char existingBlockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    unsigned char newBlockBytes[TEST_BLOCK_STORAGE_SIZE] = {0};
    struct NativeBitmapBlock_Compat* existingBlock = (struct NativeBitmapBlock_Compat*)existingBlockBytes;
    struct NativeBitmapBlock_Compat* newBlock = (struct NativeBitmapBlock_Compat*)newBlockBytes;
    struct NativeBitmapBlock_Compat* blocks[2] = {existingBlock, 0};
    int i;

    usageState.firstUsedBlock = (struct NativeBitmapBlock_Compat*)10;
    usageState.lastUsedBlock = (struct NativeBitmapBlock_Compat*)10;
    gBuildCallCount = 0;
    gApplyCallCount = 0;
    gPrepareCallCount = 0;
    if (!write_format1_file(path)) {
        return 0;
    }
    if (!F0479_MEMORY_InitializeGraphicsDatState_Compat(path, &fileState, &runtimeState)) {
        return 0;
    }
    if (F0489_MEMORY_GetOrLoadBitmapWithDefragLoop_Compat(
            path,
            &runtimeState,
            &fileState,
            0,
            loadedGraphic,
            indices,
            derivedIndices,
            blockOffsets,
            blocks,
            2,
            newBlock,
            &usageState,
            headers,
            2,
            segments,
            3,
            999,
            &allocatorState,
            &defragState,
            &firstUnusedBlock,
            &splitRemainder,
            &result) != newBlock->bitmap) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (result.orchestrator.segmentCount != 2 || result.orchestrator.movedBlockCount != 1 || result.orchestrator.compactedTopOffset != 26) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (result.normalizedRequestedSize != 14 || result.defragApplied != 1 || !result.allocator.defragmentRequested) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (allocatorState.cacheBytesAvailable != 86 || allocatorState.cacheMemoryTopOffset != 26 || firstUnusedBlock != 0 || blockOffsets[0] != 0) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    if (gBuildCallCount != 1 || gApplyCallCount != 0 || gPrepareCallCount != 1 || indices[0] != 1) {
        F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
        return 0;
    }
    for (i = 0; i < 3; i++) {
        if (loadedGraphic[i] != (unsigned char)(0x10 + i) || newBlock->bitmap[i] != loadedGraphic[i]) {
            F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
            return 0;
        }
    }
    F0479_MEMORY_FreeGraphicsDatState_Compat(&runtimeState);
    return 1;
}

int main(void) {
    if (!test_defrag_loop_hit_bypasses_orchestrator()) {
        fprintf(stderr, "test_defrag_loop_hit_bypasses_orchestrator failed\n");
        return 1;
    }
    if (!test_defrag_loop_miss_runs_orchestrator_then_loads()) {
        fprintf(stderr, "test_defrag_loop_miss_runs_orchestrator_then_loads failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
