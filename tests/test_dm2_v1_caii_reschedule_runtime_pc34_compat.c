/*
 * test_dm2_v1_caii_reschedule_runtime_pc34_compat.c — complete
 * DM2_1c9a_0cf7 replacement slice wired into the runtime
 * (DM2-003/005 follow-up).
 *
 * Verifies the self-maintaining creature timer chain:
 *   activation (CAII alloc + first timer)
 *     -> reschedule (DM2_1c9a_0db0 deletes the pending timer,
 *        c_1c9a.cpp:5699-5706; fresh timer queued, slot word@2 updated,
 *        c_1c9a.cpp:5724-5728)
 *     -> exactly ONE timer reaches the think binding on the next tick
 *        (no duplicate accumulation).
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
    raw[DB4_BASE + 5] = 0xFF;                  /* byte@5: no CAII slot */
    wr16(raw + DB4_BASE + 8, (int16_t)0xffff);
    wr16(raw + DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
}

static void test_runtime_reschedule_replaces_pending_timer(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_CreatureScheduleReceipt sched;
    DM2_V1_ThinkCreatureReceipt think;

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    (void)dm2_v1_runtime_caii_init(4);

    /* Activate: slot + first think timer. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1,
          "activation allocates slot + first timer");

    /* Reschedule before the timer fires: the pending timer is
     * replaced, never duplicated. */
    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_runtime_reschedule_creature_at(0, 0, &sched) == 1,
          "reschedule queues the replacement timer");
    CHECK(sched.replaced_existing == 1,
          "pending timer replaced through bound DM2_1c9a_0db0");
    CHECK(sched.timer_type == DM2_V1_TIMER_THINK_CREATURE_A &&
              sched.creature_type == 0x0C && sched.due_tick == 1ul,
          "replacement timer carries the source tuple");

    /* A creature without a CAII slot fails closed. */
    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_runtime_reschedule_creature_at(0, 1, &sched) == 0,
          "cell without a creature fails closed");

    /* Next tick: exactly ONE think timer dispatches. */
    dm2_v1_runtime_tick();

    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1,
          "think binding receipt published");
    CHECK(think.valid == 1 && think.think_timers == 1 &&
              think.resolved == 1 && think.last_creature_type == 0x0C,
          "exactly one timer reached the think binding "
          "(no duplicate accumulation)");

    /* After the pop, the slot timer word is stale; the reschedule
     * boundary clears it and still schedules (source: the word only
     * ever references live timers — here the stale-reference guard
     * keeps the chain fail-safe). */
    memset(&sched, 0, sizeof(sched));
    CHECK(dm2_v1_runtime_reschedule_creature_at(0, 0, &sched) == 1,
          "post-dispatch reschedule still schedules");
    dm2_v1_runtime_tick();
    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1 &&
              think.think_timers >= 2,
          "second dispatch consumed at least one more timer");
}

int main(void)
{
    printf("DM2 V1 creature timer replacement runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_1c9a.cpp:5695-5728 (complete)\n");
    printf("        skproject/SKULLWIN/c_1c9a.cpp:5734-5763 (DM2_1c9a_0db0)\n");
    printf("        skproject/SKULLWIN/c_timer.cpp:215-257 (queue/delete)\n\n");

    test_runtime_reschedule_replaces_pending_timer();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
