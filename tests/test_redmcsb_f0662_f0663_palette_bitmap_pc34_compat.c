#include "redmcsb_f0662_f0663_palette_bitmap_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct bitmap {
    int16_t width;
    int16_t height;
    uint8_t pixels[16];
} bitmap;

typedef struct capture {
    int calls;
    const uint8_t *source;
    uint8_t *destination;
    int16_t source_width;
    int16_t source_height;
    int16_t destination_width;
    int16_t destination_height;
    const uint8_t *palette;
} capture;

static void capture_blit(void *context, const uint8_t *source,
                         uint8_t *destination, int16_t source_width,
                         int16_t source_height, int16_t destination_width,
                         int16_t destination_height, const uint8_t *palette)
{
    capture *state = (capture *)context;

    state->calls += 1;
    state->source = source;
    state->destination = destination;
    state->source_width = source_width;
    state->source_height = source_height;
    state->destination_width = destination_width;
    state->destination_height = destination_height;
    state->palette = palette;
}

static int expect(int condition, const char *message)
{
    if (condition) return 1;
    fprintf(stderr, "FAIL: %s\n", message);
    return 0;
}

int main(void)
{
    bitmap source = {6, 4, {1, 2, 3, 4}};
    bitmap destination = {0, 0, {0}};
    uint8_t palette[16] = {0, 1, 2, 3, 4, 5, 6, 7,
                           8, 9, 10, 11, 12, 13, 14, 15};
    capture state;
    redmcsb_f0662_f0663_renderer_pc34_compat renderer;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    renderer.blit_shrink_palette = capture_blit;
    renderer.context = &state;

    ok &= expect(redmcsb_f0662_apply_palette_changes_pc34_compat(
                     &renderer, source.pixels, palette) == 1,
                 "F0662 dispatches in place");
    ok &= expect(state.calls == 1 && state.source == source.pixels &&
                     state.destination == source.pixels && state.source_width == 6 &&
                     state.source_height == 4 && state.destination_width == 6 &&
                     state.destination_height == 4 && state.palette == palette,
                 "F0662 preserves source dimensions and palette pointer");

    state.calls = 0;
    ok &= expect(redmcsb_f0663_copy_bitmap_with_palette_changes_pc34_compat(
                     &renderer, source.pixels, destination.pixels, palette) == 1,
                 "F0663 dispatches source-to-destination copy");
    ok &= expect(destination.width == 6 && destination.height == 4,
                 "F0663 copies source dimension prefix");
    ok &= expect(state.calls == 1 && state.source == source.pixels &&
                     state.destination == destination.pixels &&
                     state.source_width == 6 && state.source_height == 4 &&
                     state.destination_width == 6 && state.destination_height == 4 &&
                     state.palette == palette,
                 "F0663 forwards exact F0129 dimensions and palette");
    ok &= expect(redmcsb_f0662_apply_palette_changes_pc34_compat(
                     NULL, source.pixels, palette) == 0,
                 "F0662 rejects missing renderer");
    ok &= expect(redmcsb_f0663_copy_bitmap_with_palette_changes_pc34_compat(
                     &renderer, NULL, destination.pixels, palette) == 0,
                 "F0663 rejects missing source bitmap");
    ok &= expect(strstr(redmcsb_f0662_f0663_palette_bitmap_source_evidence_pc34(),
                        "F0663") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
