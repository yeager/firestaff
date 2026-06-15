#include "dm1_v1_mirror_candidate_inventory_toggle_pc34_compat.h"

#include <string.h>

/* ReDMCSB source-lock anchors for this contract-only slice:
 * COMMAND.C F0380:2180-2184 gates C007..C011 before dispatching
 * F0355_INVENTORY_Toggle_CPSE(championIndex).
 * DEFS.H:244-248 defines C007..C011; DEFS.H:712-716 defines
 * C04_CHAMPION_CLOSE_INVENTORY.
 * DEFS.H:5694 and 5700 define G0299_ui_CandidateChampionOrdinal and
 * G0305_ui_PartyChampionCount; DEFS.H:5876 defines G0423 inventory ordinal.
 * PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2248 is the cited entrypoint;
 * this module models only the F0380 gate and never toggles real inventory.
 * COMMAND.C:2302-2311 is the sibling spell/action !G0299 gate.
 */

static const Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat
    s_evidence = {
        "COMMAND.C F0380 inventory-toggle gate:2180-2184",
        "DEFS.H C007..C011 command constants:244-248",
        "DEFS.H C04_CHAMPION_CLOSE_INVENTORY:712-716",
        "DEFS.H G0299/G0305 globals:5694,5700",
        "DEFS.H G0423 inventory champion ordinal:5876",
        "PANEL.C F0355_INVENTORY_Toggle_CPSE entrypoint:2244-2248",
        "COMMAND.C F0380 spell/action !G0299 gates:2302-2311",
        "contract-only command gate; no real inventory, champion, savegame, "
        "asset, or bitmap data is loaded or claimed",
        "disjoint from dm1_v1_mirror_candidate_click_cancel, "
        "dm1_v1_mirror_candidate_close_button, "
        "dm1_v1_mirror_candidate_icon_refresh, "
        "dm1_v1_mirror_candidate_reincarnate_rearm, "
        "dm1_v1_mirror_candidate_resurrect_rearm, "
        "dm1_v1_champion_mirror, dm1_v1_champion_mirror_click_closed, "
        "dm1_v1_champion_mirror_pc34_compat"
    };

const Dm1V1MirrorCandidateInventoryToggleEvidencePc34Compat *
dm1_v1_mirror_candidate_inventory_toggle_pc34_compat_evidence(void)
{
    return &s_evidence;
}

int dm1_v1_mirror_candidate_inventory_toggle_pc34_compat_probe(
    const Dm1V1MirrorCandidateInventoryToggleInputPc34Compat *input,
    Dm1V1MirrorCandidateInventoryToggleOutputPc34Compat *output)
{
    int championIndex;

    if (!output) {
        return 0;
    }

    memset(output, 0, sizeof(*output));
    output->evidence = &s_evidence;
    output->target_champion_index =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NONE_PC34_COMPAT;
    output->computed_champion_index =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NONE_PC34_COMPAT;
    output->command_range_low =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C007_PC34_COMPAT;
    output->command_range_high =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C011_PC34_COMPAT;
    output->close_inventory_index =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_CLOSE_PC34_COMPAT;
    output->route_taken =
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_NOT_IN_INVENTORY_TOGGLE_RANGE_PC34_COMPAT;
    output->contract_only = 1;

    if (!input) {
        return 0;
    }

    output->inventory_ordinal_before =
        input->current_inventory_champion_ordinal;
    output->inventory_ordinal_after =
        input->current_inventory_champion_ordinal;
    output->command_in_inventory_toggle_range =
        input->command >=
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C007_PC34_COMPAT &&
        input->command <=
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C011_PC34_COMPAT;

    if (!output->command_in_inventory_toggle_range) {
        return 0;
    }

    championIndex = input->command -
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_C007_PC34_COMPAT;
    output->computed_champion_index = championIndex;
    output->target_champion_index = championIndex;
    output->is_close_inventory_command =
        championIndex ==
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_CLOSE_PC34_COMPAT;
    output->champion_index_inside_party =
        championIndex >= 0 &&
        championIndex < (int)input->party_champion_count;
    output->party_gate_passed =
        output->is_close_inventory_command ||
        output->champion_index_inside_party;
    output->candidate_gate_passed =
        input->candidate_champion_ordinal == 0u;

    if (!output->candidate_gate_passed) {
        output->route_taken =
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_BY_G0299_PC34_COMPAT;
        return 0;
    }
    if (!output->party_gate_passed) {
        output->route_taken =
            DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_BLOCKED_OUT_OF_PARTY_PC34_COMPAT;
        return 0;
    }

    output->should_dispatch_toggle = 1;
    output->would_call_f0355 = 1;
    output->route_taken = output->is_close_inventory_command ?
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_CLOSED_INVENTORY_PC34_COMPAT :
        DM1_V1_MIRROR_CANDIDATE_INVENTORY_TOGGLE_DISPATCHED_PARTY_CHAMPION_PC34_COMPAT;
    return 1;
}
