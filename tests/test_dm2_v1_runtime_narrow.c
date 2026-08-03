/* Tests for dm2_v1_runtime_narrow_pc34_compat — narrow callback stubs for
 * the remaining DM2 skproject timer/hero/creature/item/moverec/record/
 * light functions. */

#include "dm2_v1_runtime_narrow_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- timer category: DM2_PROCESS_TIMER_RESURRECTION ---- */

static int g_resurrect_hero = -1;
static int g_timer_deleted = -1;

static uint8_t tr_get_actor(void *ctx, int idx) { (void)ctx; (void)idx; return 3; }
static int16_t tr_get_value(void *ctx, int idx) { (void)ctx; (void)idx; return 0; }
static void tr_bring_to_life(void *ctx, int hero_idx) { (void)ctx; g_resurrect_hero = hero_idx; }
static void tr_delete_timer(void *ctx, int idx) { (void)ctx; g_timer_deleted = idx; }

static void test_process_timer_resurrection(void)
{
    DM2_V1_TimerResurrectionCallbacks cb = {
        tr_get_actor, tr_get_value, tr_bring_to_life, tr_delete_timer
    };
    dm2_v1_process_timer_resurrection(7, &cb, NULL);
    assert(g_resurrect_hero == 3);
    assert(g_timer_deleted == 7);
    printf("test_process_timer_resurrection OK\n");
}

/* ---- hero category: DM2_WOUND_PLAYER ---- */

static int g_defeated_hero = -1;
static void wp_defeated(void *ctx, int hero_idx) { (void)ctx; g_defeated_hero = hero_idx; }

static void test_wound_player(void)
{
    DM2_V1_WoundPlayerHero hero = { 10, 20, 0 };
    DM2_V1_WoundPlayerCallbacks cb = { wp_defeated };

    int16_t applied = dm2_v1_wound_player(2, &hero, 4, 0, &cb, NULL);
    assert(applied == 4);
    assert(hero.cur_hp == 6);
    assert(g_defeated_hero == -1);

    applied = dm2_v1_wound_player(2, &hero, 100, 0, &cb, NULL);
    assert(applied == 6);
    assert(hero.cur_hp == 0);
    assert(g_defeated_hero == 2);
    printf("test_wound_player OK\n");
}

/* ---- hero category: DM2_ADJUST_SKILLS ---- */

static void test_adjust_skills(void)
{
    int16_t exp[1] = { 0 };
    int16_t level[1] = { 0 };
    static const int16_t thresholds[3] = { 10, 20, 30 };
    DM2_V1_AdjustSkillsState state = { exp, level, thresholds, 3 };

    int32_t lvl = dm2_v1_adjust_skills(0, &state, 15);
    assert(exp[0] == 15);
    assert(lvl == 1);
    assert(level[0] == 1);
    printf("test_adjust_skills OK\n");
}

/* ---- creature category: DM2_WOUND_CREATURE ---- */

static int g_creature_defeated = -1;
static void wc_defeated(void *ctx, int idx) { (void)ctx; g_creature_defeated = idx; }

static void test_wound_creature(void)
{
    DM2_V1_WoundCreatureState creature = { 5 };
    DM2_V1_WoundCreatureCallbacks cb = { wc_defeated };

    int32_t applied = dm2_v1_wound_creature(9, &creature, 3, &cb, NULL);
    assert(applied == 3);
    assert(creature.cur_hp == 2);
    assert(g_creature_defeated == -1);

    applied = dm2_v1_wound_creature(9, &creature, 10, &cb, NULL);
    assert(applied == 2);
    assert(creature.cur_hp == 0);
    assert(g_creature_defeated == 9);
    printf("test_wound_creature OK\n");
}

/* ---- creature category: DM2_CREATURE_ATTACKS_PLAYER ---- */

static int g_wounded_hero = -1;
static int16_t g_wound_amount = 0;

static int cap_roll_hit(void *ctx, int c, int h) { (void)ctx; (void)c; (void)h; return 1; }
static void cap_wound(void *ctx, int hero_idx, int16_t dmg)
{
    (void)ctx;
    g_wounded_hero = hero_idx;
    g_wound_amount = dmg;
}

static void test_creature_attacks_player(void)
{
    DM2_V1_CreatureAttacksPlayerCallbacks cb = { cap_roll_hit, cap_wound };
    int32_t hit = dm2_v1_creature_attacks_player(1, 2, 7, &cb, NULL);
    assert(hit == 1);
    assert(g_wounded_hero == 2);
    assert(g_wound_amount == 7);
    printf("test_creature_attacks_player OK\n");
}

