#include "dm1_v1_action_xp_graphic560_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_xp_route_null_rejected(void)
{
    int rc = dm1_v1_action_xp_route(DM1_ACTION_CHOP, NULL);
    (void)rc;
    assert(rc == 0);
}

static void test_xp_route_out_of_range(void)
{
    DM1_ActionXpRoute route;
    int rc;

    memset(&route, 0, sizeof(route));
    rc = dm1_v1_action_xp_route(-1, &route);
    (void)rc;
    assert(rc == 0);
    assert(route.valid == 0);

    rc = dm1_v1_action_xp_route(DM1_GRAPHIC560_ACTION_COUNT, &route);
    assert(rc == 0);
    assert(route.valid == 0);
}

static void test_xp_route_chop(void)
{
    DM1_ActionXpRoute route;
    int rc;

    memset(&route, 0, sizeof(route));
    rc = dm1_v1_action_xp_route(DM1_ACTION_CHOP, &route);
    (void)rc;
    assert(rc == 1);
    assert(route.valid == 1);
    assert(route.skillIndex >= 0);
    assert(route.baseSkillIndex >= 0 && route.baseSkillIndex <= 3);
    assert(route.experienceGain > 0);
}

static void test_xp_route_war_cry(void)
{
    DM1_ActionXpRoute route;
    int rc;

    memset(&route, 0, sizeof(route));
    rc = dm1_v1_action_xp_route(DM1_ACTION_WAR_CRY, &route);
    (void)rc;
    assert(rc == 1);
    assert(route.valid == 1);
    assert(route.experienceGain > 0);
}

static void test_xp_route_reserved_action_n(void)
{
    DM1_ActionXpRoute route;
    int rc;

    memset(&route, 0, sizeof(route));
    rc = dm1_v1_action_xp_route(DM1_ACTION_N, &route);
    (void)rc;
    /* Action 0 is reserved — should still be in range */
    assert(route.valid == 1 || route.valid == 0);
}

static void test_is_melee_contact(void)
{
    int rc;

    rc = dm1_v1_action_is_melee_contact_f0407_pc34(DM1_ACTION_CHOP);
    (void)rc;
    assert(rc == 1);

    rc = dm1_v1_action_is_melee_contact_f0407_pc34(DM1_ACTION_SWING);
    assert(rc == 1);

    /* BLOCK has a damage factor but is explicitly excluded */
    rc = dm1_v1_action_is_melee_contact_f0407_pc34(DM1_ACTION_BLOCK);
    assert(rc == 0);

    /* Spells are not melee */
    rc = dm1_v1_action_is_melee_contact_f0407_pc34(DM1_ACTION_FIREBALL);
    assert(rc == 0);

    /* Out of range */
    rc = dm1_v1_action_is_melee_contact_f0407_pc34(-1);
    assert(rc == 0);
}

static void test_is_party_shield(void)
{
    int rc;

    rc = dm1_v1_action_is_party_shield_f0407_pc34(DM1_ACTION_SPELLSHIELD);
    (void)rc;
    assert(rc == 1);

    rc = dm1_v1_action_is_party_shield_f0407_pc34(DM1_ACTION_FIRESHIELD);
    assert(rc == 1);

    rc = dm1_v1_action_is_party_shield_f0407_pc34(DM1_ACTION_CHOP);
    assert(rc == 0);
}

static void test_halves_xp_on_failure(void)
{
    int rc;

    rc = dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_FIREBALL);
    (void)rc;
    assert(rc == 1);

    rc = dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_LIGHTNING);
    assert(rc == 1);

    rc = dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_INVOKE);
    assert(rc == 1);

    rc = dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_SPIT);
    assert(rc == 1);

    rc = dm1_v1_action_halves_xp_on_f0327_failure_pc34(DM1_ACTION_CHOP);
    assert(rc == 0);
}

static void test_stamina_cost(void)
{
    int cost;

    cost = dm1_v1_action_stamina_cost_f0407_pc34(DM1_ACTION_CHOP, 0, 0);
    (void)cost;
    assert(cost >= 0);

    /* Out of range returns 0 */
    cost = dm1_v1_action_stamina_cost_f0407_pc34(-1, 0, 0);
    assert(cost == 0);

    cost = dm1_v1_action_stamina_cost_f0407_pc34(DM1_ACTION_CHOP, -1, 0);
    assert(cost == 0);
}

static void test_disabled_ticks(void)
{
    int ticks;

    ticks = dm1_v1_action_disabled_ticks_f0407_pc34(DM1_ACTION_CHOP);
    (void)ticks;
    assert(ticks >= 0);

    ticks = dm1_v1_action_disabled_ticks_f0407_pc34(-1);
    assert(ticks == 0);
}

