/*
 * test_csb_v22_viewport_swap_pc34.c — CSB V2.2 per-cell modern-art swap
 *
 * Data-free unit tests for the CSB V2.2 viewport swap module.
 * Mirrors tests/test_dm2_v22_viewport_swap_pc34.c but for the CSB
 * 9-square viewport and CSB-only narrative/UI shapes.
 *
 * Coverage:
 *   - Init/shutdown + activation gating (no swap active before update)
 *   - Discriminator covers wall/floor/creature/item/field/UI/narrative shapes
 *   - asset_id and category mapping for representative shapes
 *   - SHAPE_NONE returns NULL asset_id/category
 *   - viewport_swap_update populates + unpopulated before update
 *   - render is no-op when swap not active / not populated / null args
 *   - render is no-op when modern pack missing (csb_v22_get_installed()==0)
 *   - render returns 0 cells painted (synthetic cache, no real assets)
 *   - source evidence citation
 *
 * Skips cache-load-dependent checks when the in-place cache can't
 * be loaded (hosts without user-staged CSB V22 modern assets).
 */

#include "csb_v22_viewport_swap_pc34.h"
#include "csb_v22_inplace_draw_pc34.h"
#include "csb_v22_modern_assets_pc34.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures = 0;
static int checks = 0;

#define CHECK(expr, msg) \
    do { checks++; if (!(expr)) { \
        fprintf(stderr, "FAIL %s:%d: %s — %s\n", __FILE__, __LINE__, #expr, (msg)); \
        failures++; } } while (0)

static void reset_state(void) {
    csb_v22_inplace_draw_shutdown();
    /* We do not touch csb_v22_set_installed() in tests — the test
     * fixtures leave it at the default (0) so the swap renders as
     * a no-op when the modern pack is not loaded. The probe is the
     * place that exercises the active-state path. */
}

static void test_unpopulated_inactive_before_update(void) {
    reset_state();
    CHECK(csb_v22_viewport_swap_populated() == 0,
          "swap populated==0 before first update");
    CHECK(csb_v22_viewport_swap_active() == 0,
          "swap active==0 before first update");
    CHECK(csb_v22_viewport_swap_cells_painted() == 0,
          "swap cells_painted==0 before first update");
}

static void test_discriminator_walls_floors(void) {
    /* Walls (low nibble 0..3) */
    CHECK(csb_v22_swap_shape_for_cell(0x00, 0) == CSB_V22_SWAP_SHAPE_WALL_STRAIGHT,
          "raw 0x00 -> WALL_STRAIGHT");
    CHECK(csb_v22_swap_shape_for_cell(0x01, 0) == CSB_V22_SWAP_SHAPE_WALL_CORNER_INNER,
          "raw 0x01 -> WALL_CORNER_INNER");
    CHECK(csb_v22_swap_shape_for_cell(0x02, 0) == CSB_V22_SWAP_SHAPE_WALL_CORNER_OUTER,
          "raw 0x02 -> WALL_CORNER_OUTER");
    CHECK(csb_v22_swap_shape_for_cell(0x03, 0) == CSB_V22_SWAP_SHAPE_WALL_DOORWAY,
          "raw 0x03 -> WALL_DOORWAY");

    /* Floors (low nibble 4..6) */
    CHECK(csb_v22_swap_shape_for_cell(0x04, 0) == CSB_V22_SWAP_SHAPE_FLOOR_PLAIN,
          "raw 0x04 -> FLOOR_PLAIN");
    CHECK(csb_v22_swap_shape_for_cell(0x05, 0) == CSB_V22_SWAP_SHAPE_FLOOR_CRACKED,
          "raw 0x05 -> FLOOR_CRACKED");
    CHECK(csb_v22_swap_shape_for_cell(0x06, 0) == CSB_V22_SWAP_SHAPE_FLOOR_MOSSY,
          "raw 0x06 -> FLOOR_MOSSY");

    /* Pit (0x10 + low 0x00) and stairs */
    CHECK(csb_v22_swap_shape_for_cell(0x10, 0) == CSB_V22_SWAP_SHAPE_FLOOR_PIT,
          "raw 0x10 -> FLOOR_PIT");
    CHECK(csb_v22_swap_shape_for_cell(0x11, 0) == CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_DOWN,
          "raw 0x11 -> FLOOR_STAIRS_DOWN (0x10 + bit 0 set)");
    CHECK(csb_v22_swap_shape_for_cell(0x10, 0) == CSB_V22_SWAP_SHAPE_FLOOR_PIT,
          "raw 0x10 alone -> FLOOR_PIT (low nibble 0x00 pit)");

    /* Door (0x20) */
    CHECK(csb_v22_swap_shape_for_cell(0x20, 0) == CSB_V22_SWAP_SHAPE_FLOOR_DOOR,
          "raw 0x20 -> FLOOR_DOOR");
}

static void test_discriminator_creatures_items_fields(void) {
    /* Creatures (top bit) */
    CHECK(csb_v22_swap_shape_for_cell(0x80, 0) == CSB_V22_SWAP_SHAPE_CREATURE,
          "raw 0x80 -> CREATURE (top bit set)");
    CHECK(csb_v22_swap_shape_for_cell(0xC0, 0) == CSB_V22_SWAP_SHAPE_CREATURE_PROJECTILE,
          "raw 0xC0 -> CREATURE_PROJECTILE (top+0x40 set)");

    /* Items (0x40 + low nibble) */
    CHECK(csb_v22_swap_shape_for_cell(0x40, 0) == CSB_V22_SWAP_SHAPE_ITEM_FLOOR,
          "raw 0x40 -> ITEM_FLOOR (0x40 + low 0)");
    CHECK(csb_v22_swap_shape_for_cell(0x41, 0) == CSB_V22_SWAP_SHAPE_ITEM,
          "raw 0x41 -> ITEM (0x40 + low 1)");

    /* Fields (low nibble 12..14) */
    CHECK(csb_v22_swap_shape_for_cell(0x0C, 0) == CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER,
          "raw 0x0C -> FIELD_TELEPORTER");
    CHECK(csb_v22_swap_shape_for_cell(0x0D, 0) == CSB_V22_SWAP_SHAPE_FIELD_FLUXCAGE,
          "raw 0x0D -> FIELD_FLUXCAGE");
}

static void test_asset_id_mapping_walls_floors(void) {
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_WALL_STRAIGHT),
                 "wall_dungeon_01") == 0,
          "WALL_STRAIGHT -> wall_dungeon_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_PLAIN),
                 "floor_plain_01") == 0,
          "FLOOR_PLAIN -> floor_plain_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_CRACKED),
                 "floor_cracked_01") == 0,
          "FLOOR_CRACKED -> floor_cracked_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_PIT),
                 "floor_pit_01") == 0,
          "FLOOR_PIT -> floor_pit_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_STAIRS_UP),
                 "floor_stairs_01") == 0,
          "FLOOR_STAIRS_UP -> floor_stairs_01");
}

