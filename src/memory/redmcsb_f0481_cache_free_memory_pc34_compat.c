#include "redmcsb_f0481_cache_free_memory_pc34_compat.h"

bool F0481_CACHE_FreeMemory_PC34(
    size_t byte_count,
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat *state) {
    if (state == NULL || state->release_block == NULL ||
        state->available_cache_memory_byte_count > state->cache_memory_capacity ||
        byte_count > state->cache_memory_capacity) {
        return false;
    }

    while (state->available_cache_memory_byte_count < byte_count) {
        size_t available_before_release = state->available_cache_memory_byte_count;

        if (state->first_used_bitmap_index == REDMCSB_F0481_CACHE_BITMAP_NONE ||
            !state->release_block(state->first_used_bitmap_index,
                                  state->release_context) ||
            state->available_cache_memory_byte_count <= available_before_release ||
            state->available_cache_memory_byte_count > state->cache_memory_capacity) {
            return false;
        }
    }
    return true;
}