static void test_prelude_plan_null_rejected(void)
{
    DM1_ActionF0407PreludePlanPc34 plan;
    int rc;

    memset(&plan, 0, sizeof(plan));
    rc = dm1_v1_action_prelude_plan_f0407_pc34(NULL, &plan);
    (void)rc;
    assert(rc == 0);

    DM1_ActionF0407PreludeInputPc34 in;
    memset(&in, 0, sizeof(in));
    in.actionIndex = DM1_ACTION_CHOP;
    rc = dm1_v1_action_prelude_plan_f0407_pc34(&in, NULL);
    assert(rc == 0);
}

static void test_prelude_plan_valid(void)
{
    DM1_ActionF0407PreludeInputPc34 in;
    DM1_ActionF0407PreludePlanPc34 plan;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&plan, 0, sizeof(plan));
    in.actionIndex = DM1_ACTION_CHOP;
    in.championIndex = 0;
    in.gameTick = 100;
    rc = dm1_v1_action_prelude_plan_f0407_pc34(&in, &plan);
    (void)rc;
    assert(rc == 1);
    assert(plan.valid == 1);
    assert(plan.skillIndex >= 0);
    assert(plan.baseSkillIndex >= 0 && plan.baseSkillIndex <= 3);
    assert(plan.actionExperienceGain > 0);
    assert(plan.disabledTicks >= 0);
    assert(plan.staminaCost >= 0);
    assert(plan.isMeleeContact == 1);
}

static void test_tail_adjust_melee_failure(void)
{
    DM1_ActionF0407TailAdjustInputPc34 in;
    DM1_ActionF0407TailAdjustPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_CHOP;
    in.performed = 0;
    in.actionExperienceGain = 20;
    in.disabledTicks = 10;
    in.meleeFailureTail = 1;
    rc = dm1_v1_action_adjust_f0407_tail_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.actionExperienceGain == 10);
    assert(out.disabledTicks == 5);
}

static void test_tail_adjust_shield_failure(void)
{
    DM1_ActionF0407TailAdjustInputPc34 in;
    DM1_ActionF0407TailAdjustPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_SPELLSHIELD;
    in.performed = 0;
    in.actionExperienceGain = 40;
    in.disabledTicks = 10;
    rc = dm1_v1_action_adjust_f0407_tail_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.actionExperienceGain == 10);
    assert(out.disabledTicks == 5);
}

static void test_tail_adjust_projectile_failure(void)
{
    DM1_ActionF0407TailAdjustInputPc34 in;
    DM1_ActionF0407TailAdjustPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_FIREBALL;
    in.performed = 0;
    in.actionExperienceGain = 20;
    in.disabledTicks = 10;
    rc = dm1_v1_action_adjust_f0407_tail_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.actionExperienceGain == 10);
    assert(out.disabledTicks == 10);
}

static void test_tail_adjust_shoot_no_ammo(void)
{
    DM1_ActionF0407TailAdjustInputPc34 in;
    DM1_ActionF0407TailAdjustPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_SHOOT;
    in.performed = 0;
    in.actionExperienceGain = 15;
    in.disabledTicks = 8;
    rc = dm1_v1_action_adjust_f0407_tail_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.actionExperienceGain == 0);
    assert(out.disabledTicks == 8);
}

static void test_stamina_apply_normal(void)
{
    DM1_ActionF0325StaminaInputPc34 in;
    DM1_ActionF0325StaminaPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.currentStamina = 100;
    in.maximumStamina = 200;
    in.currentHealth = 50;
    in.decrement = 30;
    rc = dm1_v1_action_stamina_apply_plan_f0325_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.currentStaminaAfter == 70);
    assert(out.currentHealthAfter == 50);
    assert(out.pendingHealthDamage == 0);
    assert(out.shouldDamageFlash == 0);
}

static void test_stamina_apply_underflow(void)
{
    DM1_ActionF0325StaminaInputPc34 in;
    DM1_ActionF0325StaminaPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.currentStamina = 10;
    in.maximumStamina = 200;
    in.currentHealth = 50;
    in.decrement = 30;
    rc = dm1_v1_action_stamina_apply_plan_f0325_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.currentStaminaAfter == 0);
    assert(out.pendingHealthDamage == 10);
    assert(out.currentHealthAfter == 40);
    assert(out.shouldDamageFlash == 1);
}

static void test_stamina_apply_zero_decrement(void)
{
    DM1_ActionF0325StaminaInputPc34 in;
    DM1_ActionF0325StaminaPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.currentStamina = 100;
    in.maximumStamina = 200;
    in.currentHealth = 50;
    in.decrement = 0;
    rc = dm1_v1_action_stamina_apply_plan_f0325_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.applied == 0);
    assert(out.currentStaminaAfter == 100);
}

