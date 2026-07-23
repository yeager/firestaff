#include "dm1_v1_g0251_g0300_dungeon_state_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "DUNGEON.C/BASE.C global", owner, 1, 1, 1 }

static const DM1_V1_G0251G0300SourceAuditPc34 k_audit[] = {
    ROW(251, "fail_closed: no verified possession owner"), ROW(252, "fail_closed: no verified possession owner"),
    ROW(253, "fail_closed: no verified possession owner"), ROW(254, "fail_closed: no verified door-info owner"),
    ROW(255, "fail_closed: no verified escape-string owner"), ROW(256, "fail_closed: no verified escape-character owner"),
    ROW(257, "fail_closed: no verified inscription-string owner"), ROW(258, "fail_closed: no verified group-direction owner"),
    ROW(259, "fail_closed: no verified Graphic559 anchor owner"), ROW(260, "fail_closed: no verified dungeon-text owner"),
    ROW(261, "fail_closed: no verified map-ornament owner"), ROW(262, "fail_closed: no verified map-ornament owner"),
    ROW(263, "fail_closed: no verified map-ornament owner"), ROW(264, "fail_closed: no verified map-creature owner"),
    ROW(265, "fail_closed: no verified inscription-ornament owner"), ROW(266, "fail_closed: no verified altar-ornament owner"),
    ROW(267, "dm1_v1_wall_ornament_alcove_f0149_pc34_compat"), ROW(268, "dm1_v1_fountain_interaction_pc34_compat"),
    ROW(269, "fail_closed: no verified current-map owner"), ROW(270, "dm1_v1_thing_list_helpers_f0156_f0159_f0160_f0161_pc34_compat"),
    ROW(271, "dm1_v1_current_map_f0173_f0174_pc34_compat"), ROW(272, "fail_closed: no verified map-index owner"),
    ROW(273, "fail_closed: no verified map-width owner"), ROW(274, "fail_closed: no verified map-height owner"),
    ROW(275, "fail_closed: no verified current-door owner"), ROW(276, "fail_closed: no verified raw-map owner"),
    ROW(277, "dm1_v1_original_save_pc34_handoff"), ROW(278, "fail_closed: no verified dungeon-header owner"),
    ROW(279, "fail_closed: no verified dungeon-map-data owner"), ROW(280, "fail_closed: no verified column-count owner"),
    ROW(281, "fail_closed: no verified map-index-table owner"), ROW(282, "fail_closed: no verified dungeon-column owner"),
    ROW(283, "dm1_v1_thing_list_helpers_f0156_f0159_f0160_f0161_pc34_compat"), ROW(284, "dm1_v1_dungeon_thing_data_pc34_compat"),
    ROW(285, "fail_closed: no verified square-ahead owner"), ROW(286, "fail_closed: no verified alcove-facing owner"),
    ROW(287, "dm1_v1_resurrection_pc34_compat"), ROW(288, "dm1_v1_fountain_interaction_pc34_compat"),
    ROW(289, "fail_closed: no verified mirror-ordinal owner"), ROW(290, "fail_closed: no verified inscription-thing owner"),
    ROW(291, "fail_closed: no verified clickable-box owner"), ROW(292, "dm1_v1_viewport_click_pc34_compat"),
    ROW(293, "fail_closed: source fuzzy CPSE state"), ROW(294, "fail_closed: no verified discarded-thing owner"),
    ROW(295, "fail_closed: source fuzzy CPSE buffer"), ROW(296, "dm1_v1_f0128_viewport_pc34_compat"),
    ROW(297, "dm1_v1_viewport_f0098_floor_ceiling_fallback_pc34_compat"), ROW(298, "dm1_v1_game_state_pc34_compat"),
    ROW(299, "dm1_v1_mirror_candidate_c040_inventory_toggle_while_panel_live_pc34_compat"), ROW(300, "dm1_v1_game_state_pc34_compat")
};

#undef ROW

const DM1_V1_G0251G0300SourceAuditPc34 *
dm1_v1_g0251_g0300_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0251G0300SourceAuditPc34 *
dm1_v1_g0251_g0300_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0251_g0300_source_audit_evidence_pc34(void)
{
    return "ReDMCSB DUNGEON.C:24-1033, DEFS.H:5633-5657, GSTHINGS.C:33, and "
           "BASE.C:7-42 are the authority for G0251-G0300. Only existing source-named "
           "PC34 owners are bound; unverified state remains fail closed. The audit does "
           "not render or synthesize behavior.";
}
