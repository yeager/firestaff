#include "dm1_v1_champion_panel_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "pass760 contract-only status-hand rotation gate. "
    "CHAMPION.C F0302:677-684 maps C00..C07 status hand slot boxes before "
    "inventory, using slotbox >> 1 and M070_HAND_SLOT_INDEX. "
    "CHAMPION.C F0297:243-298 leader-hand put and F0298:270-298 leader-hand "
    "remove stay before storage priority. CHAMPION.C F0300:511-515 clears "
    "C30+ with G0425_aT_ChestSlots and F0301:606-614 writes C30+ with "
    "G0425_aT_ChestSlots. CHAMPION.C F0302:688-712 locks occupied-slot swap "
    "order. CHAMDRAW.C F0291:621-630 locks the status-panel action-hand icon "
    "path; F0291:634-642 selects C035 acting, C034 wounded, or C033 normal "
    "slot boxes. CHAMDRAW.C F0292:898-935 is recorded only as the champion "
    "state redraw tuple, not as a mouth/eye or food/water duplicate. "
    "DEFS.H:780-817 anchors hand/backpack/belt/C30 slots; DEFS.H:2088 anchors "
    "C10_COLOR_FLESH transparency/color lineage.";

static const dm1_v1_champion_panel_status_hand_rotation_evidence_pc34_t
    s_evidence = {
        "CHAMPION.C F0302:677-684 status hand slot box 0..7 routing",
        "CHAMPION.C F0297:243-298 leader-hand put side effects",
        "CHAMPION.C F0298:270-298 leader-hand remove side effects",
        "CHAMPION.C F0300:511-515 C30+ slot clear through G0425_aT_ChestSlots",
        "CHAMPION.C F0301:606-614 C30+ slot write through G0425_aT_ChestSlots",
        "CHAMPION.C F0302:688-712 occupied-slot swap",
        "CHAMDRAW.C F0291:621-630 champion-panel action-hand slot",
        "CHAMDRAW.C F0292:898-935 champion state redraw tuple",
        "DEFS.H:780-817 C30 slots plus G0425/G0426/G0423/G0305/M070/M516 "
        "related definitions verified at their exact ReDMCSB lines",
        "DEFS.H:2088 C10_COLOR_FLESH transparency",
        "does not duplicate pass673 mouth/eye or pass683 food/water gates"
    };

static void seed_priority_chain(
    dm1_v1_champion_panel_status_hand_rotation_result_pc34_t *result)
{
    result->priority_chain[0] =
        DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_STATUS_HAND_PC34;
    result->priority_chain[1] =
        DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_LEADER_HAND_PC34;
    result->priority_chain[2] =
        DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BACKPACK_PC34;
    result->priority_chain[3] =
        DM1_V1_CHAMPION_PANEL_STATUS_HAND_SOURCE_BELT_PC34;
    result->priority_chain_count =
        DM1_V1_CHAMPION_PANEL_STATUS_HAND_CHAIN_COUNT_PC34;
    result->status_hand_precedes_leader_hand = true;
    result->leader_hand_precedes_backpack = true;
    result->backpack_precedes_belt = true;
}

dm1_v1_champion_panel_status_hand_rotation_input_pc34_t
dm1_v1_champion_panel_status_hand_rotation_default_input_pc34(void)
{
    dm1_v1_champion_panel_status_hand_rotation_input_pc34_t input;

    memset(&input, 0, sizeof(input));
    input.party_champion_count = DM1_V1_CHAMPION_PANEL_MAX_CHAMPIONS_PC34;
    input.acting_champion_index = 0;
    return input;
}

dm1_v1_champion_panel_status_hand_rotation_result_pc34_t
dm1_v1_champion_panel_status_hand_rotation_plan_pc34(
    const dm1_v1_champion_panel_status_hand_rotation_input_pc34_t *input)
{
    dm1_v1_champion_panel_status_hand_rotation_input_pc34_t local_input;
    dm1_v1_champion_panel_status_hand_rotation_result_pc34_t result;
    uint16_t champion;
    uint16_t slot;

    memset(&result, 0, sizeof(result));
    result.evidence = s_evidence;
    result.contract_only = true;
    result.no_mouth_eye_gate_duplicate = true;
    result.no_food_water_gate_duplicate = true;
    seed_priority_chain(&result);

    if (!input) {
        local_input =
            dm1_v1_champion_panel_status_hand_rotation_default_input_pc34();
        input = &local_input;
    }

    result.party_champion_count = input->party_champion_count;
    if (result.party_champion_count >
        DM1_V1_CHAMPION_PANEL_MAX_CHAMPIONS_PC34) {
        result.party_champion_count =
            DM1_V1_CHAMPION_PANEL_MAX_CHAMPIONS_PC34;
    }
    result.acting_champion_index = input->acting_champion_index;

    for (champion = 0;
         champion < DM1_V1_CHAMPION_PANEL_MAX_CHAMPIONS_PC34;
         ++champion) {
        for (slot = 0;
             slot < DM1_V1_CHAMPION_PANEL_STATUS_HAND_SLOTS_PC34;
             ++slot) {
            dm1_v1_champion_panel_status_hand_rotation_box_pc34_t *box =
                &result.boxes[result.box_count++];

            box->slot_box_index = (uint16_t)((champion << 1) | slot);
            box->champion_index = (int16_t)champion;
            box->slot_index = slot;
            box->visible_champion = champion < result.party_champion_count;
            box->action_hand =
                slot == DM1_V1_CHAMPION_PANEL_C01_SLOT_ACTION_HAND_PC34;
            box->status_hand_route_before_inventory =
                box->slot_box_index <
                DM1_V1_CHAMPION_PANEL_STATUS_HAND_SLOTBOXES_PC34;
            box->acting_action_hand =
                box->visible_champion && box->action_hand &&
                ((int16_t)champion == result.acting_champion_index);
            box->wounded_hand =
                box->visible_champion &&
                ((input->wound_masks[champion] & (uint16_t)(1u << slot)) != 0u);

            /*
             * ReDMCSB CHAMDRAW.C F0291:621-630 resolves the action-hand icon
             * before border choice; F0291:634-642 then chooses C035 for the
             * acting action hand, otherwise C034 wounded or C033 normal.
             */
            if (box->acting_action_hand) {
                box->native_bitmap_index =
                    DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34;
            } else if (box->wounded_hand) {
                box->native_bitmap_index =
                    DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34;
            } else {
                box->native_bitmap_index =
                    DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34;
            }

            if (box->visible_champion) {
                ++result.visible_box_count;
                if (box->native_bitmap_index ==
                    DM1_V1_CHAMPION_PANEL_C035_SLOT_BOX_ACTING_HAND_PC34) {
                    ++result.c035_acting_count;
                } else if (box->native_bitmap_index ==
                           DM1_V1_CHAMPION_PANEL_C034_SLOT_BOX_WOUNDED_PC34) {
                    ++result.c034_wounded_count;
                } else if (box->native_bitmap_index ==
                           DM1_V1_CHAMPION_PANEL_C033_SLOT_BOX_NORMAL_PC34) {
                    ++result.c033_normal_count;
                }
            }
        }
    }

    return result;
}

const dm1_v1_champion_panel_status_hand_rotation_evidence_pc34_t *
dm1_v1_champion_panel_status_hand_rotation_evidence_pc34(void)
{
    return &s_evidence;
}

const char *
dm1_v1_champion_panel_status_hand_rotation_source_evidence_pc34(void)
{
    return s_source_evidence;
}
