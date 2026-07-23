#include "dm1_v1_f0181_f0200_group_source_audit_pc34_compat.h"

static const DM1_V1_F0181F0200GroupSourceAuditPc34 k_group_audit[] = {
    { 181u, "GROUP.C:340-371 F0181_GROUP_DeleteEvents", "dm1_v1_group_timeline_f0179_f0181_pc34_compat", 1, 1 },
    { 182u, "GROUP.C:374-388 F0182_GROUP_StopAttacking", "dm1_v1_group_active_lifecycle_pc34_compat", 1, 1 },
    { 183u, "GROUP.C:389-449 F0183_GROUP_AddActiveGroup", "dm1_v1_group_active_state_pc34_compat", 1, 1 },
    { 184u, "GROUP.C:450-480 F0184_GROUP_RemoveActiveGroup", "dm1_v1_group_active_state_pc34_compat", 1, 1 },
    { 185u, "GROUP.C:481-549 F0185_GROUP_GetGenerated", "memory_runtime_dynamics_pc34_compat", 1, 1 },
    { 186u, "GROUP.C:550-647 F0186_GROUP_DropCreatureFixedPossessions", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1 },
    { 187u, "GROUP.C:648-675 F0187_GROUP_DropMovingCreatureFixedPossessions", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1 },
    { 188u, "GROUP.C:676-738 F0188_GROUP_DropGroupPossessions", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1 },
    { 189u, "GROUP.C:739-768 F0189_GROUP_Delete", "dm1_v1_melee_action_f0402_pc34_compat", 1, 1 },
    { 190u, "GROUP.C:769-931 F0190_GROUP_GetDamageCreatureOutcome", "dm1_v1_combat_pc34_compat", 1, 1 },
    { 191u, "GROUP.C:932-990 F0191_GROUP_GetDamageAllCreaturesOutcome", "dm1_v1_combat_pc34_compat", 1, 1 },
    { 192u, "GROUP.C:991-1012 F0192_GROUP_GetResistanceAdjustedPoisonAttack", "memory_combat_pc34_compat", 1, 1 },
    { 193u, "GROUP.C:1013-1081 F0193_GROUP_StealFromChampion", "dm1_v1_creature_ai_behavior_pc34_compat", 1, 1 },
    { 194u, "GROUP.C:1082-1097 F0194_GROUP_RemoveAllActiveGroups", "dm1_v1_creature_ai_behavior_pc34_compat", 1, 1 },
    { 195u, "GROUP.C:1098-1134 F0195_GROUP_AddAllActiveGroups", "dm1_v1_group_active_lifecycle_pc34_compat", 1, 1 },
    { 196u, "GROUP.C:1135-1175 F0196_GROUP_InitializeActiveGroups", "dm1_v1_group_active_lifecycle_pc34_compat", 1, 1 },
    { 197u, "GROUP.C:1176-1213 F0197_GROUP_IsViewPartyBlocked", "dm1_v1_creature_ai_behavior_pc34_compat", 1, 1 },
    { 198u, "GROUP.C:1214-1238 F0198_GROUP_IsSmellPartyBlocked", "dm1_v1_creature_ai_behavior_pc34_compat", 1, 1 },
    { 199u, "GROUP.C:1239-1314 F0199_GROUP_GetDistanceBetweenUnblockedSquares", "dm1_v1_creature_ai_behavior_pc34_compat", 1, 1 },
    { 200u, "GROUP.C:1315-1416 F0200_GROUP_GetDistanceToVisibleParty", "dm1_v1_creature_ai_behavior_pc34_compat", 1, 1 }
};

const DM1_V1_F0181F0200GroupSourceAuditPc34 *
dm1_v1_f0181_f0200_group_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_group_audit) / sizeof(k_group_audit[0]);
    return k_group_audit;
}

const DM1_V1_F0181F0200GroupSourceAuditPc34 *
dm1_v1_f0181_f0200_group_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_group_audit) / sizeof(k_group_audit[0]); ++index) {
        if (k_group_audit[index].symbol_number == symbol_number) return &k_group_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0181_f0200_group_source_audit_evidence_pc34(void)
{
    return "ReDMCSB GROUP.C:340-1416 is the authority for F0181-F0200. "
           "Every entry is bound to an existing DM1 owner and requires raw PC34 "
           "group, timeline, map, or creature data; unavailable input must fail closed.";
}
