#include "dm1_v1_champion_panel_all_states_pc34_compat.h"

#include <string.h>

/*
 * ReDMCSB source-locked contract gate only.
 *
 * CHAMDRAW.C F0293:1117-1143 is the all-champion redraw dispatcher. This
 * module models only the PC34 dispatch contract: active champion iteration,
 * dirty-mask OR, F0292 call order, and the G2149_ clear after the loop.
 */

static const char s_source_evidence[] =
    "contract_only=1; no real-asset bitmap parity claim; no GRAPHICS.DAT load. "
    "CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143 owns the "
    "all-state dispatch loop. Lines 1134-1138 iterate from C00_CHAMPION_FIRST "
    "while championIndex < G0305_ui_PartyChampionCount, OR P2062_ui_ into "
    "M516_CHAMPIONS[index].Attributes on PC34, and call F0292 for that index. "
    "Lines 1140-1142 clear G2149_ after the loop on PC34. "
    "DEFS.H:724-732 anchors dirty flags 0x0080..0x8000. "
    "DEFS.H:7895-7907 declares F0292/F0293. "
    "Non-overlap: this F0293 dispatch gate does not cover F0287, F0289, "
    "F0290, F0291, F0292 internals, F0622, F0354, or F0355.";

static const dm1_v1_champion_panel_all_states_pc34_compat_evidence_t s_evidence = {
    "F0293_CHAMPION_DrawAllChampionStates",
    "CHAMDRAW.C F0293:1117-1143",
    "CHAMDRAW.C F0293:1134-1138",
    "DEFS.H:724-732",
    "DEFS.H:7895-7907",
    "grep-confirmed non-overlap with existing champion_panel_hud module for F0293",
    "contract-only F0293 dispatch/order/dirty-mask gate",
    "no real-asset bitmap parity; no GRAPHICS.DAT load"
};

static const dm1_v1_champion_panel_all_states_pc34_compat_invariant_t s_invariant = {
    true,
    false,
    false,
    true,
    false,
    true,
    true,
    true,
    true,
    true,
    true,
    DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ALL_DIRTY_PC34,
    DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34
};

static uint16_t clamp_party_count(uint16_t party_champion_count,
                                  bool *rejected_overlarge_party_count)
{
    if (party_champion_count > DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34) {
        if (rejected_overlarge_party_count) {
            *rejected_overlarge_party_count = true;
        }
        return DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34;
    }
    if (rejected_overlarge_party_count) {
        *rejected_overlarge_party_count = false;
    }
    return party_champion_count;
}

dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t
dm1_v1_champion_panel_all_states_pc34_compat_probe(
    const dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t *input)
{
    dm1_v1_champion_panel_all_states_pc34_compat_probe_result_t result;
    dm1_v1_champion_panel_all_states_pc34_compat_probe_input_t local_input;
    uint16_t champion_index;

    memset(&result, 0, sizeof(result));
    result.invariant = s_invariant;
    result.evidence = s_evidence;

    if (!input) {
        memset(&local_input, 0, sizeof(local_input));
        input = &local_input;
        result.null_input_defaults_used = true;
    }

    result.party_champion_count_clamped =
        clamp_party_count(input->party_champion_count,
                          &result.rejected_overlarge_party_count);
    result.redraw_mask_applied =
        input->redraw_mask & DM1_V1_CHAMPION_PANEL_ALL_STATES_ATTR_ALL_DIRTY_PC34;
    result.pc34_pending_all_state_redraw_after = input->pc34_pending_all_state_redraw;

    for (champion_index = 0;
         champion_index < DM1_V1_CHAMPION_PANEL_ALL_STATES_MAX_CHAMPIONS_PC34;
         ++champion_index) {
        result.final_attributes[champion_index] = input->initial_attributes[champion_index];
        result.draw_state_indices[champion_index] = -1;
    }

    /*
     * CHAMDRAW.C F0293:1134-1138: for each active champion, PC34 ORs the
     * incoming dirty mask into Attributes before delegating to F0292.
     */
    for (champion_index = DM1_V1_CHAMPION_PANEL_ALL_STATES_FIRST_CHAMPION_PC34;
         champion_index < result.party_champion_count_clamped;
         ++champion_index) {
        result.final_attributes[champion_index] =
            (uint16_t)(result.final_attributes[champion_index] |
                       result.redraw_mask_applied);
        result.draw_state_indices[result.draw_state_call_count++] =
            (int)champion_index;
    }

    /* CHAMDRAW.C F0293:1140-1142: PC34 clears G2149_ after the loop. */
    result.pc34_pending_all_state_redraw_after = false;

    return result;
}

const dm1_v1_champion_panel_all_states_pc34_compat_evidence_t *
dm1_v1_champion_panel_all_states_pc34_compat_evidence(void)
{
    return &s_evidence;
}

const char *dm1_v1_champion_panel_all_states_pc34_compat_source_evidence(void)
{
    return s_source_evidence;
}
