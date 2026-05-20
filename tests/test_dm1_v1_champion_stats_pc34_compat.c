#include <stdio.h>
#include <string.h>

#include "dm1_v1_champion_stats_pc34_compat.h"

static int expect_int(const char* label, int got, int want)
{
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static M11_ChampionStats make_champion(void)
{
    M11_ChampionStatsState state;
    m11_stats_init(&state);
    (void)m11_stats_add_champion(&state, "HALK");
    state.champions[0].stats[DM1_STAT_STRENGTH] = 50;
    state.champions[0].maxStats[DM1_STAT_STRENGTH] = 50;
    state.champions[0].stats[DM1_STAT_STAMINA] = 100;
    state.champions[0].maxStats[DM1_STAT_STAMINA] = 100;
    return state.champions[0];
}

int main(void)
{
    M11_ChampionStats champion;
    int ok = 1;

    printf("probe=dm1_v1_champion_stats_pc34_compat\n");
    printf("sourceEvidence=CHAMPION.C:1078-1104,1157-1215,2025-2048; MOVESENS.C:590-598; DUNGEON.C:1082-1134\n");

    ok &= expect_int("F0306 above half keeps value",
        dm1_stats_stamina_adjusted_value_pc34(60, 100, 500), 500);
    ok &= expect_int("F0306 below half uses halved value in second term",
        dm1_stats_stamina_adjusted_value_pc34(45, 100, 500), 475);

    champion = make_champion();
    ok &= expect_int("F0309 base strength maximum load rounded",
        m11_stats_maximum_load_pc34(&champion), 500);

    champion.stats[DM1_STAT_STAMINA] = 45;
    ok &= expect_int("F0309 low stamina reduces maximum load then rounds",
        m11_stats_maximum_load_pc34(&champion), 480);

    champion = make_champion();
    champion.wounds = DM1_WOUND_LEGS;
    ok &= expect_int("F0309 leg wound removes quarter of maximum load",
        m11_stats_maximum_load_pc34(&champion), 380);

    champion = make_champion();
    champion.wounds = DM1_WOUND_FEET;
    ok &= expect_int("F0309 non-leg wound removes eighth of maximum load",
        m11_stats_maximum_load_pc34(&champion), 440);

    champion = make_champion();
    champion.feetIconIndex = DM1_ICON_ARMOUR_ELVEN_BOOTS;
    ok &= expect_int("F0309 elven boots add one sixteenth before rounding",
        m11_stats_maximum_load_pc34(&champion), 540);

    champion = make_champion();
    champion.load = 200;
    ok &= expect_int("F0310 unloaded/under threshold movement cadence",
        m11_stats_movement_ticks_pc34(&champion), 2);

    champion.load = 320;
    ok &= expect_int("F0310 heavy under max movement cadence",
        m11_stats_movement_ticks_pc34(&champion), 3);

    champion.load = 500;
    ok &= expect_int("F0310 BUG0_72 load equals max is overloaded",
        m11_stats_movement_ticks_pc34(&champion), 4);

    champion.load = 1250;
    ok &= expect_int("F0310 overloaded movement cadence scales by load",
        m11_stats_movement_ticks_pc34(&champion), 10);

    champion.wounds = DM1_WOUND_FEET;
    champion.feetIconIndex = DM1_ICON_ARMOUR_BOOT_OF_SPEED;
    champion.load = 500;
    ok &= expect_int("F0310 feet wound and boots of speed both apply",
        m11_stats_movement_ticks_pc34(&champion), 5);

    champion = make_champion();
    champion.load = 200;
    ok &= expect_int("MOVESENS rope stamina cost uses load/max load ratio",
        m11_stats_movement_stamina_cost_pc34(&champion), 11);

    return ok ? 0 : 1;
}
