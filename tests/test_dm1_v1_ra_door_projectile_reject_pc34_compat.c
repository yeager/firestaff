#include "firestaff/dm1/v1/projectile/ra_door_projectile_reject_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_assertions;
static int g_failures;

static void expect_int(const char *label, int got, int want,
                       const char *anchor)
{
    ++g_assertions;
    if (got != want) {
        ++g_failures;
        printf("FAIL %s got=%d want=%d anchor=%s\n",
               label, got, want, anchor);
    } else {
        printf("PASS %s == %d (%s)\n", label, want, anchor);
    }
}

static void expect_contains(const char *label, const char *haystack,
                            const char *needle, const char *anchor)
{
    ++g_assertions;
    if (!haystack || !needle || strstr(haystack, needle) == 0) {
        ++g_failures;
        printf("FAIL %s missing=\"%s\" anchor=%s\n",
               label, needle ? needle : "(null)", anchor);
    } else {
        printf("PASS %s contains \"%s\" (%s)\n", label, needle, anchor);
    }
}

static void test_contract_and_source_evidence(void)
{
    const DM1_V1_RaDoorProjectileRejectContractPc34 *contract =
        dm1_v1_ra_door_projectile_reject_contract_pc34();
    const char *evidence =
        dm1_v1_ra_door_projectile_reject_source_evidence_pc34();

    expect_int("contract.contract_only", contract->contract_only, 1,
               "contract-only parity gate");
    expect_int("contract.no_real_assets", contract->no_real_assets, 1,
               "no GRAPHICS.DAT/DUNGEON.DAT load");
    expect_int("contract.f0217.f0232",
               contract->f0217_calls_f0232_for_non_open_door_projectiles, 1,
               "PROJEXPL.C:F0217:514-525");
    expect_int("contract.open_door.skip",
               contract->f0217_open_door_spell_skips_f0232, 1,
               "PROJEXPL.C:F0217:485-489");
    expect_int("contract.magic_false", contract->f0217_passes_magic_attack_false,
               1, "PROJEXPL.C:F0217 calls F0232(..., C0_FALSE, 0)");
    expect_int("contract.defense_predicate",
               contract->f0232_requires_attack_at_least_defense, 1,
               "PROJEXPL.C:F0232:1583 Attack>=Defense");
    expect_int("contract.closed_gate", contract->f0232_requires_closed_door_state,
               1, "PROJEXPL.C:F0232:1585 C4_DOOR_STATE_CLOSED");
    expect_int("contract.ra_type", contract->ra_door_type, 3,
               "DUNGEON.C:565 / 801 RA door slot");
    expect_int("contract.ra_defense", contract->ra_defense, 255,
               "DUNGEON.C:565 / 801 RA defense");
    expect_int("contract.melee_cap", contract->melee_cap, 100,
               "DUNGEON.C:561 / 797 melee cap comment");
    expect_int("contract.ra_reject", contract->ra_rejected_at_melee_cap, 1,
               "100 < RA defense 255");
    expect_int("contract.wood_break", contract->wooden_destroyed_at_melee_cap, 1,
               "100 >= wooden defense 42");
    expect_int("contract.door_info_count", contract->door_info_count, 4,
               "DUNGEON.C:G0254 has four door types");

    expect_contains("contract.dungeon", contract->dungeon_anchor,
                    "DUNGEON.C:560-565", "door-info first table");
    expect_contains("contract.f0217", contract->projexpl_f0217_anchor,
                    "F0217:471-525", "projectile door branch");
    expect_contains("contract.f0232", contract->projexpl_f0232_anchor,
                    "F0232:1569-1592", "door destruction branch");
    expect_contains("contract.non_overlap", contract->non_overlap,
                    "wall-impact", "non-overlap marker");

    expect_contains("evidence.dungeon", evidence, "DUNGEON.C:560-565",
                    "source evidence");
    expect_contains("evidence.dungeon2", evidence, "796-801",
                    "duplicated door-info table");
    expect_contains("evidence.ra", evidence, "Defense=255",
                    "RA door source evidence");
    expect_contains("evidence.f0217", evidence, "PROJEXPL.C:F0217:471-525",
                    "F0217 source evidence");
    expect_contains("evidence.f0232", evidence, "PROJEXPL.C:F0232:1569-1592",
                    "F0232 source evidence");
    expect_contains("evidence.c0false", evidence, "C0_FALSE",
                    "projectile passes non-magic flag");
    expect_contains("evidence.nonoverlap", evidence, "not pass745/pass563",
                    "non-overlap source evidence");
}

