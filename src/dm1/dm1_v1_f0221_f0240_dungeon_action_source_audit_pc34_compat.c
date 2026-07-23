#include "dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34_compat.h"

#include <stddef.h>

static const char s_source_evidence[] =
    "ReDMCSB PROJEXPL.C F0221:881-903 through F0232:1554-1595; "
    "GROUP.C F0223:162-169; TIMELINE.C F0233:50-124 through F0240:682-689. "
    "Each entry retains its existing DM1 original-PC34 owner.";

static const DM1_V1_F0221F0240SourceAuditPc34Compat s_audit[] = {
    {221u, DM1_V1_F0221_F0240_OWNER_ENDGAME, 1, 1,
     "PROJEXPL.C F0221:881-903", "dm1_v1_c15_layout_pc34_compat"},
    {222u, DM1_V1_F0221_F0240_OWNER_ENDGAME, 1, 1,
     "PROJEXPL.C F0222:905-939", "dm1_v1_endgame_system_pc34_compat"},
    {223u, DM1_V1_F0221_F0240_OWNER_ENDGAME, 1, 1,
     "GROUP.C F0223:162-169; PROJEXPL.C:941-959", "dm1_v1_endgame_system_pc34_compat"},
    {224u, DM1_V1_F0221_F0240_OWNER_ENDGAME, 1, 1,
     "PROJEXPL.C F0224:961-1034", "dm1_v1_endgame_system_pc34_compat"},
    {225u, DM1_V1_F0221_F0240_OWNER_ENDGAME, 1, 1,
     "PROJEXPL.C F0225:1037-1129", "dm1_v1_endgame_system_pc34_compat"},
    {226u, DM1_V1_F0221_F0240_OWNER_GROUP_AI, 1, 1,
     "PROJEXPL.C F0226:1131-1142", "dm1_v1_creature_ai_behavior_pc34_compat"},
    {227u, DM1_V1_F0221_F0240_OWNER_GROUP_AI, 1, 1,
     "PROJEXPL.C F0227:1144-1212", "dm1_v1_creature_ai_behavior_pc34_compat"},
    {228u, DM1_V1_F0221_F0240_OWNER_GROUP_AI, 1, 1,
     "PROJEXPL.C F0228:1214-1282", "dm1_v1_creature_ai_behavior_pc34_compat"},
    {229u, DM1_V1_F0221_F0240_OWNER_GROUP_AI, 1, 1,
     "PROJEXPL.C F0229:1284-1303", "dm1_v1_creature_ai_behavior_pc34_compat"},
    {230u, DM1_V1_F0221_F0240_OWNER_COMBAT, 1, 1,
     "PROJEXPL.C F0230:1305-1414", "dm1_v1_combat_pc34_compat"},
    {231u, DM1_V1_F0221_F0240_OWNER_COMBAT, 1, 1,
     "PROJEXPL.C F0231:1416-1552", "dm1_v1_melee_action_f0402_pc34_compat"},
    {232u, DM1_V1_F0221_F0240_OWNER_COMBAT, 1, 1,
     "PROJEXPL.C F0232:1554-1595", "dm1_v1_action_xp_graphic560_pc34_compat"},
    {233u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0233:50-124", "dm1_v1_event_timer_pc34_compat"},
    {234u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0234:126-238", "dm1_v1_event_timer_pc34_compat"},
    {235u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0235:240-332", "dm1_v1_event_timer_pc34_compat"},
    {236u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0236:334-391", "dm1_v1_event_timer_pc34_compat"},
    {237u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0237:393-485", "dm1_v1_event_timer_pc34_compat"},
    {238u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0238:487-662", "dm1_v1_event_timer_pc34_compat"},
    {239u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0239:664-680", "dm1_v1_event_timer_pc34_compat"},
    {240u, DM1_V1_F0221_F0240_OWNER_TIMELINE, 1, 1,
     "TIMELINE.C F0240:682-689", "dm1_v1_event_timer_pc34_compat"}
};

const DM1_V1_F0221F0240SourceAuditPc34Compat *
dm1_v1_f0221_f0240_dungeon_action_source_audit_pc34(uint16_t function_number)
{
    size_t index;

    for (index = 0u; index < sizeof(s_audit) / sizeof(s_audit[0]); ++index) {
        if (s_audit[index].functionNumber == function_number) return &s_audit[index];
    }
    return NULL;
}

const char *dm1_v1_f0221_f0240_dungeon_action_source_evidence_pc34(void)
{
    return s_source_evidence;
}
