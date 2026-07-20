/*
 * test_dm2_v1_caii_timer_replace_pc34_compat.c — stable timer tickets,
 * DM2_1c9a_0db0 delete, and the complete DM2_1c9a_0cf7 replacement
 * slice (DM2-003/005 follow-up).
 *
 * Verifies the skproject boundaries:
 *   c_timer.cpp:235-257   DM2_QUEUE_TIMER returns a stable slot index
 *                         (session ticket stand-in);
 *   c_timer.cpp:215-232   DM2_DELETE_TIMER removes a live timer by its
 *                         stable index;
 *   c_1c9a.cpp:5734-5763  DM2_1c9a_0db0: DB4 check, record byte@5 CAII
 *                         slot, slot word@2 pending timer delete +
 *                         write-back -1;
 *   c_1c9a.cpp:5695-5728  complete DM2_1c9a_0cf7: replace-first
 *                         (c_1c9a.cpp:5699-5706) + post-queue slot word@2
 *                         store (c_1c9a.cpp:5724-5728).
 */

#include "dm2_v1_caii_alloc_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_proceed_timers_pc34_compat.h"

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

/* ── synthetic map (square_bytes == 1) ────────────────────────────── */
#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128
#define RAW_SIZE 160

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

static uint16_t rd16(const uint8_t *p)
{
    return (uint16_t)((uint16_t)p[0] | ((uint16_t)p[1] << 8));
}

static void build_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
{
    memset(d, 0, sizeof(*d));
    memset(raw, 0, RAW_SIZE);
    d->level_count = 1;
    d->level_widths[0] = 2;
    d->level_heights[0] = 3;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = MAP_BASE;
    d->column_index_base = COLUMN_BASE;
    d->square_first_thing_base = GROUND_BASE;
    d->square_first_thing_count = 2;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */

    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2,
         (int16_t)((1u << 14) | (4u << 10) | 1u));
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    /* rec0: creature type 0x0C, byte@5 = -1, no group link. */
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C;
    set->pools[4].bytes[5] = 0xFF;
    wr16(set->pools[4].bytes + 8, (int16_t)0xffff);
    /* rec1: creature type 0x07, byte@5 = -1, group link. */
    wr16(set->pools[4].bytes + 16, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[16 + 4] = 0x07;
    set->pools[4].bytes[16 + 5] = 0xFF;
    wr16(set->pools[4].bytes + 16 + 8, 0x0003);

    set->valid = 1;
}

static int16_t rec_handle(int index)
{
    return (int16_t)((4 << 10) | (index & 0x3ff));
}

