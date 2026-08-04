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
static int16_t g_accumulated_damage = 0;

static int wp_hero_exists(void *ctx, int hero_idx) { (void)ctx; (void)hero_idx; return 1; }
static int wp_is_sleeping(void *ctx) { (void)ctx; return 0; }
static int32_t wp_hero_defense_pct(void *ctx, int hero_idx, uint32_t body_mask, int is_used)
{ (void)ctx; (void)hero_idx; (void)body_mask; (void)is_used; return 0; }
static int wp_random(void *ctx, int max) { (void)ctx; (void)max; return 0; }
static int16_t wp_skill_lv(void *ctx, int hero_idx, int sg, int wb)
{ (void)ctx; (void)hero_idx; (void)sg; (void)wb; return 0; }
static void wp_resume(void *ctx) { (void)ctx; }
static void wp_add_body_status(void *ctx, int hero_idx, uint16_t s)
{ (void)ctx; (void)hero_idx; (void)s; }
static void wp_accumulate(void *ctx, int hero_idx, int16_t wound)
{ (void)ctx; (void)hero_idx; g_accumulated_damage += wound; }
static void wp_defeated(void *ctx, int hero_idx) { (void)ctx; g_defeated_hero = hero_idx; }

static void test_wound_player(void)
{
    DM2_V1_WoundPlayerHero hero = { 10, 0 };
    DM2_V1_WoundPlayerCallbacks cb = {
        wp_hero_exists, wp_is_sleeping, wp_hero_defense_pct, wp_random,
        wp_skill_lv, wp_resume, wp_add_body_status, wp_accumulate
    };

    g_accumulated_damage = 0;
    int16_t applied = dm2_v1_wound_player(2, &hero, 4, 0, &cb, NULL);
    assert(applied == 4);
    assert(g_accumulated_damage == 4);
    assert(g_defeated_hero == -1);

    g_accumulated_damage = 0;
    applied = dm2_v1_wound_player(2, &hero, 100, 0, &cb, NULL);
    assert(applied == 100);
    assert(g_accumulated_damage == 100);
    printf("test_wound_player OK\n");
}

/* ---- hero category: DM2_ADJUST_SKILLS ---- */

static int32_t g_added_exp = 0;
static int g_marked_dirty = -1;

static int16_t as_get_gametick(void *ctx) { (void)ctx; return 0; }
static int16_t as_get_exp_window_tick(void *ctx) { (void)ctx; return 0; }
static int16_t as_get_exp_scale(void *ctx) { (void)ctx; return 1; }
static int g_skill_lv_call = 0;
static int16_t as_get_skill_lv(void *ctx, int hi, int sg, int wb)
{ (void)ctx; (void)hi; (void)sg; (void)wb; return (g_skill_lv_call++ == 0) ? 1 : 2; }
static int16_t as_get_skill_exp(void *ctx, int hi, int sid)
{ (void)ctx; (void)hi; (void)sid; return 0; }
static void as_add_skill_exp(void *ctx, int hi, int sid, int32_t amount)
{ (void)ctx; (void)hi; (void)sid; g_added_exp += amount; }
static int as_random_bit(void *ctx) { (void)ctx; return 0; }
static int16_t as_get_max_hp(void *ctx, int hi) { (void)ctx; (void)hi; return 100; }
static void as_set_max_hp(void *ctx, int hi, int16_t v) { (void)ctx; (void)hi; (void)v; }
static int16_t as_get_max_stamina(void *ctx, int hi) { (void)ctx; (void)hi; return 100; }
static void as_set_max_stamina(void *ctx, int hi, int16_t v) { (void)ctx; (void)hi; (void)v; }
static int16_t as_get_max_mp(void *ctx, int hi) { (void)ctx; (void)hi; return 100; }
static void as_set_max_mp(void *ctx, int hi, int16_t v) { (void)ctx; (void)hi; (void)v; }
static void as_add_ability_max(void *ctx, int hi, int a, int16_t d)
{ (void)ctx; (void)hi; (void)a; (void)d; }
static void as_mark_dirty(void *ctx, int hi) { (void)ctx; g_marked_dirty = hi; }
static void as_display_level_up(void *ctx, int hi, int sg)
{ (void)ctx; (void)hi; (void)sg; }

static void test_adjust_skills(void)
{
    DM2_V1_AdjustSkillsCallbacks cb = {
        as_get_gametick, as_get_exp_window_tick, as_get_exp_scale,
        as_get_skill_lv, as_get_skill_exp, as_add_skill_exp,
        as_random_bit, as_get_max_hp, as_set_max_hp,
        as_get_max_stamina, as_set_max_stamina,
        as_get_max_mp, as_set_max_mp,
        as_add_ability_max, as_mark_dirty, as_display_level_up
    };

    g_added_exp = 0;
    g_marked_dirty = -1;
    g_skill_lv_call = 0;
    /* Use skill group 0 (non-combat) to avoid time-window halving/doubling.
     * Mock returns level 1 on first call, 2 on second → triggers level-up loop. */
    int32_t lvl = dm2_v1_adjust_skills(0, 0, 15, &cb, NULL);
    assert(g_added_exp == 15);
    assert(lvl == 2);
    assert(g_marked_dirty == 0);
    printf("test_adjust_skills OK\n");
}

