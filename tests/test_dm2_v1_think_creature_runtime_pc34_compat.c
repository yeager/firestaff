/*
 * test_dm2_v1_think_creature_runtime_pc34_compat.c — per-cell
 * DM2_THINK_CREATURE wired into the runtime over the session-owned
 * DM2-002 record pools (DM2-003/005 follow-up).
 *
 * Verifies the runtime wiring:
 *   - the session-owned record pool set populates lazily from the boot
 *     dungeon data once its G1 candidate evidence validates
 *     (dm2_v1_record_pool_set_init_from_dungeon);
 *   - a dispatched 0x21 timer resolves the DB4 creature record AT THE
 *     TIMER CELL via DM2_GET_CREATURE_AT (c_querydb.cpp:1486-1507);
 *   - a 0x22 timer on a cell without a creature takes the source early
 *     return (c_ai.cpp:5670-5673) — consumed, nothing simulated;
 *   - the think body stays unbound (receipted) until the CCM stream
 *     owner/grammar is proven;
 *   - without dungeon data the binding stays unready and 0x21/0x22
 *     timers are acknowledged fail-closed.
 */

#include "dm2_v1_runtime.h"
#include "dm2_v1_think_creature_pc34_compat.h"

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
 * Level 0 is 2x3 (square_bytes == 1).  Object-flag cells: (0,0) ->
 * ground idx 0 (creature chain), (0,1) -> ground idx 1 (empty chain).
 * The DB4 pool (two 16-byte records) starts right after the zero-word
 * text span so the candidate evidence validates. */
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

    /* Object flags: (0,0) idx0, (0,1) idx1. */
    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */

    /* Column bases: column 0 starts at ground idx 0, column 1 at idx 2. */
    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    /* Ground heads:
     * idx 0: (0,0) -> DB4 creature rec0 directly (direction bits set)
     * idx 1: (0,1) -> OBJECT_END_MARKER (no records chained) */
    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2, DM2_V1_RECORD_HANDLE_END);

    /* DB4 creature pool: rec0 is a creature of type 0x0C whose own link
     * terminates; rec1 is unused. */
    wr16(raw + DB4_BASE + 0, DM2_V1_RECORD_HANDLE_END);
    raw[DB4_BASE + 4] = 0x0C; /* creature type (c_ai.cpp:5849-5858) */
    wr16(raw + DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
}

static DM2_V1_SourceTimer mk_think_timer(uint32_t tick, int map,
                                         uint8_t type, int x, int y)
{
    DM2_V1_SourceTimer t;
    memset(&t, 0, sizeof(t));
    t.ticks_and_map = ((uint32_t)(map & 0xff) << 24) |
                      (tick & DM2_V1_SOURCE_TIMER_TICK_MASK);
    t.type = type;
    t.actor = 0;
    t.value_a = (int16_t)(((y & 0xff) << 8) | (x & 0xff));
    return t;
}

static void test_runtime_per_cell_think_resolution(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ThinkCreatureReceipt receipt;
    DM2_V1_SourceTimer timer;

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0); /* dungeon session: no weather chain */

    CHECK(dm2_v1_runtime_record_pools_valid() == 0,
          "record pools populate lazily, not at boot");

    timer = mk_think_timer(1, 0, DM2_V1_TIMER_THINK_CREATURE_A, 0, 0);
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "0x21 timer for the creature cell accepted");
    timer = mk_think_timer(1, 0, DM2_V1_TIMER_THINK_CREATURE_B, 0, 1);
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "0x22 timer for the empty cell accepted");

    dm2_v1_runtime_tick();

    CHECK(dm2_v1_runtime_record_pools_valid() == 1,
          "session record pools validated from the boot dungeon");
    CHECK(dm2_v1_runtime_think_creature_receipt(&receipt) == 1,
          "think binding receipt published");
    CHECK(receipt.valid == 1 && receipt.think_timers == 2,
          "both think timers consumed by the binding");
    CHECK(receipt.resolved == 1,
          "creature record resolved at the 0x21 cell");
    CHECK(receipt.no_creature_at_cell == 1,
          "0x22 cell without a creature took the source early return");
    CHECK(receipt.body_consumed == 1,
          "think body consumed the resolved creature timer");
    CHECK(receipt.last_creature_type == 0x0C,
          "resolved record exposes the DB4 creature type byte");
    CHECK(receipt.last_map == 0 &&
              (receipt.last_type == DM2_V1_TIMER_THINK_CREATURE_A ||
               receipt.last_type == DM2_V1_TIMER_THINK_CREATURE_B),
          "receipt records the last timer's map and think type");
}

static void test_runtime_think_fail_closed_without_dungeon(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_ThinkCreatureReceipt receipt;
    DM2_V1_ProceedTimersReceipt proceed;
    DM2_V1_SourceTimer timer;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    timer = mk_think_timer(1, 0, DM2_V1_TIMER_THINK_CREATURE_A, 0, 0);
    (void)dm2_v1_runtime_enqueue_source_timer(&timer, 0);
    dm2_v1_runtime_tick();

    CHECK(dm2_v1_runtime_record_pools_valid() == 0,
          "no dungeon data: session pools stay unpopulated");
    CHECK(dm2_v1_runtime_think_creature_receipt(&receipt) == 0,
          "no dungeon data: think binding stays unready");
    CHECK(dm2_v1_runtime_last_proceed_timers_receipt(&proceed) == 1 &&
              proceed.type_tally[DM2_V1_TIMER_THINK_CREATURE_A] >= 1,
          "0x21 timer still dispatched and acknowledged fail-closed");
}

int main(void)
{
    printf("DM2 V1 per-cell DM2_THINK_CREATURE runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_tim_proc.cpp:4079-4088\n");
    printf("        skproject/SKULLWIN/c_ai.cpp:5649-5677\n");
    printf("        skproject/SKULLWIN/c_querydb.cpp:1486-1507\n\n");

    test_runtime_per_cell_think_resolution();
    test_runtime_think_fail_closed_without_dungeon();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
