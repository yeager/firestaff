/*
 * test_dm2_v1_think_creature_pc34_compat.c — per-cell DM2_THINK_CREATURE
 * binding over the DM2-002 record pool.
 *
 * Verifies the skproject boundary:
 *   c_tim_proc.cpp:4079-4088  0x21/0x22 payload decode (x=getxA, y=getyA,
 *                             type word, timer map)
 *   c_ai.cpp:5670-5673        DM2_THINK_CREATURE early return when the cell
 *                             holds no creature (timer still consumed)
 *   c_querydb.cpp:1486-1507   DM2_GET_CREATURE_AT: first DB4 record on the
 *                             cell chain, direction bits ignored for the
 *                             DB test but preserved in the returned word
 *   c_map.cpp:44-69           tile record link (byte-square bit 0x10 +
 *                             column-index ground-stack table)
 */

#include "dm2_v1_think_creature_pc34_compat.h"

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

/* ── synthetic map (square_bytes == 1) ──────────────────────────────
 * Level 0 is 2x3.  Object-flag cells: (0,0) -> ground idx 0, (0,1) ->
 * ground idx 1, (1,1) -> ground idx 2. */
#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128
#define RAW_SIZE 160

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

    /* Object flags: (0,0) idx0, (0,1) idx1, (1,1) idx4. */
    raw[MAP_BASE + 0] = 0x10; /* (0,0) */
    raw[MAP_BASE + 1] = 0x10; /* (0,1) */
    raw[MAP_BASE + 4] = 0x10; /* (1,1) */

    /* Column bases: column 0 starts at ground idx 0, column 1 at idx 2. */
    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);
}

