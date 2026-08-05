/*
 * test_dm2_v1_creature_ai_loop_pc34_compat.c — DM2 creature AI loop
 * orchestrator tests.
 *
 * Source-lock anchors:
 *   skproject/SKULLWIN/c_ai.cpp:4743-5272   DM2_14cd_09e2
 *   skproject/SKULLWIN/c_ai.cpp:5275-5338   DM2_50CB
 *   skproject/SKULLWIN/c_ai.cpp:5341-5647   DM2_13e4_0982
 *   skproject/SKULLWIN/c_ai.cpp:5649-5784   DM2_THINK_CREATURE
 *
 * Synthetic GDAT fixture (no game data required).
 */

#include "dm2_v1_creature_ai_loop_pc34_compat.h"
#include "dm2_v1_creature.h"
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

#define TYPE_DYN 12
#define TYPE_STA 7

/* ── GDAT fixture (copied from test_dm2_v1_ccm_loop) ──────────────── */

#define RAW_COUNT 4
#define DATA_SIZE 64
#define ENTRY_COUNT 13

static void wr16(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)((v >> 8) & 0xffu);
}

static int16_t rec_handle(int index)
{
    return (int16_t)((4 << 10) | (index & 0x3ff));
}

static void set_word_entry(DM2_V1_GdatEntry *e, int category, int index,
                           int field, uint16_t value)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)DM2_GDAT_ENTRY_TYPE_WORD_VALUE;
    e->cls4 = (uint8_t)field;
    e->data_index = value;
}

static void set_raw_entry(DM2_V1_GdatEntry *e, int category, int index,
                          int type, int field, uint16_t raw_index)
{
    memset(e, 0, sizeof(*e));
    e->cls1 = (uint8_t)category;
    e->cls2 = (uint8_t)index;
    e->cls3 = (uint8_t)type;
    e->cls4 = (uint8_t)field;
    e->data_index = raw_index;
}

static DM2_V1_GdatEntry g_entries[ENTRY_COUNT];
static uint32_t g_raw_offsets[RAW_COUNT];
static uint32_t g_raw_sizes[RAW_COUNT];
static uint8_t g_data[DATA_SIZE];
static DM2_V1_AssetLoader g_loader;

static void build_loader(void)
{
    int n = 0;

    memset(g_data, 0, DATA_SIZE);

    /* type 12 attribution @0 */
    wr16(g_data + 0, 0x05); wr16(g_data + 2, 2);
    wr16(g_data + 4, 0x13); wr16(g_data + 6, 2);
    wr16(g_data + 8, 0x07); wr16(g_data + 10, 6);
    wr16(g_data + 12, 0xffff); wr16(g_data + 14, 0);
    /* type 12 info @16: row 2 has adj=1, row 3 has adj=2, row 5 has adj=0 */
    g_data[16 + 8] = 0x05; g_data[16 + 9] = 0x1f;
    g_data[16 + 10] = 0x41; g_data[16 + 11] = 0x33;
    g_data[16 + 13] = 0x1f; g_data[16 + 14] = 0x42;
    g_data[16 + 15] = 0x10;
    g_data[16 + 21] = 0x01; g_data[16 + 22] = 0x40;
    g_data[16 + 25] = 0x1f; g_data[16 + 26] = 0x80;
    g_data[16 + 27] = 0x10;
    /* type 7 attribution @48 / info @56 (zeros) */
    wr16(g_data + 48, 0x05); wr16(g_data + 50, 1);
    wr16(g_data + 52, 0xffff); wr16(g_data + 54, 0);

    g_raw_offsets[0] = 0;  g_raw_sizes[0] = 16;
    g_raw_offsets[1] = 16; g_raw_sizes[1] = 32;
    g_raw_offsets[2] = 48; g_raw_sizes[2] = 8;
    g_raw_offsets[3] = 56; g_raw_sizes[3] = 8;

    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                   0x05, 5);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                   0x05, 9);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                   0x01, 0x0d);
    /* table1d607e[4] = { 0xe8, 0x00, 0x00, 0x00 }: byte0 & 0x40 != 0, so
     * this row selects the static-creature branch of
     * dm2_v1_creature_strategy_select (c_ai.cpp:4772). */
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                   0x01, 0x04);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 0, 0x00);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 4, 40);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 5, 9, 0x05);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 0, 0x01);
    set_word_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURE_AI, 9, 4, 24);
    set_raw_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 0);
    set_raw_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_DYN,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 1);
    set_raw_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                  DM2_GDAT_ENTRY_TYPE_RAW8,
                  DM2_GDAT_CREATURE_ANIM_ATTRIBUTION, 2);
    set_raw_entry(&g_entries[n++], DM2_GDAT_CATEGORY_CREATURES, TYPE_STA,
                  DM2_GDAT_ENTRY_TYPE_RAW7,
                  DM2_GDAT_CREATURE_ANIM_INFO_SEQUENCE, 3);

    memset(&g_loader, 0, sizeof(g_loader));
    g_loader.data = g_data;
    g_loader.data_size = DATA_SIZE;
    g_loader.loaded = 1;
    g_loader.raw_data_count = RAW_COUNT;
    g_loader.raw_offsets = g_raw_offsets;
    g_loader.raw_sizes = g_raw_sizes;
    g_loader.entries = g_entries;
    g_loader.entry_count = (uint16_t)n;

    dm2_v1_creature_load_ai_table_from_gdat(&g_loader);
}

