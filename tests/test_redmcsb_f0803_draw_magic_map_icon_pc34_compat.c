#include <stdint.h>
#include <string.h>

#include "redmcsb_f0803_draw_magic_map_icon_pc34_compat.h"

typedef struct TestState {
    int lookup_calls;
    int blit_calls;
    int16_t graphic_index;
    int16_t rectangle[4];
    int16_t source_x;
    int16_t source_y;
    int16_t transparent_color;
    const void *source;
    void *destination;
} TestState;

static const void *get_native_bitmap(void *context, int16_t graphic_index)
{
    TestState *state = context;
    static const uint8_t magic_map_icons[1] = {0};

    state->lookup_calls++;
    state->graphic_index = graphic_index;
    return magic_map_icons;
}

static void video_blit(
    void *context,
    const void *source,
    void *destination,
    const int16_t source_rectangle[4],
    int16_t source_x,
    int16_t source_y,
    int16_t transparent_color)
{
    TestState *state = context;

    state->blit_calls++;
    state->source = source;
    state->destination = destination;
    memcpy(state->rectangle, source_rectangle, sizeof(state->rectangle));
    state->source_x = source_x;
    state->source_y = source_y;
    state->transparent_color = transparent_color;
}

int main(void)
{
    TestState state = {0};
    int viewport = 0;
    RedmcsbF0803DrawMagicMapIconHooksPc34 hooks = {
        &state, get_native_bitmap, video_blit};

    redmcsb_f0803_draw_magic_map_icon_pc34_compat(
        &hooks, 12, &viewport, 0x23, 31, 47, 9, 11);
    if (state.lookup_calls != 1 || state.graphic_index != 12 ||
        state.blit_calls != 1 || state.source == NULL ||
        state.destination != &viewport || state.rectangle[0] != 31 ||
        state.rectangle[1] != 47 || state.rectangle[2] != 9 ||
        state.rectangle[3] != 11 || state.source_x != 27 ||
        state.source_y != 22 || state.transparent_color != 10) {
        return 1;
    }

    if (strcmp(
            redmcsb_f0803_draw_magic_map_icon_source_evidence_pc34(),
            "ReDMCSB PANEL.C:534-543; F0489 lookup, F0787 XYZ zone, F0654/F0132 icon blit") !=
        0) {
        return 1;
    }

    return 0;
}
