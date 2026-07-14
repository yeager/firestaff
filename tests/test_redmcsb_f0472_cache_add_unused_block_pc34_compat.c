#include <stdio.h>

#include "redmcsb_f0472_cache_add_unused_block_pc34_compat.h"

static int test_adds_only_block_and_reanchors_list(void)
{
    ReDMCSBF0472CacheUnusedBlockPc34Compat block = { 24, &block, &block };
    ReDMCSBF0472CacheUnusedBlockPc34Compat *first_unused_block = NULL;

    F0472_CACHE_AddUnusedBlock_PC34(&block, &first_unused_block);

    return first_unused_block == &block &&
           block.previous_unused_block == NULL && block.next_unused_block == NULL;
}

static int test_inserts_equal_size_before_first_and_reanchors_list(void)
{
    ReDMCSBF0472CacheUnusedBlockPc34Compat first = { 64, NULL, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat block = { 64, NULL, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat *first_unused_block = &first;

    F0472_CACHE_AddUnusedBlock_PC34(&block, &first_unused_block);

    return first_unused_block == &block &&
           block.previous_unused_block == NULL && block.next_unused_block == &first &&
           first.previous_unused_block == &block && first.next_unused_block == NULL;
}

static int test_inserts_before_equal_middle_block_without_reanchoring(void)
{
    ReDMCSBF0472CacheUnusedBlockPc34Compat first = { 96, NULL, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat equal = { 48, &first, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat last = { 16, &equal, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat block = { 48, NULL, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat *first_unused_block = &first;

    first.next_unused_block = &equal;
    equal.next_unused_block = &last;
    F0472_CACHE_AddUnusedBlock_PC34(&block, &first_unused_block);

    return first_unused_block == &first && first.next_unused_block == &block &&
           block.previous_unused_block == &first && block.next_unused_block == &equal &&
           equal.previous_unused_block == &block && equal.next_unused_block == &last &&
           last.previous_unused_block == &equal && last.next_unused_block == NULL;
}

static int test_appends_smaller_block(void)
{
    ReDMCSBF0472CacheUnusedBlockPc34Compat first = { 96, NULL, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat last = { 32, &first, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat block = { 16, NULL, NULL };
    ReDMCSBF0472CacheUnusedBlockPc34Compat *first_unused_block = &first;

    first.next_unused_block = &last;
    F0472_CACHE_AddUnusedBlock_PC34(&block, &first_unused_block);

    return first_unused_block == &first && first.next_unused_block == &last &&
           last.previous_unused_block == &first && last.next_unused_block == &block &&
           block.previous_unused_block == &last && block.next_unused_block == NULL;
}

int main(void)
{
    if (!test_adds_only_block_and_reanchors_list() ||
        !test_inserts_equal_size_before_first_and_reanchors_list() ||
        !test_inserts_before_equal_middle_block_without_reanchoring() ||
        !test_appends_smaller_block()) {
        fprintf(stderr, "F0472 cache add-unused-block test failed\n");
        return 1;
    }
    puts("ok");
    return 0;
}
