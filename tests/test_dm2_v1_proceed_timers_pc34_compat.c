/*
 * test_dm2_v1_proceed_timers_pc34_compat.c — DM2-003 source-ordered timer
 * dispatcher.
 *
 * Verifies the DM2_PROCEED_TIMERS boundary against skproject anchors:
 *   c_tim_proc.cpp:3980-4230  type matrix, pop-before-map-switch, unknown
 *                             types fall through to `continue`
 *   c_timer.cpp:31-47         DM2_cmp_timers order (tick asc, type desc,
 *                             actor desc, source index asc)
 *   c_timer.h:64              map = high byte of l_00
 */

#include "dm2_v1_proceed_timers_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

#define MAX_LOG 64

typedef struct {
    uint8_t types[MAX_LOG];
    int count;
    int reject_type; /* return 0 for this type, -1 = never */
} HandlerLog;

static int logging_handler(void *context,
                           const DM2_V1_SourceTimer *timer,
                           uint16_t source_index,
                           DM2_V1_ProceedTimersReceipt *receipt)
{
    HandlerLog *log = (HandlerLog *)context;
    (void)source_index;
    (void)receipt;
    if (log->count < MAX_LOG) {
        log->types[log->count++] = timer->type;
    }
    if (log->reject_type >= 0 && timer->type == (uint8_t)log->reject_type) {
        return 0;
    }
    return 1;
}

/* Simple consumed handler for subdispatch tests where the context is the
 * tile-class provider, not a HandlerLog. */
static int consumed_handler(void *context,
                            const DM2_V1_SourceTimer *timer,
                            uint16_t source_index,
                            DM2_V1_ProceedTimersReceipt *receipt)
{
    (void)context;
    (void)timer;
    (void)source_index;
    (void)receipt;
    return 1;
}

static DM2_V1_SourceTimer mk_timer(uint32_t tick, int map, uint8_t type,
                                   uint8_t actor)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                      (tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    t.type = type;
    t.actor = actor;
    return t;
}

