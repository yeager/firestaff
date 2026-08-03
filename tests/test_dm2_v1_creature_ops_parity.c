/* Tests for DM2 V1 creature ops parity with skproject c_creature.cpp.
 * Covers: CAN_HANDLE_ITEM_IN, CREATURE_ATTACKS_PARTY, KILL_ON_TIMER_POSITION,
 *         CREATURE_SHOOT_ITEM, DM2_1B7D5 (CREATE_CLOUD). */

#include "dm2_v1_creature_ops_pc34_compat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

/* ---- Shared mock state ---- */
static int16_t g_rand16_val;
static int16_t mock_rand16(void *ctx, int16_t max)
{
    (void)ctx; (void)max;
    return g_rand16_val;
}

/* ==== CAN_HANDLE_ITEM_IN tests ==== */

/* Chain: record 0x1400 (type 5, dir 0) -> 0x2400 (type 9, dir 0) -> 0xFFFE */
static int16_t chi_next(void *ctx, uint16_t rw)
{
    (void)ctx;
    if (rw == 0x1400) return (int16_t)0x2400u; /* type 5 -> type 9 */
    return (int16_t)0xFFFEu;
}

static int chi_handle_all(void *ctx, uint16_t rw, int16_t ctype)
{
    (void)ctx; (void)rw; (void)ctype;
    return 1;
}

static int chi_handle_none(void *ctx, uint16_t rw, int16_t ctype)
{
    (void)ctx; (void)rw; (void)ctype;
    return 0;
}

static void test_chi_finds_first_matching(void)
{
    DM2_V1_CreatureHandleCallbacks cb = { chi_next, chi_handle_all };
    int16_t r = dm2_v1_creature_can_handle_item_in(0, 0x1400, 0xFF, &cb, NULL);
    assert((uint16_t)r == 0x1400u);
    printf("  PASS: chi_finds_first_matching\n");
}

static void test_chi_skips_non_item_types(void)
{
    /* Type 2 (db_type=2, not in 5..13 or ==9): record 0x0800 */
    DM2_V1_CreatureHandleCallbacks cb = { chi_next, chi_handle_all };
    /* Chain starts with type 2, which should be skipped */
    int16_t r = dm2_v1_creature_can_handle_item_in(0, (int16_t)0x0800u, 0xFF, &cb, NULL);
    /* Should return 0xFFFE since next after 0x0800 is not in our mock chain */
    assert((uint16_t)r == 0xFFFEu);
    printf("  PASS: chi_skips_non_item_types\n");
}

static void test_chi_direction_filter(void)
{
    DM2_V1_CreatureHandleCallbacks cb = { chi_next, chi_handle_all };
    /* 0x1400 has direction (bits 14-15) = 0. Filter for direction 1 should skip it. */
    int16_t r = dm2_v1_creature_can_handle_item_in(0, 0x1400, 1, &cb, NULL);
    /* Second item 0x2400 also has dir 0, should skip too, end chain */
    assert((uint16_t)r == 0xFFFEu);

    /* Filter for direction 0 should find 0x1400 */
    r = dm2_v1_creature_can_handle_item_in(0, 0x1400, 0, &cb, NULL);
    assert((uint16_t)r == 0x1400u);
    printf("  PASS: chi_direction_filter\n");
}

static void test_chi_empty_chain(void)
{
    DM2_V1_CreatureHandleCallbacks cb = { chi_next, chi_handle_all };
    int16_t r = dm2_v1_creature_can_handle_item_in(0, (int16_t)0xFFFEu, 0xFF, &cb, NULL);
    assert((uint16_t)r == 0xFFFEu);
    printf("  PASS: chi_empty_chain\n");
}

static void test_chi_handle_rejected(void)
{
    DM2_V1_CreatureHandleCallbacks cb = { chi_next, chi_handle_none };
    int16_t r = dm2_v1_creature_can_handle_item_in(0, 0x1400, 0xFF, &cb, NULL);
    assert((uint16_t)r == 0xFFFEu);
    printf("  PASS: chi_handle_rejected\n");
}

