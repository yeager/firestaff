#include "dm1_v1_melee_action_f0402_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_tick_plan_null(void)
{
    DM1_MeleeActionTickPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_action_tick_plan_f0402_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);

    DM1_MeleeActionTickInputPc34 in;
    memset(&in, 0, sizeof(in));
    rc = dm1_v1_melee_action_tick_plan_f0402_pc34(&in, NULL);
    assert(rc == 0);
}

static void test_tick_plan_no_champion(void)
{
    DM1_MeleeActionTickInputPc34 in;
    DM1_MeleeActionTickPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.championIndex = 0;
    in.actionIndex = 2;
    in.championPresent = 0;
    rc = dm1_v1_melee_action_tick_plan_f0402_pc34(&in, &out);
    (void)rc;
    assert(rc == 0 || rc == 1);
}

static void test_damage_emission_null(void)
{
    DM1_MeleeDamageEmissionPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_damage_emission_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_damage_emission_zero(void)
{
    DM1_MeleeDamageEmissionInputPc34 in;
    DM1_MeleeDamageEmissionPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.damage = 0;
    in.combatOutcome = 0;
    rc = dm1_v1_melee_damage_emission_plan_f0231_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
}

static void test_runtime_outcome_null(void)
{
    DM1_MeleeRuntimeOutcomePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_runtime_outcome_plan_f0407_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_kill_notify_null(void)
{
    DM1_MeleeKillNotifyPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_kill_notify_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_reach_gate_null(void)
{
    DM1_MeleeReachGatePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_reach_gate_plan_f0402_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_disrupt_material_gate_null(void)
{
    DM1_MeleeDisruptMaterialGatePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_disrupt_material_gate_plan_f0402_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_weapon_profile_null(void)
{
    DM1_MeleeWeaponProfilePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_weapon_profile_plan_f0402_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_side_effect_null(void)
{
    DM1_MeleeF0231SideEffectPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_side_effect_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_weapon_availability_null(void)
{
    DM1_MeleeF0402WeaponAvailabilityPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_weapon_availability_plan_f0402_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_command_decode_null(void)
{
    DM1_MeleeF0402CommandDecodePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_command_decode_plan_f0402_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_target_creature_null(void)
{
    DM1_MeleeF0177TargetCreaturePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_target_creature_plan_f0177_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_preflight_null(void)
{
    DM1_MeleeF0402PreflightPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_preflight_plan_f0402_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_strength_null(void)
{
    DM1_MeleeF0312StrengthPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_strength_plan_f0312_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_strength_valid(void)
{
    DM1_MeleeF0312StrengthInputPc34 in;
    DM1_MeleeF0312StrengthPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.championStrength = 50;
    in.currentStamina = 100;
    in.maximumStamina = 200;
    in.maximumLoad = 400;
    in.random16 = 5;
    rc = dm1_v1_melee_strength_plan_f0312_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.oneSixteenthMaximumLoad == 25);
}

static void test_champion_snapshot_null(void)
{
    DM1_MeleeF0231ChampionSnapshotPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_champion_snapshot_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_creature_snapshot_null(void)
{
    DM1_MeleeF0231CreatureSnapshotPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_creature_snapshot_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_damage_gate_null(void)
{
    DM1_MeleeF0231DamageGatePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_damage_gate_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_runtime_result_null(void)
{
    DM1_MeleeF0231RuntimeResultPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_runtime_result_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_luck_writeback_null(void)
{
    DM1_MeleeF0231LuckWritebackPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_luck_writeback_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_aftermath_null(void)
{
    DM1_MeleeF0231AftermathPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_aftermath_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_reaction_null(void)
{
    DM1_MeleeF0231ReactionPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_reaction_plan_f0231_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_death_smoke_null(void)
{
    DM1_MeleeF0190DeathSmokePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_death_smoke_plan_f0190_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_death_smoke_attack(void)
{
    int atk = dm1_v1_melee_death_smoke_attack_f0190_pc34(0);
    (void)atk;
    assert(atk >= 0);
}

static void test_possession_drop_null(void)
{
    DM1_MeleeF0190PossessionDropPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_possession_drop_plan_f0190_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_killed_some_state_null(void)
{
    DM1_MeleeF0190KilledSomeStatePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_killed_some_state_plan_f0190_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_killed_all_state_null(void)
{
    DM1_MeleeF0190KilledAllStatePlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_killed_all_state_plan_f0190_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_timeline_cleanup_null(void)
{
    DM1_MeleeF0190TimelineCleanupPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_timeline_cleanup_plan_f0190_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_mutation_dispatch_null(void)
{
    DM1_MeleeF0190MutationDispatchPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_melee_mutation_dispatch_plan_f0190_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

int main(void)
{
    test_tick_plan_null();
    test_tick_plan_no_champion();
    test_damage_emission_null();
    test_damage_emission_zero();
    test_runtime_outcome_null();
    test_kill_notify_null();
    test_reach_gate_null();
    test_disrupt_material_gate_null();
    test_weapon_profile_null();
    test_side_effect_null();
    test_weapon_availability_null();
    test_command_decode_null();
    test_target_creature_null();
    test_preflight_null();
    test_strength_null();
    test_strength_valid();
    test_champion_snapshot_null();
    test_creature_snapshot_null();
    test_damage_gate_null();
    test_runtime_result_null();
    test_luck_writeback_null();
    test_aftermath_null();
    test_reaction_null();
    test_death_smoke_null();
    test_death_smoke_attack();
    test_possession_drop_null();
    test_killed_some_state_null();
    test_killed_all_state_null();
    test_timeline_cleanup_null();
    test_mutation_dispatch_null();

    puts("ok: DM1 melee action F0402 (Q-DM1-05) 30 tests passed");
    return 0;
}
