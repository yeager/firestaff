/*
 * test_dm2_v1_engage_command_pc34_compat.c — unit tests for the
 * DM2 hand action dispatcher.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_engage_command_pc34_compat.h"
#include "dm2_v1_hero_ops_pc34_compat.h"

static DM2_V1_EngageCommandRequest make_request(int action_id)
{
    DM2_V1_EngageCommandRequest req;
    memset(&req, 0, sizeof(req));
    req.hero_alive = 1;
    req.hero_hp = 100;
    req.hero_max_hp = 200;
    req.hero_mp = 50;
    req.hero_abs_dir = 0;
    req.item_handle = -1;
    req.creature_at_target = -1;
    req.cmd.action_id = (int16_t)action_id;
    req.cmd.delay = 64;
    req.cmd.power_random = 10;
    req.cmd.skill_exp = 5;
    return req;
}

static void test_null_safety(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    int r = dm2_v1_engage_command(NULL, &receipt);
    assert(r == 0);
    assert(receipt.fail_closed == 1);

    r = dm2_v1_engage_command(NULL, NULL);
    assert(r == 0);

    printf("  PASS: null_safety\n");
}

static void test_dead_hero(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(1);
    req.hero_alive = 0;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);
    assert(receipt.hero_dead == 1);

    printf("  PASS: dead_hero\n");
}

static void test_attack(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(2); /* action_id 2 -> case 1 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.attack_queued == 1);
    assert(receipt.action_type == DM2_ENGAGE_ATTACK);
    assert(receipt.cooldown_applied == 64);

    printf("  PASS: attack\n");
}

static void test_attack_min_delay(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(2);
    req.cmd.delay = 10; /* below min 32 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.cooldown_applied == 32);

    printf("  PASS: attack_min_delay\n");
}

static void test_cast_missile(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(3); /* action_id 3 -> case 2 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.spell_cast == 1);
    assert(receipt.fail_closed == 1);

    printf("  PASS: cast_missile\n");
}

static void test_wield_weapon(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(4); /* case 3 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.weapon_wielded == 1);

    printf("  PASS: wield_weapon\n");
}

static void test_confuse(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(5); /* case 4 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.creature_confused == 1);

    printf("  PASS: confuse\n");
}

static void test_light(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(6); /* case 5 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.light_toggled == 1);

    printf("  PASS: light\n");
}

static void test_consume(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(16); /* case 15 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.consumed == 1);

    printf("  PASS: consume\n");
}

static void test_heal_self(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(36); /* case 35 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.healed_self == 1);

    printf("  PASS: heal_self\n");
}

static void test_heal_self_full_hp(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(36);
    req.hero_hp = req.hero_max_hp;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);
    assert(receipt.healed_self == 0);

    printf("  PASS: heal_self_full_hp\n");
}

static void test_heal_self_no_mp(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(36);
    req.hero_mp = 0;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);

    printf("  PASS: heal_self_no_mp\n");
}

static void test_release_minion(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(48); /* case 47 */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.minion_released == 1);

    printf("  PASS: release_minion\n");
}

static void test_epilogue_stamina(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(2); /* attack */
    req.cmd.power_random = 15;
    req.cmd.skill_exp = 8;
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.stamina_cost == 15);
    assert(receipt.skill_exp_gained == 8);

    printf("  PASS: epilogue_stamina\n");
}

/* Mock callbacks for CONFUSE_CREATURE */
static int mock_confuse_attacked;
static uint8_t *mock_confuse_get_record(void *ctx, uint16_t record)
{
    (void)ctx; (void)record;
    return NULL;
}
static uint8_t mock_confuse_get_type(void *ctx, uint16_t record)
{
    (void)ctx; (void)record;
    return 5;
}
static uint16_t mock_confuse_ai_resist(void *ctx, uint8_t creature_type)
{
    (void)ctx; (void)creature_type;
    return 0x0030; /* resistance in bits 4-7 = 3 */
}
static int16_t mock_confuse_rand(void *ctx, int16_t max)
{
    (void)ctx;
    return (int16_t)(max > 0 ? max - 1 : 0);
}
static void mock_confuse_attack(void *ctx, uint16_t target,
    int16_t x, int16_t y, int16_t atype, int16_t sound, int32_t damage)
{
    (void)ctx; (void)target; (void)x; (void)y;
    (void)atype; (void)sound; (void)damage;
    mock_confuse_attacked = 1;
}