static void test_door_info_table(void)
{
    const DM1_V1_RaDoorProjectileDoorInfoPc34 *door;

    expect_int("door.count",
               (int)dm1_v1_ra_door_projectile_reject_door_info_count_pc34(),
               4, "DUNGEON.C:G0254 four records");

    door = dm1_v1_ra_door_projectile_reject_door_info_pc34(0);
    expect_int("port.type", door ? door->door_type : -1, 0,
               "DUNGEON.C:562 / 798");
    expect_int("port.defense", door ? door->defense : -1, 110,
               "DUNGEON.C:562 / 798");
    expect_int("port.projectiles", door ? door->projectiles_can_pass_through : -1,
               1, "MASK0x0002_PROJECTILES_CAN_PASS_THROUGH");
    expect_int("port.creature_sight", door ? door->creatures_can_see_through : -1,
               1, "MASK0x0001_CREATURES_CAN_SEE_THROUGH");

    door = dm1_v1_ra_door_projectile_reject_door_info_pc34(1);
    expect_int("wood.type", door ? door->door_type : -1, 1,
               "DUNGEON.C:563 / 799");
    expect_int("wood.defense", door ? door->defense : -1, 42,
               "DUNGEON.C:563 / 799");
    expect_int("wood.attrs", door ? door->attributes : -1, 0,
               "wooden door has no G0254 attributes");

    door = dm1_v1_ra_door_projectile_reject_door_info_pc34(2);
    expect_int("iron.type", door ? door->door_type : -1, 2,
               "DUNGEON.C:564 / 800");
    expect_int("iron.defense", door ? door->defense : -1, 230,
               "DUNGEON.C:564 / 800");

    door = dm1_v1_ra_door_projectile_reject_door_info_pc34(3);
    expect_int("ra.type", door ? door->door_type : -1, 3,
               "DUNGEON.C:565 / 801");
    expect_int("ra.defense", door ? door->defense : -1, 255,
               "DUNGEON.C:565 / 801");
    expect_int("ra.projectiles", door ? door->projectiles_can_pass_through : -1,
               0, "RA lacks MASK0x0002_PROJECTILES_CAN_PASS_THROUGH");
    expect_int("ra.creature_sight", door ? door->creatures_can_see_through : -1,
               1, "RA has MASK0x0001_CREATURES_CAN_SEE_THROUGH");
    expect_int("ra.animated", door ? door->animated : -1, 1,
               "RA has MASK0x0004_ANIMATED");
    expect_contains("ra.anchor", door ? door->source_anchor : 0,
                    "RA defense 255", "DUNGEON.C:565 / 801");
    expect_int("door.oob",
               dm1_v1_ra_door_projectile_reject_door_info_pc34(4) == 0, 1,
               "door-info bounds");
}

