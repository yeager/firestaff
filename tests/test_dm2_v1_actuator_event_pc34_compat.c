/*
 * test_dm2_v1_actuator_event_pc34_compat.c — unit tests for the DM2
 * actuator event dispatch (ACTUATE_WALL_MECHA / ACTUATE_FLOOR_MECHA).
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_actuator_event_pc34_compat.h"
#include "dm2_v1_record_pool_pc34_compat.h"
#include "dm2_v1_timeline.h"
#include "dm2_v1_tile_record_walk_pc34_compat.h"
#include "dm2_v1_dungeon_loader.h"

/* ── Accessor tests ────────────────────────────────────────────────── */

static void test_actu_accessors(void)
{
    uint8_t r[8];
    memset(r, 0, sizeof(r));

    /* w2: type = 0x20 (relay_1), data = 0x1FF */
    r[2] = 0x20 | ((0x1FF & 0x01) << 7);
    r[3] = (uint8_t)((0x1FF >> 1) & 0xFF);
    assert(dm2_actu_type(r) == 0x20);
    assert(dm2_actu_data(r) == 0x1FF);

    /* set_data round-trip */
    dm2_actu_set_data(r, 42);
    assert(dm2_actu_data(r) == 42);
    assert(dm2_actu_type(r) == 0x20);

    /* w4 fields */
    memset(r, 0, sizeof(r));
    dm2_actu_set_active_status(r, 1);
    assert(dm2_actu_active_status(r) == 1);
    dm2_actu_set_once_only(r, 1);
    assert(dm2_actu_once_only(r) == 1);
    assert(dm2_actu_active_status(r) == 1);

    /* action_type bits 3-4 */
    r[4] = (r[4] & ~(3 << 3)) | (2 << 3);
    assert(dm2_actu_action_type(r) == 2);

    /* delay bits 7-10 of w4 (spans bytes 4 and 5) */
    {
        uint16_t w4 = dm2_actu_w4(r);
        w4 = (w4 & ~(0xFu << 7)) | (5u << 7);
        dm2_actu_set_w4(r, w4);
    }
    assert(dm2_actu_delay(r) == 5);

    /* w6: direction, xcoord, ycoord */
    memset(r, 0, sizeof(r));
    r[6] = (3 << 4) | (0x1F << 6);
    r[7] = (0x1F << 6) >> 8 | (0x1F << 3);
    assert(dm2_actu_direction(r) == 3);

    printf("  PASS: actu_accessors\n");
}

static void test_toggle_message(void)
{
    assert(dm2_toggle_actuator_message(DM2_ACTMSG_OPEN_SET, 0) == 1);
    assert(dm2_toggle_actuator_message(DM2_ACTMSG_CLOSE_CLEAR, 1) == 0);
    assert(dm2_toggle_actuator_message(DM2_ACTMSG_TOGGLE, 0) == 1);
    assert(dm2_toggle_actuator_message(DM2_ACTMSG_TOGGLE, 1) == 0);
    printf("  PASS: toggle_message\n");
}

/* ── INVOKE_ACTUATOR test ──────────────────────────────────────────── */

static void test_invoke_actuator(void)
{
    DM2_V1_SourceTimerQueue queue;
    uint8_t actu[8];

    dm2_v1_source_timer_queue_init(&queue);
    memset(actu, 0, sizeof(actu));

    /* Set target: x=10, y=15, direction=2 */
    uint16_t w6 = (2 << 4) | (10 << 6) | (15 << 11);
    actu[6] = (uint8_t)(w6 & 0xFF);
    actu[7] = (uint8_t)(w6 >> 8);

    assert(dm2_v1_invoke_actuator(&queue, actu, DM2_ACTMSG_TOGGLE, 5, 0, 100) == 1);
    assert(queue.count == 1);
    assert(queue.timers[0].type == 0x04);

    printf("  PASS: invoke_actuator\n");
}

/* ── ACTIVATE_TICK_GENERATOR test ──────────────────────────────────── */

static void test_activate_tick_generator(void)
{
    DM2_V1_SourceTimerQueue queue;
    uint8_t actu[8];

    dm2_v1_source_timer_queue_init(&queue);
    memset(actu, 0, sizeof(actu));

    /* type = TICK_GENERATOR (0x1e), data = 10 */
    actu[2] = 0x1E;
    dm2_actu_set_data(actu, 10);

    int16_t handle = 0x0C05; /* db3, index 5 */
    assert(dm2_v1_activate_tick_generator(&queue, actu, handle, 0, 1000) == 1);
    assert(queue.count == 1);
    assert(queue.timers[0].type == DM2_V1_TIMER_TICK_GENERATOR);
    assert(queue.timers[0].value_a == handle);

    /* data = 0 should NOT queue */
    dm2_v1_source_timer_queue_init(&queue);
    dm2_actu_set_data(actu, 0);
    assert(dm2_v1_activate_tick_generator(&queue, actu, handle, 0, 1000) == 1);
    assert(queue.count == 0);

    printf("  PASS: activate_tick_generator\n");
}

