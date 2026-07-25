#include "dm1_v1_f0259_quiver_refill_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_plan_struct(void)
{
    struct DM1F0259QuiverRefillPlanPc34 p;
    memset(&p, 0, sizeof(p));
    assert(p.valid == 0);
    assert(p.moved == 0);
    assert(p.thing == 0);
}

static void test_plan_null_out(void)
{
    int ok = DM1_V1_F0259_PlanQuiverRefillPc34Compat(NULL, 0, 0, NULL);
    (void)ok;
    assert(ok == 0);
}

static void test_plan_null_champion(void)
{
    struct DM1F0259QuiverRefillPlanPc34 p;
    int ok = DM1_V1_F0259_PlanQuiverRefillPc34Compat(NULL, 0, 0, &p);
    (void)ok;
    assert(ok == 0);
}

static void test_plan_empty_champion(void)
{
    struct ChampionState_Compat ch;
    struct DM1F0259QuiverRefillPlanPc34 p;
    int i;
    memset(&ch, 0, sizeof(ch));
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i)
        ch.inventory[i] = THING_NONE;
    int ok = DM1_V1_F0259_PlanQuiverRefillPc34Compat(&ch, 0, 0, &p);
    (void)ok;
    assert(ok == 1);
    assert(p.valid == 1);
    assert(p.moved == 0);
}

static void test_apply_null_out(void)
{
    int ok = DM1_V1_F0259_ApplyQuiverRefillFromDungeonPc34Compat(
        NULL, 0, 0, NULL, NULL);
    (void)ok;
    assert(ok == 0);
}

int main(void)
{
    test_plan_struct();
    test_plan_null_out();
    test_plan_null_champion();
    test_plan_empty_champion();
    test_apply_null_out();

    puts("ok: DM1 F0259 quiver refill (Q-DM1-06) 5 tests passed");
    return 0;
}
