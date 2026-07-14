#include "redmcsb_f0665_highlight_box_pc34_compat.h"

#include <stddef.h>
#include <string.h>

int redmcsb_f0665_highlight_box_enable_for_zone_pc34_compat(
    redmcsb_f0665_highlight_state_pc34_compat *state,
    const redmcsb_f0665_highlight_runtime_pc34_compat *runtime,
    int16_t zone_index)
{
    int16_t xyz[4];

    if (state == NULL || runtime == NULL || runtime->get_zone == NULL ||
        runtime->enable_screen_update == NULL || runtime->invert_box == NULL ||
        runtime->disable_screen_update == NULL ||
        runtime->wait_vertical_blank == NULL) {
        return 0;
    }
    if (!runtime->get_zone(runtime->context, zone_index, xyz)) return 0;
    runtime->enable_screen_update(runtime->context);
    memcpy(state->highlighted_zone, xyz, sizeof(xyz));
    runtime->invert_box(runtime->context, xyz);
    state->highlight_box_enabled = 1;
    runtime->disable_screen_update(runtime->context);
    runtime->wait_vertical_blank(runtime->context);
    return 1;
}

const char *redmcsb_f0665_highlight_box_source_evidence_pc34(void)
{
    return "ReDMCSB CLIKMENU.C F0665_F0362_sub (10-32); "
           "F0362_COMMAND_HighlightBoxEnable PC zone route (64-79)";
}
