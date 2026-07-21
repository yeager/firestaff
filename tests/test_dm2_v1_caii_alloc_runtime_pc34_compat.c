/*
 * test_dm2_v1_caii_alloc_runtime_pc34_compat.c — CAII lazy
 * creature-activation wired into the runtime (DM2-003/005 follow-up).
 *
 * Verifies the end-to-end activation chain:
 *   dm2_v1_runtime_alloc_caii_at
 *     (record resolved at the activation cell via DM2_GET_CREATURE_AT,
 *      the DM2_ATTACK_CREATURE reach path c_creature.cpp:347-352)
 *       -> bounded DM2_ALLOC_CAII_TO_CREATURE
 *          (c_1c9a.cpp:5772-5894) over the session CAII array
 *       -> bound scheduling producer DM2_1c9a_0cf7
 *       -> runtime source timer queue
 *       -> dm2_v1_proceed_timers dispatch on the next tick
 *       -> per-cell DM2_THINK_CREATURE binding resolving the same record.
 *
 * Also verifies the fail-closed boundaries: activation without a CAII
 * array, activation of a cell without a creature, and the source early
 * return on re-activation.
 */

#include "dm2_v1_runtime.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "dm2_v1_creature.h"

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
 * Level 0 is 2x3, (0,0) holds creature rec0 (type 0x0C, ungrouped),
 * (0,1) holds an empty chain. */
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
    raw[DB4_BASE + 4] = 0x0C;                  /* creature type byte@4 */
    raw[DB4_BASE + 5] = 0xFF;                  /* byte@5: no CAII slot */
    wr16(raw + DB4_BASE + 8, (int16_t)0xffff); /* no group link -> 0x21 */
    wr16(raw + DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
}

static void test_runtime_caii_activation_end_to_end(void)
{
    static uint8_t raw[RAW_SIZE];
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiAllocReceipt alloc;
    DM2_V1_ThinkCreatureReceipt think;

    build_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0); /* dungeon session: no weather chain */

    /* Without a CAII array the boundary stays unready. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_caii_ready() == 0,
          "CAII array not ready before init");
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 0,
          "activation without a CAII array fails closed");

    CHECK(dm2_v1_runtime_caii_init(4) == 1 &&
              dm2_v1_runtime_caii_ready() == 1,
          "session CAII array initialised with caller-owned capacity");

    /* A cell without a creature never allocates. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 1, &alloc) == 0,
          "cell without a creature fails closed at the boundary");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 0,
          "no allocation receipted for the empty cell");

    /* Activate the creature: slot + first think timer. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 1,
          "activation allocates a CAII slot");
    CHECK(alloc.valid == 1 && alloc.allocated == 1 &&
              alloc.slot_index == 0 &&
              alloc.timer_scheduled == 1 &&
              alloc.timer_type == DM2_V1_TIMER_THINK_CREATURE_A &&
              alloc.due_tick == 1ul,
          "receipt: slot 0 with a 0x21 timer due next tick");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 1,
          "alloc counter advanced");

    /* Re-activation takes the source early return. */
    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 0 &&
              alloc.already_allocated == 1,
          "re-activation takes the source early return");

    /* Next tick: the activation timer dispatches into the think
     * binding and resolves the same record. */
    dm2_v1_runtime_tick();

    CHECK(dm2_v1_runtime_think_creature_receipt(&think) == 1,
          "think binding receipt published");
    CHECK(think.valid == 1 && think.think_timers == 1 &&
              think.resolved == 1 && think.last_creature_type == 0x0C &&
              think.last_type == DM2_V1_TIMER_THINK_CREATURE_A,
          "activation timer resolved the same record end-to-end");
}

static void test_runtime_caii_fail_closed_without_dungeon(void)
{
    DM2_V1_BootProfile boot = {0};
    DM2_V1_CaiiAllocReceipt alloc;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    (void)dm2_v1_runtime_caii_init(4);

    memset(&alloc, 0, sizeof(alloc));
    CHECK(dm2_v1_runtime_alloc_caii_at(0, 0, &alloc) == 0,
          "no dungeon data: activation boundary stays unready");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 0,
          "no dungeon data: no allocation");
}