/* ── pool/CAII ─────────────────────────────────────────────────────── */

static void build_pools(DM2_V1_RecordPoolSet *set, DM2_V1_CaiiArray *caii,
                        int creature_type, int slot_idx)
{
    uint8_t *rec;

    memset(set, 0, sizeof(*set));
    memset(caii, 0, sizeof(*caii));

    set->pools[4].record_size = 16;
    set->pools[4].record_count = 2;
    set->pools[4].source_base = 0;
    set->pools[4].bytes = calloc(2, 16);
    set->valid = 1;

    caii->valid = 1;
    caii->slots = calloc(4, DM2_V1_CAII_SLOT_SIZE);
    caii->capacity = 4;

    rec = set->pools[4].bytes; /* record index 0 */
    rec[4] = (uint8_t)creature_type;
    rec[5] = (uint8_t)slot_idx;

    /* Initialize slot */
    memset(caii->slots + (size_t)slot_idx * DM2_V1_CAII_SLOT_SIZE, 0,
           DM2_V1_CAII_SLOT_SIZE);
    caii->slots[(size_t)slot_idx * DM2_V1_CAII_SLOT_SIZE + 0x1a] = 0xff;
    caii->slots[(size_t)slot_idx * DM2_V1_CAII_SLOT_SIZE + 0x17] = 0xff;
    caii->slots[(size_t)slot_idx * DM2_V1_CAII_SLOT_SIZE + 0x12] = 0xff;
}

static void free_pools(DM2_V1_RecordPoolSet *set, DM2_V1_CaiiArray *caii)
{
    free(set->pools[4].bytes);
    free(caii->slots);
}

/* ── dungeon for think_creature_main ───────────────────────────────── */

#define MAP_BASE 0
#define COLUMN_BASE 64
#define GROUND_BASE 128
#define RAW_SIZE 256

static uint8_t g_dungeon_raw[RAW_SIZE];

