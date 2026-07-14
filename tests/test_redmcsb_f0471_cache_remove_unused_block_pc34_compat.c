#include <stdio.h>

#include "redmcsb_f0471_cache_remove_unused_block_pc34_compat.h"

static int test_removes_middle_block_and_preserves_head_anchor(void) {
    ReDMCSBF0471CacheUnusedBlockPc34Compat first = { NULL, NULL };
    ReDMCSBF0471CacheUnusedBlockPc34Compat middle = { &first, NULL };
    ReDMCSBF0471CacheUnusedBlockPc34Compat last = { &middle, NULL };
    ReDMCSBF0471CacheUnusedBlockPc34Compat *first_unused_block = &first;

    first.next_unused_block = &middle;
    middle.next_unused_block = &last;
    F0471_CACHE_RemoveUnusedBlock_PC34(&middle, &first_unused_block);

    return first_unused_block == &first && first.next_unused_block == &last &&
           last.previous_unused_block == &first &&
           middle.previous_unused_block == NULL && middle.next_unused_block == NULL;
}

static int test_removes_head_and_reanchors_successor(void) {
    ReDMCSBF0471CacheUnusedBlockPc34Compat first = { NULL, NULL };
    ReDMCSBF0471CacheUnusedBlockPc34Compat next = { &first, NULL };
    ReDMCSBF0471CacheUnusedBlockPc34Compat *first_unused_block = &first;

    first.next_unused_block = &next;
    F0471_CACHE_RemoveUnusedBlock_PC34(&first, &first_unused_block);

    return first_unused_block == &next && next.previous_unused_block == NULL &&
           first.previous_unused_block == NULL && first.next_unused_block == NULL;
}

static int test_removes_only_block_and_clears_head_anchor(void) {
    ReDMCSBF0471CacheUnusedBlockPc34Compat only = { NULL, NULL };
    ReDMCSBF0471CacheUnusedBlockPc34Compat *first_unused_block = &only;

    F0471_CACHE_RemoveUnusedBlock_PC34(&only, &first_unused_block);

    return first_unused_block == NULL && only.previous_unused_block == NULL &&
           only.next_unused_block == NULL;
}

int main(void) {
    if (!test_removes_middle_block_and_preserves_head_anchor() ||
        !test_removes_head_and_reanchors_successor() ||
        !test_removes_only_block_and_clears_head_anchor()) {
        fprintf(stderr, "F0471 cache remove-unused-block test failed\n");
        return 1;
    }
    puts("ok");
    return 0;
}
