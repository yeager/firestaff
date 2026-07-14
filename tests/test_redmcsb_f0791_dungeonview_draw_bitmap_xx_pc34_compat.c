#include <stdio.h>
#include <string.h>

#include "redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct test_state {
    int init_calls;
    int width_calls;
    int height_calls;
    int blit_calls;
    int init_result;
    int source_token;
    int destination_token;
    int16_t seen_zone;
    int16_t received_x;
    int16_t received_y;
    int16_t seen_xyz[4];
    int16_t seen_source_x;
    int16_t seen_source_y;
    int16_t seen_source_width;
    int16_t seen_destination_width;
    int16_t seen_transparency;
    uint16_t seen_flip;
} test_state;

static int init_zone(void *context, const void *source, int16_t xyz[4],
                     int16_t zone, int16_t *x, int16_t *y)
{
    test_state *state = context;
    ++state->init_calls;
    if (source != &state->source_token) return 0;
    state->seen_zone = zone;
    state->received_x = *x;
    state->received_y = *y;
    xyz[0] = 10; xyz[1] = 20; xyz[2] = 30; xyz[3] = 20;
    return state->init_result;
}

static int16_t bitmap_width(void *context, const void *bitmap)
{
    test_state *state = context;
    ++state->width_calls;
    return bitmap == &state->source_token ? 100 : 224;
}

static int16_t bitmap_height(void *context, const void *bitmap)
{
    test_state *state = context;
    (void)bitmap;
    ++state->height_calls;
    return 80;
}

static void video_blit(void *context, const void *source, void *destination,
                       const int16_t xyz[4], int16_t source_x,
                       int16_t source_y, int16_t source_width,
                       int16_t destination_width, int16_t transparency,
                       uint16_t flip)
{
    test_state *state = context;
    ++state->blit_calls;
    if (source != &state->source_token || destination != &state->destination_token) {
        state->blit_calls = -100;
    }
    memcpy(state->seen_xyz, xyz, sizeof(state->seen_xyz));
    state->seen_source_x = source_x;
    state->seen_source_y = source_y;
    state->seen_source_width = source_width;
    state->seen_destination_width = destination_width;
    state->seen_transparency = transparency;
    state->seen_flip = flip;
}

static int check(int condition, const char *label)
{
    if (condition) return 0;
    fprintf(stderr, "failed: %s\n", label);
    return 1;
}

int main(void)
{
    test_state state = { 0 };
    redmcsb_f0791_renderer_pc34_compat renderer = {
        init_zone, bitmap_width, bitmap_height, video_blit, &state
    };

    redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat(
        &renderer, &state.source_token, &state.destination_token,
        REDMCSB_F0791_ZONE_UNKNOWN_PC34_COMPAT, 0, 10, 4, 5);
    if (check(state.init_calls == 0 && state.blit_calls == 0,
              "CM1_UNKNOWN returns before F0635/F0132")) return 1;

    state.init_result = 1;
    redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat(
        &renderer, &state.source_token, &state.destination_token,
        (int16_t)(REDMCSB_F0791_SHIFT_OBJECTS_AND_CREATURES_PC34_COMPAT |
                  REDMCSB_F0791_SHIFT_UNREADABLE_INSCRIPTION_PC34_COMPAT | 701),
        REDMCSB_F0791_FLIP_HORIZONTAL_PC34_COMPAT |
            REDMCSB_F0791_FLIP_VERTICAL_PC34_COMPAT,
        10, 4, 5);
    if (check(state.init_calls == 1, "calls F0635 once") ||
        check(state.seen_zone == (int16_t)(REDMCSB_F0791_SHIFT_OBJECTS_AND_CREATURES_PC34_COMPAT | 701),
              "clears only MASK0x4000 before F0635") ||
        check(state.received_x == 4 && state.received_y == 5,
              "shifted zone seeds G2154/G2155") ||
        check(state.blit_calls == 1, "F0635 success calls F0132") ||
        check(state.seen_source_x == 66 && state.seen_source_y == 55,
              "PC34 flip offsets use bitmap and zone extents") ||
        check(state.seen_source_width == 100 && state.seen_destination_width == 224,
              "uses both M100 pixel widths") ||
        check(state.seen_transparency == 10 && state.seen_flip == 3,
              "preserves C10 and flip") ||
        check(state.seen_xyz[0] == 10 && state.seen_xyz[3] == 20,
              "passes F0635 XYZ unchanged")) return 1;

    state.init_result = 0;
    redmcsb_f0791_dungeonview_draw_bitmap_xx_pc34_compat(
        &renderer, &state.source_token, &state.destination_token, 702,
        0, -1, 4, 5);
    if (check(state.init_calls == 2, "tries non-shifted zone") ||
        check(state.received_x == 0 && state.received_y == 0,
              "ordinary zone begins at zero offsets") ||
        check(state.blit_calls == 1, "F0635 failure skips F0132") ||
        check(strstr(redmcsb_f0791_dungeonview_draw_bitmap_xx_source_evidence_pc34(),
                     "DUNVIEW.C:3394-3473") != NULL,
              "records exact source evidence")) return 1;

    puts("ok: ReDMCSB F0791 PC 3.4 zone bitmap blit");
    return 0;
}
