/*
 * test_dm2_v1_cloud_pc34_compat.c — unit tests for the DM2 cloud system.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_cloud_pc34_compat.h"

/* ── Mock state ───────────────────────────────────────────────────── */

static uint8_t mock_records[16][8];
static int mock_alloc_next = 0;
static int mock_attack_party_called;
static int16_t mock_attack_party_damage;
static int mock_attack_creature_called;
static int16_t mock_attack_creature_damage;
static int mock_attack_door_called;
static int mock_timer_queued;
static int mock_noise_queued;
static int mock_dealloc_called;
static uint16_t mock_dealloc_record;
static int mock_cut_called;

static void reset_mocks(void)
{
    memset(mock_records, 0, sizeof(mock_records));
    mock_alloc_next = 0;
    mock_attack_party_called = 0;
    mock_attack_party_damage = 0;
    mock_attack_creature_called = 0;
    mock_attack_creature_damage = 0;
    mock_attack_door_called = 0;
    mock_timer_queued = 0;
    mock_noise_queued = 0;
    mock_dealloc_called = 0;
    mock_dealloc_record = 0;
    mock_cut_called = 0;
}

/* ── Mock callbacks ───────────────────────────────────────────────── */

static int16_t mock_alloc(void *ctx, int type)
{
    (void)ctx; (void)type;
    if (mock_alloc_next >= 16) return -1;
    return (int16_t)mock_alloc_next++;
}

static uint8_t *mock_get_record(void *ctx, uint16_t rec)
{
    (void)ctx;
    return mock_records[rec & 0x0F];
}

static int16_t mock_get_next(void *ctx, uint16_t rec)
{
    (void)ctx; (void)rec;
    return (int16_t)0xFFFE; /* end of list */
}

static void mock_dealloc(void *ctx, uint16_t rec)
{
    (void)ctx;
    mock_dealloc_called = 1;
    mock_dealloc_record = rec;
}

static uint16_t mock_tile_record_link(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 0xFFFE;
}

static uint8_t mock_tile_value(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return 0x00; /* corridor */
}

static uint8_t *mock_tile_record(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return mock_records[15];
}

static void mock_append(void *ctx, uint16_t rec, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)x; (void)y;
}

static void mock_cut(void *ctx, uint16_t rec, int16_t x, int16_t y)
{
    (void)ctx; (void)rec; (void)x; (void)y;
    mock_cut_called = 1;
}

static void mock_queue_timer(void *ctx, DM2_V1_CloudTimer *t)
{
    (void)ctx; (void)t;
    mock_timer_queued++;
}

static void mock_attack_party(void *ctx, int16_t damage, int16_t flags, int16_t mode)
{
    (void)ctx; (void)flags; (void)mode;
    mock_attack_party_called = 1;
    mock_attack_party_damage = damage;
}

static void mock_attack_creature(void *ctx, uint16_t rec, int16_t x, int16_t y,
                                  int16_t type, int16_t chance, int16_t damage)
{
    (void)ctx; (void)rec; (void)x; (void)y; (void)type; (void)chance;
    mock_attack_creature_called = 1;
    mock_attack_creature_damage = damage;
}

static void mock_attack_door(void *ctx, int16_t x, int16_t y, int16_t damage,
                              int16_t mode, int16_t extra)
{
    (void)ctx; (void)x; (void)y; (void)damage; (void)mode; (void)extra;
    mock_attack_door_called = 1;
}

static int16_t mock_get_creature_at(void *ctx, int16_t x, int16_t y)
{
    (void)ctx; (void)x; (void)y;
    return -1; /* no creature */
}

static int16_t mock_query_1c9a_0958(void *ctx, uint16_t rec)
{
    (void)ctx; (void)rec;
    return 0;
}

static uint8_t mock_ai_spec[32];
static uint8_t *mock_ai_from_record(void *ctx, uint8_t type)
{
    (void)ctx; (void)type;
    return mock_ai_spec;
}

static uint8_t *mock_ai_from_type(void *ctx, uint16_t type)
{
    (void)ctx; (void)type;
    return mock_ai_spec;
}

static int16_t mock_ai_flags(void *ctx, uint16_t type)
{
    (void)ctx; (void)type;
    return 0;
}

static int16_t mock_poison_resist(void *ctx, uint16_t type, uint16_t damage)
{
    (void)ctx; (void)type;
    return (int16_t)damage; /* no resistance */
}

static void mock_invoke_actuator(void *ctx, uint8_t *rec, int16_t action, int16_t value)
{
    (void)ctx; (void)rec; (void)action; (void)value;
}

static void mock_invoke_message(void *ctx, int16_t x, int16_t y, int16_t dir,
                                 int16_t action, int32_t tick)
{
    (void)ctx; (void)x; (void)y; (void)dir; (void)action; (void)tick;
}