int main(void)
{
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_ProceedTimersReceipt receipt;
    DM2_V1_TimerDispatcher dispatcher;
    HandlerLog log;

    /* ── type matrix membership (c_tim_proc.cpp:3980-4230) ──────── */
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_STEP_DOOR), "0x01 known");
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_ACTUATE_TILE), "0x04 known");
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_STEP_MISSILE), "0x1e known");
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_THINK_CREATURE_A),
          "0x21 known");
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_THINK_CREATURE_B),
          "0x22 known");
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_UPDATE_WEATHER),
          "0x54 known");
    CHECK(dm2_v1_timer_type_is_known(DM2_V1_TIMER_ALLOC_NEW_CREATURE),
          "0x5e known");
    CHECK(!dm2_v1_timer_type_is_known(0x00), "0x00 unknown (source continue)");
    CHECK(!dm2_v1_timer_type_is_known(0x03), "0x03 unknown");
    CHECK(!dm2_v1_timer_type_is_known(0x10), "0x10 unknown");
    CHECK(!dm2_v1_timer_type_is_known(0x5f), "0x5f unknown");

    /* ── source-order dispatch: tick asc, type desc, actor desc ─── */
    dm2_v1_source_timer_queue_init(&queue);
    memset(&dispatcher, 0, sizeof(dispatcher));
    memset(&log, 0, sizeof(log));
    log.reject_type = -1;
    dispatcher.context = &log;
    for (int i = 0; i < DM2_V1_TIMER_TYPE_MATRIX_SIZE; ++i) {
        dispatcher.handlers[i] = logging_handler;
    }
    /* Enqueue deliberately out of order. */
    dm2_v1_source_timer_queue_init(&queue);
    {
        DM2_V1_SourceTimer a = mk_timer(10, 1, DM2_V1_TIMER_STEP_DOOR, 1);
        DM2_V1_SourceTimer b = mk_timer(5, 2, DM2_V1_TIMER_UPDATE_WEATHER, 1);
        DM2_V1_SourceTimer c = mk_timer(5, 3, DM2_V1_TIMER_STEP_MISSILE, 1);
        DM2_V1_SourceTimer d = mk_timer(5, 4, DM2_V1_TIMER_STEP_MISSILE, 9);
        DM2_V1_SourceTimer e = mk_timer(50, 5, DM2_V1_TIMER_PROCESS_SOUND, 1);
        (void)dm2_v1_source_timer_enqueue(&queue, &a, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &b, 1);
        (void)dm2_v1_source_timer_enqueue(&queue, &c, 2);
        (void)dm2_v1_source_timer_enqueue(&queue, &d, 3);
        (void)dm2_v1_source_timer_enqueue(&queue, &e, 4);
    }
    CHECK(dm2_v1_proceed_timers(&queue, 10, &dispatcher, &receipt),
          "dispatcher ran");
    CHECK(receipt.valid, "receipt valid");
    CHECK(receipt.due_count == 4, "four timers due at tick 10");
    CHECK(receipt.dispatched_count == 4, "all bound handlers consumed");
    CHECK(receipt.skipped_unknown_type == 0, "no unknown types");
    CHECK(receipt.fail_closed_count == 0, "no fail-closed types");
    CHECK(receipt.map_switches == 4, "one map switch per popped timer");
    /* Order at tick 5: UPDATE_WEATHER (0x54) before STEP_MISSILE (0x1e)
     * because DM2_cmp_timers orders type descending within equal ticks;
     * within equal type, actor descending (9 before 1). */
    CHECK(log.count == 4, "handler saw four timers");
    CHECK(log.types[0] == DM2_V1_TIMER_UPDATE_WEATHER &&
          log.types[1] == DM2_V1_TIMER_STEP_MISSILE &&
          log.types[2] == DM2_V1_TIMER_STEP_MISSILE &&
          log.types[3] == DM2_V1_TIMER_STEP_DOOR,
          "source order: tick asc, type desc");
    CHECK(receipt.last_map == 1, "last popped timer map switches current map");

    /* ── not-due boundary ───────────────────────────────────────── */
    CHECK(queue.count == 1, "tick-50 timer retained");
    memset(&log, 0, sizeof(log));
    log.reject_type = -1;
    CHECK(dm2_v1_proceed_timers(&queue, 10, &dispatcher, &receipt),
          "dispatcher ran with no due timers");
    CHECK(receipt.due_count == 0 && log.count == 0,
          "nothing dispatched before due tick");
    CHECK(dm2_v1_proceed_timers(&queue, 50, &dispatcher, &receipt),
          "dispatcher ran at due tick");
    CHECK(receipt.due_count == 1 && log.count == 1,
          "timer dispatches exactly at its tick");

    /* ── unknown types skip like the source's continue ──────────── */
    dm2_v1_source_timer_queue_init(&queue);
    memset(&log, 0, sizeof(log));
    log.reject_type = -1;
    {
        DM2_V1_SourceTimer u = mk_timer(1, 0, 0x10, 0);
        DM2_V1_SourceTimer k = mk_timer(1, 0, DM2_V1_TIMER_STEP_DOOR, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &u, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &k, 1);
    }
    (void)dm2_v1_proceed_timers(&queue, 1, &dispatcher, &receipt);
    CHECK(receipt.due_count == 2, "both timers popped");
    CHECK(receipt.skipped_unknown_type == 1, "unknown type skipped");
    CHECK(receipt.dispatched_count == 1, "known type dispatched");
    CHECK(log.count == 1 && log.types[0] == DM2_V1_TIMER_STEP_DOOR,
          "unknown type never reaches a handler");

    /* ── fail-closed: known type without a bound handler ────────── */
    dm2_v1_source_timer_queue_init(&queue);
    memset(&dispatcher, 0, sizeof(dispatcher));
    memset(&log, 0, sizeof(log));
    log.reject_type = -1;
    dispatcher.context = &log;
    dispatcher.handlers[DM2_V1_TIMER_THINK_CREATURE_A] = logging_handler;
    {
        DM2_V1_SourceTimer think = mk_timer(2, 0, DM2_V1_TIMER_THINK_CREATURE_A, 0);
        DM2_V1_SourceTimer door = mk_timer(2, 0, DM2_V1_TIMER_STEP_DOOR, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &think, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &door, 1);
    }
    (void)dm2_v1_proceed_timers(&queue, 2, &dispatcher, &receipt);
    CHECK(receipt.due_count == 2, "both popped");
    CHECK(receipt.dispatched_count == 1, "bound think handler consumed");
    CHECK(receipt.fail_closed_count == 1,
          "unbound known type fail-closed, never simulated");
    CHECK(log.count == 1 &&
          log.types[0] == DM2_V1_TIMER_THINK_CREATURE_A,
          "only the bound handler ran");

    /* ── handler rejection is counted, dispatch continues ───────── */
    dm2_v1_source_timer_queue_init(&queue);
    memset(&dispatcher, 0, sizeof(dispatcher));
    memset(&log, 0, sizeof(log));
    dispatcher.context = &log;
    log.reject_type = DM2_V1_TIMER_STEP_DOOR;
    for (int i = 0; i < DM2_V1_TIMER_TYPE_MATRIX_SIZE; ++i) {
        dispatcher.handlers[i] = logging_handler;
    }
    {
        DM2_V1_SourceTimer door = mk_timer(3, 0, DM2_V1_TIMER_STEP_DOOR, 0);
        DM2_V1_SourceTimer light = mk_timer(3, 0, DM2_V1_TIMER_LIGHT, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &door, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &light, 1);
    }
    (void)dm2_v1_proceed_timers(&queue, 3, &dispatcher, &receipt);
    CHECK(receipt.handler_rejected_count == 1, "rejection counted");
    CHECK(receipt.dispatched_count == 1, "later timer still dispatched");

    /* ── 0x04 actuator tile subdispatch ─────────────────────────── */
    {
        /* Context-driven tile-class provider lets us exercise every class. */
        int tile_class = 4;
        extern int test_tile_class_at_with_context(void *, int, int, int);

        dm2_v1_source_timer_queue_init(&queue);
        memset(&dispatcher, 0, sizeof(dispatcher));
        dispatcher.context = &tile_class;
        dispatcher.tile_class_at = test_tile_class_at_with_context;
        dispatcher.actuator_tile[0] = consumed_handler;
        dispatcher.actuator_tile[1] = consumed_handler;
        dispatcher.actuator_tile[2] = consumed_handler;
        dispatcher.actuator_tile[4] = consumed_handler;
        dispatcher.actuator_tile[5] = consumed_handler;
        dispatcher.actuator_tile[6] = consumed_handler;
        for (tile_class = 0; tile_class < 7; tile_class++) {
            DM2_V1_SourceTimer act;
            if (tile_class == 3) {
                /* Source case 3 is an acknowledged no-op: the dispatcher
                 * tallies it but does not call a handler. */
                continue;
            }
            dm2_v1_source_timer_queue_init(&queue);
            memset(&receipt, 0, sizeof(receipt));
            act = mk_timer(4, 0, DM2_V1_TIMER_ACTUATE_TILE, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &act, 0);
            (void)dm2_v1_proceed_timers(&queue, 4, &dispatcher, &receipt);
            CHECK(receipt.actuator_tile_tally[tile_class] == 1,
                  "tile class handler tallied");
            CHECK(receipt.dispatched_count == 1,
                  "tile class handler consumed");
        }

        /* Class 3 is a source no-op: tallied but no handler called. */
        dm2_v1_source_timer_queue_init(&queue);
        memset(&receipt, 0, sizeof(receipt));
        tile_class = 3;
        {
            DM2_V1_SourceTimer act = mk_timer(4, 0, DM2_V1_TIMER_ACTUATE_TILE, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &act, 0);
        }
        (void)dm2_v1_proceed_timers(&queue, 4, &dispatcher, &receipt);
        CHECK(receipt.actuator_tile_tally[3] == 1,
              "tile class 3 no-op tallied");
        CHECK(receipt.dispatched_count == 1,
              "tile class 3 no-op consumed by dispatcher");

        /* Class > 6 fails closed. */
        dm2_v1_source_timer_queue_init(&queue);
        memset(&receipt, 0, sizeof(receipt));
        tile_class = 7;
        {
            DM2_V1_SourceTimer act = mk_timer(4, 0, DM2_V1_TIMER_ACTUATE_TILE, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &act, 0);
        }
        (void)dm2_v1_proceed_timers(&queue, 4, &dispatcher, &receipt);
        CHECK(receipt.fail_closed_count == 1 && receipt.dispatched_count == 0,
              "tile class 7 fails closed");
    }

    /* Without tile state the 0x04 boundary fails closed. */
    dm2_v1_source_timer_queue_init(&queue);
    memset(&dispatcher, 0, sizeof(dispatcher));
    {
        DM2_V1_SourceTimer act = mk_timer(5, 0, DM2_V1_TIMER_ACTUATE_TILE, 0);
        (void)dm2_v1_source_timer_enqueue(&queue, &act, 0);
    }
    (void)dm2_v1_proceed_timers(&queue, 5, &dispatcher, &receipt);
    CHECK(receipt.fail_closed_count == 1 && receipt.dispatched_count == 0,
          "0x04 without tile state fails closed");

    CHECK(dm2_v1_proceed_timers(NULL, 0, &dispatcher, &receipt) == 0,
          "NULL queue rejected");

    /* New timer types 0x02, 0x58, 0x59 route through handlers when bound. */
    {
        HandlerLog log2;
        memset(&log2, 0, sizeof(log2));
        log2.reject_type = -1;
        memset(&dispatcher, 0, sizeof(dispatcher));
        dispatcher.context = &log2;
        dispatcher.handlers[DM2_V1_TIMER_DESTROY_DOOR] = logging_handler;
        dispatcher.handlers[DM2_V1_TIMER_RELEASE_DOOR_BUTTON] = logging_handler;
        dispatcher.handlers[DM2_V1_TIMER_PROCESS_59] = logging_handler;

        dm2_v1_source_timer_queue_init(&queue);
        memset(&receipt, 0, sizeof(receipt));
        {
            DM2_V1_SourceTimer t02 = mk_timer(10, 0, DM2_V1_TIMER_DESTROY_DOOR, 0);
            DM2_V1_SourceTimer t58 = mk_timer(10, 0, DM2_V1_TIMER_RELEASE_DOOR_BUTTON, 0);
            DM2_V1_SourceTimer t59 = mk_timer(10, 0, DM2_V1_TIMER_PROCESS_59, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &t02, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &t58, 1);
            (void)dm2_v1_source_timer_enqueue(&queue, &t59, 2);
        }
        (void)dm2_v1_proceed_timers(&queue, 10, &dispatcher, &receipt);
        CHECK(log2.count == 3,
              "0x02/0x58/0x59 all dispatched through handlers");
        CHECK(receipt.dispatched_count == 3,
              "0x02/0x58/0x59 consumed count matches");
        CHECK(receipt.type_tally[DM2_V1_TIMER_DESTROY_DOOR] == 1,
              "0x02 tallied once");
        CHECK(receipt.type_tally[DM2_V1_TIMER_RELEASE_DOOR_BUTTON] == 1,
              "0x58 tallied once");
        CHECK(receipt.type_tally[DM2_V1_TIMER_PROCESS_59] == 1,
              "0x59 tallied once");
    }

    /* 0x5b and 0x5c route through handlers when bound. */
    {
        HandlerLog log3;
        memset(&log3, 0, sizeof(log3));
        log3.reject_type = -1;
        memset(&dispatcher, 0, sizeof(dispatcher));
        dispatcher.context = &log3;
        dispatcher.handlers[DM2_V1_TIMER_5B_RECORD_CLEAR] = logging_handler;
        dispatcher.handlers[DM2_V1_TIMER_5C_RECORD_SET] = logging_handler;

        dm2_v1_source_timer_queue_init(&queue);
        memset(&receipt, 0, sizeof(receipt));
        {
            DM2_V1_SourceTimer t5b = mk_timer(12, 0, DM2_V1_TIMER_5B_RECORD_CLEAR, 0);
            DM2_V1_SourceTimer t5c = mk_timer(12, 0, DM2_V1_TIMER_5C_RECORD_SET, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &t5b, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &t5c, 1);
        }
        (void)dm2_v1_proceed_timers(&queue, 12, &dispatcher, &receipt);
        CHECK(log3.count == 2, "0x5b/0x5c both dispatched");
        CHECK(receipt.type_tally[DM2_V1_TIMER_5B_RECORD_CLEAR] == 1,
              "0x5b tallied once");
        CHECK(receipt.type_tally[DM2_V1_TIMER_5C_RECORD_SET] == 1,
              "0x5c tallied once");
    }

    /* Without bound handlers, 0x02/0x58/0x59/0x5b/0x5c fail closed. */
    {
        dm2_v1_source_timer_queue_init(&queue);
        memset(&dispatcher, 0, sizeof(dispatcher));
        memset(&receipt, 0, sizeof(receipt));
        {
            DM2_V1_SourceTimer t02 = mk_timer(11, 0, DM2_V1_TIMER_DESTROY_DOOR, 0);
            DM2_V1_SourceTimer t58 = mk_timer(11, 0, DM2_V1_TIMER_RELEASE_DOOR_BUTTON, 0);
            DM2_V1_SourceTimer t59 = mk_timer(11, 0, DM2_V1_TIMER_PROCESS_59, 0);
            DM2_V1_SourceTimer t5b = mk_timer(11, 0, DM2_V1_TIMER_5B_RECORD_CLEAR, 0);
            DM2_V1_SourceTimer t5c = mk_timer(11, 0, DM2_V1_TIMER_5C_RECORD_SET, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &t02, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &t58, 1);
            (void)dm2_v1_source_timer_enqueue(&queue, &t59, 2);
            (void)dm2_v1_source_timer_enqueue(&queue, &t5b, 3);
            (void)dm2_v1_source_timer_enqueue(&queue, &t5c, 4);
        }
        (void)dm2_v1_proceed_timers(&queue, 11, &dispatcher, &receipt);
        CHECK(receipt.fail_closed_count == 5,
              "0x02/0x58/0x59/0x5b/0x5c fail closed without handlers");
        CHECK(receipt.dispatched_count == 0,
              "none dispatched without handlers");
    }

    CHECK(strstr(dm2_v1_proceed_timers_source_evidence(),
                 "c_tim_proc.cpp") != NULL,
          "source evidence cites c_tim_proc.cpp");

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_proceed_timers_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_proceed_timers_pc34_compat: all checks passed\n");
    return 0;
}

int test_tile_class_at_with_context(void *context, int map, int x, int y)
{
    int *tile_class = (int *)context;
    (void)map;
    (void)x;
    (void)y;
    return *tile_class;
}