/* ── ACTIVATE_RELAY1 test ──────────────────────────────────────────── */

static void test_activate_relay1(void)
{
    DM2_V1_SourceTimerQueue queue;
    uint8_t actu[8];

    dm2_v1_source_timer_queue_init(&queue);
    memset(actu, 0, sizeof(actu));

    /* Set data=50, delay=3, target x=5,y=8,dir=1 */
    actu[2] = DM2_ACTU_RELAY_1;
    dm2_actu_set_data(actu, 50);
    /* delay bits 7-10 of w4 */
    {
        uint16_t w4 = dm2_actu_w4(actu);
        w4 = (w4 & ~(0xFu << 7)) | (3u << 7);
        dm2_actu_set_w4(actu, w4);
    }
    /* target coords in w6 */
    uint16_t w6 = (1 << 4) | (5 << 6) | (8 << 11);
    actu[6] = (uint8_t)(w6 & 0xFF);
    actu[7] = (uint8_t)(w6 >> 8);

    assert(dm2_v1_activate_relay1(NULL, NULL, &queue, actu,
                                   DM2_ACTMSG_OPEN_SET, 0,
                                   0, 1000) == 1);
    assert(queue.count == 1);
    assert(queue.timers[0].type == 0x04);

    /* Once-only guard: set once_only=1, revert=0, action=1 -> should block */
    dm2_v1_source_timer_queue_init(&queue);
    dm2_actu_set_once_only(actu, 1);
    assert(dm2_v1_activate_relay1(NULL, NULL, &queue, actu,
                                   1, 0, 0, 1000) == 1);
    assert(queue.count == 0);

    printf("  PASS: activate_relay1\n");
}

/* ── NULL safety tests ─────────────────────────────────────────────── */

static void test_null_safety(void)
{
    DM2_V1_ActuatorEventReceipt receipt;

    assert(dm2_v1_actuate_wall_mecha(NULL, NULL, NULL,
                                      0, 0, 0, 0, 0, 0, NULL, 0, &receipt) == 0);
    assert(receipt.fail_closed == 1);

    assert(dm2_v1_actuate_floor_mecha(NULL, NULL, NULL,
                                       0, 0, 0, 0, 0, 0, NULL, 0, &receipt) == 0);
    assert(receipt.fail_closed == 1);

    assert(dm2_v1_invoke_actuator(NULL, NULL, 0, 0, 0, 0) == 0);
    assert(dm2_v1_activate_tick_generator(NULL, NULL, 0, 0, 0) == 0);

    printf("  PASS: null_safety\n");
}

/* ── Handle helpers ────────────────────────────────────────────────── */

static void test_handle_helpers(void)
{
    /* DB type from handle: bits 10-13 */
    int16_t h = (int16_t)((3 << 10) | 42); /* db3 (actuator), index 42 */
    assert(dm2_handle_db_type(h) == 3);
    assert(dm2_handle_dir(h) == (42 & 3));

    printf("  PASS: handle_helpers\n");
}

/* ── New actuator type constants ──────────────────────────────────── */

static void test_new_actuator_types(void)
{
    assert(DM2_ACTU_CROSS_SCENE == 0x27);
    assert(DM2_ACTU_CREATURE_AI_STATE == 0x28);
    assert(DM2_ACTU_CREATURE_ANIMATOR == 0x3A);
    assert(DM2_ACTU_CREATURE_DIRECTION == 0x42);

    /* TOGGLE_ACTUATOR_MESSAGE on an actuator record's once_only field:
     * this is what CROSS_SCENE does via dm2_toggle_actuator_message */
    uint8_t rec[8];
    memset(rec, 0, sizeof(rec));
    rec[2] = DM2_ACTU_CROSS_SCENE;

    dm2_actu_set_once_only(rec, 0);
    assert(dm2_actu_once_only(rec) == 0);
    dm2_actu_set_once_only(rec,
        dm2_toggle_actuator_message(0, dm2_actu_once_only(rec)));
    assert(dm2_actu_once_only(rec) == 1);

    dm2_actu_set_once_only(rec,
        dm2_toggle_actuator_message(2, dm2_actu_once_only(rec)));
    assert(dm2_actu_once_only(rec) == 0);

    dm2_actu_set_once_only(rec, 1);
    dm2_actu_set_once_only(rec,
        dm2_toggle_actuator_message(1, dm2_actu_once_only(rec)));
    assert(dm2_actu_once_only(rec) == 0);

    printf("  PASS: new_actuator_types\n");
}

/* ── Main ──────────────────────────────────────────────────────────── */

/* ── Creature killer mock callbacks ───────────────────────────────── */

typedef struct {
    int get_creature_calls;
    int type_match_calls;
    int set_ai_calls;
    int attack_calls;
    uint16_t last_creature_rec;
    int32_t last_damage;
} CKMockCtx;

