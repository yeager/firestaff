#include "redmcsb_f0480_cache_release_block_pc34_compat.h"

static bool f0480_span_end(size_t offset, size_t size, size_t *end) {
    if (size > SIZE_MAX - offset) {
        return false;
    }
    *end = offset + size;
    return true;
}

static bool f0480_spans_are_valid(
    const ReDMCSBF0480CacheReleaseStatePc34Compat *state) {
    size_t index;
    size_t previous_end = 0;

    if (state->free_span_count > state->free_span_capacity ||
        (state->free_span_count != 0 && state->free_spans == NULL)) {
        return false;
    }
    for (index = 0; index < state->free_span_count; ++index) {
        size_t end;
        if (state->free_spans[index].size == 0 ||
            !f0480_span_end(state->free_spans[index].offset,
                            state->free_spans[index].size, &end) ||
            end > state->cache_size ||
            (index != 0 && state->free_spans[index].offset <= previous_end)) {
            return false;
        }
        previous_end = end;
    }
    return true;
}

static bool f0480_used_link_is_valid(
    const ReDMCSBF0480CacheReleaseStatePc34Compat *state,
    uint16_t block_index) {
    const ReDMCSBF0480CacheBlockPc34Compat *block = &state->blocks[block_index];

    if (block->previous_used_index == REDMCSB_F0480_CACHE_BLOCK_NONE) {
        if (state->first_used_index != block_index) {
            return false;
        }
    } else if (block->previous_used_index >= state->block_count ||
               state->blocks[block->previous_used_index].next_used_index != block_index) {
        return false;
    }
    if (block->next_used_index == REDMCSB_F0480_CACHE_BLOCK_NONE) {
        return state->last_used_index == block_index;
    }
    return block->next_used_index < state->block_count &&
           state->blocks[block->next_used_index].previous_used_index == block_index;
}

bool F0480_CACHE_ReleaseBlock_PC34(
    uint16_t bitmap_index,
    ReDMCSBF0480CacheReleaseStatePc34Compat *state) {
    uint16_t *lookup_table;
    size_t lookup_count;
    size_t lookup_index;
    uint16_t block_index;
    ReDMCSBF0480CacheBlockPc34Compat *block;
    size_t block_end;
    size_t insert_at;
    bool joins_previous;
    bool joins_next;

    if (state == NULL || state->blocks == NULL || state->block_count == 0 ||
        !f0480_spans_are_valid(state)) {
        return false;
    }
    if ((bitmap_index & REDMCSB_F0480_CACHE_DERIVED_BITMAP_MASK) != 0) {
        lookup_table = state->derived_block_indices;
        lookup_count = state->derived_block_index_count;
        lookup_index = bitmap_index & ~REDMCSB_F0480_CACHE_DERIVED_BITMAP_MASK;
    } else {
        lookup_table = state->native_block_indices;
        lookup_count = state->native_block_index_count;
        lookup_index = bitmap_index;
    }
    if (lookup_table == NULL || lookup_index >= lookup_count) {
        return false;
    }
    block_index = lookup_table[lookup_index];
    if (block_index == REDMCSB_F0480_CACHE_BLOCK_NONE || block_index >= state->block_count) {
        return false;
    }
    block = &state->blocks[block_index];
    if (!block->allocated || block->bitmap_index != bitmap_index || block->size == 0 ||
        !f0480_span_end(block->offset, block->size, &block_end) ||
        block_end > state->cache_size || !f0480_used_link_is_valid(state, block_index)) {
        return false;
    }
    for (insert_at = 0; insert_at < state->free_span_count; ++insert_at) {
        if (state->free_spans[insert_at].offset > block->offset) {
            break;
        }
    }
    joins_previous = insert_at != 0 &&
        state->free_spans[insert_at - 1].offset + state->free_spans[insert_at - 1].size == block->offset;
    joins_next = insert_at != state->free_span_count &&
        block_end == state->free_spans[insert_at].offset;
    if (!joins_previous && !joins_next && state->free_span_count == state->free_span_capacity) {
        return false;
    }

    if (block->previous_used_index == REDMCSB_F0480_CACHE_BLOCK_NONE) {
        state->first_used_index = block->next_used_index;
    } else {
        state->blocks[block->previous_used_index].next_used_index = block->next_used_index;
    }
    if (block->next_used_index == REDMCSB_F0480_CACHE_BLOCK_NONE) {
        state->last_used_index = block->previous_used_index;
    } else {
        state->blocks[block->next_used_index].previous_used_index = block->previous_used_index;
    }
    lookup_table[lookup_index] = REDMCSB_F0480_CACHE_BLOCK_NONE;
    block->allocated = false;
    block->previous_used_index = REDMCSB_F0480_CACHE_BLOCK_NONE;
    block->next_used_index = REDMCSB_F0480_CACHE_BLOCK_NONE;

    if (joins_previous) {
        state->free_spans[insert_at - 1].size += block->size;
        if (joins_next) {
            state->free_spans[insert_at - 1].size += state->free_spans[insert_at].size;
            for (; insert_at + 1 < state->free_span_count; ++insert_at) {
                state->free_spans[insert_at] = state->free_spans[insert_at + 1];
            }
            --state->free_span_count;
        }
    } else if (joins_next) {
        state->free_spans[insert_at].offset = block->offset;
        state->free_spans[insert_at].size += block->size;
    } else {
        size_t index;
        for (index = state->free_span_count; index > insert_at; --index) {
            state->free_spans[index] = state->free_spans[index - 1];
        }
        state->free_spans[insert_at].offset = block->offset;
        state->free_spans[insert_at].size = block->size;
        ++state->free_span_count;
    }
    return true;
}
