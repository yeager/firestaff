#include "csb_v1_teleporter_rotation_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECK_EQ(got, want, msg) do { \
    int got_value = (int)(got); \
    int want_value = (int)(want); \
    if (got_value == want_value) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s got=%d want=%d\n", msg, got_value, want_value); } \
} while (0)

static CSB_V1_TeleporterRotationRuntimeTeleporterPc34 rel_teleporter(int rotation)
{
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter;
    teleporter.target_map_x = 21;
    teleporter.target_map_y = 7;
    teleporter.target_map_index = 3;
    teleporter.rotation = rotation;
    teleporter.absolute_rotation = 0;
    teleporter.audible = 1;
    return teleporter;
}

static CSB_V1_TeleporterRotationRuntimeTeleporterPc34 abs_teleporter(int rotation)
{
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter = rel_teleporter(rotation);
    teleporter.absolute_rotation = 1;
    teleporter.audible = 0;
    return teleporter;
}

static void seed_profile(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_PartyState *party,
                         int x,
                         int y,
                         int map_index,
                         int dir)
{
    int i;

    csb_v1_runtime_init(profile, NULL);
    csb_v1_character_init_default(party);
    party->ChampionCount = CSB_V1_MAX_CHAMPIONS;
    party->ImportedFromDM1 = 1;
    party->PartyDirection = dir & 3;
    party->LeaderIndex = 0;
    party->MagicCasterIndex = -1;
    party->PartyMapX = x;
    party->PartyMapY = y;
    for (i = 0; i < party->ChampionCount; ++i) {
        party->Champions[i].CurrentHealth = (int16_t)(80 + i);
        party->Champions[i].MaximumHealth = (int16_t)(120 + i);
        party->Champions[i].Cell = (uint8_t)i;
        party->Champions[i].Direction = (uint8_t)((dir + i) & 3);
    }
    CHECK_EQ(csb_v1_runtime_set_party_state(profile, party), 0,
             "fixture party enters CSB runtime profile");
    profile->party_x = x;
    profile->party_y = y;
    profile->current_level = map_index;
    profile->party_dir = dir & 3;
    profile->party_state.PartyMapX = x;
    profile->party_state.PartyMapY = y;
    profile->party_state.PartyDirection = dir & 3;
}

static void check_packed4(uint16_t packed,
                          int v0,
                          int v1,
                          int v2,
                          int v3,
                          const char *label)
{
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(packed, 0),
             v0, label);
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(packed, 1),
             v1, label);
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(packed, 2),
             v2, label);
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(packed, 3),
             v3, label);
}

static void test_source_evidence(void)
{
    const char *evidence =
        csb_v1_teleporter_rotation_runtime_source_evidence_pc34_compat();
    CHECK(evidence != NULL, "source evidence string is present");
    CHECK(strstr(evidence, "MOVESENS.C F0262 lines 33-111") != NULL,
          "source evidence cites F0262 group rotation");
    CHECK(strstr(evidence, "MOVESENS.C F0263 lines 113-134") != NULL,
          "source evidence cites F0263 projectile rotation");
    CHECK(strstr(evidence, "MOVESENS.C F0267 lines 493-530") != NULL,
          "source evidence cites F0267 runtime teleporter branch");
    CHECK(strstr(evidence, "DEFS.H lines 398/402") != NULL,
          "source evidence cites thing cell macros");
    CHECK(strstr(evidence, "DEFS.H lines 1367/1377") != NULL,
          "source evidence cites single-centered and attack constants");
    CHECK(strstr(evidence, "DEFS.H lines 2886-2887") != NULL,
          "source evidence cites CM1/CM2 sentinels");
    CHECK(strstr(evidence, "CHAMPION.C F0284 lines 117-130") != NULL,
          "source evidence cites party champion rotation cascade");
}

