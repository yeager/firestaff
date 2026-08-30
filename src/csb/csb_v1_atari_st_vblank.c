#include "csb_v1_atari_st_vblank.h"

#include <string.h>

void csb_v1_atari_st_vblank_init(
    CSB_V1_AtariStVBlank *state,
    CSB_V1_AtariStVBlankCallback start_palette_switch,
    CSB_V1_AtariStVBlankCallback run_original_handler,
    void *context)
{
    if (state == NULL) return;
    memset(state, 0, sizeof(*state));
    state->start_palette_switch = start_palette_switch;
    state->run_original_handler = run_original_handler;
    state->context = context;
    /* The processor enters a level-4 autovector with this mask. */
    state->interrupt_priority_mask = 4;
}

int csb_v1_atari_st_vblank_deliver(CSB_V1_AtariStVBlank *state)
{
    if (state == NULL || state->run_original_handler == NULL) return 0;

    ++state->received_count;

    /* BASE.C:E0017:T0017001 runs before CHANGE7_01's concurrent-handler
     * guard. Therefore each VBlank begins palette setup even when the rest
     * of the prior handler is still running. */
    ++state->palette_start_count;
    if (state->start_palette_switch != NULL)
        state->start_palette_switch(state->context);

    if (state->handling) {
        ++state->concurrent_count;
        if (state->concurrent_count > state->maximum_concurrent_count)
            state->maximum_concurrent_count = state->concurrent_count;
        return 1;
    }

    state->handling = 1;
    state->concurrent_count = 1;
    state->maximum_concurrent_count = 1;
    do {
        /* CHANGE7_01 lowers SR's interrupt-priority mask from 4 to 3,
         * allowing level-4 VBlanks to increment the counter. */
        state->interrupt_priority_mask = 3;
        ++state->original_handler_count;
        state->run_original_handler(state->context);
        state->interrupt_priority_mask = 4;
        --state->concurrent_count;
    } while (state->concurrent_count != 0u);
    state->handling = 0;
    return 1;
}
