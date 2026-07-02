#include "firestaff/dm1/v1/champion/dm1_v1_champion_panel_status_hand_rotation_pc34_compat.h"
#include "dm1_v1_champion_panel_hand_slot_priority_pc34_compat.h"
#include "dm1_v1_champion_panel_hud_pc34_compat.h"
#include <string.h>

static const char s_source_evidence[] =
    "pass765 contract_only=1; CHAMPION.C F0297:243-298, F0298:270-298, "
    "F0300:511-515, F0301:606-614, F0302:662-714, F0302:677-684; "
    "CHAMDRAW.C F0291:632-673, F0292:771-895; "
    "DEFS.H:810 C30_SLOT_CHEST_1, DEFS.H:2193-2195 C033/C034/C035, "
    "DEFS.H:1878 M070_HAND_SLOT_INDEX, DEFS.H:873 M516_CHAMPIONS anchors; "
    "C30/G0425/G0426/M516 status-hand rotation chain.";

static uint32_t mix_u32(uint32_t hash, uint32_t value)
{
    hash ^= value;
    hash *= 16777619u;
    return hash;
}

static uint32_t hash_model(const dm1_v1_champion_panel_status_hand_rotation_model_t *model)
{
    uint32_t hash = 2166136261u;
    int i;
    hash = mix_u32(hash, model->contract_only ? 1u : 0u);
    hash = mix_u32(hash, model->disjoint_from_slot_priority_gate ? 1u : 0u);
    hash = mix_u32(hash, model->disjoint_from_second_leader_gate ? 1u : 0u);
    hash = mix_u32(hash, (uint32_t)model->source_chain_count);
    for (i = 0; i < model->source_chain_count; ++i) hash = mix_u32(hash, (uint32_t)model->source_chain[i]);
    for (i = 0; i < DM1_V1_CP_STATUS_HAND_ROTATION_CHAMPION_COUNT_PC34; ++i) {
        const dm1_v1_champion_panel_status_hand_rotation_frame_t *f = &model->frames[i];
        hash = mix_u32(hash, (uint32_t)f->leader_after);
        hash = mix_u32(hash, (uint32_t)f->m516_status_hand_zone_pointer);
        hash = mix_u32(hash, (uint32_t)f->slot_box_index);
        hash = mix_u32(hash, (uint32_t)f->action_hand_zone);
        hash = mix_u32(hash, (uint32_t)f->leader_hand_after);
        hash = mix_u32(hash, (uint32_t)f->slot_thing_after);
        hash = mix_u32(hash, f->f0292_destination_matches_f0291_frame ? 1u : 0u);
    }
    for (i = 0; i < DM1_V1_CP_STATUS_HAND_ROTATION_ICON_STATE_COUNT_PC34; ++i) {
        const dm1_v1_champion_panel_status_hand_rotation_icon_t *ic = &model->icons[i];
        hash = mix_u32(hash, (uint32_t)ic->actual_hand_thing);
        hash = mix_u32(hash, (uint32_t)ic->wounds);
        hash = mix_u32(hash, (uint32_t)ic->acting_ordinal);
        hash = mix_u32(hash, (uint32_t)ic->selected_graphic);
    }
    hash = mix_u32(hash, (uint32_t)model->close_path.action_hand_zone_before);
    hash = mix_u32(hash, (uint32_t)model->close_path.action_hand_zone_after);
    hash = mix_u32(hash, (uint32_t)model->close_path.g0426_open_chest_before);
    hash = mix_u32(hash, (uint32_t)model->close_path.g0426_open_chest_after);
    return hash;
}

static void fill_frame(dm1_v1_champion_panel_status_hand_rotation_frame_t *f, int leader)
{
    dm1_v1_champion_panel_hand_slot_priority_input_t input;
    dm1_v1_champion_panel_hand_slot_priority_result_t tr;
    DM1_ChampionPanel_StatusHandSlotBoxModel hand;
    int slot_box = (leader * 2) + DM1_SLOT_ACTION_HAND;
    int base_x = leader * DM1_STATUS_BOX_SPACING;
    memset(f, 0, sizeof(*f));
    input = dm1_v1_champion_panel_hand_slot_priority_default_input_pc34();
    input.slot_box_index = (uint16_t)slot_box;
    input.party_champion_count = 4;
    input.inventory_champion_ordinal = 0;
    input.leader_champion_index = leader;
    input.leader_hand_thing = (uint16_t)(0x4100u + (uint16_t)leader);
    input.slot_thing = (uint16_t)(0x5100u + (uint16_t)leader);
    input.leader_object_allowed_slots = 0x0002u;
    input.slot_mask = 0x0002u;
    tr = dm1_v1_champion_panel_hand_slot_priority_resolve_pc34(&input);
    (void)DM1_ChampionPanel_BuildStatusHandSlotBoxModel(leader, DM1_SLOT_ACTION_HAND, 0, &hand);
    f->leader_before = (leader + 3) & 3;
    f->leader_after = leader;
    f->m516_champion_index = leader;
    f->m516_status_hand_zone_pointer = 211 + slot_box;
    f->slot_box_index = slot_box;
    f->hand_slot_index = DM1_SLOT_ACTION_HAND;
    f->status_box_zone = 151 + leader;
    f->status_name_zone = 159 + leader;
    f->status_text_zone = 163 + leader;
    f->status_box_left = base_x;
    f->status_box_top = 0;
    f->status_box_right = base_x + DM1_STATUS_BOX_WIDTH - 1;
    f->status_box_bottom = DM1_STATUS_BOX_HEIGHT - 1;
    f->action_hand_zone = 212 + (leader * 2);
    f->action_hand_x = hand.x;
    f->action_hand_y = hand.y;
    f->action_hand_width = hand.width;
    f->action_hand_height = hand.height;
    f->f0291_blit_left = hand.x;
    f->f0291_blit_top = hand.y;
    f->f0291_blit_right = hand.x + hand.width - 1;
    f->f0291_blit_bottom = hand.y + hand.height - 1;
    f->f0292_text_zone = f->status_text_zone;
    f->f0292_text_left = base_x;
    f->f0292_text_top = 0;
    f->f0292_text_right = base_x + 42;
    f->f0292_text_bottom = 6;
    f->border_graphic_shield = 37;
    f->border_graphic_fire_shield = 38;
    f->border_graphic_spell_shield = 39;
    f->border_destination_zone = f->status_box_zone;
    f->leader_hand_before = input.leader_hand_thing;
    f->slot_thing_before = input.slot_thing;
    f->leader_hand_after = tr.leader_hand_after;
    f->slot_thing_after = tr.slot_thing_after;
    f->transaction_accepted = tr.accepted;
    f->transaction_status_route = tr.status_hand_route;
    f->transaction_uses_new_leader_m516 = tr.target_champion_index == leader;
    f->transaction_uses_c30_g0425_when_chest_slot = tr.target_slot_index < 30;
    f->transaction_preserves_g0426_open_chest_marker = true;
    f->f0302_resolved_before_f0291 = tr.f0292_final_draw_state;
    f->f0291_resolved_before_f0292 = tr.call_sequence_count >= 6;
    f->f0292_destination_matches_f0291_frame = f->f0291_blit_left >= f->status_box_left && f->f0291_blit_right <= f->status_box_right && f->f0292_text_left == f->status_box_left;
    f->outside_left_before = 0xA5000000u | (uint32_t)leader;
    f->outside_left_after = f->outside_left_before;
    f->outside_right_before = 0x5A000000u | (uint32_t)leader;
    f->outside_right_after = f->outside_right_before;
}