static void test_party_relative_rotation_runtime(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter =
        rel_teleporter(1);
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 result;
    int i;

    seed_profile(&profile, &party, 4, 5, 1, CSB_V1_DIR_EAST);
    CHECK_EQ(csb_v1_teleporter_rotation_apply_party_pc34_compat(
                 &profile, &teleporter, &result),
             0,
             "relative party teleporter applies through runtime profile");
    CHECK_EQ(result.old_party_x, 4, "party result captures old x");
    CHECK_EQ(result.old_party_y, 5, "party result captures old y");
    CHECK_EQ(result.old_party_map_index, 1, "party result captures old map");
    CHECK_EQ(result.old_party_dir, CSB_V1_DIR_EAST, "party result captures old dir");
    CHECK_EQ(result.new_party_x, 21, "party result captures target x");
    CHECK_EQ(result.new_party_y, 7, "party result captures target y");
    CHECK_EQ(result.new_party_map_index, 3, "party result captures target map");
    CHECK_EQ(result.new_party_dir, CSB_V1_DIR_SOUTH,
             "relative teleporter rotates east + 1 to south");
    CHECK_EQ(result.used_absolute_rotation, 0,
             "relative party teleporter reports relative mode");
    CHECK_EQ(result.audible_buzz_requested, 1,
             "audible party teleporter records buzz request");
    CHECK_EQ(result.party_state_changed, 1, "party teleporter reports mutation");
    CHECK_EQ(profile.party_x, 21, "runtime party x moves to target");
    CHECK_EQ(profile.party_y, 7, "runtime party y moves to target");
    CHECK_EQ(profile.current_level, 3, "runtime map index moves to target");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_SOUTH, "runtime party dir rotates");
    CHECK_EQ(profile.party_state.PartyMapX, 21,
             "imported party snapshot x follows runtime");
    CHECK_EQ(profile.party_state.PartyMapY, 7,
             "imported party snapshot y follows runtime");
    CHECK_EQ(profile.party_state.PartyDirection, CSB_V1_DIR_SOUTH,
             "imported party snapshot direction follows runtime");

    for (i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CHECK_EQ(profile.party_state.Champions[i].Cell, (i + 1) & 3,
                 "relative party teleporter rotates champion Cell through F0284");
        CHECK_EQ(profile.party_state.Champions[i].Direction,
                 (CSB_V1_DIR_EAST + i + 1) & 3,
                 "relative party teleporter rotates champion Direction through F0284");
    }
}

static void test_party_absolute_rotation_runtime(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter =
        abs_teleporter(CSB_V1_DIR_WEST);
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 result;
    int i;

    seed_profile(&profile, &party, 8, 9, 2, CSB_V1_DIR_EAST);
    CHECK_EQ(csb_v1_teleporter_rotation_apply_party_pc34_compat(
                 &profile, &teleporter, &result),
             0,
             "absolute party teleporter applies through runtime profile");
    CHECK_EQ(result.used_absolute_rotation, 1,
             "absolute party teleporter reports absolute mode");
    CHECK_EQ(result.audible_buzz_requested, 0,
             "silent party teleporter does not record buzz request");
    CHECK_EQ(result.new_party_dir, CSB_V1_DIR_WEST,
             "absolute teleporter sets facing to rotation");
    CHECK_EQ(profile.party_dir, CSB_V1_DIR_WEST,
             "runtime party dir is absolute target");
    for (i = 0; i < CSB_V1_MAX_CHAMPIONS; ++i) {
        CHECK_EQ(profile.party_state.Champions[i].Cell, (i + 2) & 3,
                 "absolute party teleporter rotates champion Cell by F0284 delta");
        CHECK_EQ(profile.party_state.Champions[i].Direction,
                 (CSB_V1_DIR_EAST + i + 2) & 3,
                 "absolute party teleporter rotates champion Direction by F0284 delta");
    }
}

static void test_group_relative_and_absolute_rotation(void)
{
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 relative =
        rel_teleporter(1);
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 absolute =
        abs_teleporter(2);
    CSB_V1_TeleporterRotationRuntimeGroupPc34 group;
    CSB_V1_TeleporterRotationRuntimeGroupResultPc34 result;

    memset(&group, 0, sizeof(group));
    group.count = 3;
    group.creature_size = 1;
    group.directions_packed =
        csb_v1_teleporter_rotation_pack_values_pc34_compat(0, 1, 2, 3);
    group.cells_packed =
        csb_v1_teleporter_rotation_pack_values_pc34_compat(0, 1, 2, 3);
    group.behavior = 0;
    group.active_group_index = 9;
    group.source_map_index = 4;
    group.party_map_index = 4;

    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 &relative, &group, &result),
             0,
             "relative group teleporter applies");
    CHECK_EQ(result.used_absolute_rotation, 0,
             "relative group teleporter reports relative mode");
    CHECK_EQ(result.move_group_result, 1,
             "non-attack group result is ordinary teleported result");
    check_packed4(result.directions_packed, 1, 2, 3, 0,
                  "relative group teleporter rotates packed directions");
    check_packed4(result.cells_packed, 1, 2, 3, 0,
                  "relative group teleporter rotates packed cells");

    group.behavior = CSB_V1_TELEPORTER_ROTATION_BEHAVIOR_ATTACK_PC34;
    group.active_group_index = 5;
    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 &absolute, &group, &result),
             0,
             "absolute group teleporter applies");
    CHECK_EQ(result.used_absolute_rotation, 1,
             "absolute group teleporter reports absolute mode");
    CHECK_EQ(result.move_group_result, 7,
             "party-map attacking group returns activeGroupIndex + 2");
    check_packed4(result.directions_packed, 2, 2, 2, 2,
                  "absolute group teleporter sets every direction to rotation");
    check_packed4(result.cells_packed, 2, 2, 2, 2,
                  "absolute group teleporter rotates each cell by direction delta");
}