static void test_confuse_with_callback(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(5);
    DM2_V1_ConfuseCreatureCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.get_record_address = mock_confuse_get_record;
    cb.get_creature_type = mock_confuse_get_type;
    cb.query_ai_spec_resistance = mock_confuse_ai_resist;
    cb.rand16 = mock_confuse_rand;
    cb.attack_creature = mock_confuse_attack;
    cb.confuser_record = 0;

    req.confuse_cb = &cb;
    req.confuse_ctx = NULL;
    req.confuse_damage = 100;
    mock_confuse_attacked = 0;

    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.creature_confused == 1);
    assert(receipt.fail_closed == 0);
    printf("  PASS: confuse_with_callback\n");
}

/* Mock callbacks for PROCEED_LIGHT */
static int mock_light_timer_queued;
static int16_t mock_global_light;
static const int16_t mock_light_table[] = {0, 10, 20, 30, 40, 50};
static void mock_queue_light_timer(void *ctx, int16_t value, uint32_t fire_tick)
{
    (void)ctx; (void)value; (void)fire_tick;
    mock_light_timer_queued = 1;
}
static void mock_recalc_light(void *ctx)
{
    (void)ctx;
}

static void test_light_with_callback(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(6); /* case 5 */
    DM2_V1_ProceedLightCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    mock_global_light = 20;
    cb.global_light = &mock_global_light;
    cb.light_table = mock_light_table;
    cb.light_table_size = 6;
    cb.game_tick = 100;
    cb.queue_light_timer = mock_queue_light_timer;
    cb.recalc_light = mock_recalc_light;

    req.light_cb = &cb;
    req.light_ctx = NULL;
    mock_light_timer_queued = 0;

    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.light_toggled == 1);
    assert(receipt.fail_closed == 0);
    printf("  PASS: light_with_callback\n");
}

static void test_noop_action(void)
{
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_EngageCommandRequest req = make_request(1); /* case 0 = noop */
    int r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 0);
    assert(receipt.success == 0);

    printf("  PASS: noop_action\n");
}

static uint8_t g_mock_ench_aura;
static int16_t g_mock_ench_power;
static uint16_t g_mock_ench_duration;
static int g_mock_ench_called;

static void mock_proceed_enchantment(void *ctx, uint16_t mask, uint8_t aura,
                                      int16_t power, uint16_t duration)
{
    (void)ctx; (void)mask;
    g_mock_ench_aura = aura;
    g_mock_ench_power = power;
    g_mock_ench_duration = duration;
    g_mock_ench_called = 1;
}

static void test_ability_enchantment(void)
{
    DM2_V1_EngageCommandRequest req;
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_HeroState hero;
    DM2_V1_CastEnchantCallbacks cb;
    int r;

    memset(&hero, 0, sizeof(hero));
    hero.cur_hp = 50; hero.max_hp = 100; hero.cur_mp = 20;
    cb.proceed_enchantment_self = mock_proceed_enchantment;

    /* Case 12 → ability type 4, requires_mp=0 */
    req = make_request(13); /* action_id = case+1 = 13 → case 12 */
    req.cmd.delay = 40;
    req.hero = &hero;
    req.enchant_cb = &cb;
    g_mock_ench_called = 0;

    r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.enchantment_cast == 1);
    assert(!receipt.fail_closed);
    assert(g_mock_ench_called);
    assert(g_mock_ench_aura == 4);
    /* delay = MAX(32,40) << 2 = 160; duration = 160 */
    assert(g_mock_ench_duration == 160);
    /* MP not deducted (requires_mp=0) */
    assert(hero.cur_mp == 20);

    /* Without callback → fail_closed */
    req = make_request(13);
    req.cmd.delay = 40;
    req.hero = &hero;
    req.enchant_cb = NULL;
    r = dm2_v1_engage_command(&req, &receipt);
    assert(receipt.fail_closed);

    printf("  PASS: ability_enchantment\n");
}

