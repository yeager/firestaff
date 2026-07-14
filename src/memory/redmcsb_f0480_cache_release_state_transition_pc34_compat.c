#include "redmcsb_f0480_cache_release_state_transition_pc34_compat.h"

static bool f0480_release_block_end(
    const ReDMCSBF0480CacheReleaseBlockPc34Compat *block,
    size_t *end) {
    if (block->byte_count > SIZE_MAX - block->address) {
        return false;
    }
    *end = block->address + block->byte_count;
    return true;
}

static void f0480_release_remove_unused(
    ReDMCSBF0480CacheReleaseBlockPc34Compat *block,
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat *state) {
    if (block->previous_unused_block == NULL) {
        state->first_unused_block = block->next_unused_block;
    } else {
        block->previous_unused_block->next_unused_block = block->next_unused_block;
    }
    if (block->next_unused_block != NULL) {
        block->next_unused_block->previous_unused_block = block->previous_unused_block;
    }
    block->previous_unused_block = NULL;
    block->next_unused_block = NULL;
}

static void f0480_release_add_unused_by_size(
    ReDMCSBF0480CacheReleaseBlockPc34Compat *block,
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat *state) {
    ReDMCSBF0480CacheReleaseBlockPc34Compat *current = state->first_unused_block;

    block->previous_unused_block = NULL;
    block->next_unused_block = NULL;
    if (current == NULL || block->byte_count >= current->byte_count) {
        block->next_unused_block = current;
        if (current != NULL) {
            current->previous_unused_block = block;
        }
        state->first_unused_block = block;
        return;
    }
    while (current->next_unused_block != NULL &&
           block->byte_count < current->next_unused_block->byte_count) {
        current = current->next_unused_block;
    }
    block->previous_unused_block = current;
    block->next_unused_block = current->next_unused_block;
    if (block->next_unused_block != NULL) {
        block->next_unused_block->previous_unused_block = block;
    }
    current->next_unused_block = block;
}

bool F0480_CACHE_ReleaseBlock_StateTransition_PC34(
    uint16_t bitmap_index,
    ReDMCSBF0480CacheReleaseStateTransitionPc34Compat *state) {
    ReDMCSBF0480CacheReleaseBlockPc34Compat **blocks;
    size_t block_count;
    size_t lookup_index;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *block;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *current;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *previous_unused = NULL;
    ReDMCSBF0480CacheReleaseBlockPc34Compat *next_unused = NULL;
    size_t block_end;

    if (state == NULL) {
        return false;
    }
    if ((bitmap_index & REDMCSB_F0480_RELEASE_BITMAP_DERIVED_MASK) != 0) {
        blocks = state->derived_blocks;
        block_count = state->derived_block_count;
        lookup_index = bitmap_index & ~REDMCSB_F0480_RELEASE_BITMAP_DERIVED_MASK;
    } else {
        blocks = state->native_blocks;
        block_count = state->native_block_count;
        lookup_index = bitmap_index;
    }
    if (blocks == NULL || lookup_index >= block_count || blocks[lookup_index] == NULL) {
        return false;
    }
    block = blocks[lookup_index];
    if (block->bitmap_index != bitmap_index || block->byte_count == 0 ||
        !f0480_release_block_end(block, &block_end) || block_end > state->cache_top ||
        block->byte_count > SIZE_MAX - state->available_byte_count) {
        return false;
    }

    for (current = state->first_unused_block; current != NULL;
         current = current->next_unused_block) {
        size_t current_end;
        if (!f0480_release_block_end(current, &current_end)) {
            return false;
        }
        if (current_end == block->address) {
            previous_unused = current;
        }
        if (block_end == current->address) {
            next_unused = current;
        }
    }

    if (block->previous_used_block == NULL) {
        state->first_used_block = block->next_used_block;
    } else {
        block->previous_used_block->next_used_block = block->next_used_block;
    }
    if (block->next_used_block == NULL) {
        state->last_used_block = block->previous_used_block;
    } else {
        block->next_used_block->previous_used_block = block->previous_used_block;
    }
    if (state->first_referenced_used_block == block) {
        state->first_referenced_used_block = block->next_used_block;
    }
    blocks[lookup_index] = NULL;
    state->available_byte_count += block->byte_count;
    block->bitmap_index = REDMCSB_F0480_RELEASE_BITMAP_NONE;
    block->previous_used_block = NULL;
    block->next_used_block = NULL;

    if (previous_unused != NULL) {
        f0480_release_remove_unused(previous_unused, state);
        previous_unused->byte_count += block->byte_count;
        block = previous_unused;
    }
    if (next_unused != NULL) {
        f0480_release_remove_unused(next_unused, state);
        block->byte_count += next_unused->byte_count;
    }
    if (block->byte_count <= state->cache_top &&
        block->address == state->cache_top - block->byte_count) {
        state->cache_top = block->address;
    } else {
        f0480_release_add_unused_by_size(block, state);
    }
    return true;
}
