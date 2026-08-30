#include "csb_v1_atari_st_vblank.h"

#include <stdio.h>

typedef struct {
    CSB_V1_AtariStVBlank *vblank;
    unsigned palette_calls;
    unsigned original_calls;
    int inject_two_vblanks;
} TestState;

static void palette_start(void *context)
{
    TestState *state = context;
    ++state->palette_calls;
}

static void original_handler(void *context)
{
    TestState *state = context;
    ++state->original_calls;
    if (state->inject_two_vblanks && state->original_calls == 1u) {
        if (!csb_v1_atari_st_vblank_deliver(state->vblank) ||
            !csb_v1_atari_st_vblank_deliver(state->vblank))
            state->inject_two_vblanks = 0;
    }
}

int main(void)
{
    CSB_V1_AtariStVBlank vblank;
    TestState state = { &vblank, 0u, 0u, 1 };

    csb_v1_atari_st_vblank_init(&vblank, palette_start, original_handler,
                                 &state);
    if (!csb_v1_atari_st_vblank_deliver(&vblank) ||
        state.palette_calls != 3u || state.original_calls != 3u ||
        vblank.received_count != 3u || vblank.palette_start_count != 3u ||
        vblank.original_handler_count != 3u ||
        vblank.maximum_concurrent_count != 3u || vblank.concurrent_count != 0u ||
        vblank.handling || vblank.interrupt_priority_mask != 4) {
        return 1;
    }
    if (csb_v1_atari_st_vblank_deliver(NULL)) return 1;
    csb_v1_atari_st_vblank_init(&vblank, NULL, NULL, NULL);
    if (csb_v1_atari_st_vblank_deliver(&vblank)) return 1;
    puts("csb_atari_vblank: retained concurrent VBlanks and palette starts");
    return 0;
}