static void mock_noise(void *ctx, uint8_t a, uint8_t b, uint8_t c, uint8_t d,
                        int16_t x, int16_t y, int16_t e, int16_t f, int16_t g)
{
    (void)ctx; (void)a; (void)b; (void)c; (void)d;
    (void)x; (void)y; (void)e; (void)f; (void)g;
    mock_noise_queued++;
}

static int16_t mock_rand16(void *ctx, int16_t max)
{
    (void)ctx;
    return (max > 0) ? 0 : 0;
}

static bool mock_randbit(void *ctx)
{
    (void)ctx;
    return false;
}

static int16_t mock_query_03cf(void *ctx, int16_t *x, int16_t *y, uint16_t dir)
{
    (void)ctx; (void)x; (void)y; (void)dir;
    return -1; /* no next creature */
}

static int16_t mock_min16(int16_t a, int16_t b) { return a < b ? a : b; }
static int16_t mock_max16(int16_t a, int16_t b) { return a > b ? a : b; }

static uint8_t mock_vp_dirty;

static DM2_V1_CloudCallbacks make_callbacks(void)
{
    DM2_V1_CloudCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.ctx = NULL;
    cb.alloc_new_record = mock_alloc;
    cb.get_address_of_record = mock_get_record;
    cb.get_next_record_link = mock_get_next;
    cb.dealloc_record = mock_dealloc;
    cb.get_tile_record_link = mock_tile_record_link;
    cb.get_tile_value = mock_tile_value;
    cb.get_address_of_tile_record = mock_tile_record;
    cb.append_record_to = mock_append;
    cb.cut_record_from = mock_cut;
    cb.queue_timer = mock_queue_timer;
    cb.party_map = 0;
    cb.party_x = 5;
    cb.party_y = 5;
    cb.current_map = 0;
    cb.view_map = 0;
    cb.game_tick = 100;
    mock_vp_dirty = 0;
    cb.viewport_dirty = &mock_vp_dirty;
    cb.attack_party = mock_attack_party;
    cb.attack_creature = mock_attack_creature;
    cb.attack_door = mock_attack_door;
    cb.get_creature_at = mock_get_creature_at;
    cb.query_1c9a_0958 = mock_query_1c9a_0958;
    cb.query_creature_ai_spec_from_record = mock_ai_from_record;
    cb.query_creature_ai_spec_from_type = mock_ai_from_type;
    cb.query_creature_ai_spec_flags = mock_ai_flags;
    cb.apply_creature_poison_resistance = mock_poison_resist;
    cb.invoke_actuator = mock_invoke_actuator;
    cb.invoke_message = mock_invoke_message;
    cb.queue_noise_gen2 = mock_noise;
    cb.rand16 = mock_rand16;
    cb.randbit = mock_randbit;
    cb.query_1c9a_03cf = mock_query_03cf;
    cb.min16 = mock_min16;
    cb.max16 = mock_max16;
    return cb;
}

/* ── Tests ────────────────────────────────────────────────────────── */

static int pass_count = 0;
static int fail_count = 0;

#define CHECK(cond, name) do { \
    if (cond) { printf("  PASS: %s\n", name); pass_count++; } \
    else { printf("  FAIL: %s\n", name); fail_count++; } \
} while (0)