/* ── floor-mecha dispatch fixture ──────────────────────────────────────
 * Cell (0,0): square class 1 (floor), ground chain DB3 rec0 (type 0x3a)
 * -> DB4 rec0 (creature type 12).  Cell (0,1): square class 1, ground
 * chain starts directly at DB4 rec1 — the walk's DB > 3 source return.
 * G1 layout: text end 256, DB3 span 256..263 (1 x 8 bytes), DB4 span
 * 264..295 (2 x 16 bytes), extension base 296. */
#define FM_DB3_BASE 256
#define FM_DB4_BASE 264
#define FM_G1_EXT_BASE 296

static void build_floor_mecha_dungeon(DM2_V1_DungeonData *d, uint8_t *raw)
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
    d->g1_extension_base = FM_G1_EXT_BASE;
    d->thing_type_counts[3] = 1;
    d->thing_type_counts[4] = 2;
    d->raw_data = raw;
    d->raw_size = RAW_SIZE;

    raw[MAP_BASE + 0] = 0x30; /* (0,0): class 1 floor + object flag */
    raw[MAP_BASE + 1] = 0x30; /* (0,1): class 1 floor + object flag */

    wr16(raw + COLUMN_BASE + 0, 0);
    wr16(raw + COLUMN_BASE + 2, 2);

    /* (0,0): chain root DB3 rec0; (0,1): chain root DB4 rec1 (dir 1). */
    wr16(raw + GROUND_BASE + 0, (int16_t)((3u << 10) | 0u));
    wr16(raw + GROUND_BASE + 2,
         (int16_t)((1u << 14) | (4u << 10) | 1u));

    /* DB3 rec0: next link -> DB4 rec0 (dir 2), type word@2 = 0x3a. */
    wr16(raw + FM_DB3_BASE + 0,
         (int16_t)((2u << 14) | (4u << 10) | 0u));
    wr16(raw + FM_DB3_BASE + 2, 0x3a);

    wr16(raw + FM_DB4_BASE + 0, DM2_V1_RECORD_HANDLE_END);
    raw[FM_DB4_BASE + 4] = 0x0C;                  /* creature type 12 */
    raw[FM_DB4_BASE + 5] = 0xFF;                  /* no CAII slot */
    wr16(raw + FM_DB4_BASE + 8, (int16_t)0xffff);
    wr16(raw + FM_DB4_BASE + 16, DM2_V1_RECORD_HANDLE_END);
    raw[FM_DB4_BASE + 16 + 4] = 0x0C;
    raw[FM_DB4_BASE + 16 + 5] = 0xFF;
    wr16(raw + FM_DB4_BASE + 16 + 8, (int16_t)0xffff);
}

static void fm_set_word_entry(DM2_V1_GdatEntry *e, int category, int index,
                              int field, uint16_t value)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    e->cls4 = (uint8_t)field;
    e->data_index = value;
}