static void build_dungeon(DM2_V1_DungeonData *d,
                          DM2_V1_RecordPoolSet *set)
{
    memset(d, 0, sizeof(*d));
    memset(g_dungeon_raw, 0, RAW_SIZE);
    d->level_count = 1;
    d->level_widths[0] = 4;
    d->level_heights[0] = 4;
    d->level_offsets[0] = 0;
    d->square_bytes = 1;
    d->raw_map_data_base = MAP_BASE;
    d->column_index_base = COLUMN_BASE;
    d->square_first_thing_base = GROUND_BASE;
    d->square_first_thing_count = 2;
    d->raw_data = g_dungeon_raw;
    d->raw_size = RAW_SIZE;

    /* Cell (1,0) has objects — column-major: x*h+y = 1*4+0 = 4 */
    g_dungeon_raw[MAP_BASE + 4] = 0x10;
    wr16(g_dungeon_raw + COLUMN_BASE + 0, 0);
    wr16(g_dungeon_raw + COLUMN_BASE + 2, 1);
    /* Ground stack: thing at idx 1 -> creature record 0 (column for x=1
     * starts at index 1). */
    wr16(g_dungeon_raw + GROUND_BASE + 2, (uint16_t)rec_handle(0));
    /* Next link in record: end */
    wr16(set->pools[4].bytes + 0, (uint16_t)DM2_V1_RECORD_HANDLE_END);
}

/* ── test callbacks ────────────────────────────────────────────────── */

static int g_go_there_called = 0;
static int test_go_there_cb(void *ctx, uint8_t *slot,
                            uint16_t creature_type,
                            int mode, int x, int y, int dir,
                            int16_t *parw00, int16_t *parw01)
{
    (void)ctx; (void)slot; (void)creature_type;
    (void)mode; (void)x; (void)y; (void)dir;
    (void)parw00; (void)parw01;
    g_go_there_called++;
    return 0;
}

static int g_xact_called = 0;
static int test_xact_cb(void *ctx, uint8_t *slot,
                        uint16_t creature_type,
                        unsigned long game_tick, int iteration)
{
    (void)ctx; (void)creature_type; (void)game_tick;
    g_xact_called++;
    if (iteration >= 2) {
        slot[0x1a] = 0x02;
    }
    return 0;
}

static int g_prepare_called = 0;
static void *test_prepare_cb(void *ctx, uint16_t creature_handle,
                              int x, int y, int think_type)
{
    (void)ctx; (void)creature_handle; (void)x; (void)y; (void)think_type;
    g_prepare_called++;
    return (void *)(uintptr_t)1;
}

static int g_unprepare_called = 0;
static void test_unprepare_cb(void *ctx, void *prepare_token)
{
    (void)ctx; (void)prepare_token;
    g_unprepare_called++;
}

static int g_dispatch_called = 0;
static int test_dispatch_cb(void *ctx, uint16_t creature_handle,
                            const DM2_V1_SourceTimer *timer,
                            int think_type)
{
    (void)ctx; (void)creature_handle; (void)timer; (void)think_type;
    g_dispatch_called++;
    return 1;
}

/* ================================================================== */
/* Tests                                                               */
/* ================================================================== */

static void test_anim_frame_resolve_basic(void)
{
    DM2_V1_AnimFrameResolveReceipt rc;
    int16_t frame = 0;
    const uint8_t *row = NULL;
    int ret;

    build_loader();
    ret = dm2_v1_creature_animation_frame_resolve(
        &g_loader, TYPE_DYN, 0, &frame, &row, &rc);

    CHECK(rc.valid == 1, "50CB: receipt should be valid");
    CHECK(ret >= 0, "50CB: should succeed");
    CHECK(row != NULL, "50CB: row should be set");
    CHECK(!rc.gdat_missing, "50CB: gdat should be found");
    CHECK(!rc.table_oob, "50CB: should not be out of bounds");
}

static void test_anim_frame_resolve_null_loader(void)
{
    DM2_V1_AnimFrameResolveReceipt rc;
    int16_t frame = 0;
    const uint8_t *row = NULL;
    int ret;

    ret = dm2_v1_creature_animation_frame_resolve(
        NULL, TYPE_DYN, 0, &frame, &row, &rc);
    CHECK(ret == -1, "50CB null: should fail");
}

static void test_strategy_select_null_inputs(void)
{
    DM2_V1_CreatureStrategyReceipt rc;
    int ret;

    dm2_v1_creature_strategy_receipt_init(&rc);
    ret = dm2_v1_creature_strategy_select(
        NULL, NULL, 0, 0, 0, 0, 0, NULL, NULL, NULL, NULL, &rc);
    CHECK(ret == 0, "strategy null: should fail");
    CHECK(rc.fail_closed_reason != 0, "strategy null: reason set");
}