static void test_calc_cloud_damage_no_effect(void)
{
    printf("test_calc_cloud_damage_no_effect\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Cloud type 0 (no effect) — type in low 7 bits of w2 */
    uint8_t *rp = mock_records[0];
    rp[2] = 0x00; /* cloud_type = 0 */
    rp[3] = 0x10; /* strength = 0x10 */

    DM2_V1_CalcCloudDamageReceipt r = dm2_v1_calc_cloud_damage(&cb, 0, 0xFFFF);
    CHECK(r.damage == 0, "cloud type 0 deals no damage");
}

static void test_calc_cloud_damage_party_type2(void)
{
    printf("test_calc_cloud_damage_party_type2\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Cloud type 2 — party-only (bit 2 set in table) */
    uint8_t *rp = mock_records[0];
    rp[2] = 0x02; /* cloud_type = 2 */
    rp[3] = 0x20; /* strength = 0x20 */

    DM2_V1_CalcCloudDamageReceipt r = dm2_v1_calc_cloud_damage(&cb, 0, 0xFFFF);
    CHECK(r.damage == 0x20, "cloud type 2 damages party with base strength");
}

static void test_calc_cloud_damage_out_of_range(void)
{
    printf("test_calc_cloud_damage_out_of_range\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Cloud type >= 8 — out of table range */
    uint8_t *rp = mock_records[0];
    rp[2] = 0x08; /* cloud_type = 8 */
    rp[3] = 0x20;

    DM2_V1_CalcCloudDamageReceipt r = dm2_v1_calc_cloud_damage(&cb, 0, 0xFFFF);
    CHECK(r.damage == 0, "cloud type 8 (out of range) deals no damage");
}

static void test_create_cloud_basic(void)
{
    printf("test_create_cloud_basic\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    DM2_V1_CreateCloudReceipt r = dm2_v1_create_cloud(&cb, 0xFF81, 0x10,
                                                       3, 4, 0xFF);
    CHECK(r.created == true, "cloud created successfully");
    CHECK(r.record_index == 0, "record index is 0");
    CHECK(mock_timer_queued == 1, "timer was queued");
    CHECK(mock_noise_queued == 1, "noise was queued");
}

static void test_create_cloud_alloc_fail(void)
{
    printf("test_create_cloud_alloc_fail\n");
    reset_mocks();
    mock_alloc_next = 16; /* exhaust pool */
    DM2_V1_CloudCallbacks cb = make_callbacks();

    DM2_V1_CreateCloudReceipt r = dm2_v1_create_cloud(&cb, 0xFF81, 0x10,
                                                       3, 4, 0xFF);
    CHECK(r.created == false, "creation fails when pool exhausted");
    CHECK(r.record_index == -1, "record index is -1");
}

static void test_process_cloud_dealloc(void)
{
    printf("test_process_cloud_dealloc\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Set up cloud record with type 0 (will skip to dealloc) */
    uint8_t *rp = mock_records[1];
    rp[2] = 0x00; /* cloud_type = 0 */
    rp[3] = 0x10;

    DM2_V1_CloudTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.value_a = (int16_t)(3 | (4 << 8)); /* x=3, y=4 */
    timer.value_b = 1; /* record index */

    DM2_V1_ProcessCloudReceipt r = dm2_v1_process_cloud(&cb, &timer);
    CHECK(r.deallocated == true, "cloud record deallocated");
    CHECK(r.requeued == false, "timer not requeued");
    CHECK(mock_dealloc_called == 1, "dealloc was called");
    CHECK(mock_cut_called == 1, "cut_record_from was called");
}

static void test_process_cloud_poison_decay(void)
{
    printf("test_process_cloud_poison_decay\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Poison cloud (type 7) with strength 0x10 (>= 6) */
    uint8_t *rp = mock_records[1];
    rp[2] = 0x07; /* cloud_type = 7 */
    rp[3] = 0x10; /* strength = 0x10 */

    DM2_V1_CloudTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.value_a = (int16_t)(3 | (4 << 8));
    timer.value_b = 1;

    DM2_V1_ProcessCloudReceipt r = dm2_v1_process_cloud(&cb, &timer);
    CHECK(r.requeued == true, "poison cloud requeued");
    CHECK(r.deallocated == false, "poison cloud not deallocated");
    CHECK(mock_timer_queued == 1, "timer was requeued");

    /* Verify strength was reduced by 3 */
    CHECK(rp[3] == 0x0D, "strength decayed from 0x10 to 0x0D");
}

static void test_process_cloud_poison_expire(void)
{
    printf("test_process_cloud_poison_expire\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Poison cloud with strength 5 (< 6, will not decay, will expire) */
    uint8_t *rp = mock_records[1];
    rp[2] = 0x07;
    rp[3] = 0x05;

    DM2_V1_CloudTimer timer;
    memset(&timer, 0, sizeof(timer));
    timer.value_a = (int16_t)(3 | (4 << 8));
    timer.value_b = 1;

    DM2_V1_ProcessCloudReceipt r = dm2_v1_process_cloud(&cb, &timer);
    CHECK(r.requeued == false, "weak poison cloud not requeued");
    CHECK(r.deallocated == true, "weak poison cloud deallocated");
}

static void test_cloud_type_table(void)
{
    printf("test_cloud_type_table\n");
    CHECK(dm2_cloud_type_table[0] == 0x00, "type 0 has no flags");
    CHECK(dm2_cloud_type_table[1] == 0x0F, "type 1 has all flags");
    CHECK(dm2_cloud_type_table[2] == 0x04, "type 2 has party-only flag");
    CHECK(dm2_cloud_type_table[7] == 0x0F, "type 7 has all flags");
}

static void test_actuator_scan_empty_tile(void)
{
    printf("test_actuator_scan_empty_tile\n");
    reset_mocks();
    DM2_V1_CloudCallbacks cb = make_callbacks();

    /* Empty tile — get_tile_record_link returns 0xFFFE immediately */
    int32_t result = dm2_v1_cloud_actuator_scan(&cb, 0xFF80, 3, 4, 0x10);
    CHECK(result == (int32_t)0xFFFE, "scan returns end-of-list on empty tile");
}

/* ── Main ─────────────────────────────────────────────────────────── */

int main(void)
{
    printf("=== DM2 Cloud System Tests ===\n\n");

    test_calc_cloud_damage_no_effect();
    test_calc_cloud_damage_party_type2();
    test_calc_cloud_damage_out_of_range();
    test_create_cloud_basic();
    test_create_cloud_alloc_fail();
    test_process_cloud_dealloc();
    test_process_cloud_poison_decay();
    test_process_cloud_poison_expire();
    test_cloud_type_table();
    test_actuator_scan_empty_tile();

    printf("\n=== Results: %d passed, %d failed ===\n", pass_count, fail_count);
    return fail_count > 0 ? 1 : 0;
}
