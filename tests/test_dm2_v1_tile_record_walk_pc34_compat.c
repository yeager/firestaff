/*
 * test_dm2_v1_tile_record_walk_pc34_compat.c — DM2-002 tile record-link
 * walk plus the XACT_85 / ACTIVATE_CREATURE_KILLER AI-stop callers.
 *
 * Verifies the skproject boundary:
 *   c_map.cpp:61-69          tile record link (bit-0x10 object flag +
 *                            column-index ground-stack table)
 *   c_record.cpp:54-57       next-link walk, OBJECT_END_MARKER end
 *   c_ai.cpp:2078-2117       DM2_PROCEED_XACT_85: DB>3 break, DB2 word@2
 *                            match (b_1e=1, b_1a=59, return -2), walk-end
 *                            AI-stop + b_1a=51, return -3
 *   c_tim_proc.cpp:2907-2988 DM2_ACTIVATE_CREATURE_KILLER: rectangular
 *                            sweep, 1c9a_09b9 word@8 filter, mode 0/1
 *                            skip, mode 2 AI-stop, mode >2 abort, action
 *                            0x28 attack word with the 0x8000 flag
 */

#include "dm2_v1_tile_record_walk_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int g_failures = 0;

#define CHECK(cond, msg)                                              \
    do {                                                              \
        if (!(cond)) {                                                \
            fprintf(stderr, "FAIL: %s (line %d)\n", msg, __LINE__);   \
            ++g_failures;                                             \
        }                                                             \
    } while (0)

#define RAW_SIZE 256
#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128

static int16_t mk_handle(int pool, int index)
{
    return (int16_t)((pool << 10) | (index & 0x3ff));
}

static void wr16(uint8_t *p, int16_t v)
{
    uint16_t u = (uint16_t)v;
    p[0] = (uint8_t)(u & 0xffu);
    p[1] = (uint8_t)((u >> 8) & 0xffu);
}

/* ── XACT fixture: 2x2 map ──────────────────────────────────────────
 * Flagged cells: (0,0) -> ground idx 0, (0,1) -> idx 1, (1,1) -> idx 2.
 * idx 0: DB2 rec1 (near-miss) -> DB2 rec0 (match) -> END
 * idx 1: DB3 rec0 -> END
 * idx 2: DB3 rec1 -> itself (corrupt self-loop) */
static void build_xact_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
{
    memset(d, 0, sizeof(*d));
    memset(raw, 0, RAW_SIZE);
    d->level_count = 1;
    d->level_widths[0] = 2;
    d->level_heights[0] = 2;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = MAP_BASE;
    d->column_index_base = COLUMN_BASE;
    d->square_first_thing_base = GROUND_BASE;
    d->square_first_thing_count = 3;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */
    raw[MAP_BASE + 3] = 0x10; /* (1,1) */
    wr16(raw + COLUMN_BASE + 0, 0); /* column 0 base */
    wr16(raw + COLUMN_BASE + 2, 2); /* column 1 base */

    wr16(raw + GROUND_BASE + 0, mk_handle(2, 1));
    wr16(raw + GROUND_BASE + 2, mk_handle(3, 0));
    wr16(raw + GROUND_BASE + 4, mk_handle(3, 1));
}

