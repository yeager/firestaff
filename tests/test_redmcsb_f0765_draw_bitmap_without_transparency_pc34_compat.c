#include <stdio.h>
#include <string.h>

#include "redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct test_state {
    int init_calls;
    int zone_calls;
    int pixel_width_calls;
    int blit_calls;
    int return_null_bitmap;
    int accept_zone;
    int16_t seen_native_index;
    int16_t seen_zone_index;
    int16_t seen_xyz[4];
    int16_t seen_source_x;
    int16_t seen_source_y;
    int16_t seen_source_pixel_width;
    int16_t seen_viewport_pixel_width;
    int16_t seen_transparency_color;
    int bitmap_token;
    int viewport_token;
} test_state;

static void *init_bitmap(void *context, int16_t native_bitmap_index,
                         redmcsb_f0765_bitmap_struct2_pc34 *bitmap_struct2)
{
    test_state *state = context;
    ++state->init_calls;
    state->seen_native_index = native_bitmap_index;
    bitmap_struct2->x = 12;
    bitmap_struct2->y = 24;
    bitmap_struct2->width = 30;
    bitmap_struct2->height = 40;
    return state->return_null_bitmap ? NULL : &state->bitmap_token;
}

static int init_zone(void *context, void *bitmap, int16_t xyz[4],
                     int16_t zone_index, int16_t *width, int16_t *height)
{
    test_state *state = context;
    ++state->zone_calls;
    if (bitmap != &state->bitmap_token) {
        return 0;
    }
    state->seen_zone_index = zone_index;
    xyz[0] = 1;
    xyz[1] = 2;
    xyz[2] = 3;
    xyz[3] = 4;
    *width = 33;
    *height = 44;
    return state->accept_zone;
}

static int16_t bitmap_pixel_width(void *context, const void *bitmap)
{
    test_state *state = context;
    ++state->pixel_width_calls;
    return bitmap == &state->bitmap_token ? 160 : 0;
}

static void video_blit(void *context, const void *bitmap, void *viewport,
                       const int16_t xyz[4], int16_t source_x,
                       int16_t source_y, int16_t source_pixel_width,
                       int16_t viewport_pixel_width,
                       int16_t transparency_color)
{
    test_state *state = context;
    ++state->blit_calls;
    if (bitmap != &state->bitmap_token || viewport != &state->viewport_token) {
        state->blit_calls = -100;
    }
    memcpy(state->seen_xyz, xyz, sizeof(state->seen_xyz));
    state->seen_source_x = source_x;
    state->seen_source_y = source_y;
    state->seen_source_pixel_width = source_pixel_width;
    state->seen_viewport_pixel_width = viewport_pixel_width;
    state->seen_transparency_color = transparency_color;
}

static int check(int condition, const char *label)
{
    if (condition) {
        return 0;
    }
    fprintf(stderr, "failed: %s\n", label);
    return 1;
}

int main(void)
{
    test_state state = { 0 };
    redmcsb_f0765_renderer_pc34_compat renderer = {
        init_bitmap, init_zone, bitmap_pixel_width, video_blit,
        &state, &state.viewport_token, 224
    };

    redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat(
        &renderer, 77, 88);
    if (check(state.init_calls == 1, "initializes STRUCT2") ||
        check(state.zone_calls == 1, "initializes zone") ||
        check(state.pixel_width_calls == 0, "does not blit rejected zone") ||
        check(state.blit_calls == 0, "does not blit rejected zone")) {
        return 1;
    }

    state.accept_zone = 1;
    redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat(
        &renderer, 91, 92);
    if (check(state.init_calls == 2, "initializes on accepted path") ||
        check(state.seen_native_index == 91, "passes bitmap index") ||
        check(state.seen_zone_index == 92, "passes zone index") ||
        check(state.pixel_width_calls == 1, "gets native bitmap width") ||
        check(state.blit_calls == 1, "blits accepted zone") ||
        check(state.seen_xyz[0] == 1 && state.seen_xyz[3] == 4,
              "passes F0635 XYZ") ||
        check(state.seen_source_x == 45 && state.seen_source_y == 68,
              "uses STRUCT2 right and bottom") ||
        check(state.seen_source_pixel_width == 160, "uses M100 width") ||
        check(state.seen_viewport_pixel_width == 224, "uses viewport width") ||
        check(state.seen_transparency_color ==
                  REDMCSB_F0765_COLOR_NO_TRANSPARENCY_PC34_COMPAT,
              "forces opaque blit") ||
        check(strstr(
                  redmcsb_f0765_draw_bitmap_without_transparency_source_evidence_pc34(),
                  "DUNVIEW.C:3159-3185") != NULL,
              "records source evidence")) {
        return 1;
    }

    state.return_null_bitmap = 1;
    redmcsb_f0765_draw_bitmap_without_transparency_pc34_compat(
        &renderer, 93, 94);
    if (check(state.init_calls == 3, "tries null bitmap") ||
        check(state.zone_calls == 2, "does not frame null bitmap") ||
        check(state.blit_calls == 1, "does not blit null bitmap")) {
        return 1;
    }

    puts("ok: ReDMCSB F0765 PC 3.4 opaque bitmap draw");
    return 0;
}
