/* ReDMCSB MENU.C F0401:969-976 and F0407:1398-1440,1536-1541: action
 * receipts consume bounded M002_RANDOM/M005_RANDOM results without wrapping. */
#include "dm1_v1_action_xp_graphic560_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_window_domain(void)
{
    DM1_ActionWindowInputPc34 input;
    DM1_ActionWindowPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.earthSkillLevel = 5;
    input.randomDraw = 12;
    assert(dm1_v1_action_window_plan_f0407_pc34(&input, &plan) == 1);
    assert(plan.valid == 1);
    assert(plan.durationTicks == 17);

    input.randomDraw = 13;
    memset(&plan, 0xA5, sizeof(plan));
    assert(dm1_v1_action_window_plan_f0407_pc34(&input, &plan) == 0);
    assert(plan.valid == 0);

    input.randomDraw = -1;
    memset(&plan, 0xA5, sizeof(plan));
    assert(dm1_v1_action_window_plan_f0407_pc34(&input, &plan) == 0);
    assert(plan.valid == 0);
}

static void test_fright_domain(void)
{
    DM1_ActionFrightInputPc34 input;
    DM1_ActionFrightPlanPc34 plan;

    memset(&input, 0, sizeof(input));
    input.actionIndex = DM1_ACTION_WAR_CRY;
    input.influenceSkillLevel = 4;
    input.fearResistance = 2;
    input.movementTicks = 4;
    input.randomDraw = 2;
    assert(dm1_v1_action_fright_plan_f0401_pc34(&input, &plan) == 1);
    assert(plan.valid == 1);
    assert(plan.frightened == 1);

    input.randomDraw = 7;
    memset(&plan, 0xA5, sizeof(plan));
    assert(dm1_v1_action_fright_plan_f0401_pc34(&input, &plan) == 0);
    assert(plan.valid == 0);
}

static void test_flip_domain(void)
{
    DM1_ActionFlipInputPc34 input;
    DM1_ActionFlipPlanPc34 plan;

    input.randomDraw = 0;
    assert(dm1_v1_action_flip_plan_f0407_pc34(&input, &plan) == 1);
    assert(plan.valid == 1);
    assert(plan.comesUpHeads == 0);

    input.randomDraw = 1;
    assert(dm1_v1_action_flip_plan_f0407_pc34(&input, &plan) == 1);
    assert(plan.valid == 1);
    assert(plan.comesUpHeads == 1);

    input.randomDraw = 2;
    memset(&plan, 0xA5, sizeof(plan));
    assert(dm1_v1_action_flip_plan_f0407_pc34(&input, &plan) == 0);
    assert(plan.valid == 0);
}

int main(void)
{
    test_window_domain();
    test_fright_domain();
    test_flip_domain();
    puts("PASS dm1_v1_action_random_domain_pc34_compat");
    return 0;
}