static void test_group_centered_and_quarter_rotation(void)
{
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 relative =
        rel_teleporter(3);
    CSB_V1_TeleporterRotationRuntimeGroupPc34 group;
    CSB_V1_TeleporterRotationRuntimeGroupResultPc34 result;

    memset(&group, 0, sizeof(group));
    group.count = 0;
    group.creature_size = 1;
    group.directions_packed =
        csb_v1_teleporter_rotation_pack_values_pc34_compat(1, 0, 0, 0);
    group.cells_packed =
        CSB_V1_TELEPORTER_ROTATION_SINGLE_CENTERED_CREATURE_PC34;
    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 &relative, &group, &result),
             0,
             "single-centered group teleporter applies");
    CHECK_EQ(result.cells_packed,
             CSB_V1_TELEPORTER_ROTATION_SINGLE_CENTERED_CREATURE_PC34,
             "single-centered group keeps C0xFF cells sentinel");
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
                 result.directions_packed, 0),
             0,
             "single-centered group still rotates direction");

    group.count = 1;
    group.creature_size =
        CSB_V1_TELEPORTER_ROTATION_SIZE_QUARTER_SQUARE_PC34;
    group.directions_packed =
        csb_v1_teleporter_rotation_pack_values_pc34_compat(1, 1, 0, 0);
    group.cells_packed =
        csb_v1_teleporter_rotation_pack_values_pc34_compat(0, 2, 0, 0);
    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 &relative, &group, &result),
             0,
             "quarter-square group teleporter applies");
    check_packed4(result.directions_packed, 0, 0, 0, 0,
                  "quarter-square relative teleporter rotates directions");
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
                 result.cells_packed, 0),
             3,
             "quarter-square relative teleporter rotates first cell by teleporter rotation");
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(
                 result.cells_packed, 1),
             1,
             "quarter-square relative teleporter rotates second cell by teleporter rotation");
}

static void test_projectile_and_object_rotation(void)
{
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 relative =
        rel_teleporter(1);
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 absolute =
        abs_teleporter(1);
    CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 projectile_result;
    CSB_V1_TeleporterRotationRuntimeObjectResultPc34 object_result;
    uint16_t thing_cell2 =
        csb_v1_teleporter_rotation_thing_with_cell_pc34_compat(0x0123u, 2);
    uint16_t thing_cell1 =
        csb_v1_teleporter_rotation_thing_with_cell_pc34_compat(0x0321u, 1);

    CHECK_EQ(csb_v1_teleporter_rotation_thing_cell_pc34_compat(thing_cell2),
             2,
             "thing cell helper decodes M011_CELL bits");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
                 &relative, thing_cell2, CSB_V1_DIR_WEST, &projectile_result),
             0,
             "relative projectile teleporter applies");
    CHECK_EQ(projectile_result.direction, CSB_V1_DIR_NORTH,
             "relative projectile direction rotates through G0400");
    CHECK_EQ(csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                 projectile_result.thing),
             3,
             "relative projectile teleporter rotates thing cell");
    CHECK_EQ(projectile_result.used_absolute_rotation, 0,
             "relative projectile teleporter reports relative mode");

    CHECK_EQ(csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
                 &absolute, thing_cell2, CSB_V1_DIR_WEST, &projectile_result),
             0,
             "absolute projectile teleporter applies");
    CHECK_EQ(projectile_result.direction, CSB_V1_DIR_EAST,
             "absolute projectile direction is teleporter rotation");
    CHECK_EQ(csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                 projectile_result.thing),
             2,
             "absolute projectile teleporter leaves thing cell unchanged");
    CHECK_EQ(projectile_result.used_absolute_rotation, 1,
             "absolute projectile teleporter reports absolute mode");

    CHECK_EQ(csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                 &relative, thing_cell1, 4, &object_result),
             0,
             "relative object teleporter applies");
    CHECK_EQ(object_result.cell_rotated, 1,
             "relative object teleporter marks cell rotation");
    CHECK_EQ(csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                 object_result.thing),
             2,
             "relative object teleporter rotates thing cell");

    CHECK_EQ(csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                 &absolute, thing_cell1, 4, &object_result),
             0,
             "absolute object teleporter applies");
    CHECK_EQ(object_result.cell_rotated, 0,
             "absolute object teleporter leaves object cell unrotated");
    CHECK_EQ(csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                 object_result.thing),
             1,
             "absolute object teleporter preserves thing cell");

    CHECK_EQ(csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                 &relative,
                 thing_cell1,
                 CSB_V1_TELEPORTER_ROTATION_SOURCE_PROJECTILE_ASSOCIATED_OBJECT_PC34,
                 &object_result),
             0,
             "projectile-associated object teleporter applies");
    CHECK_EQ(object_result.associated_projectile_object_exempt, 1,
             "projectile-associated object reports CM2 exemption");
    CHECK_EQ(object_result.cell_rotated, 0,
             "projectile-associated object is not cell-rotated");
    CHECK_EQ(csb_v1_teleporter_rotation_thing_cell_pc34_compat(
                 object_result.thing),
             1,
             "projectile-associated object preserves thing cell");
}

