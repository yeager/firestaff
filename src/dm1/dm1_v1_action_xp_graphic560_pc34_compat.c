/*
 * dm1_v1_action_xp_graphic560_pc34_compat.c
 *
 * DM1 V1 (PC 3.4 English) action→skill / action→XP routing fixture.
 * Source-locked to ReDMCSB MENU.C G0496 (skill) and G0497 (XP gain)
 * through the shared PC 3.4 EN source-lock accessors.
 *
 * See header for full provenance and citation table.
 */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"
#include "firestaff/dm1/v1/G0496_pc34_compat.h"
#include "firestaff/dm1/v1/G0497_pc34_compat.h"

/* ReDMCSB CHAMPION.C F0304 line ~874: base skill = (sub - 4) >> 2.
 * For base skills (0..3) the mapping is identity. */
static int sub_skill_base_index(int skillIndex) {
    if (skillIndex < 0 || skillIndex >= 20) return 0;
    if (skillIndex < 4) return skillIndex;
    return (skillIndex - 4) >> 2;
}

int dm1_v1_action_xp_route(int actionIndex, DM1_ActionXpRoute* out) {
    int skillIndex;
    int experienceGain;
    if (!out) return 0;
    if (actionIndex < 0 || actionIndex >= DM1_GRAPHIC560_ACTION_COUNT) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    skillIndex = dm1_v1_graphic560_action_skill_index_get_pc34(actionIndex);
    experienceGain = dm1_v1_g0497_get_pc34(actionIndex);
    if (skillIndex < 0 || experienceGain < 0) {
        out->valid = 0;
        out->skillIndex = 0;
        out->baseSkillIndex = 0;
        out->experienceGain = 0;
        return 0;
    }
    out->valid = 1;
    out->skillIndex = skillIndex;
    out->baseSkillIndex = sub_skill_base_index(out->skillIndex);
    out->experienceGain = experienceGain;
    return 1;
}
