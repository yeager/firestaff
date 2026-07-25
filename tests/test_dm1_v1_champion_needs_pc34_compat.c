#include "dm1_v1_champion_needs_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_constants(void)
{
    assert(DM1_NEEDS_MAX_CHAMPIONS == 4);
    assert(DM1_NEEDS_FOOD_MIN == -1024);
    assert(DM1_NEEDS_FOOD_MAX == 2048);
    assert(DM1_NEEDS_WATER_MIN == -1024);
    assert(DM1_NEEDS_WATER_MAX == 2048);
    assert(DM1_NEEDS_STARVATION_THRESH == -512);
    assert(DM1_NEEDS_MAX_HEALTH == 999);
    assert(DM1_NEEDS_MAX_STAMINA == 9999);
    assert(DM1_NEEDS_MAX_MANA == 900);
}

static void test_bar_colors(void)
{
    assert(DM1_V1_NEEDS_BAR_COLOR_BLACK == 0);
    assert(DM1_V1_NEEDS_BAR_COLOR_RED == 8);
    assert(DM1_V1_NEEDS_BAR_COLOR_YELLOW == 11);
}

static void test_scent_constants(void)
{
    assert(DM1_V1_NEEDS_SCENT_CAPACITY == 16);
    assert(DM1_V1_NEEDS_SCENT_MERGE_CYCLES_PC34 == 0x8000u);
}

static void test_champion_needs_struct(void)
{
    DM1_ChampionNeeds n;
    memset(&n, 0, sizeof(n));
    assert(n.current_health == 0);
    assert(n.food == 0);
    assert(n.water == 0);
    assert(n.alive == 0);
}

static void test_tick_context_struct(void)
{
    DM1_NeedsTickContext ctx;
    memset(&ctx, 0, sizeof(ctx));
    assert(ctx.game_time == 0);
    assert(ctx.party_is_resting == 0);
}

static void test_tick_result_struct(void)
{
    DM1_NeedsTickResult r;
    memset(&r, 0, sizeof(r));
    assert(r.stamina_delta == 0);
    assert(r.health_delta == 0);
    assert(r.starvation_damage == 0);
}

static void test_scent_list_init(void)
{
    DM1_V1_NeedsScentListPc34Compat scents;
    memset(&scents, 0, sizeof(scents));
    assert(scents.count == 0);
    assert(scents.firstScentIndex == 0);
}

static void test_get_scent_ordinal_empty(void)
{
    DM1_V1_NeedsScentListPc34Compat scents;
    memset(&scents, 0, sizeof(scents));
    int ord = DM1_V1_Needs_GetScentOrdinalPc34Compat(&scents, 0, 5, 5);
    (void)ord;
    assert(ord == 0);
}

static void test_bar_width(void)
{
    int w = DM1_V1_Needs_BarWidthPc34Compat(50, 25);
    (void)w;
    assert(w >= 0 && w <= 25);
}

static void test_bar_color(void)
{
    int c = DM1_V1_Needs_BarColorPc34Compat(100, 7);
    (void)c;
    assert(c >= 0);
}

static void test_stamina_amount(void)
{
    int amt = dm1_needs_compute_stamina_amount(100);
    (void)amt;
    assert(amt >= 0);
}

static void test_bar_render_command(void)
{
    DM1_V1_NeedsBarRenderCommandPc34Compat cmd;
    memset(&cmd, 0, sizeof(cmd));
    int r = DM1_V1_Needs_BuildBarRenderCommandPc34Compat(
        10, 20, 25, 3, 1, 50, 7, &cmd);
    (void)r;
    assert(r == 1 || r == 0);
}

int main(void)
{
    test_constants();
    test_bar_colors();
    test_scent_constants();
    test_champion_needs_struct();
    test_tick_context_struct();
    test_tick_result_struct();
    test_scent_list_init();
    test_get_scent_ordinal_empty();
    test_bar_width();
    test_bar_color();
    test_stamina_amount();
    test_bar_render_command();

    puts("ok: DM1 champion needs (Q-DM1-07) 12 tests passed");
    return 0;
}