static void test_hero_act_enchantment(void)
{
    DM2_V1_EngageCommandRequest req;
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_HeroState hero;
    DM2_V1_CastEnchantCallbacks cb;
    int r;

    memset(&hero, 0, sizeof(hero));
    hero.cur_hp = 50; hero.max_hp = 100; hero.cur_mp = 20;
    cb.proceed_enchantment_self = mock_proceed_enchantment;

    /* Case 33 with timer_type 0x22 → ench_type 0, requires_mp=1 */
    req = make_request(34); /* action_id 34 → case 33 */
    req.cmd.delay = 40;
    req.timer_type = 0x22;
    req.hero = &hero;
    req.enchant_cb = &cb;
    g_mock_ench_called = 0;

    r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.enchantment_cast == 1);
    assert(g_mock_ench_called);
    assert(g_mock_ench_aura == 0);
    /* delay = MAX(32,40) * 3 = 120; duration = 120 */
    assert(g_mock_ench_duration == 120);
    /* MP deducted: 20 - 4 = 16 */
    assert(hero.cur_mp == 16);

    printf("  PASS: hero_act_enchantment\n");
}

static int g_mock_shoot_called;
static uint16_t g_mock_shoot_item_val;

static void mock_shoot_item(void *ctx, uint16_t item, int16_t x, int16_t y,
                             int direction, int facing, uint8_t damage,
                             uint8_t speed, uint8_t power)
{
    (void)ctx; (void)x; (void)y; (void)direction; (void)facing;
    (void)damage; (void)speed; (void)power;
    g_mock_shoot_called = 1;
    g_mock_shoot_item_val = item;
}

static void test_cast_missile_with_callback(void)
{
    DM2_V1_EngageCommandRequest req;
    DM2_V1_EngageCommandReceipt receipt;
    DM2_V1_HeroState hero;
    DM2_V1_ShootMissileCallbacks scb;
    int r;

    memset(&hero, 0, sizeof(hero));
    hero.cur_hp = 50; hero.max_hp = 100; hero.cur_mp = 20;
    memset(&scb, 0, sizeof(scb));
    scb.shoot_item = mock_shoot_item;

    req = make_request(3); /* action_id 3 → case 2 */
    req.cmd.delay = 50;
    req.hero = &hero;
    req.shoot_cb = &scb;
    req.shoot_item = 0x42;
    req.shoot_spell_power = 30;
    req.shoot_mp_cost = 5;
    req.party_x = 10; req.party_y = 12;
    req.hero_abs_dir = 2; req.hero_party_pos = 1;
    g_mock_shoot_called = 0;

    r = dm2_v1_engage_command(&req, &receipt);
    assert(r == 1);
    assert(receipt.spell_cast == 1);
    assert(!receipt.fail_closed);
    assert(g_mock_shoot_called);
    assert(g_mock_shoot_item_val == 0x42);
    assert(hero.cur_mp == 15); /* 20 - 5 */

    /* Insufficient MP → fails, no shoot */
    hero.cur_mp = 3;
    g_mock_shoot_called = 0;
    r = dm2_v1_engage_command(&req, &receipt);
    /* cast returns 0 → skill_exp halved, but success still 1 from dispatch */
    assert(receipt.spell_cast == 1);
    assert(!g_mock_shoot_called);

    printf("  PASS: cast_missile_with_callback\n");
}

int main(void)
{
    printf("test_dm2_v1_engage_command_pc34_compat:\n");
    test_null_safety();
    test_dead_hero();
    test_attack();
    test_attack_min_delay();
    test_cast_missile();
    test_wield_weapon();
    test_confuse();
    test_light();
    test_consume();
    test_heal_self();
    test_heal_self_full_hp();
    test_heal_self_no_mp();
    test_release_minion();
    test_epilogue_stamina();
    test_confuse_with_callback();
    test_light_with_callback();
    test_noop_action();
    test_ability_enchantment();
    test_hero_act_enchantment();
    test_cast_missile_with_callback();
    printf("All engage command tests passed.\n");
    return 0;
}