static void test_invalid_inputs(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_PartyState party;
    CSB_V1_TeleporterRotationRuntimeTeleporterPc34 teleporter =
        rel_teleporter(1);
    CSB_V1_TeleporterRotationRuntimePartyResultPc34 party_result;
    CSB_V1_TeleporterRotationRuntimeGroupPc34 group;
    CSB_V1_TeleporterRotationRuntimeGroupResultPc34 group_result;
    CSB_V1_TeleporterRotationRuntimeProjectileResultPc34 projectile_result;
    CSB_V1_TeleporterRotationRuntimeObjectResultPc34 object_result;

    seed_profile(&profile, &party, 1, 1, 0, CSB_V1_DIR_NORTH);
    memset(&group, 0, sizeof(group));
    CHECK_EQ(csb_v1_teleporter_rotation_apply_party_pc34_compat(
                 NULL, &teleporter, &party_result),
             -1,
             "NULL party profile is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_party_pc34_compat(
                 &profile, NULL, &party_result),
             -1,
             "NULL party teleporter is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 NULL, &group, &group_result),
             -1,
             "NULL group teleporter is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 &teleporter, NULL, &group_result),
             -1,
             "NULL group is rejected");
    group.count = CSB_V1_MAX_CHAMPIONS;
    CHECK_EQ(csb_v1_teleporter_rotation_apply_group_pc34_compat(
                 &teleporter, &group, &group_result),
             -1,
             "oversized group count is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
                 NULL, 0, 0, &projectile_result),
             -1,
             "NULL projectile teleporter is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_projectile_pc34_compat(
                 &teleporter, 0, 0, NULL),
             -1,
             "NULL projectile result is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                 NULL, 0, 0, &object_result),
             -1,
             "NULL object teleporter is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_apply_object_cell_pc34_compat(
                 &teleporter, 0, 0, NULL),
             -1,
             "NULL object result is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(0, -1),
             -1,
             "negative packed index is rejected");
    CHECK_EQ(csb_v1_teleporter_rotation_get_packed_value_pc34_compat(0, 4),
             -1,
             "packed index past four creatures is rejected");
}

int main(void)
{
    printf("=== CSB V1 Teleporter Rotation Runtime Gate ===\n\n");
    test_source_evidence();
    test_party_relative_rotation_runtime();
    test_party_absolute_rotation_runtime();
    test_group_relative_and_absolute_rotation();
    test_group_centered_and_quarter_rotation();
    test_projectile_and_object_rotation();
    test_invalid_inputs();
    CHECK(passed + failed >= 90,
          "regression keeps at least 90 assertions active");
    printf("\nPASSED: %d\nFAILED: %d\nASSERTIONS: %d\n",
           passed, failed, passed + failed);
    if (failed == 0) {
        puts("all PASS: CSB V1 teleporter absolute/relative rotation covers party runtime, group cells/directions, projectile direction/cell, and object-cell CM2 exemption");
        puts("sourceEvidence=ReDMCSB MOVESENS.C F0262 lines 33-111; F0263 lines 113-134; F0267 lines 493-530; DEFS.H M011/M015/CM2; CHAMPION.C F0284 lines 117-130");
    }
    return failed == 0 ? 0 : 1;
}