static uint16_t ck_mock_get_creature(void *ctx, int16_t x, int16_t y) {
    CKMockCtx *m = (CKMockCtx *)ctx;
    m->get_creature_calls++;
    if (x == 3 && y == 3) return 42;
    return 0xFFFFu;
}

static int ck_mock_type_matches(void *ctx, uint16_t rec, uint16_t type) {
    CKMockCtx *m = (CKMockCtx *)ctx;
    m->type_match_calls++;
    (void)type;
    return (rec == 42) ? 1 : 0;
}

static void ck_mock_set_ai(void *ctx, uint16_t rec,
                            int16_t x, int16_t y, uint8_t state, int32_t flag) {
    CKMockCtx *m = (CKMockCtx *)ctx;
    m->set_ai_calls++;
    m->last_creature_rec = rec;
    (void)x; (void)y; (void)state; (void)flag;
}

static void ck_mock_attack(void *ctx, uint16_t rec,
                            int16_t x, int16_t y, int32_t damage,
                            int32_t threshold, int32_t flag) {
    CKMockCtx *m = (CKMockCtx *)ctx;
    m->attack_calls++;
    m->last_creature_rec = rec;
    m->last_damage = damage;
    (void)x; (void)y; (void)threshold; (void)flag;
}

static void test_creature_killer(void)
{
    DM2_V1_CreatureKillerReceipt receipt;
    CKMockCtx mock;
    DM2_V1_CreatureKillerCallbacks cb;
    int ret;

    printf("  creature_killer: ");

    /* NULL safety */
    ret = dm2_v1_activate_creature_killer(NULL, 0, 0, 0, 0, 0, 0, 0, 0, NULL);
    assert(ret == 0);

    memset(&receipt, 0, sizeof(receipt));
    ret = dm2_v1_activate_creature_killer(NULL, 0, 0, 0, 0, 0, 0, 0, 0, &receipt);
    assert(ret == 0);
    assert(receipt.valid == 0);

    /* Set up callbacks */
    memset(&mock, 0, sizeof(mock));
    memset(&cb, 0, sizeof(cb));
    cb.ctx = &mock;
    cb.map_width = 10;
    cb.map_height = 10;
    cb.get_creature_at = ck_mock_get_creature;
    cb.creature_type_matches = ck_mock_type_matches;
    cb.set_creature_ai_state = ck_mock_set_ai;
    cb.attack_creature = ck_mock_attack;

    /* Type 0x0b (CREATURE_KILLER) with actu_data_lo=2: set AI state */
    memset(&mock, 0, sizeof(mock));
    /* actu at (3,3), timer at (3,3) -> 1x1 area scanning just (3,3) */
    ret = dm2_v1_activate_creature_killer(&cb, 2, 0, 3, 3, 3, 3, 0x0b, 0, &receipt);
    assert(ret == 1);
    assert(receipt.valid == 1);
    assert(receipt.cells_scanned == 1);
    assert(receipt.creatures_found == 1);
    assert(mock.set_ai_calls == 1);
    assert(mock.last_creature_rec == 42);

    /* Type 0x28 (CREATURE_AI_STATE): attack creature */
    memset(&mock, 0, sizeof(mock));
    ret = dm2_v1_activate_creature_killer(&cb, 5, 0, 3, 3, 3, 3, 0x28, 1, &receipt);
    assert(ret == 1);
    assert(receipt.valid == 1);
    assert(mock.attack_calls == 1);
    assert(mock.last_damage == (5 | 0x8000)); /* action_type != 0 -> OR 0x8000 */

    /* Type 0x0b with actu_data_lo=0: skip (continue, no AI state set) */
    memset(&mock, 0, sizeof(mock));
    ret = dm2_v1_activate_creature_killer(&cb, 0, 0, 3, 3, 3, 3, 0x0b, 0, &receipt);
    assert(ret == 1);
    assert(mock.set_ai_calls == 0);

    /* Type 0x0b with actu_data_lo=1: skip (continue, no AI state set) */
    memset(&mock, 0, sizeof(mock));
    ret = dm2_v1_activate_creature_killer(&cb, 1, 0, 3, 3, 3, 3, 0x0b, 0, &receipt);
    assert(ret == 1);
    assert(mock.set_ai_calls == 0);

    /* Type 0x0b with actu_data_lo=3: early return (unknown value) */
    memset(&mock, 0, sizeof(mock));
    ret = dm2_v1_activate_creature_killer(&cb, 3, 0, 3, 3, 3, 3, 0x0b, 0, &receipt);
    assert(ret == 1);
    assert(mock.set_ai_calls == 0);

    printf("PASS\n");
}

int main(void)
{
    printf("test_dm2_v1_actuator_event_pc34_compat:\n");
    test_actu_accessors();
    test_toggle_message();
    test_invoke_actuator();
    test_activate_tick_generator();
    test_activate_relay1();
    test_null_safety();
    test_handle_helpers();
    test_new_actuator_types();
    test_creature_killer();
    printf("All actuator event tests passed.\n");
    return 0;
}
