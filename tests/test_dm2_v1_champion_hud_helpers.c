#include "dm2_v1_champion_hud_helpers.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect_true(int condition, const char *label)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", label);
        ++failures;
    }
}

static void test_process_item_bonus(void)
{
    DM2_V1_ProcessItemBonusInput input = {0x1234u, 40, 7, 0, 99};
    DM2_V1_ChampionHudReceipt receipt;

    expect_true(dm2_v1_PROCESS_ITEM_BONUS(&input, &receipt) == 47,
                "PROCESS_ITEM_BONUS applies bounded positive bonus");
    expect_true(receipt.handled && receipt.source_locked && receipt.valid &&
                    receipt.result == 47 && receipt.dirty &&
                    strcmp(receipt.symbol, "PROCESS_ITEM_BONUS") == 0,
                "item bonus receipt records source symbol and dirty result");

    input.current_value = 95;
    input.item_bonus = 20;
    expect_true(dm2_v1_PROCESS_ITEM_BONUS(&input, &receipt) == 99,
                "PROCESS_ITEM_BONUS clamps to maximum");

    input.item_ref = DM2_V1_CHAMPION_HUD_NULL_ITEM;
    expect_true(dm2_v1_PROCESS_ITEM_BONUS(&input, &receipt) == 0,
                "PROCESS_ITEM_BONUS blocks null item");
    expect_true(receipt.blocked && !receipt.valid,
                "null item bonus is fail-closed");
}

static void test_query_player_skill_lv(void)
{
    DM2_V1_PlayerSkillInput input = {4096u, 1u, 0u, 8u};
    DM2_V1_ChampionHudReceipt receipt;

    expect_true(dm2_v1_QUERY_PLAYER_SKILL_LV(&input, &receipt) == 2u,
                "QUERY_PLAYER_SKILL_LV advances through bounded thresholds");
    expect_true(receipt.valid && receipt.result == 2 &&
                    strcmp(receipt.symbol, "QUERY_PLAYER_SKILL_LV") == 0,
                "skill level receipt records source symbol");

    input.experience = 0u;
    input.base_level = 14u;
    input.temporary_bonus = 5u;
    input.maximum_level = 15u;
    expect_true(dm2_v1_QUERY_PLAYER_SKILL_LV(&input, &receipt) == 15u,
                "QUERY_PLAYER_SKILL_LV clamps temporary boost");
    expect_true(receipt.dirty,
                "temporary skill boost marks display-affecting result");

    expect_true(dm2_v1_QUERY_PLAYER_SKILL_LV(0, &receipt) == 0u,
                "QUERY_PLAYER_SKILL_LV blocks missing input");
    expect_true(receipt.blocked && !receipt.valid,
                "missing skill input is fail-closed");
}

static void test_refresh_player_stat_disp(void)
{
    DM2_V1_PlayerStatDisplayInput input = {35, 70, 30, 70, 12};
    DM2_V1_PlayerStatDisplay display;
    DM2_V1_ChampionHudReceipt receipt;

    expect_true(dm2_v1_REFRESH_PLAYER_STAT_DISP(&input, &display,
                                                &receipt) == 1,
                "REFRESH_PLAYER_STAT_DISP accepts bounded stat input");
    expect_true(display.percent == 50u && display.redraw_value &&
                    !display.redraw_maximum && display.redraw_bar &&
                    receipt.valid && receipt.result == 50 &&
                    strcmp(receipt.symbol,
                           "REFRESH_PLAYER_STAT_DISP") == 0,
                "stat display receipt records percent and redraw flags");

    input.current_value = 90;
    input.maximum_value = 80;
    input.previous_current_value = 80;
    input.previous_maximum_value = 80;
    expect_true(dm2_v1_REFRESH_PLAYER_STAT_DISP(&input, &display,
                                                &receipt) == 1,
                "REFRESH_PLAYER_STAT_DISP clamps current to maximum");
    expect_true(display.current_value == 80 && display.percent == 100u,
                "stat display percent is capped at full bar");

    input.maximum_value = 0;
    expect_true(dm2_v1_REFRESH_PLAYER_STAT_DISP(&input, &display,
                                                &receipt) == 0,
                "REFRESH_PLAYER_STAT_DISP blocks zero maximum");
    expect_true(receipt.blocked && !receipt.valid,
                "invalid stat display input is fail-closed");
}

static void test_bar_color_queries(void)
{
    DM2_V1_BarColorReceipt receipt;

    /* QUERY_FOOD_WATER_BAR_COLOR: no GDAT → default */
    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(-1, 5, &receipt) == 5,
                "food/water bar color returns default when no GDAT");
    expect_true(receipt.valid && !receipt.gdat_override && receipt.color == 5,
                "food/water receipt: no override, default color");

    /* QUERY_FOOD_WATER_BAR_COLOR: GDAT found → 256 + value */
    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(42, 5, &receipt) == 298,
                "food/water bar color returns 256+gdat when found");
    expect_true(receipt.valid && receipt.gdat_override && receipt.color == 298,
                "food/water receipt: GDAT override active");

    /* QUERY_3STAT_BAR_COLOR: no GDAT → default */
    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(-1, 7, &receipt) == 7,
                "3stat bar color returns default when no GDAT");
    expect_true(receipt.valid && !receipt.gdat_override,
                "3stat receipt: no override");

    /* QUERY_3STAT_BAR_COLOR: GDAT found → raw value (no 256 offset) */
    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(11, 7, &receipt) == 11,
                "3stat bar color returns GDAT value directly");
    expect_true(receipt.valid && receipt.gdat_override && receipt.color == 11,
                "3stat receipt: GDAT override active");

    /* NULL receipt is safe */
    expect_true(dm2_v1_QUERY_FOOD_WATER_BAR_COLOR(-1, 14, 0) == 14,
                "food/water bar color works with NULL receipt");
    expect_true(dm2_v1_QUERY_3STAT_BAR_COLOR(-1, 8, 0) == 8,
                "3stat bar color works with NULL receipt");
}

static void test_hero_bar_color_table(void)
{
    expect_true(dm2_v1_default_hero_bar_color[0] == 7,
                "hero 0 bar color is 7");
    expect_true(dm2_v1_default_hero_bar_color[1] == 11,
                "hero 1 bar color is 11");
    expect_true(dm2_v1_default_hero_bar_color[2] == 8,
                "hero 2 bar color is 8");
    expect_true(dm2_v1_default_hero_bar_color[3] == 14,
                "hero 3 bar color is 14");
}

int main(void)
{
    test_process_item_bonus();
    test_query_player_skill_lv();
    test_refresh_player_stat_disp();
    test_bar_color_queries();
    test_hero_bar_color_table();
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "PROCESS_ITEM_BONUS:5277") != 0,
                "source evidence includes item bonus symbol");
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "REFRESH_PLAYER_STAT_DISP:14573") != 0,
                "source evidence includes stat display symbol");
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "QUERY_FOOD_WATER_BAR_COLOR:13194") != 0,
                "source evidence includes food/water bar color symbol");
    expect_true(strstr(dm2_v1_champion_hud_helpers_source_evidence(),
                       "QUERY_3STAT_BAR_COLOR:13203") != 0,
                "source evidence includes 3stat bar color symbol");
    if (failures) {
        return 1;
    }
    puts("DM2 champion/HUD helpers: ok");
    return 0;
}
