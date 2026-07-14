#include "redmcsb_f0666_endgame_pc34_compat.h"

#include <stddef.h>

int redmcsb_f0666_endgame_pc34_compat(
    redmcsb_f0666_endgame_state_pc34_compat *state,
    const redmcsb_f0666_endgame_runtime_pc34_compat *runtime)
{
    if (state == NULL || runtime == NULL || runtime->hide_pointer == NULL ||
        runtime->close_graphics_dat == NULL || runtime->restore_cpsx == NULL ||
        runtime->transfer_to_endgame_boundary == NULL) {
        return 0;
    }
    while (state->hide_mouse_pointer_request_count <= 0) {
        runtime->hide_pointer(runtime->context);
    }
    runtime->close_graphics_dat(runtime->context);
    runtime->close_graphics_dat(runtime->context);
    runtime->restore_cpsx(runtime->context);
    runtime->transfer_to_endgame_boundary(runtime->context);
    return 1;
}

const char *redmcsb_f0666_endgame_source_evidence_pc34(void)
{
    return "ReDMCSB ENDGAME.C F0666_endgame (984-1014), "
           "I34E/I34M F0478/F0750/longjmp route";
}
