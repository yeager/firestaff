#include "firestaff/dm1/v1/champion/dm1_v1_champion_panel_second_leader_hand_slot_priority_pc34_compat.h"

#include "dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"

#include <string.h>

static const char s_source_evidence[] =
    "pass764 contract_only=1; champion index 1 leader status-hand priority "
    "extension. CHAMPION.C F0302:677-684 maps slot-box 3 to champion 1 action "
    "hand with M070 before inventory routing; F0302:688-712 preserves leader "
    "hand snapshot, F0298, F0300, F0297, F0301, final F0292 order. "
    "CHAMPION.C F0297/F0298:243-298, F0300:511-515, F0301:606-614 preserve "
    "leader/load/storage side effects. CHAMDRAW.C F0287:307-342 uses champion "
    "1 bar zones C196/C200/C204 and C12 blank plus champion colour fill. "
    "CHAMDRAW.C F0292:771-815 fills C152 67x29 with C12, F0292:843-895 uses "
    "the PC34 C11 leader/C09 nonleader name-color cascade, and F0292:1019-1051 "
    "draws the 19x14 champion icon in C114 for champion 1. CHAMDRAW.C "
    "F0291:632-651 keeps C033/C034/C035 18x18 hand-slot-box cascade at C214. "
    "DEFS.H:780-817,1878,2178-2199,3779-3807,5700-5881 anchor the constants.";

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t hash_model(
    const dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t *model)
{
    uint32_t hash = 2166136261u;

    hash = mix_u32(hash, (uint32_t)model->champion_index);
    hash = mix_u32(hash, (uint32_t)model->leader_index);
    hash = mix_u32(hash, (uint32_t)model->slot_box_index);
    hash = mix_u32(hash, (uint32_t)model->target_slot_index);
    hash = mix_u32(hash, (uint32_t)model->status_zone);
    hash = mix_u32(hash, (uint32_t)model->status_x);
    hash = mix_u32(hash, (uint32_t)model->status_y);
    hash = mix_u32(hash, (uint32_t)model->status_width);
    hash = mix_u32(hash, (uint32_t)model->status_height);
    hash = mix_u32(hash, (uint32_t)model->status_fill_color);
    hash = mix_u32(hash, (uint32_t)model->name_zone);
    hash = mix_u32(hash, (uint32_t)model->name_text_zone);
    hash = mix_u32(hash, (uint32_t)model->leader_name_color);
    hash = mix_u32(hash, (uint32_t)model->nonleader_name_color);
    hash = mix_u32(hash, (uint32_t)model->hp_bar_zone);
    hash = mix_u32(hash, (uint32_t)model->hp_bar_x);
    hash = mix_u32(hash, (uint32_t)model->hp_bar_y);
    hash = mix_u32(hash, (uint32_t)model->hp_blank_height);
    hash = mix_u32(hash, (uint32_t)model->hp_fill_height);
    hash = mix_u32(hash, (uint32_t)model->hp_blank_color);
    hash = mix_u32(hash, (uint32_t)model->hp_fill_color);
    hash = mix_u32(hash, (uint32_t)model->icon_zone);
    hash = mix_u32(hash, (uint32_t)model->icon_width);
    hash = mix_u32(hash, (uint32_t)model->icon_height);
    hash = mix_u32(hash, (uint32_t)model->icon_fill_color);
    hash = mix_u32(hash, (uint32_t)model->action_hand_zone);
    hash = mix_u32(hash, (uint32_t)model->action_hand_x);
    hash = mix_u32(hash, (uint32_t)model->action_hand_y);
    hash = mix_u32(hash, (uint32_t)model->action_hand_width);
    hash = mix_u32(hash, (uint32_t)model->action_hand_height);
    hash = mix_u32(hash, (uint32_t)model->action_hand_graphic);
    hash = mix_u32(hash, model->transaction_accepted ? 1u : 0u);
    hash = mix_u32(hash, (uint32_t)model->transaction_target_champion_index);
    hash = mix_u32(hash, (uint32_t)model->transaction_target_slot_index);
    hash = mix_u32(hash, model->transaction_leader_is_target ? 1u : 0u);
    hash = mix_u32(hash, model->f0292_final_draw_state ? 1u : 0u);
    return hash;
}

const char *
dm1_v1_champion_panel_second_leader_hand_slot_priority_source_pc34(void)
{
    return s_source_evidence;
}

