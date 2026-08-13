/*
 * test_dm2_v1_creature_schedule_pc34_compat.c — creature-scheduling
 * producer DM2_1c9a_0cf7 (bounded compatibility slice).
 *
 * Verifies the skproject boundary (c_1c9a.cpp:5695-5728):
 *   - the creature record resolves at the cell via DM2_GET_CREATURE_AT
 *     (c_1c9a.cpp:5698, c_querydb.cpp:1486-1507);
 *   - the timer type is 0x22 when the record group/leader link (word@8)
 *     is not 0xffff, else 0x21 (c_1c9a.cpp:5708-5712);
 *   - the owner is the record creature-type byte@4 (c_1c9a.cpp:5713-5714);
 *   - the due tick is gametick + 1 and the map word rides the high byte
 *     (setmticks, c_1c9a.cpp:5707);
 *   - the payload is setxyA(x, y) = CUTX16(x) | (CUTX16(y) << 8)
 *     (c_1c9a.cpp:5715-5716), matching the think consumer;
 *   - a cell without a creature fails closed without queue mutation;
 *   - the CAII slot timer word and the DM2_1c9a_0db0 delete stay
 *     host-owned (receipted replaced_existing == 0, never simulated).
 */

#include "dm2_v1_creature_schedule_pc34_compat.h"

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

/* ── synthetic map (square_bytes == 1) ──────────────────────────────
 * Level 0 is 2x3.  Object-flag cells: (0,0) -> ground idx 0 (creature
 * without a group link), (0,1) -> ground idx 1 (creature with a group
 * link), (1,1) -> ground idx 2 (empty chain). */
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
    d->square_first_thing_count = 3;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    /* Object flags: (0,0) idx0, (0,1) idx1, (1,1) idx2. */
    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */
    raw[MAP_BASE + 4] = 0x10; /* (1,1) */

    /* Column bases: column 0 starts at ground idx 0, column 1 at idx 2. */
    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    /* Ground heads:
     * idx 0: (0,0) -> DB4 creature rec0 directly (direction bits set)
     * idx 1: (0,1) -> DB4 creature rec1 directly
     * idx 2: (1,1) -> OBJECT_END_MARKER (no records chained) */
    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2,
         (int16_t)((1u << 14) | (4u << 10) | 1u));
    wr16(raw + GROUND_BASE + 4, DM2_V1_RECORD_HANDLE_END);
}

