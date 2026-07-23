#include "dm1_v1_g0451_g0500_graphic560_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "COMMAND.C/MENU.C/ACTIDRAW.C Graphic560 global", owner, 1, 1, 1 }

static const DM1_V1_G0451G0500SourceAuditPc34 k_audit[] = {
    ROW(451, "fail_closed: no verified frozen-game input-table owner"), ROW(452, "fail_closed: no verified action-area-name input owner"),
    ROW(453, "fail_closed: no verified action-icon input-table owner"), ROW(454, "fail_closed: no verified spell-area input-table owner"),
    ROW(455, "fail_closed: no verified champion-hand input-table owner"), ROW(456, "fail_closed: no verified chest input-table owner"),
    ROW(457, "fail_closed: no verified resurrection input-table owner"), ROW(458, "fail_closed: no verified primary-keyboard table owner"),
    ROW(459, "fail_closed: no verified secondary-keyboard table owner"), ROW(460, "fail_closed: no verified party-rest keyboard owner"),
    ROW(461, "fail_closed: no verified frozen-game keyboard owner"), ROW(462, "fail_closed: no verified object-pile box owner"),
    ROW(463, "fail_closed: no verified movement-arrow box owner"), ROW(464, "fail_closed: no verified spell-symbol box owner"),
    ROW(465, "fail_closed: no verified forward-arrow counter owner"), ROW(466, "fail_closed: no verified right-arrow counter owner"),
    ROW(467, "fail_closed: no verified dialog-patch owner"), ROW(468, "fail_closed: no verified dialog-patch owner"),
    ROW(469, "fail_closed: no verified dialog-patch owner"), ROW(470, "fail_closed: no verified dialog-patch owner"),
    ROW(471, "fail_closed: no verified viewport-dialog input owner"), ROW(472, "fail_closed: no verified viewport-dialog input owner"),
    ROW(473, "fail_closed: no verified viewport-dialog input owner"), ROW(474, "fail_closed: no verified viewport-dialog input owner"),
    ROW(475, "fail_closed: no verified screen-dialog input owner"), ROW(476, "fail_closed: no verified screen-dialog input owner"),
    ROW(477, "fail_closed: no verified screen-dialog input owner"), ROW(478, "fail_closed: no verified screen-dialog input owner"),
    ROW(479, "fail_closed: no verified Graphic561 anchor owner"), ROW(480, "fail_closed: no verified dialog-input-set owner"),
    ROW(481, "fail_closed: no verified keyboard-backup owner"), ROW(482, "fail_closed: no verified keyboard-backup owner"),
    ROW(483, "fail_closed: no verified mouse-backup owner"), ROW(484, "fail_closed: no verified mouse-backup owner"),
    ROW(485, "dm1_v1_graphic560_symbol_base_mana_cost_pc34_compat"), ROW(486, "dm1_v1_graphic560_symbol_mana_cost_multiplier_pc34_compat"),
    ROW(487, "dm1_v1_g0487_pc34_compat"), ROW(488, "fail_closed: CPSE fuzzy-bit boundary"),
    ROW(489, "dm1_v1_g0489_pc34_compat"), ROW(490, "dm1_v1_g0490_pc34_compat"),
    ROW(491, "dm1_v1_graphic560_action_disabled_ticks_pc34_compat"), ROW(492, "dm1_v1_graphic560_action_damage_factor_pc34_compat"),
    ROW(493, "dm1_v1_graphic560_action_hit_probability_pc34_compat"), ROW(494, "dm1_v1_graphic560_action_stamina_pc34_compat"),
    ROW(495, "dm1_v1_graphic560_action_defense_pc34_compat"), ROW(496, "dm1_v1_graphic560_action_skill_index_pc34_compat"),
    ROW(497, "dm1_v1_g0497_pc34_compat"), ROW(498, "fail_closed: no verified action-icon-palette owner"),
    ROW(499, "dm1_v1_graphic560_box_action_area_3_actions_menu_pc34_compat"), ROW(500, "dm1_v1_graphic560_box_action_area_2_actions_menu_pc34_compat")
};

#undef ROW

const DM1_V1_G0451G0500SourceAuditPc34 *
dm1_v1_g0451_g0500_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0451G0500SourceAuditPc34 *
dm1_v1_g0451_g0500_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0451_g0500_source_audit_evidence_pc34(void)
{
    return "ReDMCSB COMMAND.C:38-51/1841-1853, CLIKMENU.C:220-221, MENU.C:16-38, "
           "ACTIDRAW.C:3, and DEFS.H:5995-5999 are the authority for G0451-G0500. "
           "Only existing source-locked PC 3.4 EN Graphic560 owners are bound; "
           "Graphic561, input, palette, and CPSE globals fail closed. The audit does "
           "not render or synthesize behavior.";
}