static DM2_V1_SourceTimer mk_timer(uint32_t tick, uint8_t type)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = tick & DM2_V1_SOURCE_TIMER_TICK_MASK;
    t.type = type;
    return t;
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_CreatureScheduleReceipt sched;
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiDeleteTimerReceipt del;
    DM2_V1_SourceTimer popped;
    const uint8_t *slot;

    /* ── (a) stable tickets over the source queue ──────────────────── */
    {
        DM2_V1_SourceTimerQueue q;
        DM2_V1_SourceTimer a = mk_timer(10, 0x21);
        DM2_V1_SourceTimer b = mk_timer(5, 0x54);   /* sorts first */
        DM2_V1_SourceTimer c = mk_timer(20, 0x22);
        uint32_t ta, tb, tc;

        dm2_v1_source_timer_queue_init(&q);
        ta = dm2_v1_source_timer_enqueue_ticketed(&q, &a, 0, NULL);
        tb = dm2_v1_source_timer_enqueue_ticketed(&q, &b, 0, NULL);
        tc = dm2_v1_source_timer_enqueue_ticketed(&q, &c, 0, NULL);
        CHECK(ta != 0 && tb != 0 && tc != 0 &&
                  ta != tb && tb != tc && ta != tc,
              "ticketed enqueue issues distinct stable tickets");
        CHECK(dm2_v1_source_timer_cancel(&q, tb) == 1,
              "cancel removes the ticketed timer");
        CHECK(q.count == 2, "queue shrinks on cancel");
        CHECK(dm2_v1_source_timer_pop_due(&q, 10u, &popped, NULL) ==
                  DM2_V1_SOURCE_TIMER_OK && popped.type == 0x21,
              "cancelled timer no longer pops; order preserved");
        CHECK(dm2_v1_source_timer_cancel(&q, tb) == 0 &&
                  dm2_v1_source_timer_cancel(&q, 0u) == 0,
              "unknown/zero tickets fail closed");
        CHECK(dm2_v1_source_timer_cancel(&q, ta) == 0,
              "an already-popped ticket fails closed (stale reference)");
        CHECK(dm2_v1_source_timer_cancel(&q, tc) == 1 &&
                  q.count == 0,
              "remaining timer cancels cleanly");
    }

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_caii_array_init(&caii, 4);
    dm2_v1_source_timer_queue_init(&queue);

    /* ── (b) alloc stores the issued ticket in slot word@2 ─────────── */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 100ul, rec_handle(0),
                                        0, 0, &alloc) == 1,
          "activation allocates slot + first timer");
    slot = dm2_v1_caii_slot(&caii, 0);
    CHECK(slot != NULL && rd16(slot + 2) != 0xffffu && rd16(slot + 2) != 0u,
          "slot word@2 stores the issued ticket (c_1c9a.cpp:5724-5728)");

    /* ── (c) DM2_1c9a_0db0 deletes the pending timer ───────────────── */
    memset(&del, 0, sizeof(del));
    CHECK(dm2_v1_caii_delete_timer(&set, &caii, &queue,
                                   rec_handle(0), &del) == 1,
          "0db0 deletes the pending timer");
    CHECK(del.valid == 1 && del.deleted == 1 && del.slot_index == 0 &&
              del.cancelled_ticket != 0,
          "delete receipted with the cancelled ticket");
    CHECK(rd16(dm2_v1_caii_slot(&caii, 0) + 2) == 0xffffu,
          "slot word@2 written back to -1");
    CHECK(queue.count == 0, "queue no longer holds the timer");

    /* second delete: no pending timer */
    memset(&del, 0, sizeof(del));
    CHECK(dm2_v1_caii_delete_timer(&set, &caii, &queue,
                                   rec_handle(0), &del) == 0 &&
              del.no_pending_timer == 1,
          "second delete receipted as no-pending-timer");
    /* record without a slot */
    memset(&del, 0, sizeof(del));
    CHECK(dm2_v1_caii_delete_timer(&set, &caii, &queue,
                                   rec_handle(1), &del) == 0 &&
              del.no_caii_slot == 1,
          "record without a CAII slot fails closed");
    /* non-DB4 handle rejected (c_1c9a.cpp:5741-5744) */
    memset(&del, 0, sizeof(del));
    CHECK(dm2_v1_caii_delete_timer(&set, &caii, &queue,
                                   (int16_t)((5 << 10) | 0), &del) == 0 &&
              del.not_creature_db == 1,
          "non-creature DB handle rejected");
    CHECK(strstr(del.source_evidence, "c_1c9a.cpp:5734-5763") != NULL,
          "delete evidence cites DM2_1c9a_0db0");

    /* ── (d) complete 0cf7 replacement: delete + requeue + store ───── */
    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_caii_schedule_creature_at(&set, &dungeon, &caii, &queue,
                                           0, 200ul, 0, 0, &sched) == 1,
          "CAII-aware schedule queues a fresh timer");
    CHECK(sched.replaced_existing == 0 && queue.count == 1,
          "no pending timer: nothing replaced");
    {
        uint16_t first_ticket = rd16(dm2_v1_caii_slot(&caii, 0) + 2);
        memset(&sched, 0, sizeof(sched));
        CHECK(dm2_v1_caii_schedule_creature_at(&set, &dungeon, &caii,
                                               &queue, 0, 250ul,
                                               0, 0, &sched) == 1,
              "reschedule replaces the pending timer");
        CHECK(sched.replaced_existing == 1,
              "replacement receipted (c_1c9a.cpp:5699-5706)");
        CHECK(sched.due_tick == 251ul,
              "new timer carries the new due tick");
        CHECK(queue.count == 1,
              "replacement never accumulates duplicate timers");
        CHECK(rd16(dm2_v1_caii_slot(&caii, 0) + 2) != first_ticket &&
                  rd16(dm2_v1_caii_slot(&caii, 0) + 2) ==
                      (uint16_t)sched.timer_ticket,
              "slot word@2 stores the new ticket");
        CHECK(dm2_v1_source_timer_pop_due(&queue, 251u, &popped, NULL) ==
                  DM2_V1_SOURCE_TIMER_OK &&
                  popped.type == DM2_V1_TIMER_THINK_CREATURE_A &&
                  popped.actor == 0x0C,
              "surviving timer is the replacement");
    }

    /* record without a slot: fail closed (source would index OOB) */
    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_caii_schedule_creature_at(&set, &dungeon, &caii, &queue,
                                           0, 300ul, 0, 1, &sched) == 0 &&
              sched.no_caii_slot == 1,
          "unslotted creature fails closed at the CAII-aware boundary");
    CHECK(strstr(sched.source_evidence, "c_1c9a.cpp:5695-5728") != NULL,
          "schedule evidence cites the complete DM2_1c9a_0cf7");

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_timer_replace_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_timer_replace_pc34_compat: all checks passed\n");
    return 0;
}