static void expect_sim(const char *prefix, int door_type, int attack,
                       int open_door, int closed, int want_reaches_f0232,
                       int want_destroyed, int want_reject,
                       const char *anchor)
{
    DM1_V1_RaDoorProjectileRejectResultPc34 result;
    char label[96];

    memset(&result, 0, sizeof(result));
    snprintf(label, sizeof(label), "%s.rc", prefix);
    expect_int(label,
               dm1_v1_ra_door_projectile_reject_simulate_pc34(
                   door_type, attack, open_door, closed, &result),
               1, anchor);
    snprintf(label, sizeof(label), "%s.type", prefix);
    expect_int(label, result.door_type, door_type, anchor);
    snprintf(label, sizeof(label), "%s.reaches_f0232", prefix);
    expect_int(label, result.f0217_reaches_f0232, want_reaches_f0232, anchor);
    snprintf(label, sizeof(label), "%s.magic_false", prefix);
    expect_int(label, result.magic_attack_flag, 0, "F0217 passes C0_FALSE");
    snprintf(label, sizeof(label), "%s.destroyed", prefix);
    expect_int(label, result.f0232_destroyed, want_destroyed, anchor);
    snprintf(label, sizeof(label), "%s.rejected", prefix);
    expect_int(label, result.source_locked_reject, want_reject, anchor);
    snprintf(label, sizeof(label), "%s.consumed", prefix);
    expect_int(label, result.projectile_consumed, 1, "F0217 returns after impact");
    snprintf(label, sizeof(label), "%s.anchor", prefix);
    expect_contains(label, result.source_anchor, "F0232:1569-1592", anchor);
}

static void test_projectile_rejection_matrix(void)
{
    DM1_V1_RaDoorProjectileRejectResultPc34 result;

    expect_sim("wood.100", 1, 100, 0, 1, 1, 1, 0,
               "PROJEXPL.C:F0232 Attack>=42 destroys closed wooden door");
    expect_sim("wood.41", 1, 41, 0, 1, 1, 0, 1,
               "PROJEXPL.C:F0232 Attack<42 rejects wooden door");
    expect_sim("ra.100", 3, 100, 0, 1, 1, 0, 1,
               "DUNGEON.C:565 / 801 RA defense 255 rejects melee-cap attack");
    expect_sim("iron.100", 2, 100, 0, 1, 1, 0, 1,
               "DUNGEON.C:564 / 800 iron defense 230 rejects melee-cap attack");
    expect_sim("port.100", 0, 100, 0, 1, 1, 0, 1,
               "DUNGEON.C:562 / 798 portcullis defense 110 rejects melee-cap attack");
    expect_sim("ra.open_door_spell", 3, 100, 1, 1, 0, 0, 1,
               "PROJEXPL.C:F0217 Open Door branch skips F0232");
    expect_sim("wood.not_closed", 1, 100, 0, 0, 1, 0, 1,
               "PROJEXPL.C:F0232 destroys only C4 closed doors");

    memset(&result, 0, sizeof(result));
    expect_int("invalid.type.rc",
               dm1_v1_ra_door_projectile_reject_simulate_pc34(
                   99, 100, 0, 1, &result),
               0, "invalid door type rejected by contract helper");
    expect_int("null.out.rc",
               dm1_v1_ra_door_projectile_reject_simulate_pc34(
                   3, 100, 0, 1, 0),
               0, "null output rejected by contract helper");

    memset(&result, 0, sizeof(result));
    expect_int("ra.attack100.rc",
               dm1_v1_ra_door_projectile_reject_simulate_pc34(
                   3, 100, 0, 1, &result),
               1, "melee-cap attack envelope");
    expect_int("ra.attack100.attack", result.bounded_attack, 100,
               "DUNGEON.C:561 / 797 melee cap comment");
    expect_int("ra.attack100.defense", result.door_defense, 255,
               "DUNGEON.C:565 / 801 RA defense");
}

int main(void)
{
    test_contract_and_source_evidence();
    test_door_info_table();
    test_projectile_rejection_matrix();

    if (g_failures) {
        printf("FAILURES=%d ASSERTIONS=%d\n", g_failures, g_assertions);
        return 1;
    }
    printf("OK dm1_v1_ra_door_projectile_reject_pc34_compat assertions=%d\n",
           g_assertions);
    return 0;
}
