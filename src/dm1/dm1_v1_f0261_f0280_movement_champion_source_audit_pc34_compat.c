#include "dm1_v1_f0261_f0280_movement_champion_source_audit_pc34_compat.h"

static const DM1_V1_F0261F0280SourceAuditPc34 k_audit[] = {
    { 261u, "TIMELINE.C:1833 F0261_TIMELINE_Process_CPSEF", "dm1_v1_event_timer_pc34_compat", 1, 1, 1 },
    { 262u, "MOVESENS.C:33 F0262_MOVE_GetTeleporterRotatedGroupResult", "memory_movement_pc34_compat", 1, 1, 1 },
    { 263u, "MOVESENS.C:113 F0263_MOVE_GetTeleporterRotatedProjectileThing", "memory_movement_pc34_compat", 1, 1, 1 },
    { 264u, "MOVESENS.C:136 F0264_MOVE_IsLevitating", "dm1_v1_collision_door_pc34_compat", 1, 1, 1 },
    { 265u, "MOVESENS.C:169 F0265_MOVE_CreateEvent60To61_MoveGroup", "memory_movement_pc34_compat", 1, 1, 1 },
    { 266u, "MOVESENS.C:195 F0266_MOVE_IsKilledByProjectileImpact", "memory_movement_pc34_compat", 1, 1, 1 },
    { 267u, "MOVESENS.C:316 F0267_MOVE_GetMoveResult_CPSCE", "dm1_v1_movement_pipeline_pc34_compat", 1, 1, 1 },
    { 268u, "MOVESENS.C:1000 F0268_SENSOR_AddEvent", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 269u, "MOVESENS.C:1038 F0269_SENSOR_AddSkillExperience", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 270u, "MOVESENS.C:1081 F0270_SENSOR_TriggerLocalEffect", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 271u, "MOVESENS.C:1100 F0271_SENSOR_ProcessRotationEffect", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 272u, "MOVESENS.C:1154 F0272_SENSOR_TriggerEffect", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 273u, "MOVESENS.C:1210 F0273_SENSOR_GetObjectOfTypeInCell", "dm1_v1_sensor_get_object_of_type_in_cell_pc34_compat", 1, 1, 1 },
    { 274u, "MOVESENS.C:1234 F0274_SENSOR_IsObjectInPartyPossession", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 275u, "MOVESENS.C:1309 F0275_SENSOR_IsTriggeredByClickOnWall", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 276u, "MOVESENS.C:1553 F0276_SENSOR_ProcessThingAdditionOrRemoval", "dm1_v1_sensor_trigger_pc34_compat", 1, 1, 1 },
    { 277u, "DEFS.H:7838 F0277_CPSE_IsFuzzySectorValid_FuzzyBits", "dm1_v1_fuzzy_sector_analyzed_pc34_compat", 1, 1, 1 },
    { 278u, "REVIVE.C champion-start decode F0278", "dm1_v1_resurrection_pc34_compat", 1, 1, 1 },
    { 279u, "REVIVE.C:9 F0279_CHAMPION_GetDecodedValue", "dm1_v1_resurrection_pc34_compat", 1, 1, 1 },
    { 280u, "REVIVE.C:63 F0280_CHAMPION_AddCandidateChampionToParty", "dm1_v1_entrance_champion_select_pc34_compat", 1, 1, 1 }
};

const DM1_V1_F0261F0280SourceAuditPc34 *
dm1_v1_f0261_f0280_source_audit_pc34(size_t *out_count)
{
    if (out_count) *out_count = sizeof(k_audit) / sizeof(k_audit[0]);
    return k_audit;
}

const DM1_V1_F0261F0280SourceAuditPc34 *
dm1_v1_f0261_f0280_source_audit_find_pc34(unsigned int symbol_number)
{
    size_t index;
    for (index = 0u; index < sizeof(k_audit) / sizeof(k_audit[0]); ++index) {
        if (k_audit[index].symbol_number == symbol_number) return &k_audit[index];
    }
    return 0;
}

const char *dm1_v1_f0261_f0280_source_audit_evidence_pc34(void)
{
    return "ReDMCSB TIMELINE.C, MOVESENS.C, REVIVE.C, and DEFS.H are the "
           "authority for F0261-F0280. This audit records only existing owners; "
           "they require raw original data and fail closed without it. The audit "
           "does not mutate party state or synthesize UI.";
}