static void test_strategy_select_static_creature(void)
{
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_CreatureStrategyReceipt rc;
    int ret;

    build_loader();
    build_pools(&set, &caii, TYPE_STA, 0);
    dm2_v1_creature_strategy_receipt_init(&rc);

    ret = dm2_v1_creature_strategy_select(
        &set, &caii, rec_handle(0), TYPE_STA,
        5, 5, 100, NULL, NULL, NULL, NULL, &rc);

    CHECK(ret == 1, "strategy static: should succeed");
    CHECK(rc.valid == 1, "strategy static: receipt valid");
    CHECK(rc.is_static == 1, "strategy static: should be static");
    CHECK(rc.final_command == 0, "strategy static: command=0");

    free_pools(&set, &caii);
}

static void test_strategy_select_dynamic_xact_loop(void)
{
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_CreatureStrategyReceipt rc;
    int ret;

    build_loader();
    build_pools(&set, &caii, TYPE_DYN, 0);
    g_go_there_called = 0;
    g_xact_called = 0;
    dm2_v1_creature_strategy_receipt_init(&rc);

    ret = dm2_v1_creature_strategy_select(
        &set, &caii, rec_handle(0), TYPE_DYN,
        5, 5, 100,
        test_go_there_cb, NULL,
        test_xact_cb, NULL, &rc);

    CHECK(ret == 1, "strategy dynamic: should succeed");
    CHECK(rc.valid == 1, "strategy dynamic: receipt valid");
    CHECK(rc.is_static == 0, "strategy dynamic: not static");
    CHECK(rc.go_there_tried == 1, "strategy dynamic: go_there tried");
    CHECK(g_go_there_called == 1, "strategy dynamic: go_there called");
    CHECK(rc.xact_loop_entered == 1, "strategy dynamic: xact entered");
    CHECK(g_xact_called == 3, "strategy dynamic: xact 3 calls");
    CHECK(rc.final_command == 0x02, "strategy dynamic: command=2");

    free_pools(&set, &caii);
}

static void test_strategy_select_no_go_there(void)
{
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_CreatureStrategyReceipt rc;
    int ret;

    build_loader();
    build_pools(&set, &caii, TYPE_DYN, 0);
    dm2_v1_creature_strategy_receipt_init(&rc);

    ret = dm2_v1_creature_strategy_select(
        &set, &caii, rec_handle(0), TYPE_DYN,
        5, 5, 100, NULL, NULL, test_xact_cb, NULL, &rc);

    CHECK(ret == 0, "strategy no_go_there: fail closed");
    CHECK(rc.fail_closed_reason == 3, "strategy no_go_there: reason=3");

    free_pools(&set, &caii);
}

static void test_ccm_dispatch_null_inputs(void)
{
    DM2_V1_CcmDispatchReceipt rc;
    int ret;

    dm2_v1_ccm_dispatch_receipt_init(&rc);
    ret = dm2_v1_creature_ccm_dispatch(
        NULL, NULL, NULL, NULL, NULL, 0, NULL, 0,
        NULL, NULL, NULL, 0, 0, 0, 0, 0, 0,
        NULL, NULL, NULL, NULL, NULL, NULL, NULL, NULL, &rc);

    CHECK(ret == 0, "ccm_dispatch null: should fail");
    CHECK(rc.fail_closed_reason != 0, "ccm_dispatch null: reason set");
}

