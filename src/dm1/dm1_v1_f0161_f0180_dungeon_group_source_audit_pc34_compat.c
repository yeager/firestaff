#include "dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34_compat.h"

#include <stddef.h>

static const char s_source_evidence[] =
    "ReDMCSB DUNGEON.C F0161:1730-1750 through F0174:2742-2756; "
    "GROUP.C F0175:52-65 through F0180:311-340. Existing DM1 owners "
    "consume loaded original PC34 material and remain the only runtime paths.";

static const DM1_V1_F0161F0180SourceAuditPc34Compat s_audit[] = {
    {161u, DM1_V1_F0161_F0180_OWNER_SQUARE_THING, 1, 1,
     "DUNGEON.C F0161:1730-1750", "memory_dungeon_dat_pc34_compat:F0512"},
    {162u, DM1_V1_F0161_F0180_OWNER_SQUARE_THING, 1, 1,
     "DUNGEON.C F0162:1752-1766", "memory_dungeon_dat_pc34_compat:F0513"},
    {163u, DM1_V1_F0161_F0180_OWNER_THING_LIFECYCLE, 1, 1,
     "DUNGEON.C F0163:1769-1838", "memory_dungeon_dat_pc34_compat:F0514"},
    {164u, DM1_V1_F0161_F0180_OWNER_THING_LIFECYCLE, 1, 1,
     "DUNGEON.C F0164:1840-1905", "memory_dungeon_dat_pc34_compat:F0515"},
    {165u, DM1_V1_F0161_F0180_OWNER_THING_LIFECYCLE, 1, 1,
     "DUNGEON.C F0165:1923-2075", "dm1_csb_f0165_dungeon_discarded_thing_pc34_compat"},
    {166u, DM1_V1_F0161_F0180_OWNER_THING_LIFECYCLE, 1, 1,
     "DUNGEON.C F0166:2077-2137", "memory_dungeon_dat_pc34_compat:F0516"},
    {167u, DM1_V1_F0161_F0180_OWNER_THING_LIFECYCLE, 1, 1,
     "DUNGEON.C F0167:2140-2200", "memory_dungeon_dat_pc34_compat:F0517"},
    {168u, DM1_V1_F0161_F0180_OWNER_WALL_TEXT_ORNAMENT, 1, 1,
     "DUNGEON.C F0168:2206-2368", "dm1_v1_inscription_host_material_pc34_compat"},
    {169u, DM1_V1_F0161_F0180_OWNER_WALL_TEXT_ORNAMENT, 1, 1,
     "DUNGEON.C F0169:2371-2379", "dm1_v1_random_ornament_pc34_compat"},
    {170u, DM1_V1_F0161_F0180_OWNER_WALL_TEXT_ORNAMENT, 1, 1,
     "DUNGEON.C F0170:2382-2404", "dm1_v1_random_ornament_pc34_compat"},
    {171u, DM1_V1_F0161_F0180_OWNER_WALL_TEXT_ORNAMENT, 1, 1,
     "DUNGEON.C F0171:2407-2462", "dm1_v1_random_ornament_pc34_compat"},
    {172u, DM1_V1_F0161_F0180_OWNER_WALL_TEXT_ORNAMENT, 1, 1,
     "DUNGEON.C F0172:2466-2722", "dm1_v1_champion_mirror_pc34_compat"},
    {173u, DM1_V1_F0161_F0180_OWNER_MAP_CONTEXT, 1, 1,
     "DUNGEON.C F0173:2724-2740", "dm1_v1_dungeon_data_pc34_compat"},
    {174u, DM1_V1_F0161_F0180_OWNER_MAP_CONTEXT, 1, 1,
     "DUNGEON.C F0174:2742-2756", "dm1_v1_dungeon_data_pc34_compat"},
    {175u, DM1_V1_F0161_F0180_OWNER_GROUP_TARGETING, 1, 1,
     "GROUP.C F0175:52-65", "dm1_v1_dungeon_thing_data_pc34_compat"},
    {176u, DM1_V1_F0161_F0180_OWNER_GROUP_TARGETING, 1, 1,
     "GROUP.C F0176:69-107", "dm1_v1_dungeon_thing_data_pc34_compat"},
    {177u, DM1_V1_F0161_F0180_OWNER_GROUP_TARGETING, 1, 1,
     "GROUP.C F0177:109-158", "dm1_v1_dungeon_thing_data_pc34_compat"},
    {178u, DM1_V1_F0161_F0180_OWNER_GROUP_LIVE_STATE, 1, 1,
     "GROUP.C F0178:174-185", "dm1_v1_teleporter_pit_pc34_compat"},
    {179u, DM1_V1_F0161_F0180_OWNER_GROUP_LIVE_STATE, 1, 1,
     "GROUP.C F0179:187-308", "dm1_v1_group_active_state_pc34_compat"},
    {180u, DM1_V1_F0161_F0180_OWNER_GROUP_LIVE_STATE, 1, 1,
     "GROUP.C F0180:311-340", "dm1_v1_group_active_state_pc34_compat"}
};

const DM1_V1_F0161F0180SourceAuditPc34Compat *
dm1_v1_f0161_f0180_dungeon_group_source_audit_pc34(uint16_t function_number)
{
    size_t index;

    for (index = 0u; index < sizeof(s_audit) / sizeof(s_audit[0]); ++index) {
        if (s_audit[index].functionNumber == function_number) return &s_audit[index];
    }
    return NULL;
}

const char *dm1_v1_f0161_f0180_dungeon_group_source_evidence_pc34(void)
{
    return s_source_evidence;
}
