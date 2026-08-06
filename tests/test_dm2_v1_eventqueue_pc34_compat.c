/*
 * test_dm2_v1_eventqueue_pc34_compat.c — unit tests for DM2 event queue.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_eventqueue_pc34_compat.h"

static void test_init(void)
{
    DM2_V1_EventQueue eq;
    memset(&eq, 0xFF, sizeof(eq));
    dm2_v1_eventqueue_init(&eq);
    assert(eq.idx == 0);
    assert(eq.out_idx == 0);
    assert(eq.entries == 0);
    assert(!eq.fetch_busy);
    assert(!eq.singleevent_available);
    assert(eq.event_heroidx == 0);
    assert(eq.event_unk05 == 0);
    assert(eq.event_unk09 == 0);
    assert(!eq.event_unk0f);
    printf("  PASS test_init\n");
}

static void test_set(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    dm2_v1_eventqueue_set(&eq, 3, 100, 200, 0x01);
    assert(eq.idx == 3);
    assert(eq.data[3].x == 100);
    assert(eq.data[3].y == 200);
    assert(eq.data[3].b == 0x01);
    assert(eq.entries == 1);
    printf("  PASS test_set\n");
}

static void test_queue_event_basic(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    DM2_V1_QueueEventReceipt r = dm2_v1_eventqueue_queue_event(&eq, 50, 60, 0x01);
    assert(r.queued);
    assert(eq.entries == 1);
    assert(eq.data[r.slot].x == 50);
    assert(eq.data[r.slot].y == 60);
    assert(eq.data[r.slot].b == 0x01);
    printf("  PASS test_queue_event_basic\n");
}

static void test_queue_event_when_busy(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    eq.fetch_busy = true;
    DM2_V1_QueueEventReceipt r = dm2_v1_eventqueue_queue_event(&eq, 10, 20, 0x02);
    assert(!r.queued);
    assert(eq.singleevent_available);
    assert(eq.singleevent.x == 10);
    assert(eq.singleevent.y == 20);
    assert(eq.singleevent.b == 0x02);
    printf("  PASS test_queue_event_when_busy\n");
}

static void test_process_singleevent(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    /* Buffer a single event */
    eq.singleevent_available = true;
    eq.singleevent.x = 77;
    eq.singleevent.y = 88;
    eq.singleevent.b = 0x01;
    DM2_V1_ProcessSingleEventReceipt r =
        dm2_v1_eventqueue_process_singleevent(&eq);
    assert(r.processed);
    assert(r.had_event);
    assert(!eq.singleevent_available);
    assert(eq.entries == 1);
    printf("  PASS test_process_singleevent\n");
}

static void test_process_singleevent_none(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    DM2_V1_ProcessSingleEventReceipt r =
        dm2_v1_eventqueue_process_singleevent(&eq);
    assert(!r.processed);
    assert(!r.had_event);
    printf("  PASS test_process_singleevent_none\n");
}

static void test_queue_0x20(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    DM2_V1_QueueEventReceipt r = dm2_v1_eventqueue_queue_0x20(&eq, 0x1e);
    assert(r.queued);
    assert(eq.data[r.slot].b == 0x20);
    assert(eq.data[r.slot].x == 0x1e);
    assert(!eq.fetch_busy);
    printf("  PASS test_queue_0x20\n");
}

static void test_source_button_edge_and_keyboard_cap(void)
{
    DM2_V1_EventQueue eq;
    DM2_V1_QueueEventReceipt r;
    int i;

    dm2_v1_eventqueue_init(&eq);
    /* A normal 0x04 has the nine-entry source capacity. */
    for (i = 0; i < 9; ++i) {
        r = dm2_v1_eventqueue_queue_event(&eq, (int16_t)i, 0, 0x04);
        assert(r.queued);
    }
    assert(eq.entries == 9);
    /* At saturation 0x02 is not queued; it only sets the one-shot edge. */
    r = dm2_v1_eventqueue_queue_event(&eq, 99, 0, 0x02);
    assert(!r.queued && eq.button0x2);
    r = dm2_v1_eventqueue_queue_event(&eq, 100, 0, 0x04);
    assert(!r.queued && !eq.button0x2);

    dm2_v1_eventqueue_init(&eq);
    eq.data[1].y = 1234;
    for (i = 0; i < 7; ++i) {
        r = dm2_v1_eventqueue_queue_0x20(&eq, (int16_t)(0x10 + i));
        assert(r.queued);
    }
    assert(eq.entries == 7);
    r = dm2_v1_eventqueue_queue_0x20(&eq, 0x1f);
    assert(!r.queued && eq.entries == 7);
    /* First insertion is slot 1 and must retain source-owned y. */
    assert(eq.data[1].y == 1234);
    printf("  PASS test_source_button_edge_and_keyboard_cap\n");
}

static void test_flush_keeps_buttons(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    /* Add mixed events */
    dm2_v1_eventqueue_queue_event(&eq, 1, 1, 0x01);  /* not button */
    dm2_v1_eventqueue_queue_event(&eq, 2, 2, 0x04);  /* button - keep */
    dm2_v1_eventqueue_queue_event(&eq, 3, 3, 0x20);  /* keyboard - drop */
    dm2_v1_eventqueue_queue_event(&eq, 4, 4, 0x40);  /* button - keep */
    dm2_v1_eventqueue_queue_event(&eq, 5, 5, 0x60);  /* button - keep */
    assert(eq.entries == 5);

    dm2_v1_eventqueue_flush(&eq);
    assert(eq.entries == 3);
    assert(eq.data[0].x == 2);
    assert(eq.data[0].b == 0x04);
    assert(eq.data[1].x == 4);
    assert(eq.data[1].b == 0x40);
    assert(eq.data[2].x == 5);
    assert(eq.data[2].b == 0x60);
    assert(eq.event_unk05 == -1);
    assert(eq.event_unk09 == -1);
    printf("  PASS test_flush_keeps_buttons\n");
}

static void test_queue_overflow(void)
{
    DM2_V1_EventQueue eq;
    dm2_v1_eventqueue_init(&eq);
    /* Default cap for non-button events is 7 */
    int queued = 0;
    for (int i = 0; i < 10; i++) {
        DM2_V1_QueueEventReceipt r =
            dm2_v1_eventqueue_queue_event(&eq, (int16_t)i, 0, 0x01);
        if (r.queued) queued++;
    }
    assert(queued == 7);
    assert(eq.entries == 7);
    printf("  PASS test_queue_overflow\n");
}

int main(void)
{
    printf("test_dm2_v1_eventqueue_pc34_compat\n");
    test_init();
    test_set();
    test_queue_event_basic();
    test_queue_event_when_busy();
    test_process_singleevent();
    test_process_singleevent_none();
    test_queue_0x20();
    test_source_button_edge_and_keyboard_cap();
    test_flush_keeps_buttons();
    test_queue_overflow();
    printf("All tests passed.\n");
    return 0;
}