static void test_runtime_floor_mecha_activation(void)
{
    static uint8_t raw[RAW_SIZE];
    static uint8_t gdat_data[1];
    DM2_V1_GdatEntry entries[3];
    uint32_t raw_offsets[1] = { 0 };
    uint32_t raw_sizes[1] = { 0 };
    DM2_V1_AssetLoader loader;
    DM2_V1_DungeonData dungeon;
    DM2_V1_BootProfile boot = {0};
    DM2_V1_SourceTimer timer;
    DM2_V1_RuntimeFloorMechaReceipt fm;

    build_floor_mecha_dungeon(&dungeon, raw);
    boot.dungeon_data = &dungeon;

    dm2_v1_runtime_init(&boot);
    dm2_v1_runtime_set_outdoor(0);
    CHECK(dm2_v1_runtime_caii_init(4) == 1,
          "floor-mecha: session CAII array ready");

    /* Type 12 -> AI row 5 -> flags 0x0001 (bit0 set): the animate gate
     * admits the alloc data-backed. */
    fm_set_word_entry(&entries[0], DM2_GDAT_CATEGORY_CREATURES,
                      12, 0x05, 5);
    fm_set_word_entry(&entries[1], DM2_GDAT_CATEGORY_CREATURE_AI,
                      5, 0, 0x01);
    fm_set_word_entry(&entries[2], DM2_GDAT_CATEGORY_CREATURE_AI,
                      5, 4, 40);
    memset(&loader, 0, sizeof(loader));
    loader.data = gdat_data;
    loader.data_size = 1;
    loader.loaded = 1;
    loader.raw_data_count = 1;
    loader.raw_offsets = raw_offsets;
    loader.raw_sizes = raw_sizes;
    loader.entries = entries;
    loader.entry_count = 3;
    CHECK(dm2_v1_creature_load_ai_table_from_gdat(&loader) == 1,
          "floor-mecha: synthetic GDAT session loads one AI row");

    /* 0x04 timer at (0,0), due immediately: square class 1 dispatches
     * the bounded DM2_ACTUATE_FLOOR_MECHA walk; the DB3 type-0x3a
     * record fires DM2_ANIMATE_CREATURE's CAII gate; the DB4 link then
     * takes the source's DB > 3 whole-function return. */
    memset(&timer, 0, sizeof(timer));
    timer.type = DM2_V1_TIMER_ACTUATE_TILE;
    timer.value_a = (int16_t)(0 | (0 << 8));
    timer.ticks_and_map = 0;
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "floor-mecha: 0x04 timer queued for (0,0)");
    dm2_v1_runtime_tick();

    memset(&fm, 0, sizeof(fm));
    CHECK(dm2_v1_runtime_floor_mecha_receipt(&fm) == 1 &&
              fm.timers == 1 && fm.records_0x3a == 1 &&
              fm.activations == 1 && fm.allocs == 1 &&
              fm.db_break == 1 && fm.walk_failed == 0,
          "floor-mecha: 0x3a record activated the creature, DB4 broke the walk");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 1,
          "floor-mecha: the bound allocator consumed one CAII slot");

    /* 0x04 timer at (0,1): the chain starts at a DB4 record — the
     * source's DB > 3 return fires before any 0x3a record. */
    memset(&timer, 0, sizeof(timer));
    timer.type = DM2_V1_TIMER_ACTUATE_TILE;
    timer.value_a = (int16_t)(0 | (1 << 8));
    timer.ticks_and_map = 0;
    CHECK(dm2_v1_runtime_enqueue_source_timer(&timer, 0) ==
              DM2_V1_SOURCE_TIMER_OK,
          "floor-mecha: 0x04 timer queued for (0,1)");
    dm2_v1_runtime_tick();

    memset(&fm, 0, sizeof(fm));
    CHECK(dm2_v1_runtime_floor_mecha_receipt(&fm) == 1 &&
              fm.timers == 2 && fm.records_0x3a == 1 &&
              fm.activations == 1 && fm.allocs == 1 &&
              fm.db_break == 2 && fm.walk_failed == 0,
          "floor-mecha: DB4-rooted chain takes the DB > 3 source return");
    CHECK(dm2_v1_runtime_caii_alloc_count() == 1,
          "floor-mecha: no second activation on the DB4-rooted cell");

    dm2_v1_creature_reset_ai_table();
}

int main(void)
{
    printf("DM2 V1 CAII lazy creature-activation runtime wiring\n");
    printf("Source: skproject/SKULLWIN/c_1c9a.cpp:5772-5894\n");
    printf("        skproject/SKULLWIN/c_creature.cpp:347-352 (activation)\n");
    printf("        skproject/SKULLWIN/c_1c9a.cpp:5695-5728 (producer)\n\n");

    test_runtime_caii_activation_end_to_end();
    test_runtime_caii_fail_closed_without_dungeon();
    test_runtime_floor_mecha_activation();

    printf("\n%d passed, %d failed\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
