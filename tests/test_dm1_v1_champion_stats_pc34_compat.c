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

static DM1_V1_ChampionStatsPc34 make_champion(void)
{
    DM1_V1_ChampionStatsStatePc34 state;
    DM1_V1_ChampionStats_InitPc34Compat(&state);
    (void)DM1_V1_ChampionStats_AddChampionPc34Compat(&state, "HALK");
    state.champions[0].stats[DM1_STAT_STRENGTH] = 50;
    state.champions[0].maxStats[DM1_STAT_STRENGTH] = 50;
    state.champions[0].stats[DM1_STAT_STAMINA] = 100;
    state.champions[0].maxStats[DM1_STAT_STAMINA] = 100;
    return state.champions[0];
}

int main(void)
{
    DM1_V1_ChampionStatsPc34 champion;
    int ok = 1;

    printf("probe=dm1_v1_champion_stats_pc34_compat\n");
    printf("sourceEvidence=PANEL.C:2081-2105; CHAMPION.C:1078-1104,1157-1215,2025-2048; CHAMDRAW.C:958-1006; MOVESENS.C:590-598; DUNGEON.C:1082-1134\n");

    ok &= expect_int("F0306 above half keeps value",
        DM1_V1_ChampionStats_StaminaAdjustedValuePc34Compat(60, 100, 500), 500);
    ok &= expect_int("F0306 below half uses halved value in second term",
        DM1_V1_ChampionStats_StaminaAdjustedValuePc34Compat(45, 100, 500), 475);

    champion = make_champion();
    ok &= expect_int("F0309 base strength maximum load rounded",
        DM1_V1_ChampionStats_MaximumLoadPc34Compat(&champion), 500);

    champion.stats[DM1_STAT_STAMINA] = 45;
    ok &= expect_int("F0309 low stamina reduces maximum load then rounds",
        DM1_V1_ChampionStats_MaximumLoadPc34Compat(&champion), 480);

    champion = make_champion();
    champion.wounds = DM1_WOUND_LEGS;
    ok &= expect_int("F0309 leg wound removes quarter of maximum load",
        DM1_V1_ChampionStats_MaximumLoadPc34Compat(&champion), 380);

    champion = make_champion();
    champion.wounds = DM1_WOUND_FEET;
    ok &= expect_int("F0309 non-leg wound removes eighth of maximum load",
        DM1_V1_ChampionStats_MaximumLoadPc34Compat(&champion), 440);

    champion = make_champion();
    champion.feetIconIndex = DM1_ICON_ARMOUR_ELVEN_BOOTS;
    ok &= expect_int("F0309 elven boots add one sixteenth before rounding",
        DM1_V1_ChampionStats_MaximumLoadPc34Compat(&champion), 540);

    ok &= expect_int("PANEL F0351 statistic below maximum is red",
        DM1_V1_ChampionStats_StatisticColorPc34Compat(49, 50), DM1_STAT_COLOR_RED);
    ok &= expect_int("PANEL F0351 statistic equal to maximum is lightest gray",
        DM1_V1_ChampionStats_StatisticColorPc34Compat(50, 50), DM1_STAT_COLOR_LIGHTEST_GRAY);
    ok &= expect_int("PANEL F0351 statistic above maximum is light green",
        DM1_V1_ChampionStats_StatisticColorPc34Compat(51, 50), DM1_STAT_COLOR_LIGHT_GREEN);

    champion = make_champion();
    champion.stats[DM1_STAT_STRENGTH] = 42;
    champion.maxStats[DM1_STAT_STRENGTH] = 50;
    ok &= expect_int("PANEL F0351 champion statistic reads current/max rows",
        DM1_V1_ChampionStats_ChampionStatisticColorPc34Compat(&champion, DM1_STAT_STRENGTH),
        DM1_STAT_COLOR_RED);
    champion.stats[DM1_STAT_STRENGTH] = 60;
    ok &= expect_int("PANEL F0351 champion statistic buff above max is light green",
        DM1_V1_ChampionStats_ChampionStatisticColorPc34Compat(&champion, DM1_STAT_STRENGTH),
        DM1_STAT_COLOR_LIGHT_GREEN);
    ok &= expect_int("PANEL F0351 invalid statistic defaults to lightest gray",
        DM1_V1_ChampionStats_ChampionStatisticColorPc34Compat(&champion, DM1_STAT_COUNT),
        DM1_STAT_COLOR_LIGHTEST_GRAY);

    champion = make_champion();
    champion.load = 200;
    ok &= expect_int("F0310 unloaded/under threshold movement cadence",
        DM1_V1_ChampionStats_MovementTicksPc34Compat(&champion), 2);

    champion.load = 320;
    ok &= expect_int("F0310 heavy under max movement cadence",
        DM1_V1_ChampionStats_MovementTicksPc34Compat(&champion), 3);

    champion.load = 500;
    ok &= expect_int("F0310 BUG0_72 load equals max is overloaded",
        DM1_V1_ChampionStats_MovementTicksPc34Compat(&champion), 4);

    champion.load = 1250;
    ok &= expect_int("F0310 overloaded movement cadence scales by load",
        DM1_V1_ChampionStats_MovementTicksPc34Compat(&champion), 10);

    champion.wounds = DM1_WOUND_FEET;
    champion.feetIconIndex = DM1_ICON_ARMOUR_BOOT_OF_SPEED;
    champion.load = 500;
    ok &= expect_int("F0310 feet wound and boots of speed both apply",
        DM1_V1_ChampionStats_MovementTicksPc34Compat(&champion), 5);

    champion = make_champion();
    champion.load = 200;
    ok &= expect_int("MOVESENS rope stamina cost uses load/max load ratio",
        DM1_V1_ChampionStats_MovementStaminaCostPc34Compat(&champion), 11);

    champion = make_champion();
    champion.load = 312;
    ok &= expect_int("CHAMDRAW F0292 load at five-eighths threshold stays gray",
        DM1_V1_ChampionStats_LoadColorPc34Compat(&champion), DM1_LOAD_COLOR_LIGHTEST_GRAY);

    champion.load = 313;
    ok &= expect_int("CHAMDRAW F0292 load above five-eighths max is yellow",
        DM1_V1_ChampionStats_LoadColorPc34Compat(&champion), DM1_LOAD_COLOR_YELLOW);

    champion.load = 500;
    ok &= expect_int("CHAMDRAW F0292 load exactly at max remains yellow",
        DM1_V1_ChampionStats_LoadColorPc34Compat(&champion), DM1_LOAD_COLOR_YELLOW);

    champion.load = 501;
    ok &= expect_int("CHAMDRAW F0292 load above max is red",
        DM1_V1_ChampionStats_LoadColorPc34Compat(&champion), DM1_LOAD_COLOR_RED);

    champion = make_champion();
    champion.load = 200;
    {
        char loadString[16];
        if (!DM1_V1_ChampionStats_FormatLoadPc34Compat(&champion, loadString, sizeof(loadString)) ||
            strcmp(loadString, " 20.0/ 50 KG") != 0) {
            fprintf(stderr, "FAIL CHAMDRAW F0292 load string got='%s' want=' 20.0/ 50 KG'\n",
                    loadString);
            ok = 0;
        }
    }

    return ok ? 0 : 1;
}
