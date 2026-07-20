/*
 * test_dm2_v1_caii_free_pc34_compat.c — CAII slot free
 * DM2_1c9a_0fcb (bounded compatibility slice).
 *
 * Verifies the skproject boundary (c_1c9a.cpp:5896-5944):
 *   - out-of-range slot indexes fail closed (the source would index
 *     out of bounds at slot == capacity, c_1c9a.cpp:5905);
 *   - already-free slots take the source early return;
 *   - the DB4 handle rebuild (slot word@0 | 0x1000, c_1c9a.cpp:5915);
 *   - the pending timer is deleted through the bound DM2_1c9a_0db0
 *     path (c_1c9a.cpp:5933);
 *   - slot byte@1a = 0, alloc counter--, record byte@5 = -1, slot
 *     word@0 = -1 (c_1c9a.cpp:5932-5941);
 *   - the DM2_DELETE_CREATURE_RECORD branch stays unbound (receipted,
 *     never simulated) — the AI-spec flag table owner is unproven;
 *   - a freed slot is reusable by a later allocation (lifecycle).
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
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C;
    set->pools[4].bytes[5] = 0xFF;
    wr16(set->pools[4].bytes + 8, (int16_t)0xffff);
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
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiFreeReceipt rc;
    const uint8_t *slot;

    build_dungeon(&dungeon, raw);
    build_pools(&set);
    dm2_v1_caii_array_init(&caii, 2);
    dm2_v1_source_timer_queue_init(&queue);

    /* ── (a) guard paths ───────────────────────────────────────────── */
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, -1, &rc) == 0 &&
              rc.out_of_range == 1,
          "negative slot index fails closed");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, 2, &rc) == 0 &&
              rc.out_of_range == 1,
          "slot == capacity fails closed (source would index OOB)");
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, 0, &rc) == 0 &&
              rc.already_free == 1,
          "already-free slot takes the source early return");
    CHECK(dm2_v1_caii_free_slot(NULL, &caii, &queue, 0, &rc) == 0,
          "null pool set rejected");

    /* ── (b) alloc two, free the first with a pending timer ────────── */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 100ul, rec_handle(0),
                                        0, 0, &alloc) == 1,
          "first creature activates");
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 100ul, rec_handle(1),
                                        0, 1, &alloc) == 1,
          "second creature activates");
    CHECK(queue.count == 2 && caii.alloc_count == 2,
          "two timers pending, two slots owned");

    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, 0, &rc) == 1,
          "freeing slot 0 succeeds");
    CHECK(rc.valid == 1 && rc.freed == 1 && rc.record_index == 0,
          "free receipted with the rebuilt record index");
    CHECK(rc.had_pending_timer == 1,
          "pending timer deleted through bound DM2_1c9a_0db0");
    CHECK(rc.record_delete_unbound == 1,
          "DM2_DELETE_CREATURE_RECORD branch stays unbound");
    CHECK(queue.count == 1, "freed slot's timer removed from the queue");
    CHECK(caii.alloc_count == 1, "alloc counter decremented");
    CHECK(set.pools[4].bytes[5] == 0xFF,
          "record byte@5 cleared to -1");
    slot = dm2_v1_caii_slot(&caii, 0);
    CHECK(slot != NULL && (int16_t)rd16(slot + 0) < 0 &&
              slot[0x1a] == 0,
          "slot marked free, mode byte cleared");
    CHECK(set.pools[4].bytes[16 + 5] == 1,
          "second creature untouched");

    /* ── (c) lifecycle: the freed slot is reusable ─────────────────── */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_caii_alloc_to_creature(&set, &dungeon, &caii, &queue,
                                        0, 200ul, rec_handle(0),
                                        0, 0, &alloc) == 1 &&
              alloc.slot_index == 0,
          "freed slot reusable by a later activation");
    CHECK(queue.count == 2 && caii.alloc_count == 2,
          "queue and counter consistent after reuse");

    /* free without a pending timer: delete the timer first, then free */
    memset(&rc, 0, sizeof(rc));
    {
        DM2_V1_CaiiDeleteTimerReceipt del;
        memset(&del, 0, sizeof(del));
        CHECK(dm2_v1_caii_delete_timer(&set, &caii, &queue,
                                       rec_handle(0), &del) == 1,
              "timer deleted before free");
    }
    memset(&rc, 0, sizeof(rc));
    CHECK(dm2_v1_caii_free_slot(&set, &caii, &queue, 0, &rc) == 1 &&
              rc.had_pending_timer == 0,
          "free without a pending timer receipted correctly");
    CHECK(strstr(rc.source_evidence, "c_1c9a.cpp:5896-5944") != NULL,
          "source evidence cites DM2_1c9a_0fcb");

    dm2_v1_caii_array_free(&caii);
    dm2_v1_record_pool_set_free(&set);

    if (g_failures != 0) {
        fprintf(stderr,
                "dm2_v1_caii_free_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_caii_free_pc34_compat: all checks passed\n");
    return 0;
}