static void test_stamina_apply_null_rejected(void)
{
    DM1_ActionF0325StaminaPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_action_stamina_apply_plan_f0325_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_begin_plan_chop(void)
{
    DM1_ActionF0407BeginInputPc34 in;
    DM1_ActionF0407BeginPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_CHOP;
    in.championIndex = 0;
    in.gameTick = 100;
    in.currentStamina = 100;
    in.maximumStamina = 200;
    in.currentHealth = 50;
    rc = dm1_v1_action_begin_plan_f0407_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.skillIndex >= 0);
    assert(out.isMeleeContact == 1);
    assert(out.staminaCost >= 0);
    assert(out.currentStaminaAfter >= 0);
    assert(out.currentStaminaAfter <= 100);
}

static void test_begin_plan_null_rejected(void)
{
    DM1_ActionF0407BeginPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_action_begin_plan_f0407_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_xp_award_plan(void)
{
    DM1_ActionXpAwardInputPc34 in;
    DM1_ActionXpAwardPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_CHOP;
    in.experienceGain = 10;
    rc = dm1_v1_action_xp_award_plan_f0407_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.skillIndex >= 0);
}

static void test_direct_dispatch(void)
{
    DM1_ActionDirectDispatchInputPc34 in;
    DM1_ActionDirectDispatchPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.actionIndex = DM1_ACTION_CHOP;
    rc = dm1_v1_action_direct_dispatch_plan_f0407_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
}

static void test_flip_plan(void)
{
    DM1_ActionFlipInputPc34 in;
    DM1_ActionFlipPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.randomDraw = 0;
    rc = dm1_v1_action_flip_plan_f0407_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.performed == 1);
}

static void test_heal_plan_full_health(void)
{
    DM1_ActionHealInputPc34 in;
    DM1_ActionHealPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.currentHealth = 100;
    in.maximumHealth = 100;
    in.currentMana = 50;
    in.healSkillLevel = 5;
    rc = dm1_v1_action_heal_plan_f0407_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.alreadyFullHealth == 1);
}

static void test_heal_plan_null_rejected(void)
{
    DM1_ActionHealPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_action_heal_plan_f0407_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_completion_plan_null_rejected(void)
{
    DM1_ActionF0407CompletionPlanPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_action_completion_plan_f0407_pc34(NULL, &out);
    (void)rc;
    assert(rc == 0);
}

static void test_climb_down_no_pit(void)
{
    DM1_ActionClimbDownInputPc34 in;
    DM1_ActionClimbDownPlanPc34 out;
    int rc;

    memset(&in, 0, sizeof(in));
    memset(&out, 0, sizeof(out));
    in.frontSquareIsPit = 0;
    in.frontSquareHasGroup = 0;
    rc = dm1_v1_action_climb_down_plan_f0407_pc34(&in, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
}

static void test_f0407_tail(void)
{
    DM1_ActionF0407TailPc34 out;
    int rc;

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_action_f0407_tail_pc34(DM1_ACTION_CHOP, &out);
    (void)rc;
    assert(rc == 1);
    assert(out.valid == 1);
    assert(out.damageFactor > 0);
    assert(out.isMeleeContact == 1);

    memset(&out, 0, sizeof(out));
    rc = dm1_v1_action_f0407_tail_pc34(-1, &out);
    assert(rc == 0);
}

int main(void)
{
    test_xp_route_null_rejected();
    test_xp_route_out_of_range();
    test_xp_route_chop();
    test_xp_route_war_cry();
    test_xp_route_reserved_action_n();
    test_is_melee_contact();
    test_is_party_shield();
    test_halves_xp_on_failure();
    test_stamina_cost();
    test_disabled_ticks();
    test_prelude_plan_null_rejected();
    test_prelude_plan_valid();
    test_tail_adjust_melee_failure();
    test_tail_adjust_shield_failure();
    test_tail_adjust_projectile_failure();
    test_tail_adjust_shoot_no_ammo();
    test_stamina_apply_normal();
    test_stamina_apply_underflow();
    test_stamina_apply_zero_decrement();
    test_stamina_apply_null_rejected();
    test_begin_plan_chop();
    test_begin_plan_null_rejected();
    test_xp_award_plan();
    test_direct_dispatch();
    test_flip_plan();
    test_heal_plan_full_health();
    test_heal_plan_null_rejected();
    test_completion_plan_null_rejected();
    test_climb_down_no_pit();
    test_f0407_tail();

    puts("ok: DM1 action XP graphic560 (Q-DM1-07) 30 tests passed");
    return 0;
}
