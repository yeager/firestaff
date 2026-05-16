#include <stdio.h>
#include <stdint.h>

#include "memory_cache_compact_used_pc34_compat.h"

static int test_compact_native_block_updates_block_offset_and_first_used(void) {
    unsigned short nativeIndices[4] = {0xFFFF, 2, 0xFFFF, 0xFFFF};
    unsigned short derivedIndices[2] = {0xFFFF, 0xFFFF};
    unsigned long blockOffsets[4] = {10, 20, 100, 40};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheCompactUsedResult_Compat result = {0};
    usageState.firstUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)100;
    return MEMORY_CACHE_CompactUsedBlock_Compat(1, 0, 100, 60, nativeIndices, derivedIndices, blockOffsets, &usageState, 4, 2, &result) == 1 &&
           result.blockIndex == 2 && result.moved == 1 && blockOffsets[2] == 60 &&
           (uintptr_t)usageState.firstUsedBlock == 60;
}

static int test_compact_derived_block_updates_last_and_first_referenced(void) {
    unsigned short nativeIndices[2] = {0xFFFF, 0xFFFF};
    unsigned short derivedIndices[4] = {0xFFFF, 0, 0xFFFF, 0xFFFF};
    unsigned long blockOffsets[2] = {200, 300};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheCompactUsedResult_Compat result = {0};
    usageState.lastUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)200;
    usageState.firstReferencedUsedBlock = (struct NativeBitmapBlock_Compat*)(uintptr_t)200;
    return MEMORY_CACHE_CompactUsedBlock_Compat(1, 1, 200, 120, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 4, &result) == 1 &&
           result.blockIndex == 0 && result.moved == 1 && blockOffsets[0] == 120 &&
           (uintptr_t)usageState.lastUsedBlock == 120 && (uintptr_t)usageState.firstReferencedUsedBlock == 120;
}

static int test_compact_no_move_keeps_offsets_and_reports_success(void) {
    unsigned short nativeIndices[2] = {0, 0xFFFF};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {88};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheCompactUsedResult_Compat result = {0};
    return MEMORY_CACHE_CompactUsedBlock_Compat(0, 0, 88, 88, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &result) == 1 &&
           result.moved == 0 && blockOffsets[0] == 88;
}

static int test_compact_fails_when_bitmap_index_has_no_registered_block(void) {
    unsigned short nativeIndices[2] = {0xFFFF, 0xFFFF};
    unsigned short derivedIndices[1] = {0xFFFF};
    unsigned long blockOffsets[1] = {0};
    struct MemoryCacheUsageState_Compat usageState = {0};
    struct MemoryCacheCompactUsedResult_Compat result = {0};
    return MEMORY_CACHE_CompactUsedBlock_Compat(1, 0, 50, 20, nativeIndices, derivedIndices, blockOffsets, &usageState, 2, 1, &result) == 0;
}

int main(void) {
    if (!test_compact_native_block_updates_block_offset_and_first_used()) {
        fprintf(stderr, "test_compact_native_block_updates_block_offset_and_first_used failed\n");
        return 1;
    }
    if (!test_compact_derived_block_updates_last_and_first_referenced()) {
        fprintf(stderr, "test_compact_derived_block_updates_last_and_first_referenced failed\n");
        return 1;
    }
    if (!test_compact_no_move_keeps_offsets_and_reports_success()) {
        fprintf(stderr, "test_compact_no_move_keeps_offsets_and_reports_success failed\n");
        return 1;
    }
    if (!test_compact_fails_when_bitmap_index_has_no_registered_block()) {
        fprintf(stderr, "test_compact_fails_when_bitmap_index_has_no_registered_block failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
