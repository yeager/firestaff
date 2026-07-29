#include "dm2_v1_champion_stat_bridge.h"
#include <assert.h>
#include <string.h>
#include <stdio.h>

static void test_single_hero_full_health(void)
{
    DM2_V1_ChampionStatInput in = {
        .cur_hp = 100, .max_hp = 100,
        .cur_stamina = 200, .max_stamina = 200,
        .cur_mp = 50, .max_mp = 50,
        .is_leader = 1, .spell_ready = 0
    };
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(&in, NULL, 1, -1, &r));
    assert(r.valid && r.champion_count == 1);
    assert(r.champions[0].hp_pct == 100);
    assert(r.champions[0].stamina_pct == 100);
    assert(r.champions[0].mana_pct == 100);
    assert(r.champions[0].is_alive && r.champions[0].is_leader);
    assert(r.champions[0].hp_bar_color == 7);
    assert(r.champions[0].redraw_hp);
}

static void test_partial_health(void)
{
    DM2_V1_ChampionStatInput in = {
        .cur_hp = 30, .max_hp = 100,
        .cur_stamina = 50, .max_stamina = 200,
        .cur_mp = 10, .max_mp = 40,
    };
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(&in, NULL, 1, -1, &r));
    assert(r.champions[0].hp_pct == 30);
    assert(r.champions[0].stamina_pct == 25);
    assert(r.champions[0].mana_pct == 25);
}

static void test_dead_hero(void)
{
    DM2_V1_ChampionStatInput in = {
        .cur_hp = 0, .max_hp = 100,
        .cur_stamina = 0, .max_stamina = 200,
        .cur_mp = 0, .max_mp = 50,
    };
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(&in, NULL, 1, -1, &r));
    assert(!r.champions[0].is_alive);
    assert(r.champions[0].hp_pct == 0);
}

static void test_mana_exceeds_max(void)
{
    DM2_V1_ChampionStatInput in = {
        .cur_hp = 50, .max_hp = 50,
        .cur_stamina = 100, .max_stamina = 100,
        .cur_mp = 60, .max_mp = 40,
    };
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(&in, NULL, 1, -1, &r));
    assert(r.champions[0].mana_pct == 100);
}

static void test_four_heroes_bar_colors(void)
{
    DM2_V1_ChampionStatInput ins[4];
    memset(ins, 0, sizeof(ins));
    for (int i = 0; i < 4; i++) {
        ins[i].cur_hp = 50;
        ins[i].max_hp = 100;
        ins[i].cur_stamina = 100;
        ins[i].max_stamina = 100;
        ins[i].cur_mp = 25;
        ins[i].max_mp = 50;
    }
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(ins, NULL, 4, -1, &r));
    assert(r.champion_count == 4);
    assert(r.champions[0].hp_bar_color == 7);
    assert(r.champions[1].hp_bar_color == 11);
    assert(r.champions[2].hp_bar_color == 8);
    assert(r.champions[3].hp_bar_color == 14);
}

static void test_redraw_with_prev(void)
{
    DM2_V1_ChampionStatInput in = {
        .cur_hp = 50, .max_hp = 100,
        .cur_stamina = 100, .max_stamina = 200,
        .cur_mp = 25, .max_mp = 50,
    };
    DM2_V1_ChampionStatPrev prev = {
        .prev_hp = 50, .prev_max_hp = 100,
        .prev_stamina = 100, .prev_max_stamina = 200,
        .prev_mp = 30, .prev_max_mp = 50,
    };
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(&in, &prev, 1, -1, &r));
    assert(!r.champions[0].redraw_hp);
    assert(!r.champions[0].redraw_stamina);
    assert(r.champions[0].redraw_mana);
}

static void test_gdat_override(void)
{
    DM2_V1_ChampionStatInput in = {
        .cur_hp = 50, .max_hp = 100,
        .cur_stamina = 100, .max_stamina = 100,
        .cur_mp = 25, .max_mp = 50,
    };
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(dm2_v1_champion_stat_bridge_compute(&in, NULL, 1, 9, &r));
    assert(r.champions[0].hp_bar_color == 9);
}

static void test_null_inputs(void)
{
    DM2_V1_ChampionStatBridgeReceipt r;
    assert(!dm2_v1_champion_stat_bridge_compute(NULL, NULL, 1, -1, &r));
    assert(!r.valid);
    assert(!dm2_v1_champion_stat_bridge_compute(NULL, NULL, 0, -1, &r));
}

int main(void)
{
    test_single_hero_full_health();
    test_partial_health();
    test_dead_hero();
    test_mana_exceeds_max();
    test_four_heroes_bar_colors();
    test_redraw_with_prev();
    test_gdat_override();
    test_null_inputs();
    assert(dm2_v1_champion_stat_bridge_source_evidence() != NULL);
    printf("All dm2_v1_champion_stat_bridge tests passed.\n");
    return 0;
}