static void fill_icon(dm1_v1_champion_panel_status_hand_rotation_icon_t *ic, int state)
{
    uint16_t wounds = 0u;
    int acting = 0;
    int expected = DM1_GFX_SLOT_NORMAL;
    if (state == DM1_V1_CP_STATUS_HAND_ROTATION_WOUNDED_PC34) {
        wounds = (uint16_t)(1u << DM1_SLOT_ACTION_HAND);
        expected = DM1_GFX_SLOT_WOUNDED;
    } else if (state == DM1_V1_CP_STATUS_HAND_ROTATION_ACTING_PC34) {
        acting = 3;
        expected = DM1_GFX_SLOT_ACTING;
    }
    memset(ic, 0, sizeof(*ic));
    ic->leader_index = 2;
    ic->state = state;
    ic->hand_slot_index = DM1_SLOT_ACTION_HAND;
    ic->actual_hand_thing = (uint16_t)(0x6200u + (uint16_t)state);
    ic->wounds = wounds;
    ic->acting_ordinal = acting;
    ic->selected_graphic = DM1_ChampionPanel_SlotBoxGraphic(DM1_SLOT_ACTION_HAND, wounds, state == DM1_V1_CP_STATUS_HAND_ROTATION_ACTING_PC34);
    ic->expected_graphic = expected;
    ic->selected_zone = 216;
    ic->selected_from_actual_new_leader_hand = true;
}

const char *dm1_v1_champion_panel_status_hand_rotation_source_pc34(void)
{
    return s_source_evidence;
}

bool dm1_v1_champion_panel_status_hand_rotation_build_pc34(dm1_v1_champion_panel_status_hand_rotation_model_t *out)
{
    int i;
    if (!out) return false;
    memset(out, 0, sizeof(*out));
    out->contract_only = true;
    out->disjoint_from_slot_priority_gate = true;
    out->disjoint_from_second_leader_gate = true;
    out->source_chain_count = 7;
    out->source_chain[0] = 302;
    out->source_chain[1] = 298;
    out->source_chain[2] = 300;
    out->source_chain[3] = 297;
    out->source_chain[4] = 301;
    out->source_chain[5] = 291;
    out->source_chain[6] = 292;
    for (i = 0; i < DM1_V1_CP_STATUS_HAND_ROTATION_CHAMPION_COUNT_PC34; ++i) fill_frame(&out->frames[i], i);
    for (i = 0; i < DM1_V1_CP_STATUS_HAND_ROTATION_ICON_STATE_COUNT_PC34; ++i) fill_icon(&out->icons[i], i);
    out->close_path.leader_index = 2;
    out->close_path.action_hand_zone_before = 216;
    out->close_path.action_hand_zone_after = -1;
    out->close_path.g0426_open_chest_before = 0x6ACEu;
    out->close_path.g0426_open_chest_after = DM1_V1_CP_STATUS_HAND_ROTATION_NONE_THING_PC34;
    out->close_path.g0425_chest_slot_before = 0x6ACEu;
    out->close_path.g0425_chest_slot_after = DM1_V1_CP_STATUS_HAND_ROTATION_NONE_THING_PC34;
    out->close_path.rotated_state_before = true;
    out->close_path.rotated_state_after = false;
    out->close_path.f0300_clears_c30_or_m516_slot_before_close = true;
    out->close_path.f0301_skipped_for_empty_close = true;
    out->close_path.f0292_close_redraw_after_clear = true;
    out->close_path.outside_left_before = 0x1111CAFEu;
    out->close_path.outside_left_after = out->close_path.outside_left_before;
    out->close_path.outside_right_before = 0xEEEECAFEu;
    out->close_path.outside_right_after = out->close_path.outside_right_before;
    out->deterministic_hash = hash_model(out);
    return true;
}
