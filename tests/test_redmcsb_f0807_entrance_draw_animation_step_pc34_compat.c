#include <stdio.h>
#include <string.h>

#include "redmcsb_f0807_entrance_draw_animation_step_pc34_compat.h"

typedef struct TestState {
    int width_calls;
    int blit_calls;
    int source_token;
    int screen_token;
    const void *source;
    void *destination;
    int16_t xyz[4];
    int16_t source_x;
    int16_t source_y;
    int16_t source_width;
    int16_t destination_width;
    int16_t transparency;
    int16_t flip;
} TestState;

static int16_t bitmap_pixel_width(void *context, const void *bitmap)
{
    TestState *state = context;
    ++state->width_calls;
    return bitmap == &state->source_token ? INT16_C(144) : INT16_C(-1);
}

static void video_blit(void *context, const void *source, void *destination,
                       const int16_t xyz[4], int16_t source_x,
                       int16_t source_y, int16_t source_width,
                       int16_t destination_width, int16_t transparency,
                       int16_t flip)
{
    TestState *state = context;
    ++state->blit_calls;
    state->source = source;
    state->destination = destination;
    memcpy(state->xyz, xyz, sizeof(state->xyz));
    state->source_x = source_x;
    state->source_y = source_y;
    state->source_width = source_width;
    state->destination_width = destination_width;
    state->transparency = transparency;
    state->flip = flip;
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
    const int16_t xyz[4] = { 1, 2, 319, 199 };
    TestState state = { 0 };
    const redmcsb_f0766_renderer_pc34_compat renderer = {
        bitmap_pixel_width, video_blit, &state, &state.screen_token, 320
    };

    redmcsb_f0807_entrance_draw_animation_step_pc34_compat(
        &renderer, &state.source_token, xyz);

    if (check(state.width_calls == 1, "delegates bitmap width to F0766") ||
        check(state.blit_calls == 1, "issues exactly one screen blit") ||
        check(state.source == &state.source_token,
              "preserves G2219 entrance animation bitmap") ||
        check(state.destination == &state.screen_token,
              "uses the renderer screen bitmap") ||
        check(memcmp(state.xyz, xyz, sizeof(xyz)) == 0,
              "passes G2222 XYZ unchanged") ||
        check(state.source_x == 0 && state.source_y == 0,
              "uses the F0766 source origin") ||
        check(state.source_width == 144 && state.destination_width == 320,
              "retains F0766 bitmap and screen widths") ||
        check(state.transparency ==
                  REDMCSB_F0807_COLOR_NO_TRANSPARENCY_PC34_COMPAT,
              "passes CM1_COLOR_NO_TRANSPARENCY") ||
        check(state.flip == REDMCSB_F0766_NO_FLIP_PC34_COMPAT,
              "retains F0766 no-flip behavior") ||
        check(strstr(
                  redmcsb_f0807_entrance_draw_animation_step_source_evidence_pc34(),
                  "ENTRANCE.C:85-90") != NULL,
              "records exact source evidence")) {
        return 1;
    }

    puts("ok: ReDMCSB F0807 entrance animation step");
    return 0;
}
