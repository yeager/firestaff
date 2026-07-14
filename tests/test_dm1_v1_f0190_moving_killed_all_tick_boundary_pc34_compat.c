/* ReDMCSB GROUP.C F0190 -> F0213 tick-domain handoff regression. */
#include "dm1_v1_melee_action_f0402_pc34_compat.h"

#include <assert.h>
#include <limits.h>
#include <stdio.h>
#include <string.h>

static void fill_valid_input(DM1_MeleeF0190MovingKilledAllAfterplayInputPc34* in)
{
    memset(in, 0, sizeof(*in));
    in->outcome = COMBAT_OUTCOME_KILLED_ALL_CREATURES;
    in->groupIndex = 3;
    in->creatureAttributes = DM1_SIZE_FULL_SQUARE;
    in->sourceMapIndex = 1;
    in->sourceMapX = 4;
    in->sourceMapY = 5;
    in->sourceCell = EXPLOSION_CELL_CENTERED;
    in->destinationMapIndex = 1;
    in->destinationMapX = 5;
    in->destinationMapY = 5;
}

int main(void)
{
    DM1_MeleeF0190MovingKilledAllAfterplayInputPc34 input;
    DM1_MeleeF0190MovingKilledAllAfterplayPlanPc34 plan;

    fill_valid_input(&input);
    input.currentTick = (unsigned int)INT_MAX;
    assert(dm1_v1_melee_moving_killed_all_afterplay_plan_f0190_pc34(
               &input, &plan) &&
           plan.valid && plan.shouldPresentSourceSmoke &&
           plan.sourceSmokeCreateInput.currentTick == INT_MAX);

    input.currentTick = (unsigned int)INT_MAX + 1u;
    assert(dm1_v1_melee_moving_killed_all_afterplay_plan_f0190_pc34(
               &input, &plan) &&
           !plan.valid && !plan.shouldPresentSourceSmoke &&
           !plan.requiresDeferredDestinationCleanup && plan.groupIndex == -1);

    puts("PASS dm1_v1_f0190_moving_killed_all_tick_boundary_pc34_compat");
    return 0;
}
