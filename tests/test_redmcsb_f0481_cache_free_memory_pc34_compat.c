#include <stdio.h>

#include "redmcsb_f0481_cache_free_memory_pc34_compat.h"

typedef struct {
    const uint16_t *bitmap_indices;
    const size_t *released_byte_counts;
    size_t count;
    size_t next_index;
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat *state;
} ReleaseScript;

static bool release_first_used(uint16_t bitmap_index, void *context) {
    ReleaseScript *script = context;

    if (script->next_index >= script->count ||
        bitmap_index != script->bitmap_indices[script->next_index]) {
        return false;
    }
    script->state->available_cache_memory_byte_count +=
        script->released_byte_counts[script->next_index++];
    script->state->first_used_bitmap_index =
        script->next_index == script->count
            ? REDMCSB_F0481_CACHE_BITMAP_NONE
            : script->bitmap_indices[script->next_index];
    return true;
}

static int test_releases_first_used_bitmap_once(void) {
    const uint16_t bitmap_indices[] = { 7, 11 };
    const size_t released_byte_counts[] = { 8, 12 };
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat state = { 4, 32, 7, release_first_used, NULL };
    ReleaseScript script = { bitmap_indices, released_byte_counts, 2, 0, &state };

    state.release_context = &script;
    return F0481_CACHE_FreeMemory_PC34(12, &state) && script.next_index == 1 &&
        state.available_cache_memory_byte_count == 12 && state.first_used_bitmap_index == 11;
}

static int test_releases_first_used_until_request_is_met(void) {
    const uint16_t bitmap_indices[] = { 3, 5, 9 };
    const size_t released_byte_counts[] = { 4, 6, 10 };
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat state = { 2, 32, 3, release_first_used, NULL };
    ReleaseScript script = { bitmap_indices, released_byte_counts, 3, 0, &state };

    state.release_context = &script;
    return F0481_CACHE_FreeMemory_PC34(12, &state) && script.next_index == 2 &&
        state.available_cache_memory_byte_count == 12 && state.first_used_bitmap_index == 9;
}

static int test_empty_used_list_fails_without_release(void) {
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat state = {
        4, 32, REDMCSB_F0481_CACHE_BITMAP_NONE, release_first_used, NULL
    };

    return !F0481_CACHE_FreeMemory_PC34(8, &state) &&
        state.available_cache_memory_byte_count == 4;
}

static bool release_without_reclaiming(uint16_t bitmap_index, void *context) {
    (void)bitmap_index;
    (void)context;
    return true;
}

static int test_non_reclaiming_release_fails(void) {
    ReDMCSBF0481CacheFreeMemoryStatePc34Compat state = {
        4, 32, 7, release_without_reclaiming, NULL
    };

    return !F0481_CACHE_FreeMemory_PC34(8, &state) &&
        state.available_cache_memory_byte_count == 4 && state.first_used_bitmap_index == 7;
}

int main(void) {
    if (!test_releases_first_used_bitmap_once() ||
        !test_releases_first_used_until_request_is_met() ||
        !test_empty_used_list_fails_without_release() ||
        !test_non_reclaiming_release_fails()) {
        fprintf(stderr, "F0481 cache free-memory adapter test failed\n");
        return 1;
    }
    puts("ok");
    return 0;
}