/* ==== KILL_ON_TIMER_POSITION tests ==== */
static int g_kill_deleted;
static int g_kill_flag;
static void kill_mock_delete(void *ctx, uint16_t x, uint16_t y, int p1, int p2)
{
    (void)ctx;
    g_kill_deleted = (int)((x << 8) | y);
    assert(p1 == 0);
    assert(p2 == 1);
}

static void test_kill_on_timer_basic(void)
{
    g_kill_deleted = 0;
    g_kill_flag = 0;
    DM2_V1_CreatureKillCallbacks cb = { kill_mock_delete, &g_kill_flag };
    dm2_v1_creature_kill_on_timer_position(10, 15, &cb, NULL);
    assert(g_kill_deleted == ((10 << 8) | 15));
    assert(g_kill_flag == 1);
    printf("  PASS: kill_on_timer_basic\n");
}

static void test_kill_on_timer_null_flag(void)
{
    g_kill_deleted = 0;
    DM2_V1_CreatureKillCallbacks cb = { kill_mock_delete, NULL };
    dm2_v1_creature_kill_on_timer_position(3, 7, &cb, NULL);
    assert(g_kill_deleted == ((3 << 8) | 7));
    printf("  PASS: kill_on_timer_null_flag\n");
}

/* ==== CREATE_CLOUD (DM2_1B7D5) tests ==== */
static int g_cloud_spell_id;
static uint16_t g_cloud_strength;
static uint16_t g_cloud_x, g_cloud_y;
static int g_cloud_param;
static int cloud_create(void *ctx, int spell_id, uint16_t strength,
                        uint16_t x, uint16_t y, int param)
{
    (void)ctx;
    g_cloud_spell_id = spell_id;
    g_cloud_strength = strength;
    g_cloud_x = x;
    g_cloud_y = y;
    g_cloud_param = param;
    return 1;
}

static void test_create_cloud_params(void)
{
    g_rand16_val = 10; /* RAND16(40) returns 10 -> strength = 30 */
    DM2_V1_CreateCloudCallbacks cb = { mock_rand16, cloud_create };
    int r = dm2_v1_creature_create_cloud(5, 8, &cb, NULL);
    assert(r == 1);
    assert(g_cloud_spell_id == (int)0xFF8E);
    assert(g_cloud_strength == 30); /* 10 + 20 */
    assert(g_cloud_x == 5);
    assert(g_cloud_y == 8);
    assert(g_cloud_param == 0xFF);
    printf("  PASS: create_cloud_params\n");
}

static void test_create_cloud_max(void)
{
    g_rand16_val = 39; /* max from RAND16(40) -> strength = 59 */
    DM2_V1_CreateCloudCallbacks cb = { mock_rand16, cloud_create };
    dm2_v1_creature_create_cloud(0, 0, &cb, NULL);
    assert(g_cloud_strength == 59);
    printf("  PASS: create_cloud_max\n");
}

static void test_create_cloud_min(void)
{
    g_rand16_val = 0; /* min -> strength = 20 */
    DM2_V1_CreateCloudCallbacks cb = { mock_rand16, cloud_create };
    dm2_v1_creature_create_cloud(0, 0, &cb, NULL);
    assert(g_cloud_strength == 20);
    printf("  PASS: create_cloud_min\n");
}

/* ==== CREATURE_SHOOT_ITEM tests ==== */
static int16_t g_shoot_find_result;
static int16_t shoot_find(void *ctx, int8_t cap, uint16_t inv, int filter)
{
    (void)ctx; (void)cap; (void)inv; (void)filter;
    return g_shoot_find_result;
}

static uint16_t g_shoot_cut_record;
static void shoot_cut(void *ctx, uint16_t rec, uint16_t *from, int x, int y)
{
    (void)ctx; (void)from; (void)x; (void)y;
    g_shoot_cut_record = rec;
}

