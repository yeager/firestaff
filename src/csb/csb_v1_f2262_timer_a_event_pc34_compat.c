#include "csb_v1_f2262_timer_a_event_pc34_compat.h"

#include <string.h>

int csb_v1_f2262_timer_a_event_pc34(
    CSB_V1_F2262_TimerAEventState_PC34 *state,
    CSB_V1_F2262_TimerAEventReceipt_PC34 *out_receipt)
{
    CSB_V1_F2262_TimerAEventReceipt_PC34 receipt;

    memset(&receipt, 0, sizeof(receipt));
    if (!state || state->wait_for_input_maximum_vblank_count == 0u) {
        if (out_receipt) *out_receipt = receipt;
        return 0;
    }

    receipt.wait_for_input_vblank_count_before =
        state->wait_for_input_vblank_count;
    receipt.wait_for_input_maximum_vblank_count =
        state->wait_for_input_maximum_vblank_count;
    receipt.stop_waiting_before = state->stop_waiting_for_player_input ? 1 : 0;

    if (state->wait_for_input_vblank_count != UINT16_MAX) {
        state->wait_for_input_vblank_count++;
    }
    receipt.input_wait_advanced = 1;
    if (state->wait_for_input_vblank_count >=
        state->wait_for_input_maximum_vblank_count) {
        state->stop_waiting_for_player_input = 1;
        receipt.stop_waiting_set = 1;
    }

    if (state->fm_towns_timer_available) {
        state->fm_towns_counter--;
        receipt.fm_towns_counter_advanced = 1;
    }
    if (state->fm_towns_volume_fade_available &&
        state->fm_towns_volume_fade_active) {
        if (state->fm_towns_volume > 0) {
            state->fm_towns_volume--;
        } else {
            state->fm_towns_volume_muted = 1;
            state->fm_towns_volume_fade_active = 0;
        }
        receipt.fm_towns_volume_advanced = 1;
    }
    receipt.fm_towns_platform_branch_unavailable =
        (!state->fm_towns_timer_available &&
         !state->fm_towns_volume_fade_available) ? 1 : 0;

    receipt.wait_for_input_vblank_count_after =
        state->wait_for_input_vblank_count;
    receipt.stop_waiting_after = state->stop_waiting_for_player_input ? 1 : 0;
    receipt.valid = 1;
    if (out_receipt) *out_receipt = receipt;
    return 1;
}

const char *csb_v1_f2262_timer_a_event_source_evidence_pc34(void)
{
    return "ReDMCSB TOWNSIO.C:134-161 F2262_TIMER_A_EVENT increments "
           "G0317_i_WaitForInputVerticalBlankCount and sets "
           "G0321_B_StopWaitingForPlayerInput when it reaches G0318; "
           "SWITCH.C:1071-1078 and ANIMTOWN.C:687-707 show the optional "
           "MEDIA670 FM-Towns sound counter/fade branch.";
}