static void test_think_creature_main_no_creature(void)
{
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_DungeonData dungeon;
    DM2_V1_ThinkCreatureMainReceipt rc;
    int ret;

    build_loader();
    build_pools(&set, &caii, TYPE_DYN, 0);
    build_dungeon(&dungeon, &set);
    dm2_v1_think_creature_main_receipt_init(&rc);
    g_prepare_called = 0;

    /* Cell (2,0) has no creature */
    ret = dm2_v1_think_creature_main(
        &set, &dungeon, 0, 2, 0, 0x21, 100,
        test_prepare_cb, NULL,
        test_unprepare_cb, NULL,
        NULL, NULL,
        test_dispatch_cb, NULL,
        NULL, NULL, &rc);

    CHECK(ret == 1, "think no_creature: succeeds (timer consumed)");
    CHECK(rc.valid == 1, "think no_creature: receipt valid");
    CHECK(g_prepare_called == 0, "think no_creature: prepare not called");

    free_pools(&set, &caii);
}

static void test_think_creature_main_with_creature(void)
{
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_DungeonData dungeon;
    DM2_V1_ThinkCreatureMainReceipt rc;
    int ret;

    build_loader();
    build_pools(&set, &caii, TYPE_DYN, 0);
    build_dungeon(&dungeon, &set);
    dm2_v1_think_creature_main_receipt_init(&rc);
    g_prepare_called = 0;
    g_unprepare_called = 0;
    g_dispatch_called = 0;

    /* Cell (1,0) has a creature */
    ret = dm2_v1_think_creature_main(
        &set, &dungeon, 0, 1, 0, 0x21, 100,
        test_prepare_cb, NULL,
        test_unprepare_cb, NULL,
        NULL, NULL,
        test_dispatch_cb, NULL,
        NULL, NULL, &rc);

    CHECK(ret == 1, "think with_creature: should succeed");
    CHECK(rc.valid == 1, "think with_creature: receipt valid");
    CHECK(rc.creature_resolved == 1, "think with_creature: resolved");
    CHECK(rc.creature_type == TYPE_DYN, "think with_creature: type");
    CHECK(rc.prepare_ran == 1, "think with_creature: prepare ran");
    CHECK(rc.unprepare_ran == 1, "think with_creature: unprepare ran");
    CHECK(g_dispatch_called == 1, "think with_creature: dispatched");
    CHECK(rc.dispatched_0982 == 1, "think with_creature: 0982");

    free_pools(&set, &caii);
}

static void test_think_creature_main_no_prepare(void)
{
    DM2_V1_RecordPoolSet set;
    DM2_V1_CaiiArray caii;
    DM2_V1_DungeonData dungeon;
    DM2_V1_ThinkCreatureMainReceipt rc;
    int ret;

    build_loader();
    build_pools(&set, &caii, TYPE_DYN, 0);
    build_dungeon(&dungeon, &set);
    dm2_v1_think_creature_main_receipt_init(&rc);

    ret = dm2_v1_think_creature_main(
        &set, &dungeon, 0, 1, 0, 0x21, 100,
        NULL, NULL, NULL, NULL,
        NULL, NULL, NULL, NULL,
        NULL, NULL, &rc);

    CHECK(ret == 0, "think no_prepare: fail closed");
    CHECK(rc.fail_closed_reason == 2, "think no_prepare: reason=2");

    free_pools(&set, &caii);
}

static void test_source_evidence(void)
{
    const char *evidence = dm2_v1_creature_ai_loop_source_evidence();
    CHECK(evidence != NULL, "evidence: not null");
    CHECK(strlen(evidence) > 100, "evidence: has content");
}

int main(void)
{
    test_anim_frame_resolve_basic();
    test_anim_frame_resolve_null_loader();
    test_strategy_select_null_inputs();
    test_strategy_select_static_creature();
    test_strategy_select_dynamic_xact_loop();
    test_strategy_select_no_go_there();
    test_ccm_dispatch_null_inputs();
    test_think_creature_main_no_creature();
    test_think_creature_main_with_creature();
    test_think_creature_main_no_prepare();
    test_source_evidence();

    if (g_failures > 0) {
        fprintf(stderr, "\n%d test(s) FAILED\n", g_failures);
        return EXIT_FAILURE;
    }
    printf("All creature_ai_loop tests passed.\n");
    return EXIT_SUCCESS;
}
