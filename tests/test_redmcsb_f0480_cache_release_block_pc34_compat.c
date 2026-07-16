#include <stdio.h>

#include "redmcsb_f0480_cache_release_block_pc34_compat.h"

static ReDMCSBF0480CacheReleaseStatePc34Compat make_state(
    ReDMCSBF0480CacheBlockPc34Compat *blocks,
    uint16_t *native_indices,
    uint16_t *derived_indices,
    ReDMCSBF0480CacheFreeSpanPc34Compat *free_spans,
    size_t free_span_count) {
    ReDMCSBF0480CacheReleaseStatePc34Compat state = {
        blocks, 3, native_indices, 2, derived_indices, 1,
        free_spans, free_span_count, 3, 64,
        0, 2
    };
    return state;
}

static int test_native_release_unlinks_and_coalesces_both_sides(void) {
    ReDMCSBF0480CacheBlockPc34Compat blocks[3] = {
        { 8, 8, 0, REDMCSB_F0480_CACHE_BLOCK_NONE, 1, true },
        { 24, 8, 0, 0, 2, true },
        { 40, 8, 2, 1, REDMCSB_F0480_CACHE_BLOCK_NONE, true }
    };
    uint16_t native_indices[2] = { 1, REDMCSB_F0480_CACHE_BLOCK_NONE };
    uint16_t derived_indices[1] = { 2 };
    ReDMCSBF0480CacheFreeSpanPc34Compat free_spans[3] = { { 16, 8 }, { 32, 8 } };
    ReDMCSBF0480CacheReleaseStatePc34Compat state = make_state(
        blocks, native_indices, derived_indices, free_spans, 2);

    return F0480_CACHE_ReleaseBlock_PC34(0, &state) &&
        native_indices[0] == REDMCSB_F0480_CACHE_BLOCK_NONE && !blocks[1].allocated &&
        blocks[0].next_used_index == 2 && blocks[2].previous_used_index == 0 &&
        state.first_used_index == 0 && state.last_used_index == 2 &&
        state.free_span_count == 1 && free_spans[0].offset == 16 && free_spans[0].size == 24;
}

static int test_derived_release_uses_high_bit_lookup(void) {
    ReDMCSBF0480CacheBlockPc34Compat blocks[3] = {
        { 8, 8, 0, REDMCSB_F0480_CACHE_BLOCK_NONE, 1, true },
        { 24, 8, 0, 0, 2, true },
        { 40, 8, UINT16_C(0x8000), 1, REDMCSB_F0480_CACHE_BLOCK_NONE, true }
    };
    uint16_t native_indices[2] = { 1, REDMCSB_F0480_CACHE_BLOCK_NONE };
    uint16_t derived_indices[1] = { 2 };
    ReDMCSBF0480CacheFreeSpanPc34Compat free_spans[3] = { { 48, 16 } };
    ReDMCSBF0480CacheReleaseStatePc34Compat state = make_state(
        blocks, native_indices, derived_indices, free_spans, 1);

    return F0480_CACHE_ReleaseBlock_PC34(UINT16_C(0x8000), &state) &&
        derived_indices[0] == REDMCSB_F0480_CACHE_BLOCK_NONE && !blocks[2].allocated &&
        blocks[1].next_used_index == REDMCSB_F0480_CACHE_BLOCK_NONE &&
        state.last_used_index == 1 && state.free_span_count == 1 &&
        free_spans[0].offset == 40 && free_spans[0].size == 24;
}

static int test_malformed_target_does_not_mutate_state(void) {
    ReDMCSBF0480CacheBlockPc34Compat blocks[3] = {
        { 8, 8, 0, REDMCSB_F0480_CACHE_BLOCK_NONE, 1, true },
        { 24, 8, 0, 0, 2, true },
        { 40, 8, 2, 1, REDMCSB_F0480_CACHE_BLOCK_NONE, true }
    };
    uint16_t native_indices[2] = { 1, REDMCSB_F0480_CACHE_BLOCK_NONE };
    uint16_t derived_indices[1] = { 2 };
    ReDMCSBF0480CacheFreeSpanPc34Compat free_spans[3] = { { 48, 16 } };
    ReDMCSBF0480CacheReleaseStatePc34Compat state = make_state(
        blocks, native_indices, derived_indices, free_spans, 1);

    return !F0480_CACHE_ReleaseBlock_PC34(1, &state) && native_indices[0] == 1 &&
        blocks[1].allocated && blocks[0].next_used_index == 1 &&
        state.free_span_count == 1 && free_spans[0].offset == 48 && free_spans[0].size == 16;
}

int main(void) {
    if (!test_native_release_unlinks_and_coalesces_both_sides() ||
        !test_derived_release_uses_high_bit_lookup() ||
        !test_malformed_target_does_not_mutate_state()) {
        fprintf(stderr, "F0480 cache release adapter test failed\n");
        return 1;
    }
    puts("ok");
    return 0;
}