static void build_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    /* DB4 creature pool: two 16-byte records.
     * rec0: creature type 0x0C, group/leader link word@8 == 0xffff
     *       (ungrouped -> 0x21 timer, c_1c9a.cpp:5708-5712).
     * rec1: creature type 0x07, group/leader link word@8 == 0x0003
     *       (grouped -> 0x22 timer). */
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C; /* creature type byte@4 */
    wr16(set->pools[4].bytes + 8, (int16_t)0xffff); /* no group link */
    wr16(set->pools[4].bytes + 16, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[16 + 4] = 0x07;
    wr16(set->pools[4].bytes + 16 + 8, 0x0003); /* group/leader link */

    set->valid = 1;
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_CreatureScheduleReceipt rc;
    DM2_V1_SourceTimer popped;
    uint16_t source_index = 0xffffu;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_source_timer_queue_init(&queue);

    /* ── (a) ungrouped creature: 0x21 timer, due gametick + 1 ──────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      0, 100ul, 0, 0, &rc) == 1,
          "ungrouped creature cell schedules");
    CHECK(rc.valid == 1 && rc.resolved == 1 && rc.enqueued == 1,
          "receipt valid/resolved/enqueued");
    CHECK(rc.timer_type == DM2_V1_TIMER_THINK_CREATURE_A &&
              rc.has_group_link == 0,
          "word@8 == 0xffff yields a 0x21 timer");
    CHECK(rc.creature_type == 0x0C,
          "owner is the record creature-type byte@4");
    CHECK(rc.due_tick == 101ul && rc.map_id == 0,
          "due = gametick + 1 with the caller-owned map");
    CHECK(rc.replaced_existing == 0,
          "CAII slot delete stays host-owned (receipted, not simulated)");
    CHECK(queue.count == 1, "exactly one timer queued");

    /* Not due before gametick + 1; due exactly at gametick + 1. */
    CHECK(dm2_v1_source_timer_pop_due(&queue, 100u, &popped,
                                      &source_index) ==
              DM2_V1_SOURCE_TIMER_NOT_DUE,
          "timer not due at gametick");
    CHECK(dm2_v1_source_timer_pop_due(&queue, 101u, &popped,
                                      &source_index) == DM2_V1_SOURCE_TIMER_OK,
          "timer due at gametick + 1");
    CHECK(popped.type == DM2_V1_TIMER_THINK_CREATURE_A &&
              popped.actor == 0x0C,
          "queued timer carries the source type and owner");
    CHECK(popped.ticks_and_map ==
              UINT32_C(101),
          "queued timer carries map<<24 | due tick");
    CHECK(popped.value_a == 0,
          "setxyA payload for cell (0,0)");

    /* ── (b) grouped creature: 0x22 timer, payload packs y ─────────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      0, 7ul, 0, 1, &rc) == 1,
          "grouped creature cell schedules");
    CHECK(rc.timer_type == DM2_V1_TIMER_THINK_CREATURE_B &&
              rc.has_group_link == 1,
          "word@8 != 0xffff yields a 0x22 timer");
    CHECK(rc.creature_type == 0x07 && rc.due_tick == 8ul,
          "grouped owner and due tick receipted");
    CHECK(dm2_v1_source_timer_pop_due(&queue, 8u, &popped,
                                      &source_index) == DM2_V1_SOURCE_TIMER_OK,
          "grouped timer pops at its due tick");
    CHECK(popped.type == DM2_V1_TIMER_THINK_CREATURE_B &&
              popped.value_a == (int16_t)(0 | (1 << 8)),
          "setxyA payload packs x | (y << 8)");

    /* ── (c) cell without a creature fails closed ──────────────────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      0, 50ul, 1, 1, &rc) == 0,
          "cell without a creature fails closed");
    CHECK(rc.valid == 0 && rc.resolved == 0 && rc.enqueued == 0,
          "fail-closed receipt carries no mutation evidence");
    CHECK(queue.count == 0, "fail-closed path never mutates the queue");

    /* ── (d) argument validation fails closed ──────────────────────── */
    CHECK(dm2_v1_creature_schedule_at(NULL, &dungeon, &queue,
                                      0, 1ul, 0, 0, &rc) == 0,
          "null pool set rejected");
    CHECK(dm2_v1_creature_schedule_at(&set, NULL, &queue,
                                      0, 1ul, 0, 0, &rc) == 0,
          "null dungeon rejected");
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, NULL,
                                      0, 1ul, 0, 0, &rc) == 0,
          "null queue rejected");
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      256, 1ul, 0, 0, &rc) == 0,
          "out-of-range map rejected");
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      0, 1ul, -1, 0, &rc) == 0,
          "out-of-range cell rejected");
    CHECK(dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      0, 1ul, 0, 0, NULL) == 1,
          "NULL receipt still schedules");
    (void)dm2_v1_source_timer_pop_due(&queue, 2u, &popped, &source_index);

    /* ── (e) source evidence cites the producer ────────────────────── */
    memset(&rc, 0, sizeof(rc));
    (void)dm2_v1_creature_schedule_at(&set, &dungeon, &queue,
                                      0, 1ul, 0, 0, &rc);
    CHECK(strstr(rc.source_evidence, "c_1c9a.cpp:5695-5728") != NULL,
          "source evidence cites DM2_1c9a_0cf7");

    dm2_v1_record_pool_set_free(&set);

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_creature_schedule_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_creature_schedule_pc34_compat: all checks passed\n");
    return 0;
}
