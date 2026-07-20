#include "csb_v1_f2262_timer_a_event_pc34_compat.h"

#include <assert.h>
#include <string.h>

int main(void)
{
    CSB_V1_F2262_TimerAEventState_PC34 state;
    CSB_V1_F2262_TimerAEventReceipt_PC34 receipt;

    /* Under NDEBUG (Release) the assert-only uses of receipt compile
     * out; keep the variable marked used so -Werror stays green. */
    (void)receipt;
    memset(&state, 0, sizeof(state));
    state.wait_for_input_maximum_vblank_count = 3u;
    assert(csb_v1_f2262_timer_a_event_pc34(&state, &receipt));
    assert(receipt.valid);
    assert(receipt.input_wait_advanced);
    assert(receipt.wait_for_input_vblank_count_before == 0u);
    assert(receipt.wait_for_input_vblank_count_after == 1u);
    assert(!receipt.stop_waiting_set);
    assert(!state.stop_waiting_for_player_input);
    assert(receipt.fm_towns_platform_branch_unavailable);

    assert(csb_v1_f2262_timer_a_event_pc34(&state, &receipt));
    assert(receipt.wait_for_input_vblank_count_after == 2u);
    assert(!receipt.stop_waiting_set);
    assert(csb_v1_f2262_timer_a_event_pc34(&state, &receipt));
    assert(receipt.wait_for_input_vblank_count_before == 2u);
    assert(receipt.wait_for_input_vblank_count_after == 3u);
    assert(receipt.stop_waiting_set);
    assert(state.stop_waiting_for_player_input);

    memset(&state, 0, sizeof(state));
    state.wait_for_input_maximum_vblank_count = 1u;
    state.fm_towns_timer_available = 1;
    state.fm_towns_counter = 7;
    state.fm_towns_volume_fade_available = 1;
    state.fm_towns_volume_fade_active = 1;
    state.fm_towns_volume = 2;
    assert(csb_v1_f2262_timer_a_event_pc34(&state, &receipt));
    assert(receipt.fm_towns_counter_advanced);
    assert(state.fm_towns_counter == 6);
    assert(receipt.fm_towns_volume_advanced);
    assert(state.fm_towns_volume == 1);
    assert(!receipt.fm_towns_platform_branch_unavailable);

    state.fm_towns_volume = 0;
    assert(csb_v1_f2262_timer_a_event_pc34(&state, &receipt));
    assert(receipt.fm_towns_volume_advanced);
    assert(state.fm_towns_volume_muted);
    assert(!state.fm_towns_volume_fade_active);

    memset(&state, 0, sizeof(state));
    assert(!csb_v1_f2262_timer_a_event_pc34(&state, &receipt));
    assert(!receipt.valid);
    assert(csb_v1_f2262_timer_a_event_source_evidence_pc34()[0] != '\0');
    return 0;
}