static void build_xact_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    /* DB2 (4-byte records): rec0 matches the word@2 probe
     * (0x0802: (w & 0x6) == 0x2, (w >> 11) == 1); rec1 is the
     * near-miss 0x0806 ((w & 0x6) == 0x6) and chains to rec0. */
    set->pools[2].record_size = 4;
    set->pools[2].record_count = 2;
    set->pools[2].source_base = 0;
    set->pools[2].bytes = calloc(2, 4);
    wr16(set->pools[2].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    wr16(set->pools[2].bytes + 2, (int16_t)0x0802);
    wr16(set->pools[2].bytes + 4, mk_handle(2, 0));
    wr16(set->pools[2].bytes + 6, (int16_t)0x0806);

    /* DB3 (4-byte records): rec0 terminates; rec1 loops onto itself. */
    set->pools[3].record_size = 4;
    set->pools[3].record_count = 2;
    set->pools[3].source_base = 8;
    set->pools[3].bytes = calloc(2, 4);
    wr16(set->pools[3].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    wr16(set->pools[3].bytes + 4, mk_handle(3, 1));

    /* DB4 creature rec0: the acting creature, owns CAII slot 0. */
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 1;
    set->pools[4].source_base = 16;
    set->pools[4].bytes = calloc(1, 16);
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C; /* creature type */
    set->pools[4].bytes[5] = 0;    /* CAII slot 0 */

    set->valid = 1;
}

/* ── killer fixture: 4x4 map ────────────────────────────────────────
 * Flagged cells: (2,2) -> ground idx 0 (DB4 rec0, word@8 = 7),
 *                (3,3) -> ground idx 1 (DB4 rec1, word@8 = 9). */
static void build_killer_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
{
    memset(d, 0, sizeof(*d));
    memset(raw, 0, RAW_SIZE);
    d->level_count = 1;
    d->level_widths[0] = 4;
    d->level_heights[0] = 4;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = MAP_BASE;
    d->column_index_base = COLUMN_BASE;
    d->square_first_thing_base = GROUND_BASE;
    d->square_first_thing_count = 2;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 2 * 4 + 2] = 0x10; /* (2,2) */
    raw[MAP_BASE + 3 * 4 + 3] = 0x10; /* (3,3) */
    wr16(raw + COLUMN_BASE + 2 * 2, 0); /* column 2 base */
    wr16(raw + COLUMN_BASE + 3 * 2, 1); /* column 3 base */

    wr16(raw + GROUND_BASE + 0, mk_handle(4, 0));
    wr16(raw + GROUND_BASE + 2, mk_handle(4, 1));
}