static void build_pools(DM2_V1_RecordPoolSet *set,
                        uint8_t *raw)
{
    memset(set, 0, sizeof(*set));

    /* DB4 creature pool: two 16-byte records.  Record 0 is a creature of
     * type 0x0C whose own link terminates. */
    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    wr16(set->pools[4].bytes + 0, DM2_V1_RECORD_HANDLE_END);
    set->pools[4].bytes[4] = 0x0C; /* creature type (c_ai.cpp:5849-5858) */
    wr16(set->pools[4].bytes + 16, DM2_V1_RECORD_HANDLE_END);

    /* DB5 weapon pool: three 4-byte records used as non-creature chain
     * links.  rec0 -> DB4 rec0 (direction bits set on the link word),
     * rec1 terminates, rec2 loops onto itself (corrupt chain). */
    set->pools[5].record_size = 4;
    set->pools[5].record_count = 3;
    set->pools[5].source_base = 32;
    set->pools[5].bytes = calloc(3, 4);
    wr16(set->pools[5].bytes + 0,
         (int16_t)((1u << 14) | (4u << 10) | 0u)); /* -> creature, dir 1 */
    wr16(set->pools[5].bytes + 4, DM2_V1_RECORD_HANDLE_END);
    wr16(set->pools[5].bytes + 8, mk_handle(5, 2)); /* self-loop */

    set->valid = 1;

    /* Ground-stack heads:
     * idx 0: (0,0) -> weapon rec0 with direction bits (chain to creature)
     * idx 1: (0,1) -> weapon rec1 (chain without a creature)
     * idx 2: (1,1) -> weapon rec2 (corrupt self-loop chain) */
    wr16(raw + GROUND_BASE + 0,
         (int16_t)((2u << 14) | (5u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2, mk_handle(5, 1));
    wr16(raw + GROUND_BASE + 4, mk_handle(5, 2));
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

typedef struct {
    int calls;
    int16_t last_record;
    int last_map;
    int last_x;
    int last_y;
    int last_type;
} BodyLog;

static int logging_body(void *context, int16_t record,
                        const DM2_V1_SourceTimer *timer,
                        int map, int x, int y, int think_type)
{
    BodyLog *log = (BodyLog *)context;
    (void)timer;
    log->calls++;
    log->last_record = record;
    log->last_map = map;
    log->last_x = x;
    log->last_y = y;
    log->last_type = think_type;
    return think_type == DM2_V1_TIMER_THINK_CREATURE_A ? 1 : 0;
}

int main(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_RecordPoolSet set;
    DM2_V1_ThinkCreatureBinding binding;
    int16_t creature;

    build_dungeon(&dungeon, raw);
    build_pools(&set, raw);

    /* ── DM2_GET_CREATURE_AT (c_querydb.cpp:1486-1507) ───────────── */
    creature = dm2_v1_get_creature_at(&set, &dungeon, 0, 0, 0);
    CHECK(creature == (int16_t)((1u << 14) | (4u << 10) | 0u),
          "first DB4 record on the cell chain, direction bits preserved");
    CHECK(dm2_v1_record_handle_pool(creature) == 4,
          "direction bits masked from the DB test");

    CHECK(dm2_v1_get_creature_at(&set, &dungeon, 0, 0, 1) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "chain without a creature returns the source 0xffff");
    CHECK(dm2_v1_get_creature_at(&set, &dungeon, 0, 0, 2) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "cell without the 0x10 object flag has no tile link");
    CHECK(dm2_v1_get_creature_at(&set, &dungeon, 0, 1, 1) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "corrupt self-loop chain is bounded, fail-closed");
    CHECK(dm2_v1_get_creature_at(&set, &dungeon, 0, -1, 0) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "out-of-bounds cell fails closed");
    CHECK(dm2_v1_get_creature_at(NULL, &dungeon, 0, 0, 0) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "NULL pool set fails closed");
    CHECK(dm2_v1_get_creature_at(&set, NULL, 0, 0, 0) ==
              DM2_V1_RECORD_HANDLE_NULL,
          "NULL dungeon fails closed");

    /* ── handler decode + source early return ────────────────────── */
    dm2_v1_think_creature_binding_init(&binding, &set, &dungeon);
    {
        DM2_V1_SourceTimer t =
            mk_think_timer(7, 0, DM2_V1_TIMER_THINK_CREATURE_A, 0, 0);
        DM2_V1_ProceedTimersReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_think_creature_timer_handler(&binding, &t, 0, &rc) == 1,
              "think timer consumed like the source's void return");
        CHECK(binding.receipt.think_timers == 1, "think timer counted");
        CHECK(binding.receipt.resolved == 1, "creature resolved at cell");
        CHECK(binding.receipt.last_record ==
                  (int16_t)((1u << 14) | (4u << 10) | 0u),
              "resolved record receipted");
        CHECK(binding.receipt.last_creature_type == 0x0C,
              "creature type read from DB4 record byte@4");
        CHECK(binding.receipt.last_map == 0 && binding.receipt.last_x == 0 &&
                  binding.receipt.last_y == 0 &&
                  binding.receipt.last_type == DM2_V1_TIMER_THINK_CREATURE_A,
              "payload decode receipted (map, x, y, type)");
        CHECK(binding.receipt.body_unbound == 1,
              "no DM2-owned think body: fail-closed, never simulated");
    }

    /* Cell without a creature: source early return, timer consumed. */
    {
        DM2_V1_SourceTimer t =
            mk_think_timer(7, 0, DM2_V1_TIMER_THINK_CREATURE_B, 0, 1);
        DM2_V1_ProceedTimersReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_think_creature_timer_handler(&binding, &t, 0, &rc) == 1,
              "empty-cell think timer consumed (source early return)");
        CHECK(binding.receipt.no_creature_at_cell == 1,
              "early return receipted");
        CHECK(binding.receipt.last_type == DM2_V1_TIMER_THINK_CREATURE_B,
              "0x22 type word receipted");
    }

    /* Wrong type / incomplete binding reject. */
    {
        DM2_V1_SourceTimer t =
            mk_think_timer(7, 0, DM2_V1_TIMER_STEP_MISSILE, 0, 0);
        DM2_V1_ThinkCreatureBinding empty;
        DM2_V1_ProceedTimersReceipt rc;
        memset(&rc, 0, sizeof(rc));
        CHECK(dm2_v1_think_creature_timer_handler(&binding, &t, 0, &rc) == 0,
              "non-think type rejected");
        memset(&empty, 0, sizeof(empty));
        CHECK(dm2_v1_think_creature_timer_handler(&empty, &t, 0, &rc) == 0,
              "incomplete binding fails closed");
    }

    /* ── bound think body receives the resolved record ───────────── */
    {
        BodyLog log;
        DM2_V1_SourceTimer ta =
            mk_think_timer(8, 0, DM2_V1_TIMER_THINK_CREATURE_A, 0, 0);
        DM2_V1_SourceTimer tb =
            mk_think_timer(8, 0, DM2_V1_TIMER_THINK_CREATURE_B, 0, 0);
        DM2_V1_ProceedTimersReceipt rc;
        memset(&log, 0, sizeof(log));
        memset(&rc, 0, sizeof(rc));
        binding.think_body = logging_body;
        binding.think_body_context = &log;
        CHECK(dm2_v1_think_creature_timer_handler(&binding, &ta, 0, &rc) == 1,
              "0x21 consumed with bound body");
        CHECK(log.calls == 1 && log.last_record ==
                  (int16_t)((1u << 14) | (4u << 10) | 0u) &&
                  log.last_map == 0 && log.last_x == 0 && log.last_y == 0 &&
                  log.last_type == DM2_V1_TIMER_THINK_CREATURE_A,
              "body received record + decoded payload");
        CHECK(binding.receipt.body_consumed == 1, "body consume receipted");
        CHECK(dm2_v1_think_creature_timer_handler(&binding, &tb, 0, &rc) == 1,
              "0x22 consumed even when the body rejects");
        CHECK(binding.receipt.body_rejected == 1, "body rejection receipted");
        binding.think_body = NULL;
        binding.think_body_context = NULL;
    }

    /* ── integration through DM2_PROCEED_TIMERS ───────────────────── */
    {
        DM2_V1_SourceTimerQueue queue;
        DM2_V1_TimerDispatcher dispatcher;
        DM2_V1_ProceedTimersReceipt rc;
        DM2_V1_ThinkCreatureBinding run_binding;

        dm2_v1_think_creature_binding_init(&run_binding, &set, &dungeon);
        dm2_v1_source_timer_queue_init(&queue);
        memset(&dispatcher, 0, sizeof(dispatcher));
        dispatcher.context = &run_binding;
        dispatcher.handlers[DM2_V1_TIMER_THINK_CREATURE_A] =
            dm2_v1_think_creature_timer_handler;
        dispatcher.handlers[DM2_V1_TIMER_THINK_CREATURE_B] =
            dm2_v1_think_creature_timer_handler;
        {
            DM2_V1_SourceTimer a =
                mk_think_timer(3, 0, DM2_V1_TIMER_THINK_CREATURE_A, 0, 0);
            DM2_V1_SourceTimer b =
                mk_think_timer(3, 0, DM2_V1_TIMER_THINK_CREATURE_B, 0, 1);
            (void)dm2_v1_source_timer_enqueue(&queue, &a, 0);
            (void)dm2_v1_source_timer_enqueue(&queue, &b, 1);
        }
        CHECK(dm2_v1_proceed_timers(&queue, 3, &dispatcher, &rc) == 1,
              "dispatcher ran");
        CHECK(rc.due_count == 2 && rc.dispatched_count == 2 &&
                  rc.fail_closed_count == 0,
              "both think timers dispatched through the bound handler");
        CHECK(run_binding.receipt.think_timers == 2 &&
                  run_binding.receipt.resolved == 1 &&
                  run_binding.receipt.no_creature_at_cell == 1 &&
                  run_binding.receipt.body_unbound == 1,
              "per-cell resolution receipted across the dispatch");
        CHECK(rc.type_tally[DM2_V1_TIMER_THINK_CREATURE_A] == 1 &&
                  rc.type_tally[DM2_V1_TIMER_THINK_CREATURE_B] == 1,
              "type tally per source type");
    }

    CHECK(strstr(dm2_v1_think_creature_source_evidence(),
                 "c_querydb.cpp") != NULL,
          "source evidence cites c_querydb.cpp");

    dm2_v1_record_pool_set_free(&set);

    if (g_failures != 0) {
        fprintf(stderr, "dm2_v1_think_creature_pc34_compat: %d failure(s)\n",
                g_failures);
        return 1;
    }
    printf("dm2_v1_think_creature_pc34_compat: all checks passed\n");
    return 0;
}
