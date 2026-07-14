#include <stdio.h>
#include <string.h>

#include "redmcsb_f0766_blit_to_screen_pc34_compat.h"

#if !defined(__STDC_VERSION__) || __STDC_VERSION__ < 201112L
#error "This test requires C11 or later."
#endif

typedef struct test_state {
    int pixel_width_calls;
    int blit_calls;
    int source_token;
    int screen_token;
    const void *seen_source;
    void *seen_destination;
    int16_t seen_xyz[4];
    int16_t seen_source_x;
    int16_t seen_source_y;
    int16_t seen_source_width;
    int16_t seen_destination_width;
    int16_t seen_transparent_color;
    int16_t seen_flip;
} test_state;

static int16_t bitmap_pixel_width(void *context, const void *bitmap)
{
    test_state *state = context;
    ++state->pixel_width_calls;
    return bitmap == &state->source_token ? 123 : -1;
}

static void video_blit(void *context, const void *source_bitmap,
                       void *destination_bitmap, const int16_t xyz[4],
                       int16_t source_x, int16_t source_y,
                       int16_t source_pixel_width,
                       int16_t destination_pixel_width,
                       int16_t transparent_color, int16_t flip)
{
    test_state *state = context;
    ++state->blit_calls;
    state->seen_source = source_bitmap;
    state->seen_destination = destination_bitmap;
    memcpy(state->seen_xyz, xyz, sizeof(state->seen_xyz));
    state->seen_source_x = source_x;
    state->seen_source_y = source_y;
    state->seen_source_width = source_pixel_width;
    state->seen_destination_width = destination_pixel_width;
    state->seen_transparent_color = transparent_color;
    state->seen_flip = flip;
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
    const int16_t xyz[4] = { 7, 11, 13, 17 };
    test_state state = { 0 };
    redmcsb_f0766_renderer_pc34_compat renderer = {
        bitmap_pixel_width,
        video_blit,
        &state,
        &state.screen_token,
        320
    };

    redmcsb_f0766_blit_to_screen_pc34_compat(
        &renderer, &state.source_token, xyz, -1);

    if (check(state.pixel_width_calls == 1, "reads M100 bitmap width once") ||
        check(state.blit_calls == 1, "issues exactly one F0132 blit") ||
        check(state.seen_source == &state.source_token,
              "preserves source bitmap") ||
        check(state.seen_destination == &state.screen_token,
              "uses G0348 screen bitmap") ||
        check(memcmp(state.seen_xyz, xyz, sizeof(xyz)) == 0,
              "passes supplied XYZ unchanged") ||
        check(state.seen_source_x == 0 && state.seen_source_y == 0,
              "uses source origin zero zero") ||
        check(state.seen_source_width == 123,
              "uses bitmap M100 pixel width") ||
        check(state.seen_destination_width == 320,
              "uses C320 screen width") ||
        check(state.seen_transparent_color == -1,
              "passes supplied transparency") ||
        check(state.seen_flip == REDMCSB_F0766_NO_FLIP_PC34_COMPAT,
              "uses MASK0x0000_NO_FLIP") ||
        check(strstr(redmcsb_f0766_blit_to_screen_source_evidence_pc34(),
                     "BASE.C:1374-1391") != NULL,
              "records source evidence")) {
        return 1;
    }

    puts("ok: ReDMCSB F0766 PC 3.4 screen blit");
    return 0;
}
