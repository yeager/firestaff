#include "redmcsb_f0661_get_shrinked_bitmap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct bitmap {
    int16_t width;
    int16_t height;
    uint8_t pixels[32];
} bitmap;

typedef struct capture {
    bitmap native_bitmap;
    bitmap derived_bitmap;
    int cached;
    int cache_checks;
    int native_reads;
    int derived_reads;
    int blits;
    int cache_adds;
    int16_t native_index;
    int16_t derived_index;
    int16_t source_width;
    int16_t source_height;
    int16_t destination_width;
    int16_t destination_height;
    const uint8_t *palette;
} capture;

static int is_cached(void *context, int16_t index)
{
    capture *state = (capture *)context;

    state->cache_checks += 1;
    state->derived_index = index;
    return state->cached;
}

static const uint8_t *get_native(void *context, int16_t index)
{
    capture *state = (capture *)context;

    state->native_reads += 1;
    state->native_index = index;
    return state->native_bitmap.pixels;
}

static uint8_t *get_derived(void *context, int16_t index)
{
    capture *state = (capture *)context;

    state->derived_reads += 1;
    state->derived_index = index;
    return state->derived_bitmap.pixels;
}

static void blit(void *context, const uint8_t *source, uint8_t *destination,
                 int16_t source_width, int16_t source_height,
                 int16_t destination_width, int16_t destination_height,
                 const uint8_t *palette)
{
    capture *state = (capture *)context;

    (void)source;
    (void)destination;
    state->blits += 1;
    state->source_width = source_width;
    state->source_height = source_height;
    state->destination_width = destination_width;
    state->destination_height = destination_height;
    state->palette = palette;
}

static void add_cache(void *context, int16_t index)
{
    capture *state = (capture *)context;

    state->cache_adds += 1;
    state->derived_index = index;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    capture state;
    uint8_t palette[16] = {0};
    redmcsb_f0661_renderer_pc34_compat renderer;
    uint8_t *result;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    memset(&renderer, 0, sizeof(renderer));
    state.native_bitmap.width = 96;
    state.native_bitmap.height = 45;
    renderer.is_derived_bitmap_cached = is_cached;
    renderer.get_native_bitmap = get_native;
    renderer.get_derived_bitmap = get_derived;
    renderer.blit_shrink_palette = blit;
    renderer.add_derived_bitmap_to_cache = add_cache;
    renderer.context = &state;

    result = redmcsb_f0661_get_shrinked_bitmap_pc34_compat(
        &renderer, 14, 3, 42, 37, palette);
    ok &= expect(result == state.derived_bitmap.pixels,
                 "F0661 returns caller-owned derived bitmap");
    ok &= expect(state.cache_checks == 1 && state.native_reads == 1 &&
                     state.derived_reads == 1 && state.blits == 1 &&
                     state.cache_adds == 1 && state.native_index == 14 &&
                     state.derived_index == 3,
                 "F0661 cache-miss call ordering");
    ok &= expect(state.derived_bitmap.width == 42 && state.derived_bitmap.height == 37 &&
                     state.source_width == 96 && state.source_height == 45 &&
                     state.destination_width == 42 && state.destination_height == 37 &&
                     state.palette == palette,
                 "F0661 writes derived dimensions before F0129");

    state.cached = 1;
    state.native_reads = 0;
    state.derived_reads = 0;
    state.blits = 0;
    state.cache_adds = 0;
    result = redmcsb_f0661_get_shrinked_bitmap_pc34_compat(
        &renderer, 99, 3, 64, 32, NULL);
    ok &= expect(result == state.derived_bitmap.pixels && state.native_reads == 0 &&
                     state.derived_reads == 1 && state.blits == 0 && state.cache_adds == 0,
                 "F0661 cache hit does not fetch, resize, blit, or re-add");
    ok &= expect(redmcsb_f0661_get_shrinked_bitmap_pc34_compat(
                     NULL, 1, 2, 3, 4, NULL) == NULL,
                 "F0661 rejects missing renderer");
    ok &= expect(strstr(redmcsb_f0661_get_shrinked_bitmap_source_evidence_pc34(),
                        "F0661") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
