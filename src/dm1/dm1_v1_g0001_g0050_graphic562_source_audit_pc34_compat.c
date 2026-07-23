#include "dm1_v1_g0001_g0050_graphic562_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "DATA.C:7-88 Graphic562 global", owner, 1, 1, 1 }

static const DM1_V1_G0001G0050SourceAuditPc34 k_audit[] = {
    ROW(1, "dm1_v1_box_action_area_pc34_compat"),
    ROW(2, "dm1_v1_box_movement_arrows_pc34_compat"),
    ROW(3, "dm1_v1_box_title_strikes_back_destination_pc34_compat"),
    ROW(4, "dm1_v1_box_title_strikes_back_source_pc34_compat"),
    ROW(5, "dm1_v1_box_title_presents_pc34_compat"),
    ROW(6, "dm1_v1_box_entrance_dungeon_view_pc34_compat"),
    ROW(7, "dm1_v1_box_entrance_opening_door_left_pc34_compat"),
    ROW(8, "dm1_v1_box_entrance_opening_door_right_pc34_compat"),
    ROW(9, "dm1_v1_box_entrance_doors_pc34_compat"),
    ROW(10, "dm1_v1_entrance_closed_door_left_pc34_compat"),
    ROW(11, "dm1_v1_entrance_closed_door_right_pc34_compat"),
    ROW(12, "dm1_v1_endgame_the_end_pc34_compat"),
    ROW(13, "dm1_v1_endgame_restart_outer_pc34_compat"),
    ROW(14, "dm1_v1_endgame_restart_inner_pc34_compat"),
    ROW(15, "dm1_v1_endgame_champion_mirror_pc34_compat"),
    ROW(16, "dm1_v1_endgame_champion_portrait_pc34_compat"),
    ROW(17, "dm1_v1_palette_changes_no_changes_pc34_compat"),
    ROW(18, "dm1_v1_mandatory_graphic_indices_pc34_compat"),
    ROW(19, "dm1_v1_palette_credits_pc34_compat"),
    ROW(20, "dm1_v1_palette_entrance_pc34_compat"),
    ROW(21, "dm1_v1_palette_dungeon_view_pc34_compat"),
    ROW(22, "dm1_v1_indirect_stop_expiring_event_pc34_compat"),
    ROW(23, "dm1_v1_ordered_cells_to_attack_pc34_compat"),
    ROW(24, "dm1_v1_wound_probability_index_to_mask_pc34_compat"),
    ROW(25, "dm1_v1_steal_from_slot_indices_pc34_compat"),
    ROW(26, "dm1_v1_icon_graphic_first_icon_index_pc34_compat"),
    ROW(27, "fail_closed: source-unreferenced box"),
    ROW(28, "dm1_v1_champion_panel_box_leader_hand_object_name_pc34_compat"),
    ROW(29, "dm1_v1_charge_count_to_torch_type_pc34_compat"),
    ROW(30, "dm1_v1_slot_boxes_pc34_compat"),
    ROW(31, "dm1_v1_fuzzy_sector_analyzed_pc34_compat"),
    ROW(32, "dm1_v1_champion_panel_box_panel_pc34_compat"),
    ROW(33, "dm1_v1_champion_panel_box_arrow_or_eye_pc34_compat"),
    ROW(34, "dm1_v1_champion_panel_box_object_description_circle_pc34_compat"),
    ROW(35, "dm1_v1_champion_panel_box_food_pc34_compat"),
    ROW(36, "dm1_v1_champion_panel_box_water_pc34_compat"),
    ROW(37, "dm1_v1_champion_panel_box_poisoned_pc34_compat"),
    ROW(38, "dm1_v1_slot_masks_pc34_compat"),
    ROW(39, "dm1_v1_light_power_to_light_amount_pc34_compat"),
    ROW(40, "dm1_v1_palette_index_to_light_amount_pc34_compat"),
    ROW(41, "fail_closed: dm1_v1_viewport_floppy_zzz_cross_pc34_compat platform boundary"),
    ROW(42, "dm1_v1_bitmap_arrow_pointer_pc34_compat"),
    ROW(43, "dm1_v1_bitmap_hand_pointer_pc34_compat"),
    ROW(44, "dm1_v1_palette_changes_mouse_pointer_icon_pc34_compat"),
    ROW(45, "dm1_v1_palette_changes_mouse_pointer_icon_shadow_pc34_compat"),
    ROW(46, "dm1_v1_champion_color_pc34_compat"),
    ROW(47, "dm1_v1_champion_portrait_box_champion_portrait_pc34_compat"),
    ROW(48, "dm1_v1_champion_portrait_box_mouth_pc34_compat"),
    ROW(49, "dm1_v1_champion_portrait_box_eye_pc34_compat"),
    ROW(50, "dm1_v1_wound_defense_factor_pc34_compat")
};

#undef ROW

const DM1_V1_G0001G0050SourceAuditPc34 *
dm1_v1_g0001_g0050_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0001G0050SourceAuditPc34 *
dm1_v1_g0001_g0050_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0001_g0050_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DATA.C:7-88 defines Graphic562 globals G0001-G0050. "
           "This inventory binds only existing source-named DM1 owners; G0027 and "
           "G0041 remain fail closed. All routes require authentic raw PC34 material. "
           "The audit does not render or synthesize behavior.";
}
