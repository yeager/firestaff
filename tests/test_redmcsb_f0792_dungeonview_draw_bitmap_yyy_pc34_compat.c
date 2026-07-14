#include <stdint.h>
#include <string.h>

#include "redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat.h"

typedef struct TestState {
    int init_calls;
    int zone_calls;
    int width_calls;
    int blit_calls;
    int16_t init_index;
    int16_t zone_index;
    int16_t blit_xyz[4];
    int16_t blit_x;
    int16_t blit_y;
    int16_t source_width;
    int16_t destination_width;
    int16_t transparent_color;
    int16_t flip;
    int return_null;
    int resolve_zone;
} TestState;

static void *init_bitmap(
    void *context,
    int16_t native_bitmap_index,
    RedmcsbF0792BitmapStruct2Pc34 *bitmap)
{
    TestState *state = context;
    static uint8_t pixels[1];

    state->init_calls++;
    state->init_index = native_bitmap_index;
    bitmap->x = 10;
    bitmap->y = 20;
    bitmap->width = 30;
    bitmap->height = 40;
    return state->return_null ? NULL : pixels;
}

static bool resolve_zone(
    void *context,
    const void *bitmap,
    int16_t xyz[4],
    int16_t zone_index,
    int16_t *width,
    int16_t *height)
{
    TestState *state = context;

    (void)bitmap;
    state->zone_calls++;
    state->zone_index = zone_index;
    xyz[0] = 1;
    xyz[1] = 2;
    xyz[2] = 3;
    xyz[3] = 4;
    *width = 7;
    *height = 8;
    return state->resolve_zone != 0;
}

static int16_t bitmap_pixel_width(void *context, const void *bitmap)
{
    TestState *state = context;

    (void)bitmap;
    state->width_calls++;
    return 99;
}

static void video_blit(
    void *context,
    const void *source,
    void *destination,
    const int16_t xyz[4],
    int16_t destination_x,
    int16_t destination_y,
    int16_t source_pixel_width,
    int16_t destination_pixel_width,
    int16_t transparent_color,
    int16_t flip)
{
    TestState *state = context;

    (void)source;
    (void)destination;
    state->blit_calls++;
    memcpy(state->blit_xyz, xyz, sizeof(state->blit_xyz));
    state->blit_x = destination_x;
    state->blit_y = destination_y;
    state->source_width = source_pixel_width;
    state->destination_width = destination_pixel_width;
    state->transparent_color = transparent_color;
    state->flip = flip;
}

int main(void)
{
    TestState state = {0};
    RedmcsbF0792DungeonviewHooksPc34 hooks = {
        &state, init_bitmap, resolve_zone, bitmap_pixel_width, video_blit};

    state.resolve_zone = 1;
    redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat(
        &hooks, &state, 224, -13, 712, 1);
    if (state.init_calls != 1 || state.init_index != -13 || state.zone_calls != 1 ||
        state.zone_index != 712 || state.width_calls != 1 || state.blit_calls != 1 ||
        state.blit_xyz[0] != 1 || state.blit_xyz[1] != 2 || state.blit_xyz[2] != 3 ||
        state.blit_xyz[3] != 4 || state.blit_x != 17 || state.blit_y != 28 ||
        state.source_width != 99 || state.destination_width != 224 ||
        state.transparent_color != -1 || state.flip != 1) {
        return 1;
    }

    state = (TestState){0};
    hooks.context = &state;
    state.return_null = 1;
    redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat(
        &hooks, &state, 224, 4, 712, 0);
    if (state.init_calls != 1 || state.zone_calls != 0 || state.width_calls != 0 ||
        state.blit_calls != 0) {
        return 1;
    }

    state = (TestState){0};
    hooks.context = &state;
    redmcsb_f0792_dungeonview_draw_bitmap_yyy_pc34_compat(
        &hooks, &state, 224, 4, 712, 0);
    if (state.init_calls != 1 || state.zone_calls != 1 || state.width_calls != 0 ||
        state.blit_calls != 0) {
        return 1;
    }

    if (strcmp(
            redmcsb_f0792_dungeonview_draw_bitmap_yyy_source_evidence_pc34(),
            "ReDMCSB DUNVIEW.C:3288-3301; F0630, F0635 and opaque F0132 viewport blit") !=
        0) {
        return 1;
    }

    return 0;
}
