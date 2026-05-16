#include <stdio.h>

#include "memory_cache_allocation_boundary_pc34_compat.h"

struct NativeBitmapBlockStorage_Compat {
    unsigned long align;
    unsigned char bytes[128];
};

static struct NativeBitmapBlock_Compat* as_block(struct NativeBitmapBlockStorage_Compat* storage) {
    return (struct NativeBitmapBlock_Compat*)storage->bytes;
}

static int test_normalize_block_size_rounds_up_odd_sizes(void) {
    return MEMORY_CACHE_NormalizeBlockSize_Compat(20) == 20 &&
           MEMORY_CACHE_NormalizeBlockSize_Compat(21) == 22;
}

static int test_find_first_free_block_slot_finds_gap(void) {
    struct NativeBitmapBlockStorage_Compat s0 = {0};
    struct NativeBitmapBlockStorage_Compat s2 = {0};
    struct NativeBitmapBlock_Compat* blocks[4] = {as_block(&s0), 0, as_block(&s2), 0};
    return MEMORY_CACHE_FindFirstFreeBlockSlot_Compat(blocks, 4) == 1;
}

static int test_register_native_block_sets_index_and_slot(void) {
    unsigned short indices[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    unsigned short outIndex = 0xFFFF;
    return MEMORY_CACHE_RegisterNativeBlock_Compat(2, block, indices, blocks, 4, &outIndex) == 1 &&
           outIndex == 0 && indices[2] == 0 && blocks[0] == block && block->bitmapIndex == 2;
}

static int test_register_derived_block_sets_flagged_index_and_slot(void) {
    unsigned short indices[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
    struct NativeBitmapBlock_Compat* blocks[4] = {0};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    struct NativeBitmapBlock_Compat* block = as_block(&storage);
    unsigned short outIndex = 0xFFFF;
    return MEMORY_CACHE_RegisterDerivedBlock_Compat(3, block, indices, blocks, 4, &outIndex) == 1 &&
           outIndex == 0 && indices[3] == 0 && blocks[0] == block && block->bitmapIndex == (unsigned short)(3 | 0x8000);
}

static int test_register_fails_when_no_slot_exists(void) {
    struct NativeBitmapBlockStorage_Compat s0 = {0};
    struct NativeBitmapBlockStorage_Compat s1 = {0};
    struct NativeBitmapBlockStorage_Compat s2 = {0};
    struct NativeBitmapBlock_Compat* blocks[3] = {as_block(&s0), as_block(&s1), as_block(&s2)};
    unsigned short indices[4] = {0xFFFF,0xFFFF,0xFFFF,0xFFFF};
    struct NativeBitmapBlockStorage_Compat storage = {0};
    unsigned short outIndex = 0xFFFF;
    return MEMORY_CACHE_RegisterNativeBlock_Compat(1, as_block(&storage), indices, blocks, 3, &outIndex) == 0 &&
           outIndex == 0xFFFF && indices[1] == 0xFFFF;
}

int main(void) {
    if (!test_normalize_block_size_rounds_up_odd_sizes()) {
        fprintf(stderr, "test_normalize_block_size_rounds_up_odd_sizes failed\n");
        return 1;
    }
    if (!test_find_first_free_block_slot_finds_gap()) {
        fprintf(stderr, "test_find_first_free_block_slot_finds_gap failed\n");
        return 1;
    }
    if (!test_register_native_block_sets_index_and_slot()) {
        fprintf(stderr, "test_register_native_block_sets_index_and_slot failed\n");
        return 1;
    }
    if (!test_register_derived_block_sets_flagged_index_and_slot()) {
        fprintf(stderr, "test_register_derived_block_sets_flagged_index_and_slot failed\n");
        return 1;
    }
    if (!test_register_fails_when_no_slot_exists()) {
        fprintf(stderr, "test_register_fails_when_no_slot_exists failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
