#include <stdio.h>

#include "redmcsb_f0480_cache_release_state_transition_pc34_compat.h"

static int test_native_release_coalesces_and_unlinks_all_used_anchors(void) {
    ReDMCSBF0480CacheReleaseBlockPc34Compat first = { 0, 8, 7, NULL, NULL, NULL, NULL };
    ReDMCSBF0480CacheReleaseBlockPc34Compat middle = { 16, 8, 1, &first, NULL, NULL, NULL };
    ReDMCSBF0480CacheReleaseBlockPc34Compat left_free = { 8, 8, 0, NULL, NULL, NULL, NULL };
    ReDMCSBF0480CacheReleaseBlockPc34Compat right_free = { 24, 8, 0, NULL, NULL, NULL, NULL };
    ReDMCSBF0480CacheReleaseBlockPc34Compat *native[2] = { NULL, &middle };
    ReDMCSBF0480CacheReleaseBlockPc34Compat *derived[1] = { NULL };
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat state = {
        native, 2, derived, 1, &first, &middle, &middle, &left_free,
        12, 64
    };

    left_free.next_unused_block = &right_free;
    right_free.previous_unused_block = &left_free;
    return F0480_CACHE_ReleaseBlock_StateTransition_PC34(1, &state) &&
        native[1] == NULL && middle.bitmap_index == REDMCSB_F0480_RELEASE_BITMAP_NONE &&
        state.first_used_block == &first && state.last_used_block == &first &&
        state.first_referenced_used_block == NULL && first.next_used_block == NULL &&
        state.first_unused_block == &left_free && left_free.byte_count == 24 &&
        left_free.next_unused_block == NULL && state.available_byte_count == 20;
}

static int test_derived_release_contracts_cache_top(void) {
    ReDMCSBF0480CacheReleaseBlockPc34Compat block = {
        40, 24, UINT16_C(0x8000), NULL, NULL, NULL, NULL
    };
    ReDMCSBF0480CacheReleaseBlockPc34Compat *native[1] = { NULL };
    ReDMCSBF0480CacheReleaseBlockPc34Compat *derived[1] = { &block };
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat state = {
        native, 1, derived, 1, &block, &block, &block, NULL, 8, 64
    };

    return F0480_CACHE_ReleaseBlock_StateTransition_PC34(UINT16_C(0x8000), &state) &&
        derived[0] == NULL && state.first_used_block == NULL &&
        state.last_used_block == NULL && state.first_referenced_used_block == NULL &&
        state.first_unused_block == NULL && state.cache_top == 40 &&
        state.available_byte_count == 32;
}

static int test_invalid_lookup_preserves_state(void) {
    ReDMCSBF0480CacheReleaseBlockPc34Compat block = {
        8, 8, 0, NULL, NULL, NULL, NULL
    };
    ReDMCSBF0480CacheReleaseBlockPc34Compat *native[1] = { &block };
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat state = {
        native, 1, NULL, 0, &block, &block, &block, NULL, 3, 32
    };

    return !F0480_CACHE_ReleaseBlock_StateTransition_PC34(1, &state) &&
        native[0] == &block && state.first_used_block == &block &&
        state.available_byte_count == 3 && state.cache_top == 32;
}

int main(void) {
    if (!test_native_release_coalesces_and_unlinks_all_used_anchors() ||
        !test_derived_release_contracts_cache_top() ||
        !test_invalid_lookup_preserves_state()) {
        fprintf(stderr, "F0480 release state transition test failed\n");
        return 1;
    }
    puts("ok");
    return 0;
}
