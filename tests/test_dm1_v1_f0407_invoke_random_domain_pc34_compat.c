/* ReDMCSB MENU.C F0407:1480-1493: INVOKE uses M003_RANDOM(128) then
 * M002_RANDOM(6); the runtime projectile plan must not manufacture values
 * outside either source domain. */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_invoke_uses_source_random_domains(void)
{
    DM1_ActionProjectileSpellInputPc34 input;
    DM1_ActionProjectileSpellPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.actionIndex = DM1_ACTION_INVOKE;
    input.skillLevel = 6;
    input.currentMana = 8;
    input.maximumMana = 80;
    input.invokeEnergyRoll = 0;
    input.invokeFamilyRoll = 0;
    assert(dm1_v1_action_projectile_spell_plan_f0407_pc34(&input, &plan) == 1);
    assert(plan.valid == 1);
    assert(plan.baseKineticEnergy == 100);
    assert(plan.subtype == PROJECTILE_SUBTYPE_POISON_BOLT);

    input.invokeEnergyRoll = 127;
    input.invokeFamilyRoll = 5;
    assert(dm1_v1_action_projectile_spell_plan_f0407_pc34(&input, &plan) == 1);
    assert(plan.valid == 1);
    assert(plan.baseKineticEnergy == 227);
    assert(plan.subtype == PROJECTILE_SUBTYPE_FIREBALL);

    input.invokeEnergyRoll = 128;
    input.invokeFamilyRoll = 0;
    memset(&plan, 0xA5, sizeof(plan));
    assert(dm1_v1_action_projectile_spell_plan_f0407_pc34(&input, &plan) == 0);
    assert(plan.valid == 0);

    input.invokeEnergyRoll = 0;
    input.invokeFamilyRoll = 6;
    memset(&plan, 0xA5, sizeof(plan));
    assert(dm1_v1_action_projectile_spell_plan_f0407_pc34(&input, &plan) == 0);
    assert(plan.valid == 0);
}

int main(void)
{
    test_invoke_uses_source_random_domains();
    puts("PASS dm1_v1_f0407_invoke_random_domain_pc34_compat");
    return 0;
}
