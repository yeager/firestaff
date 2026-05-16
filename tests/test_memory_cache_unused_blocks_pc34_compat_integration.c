#include <stdio.h>

#include "memory_cache_unused_blocks_pc34_compat.h"

static int test_add_unused_block_orders_by_size_descending(void) {
    struct MemoryCacheUnusedBlock_Compat a = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {24, 0, 0};
    struct MemoryCacheUnusedBlock_Compat c = {32, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = 0;
    F0472_CACHE_AddUnusedBlock_Compat(&a, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&b, &first);
    F0472_CACHE_AddUnusedBlock_Compat(&c, &first);
    return first == &a && a.nextUnusedBlock == &c && c.previousUnusedBlock == &a && c.nextUnusedBlock == &b && b.previousUnusedBlock == &c;
}

static int test_remove_unused_block_unlinks_middle(void) {
    struct MemoryCacheUnusedBlock_Compat a = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {32, 0, 0};
    struct MemoryCacheUnusedBlock_Compat c = {24, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &a;
    a.nextUnusedBlock = &b;
    b.previousUnusedBlock = &a;
    b.nextUnusedBlock = &c;
    c.previousUnusedBlock = &b;
    F0471_CACHE_RemoveUnusedBlock_Compat(&b, &first);
    return first == &a && a.nextUnusedBlock == &c && c.previousUnusedBlock == &a && b.previousUnusedBlock == 0 && b.nextUnusedBlock == 0;
}

static int test_remove_unused_block_updates_head(void) {
    struct MemoryCacheUnusedBlock_Compat a = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat b = {32, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &a;
    a.nextUnusedBlock = &b;
    b.previousUnusedBlock = &a;
    F0471_CACHE_RemoveUnusedBlock_Compat(&a, &first);
    return first == &b && b.previousUnusedBlock == 0;
}

static int test_register_split_remainder_honors_minimum_threshold(void) {
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheUnusedBlock_Compat head = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &head;
    return MEMORY_CACHE_RegisterSplitUnusedRemainder_Compat(&remainder, 12, 24, &first) == 0 &&
           first == &head && head.nextUnusedBlock == 0;
}

static int test_register_split_remainder_adds_sorted_block(void) {
    struct MemoryCacheUnusedBlock_Compat remainder = {0, 0, 0};
    struct MemoryCacheUnusedBlock_Compat head = {40, 0, 0};
    struct MemoryCacheUnusedBlock_Compat tail = {16, 0, 0};
    struct MemoryCacheUnusedBlock_Compat* first = &head;
    head.nextUnusedBlock = &tail;
    tail.previousUnusedBlock = &head;
    return MEMORY_CACHE_RegisterSplitUnusedRemainder_Compat(&remainder, 24, 24, &first) == 1 &&
           first == &head && head.nextUnusedBlock == &remainder && remainder.previousUnusedBlock == &head && remainder.nextUnusedBlock == &tail && tail.previousUnusedBlock == &remainder;
}

int main(void) {
    if (!test_add_unused_block_orders_by_size_descending()) {
        fprintf(stderr, "test_add_unused_block_orders_by_size_descending failed\n");
        return 1;
    }
    if (!test_remove_unused_block_unlinks_middle()) {
        fprintf(stderr, "test_remove_unused_block_unlinks_middle failed\n");
        return 1;
    }
    if (!test_remove_unused_block_updates_head()) {
        fprintf(stderr, "test_remove_unused_block_updates_head failed\n");
        return 1;
    }
    if (!test_register_split_remainder_honors_minimum_threshold()) {
        fprintf(stderr, "test_register_split_remainder_honors_minimum_threshold failed\n");
        return 1;
    }
    if (!test_register_split_remainder_adds_sorted_block()) {
        fprintf(stderr, "test_register_split_remainder_adds_sorted_block failed\n");
        return 1;
    }
    printf("ok\n");
    return 0;
}