static void build_killer_pools(DM2_V1_RecordPoolSet *set)
{
    memset(set, 0, sizeof(*set));

    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C;
    set->pools[4].bytes[5] = 0;    /* CAII slot 0 */
    wr16(set->pools[4].bytes + 8, 7); /* word@8 = 7 (filter match) */
    wr16(set->pools[4].bytes + 16, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[16 + 4] = 0x0C;
    set->pools[4].bytes[16 + 5] = 1;  /* CAII slot 1 */
    wr16(set->pools[4].bytes + 16 + 8, 9); /* word@8 = 9 (filter miss) */

    set->valid = 1;
}

typedef struct {
    int visits;
    int16_t last;
} WalkLog;

static int counting_visitor(void *context, int16_t handle,
                            const uint8_t *record)
{
    WalkLog *log = (WalkLog *)context;
    (void)record;
    ++log->visits;
    log->last = handle;
    return 0;
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;

    /* ── the walk primitive itself ───────────────────────────────── */
    build_xact_dungeon(&dungeon, raw);
    build_xact_pools(&set);

    CHECK(dm2_v1_tile_record_link(&dungeon, 0, 0, 0) == mk_handle(2, 1),
          "tile link returns the ground-stack head");
    CHECK(dm2_v1_tile_record_link(&dungeon, 0, 1, 0) ==
              DM2_V1_RECORD_HANDLE_END,
          "cell without the 0x10 flag yields OBJECT_END_MARKER");
    CHECK(dm2_v1_tile_record_link(NULL, 0, 0, 0) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "NULL dungeon fails closed");

    {
        WalkLog log;
        DM2_V1_TileRecordWalkReceipt rc;
        memset(&log, 0, sizeof(log));
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_tile_record_walk(&set, &dungeon, 0, 0, 0,
                                      counting_visitor, &log, &rc) == 1,
              "walk over the two-record chain completes");
        CHECK(log.visits == 2 && log.last == mk_handle(2, 0),
              "walk visits both DB2 records in link order");
        CHECK(rc.ended_at_marker == 1 && rc.records_visited == 2,
              "walk ends at OBJECT_END_MARKER");
    }
    {
        WalkLog log;
        DM2_V1_TileRecordWalkReceipt rc;
        memset(&log, 0, sizeof(log));
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_tile_record_walk(&set, &dungeon, 0, 1, 1,
                                      counting_visitor, &log, &rc) == 0,
              "corrupt self-loop chain is bounded, fail-closed");
        CHECK(rc.fail_closed == 1, "fail-closed receipted");
    }
    {
        WalkLog log;
        DM2_V1_TileRecordWalkReceipt rc;
        memset(&log, 0, sizeof(log));
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_tile_record_walk(&set, &dungeon, 0, 1, 0,
                                      counting_visitor, &log, &rc) == 1,
              "empty chain completes at the marker");
        CHECK(log.visits == 0 && rc.ended_at_marker == 1,
              "no records visited on an empty chain");
    }

    /* ── DM2_PROCEED_XACT_85 (c_ai.cpp:2078-2117) ────────────────── */
    dm2_v1_caii_array_init(&caii, 4);
    dm2_v1_source_timer_queue_init(&queue);
    caii.slots[0x17] = 0; /* slot 0: mode/dir bytes clear of 0x13 */
    caii.slots[0x1a] = 0;

    {
        DM2_V1_Xact85Receipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_proceed_xact_85(&set, &dungeon, &caii, &queue,
                                     0, 100, mk_handle(4, 0), 0, 0,
                                     &rc) == 1,
              "XACT_85 match path completes");
        CHECK(rc.match_found == 1 && rc.return_code == -2,
              "DB2 word@2 match returns -2");
        CHECK(rc.walk_records == 2,
              "the near-miss DB2 record was walked first");
        CHECK(caii.slots[0x1e] == 1 && caii.slots[0x1a] == 59,
              "slot byte@0x1e = 1 and byte@0x1a = 59 (s350.creatures)");
        CHECK(rc.slot_b1e_written == 1 && rc.slot_b1a == 59,
              "match writes receipted");
    }
    {
        DM2_V1_Xact85Receipt rc;
        memset(&rc, 0, sizeof(rc));
        caii.slots[0x1a] = 0; /* reset mode byte */
        CHECK(dm2_v1_proceed_xact_85(&set, &dungeon, &caii, &queue,
                                     0, 100, mk_handle(4, 0), 0, 1,
                                     &rc) == 1,
              "XACT_85 walk-end path completes");
        CHECK(rc.match_found == 0 && rc.return_code == -3,
              "walk end returns -3");
        CHECK(rc.ai_stop_invoked == 1,
              "bound DM2_ai_13e4_0360 AI-stop invoked (dir 0x13)");
        CHECK(caii.slots[0x1a] == 51,
              "slot byte@0x1a = 51 after the walk end");
        CHECK(rc.walk_records == 1 && rc.walk_db_break == 0,
              "DB3 record walked, no DB break");
    }
    {
        DM2_V1_Xact85Receipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_proceed_xact_85(&set, &dungeon, &caii, &queue,
                                     0, 100, mk_handle(4, 0), 1, 1,
                                     &rc) == 0,
              "corrupt chain under XACT_85 fails closed");
        CHECK(rc.walk_fail_closed == 1, "walk fail-closed receipted");
        CHECK(dm2_v1_proceed_xact_85(&set, &dungeon, &caii, &queue,
                                     0, 100, mk_handle(4, 9), 0, 0,
                                     &rc) == 0,
              "unresolvable creature record (no CAII slot) fails closed");
    }

    dm2_v1_record_pool_set_free(&set);

    /* ── DM2_ACTIVATE_CREATURE_KILLER (c_tim_proc.cpp:2907-2988) ── */
    build_killer_dungeon(&dungeon, raw);
    build_killer_pools(&set);
    dm2_v1_caii_array_free(&caii);
    dm2_v1_caii_array_init(&caii, 4);
    dm2_v1_source_timer_queue_init(&queue);
    caii.slots[0x17] = 0;
    caii.slots[0x1a] = 0;
    caii.slots[DM2_V1_CAII_SLOT_SIZE + 0x17] = 0;
    caii.slots[DM2_V1_CAII_SLOT_SIZE + 0x1a] = 0;

    {
        DM2_V1_CreatureKillerWalkReceipt rc;
        memset(&rc, 0, sizeof(rc));
        /* 3x3 sweep around (2,2): ebx=ecx=2, radii |2-1| = 1. */
        CHECK(dm2_v1_activate_creature_killer_walk(
                  &set, &dungeon, &caii, &queue, NULL, 0, 100, 4, 4,
                  2, 7, 2, 2, 1, 1, 0xb, 0, 0, 0, &rc) == 1,
              "killer sweep completes");
        CHECK(rc.cells_visited == 9 && rc.cells_out_of_bounds == 0,
              "all 9 in-bounds cells visited");
        CHECK(rc.creatures_found == 2, "both creatures resolved");
        CHECK(rc.type_filter_miss == 1,
              "word@8 filter rejects the second creature");
        CHECK(rc.ai_stops == 1, "mode 2 AI-stops the matching creature");
    }
    {
        DM2_V1_CreatureKillerWalkReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_activate_creature_killer_walk(
                  &set, &dungeon, &caii, &queue, NULL, 0, 100, 4, 4,
                  1, 7, 2, 2, 1, 1, 0xb, 0, 0, 0, &rc) == 1,
              "mode 1 sweep completes without stops");
        CHECK(rc.ai_stops == 0 && rc.creatures_found == 2,
              "mode 1 skips every creature");
    }
    {
        DM2_V1_CreatureKillerWalkReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_activate_creature_killer_walk(
                  &set, &dungeon, &caii, &queue, NULL, 0, 100, 4, 4,
                  3, 7, 2, 2, 1, 1, 0xb, 0, 0, 0, &rc) == 1,
              "mode above 2 completes through the source abort");
        CHECK(rc.aborted == 1, "mode > 2 aborts the sweep");
    }
    {
        DM2_V1_CreatureKillerWalkReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_activate_creature_killer_walk(
                  &set, &dungeon, &caii, &queue, NULL, 0, 100, 4, 4,
                  0x20, 7, 2, 2, 1, 1, 0x28, 1, 5, 6, &rc) == 1,
              "action 0x28 sweep completes");
        CHECK(rc.attacks == 1,
              "attack invoked on the filter-matching creature");
        CHECK(rc.attacks_completed == 0,
              "unwired attack providers fail closed, never simulated");
    }
    {
        DM2_V1_CreatureKillerWalkReceipt rc;
        memset(&rc, 0, sizeof(rc));
        /* sweep x/y in {-1,0,1} around (0,0): 5 cells out of bounds */
        CHECK(dm2_v1_activate_creature_killer_walk(
                  &set, &dungeon, &caii, &queue, NULL, 0, 100, 4, 4,
                  2, 7, 0, 0, 1, 1, 0xb, 0, 0, 0, &rc) == 1,
              "edge sweep completes");
        CHECK(rc.cells_visited == 4 && rc.cells_out_of_bounds == 5,
              "out-of-bounds cells skipped by the map bounds check");
        CHECK(rc.creatures_found == 0, "no creatures in the edge sweep");
    }
    {
        DM2_V1_CreatureKillerWalkReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_activate_creature_killer_walk(
                  &set, &dungeon, &caii, &queue, NULL, 0, 100, 4, 4,
                  2, 7, 2, 2, 1, 1, 0x07, 0, 0, 0, &rc) == 1,
              "unknown action completes, receipted");
        CHECK(rc.unknown_action == 1 && rc.ai_stops == 0 &&
                  rc.attacks == 0,
              "action neither 0xb nor 0x28 receipted, never simulated");
    }

    CHECK(strstr(dm2_v1_tile_record_walk_source_evidence(),
                 "c_tim_proc.cpp") != NULL,
          "source evidence cites c_tim_proc.cpp");

    dm2_v1_record_pool_set_free(&set);
    dm2_v1_caii_array_free(&caii);

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_tile_record_walk_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_tile_record_walk_pc34_compat: all checks passed\n");
    return 0;
}