static int16_t shoot_between(int16_t lo, int16_t hi, int16_t val)
{
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

static uint16_t g_shot_item;
static uint8_t g_shot_damage;
static int g_shot_type;
static void shoot_item(void *ctx, uint16_t item, uint16_t x, uint16_t y,
                       int dir, int face, uint8_t dmg, uint8_t spd, int type)
{
    (void)ctx; (void)x; (void)y; (void)dir; (void)face; (void)spd;
    g_shot_item = item;
    g_shot_damage = dmg;
    g_shot_type = type;
}

static void test_shoot_item_no_ammo(void)
{
    g_shoot_find_result = (int16_t)0xFFFEu;
    DM2_V1_CreatureShootCallbacks cb = { shoot_find, shoot_cut, mock_rand16,
                                          shoot_between, shoot_item };
    DM2_V1_CreatureShootState st = { 0, 0x1000, 5, 8, 0, 1, 20, 4 };
    int r = dm2_v1_creature_shoot_item(&st, &cb, NULL);
    assert(r == 1); /* No ammo -> fail */
    printf("  PASS: shoot_item_no_ammo\n");
}

static void test_shoot_item_success(void)
{
    g_shoot_find_result = 0x1800; /* found item */
    g_rand16_val = 3;
    DM2_V1_CreatureShootCallbacks cb = { shoot_find, shoot_cut, mock_rand16,
                                          shoot_between, shoot_item };
    DM2_V1_CreatureShootState st = { 0, 0x1000, 5, 8, 2, 1, 40, 4 };
    int r = dm2_v1_creature_shoot_item(&st, &cb, NULL);
    assert(r == 0);
    assert(g_shoot_cut_record == 0x1800u);
    assert(g_shot_item == 0x1800u);
    assert(g_shot_type == 0x08);
    /* Damage: base = 40/4+1=11, +rand(11)=3 -> 14, +rand(14)=3 -> 17
     * between(20,255,17) = 20 */
    assert(g_shot_damage == 20);
    printf("  PASS: shoot_item_success\n");
}

/* ==== CREATURE_ATTACKS_PARTY_FULL tests ==== */
static int16_t cap_abs(int16_t v) { return v < 0 ? (int16_t)-v : v; }
static int16_t g_vector_dir;
static int16_t cap_vector_dir(int16_t x1, int16_t y1, int16_t x2, int16_t y2)
{
    (void)x1; (void)y1; (void)x2; (void)y2;
    return g_vector_dir;
}
static int16_t cap_randdir_val;
static int16_t cap_randdir(void *ctx) { (void)ctx; return cap_randdir_val; }
static int g_randbit_val;
static int cap_randbit(void *ctx) { (void)ctx; return g_randbit_val; }
static int16_t cap_rand_full_val;
static int16_t cap_rand_full(void *ctx) { (void)ctx; return cap_rand_full_val; }
static int16_t cap_min(int16_t a, int16_t b) { return a < b ? a : b; }
static int16_t g_player_at_pos;
static int16_t cap_get_player(void *ctx, uint8_t pos)
{
    (void)ctx; (void)pos;
    return g_player_at_pos;
}
static int16_t g_find_hero_result;
static int16_t cap_find_hero(void *ctx, uint16_t x, uint16_t y, int f)
{
    (void)ctx; (void)x; (void)y; (void)f;
    return g_find_hero_result;
}
static int16_t g_attack_player_result;
static int g_attack_player_hero;
static int16_t cap_attacks_player(void *ctx, uint16_t inv, int16_t hero)
{
    (void)ctx; (void)inv;
    g_attack_player_hero = hero;
    return g_attack_player_result;
}
static uint8_t g_tile_type;
static uint8_t cap_get_tile(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return g_tile_type;
}
static int g_door_attacked;
static int cap_attack_door(void *ctx, int16_t x, int16_t y, int16_t d, int p1, int p2)
{
    (void)ctx; (void)x; (void)y; (void)d; (void)p1; (void)p2;
    g_door_attacked = 1;
    return 0;
}
static int16_t cap_cac(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 0;
}
static int16_t g_creature_weight;
static int16_t cap_weight(void *ctx, uint16_t r)
{
    (void)ctx; (void)r;
    return g_creature_weight;
}
static int g_pushed;
static void cap_push(void *ctx, int16_t x, int16_t y, int16_t d, int f)
{
    (void)ctx; (void)x; (void)y; (void)d; (void)f;
    g_pushed = 1;
}
static int32_t g_last_tick;
static uint8_t g_out_b1c;
static uint8_t g_out_b28[4];
static uint8_t g_out_b29[4];

static DM2_V1_CreatureAttacksPartyCallbacks make_cap_cb(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.abs_fn = cap_abs;
    cb.calc_vector_dir = cap_vector_dir;
    cb.rand16 = mock_rand16;
    cb.randdir = cap_randdir;
    cb.randbit = cap_randbit;
    cb.rand_full = cap_rand_full;
    cb.min_fn = cap_min;
    cb.get_player_at_position = cap_get_player;
    cb.find_hero_at = cap_find_hero;
    cb.creature_attacks_player = cap_attacks_player;
    cb.get_tile_type = cap_get_tile;
    cb.attack_door = cap_attack_door;
    cb.creature_attacks_creature = cap_cac;
    cb.get_creature_weight = cap_weight;
    cb.push_party = cap_push;
    cb.out_last_attack_tick = &g_last_tick;
    cb.out_creature_b_1c = &g_out_b1c;
    cb.out_hero_b_28 = g_out_b28;
    cb.out_hero_b_29 = g_out_b29;
    return cb;
}

static DM2_V1_CreatureAttacksPartyState make_cap_state(void)
{
    DM2_V1_CreatureAttacksPartyState st;
    memset(&st, 0, sizeof(st));
    st.spx_word_0e = 0x0100; /* direction bits: (0x0100 << 6) >> 14 = 1 */
    st.creature_x = 5;
    st.creature_y = 5;
    st.target_x = 5;
    st.target_y = 5;
    st.current_map = 1;
    st.party_map = 1;
    st.party_x = 5;
    st.party_y = 5;
    st.gametick = 100;
    st.heros_in_party = 2;
    st.hero_curHP[0] = 50;
    st.hero_curHP[1] = 30;
    st.creature_b_1c = 0;
    return st;
}

static void test_cap_null_safety(void)
{
    assert(dm2_v1_creature_attacks_party_full(NULL, NULL, NULL) == 1);
    printf("  PASS: cap_null_safety\n");
}

static void test_cap_too_far(void)
{
    /* Creature at (5,5), target at (5,8) — Manhattan dist 3 > 1 */
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    st.creature_x = 5;
    st.creature_y = 5;
    st.target_x = 5;
    st.target_y = 8;
    int r = dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(r == 1);
    printf("  PASS: cap_too_far\n");
}

static void test_cap_same_tile_attacks_hero(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    g_player_at_pos = 0;
    g_find_hero_result = 0;
    g_attack_player_result = 5;
    g_vector_dir = 0;
    g_rand16_val = 0;
    memset(g_out_b29, 0, 4);
    int r = dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(r == 0);
    assert(g_last_tick == 100);
    printf("  PASS: cap_same_tile_attacks_hero\n");
}

static void test_cap_no_alive_heroes(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    st.hero_curHP[0] = 0;
    st.hero_curHP[1] = 0;
    int r = dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(r == 1);
    printf("  PASS: cap_no_alive_heroes\n");
}

static void test_cap_not_at_party_door(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    /* Party not at target */
    st.party_x = 10;
    g_tile_type = 4; /* door */
    g_door_attacked = 0;
    g_rand16_val = 5;
    (void)dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    /* attack_door returns 0, then creature_attacks_creature returns 0 */
    assert(g_door_attacked == 1);
    printf("  PASS: cap_not_at_party_door\n");
}

static void test_cap_push_heavy(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    st.creature_b_1a = 0x26;
    /* Creature adjacent, party at target, attack succeeds, then push */
    st.creature_x = 4;
    st.creature_y = 5;
    st.ai_byte_0 = 0; /* no direction check flag */
    g_vector_dir = 0;
    g_player_at_pos = 0;
    g_find_hero_result = 0;
    g_attack_player_result = 5;
    g_rand16_val = 0;
    memset(g_out_b29, 0, 4);
    g_creature_weight = 200; /* > 0x64 */
    cap_rand_full_val = 0x0F; /* & 0xF != 0 -> skip randdir */
    g_pushed = 0;
    dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(g_pushed == 1);
    printf("  PASS: cap_push_heavy\n");
}

static void test_cap_push_light_randdir_zero(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    st.creature_b_1a = 0x26;
    st.creature_x = 4;
    st.creature_y = 5;
    st.ai_byte_0 = 0;
    g_vector_dir = 0;
    g_player_at_pos = 0;
    g_find_hero_result = 0;
    g_attack_player_result = 5;
    g_rand16_val = 0;
    memset(g_out_b29, 0, 4);
    g_creature_weight = 50; /* <= 0x64 */
    cap_randdir_val = 0; /* randdir == 0 -> return 0, no push */
    g_pushed = 0;
    int r = dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(r == 0);
    assert(g_pushed == 0);
    printf("  PASS: cap_push_light_randdir_zero\n");
}

static void test_cap_direction_mismatch_with_flag(void)
{
    /* Adjacent, wrong direction, AI bit 2 set -> bail */
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    st.creature_x = 5;
    st.creature_y = 4; /* adjacent to target 5,5 */
    st.ai_byte_0 = 0x04; /* bit 2 set */
    g_vector_dir = 2; /* dir to target = 2 */
    st.spx_word_0e = 0x0000; /* facing dir = 0 */
    int r = dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(r == 1);
    printf("  PASS: cap_direction_mismatch_with_flag\n");
}

static void test_cap_type6_direction_wrap(void)
{
    DM2_V1_CreatureAttacksPartyCallbacks cb = make_cap_cb();
    DM2_V1_CreatureAttacksPartyState st = make_cap_state();
    st.creature_x = 5;
    st.creature_y = 4;
    st.creature_b_20 = 6; /* type 6 -> direction wraps +2 */
    st.ai_byte_0 = 0x04;
    /* SPX word 0x0E = 0x0000 -> facing = 0, wrapped = (0+2)&3 = 2 */
    st.spx_word_0e = 0x0000;
    g_vector_dir = 2; /* dir to target = 2, matches wrapped */
    /* Should NOT bail on direction mismatch */
    g_player_at_pos = 0;
    g_find_hero_result = 0;
    g_attack_player_result = 5;
    g_rand16_val = 0;
    memset(g_out_b29, 0, 4);
    int r = dm2_v1_creature_attacks_party_full(&st, &cb, NULL);
    assert(r == 0);
    printf("  PASS: cap_type6_direction_wrap\n");
}

int main(void)
{
    printf("test_dm2_v1_creature_ops_parity:\n");

    /* CAN_HANDLE_ITEM_IN */
    test_chi_finds_first_matching();
    test_chi_skips_non_item_types();
    test_chi_direction_filter();
    test_chi_empty_chain();
    test_chi_handle_rejected();

    /* KILL_ON_TIMER_POSITION */
    test_kill_on_timer_basic();
    test_kill_on_timer_null_flag();

    /* CREATE_CLOUD (DM2_1B7D5) */
    test_create_cloud_params();
    test_create_cloud_max();
    test_create_cloud_min();

    /* CREATURE_SHOOT_ITEM */
    test_shoot_item_no_ammo();
    test_shoot_item_success();

    /* CREATURE_ATTACKS_PARTY_FULL */
    test_cap_null_safety();
    test_cap_too_far();
    test_cap_same_tile_attacks_hero();
    test_cap_no_alive_heroes();
    test_cap_not_at_party_door();
    test_cap_push_heavy();
    test_cap_push_light_randdir_zero();
    test_cap_direction_mismatch_with_flag();
    test_cap_type6_direction_wrap();

    printf("All creature_ops_parity tests passed.\n");
    return 0;
}