static void test_asset_id_mapping_creatures_fields(void) {
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_CREATURE),
                 "creature_chaos_fiend_01") == 0,
          "CREATURE -> creature_chaos_fiend_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FIELD_TELEPORTER),
                 "field_teleporter_01") == 0,
          "FIELD_TELEPORTER -> field_teleporter_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_FIELD_CHAOS_RIFT),
                 "field_chaos_rift_01") == 0,
          "FIELD_CHAOS_RIFT -> field_chaos_rift_01");
}

static void test_asset_id_mapping_csb_only_shapes(void) {
    /* CSB-only narrative shapes */
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_PRISON_DOOR),
                 "door_prison_01") == 0,
          "PRISON_DOOR -> door_prison_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_DSA_SCROLL),
                 "dsa_scroll_01") == 0,
          "DSA_SCROLL -> dsa_scroll_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_LORD_ORDER),
                 "statue_lord_order_01") == 0,
          "LORD_ORDER -> statue_lord_order_01");
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_CHAOS_RUNE),
                 "chaos_rune_marker_01") == 0,
          "CHAOS_RUNE -> chaos_rune_marker_01");

    /* CSB-only UI: DSA rune */
    CHECK(strcmp(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_UI_DSA_RUNE),
                 "chaos_rune_01") == 0,
          "UI_DSA_RUNE -> chaos_rune_01");
}

static void test_category_mapping(void) {
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_WALL_STRAIGHT),
                 "wall_shapes") == 0,
          "WALL_STRAIGHT -> wall_shapes");
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_PLAIN),
                 "floor_shapes") == 0,
          "FLOOR_PLAIN -> floor_shapes");
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_CREATURE),
                 "creature_shapes") == 0,
          "CREATURE -> creature_shapes");
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_FLOOR_DOOR),
                 "door_shapes") == 0,
          "FLOOR_DOOR -> door_shapes");
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_UI_CHROME),
                 "ui_chrome") == 0,
          "UI_CHROME -> ui_chrome");
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_DSA_SCROLL),
                 "dsa_scrolls") == 0,
          "DSA_SCROLL -> dsa_scrolls (CSB-only category)");
    CHECK(strcmp(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_UI_DSA_RUNE),
                 "chaos_runes") == 0,
          "UI_DSA_RUNE -> chaos_runes (CSB-only category)");
}