/* ---- item category: DM2_MOVE_ITEM_TO ---- */

static uint16_t g_unlinked_item = 0xFFFF;
static int16_t g_linked_x = -1, g_linked_y = -1;

static void mit_unlink(void *ctx, uint16_t item) { (void)ctx; g_unlinked_item = item; }
static void mit_link(void *ctx, uint16_t item, int16_t x, int16_t y)
{
    (void)ctx; (void)item;
    g_linked_x = x;
    g_linked_y = y;
}

static void test_move_item_to(void)
{
    DM2_V1_MoveItemToCallbacks cb = { mit_unlink, mit_link };
    dm2_v1_move_item_to(42, 5, 6, 0, &cb, NULL);
    assert(g_unlinked_item == 42);
    assert(g_linked_x == 5);
    assert(g_linked_y == 6);
    printf("test_move_item_to OK\n");
}

/* ---- moverec category: DM2_TRY_PUSH_OBJECT_TO ---- */

static int tpo_tile_free(void *ctx, int16_t x, int16_t y) { (void)ctx; (void)x; (void)y; return 1; }
static int g_pushed_record = -1;
static void tpo_move(void *ctx, int32_t record, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    g_pushed_record = record;
}

static void test_try_push_object_to(void)
{
    DM2_V1_TryPushObjectToCallbacks cb = { tpo_tile_free, tpo_move };
    int16_t out_x = -1, out_y = -1;
    int32_t ok = dm2_v1_try_push_object_to(99, 3, 4, &out_x, &out_y, &cb, NULL);
    assert(ok == 1);
    assert(g_pushed_record == 99);
    assert(out_x == 3 && out_y == 4);
    printf("test_try_push_object_to OK\n");
}

/* ---- record category: DM2_RECYCLE_A_RECORD_FROM_THE_WORLD ---- */

static int rr_importance(void *ctx, int idx)
{
    (void)ctx;
    static const int importance[3] = { 5, 1, 9 };
    return importance[idx];
}
static int g_freed_record = -1;
static void rr_free(void *ctx, int idx) { (void)ctx; g_freed_record = idx; }

static void test_recycle_record(void)
{
    DM2_V1_RecycleRecordCallbacks cb = { 3, rr_importance, rr_free };
    int32_t idx = dm2_v1_recycle_a_record_from_the_world(0, &cb, NULL);
    assert(idx == 1);
    assert(g_freed_record == 1);
    printf("test_recycle_record OK\n");
}

/* ---- light category: DM2_CHECK_RECOMPUTE_LIGHT ---- */

static int g_light_dirty = 1;
static int g_recomputed = 0;

static int crl_is_dirty(void *ctx) { (void)ctx; return g_light_dirty; }
static void crl_recompute(void *ctx) { (void)ctx; g_recomputed = 1; }
static void crl_clear(void *ctx) { (void)ctx; g_light_dirty = 0; }

static void test_check_recompute_light(void)
{
    DM2_V1_CheckRecomputeLightCallbacks cb = { crl_is_dirty, crl_recompute, crl_clear };
    int32_t did = dm2_v1_check_recompute_light(&cb, NULL);
    assert(did == 1);
    assert(g_recomputed == 1);
    assert(g_light_dirty == 0);

    did = dm2_v1_check_recompute_light(&cb, NULL);
    assert(did == 0);
    printf("test_check_recompute_light OK\n");
}

/* ---- engage/dispatch category: DM2_ENGAGE_COMMAND ---- */

static int32_t ec_dispatch(void *ctx, int hero_idx, int command_id)
{
    (void)ctx;
    return hero_idx * 100 + command_id;
}

static void test_engage_command(void)
{
    DM2_V1_EngageCommandCallbacks cb = { ec_dispatch };
    int32_t result = dm2_v1_engage_command(2, 5, &cb, NULL);
    assert(result == 205);
    printf("test_engage_command OK\n");
}

int main(void)
{
    test_process_timer_resurrection();
    test_wound_player();
    test_adjust_skills();
    test_wound_creature();
    test_creature_attacks_player();
    test_move_item_to();
    test_try_push_object_to();
    test_recycle_record();
    test_check_recompute_light();
    test_engage_command();
    printf("All dm2_v1_runtime_narrow tests passed.\n");
    return 0;
}
