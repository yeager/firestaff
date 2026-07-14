#include "redmcsb_f0657_f0658_viewport_bitmap_index_pc34_compat.h"

#include <stdio.h>
#include <string.h>

typedef struct capture {
    uint8_t bitmap[4];
    int init_calls;
    int resolve_calls;
    int blit_calls;
    int resolve_result;
    int16_t init_index;
    int16_t resolved_input_x;
    int16_t resolved_input_y;
    int16_t xyz[4];
    int16_t blit_x;
    int16_t blit_y;
    int16_t source_width;
    int16_t destination_width;
    int16_t transparent;
    int16_t flip;
} capture;

static const uint8_t *init_bitmap(void *context, int16_t index,
                                  redmcsb_f0657_f0658_bitmap_struct2_pc34_compat *out)
{
    capture *state = (capture *)context;

    state->init_calls += 1;
    state->init_index = index;
    if (index == 77) return NULL;
    out->x = 20;
    out->y = 30;
    out->width = 7;
    out->height = 9;
    return state->bitmap;
}

static int resolve_zone(void *context, const uint8_t *bitmap, int16_t xyz[4],
                        int16_t zone_index, int16_t *x, int16_t *y)
{
    capture *state = (capture *)context;

    (void)bitmap;
    state->resolve_calls += 1;
    state->resolved_input_x = *x;
    state->resolved_input_y = *y;
    if (!state->resolve_result || zone_index != 42) return 0;
    xyz[0] = 1;
    xyz[1] = 2;
    xyz[2] = 3;
    xyz[3] = 4;
    *x = (int16_t)(*x + 4);
    *y = (int16_t)(*y + 5);
    return 1;
}

static int16_t pixel_width(void *context, const uint8_t *bitmap)
{
    (void)context;
    (void)bitmap;
    return 17;
}

static void blit(void *context, const uint8_t *source, uint8_t *destination,
                 const int16_t xyz[4], int16_t x, int16_t y,
                 int16_t source_width, int16_t destination_width,
                 int16_t transparent, int16_t flip)
{
    capture *state = (capture *)context;

    (void)source;
    (void)destination;
    state->blit_calls += 1;
    memcpy(state->xyz, xyz, sizeof(state->xyz));
    state->blit_x = x;
    state->blit_y = y;
    state->source_width = source_width;
    state->destination_width = destination_width;
    state->transparent = transparent;
    state->flip = flip;
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
    uint8_t viewport[8];
    int16_t direct_xyz[4] = {8, 9, 10, 11};
    redmcsb_f0657_f0658_renderer_pc34_compat renderer;
    int ok = 1;

    memset(&state, 0, sizeof(state));
    memset(&renderer, 0, sizeof(renderer));
    renderer.init_bitmap_struct2 = init_bitmap;
    renderer.resolve_zone = resolve_zone;
    renderer.bitmap_pixel_width = pixel_width;
    renderer.video_blit = blit;
    renderer.context = &state;
    renderer.viewport_bitmap = viewport;
    renderer.viewport_pixel_width = 320;

    ok &= expect(redmcsb_f0657_blit_bitmap_index_to_viewport_zone_with_transparency_pc34_compat(
                     &renderer, 5, direct_xyz, 12) == 1,
                 "F0657 dispatches initialized bitmap");
    ok &= expect(state.init_calls == 1 && state.init_index == 5 &&
                     state.blit_calls == 1 && state.blit_x == 20 &&
                     state.blit_y == 30,
                 "F0657 uses F0630 STRUCT2 origin");
    ok &= expect(memcmp(state.xyz, direct_xyz, sizeof(direct_xyz)) == 0 &&
                     state.source_width == 17 && state.destination_width == 320 &&
                     state.transparent == 12 && state.flip == 0,
                 "F0657 forwards F0132 PC34 arguments");

    state.blit_calls = 0;
    state.resolve_result = 1;
    ok &= expect(redmcsb_f0658_blit_bitmap_index_to_zone_index_with_transparency_pc34_compat(
                     &renderer, -1, 42, 10) == 1,
                 "F0658 dispatches negative bitmap index");
    ok &= expect(state.resolve_calls == 1 && state.resolved_input_x == 7 &&
                     state.resolved_input_y == 9,
                 "F0658 seeds F0635 offsets from negative STRUCT2 dimensions");
    ok &= expect(state.blit_calls == 1 && state.blit_x == 31 &&
                     state.blit_y == 44 && state.xyz[0] == 1 && state.xyz[3] == 4,
                 "F0658 adds F0635 output to STRUCT2 origin");

    state.blit_calls = 0;
    ok &= expect(redmcsb_f0658_blit_bitmap_index_to_zone_index_with_transparency_pc34_compat(
                     &renderer, 3, 42, 10) == 1 && state.resolved_input_x == 0 &&
                     state.resolved_input_y == 0 && state.blit_x == 24 &&
                     state.blit_y == 35,
                 "F0658 starts non-negative bitmap offsets at zero");

    state.blit_calls = 0;
    state.resolve_result = 0;
    ok &= expect(redmcsb_f0658_blit_bitmap_index_to_zone_index_with_transparency_pc34_compat(
                     &renderer, -1, 42, 10) == 0 && state.blit_calls == 0,
                 "F0658 does not dispatch after F0635 failure");
    ok &= expect(redmcsb_f0657_blit_bitmap_index_to_viewport_zone_with_transparency_pc34_compat(
                     &renderer, 77, direct_xyz, 10) == 0,
                 "F0657 rejects unavailable real bitmap");
    ok &= expect(strstr(redmcsb_f0657_f0658_viewport_bitmap_index_source_evidence_pc34(),
                        "F0658") != NULL,
                 "source evidence is available");
    return ok ? 0 : 1;
}