static void test_none_shape_returns_null(void) {
    CHECK(csb_v22_swap_asset_id_for_shape(CSB_V22_SWAP_SHAPE_NONE) == NULL,
          "SHAPE_NONE -> NULL asset_id");
    CHECK(csb_v22_swap_category_for_shape(CSB_V22_SWAP_SHAPE_NONE) == NULL,
          "SHAPE_NONE -> NULL category");
}

static void test_update_populates(void) {
    reset_state();
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x04, 0x80 },
        { 0x05, 0x40, 0x06 },
        { 0x10, 0x01, 0x20 }
    };
    CHECK(csb_v22_viewport_swap_populated() == 0,
          "pre-update populated==0");
    csb_v22_viewport_swap_update(0, (const unsigned char (*)[3])raw_cells);
    CHECK(csb_v22_viewport_swap_populated() == 1,
          "post-update populated==1");
}

static void test_update_with_null_safe(void) {
    reset_state();
    csb_v22_viewport_swap_update(0, NULL);
    CHECK(csb_v22_viewport_swap_populated() == 1,
          "update with NULL cells still marks populated==1");
    CHECK(csb_v22_viewport_swap_active() == 0,
          "active==0 without installed pack");
}

static void test_render_no_op_when_not_active(void) {
    reset_state();
    unsigned char raw_cells[3][3] = {
        { 0x00, 0x04, 0x80 },
        { 0x05, 0x40, 0x06 },
        { 0x10, 0x01, 0x20 }
    };
    csb_v22_viewport_swap_update(0, (const unsigned char (*)[3])raw_cells);
    unsigned char fb[1920 * 1080];
    memset(fb, 0xAA, sizeof(fb));
    int painted = csb_v22_viewport_swap_render(fb, 1920, 1080);
    CHECK(painted == 0,
          "render is no-op when modern pack not installed (active==0)");
    /* All fb bytes should be unchanged (sentinel 0xAA). */
    int all_sentinel = 1;
    for (int i = 0; i < (int)sizeof(fb); ++i) {
        if (fb[i] != 0xAA) { all_sentinel = 0; break; }
    }
    CHECK(all_sentinel,
          "render no-op leaves framebuffer unchanged");
    CHECK(csb_v22_viewport_swap_cells_painted() == 0,
          "cells_painted counter stays 0 when swap not active");
}

static void test_render_null_args(void) {
    reset_state();
    int painted = csb_v22_viewport_swap_render(NULL, 1920, 1080);
    CHECK(painted == 0, "render(NULL fb) -> 0 cells");
    unsigned char fb[10];
    memset(fb, 0, sizeof(fb));
    painted = csb_v22_viewport_swap_render(fb, 0, 1080);
    CHECK(painted == 0, "render(0 width) -> 0 cells");
    painted = csb_v22_viewport_swap_render(fb, 1920, 0);
    CHECK(painted == 0, "render(0 height) -> 0 cells");
}

static void test_source_evidence(void) {
    const char* ev = csb_v22_viewport_swap_source_evidence();
    CHECK(ev != NULL, "evidence non-null");
    CHECK(strlen(ev) > 0, "evidence non-empty");
    CHECK(strstr(ev, "ReDMCSB") != NULL,
          "evidence cites ReDMCSB");
    CHECK(strstr(ev, "CSBWin") != NULL,
          "evidence cites CSBWin");
    CHECK(strstr(ev, "Viewport") != NULL,
          "evidence cites 9-square viewport");
}

int main(void) {
    test_unpopulated_inactive_before_update();
    test_discriminator_walls_floors();
    test_discriminator_creatures_items_fields();
    test_asset_id_mapping_walls_floors();
    test_asset_id_mapping_creatures_fields();
    test_asset_id_mapping_csb_only_shapes();
    test_category_mapping();
    test_none_shape_returns_null();
    test_update_populates();
    test_update_with_null_safe();
    test_render_no_op_when_not_active();
    test_render_null_args();
    test_source_evidence();

    printf("csb_v22_viewport_swap_pc34: checks=%d failures=%d\n", checks, failures);
    if (failures > 0) {
        printf("csb_v22_viewport_swap_pc34: FAIL\n");
        return 1;
    }
    printf("csb_v22_viewport_swap_pc34: PASS\n");
    return 0;
}
