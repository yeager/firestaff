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
                                      0, 0, 0, 0, 0, 0, &receipt) == 0);
    assert(receipt.fail_closed == 1);

    assert(dm2_v1_actuate_floor_mecha(NULL, NULL, NULL,
                                       0, 0, 0, 0, 0, 0, &receipt) == 0);
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

/* ── Main ──────────────────────────────────────────────────────────── */

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
    printf("All actuator event tests passed.\n");
    return 0;
}
