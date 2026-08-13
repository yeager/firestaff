/*
 * test_dm2_v1_caii_alloc_pc34_compat.c — CAII slot allocator
 * DM2_ALLOC_CAII_TO_CREATURE (bounded compatibility slice).
 *
 * Verifies the skproject boundary (c_1c9a.cpp:5772-5894):
 *   - the source early return when the record already owns a slot
 *     (record byte@5 != 0xff, c_1c9a.cpp:5783-5784);
 *   - free-slot scan over 34-byte slots, free = signed word@0 < 0;
 *   - slot init: word@0 = bare record index (handle & 0x3ff,
 *     c_1c9a.cpp:5813-5815 + 5915), word@2 = -1, byte@6 =
 *     (gametick >> 2) - 1, byte@4 = gametick - 0x7f, word@0xc =
 *     (x & 0x1f) | ((y & 0x1f) << 5) | ((map & 0x3f) << 10),
 *     byte@0x16/0x17 = -1, byte@7 = 0, record byte@5 = slot index,
 *     alloc counter++ (c_1c9a.cpp:5809-5866);
 *   - the bound scheduling producer queues the first 0x21/0x22 timer
 *     (c_1c9a.cpp:5860);
 *   - slot byte@1a = 0 for grouped, 0x11 for ungrouped records
 *     (c_1c9a.cpp:5861-5866);
 *   - the no-free-slot path fails closed without mutation (the source
 *     recycle path c_1c9a.cpp:5880-5891 is unproven);
 *   - PREPARE/UNPREPARE local var + s350 group scan stay host-owned
 *     (receipted, never simulated).
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

/* ── synthetic map (square_bytes == 1) ──────────────────────────────
 * Level 0 is 2x3.  (0,0) -> creature rec0 (ungrouped), (0,1) ->
 * creature rec1 (grouped). */
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
    /* rec0: creature type 0x0C, byte@5 = -1 (no slot), no group link. */
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C;
    set->pools[4].bytes[5] = 0xFF;
    wr16(set->pools[4].bytes + 8, (int16_t)0xffff);
    /* rec1: creature type 0x07, byte@5 = -1, group/leader link 0x0003. */
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

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_SourceTimerQueue queue;
    DM2_V1_CaiiAllocReceipt rc;
    const uint8_t *slot;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_caii_array_init(&caii, 4);
    dm2_v1_source_timer_queue_init(&queue);

    /* ── (a) ungrouped creature: slot init + 0x21 timer ────────────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 100ul, rec_handle(0),
                                        0, 0, &rc) == 1,
          "ungrouped creature allocates a CAII slot");
    CHECK(rc.valid == 1 && rc.allocated == 1 && rc.slot_index == 0,
          "receipt valid/allocated with slot 0");
    CHECK(rc.record_flag_rewrite_noop == 1,
          "word@0xe bit-10 dance receipted as a no-op");
    CHECK(rc.local_var_unbound == 1 && rc.group_scan_unbound == 1,
          "CCM local var and s350 group scan stay host-owned");
    CHECK(rc.timer_scheduled == 1 &&
              rc.timer_type == DM2_V1_TIMER_THINK_CREATURE_A &&
              rc.due_tick == 101ul,
          "producer queued the first 0x21 timer due next tick");
    CHECK(caii.alloc_count == 1, "alloc counter tracks ddat.v1d4020");

    slot = dm2_v1_caii_slot(&caii, 0);
    CHECK(slot != NULL, "slot 0 readable");
    CHECK(rd16(slot + 0) == 0,
          "slot word@0 stores the bare record index (handle & 0x3ff)");
    CHECK(rd16(slot + 2) != 0xffffu && rd16(slot + 2) != 0u,
          "slot word@2 stores the issued timer ticket "
          "(c_1c9a.cpp:5724-5728)");
    CHECK(slot[6] == (uint8_t)((100ul >> 2) - 1u),
          "slot byte@6 = (gametick >> 2) - 1");
    CHECK(slot[4] == (uint8_t)(100ul - 0x7ful),
          "slot byte@4 = gametick - 0x7f");
    CHECK(rd16(slot + 0xc) == (uint16_t)(0 | (0 << 5) | (0 << 10)),
          "slot word@0xc packs x | y<<5 | map<<10");
    CHECK(slot[0x16] == 0xFF && slot[0x17] == 0xFF && slot[7] == 0,
          "slot bytes 0x16/0x17 = -1, byte@7 = 0");
    CHECK(slot[0x1a] == 0x11u,
          "ungrouped record starts in mode 0x11");
    CHECK(set.pools[4].bytes[5] == 0,
          "record byte@5 back-references the slot");
    CHECK(queue.count == 1, "exactly one timer queued");

    /* ── (b) second alloc for the same record: source early return ── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 101ul, rec_handle(0),
                                        0, 0, &rc) == 0,
          "record with a slot takes the source early return");
    CHECK(rc.already_allocated == 1 && rc.allocated == 0,
          "early return receipted as already_allocated");
    CHECK(queue.count == 1 && caii.alloc_count == 1,
          "early return never mutates queue or counter");

    /* ── (c) grouped creature: mode 0x00 + 0x22 timer ──────────────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 7ul, rec_handle(1),
                                        0, 1, &rc) == 1,
          "grouped creature allocates a CAII slot");
    CHECK(rc.slot_index == 1 && rc.timer_type == DM2_V1_TIMER_THINK_CREATURE_B,
          "second slot with a 0x22 timer");
    slot = dm2_v1_caii_slot(&caii, 1);
    CHECK(slot != NULL && slot[0x1a] == 0x00u,
          "grouped record starts in mode 0x00");
    CHECK(rd16(slot + 0) == 1 && set.pools[4].bytes[16 + 5] == 1,
          "slot and record cross-reference each other");
    CHECK(rd16(slot + 0xc) == (uint16_t)(0 | (1 << 5) | (0 << 10)),
          "grouped slot packs the activation cell");
    CHECK(queue.count == 2 && caii.alloc_count == 2,
          "queue and counter advance per allocation");

    /* ── (d) no free slot fails closed without mutation ────────────── */
    {
        DM2_V1_CaiiArray tiny;
        DM2_V1_RecordPoolSet set2;
        static uint8_t raw2[RAW_SIZE];
        DM2_V1_DungeonData dungeon2;
        DM2_V1_SourceTimerQueue queue2;

        build_dungeon(&dungeon2, raw2);
        build_pools(&set2);
        dm2_v1_caii_array_init(&tiny, 1);
        dm2_v1_source_timer_queue_init(&queue2);

        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_caii_alloc_to_creature(&set2, &dungeon2, &tiny,
                                            &queue2, 0, 1ul,
                                            rec_handle(0), 0, 0, &rc) == 1,
              "single-slot array takes the first creature");
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_caii_alloc_to_creature(&set2, &dungeon2, &tiny,
                                            &queue2, 0, 1ul,
                                            rec_handle(1), 0, 1, &rc) == 0,
              "full array fails closed (recycle path unproven)");
        CHECK(rc.no_free_slot == 1 && rc.allocated == 0,
              "no-free-slot receipted");
        CHECK(set2.pools[4].bytes[16 + 5] == 0xFF,
              "failed allocation never touches the record");
        CHECK(queue2.count == 1 && tiny.alloc_count == 1,
              "failed allocation never mutates queue or counter");

        dm2_v1_caii_array_free(&tiny);
        dm2_v1_record_pool_set_free(&set2);
    }

    /* ── (e) argument validation fails closed ──────────────────────── */
    CHECK(dm2_v1_caii_alloc_to_creature(NULL, &dungeon, &caii, &queue,
                                        0, 1ul, rec_handle(0), 0, 0,
                                        &rc) == 0,
          "null pool set rejected");
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, NULL, &queue,
                                        0, 1ul, rec_handle(0), 0, 0,
                                        &rc) == 0,
          "null CAII array rejected");
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 1ul, DM2_V1_RECORD_HANDLE_NULL,
                                        0, 0, &rc) == 0,
          "null record handle rejected");
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0x40, 1ul, rec_handle(0), 0, 0,
                                        &rc) == 0,
          "out-of-range map rejected");

    /* ── (f) source evidence cites the allocator ───────────────────── */
    memset(&rc, 0, sizeof(rc));
    {
        DM2_V1_RecordPoolSet set3;
        static uint8_t raw3[RAW_SIZE];
        DM2_V1_DungeonData dungeon3;
        DM2_V1_CaiiArray caii3;
        DM2_V1_SourceTimerQueue queue3;

        build_dungeon(&dungeon3, raw3);
        build_pools(&set3);
        dm2_v1_caii_array_init(&caii3, 2);
        dm2_v1_source_timer_queue_init(&queue3);
        (void)dm2_v1_caii_alloc_to_creature(&set3, &dungeon3, &caii3,
                                            &queue3, 0, 1ul,
                                            rec_handle(0), 0, 0, &rc);
        CHECK(strstr(rc.source_evidence, "c_1c9a.cpp:5772-5894") != NULL,
              "source evidence cites DM2_ALLOC_CAII_TO_CREATURE");
        dm2_v1_caii_array_free(&caii3);
        dm2_v1_record_pool_set_free(&set3);
    }

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_alloc_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_alloc_pc34_compat: all checks passed\n");
    return 0;
}
