#include "redmcsb_f0664_knock_front_wall_pc34_compat.h"

#include <stddef.h>

int redmcsb_f0664_process_click_in_dungeon_view_knock_on_front_wall_pc34_compat(
    redmcsb_f0664_wall_click_state_pc34_compat *state,
    const redmcsb_f0664_wall_click_runtime_pc34_compat *runtime,
    int closed_imaginary_fake_wall,
    uint16_t map_x,
    uint16_t map_y)
{
    if (state == NULL || runtime == NULL || state->party_champion_count == 0) {
        return 0;
    }
    if (closed_imaginary_fake_wall) {
        if (state->pressing_closed_imaginary_fake_wall) return 0;
        if (runtime->get_mouse_state == NULL) return 0;
        state->ignore_mouse_movements = 1;
        state->pressing_closed_imaginary_fake_wall = 1;
        runtime->get_mouse_state(runtime->context, &state->mouse_buttons_status);
        if ((state->mouse_buttons_status & REDMCSB_F0664_PC34_MOUSE_LEFT_BUTTON) == 0) {
            state->ignore_mouse_movements = 0;
            state->pressing_closed_imaginary_fake_wall = 0;
        } else if (runtime->hide_pointer != NULL) {
            runtime->hide_pointer(runtime->context);
        }
    } else {
        if (runtime->request_sound == NULL) return 0;
        runtime->request_sound(runtime->context, map_x, map_y,
                               REDMCSB_F0664_PC34_SOUND_WOODEN_THUD,
                               REDMCSB_F0664_PC34_SOUND_PLAY_IMMEDIATELY);
    }
    state->stop_waiting_for_player_input = 1;
    return 1;
}

const char *redmcsb_f0664_knock_front_wall_source_evidence_pc34(void)
{
    return "ReDMCSB CLIKVIEW.C "
           "F0664_COMMAND_ProcessType80_ClickInDungeonView_KnockOnFrontWall "
           "(30-69), PC I34E/I34M branch";
}