/* ---- creature category: DM2_WOUND_CREATURE ---- */

static int g_creature_defeated = -1;

static void wc_defeated(void *ctx, int idx) { (void)ctx; g_creature_defeated = idx; }
static void wc_notify(void *ctx, int idx) { (void)ctx; (void)idx; }
static int wc_can_die(void *ctx, int idx) { (void)ctx; (void)idx; return 1; }
static void wc_set_timeout(void *ctx, int idx, int t) { (void)ctx; (void)idx; (void)t; }
static void wc_play_death(void *ctx, int idx) { (void)ctx; (void)idx; }
static int wc_random(void *ctx, int max) { (void)ctx; (void)max; return 1; }
static void wc_flee(void *ctx, int idx) { (void)ctx; (void)idx; }

static void test_wound_creature(void)
{
    DM2_V1_WoundCreatureState creature = { 5, 10, 0, 1, 0 };
    DM2_V1_WoundCreatureCallbacks cb = {
        wc_notify, wc_can_die, wc_set_timeout, wc_play_death,
        wc_defeated, wc_random, wc_flee
    };

    g_creature_defeated = -1;
    int32_t result = dm2_v1_wound_creature(9, &creature, 3, &cb, NULL);
    assert(result == 0);
    assert(creature.cur_hp == 2);
    assert(g_creature_defeated == -1);

    result = dm2_v1_wound_creature(9, &creature, 10, &cb, NULL);
    assert(creature.cur_hp == 0);
    assert(g_creature_defeated == 9);
    printf("test_wound_creature OK\n");
}

/* ---- creature category: DM2_CREATURE_ATTACKS_PLAYER ---- */

static int g_wounded_hero = -1;
static int16_t g_wound_amount = 0;

static int cap_get_profile(void *ctx, int ci, DM2_V1_CreatureAttackProfile *out)
{
    (void)ctx; (void)ci;
    out->to_hit_base = 100;
    out->attack_type = 0;
    out->max_damage = 50;
    out->poison_chance = 0;
    out->is_surprise_attack = 1;
    return 1;
}
static int cap_get_dex(void *ctx, int hi) { (void)ctx; (void)hi; return 10; }
static int cap_use_luck(void *ctx, int hi, int c) { (void)ctx; (void)hi; (void)c; return 0; }
static int cap_fight_skill(void *ctx, int hi) { (void)ctx; (void)hi; return 5; }
static int cap_random(void *ctx, int max) { (void)ctx; (void)max; return 7; }
static int cap_rand_bit(void *ctx) { (void)ctx; return 0; }
static int cap_rand_dir(void *ctx) { (void)ctx; return 0; }
static int16_t cap_wound(void *ctx, int hero_idx, int16_t dmg)
{
    (void)ctx;
    g_wounded_hero = hero_idx;
    g_wound_amount = dmg;
    return dmg;
}
static void cap_hit_noise(void *ctx, int ci, int hi, int16_t d)
{ (void)ctx; (void)ci; (void)hi; (void)d; }
static int cap_resist(void *ctx, int hi, int16_t pc) { (void)ctx; (void)hi; (void)pc; return 0; }
static void cap_poison(void *ctx, int hi, int a) { (void)ctx; (void)hi; (void)a; }
static void cap_resume(void *ctx) { (void)ctx; }

static void test_creature_attacks_player(void)
{
    DM2_V1_CreatureAttacksPlayerCallbacks cb = {
        cap_get_profile, cap_get_dex, cap_use_luck, cap_fight_skill,
        cap_random, cap_rand_bit, cap_rand_dir,
        cap_wound, cap_hit_noise, cap_resist, cap_poison, cap_resume
    };
    g_wounded_hero = -1;
    g_wound_amount = 0;
    int32_t hit = dm2_v1_creature_attacks_player(1, 2, &cb, NULL);
    assert(hit > 0);
    assert(g_wounded_hero == 2);
    assert(g_wound_amount == hit);
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

static int ec_is_dead(void *ctx, int hi) { (void)ctx; (void)hi; return 0; }
static int ec_resolve(void *ctx, int hi, int raw)
{ (void)ctx; (void)hi; return raw; }
static int32_t ec_dispatch(void *ctx, int hero_idx, int cmd, int alt)
{
    (void)ctx; (void)alt;
    return hero_idx * 100 + cmd;
}

static void test_engage_command(void)
{
    DM2_V1_EngageCommandCallbacks cb = { ec_is_dead, ec_resolve, ec_dispatch };
    int32_t result = dm2_v1_engage_command_narrow(2, 5, &cb, NULL);
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