bool dm1_v1_champion_panel_second_leader_hand_slot_priority_build_pc34(
    dm1_v1_champion_panel_second_leader_hand_slot_priority_model_t *out_model)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input;
    dm1_v1_champion_panel_hand_slot_priority_result_t transaction;
    DM1_ChampionPanel_StatusBoxModel status;
    DM1_ChampionPanel_BarFillModel hp_bar;
    DM1_ChampionPanel_StatusHandSlotBoxModel action_hand;
    int call_index;

    if (!out_model) {
        return false;
    }

    memset(out_model, 0, sizeof(*out_model));

    input = dm1_v1_champion_panel_hand_slot_priority_default_input_pc34();
    input.slot_box_index = DM1_V1_CP_SECOND_LEADER_SLOT_BOX_INDEX_PC34;
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.leader_champion_index = DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34;
    input.leader_hand_thing = 0x0101u;
    input.slot_thing = 0x0202u;
    input.leader_object_allowed_slots = 0x0002u;
    input.slot_mask = 0x0002u;
    transaction =
        dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);

    if (!transaction.accepted ||
        !DM1_ChampionPanel_BuildStatusBoxModel(
            DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34,
            DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34,
            0,
            100,
            &status) ||
        !DM1_ChampionPanel_BuildPc34BarFillModel(
            DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34,
            DM1_STATUS_VALUE_HEALTH,
            50,
            100,
            &hp_bar) ||
        !DM1_ChampionPanel_BuildStatusHandSlotBoxModel(
            DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34,
            DM1_SLOT_ACTION_HAND,
            1,
            &action_hand)) {
        return false;
    }

    out_model->champion_index = DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34;
    out_model->leader_index = DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34;
    out_model->slot_box_index = DM1_V1_CP_SECOND_LEADER_SLOT_BOX_INDEX_PC34;
    out_model->target_slot_index = DM1_SLOT_ACTION_HAND;
    out_model->status_zone = DM1_V1_CP_SECOND_LEADER_STATUS_ZONE_PC34;
    out_model->status_x =
        DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34 *
        DM1_STATUS_BOX_SPACING;
    out_model->status_y = 0;
    out_model->status_width = DM1_STATUS_BOX_WIDTH;
    out_model->status_height = DM1_STATUS_BOX_HEIGHT;
    out_model->status_fill_color = status.fillColor;
    out_model->name_zone = DM1_V1_CP_SECOND_LEADER_NAME_ZONE_PC34;
    out_model->name_text_zone = DM1_V1_CP_SECOND_LEADER_NAME_TEXT_ZONE_PC34;
    out_model->leader_name_color = DM1_COLOR_YELLOW;
    out_model->nonleader_name_color = DM1_COLOR_GOLD;
    out_model->hp_bar_zone = hp_bar.zoneId;
    out_model->hp_bar_x = hp_bar.x;
    out_model->hp_bar_y = hp_bar.y;
    out_model->hp_blank_height = hp_bar.blankHeight;
    out_model->hp_fill_height = hp_bar.fillHeight;
    out_model->hp_blank_color = hp_bar.blankColor;
    out_model->hp_fill_color = hp_bar.fillColor;
    out_model->icon_zone = DM1_V1_CP_SECOND_LEADER_ICON_ZONE_PC34;
    out_model->icon_width = DM1_CHAMPION_ICON_WIDTH;
    out_model->icon_height = DM1_CHAMPION_ICON_HEIGHT;
    out_model->icon_fill_color =
        DM1_ChampionColor[DM1_V1_CP_SECOND_LEADER_CHAMPION_INDEX_PC34];
    out_model->action_hand_zone =
        DM1_V1_CP_SECOND_LEADER_ACTION_HAND_ZONE_PC34;
    out_model->action_hand_x = action_hand.x;
    out_model->action_hand_y = action_hand.y;
    out_model->action_hand_width = action_hand.width;
    out_model->action_hand_height = action_hand.height;
    out_model->action_hand_graphic = action_hand.graphicId;
    out_model->status_hand_precedes_inventory =
        transaction.status_hand_route &&
        !transaction.inventory_route;
    out_model->leader_hand_precedes_storage_write =
        transaction.leader_hand_checked_before_storage_write;
    out_model->backpack_precedes_belt =
        transaction.backpack_precedes_belt_in_priority_chain;
    out_model->f0292_final_draw_state =
        transaction.f0292_final_draw_state;
    out_model->contract_only = true;
    out_model->transaction_accepted = transaction.accepted;
    out_model->transaction_status_hand_route = transaction.status_hand_route;
    out_model->transaction_inventory_route = transaction.inventory_route;
    out_model->transaction_leader_is_target = transaction.leader_is_target;
    out_model->transaction_target_champion_index =
        transaction.target_champion_index;
    out_model->transaction_target_champion_ordinal =
        transaction.target_champion_ordinal;
    out_model->transaction_target_slot_index = transaction.target_slot_index;
    out_model->transaction_call_sequence_count =
        transaction.call_sequence_count;
    for (call_index = 0; call_index < transaction.call_sequence_count &&
                         call_index < 8; ++call_index) {
        out_model->transaction_call_sequence[call_index] =
            (int)transaction.call_sequence[call_index];
    }
    out_model->deterministic_hash = hash_model(out_model);
    return true;
}
