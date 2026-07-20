/*
 * test_dm2_v1_caii_free_runtime_pc34_compat.c — CAII slot free wired
 * into the runtime (DM2-003/005 follow-up).
 *
 * Verifies the full slot lifecycle over the runtime boundaries:
 *   activation (alloc + first timer)
 *     -> free (DM2_1c9a_0fcb bounded slice: pending timer cancelled
 *        through bound DM2_1c9a_0db0, record byte@5 cleared, slot
 *        marked free)
 *     -> next tick dispatches NOTHING for the freed creature
 *     -> re-activation reuses the freed slot and the think chain
 *        resumes end-to-end.
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
    raw[DB4_BASE + 4] = 0x0C;
    raw[DB4_BASE + 5] = 0xFF;
    wr16(raw + DB4_BASE + 8, (int16_t)0xffff);
    wr16(raw + DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
}

static void test_runtime_caii_slot_lifecycle(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CaiiFreeReceipt free_rc;
    DM2_V1_ThinkCreatureReceipt think;

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    (void)dm2_v1_runtime_caii_init(4);

    /* Free without an allocation fails closed. */
    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 0 &&
              free_rc.already_free == 1,
          "free before activation takes the source early return");

    /* Activate, then free before the timer fires. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1 &&
              alloc.slot_index == 0,
          "activation allocates slot 0 + first timer");

    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 1,
          "runtime boundary frees the slot");
    CHECK(free_rc.freed == 1 && free_rc.had_pending_timer == 1 &&
              free_rc.record_delete_unbound == 1,
          "free receipt: pending timer cancelled, record branch unbound");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 0,
          "alloc counter back to zero");

    /* Next tick: nothing dispatches for the freed creature. */
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1 &&
              think.think_timers == 0,
          "freed creature's timer never reaches the think binding");

    /* Re-activation reuses the freed slot; the chain resumes. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1 &&
              alloc.slot_index == 0,
          "re-activation reuses the freed slot");
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1 &&
              think.think_timers == 1 && think.resolved == 1 &&
              think.last_creature_type == 0x0C,
          "think chain resumes end-to-end after reuse");
}

static void test_runtime_caii_free_fail_closed_without_session(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiFreeReceipt free_rc;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);

    memset(&free_rc, 0, sizeof(free_rc));
    CHECK(dm2_v1_runtime_free_caii_slot(0, &free_rc) == 0,
          "no CAII session: free boundary stays unready");
}

int main(void)
{
    printf("DM2 V1 CAII slot free runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_1c9a.cpp:5896-5944 (DM2_1c9a_0fcb)\n");
    printf("        skproject/SKULLWIN/c_1c9a.cpp:5734-5763 (DM2_1c9a_0db0)\n\n");

    test_runtime_caii_slot_lifecycle();
    test_runtime_caii_free_fail_closed_without_session();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
