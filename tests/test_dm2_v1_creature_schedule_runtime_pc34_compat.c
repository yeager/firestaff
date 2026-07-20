/*
 * test_dm2_v1_creature_schedule_runtime_pc34_compat.c — creature
 * scheduling producer wired into the runtime (DM2-003 follow-up).
 *
 * Verifies the end-to-end chain:
 *   dm2_v1_runtime_schedule_creature_at
 *     (c_1c9a.cpp:5695-5728 DM2_1c9a_0cf7, bounded slice)
 *       -> runtime source timer queue (DM2_QUEUE_TIMER route)
 *       -> dm2_v1_proceed_timers dispatch on the next tick
 *       -> per-cell DM2_THINK_CREATURE binding
 *          (c_tim_proc.cpp:4079-4088, c_ai.cpp:5649-5677)
 *          resolving the same record the producer scheduled.
 *
 * Also verifies the fail-closed boundaries: a cell without a creature
 * never enqueues, and without dungeon data the boundary stays unready.
 */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

/* ── synthetic map with G1 pool evidence ─────────────────────────────
 * Same layout as the think-runtime fixture: level 0 is 2x3, (0,0) holds
 * creature rec0, (0,1) holds an empty chain.  rec0 carries creature type
 * 0x0C and group/leader link word@8 == 0xffff (ungrouped -> 0x21). */
#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128
#define TEXT_BASE 256
#define DB4_BASE 256
#define G1_EXT_BASE (DB4_BASE + 2 * 16)
#define RAW_SIZE 512

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
    d->square_first_thing_count = 2;
    d->text_data_base = TEXT_BASE;
    d->text_word_count = 0;
    d->g1_extension_base = G1_EXT_BASE;
    d->thing_type_counts[4] = 2;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */

    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2, DM2_V1_RECORD_HANDLE_END);

    wr16(raw + DB4_BASE + 0, DM2_V1_RECORD_HANDLE_END);
    raw[DB4_BASE + 4] = 0x0C;              /* creature type byte@4 */
    wr16(raw + DB4_BASE + 8, (int16_t)0xffff); /* no group link -> 0x21 */
    wr16(raw + DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
}

static void test_runtime_schedule_think_end_to_end(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CreatureScheduleReceipt sched;
    DM2_V1_ThinkCreatureReceipt think;

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0); /* dungeon session: no weather chain */

    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_runtime_schedule_creature_at(0, 0, 0, &sched) == 1,
          "producer boundary schedules the creature at (0,0)");
    CHECK(sched.valid == 1 && sched.enqueued == 1 &&
              sched.timer_type == DM2_V1_TIMER_THINK_CREATURE_A &&
              sched.creature_type == 0x0C && sched.due_tick == 1ul,
          "receipt: 0x21 timer for creature 0x0C due next tick");
    CHECK(dm2_v1_runtime_record_pools_valid() == 1,
          "producer boundary populates the session pools lazily");

    /* The empty cell never enqueues. */
    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_runtime_schedule_creature_at(0, 0, 1, &sched) == 0 &&
              sched.valid == 0,
          "cell without a creature fails closed at the boundary");

    /* Next tick: the queued timer dispatches into the think binding. */
    dm2_v1_runtime_tick();

    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1,
          "think binding receipt published");
    CHECK(think.valid == 1 && think.think_timers == 1 &&
              think.resolved == 1 && think.last_creature_type == 0x0C &&
              think.last_type == DM2_V1_TIMER_THINK_CREATURE_A,
          "producer-scheduled timer resolved the same record end-to-end");
    CHECK(think.no_creature_at_cell == 0,
          "no stray timer for the empty cell entered the queue");
}

static void test_runtime_schedule_fail_closed_without_dungeon(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CreatureScheduleReceipt sched;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_runtime_schedule_creature_at(0, 0, 0, &sched) == 0,
          "no dungeon data: producer boundary stays unready");
    CHECK(dm2_v1_runtime_record_pools_valid() == 0,
          "no dungeon data: session pools stay unpopulated");
}

int main(void)
{
    printf("DM2 V1 creature scheduling producer runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_1c9a.cpp:5695-5728 (DM2_1c9a_0cf7)\n");
    printf("        skproject/SKULLWIN/c_tim_proc.cpp:4079-4088 (0x21/0x22)\n");
    printf("        skproject/SKULLWIN/c_ai.cpp:5649-5677 (DM2_THINK_CREATURE)\n\n");

    test_runtime_schedule_think_end_to_end();
    test_runtime_schedule_fail_closed_without_dungeon();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
