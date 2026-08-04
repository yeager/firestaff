/*
 * test_dm2_v1_loadlevel_pc34_compat.c -- unit tests for DM2 level loading.
 */

#include <assert.h>
#include <stdio.h>
#include <string.h>

#include "dm2_v1_loadlevel_pc34_compat.h"

/* ── Test mark_dyn_load ─────────────────────────────────────────────── */

static void test_mark_dyn_load(void)
{
    DM2_V1_DynLoadState state;
    memset(&state, 0, sizeof(state));

    DM2_V1_MarkDynLoadReceipt r = dm2_v1_mark_dyn_load(&state, 0x01FF02FF);
    assert(r.entry_index == 0);
    assert(state.count == 1);
    assert(state.entries[0].cat  == 0x01);
    assert(state.entries[0].type == 0xFF);
    assert(state.entries[0].sub1 == 0x02);
    assert(state.entries[0].sub2 == 0xFF);
    assert(state.entries[0].flags == 0);

    /* Second entry */
    r = dm2_v1_mark_dyn_load(&state, 0x0D7E02FF);
    assert(r.entry_index == 1);
    assert(state.count == 2);
    assert(state.entries[1].cat == 0x0D);
    assert(state.entries[1].type == 0x7E);

    printf("  PASS: mark_dyn_load\n");
}

/* ── Test mark_dyn_load_hires ───────────────────────────────────────── */

static void test_mark_dyn_load_hires(void)
{
    DM2_V1_DynLoadState state;
    memset(&state, 0, sizeof(state));

    dm2_v1_mark_dyn_load_hires(&state, 0x01000400);
    assert(state.count == 1);
    assert((state.entries[0].flags & DM2_V1_LOADLEVEL_DYN_FLAG_HIRES) != 0);

    printf("  PASS: mark_dyn_load_hires\n");
}

/* ── Test mark_dyn_load_with_flag ───────────────────────────────────── */

static void test_mark_dyn_load_with_flag(void)
{
    DM2_V1_DynLoadState state;
    memset(&state, 0, sizeof(state));

    dm2_v1_mark_dyn_load_with_flag(&state, 0x16000508, 0x0B);
    assert(state.count == 2);
    assert(state.entries[0].flags == (int16_t)0x8001);

    printf("  PASS: mark_dyn_load_with_flag\n");
}

/* ── Test misc item sorted insertion ────────────────────────────────── */

static int16_t mock_gdat_idx(void *ctx, uint8_t cat, uint8_t type,
    uint8_t sub1, uint8_t sub2)
{
    (void)ctx; (void)cat; (void)sub1;
    if (sub2 == 0) {
        /* Return 0x4000 flag for types 1, 2, 3 */
        if (type >= 1 && type <= 3) return 0x4000;
        return 0;
    }
    if (sub2 == 2) {
        /* Sort keys: type 1 -> 30, type 2 -> 10, type 3 -> 20 */
        if (type == 1) return 30;
        if (type == 2) return 10;
        if (type == 3) return 20;
    }
    return 0;
}

static void test_load_miscitem(void)
{
    DM2_V1_LoadLevelCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.query_gdat_entry_data_index = mock_gdat_idx;
    cb.ctx = NULL;

    DM2_V1_MiscItemState misc;
    memset(&misc, 0, sizeof(misc));

    dm2_v1_load_miscitem(&cb, &misc);
    assert(misc.loaded);
    assert(misc.count == 3);

    /* Should be sorted by key: 10, 20, 30 */
    assert(misc.sort_keys[0] == 10);
    assert(misc.sort_keys[1] == 20);
    assert(misc.sort_keys[2] == 30);

    /* Loading again should be a no-op */
    dm2_v1_load_miscitem(&cb, &misc);
    assert(misc.count == 3);

    printf("  PASS: load_miscitem\n");
}

/* ── Test graphics table loading ────────────────────────────────────── */

static void mock_change_map(void *ctx, int32_t map)
{
    (void)ctx; (void)map;
}

static void test_load_graphics_table(void)
{
    DM2_V1_LoadLevelCallbacks cb;
    memset(&cb, 0, sizeof(cb));
    cb.change_current_map_to = mock_change_map;
    cb.ctx = NULL;

    DM2_V1_LevelGraphicsState gfx;
    memset(&gfx, 0, sizeof(gfx));

    dm2_v1_load_locallevel_graphics_table(&cb, &gfx, 15, 20, 3);
    assert(gfx.party_x == 15);
    assert(gfx.party_y == 20);
    assert(gfx.view_map == 3);

    printf("  PASS: load_graphics_table\n");
}

/* ── Test null safety ───────────────────────────────────────────────── */

static void test_null_safety(void)
{
    DM2_V1_MarkDynLoadReceipt r = dm2_v1_mark_dyn_load(NULL, 0);
    assert(r.entry_index == -1);

    dm2_v1_mark_dyn_load_hires(NULL, 0); /* should not crash */
    dm2_v1_mark_dyn_load_with_flag(NULL, 0, 0);
    dm2_v1_load_miscitem(NULL, NULL);
    dm2_v1_load_locallevel_graphics_table(NULL, NULL, 0, 0, 0);
    dm2_v1_process_level_actuators(NULL, 0);
    dm2_v1_load_newmap(NULL, NULL, NULL, NULL, 0, 0, 0, 0);

    printf("  PASS: null_safety\n");
}

/* ── Main ───────────────────────────────────────────────────────────── */

int main(void)
{
    printf("test_dm2_v1_loadlevel_pc34_compat:\n");
    test_null_safety();
    test_mark_dyn_load();
    test_mark_dyn_load_hires();
    test_mark_dyn_load_with_flag();
    test_load_miscitem();
    test_load_graphics_table();
    printf("All loadlevel tests passed.\n");
    return 0;
}
