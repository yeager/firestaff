#include "dm1_v1_f0301_f0320_core_action_source_audit_pc34_compat.h"

#include <stddef.h>

static const char s_source_evidence[] =
    "ReDMCSB CHAMPION.C F0301:587-660 through F0320:1689-1800. "
    "All entries retain existing DM1 owners over original PC34 material.";

static const DM1_V1_F0301F0320SourceAuditPc34Compat s_audit[] = {
    {301u, DM1_V1_F0301_F0320_OWNER_SLOT_SKILL, 1, 1, "CHAMPION.C F0301:587-660", "dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat"},
    {302u, DM1_V1_F0301_F0320_OWNER_SLOT_SKILL, 1, 1, "CHAMPION.C F0302:662-711", "dm1_v1_champion_panel_action_hand_slot_priority_pc34_compat"},
    {303u, DM1_V1_F0301_F0320_OWNER_SLOT_SKILL, 1, 1, "CHAMPION.C F0303:715-820", "dm1_v1_skill_experience_pc34_compat"},
    {304u, DM1_V1_F0301_F0320_OWNER_SLOT_SKILL, 1, 1, "CHAMPION.C F0304:823-977", "dm1_v1_skill_experience_pc34_compat"},
    {305u, DM1_V1_F0301_F0320_OWNER_SLOT_SKILL, 1, 1, "CHAMPION.C F0305:1061-1074", "dm1_v1_throw_shoot_pc34_compat"},
    {306u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0306:1078-1104", "dm1_v1_champion_stats_pc34_compat"},
    {307u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0307:1106-1119", "dm1_v1_combat_pc34_compat"},
    {308u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0308:1123-1155", "dm1_v1_combat_pc34_compat"},
    {309u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0309:1157-1177", "dm1_v1_champion_stats_pc34_compat"},
    {310u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0310:1180-1215", "dm1_v1_champion_stats_pc34_compat"},
    {311u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0311:1218-1234", "dm1_v1_combat_pc34_compat"},
    {312u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0312:1237-1303", "dm1_v1_combat_pc34_compat"},
    {313u, DM1_V1_F0301_F0320_OWNER_STATS_COMBAT, 1, 1, "CHAMPION.C F0313:1305-1380", "dm1_v1_wound_defense_factor_pc34_compat"},
    {314u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0314:1382-1414", "dm1_v1_mirror_candidate_c146_sleep_wakeup_repaint_gate_pc34_compat"},
    {315u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0315:1418-1449", "dm1_v1_champion_needs_pc34_compat"},
    {316u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0316:1451-1466", "dm1_v1_champion_needs_pc34_compat"},
    {317u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0317:1472-1499", "dm1_v1_champion_needs_pc34_compat"},
    {318u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0318:1527-1551", "dm1_v1_chest_auto_close_on_leader_death_pc34_compat"},
    {319u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0319:1552-1668", "dm1_v1_champion_panel_pending_damage_apply_pc34_compat"},
    {320u, DM1_V1_F0301_F0320_OWNER_CHAMPION_LIFECYCLE, 1, 1, "CHAMPION.C F0320:1689-1800", "dm1_v1_champion_panel_pending_damage_apply_pc34_compat"}
};

const DM1_V1_F0301F0320SourceAuditPc34Compat *
dm1_v1_f0301_f0320_core_action_source_audit_pc34(uint16_t function_number)
{
    size_t index;

    for (index = 0u; index < sizeof(s_audit) / sizeof(s_audit[0]); ++index) {
        if (s_audit[index].functionNumber == function_number) return &s_audit[index];
    }
    return NULL;
}

const char *dm1_v1_f0301_f0320_core_action_source_evidence_pc34(void)
{
    return s_source_evidence;
}
