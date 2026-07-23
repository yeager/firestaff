#include "dm1_v1_g0501_g0550_graphic560_save_source_audit_pc34_compat.h"

#define ROW(number, owner) { number##u, "MENU.C/DECOMPDU.C/DEFS.H Graphic560/save global", owner, 1, 1, 1 }

static const DM1_V1_G0501G0550SourceAuditPc34 k_audit[] = {
    ROW(501, "dm1_v1_graphic560_box_action_area_1_action_menu_pc34_compat"), ROW(502, "dm1_v1_graphic560_box_action_area_medium_damage_pc34_compat"),
    ROW(503, "dm1_v1_graphic560_box_action_area_small_damage_pc34_compat"), ROW(504, "dm1_v1_graphic560_box_spell_area_controls_pc34_compat"),
    ROW(505, "fail_closed: no verified Graphic560 anchor owner"), ROW(506, "fail_closed: no verified acting-champion owner"),
    ROW(507, "fail_closed: no verified action-count owner"), ROW(508, "fail_closed: no verified action-refresh owner"),
    ROW(509, "fail_closed: no verified action-icon owner"), ROW(510, "fail_closed: no source symbol"),
    ROW(511, "fail_closed: no source symbol"), ROW(512, "fail_closed: no source symbol"),
    ROW(513, "fail_closed: no verified action-damage owner"), ROW(514, "fail_closed: no verified caster-index owner"),
    ROW(515, "fail_closed: no verified spell-line bitmap owner"), ROW(516, "fail_closed: no verified spell-lines bitmap owner"),
    ROW(517, "fail_closed: no verified action-target Thing owner"), ROW(518, "fail_closed: no verified system-return owner"),
    ROW(519, "fail_closed: no verified game-data pointer owner"), ROW(520, "fail_closed: no verified game-backup pointer owner"),
    ROW(521, "fail_closed: no verified game-file-handle owner"), ROW(522, "fail_closed: unreferenced source state"),
    ROW(523, "fail_closed: no verified restart-request owner"), ROW(524, "fail_closed: no verified restart-allowed owner"),
    ROW(525, "dm1_v1_dungeon_decompressor_pc34_compat;dm1_v1_save_load_system_pc34_compat"),
    ROW(526, "dm1_v1_dungeon_decompressor_pc34_compat;dm1_v1_save_load_system_pc34_compat"),
    ROW(527, "dm1_v1_dungeon_decompressor_pc34_compat;dm1_v1_save_load_system_pc34_compat"),
    ROW(528, "dm1_v1_dungeon_decompressor_pc34_compat;dm1_v1_save_load_system_pc34_compat"),
    ROW(529, "fail_closed: unreferenced source state"), ROW(530, "fail_closed: no verified decompression-mode owner"),
    ROW(531, "fail_closed: no verified decompression-pointer owner"), ROW(532, "fail_closed: no verified decompression-count owner"),
    ROW(533, "fail_closed: CPSE code-patch boundary"), ROW(534, "dm1_v1_save_load_system_pc34_compat"),
    ROW(535, "fail_closed: no verified bit-mask-table owner"), ROW(536, "fail_closed: no verified save-message owner"),
    ROW(537, "fail_closed: no verified save-message owner"), ROW(538, "fail_closed: no verified save-message owner"),
    ROW(539, "fail_closed: no verified save-message owner"), ROW(540, "fail_closed: platform disk-message boundary"),
    ROW(541, "fail_closed: no verified save-message owner"), ROW(542, "fail_closed: platform disk-message boundary"),
    ROW(543, "fail_closed: no verified save-message owner"), ROW(544, "fail_closed: no verified save-message owner"),
    ROW(545, "fail_closed: no verified save-message owner"), ROW(546, "fail_closed: platform disk-message boundary"),
    ROW(547, "fail_closed: no verified save-message owner"), ROW(548, "fail_closed: platform disk-message boundary"),
    ROW(549, "fail_closed: platform disk-message boundary"), ROW(550, "fail_closed: no verified save-message owner")
};

#undef ROW

const DM1_V1_G0501G0550SourceAuditPc34 *
dm1_v1_g0501_g0550_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_G0501G0550SourceAuditPc34 *
dm1_v1_g0501_g0550_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_g0501_g0550_source_audit_evidence_pc34(void)
{
    return "ReDMCSB MENU.C:37-41/504-527, DECOMPDU.C:12-25, READWRIT.C:10/349, "
           "and DEFS.H:6044-6099 are the authority for G0501-G0550. Only existing "
           "source-locked PC 3.4 EN Graphic560 tables and raw save-header fields are "
           "bound; UI state, decompression state, and disk text fail closed. The audit "
           "does not render or synthesize behavior.";
}
