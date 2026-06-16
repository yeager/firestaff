/* test_csb_v22_shapes_pc34.c
 *
 * CSB V2.2 modern shape book unit test. Mirrors the DM1 V2.2
 * shape coverage with CSB-specific extras (prison door, chaos
 * rune, DSA scroll, 9-square wall variants).
 */
#include "csb_v22_shapes.h"
#include "csb_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_init(void) {
    csb_v22_shapes_init();
    check(1, "init");
}

static void t_material_count(void) {
    int n = csb_v22_material_count();
    check(n >= 10, "materials >= 10");
    check(n <= 32, "materials <= 32");
}

static void t_material_oob(void) {
    const CSB_V22_Material* m = csb_v22_material_get(-1);
    check(m != NULL, "OOB material -1 not null");
    m = csb_v22_material_get(9999);
    check(m != NULL, "OOB material 9999 not null");
    /* OOB material is the plain-stone default (index 0). */
    check(m == csb_v22_material_get(0), "OOB material returns default[0]");
}

static void t_material_in_range(void) {
    /* Index 5 = chaos rune, must have high emission_strength. */
    const CSB_V22_Material* chaos = csb_v22_material_get(5);
    check(chaos != NULL, "chaos material");
    check(chaos->emission_strength >= 0.5f, "chaos has high emission");
    /* Index 6 = prison door iron, must be metallic. */
    const CSB_V22_Material* iron = csb_v22_material_get(6);
    check(iron != NULL && iron->metallic >= 0.5f, "prison iron is metallic");
}

static void t_wall_shape_in_range(void) {
    CSB_V22_WallShape w = csb_v22_wall_shape_get(CSB_V22_WALL_VARIANT_D0_CENTER);
    check(w.base_texture_id == 0, "D0C wall base tex");
    /* PRISON variant must be CSB-only (no DM1 equivalent). */
    CSB_V22_WallShape prison = csb_v22_wall_shape_get(CSB_V22_WALL_VARIANT_PRISON);
    check(prison.base_texture_id == 6, "PRISON variant base tex");
}

static void t_wall_shape_oob(void) {
    CSB_V22_WallShape w = csb_v22_wall_shape_get((CSB_V22_WallVariant)9999);
    /* OOB returns D0_CENTER default. */
    check(w.base_texture_id == 0, "OOB wall variant returns D0C default");
}

static void t_floor_shape_basic(void) {
    /* M034 plain (0x04) -> plain floor. */
    CSB_V22_FloorShape f = csb_v22_floor_shape_get(0x04, 0);
    check(f.pit_present == 0, "plain floor no pit");
    check(f.stairs_present == 0, "plain floor no stairs");
    /* M034 pit (0x40 flag) -> pit. */
    CSB_V22_FloorShape pit = csb_v22_floor_shape_get(0x40, 0);
    check(pit.pit_present == 1, "pit flag -> pit_present=1");
    /* M034 stairs up (0x10 flag) -> stairs up. */
    CSB_V22_FloorShape up = csb_v22_floor_shape_get(0x10, 0);
    check(up.stairs_present == 1 && up.stairs_direction == 0, "stairs up");
    /* M034 stairs down (0x11 flag) -> stairs down. */
    CSB_V22_FloorShape down = csb_v22_floor_shape_get(0x11, 0);
    check(down.stairs_present == 1 && down.stairs_direction == 1, "stairs down");
}

static void t_shape_for_cell(void) {
    /* Wall cell (base 0) -> wall shape. */
    CSB_V22_ShapeParams p = csb_v22_shape_for_cell(0x00, 0, 0, 0);
    check(p.type == CSB_V22_SHAPE_WALL_STRAIGHT, "cell 0 -> wall straight");
    /* Floor cell (base 4) -> floor plain. */
    CSB_V22_ShapeParams floor = csb_v22_shape_for_cell(0x04, 0, 1, 0);
    check(floor.type == CSB_V22_SHAPE_FLOOR_PLAIN, "cell 4 -> floor plain");
    /* Pit (flag 0x40) -> floor pit. */
    CSB_V22_ShapeParams pit = csb_v22_shape_for_cell(0x40, 0, 2, -1);
    check(pit.type == CSB_V22_SHAPE_FLOOR_PIT, "cell 0x40 -> floor pit");
    /* Door (flag 0x20) -> wall doorway. */
    CSB_V22_ShapeParams door = csb_v22_shape_for_cell(0x20, 0, 1, 0);
    check(door.type == CSB_V22_SHAPE_WALL_DOORWAY, "cell 0x20 -> wall doorway");
}

static void t_shape_clamp(void) {
    /* Out-of-range depth/lateral should clamp, not crash. */
    CSB_V22_ShapeParams p = csb_v22_shape_for_cell(0x04, 0, 99, 99);
    check(p.type >= 0 && p.type < CSB_V22_SHAPE_COUNT, "clamp depth=99");
    CSB_V22_ShapeParams p2 = csb_v22_shape_for_cell(0x04, 99, 0, 0);
    check(p2.type >= 0 && p2.type < CSB_V22_SHAPE_COUNT, "clamp dir=99");
}

static void t_shape_for_view_square(void) {
    /* view_square 0..8 covers 3x3. element 0=floor, 1=ceiling, 2=wall. */
    CSB_V22_ShapeParams floor = csb_v22_shape_for_view_square(0, 0, 0);
    check(floor.type == CSB_V22_SHAPE_FLOOR_PLAIN, "vq 0,elem 0 = floor");
    CSB_V22_ShapeParams ceil = csb_v22_shape_for_view_square(4, 1, 0);
    check(ceil.type == CSB_V22_SHAPE_CEILING_PLAIN, "vq 4,elem 1 = ceiling");
    CSB_V22_ShapeParams wall = csb_v22_shape_for_view_square(8, 2, 0);
    check(wall.type == CSB_V22_SHAPE_WALL_STRAIGHT, "vq 8,elem 2 = wall");
}

static void t_prison_door_open_progress(void) {
    /* open_progress 0 = dark iron, 100 = bright sky. */
    CSB_V22_ShapeParams closed = csb_v22_shape_for_prison_door(0);
    check(closed.type == CSB_V22_SHAPE_PRISON_DOOR, "prison type");
    check(closed.color_tint[0] <= 80, "closed prison is dim");
    CSB_V22_ShapeParams open = csb_v22_shape_for_prison_door(100);
    check(open.color_tint[0] >= 240, "open prison is bright");
    CSB_V22_ShapeParams mid = csb_v22_shape_for_prison_door(50);
    check(mid.color_tint[0] >= 100 && mid.color_tint[0] <= 200, "mid prison in range");
}

static void t_chaos_rune_colors(void) {
    /* Each rune index has a distinct tint. */
    CSB_V22_ShapeParams r0 = csb_v22_shape_for_chaos_rune(0);
    CSB_V22_ShapeParams r1 = csb_v22_shape_for_chaos_rune(1);
    CSB_V22_ShapeParams r2 = csb_v22_shape_for_chaos_rune(2);
    CSB_V22_ShapeParams r3 = csb_v22_shape_for_chaos_rune(3);
    check(r0.type == CSB_V22_SHAPE_CHAOS_RUNE, "r0 type");
    check(r0.lighting_mode == CSB_V22_LIGHT_CHAOS_GLOW, "r0 chaos glow");
    /* At least one channel must differ between consecutive runes. */
    int diff01 = (r0.color_tint[0] != r1.color_tint[0] ||
                  r0.color_tint[1] != r1.color_tint[1] ||
                  r0.color_tint[2] != r1.color_tint[2]);
    int diff12 = (r1.color_tint[0] != r2.color_tint[0] ||
                  r1.color_tint[1] != r2.color_tint[1] ||
                  r1.color_tint[2] != r2.color_tint[2]);
    check(diff01, "rune 0 vs 1 differ");
    check(diff12, "rune 1 vs 2 differ");
    /* Wraps: r4 should equal r0. */
    CSB_V22_ShapeParams r4 = csb_v22_shape_for_chaos_rune(4);
    check(r4.color_tint[0] == r0.color_tint[0] &&
          r4.color_tint[1] == r0.color_tint[1] &&
          r4.color_tint[2] == r0.color_tint[2], "rune 4 wraps to rune 0");
}

static void t_dsa_scroll(void) {
    CSB_V22_ShapeParams s = csb_v22_shape_for_dsa_scroll(0);
    check(s.type == CSB_V22_SHAPE_DSA_SCROLL, "scroll type");
    check(s.lighting_mode == CSB_V22_LIGHT_MAGICAL_GLOW, "scroll magical glow");
    check(s.color_tint[0] >= 200, "scroll warm tint");
}

static void t_evidence(void) {
    const char* ev = csb_v22_shapes_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "9-square") != NULL, "ev 9-square");
    check(strstr(ev, "PRISON_DOOR") != NULL, "ev PRISON_DOOR");
    check(strstr(ev, "CHAOS_RUNE") != NULL, "ev CHAOS_RUNE");
    check(strstr(ev, "DSA") != NULL, "ev DSA");
    check(strstr(ev, "Lord Order") != NULL || strstr(ev, "LORD_ORDER") != NULL, "ev Lord Order");
}

static void t_9square_3x3_coverage(void) {
    /* 9-square layout: 3 depth x 3 lateral = 9 cells, 9 wall
     * variants (D2L/D2R/D2C, D1L/D1R/D1C, D0L/D0R/D0C). */
    for (int d = 0; d < 3; d++) {
        for (int lat = -1; lat <= 1; lat++) {
            CSB_V22_ShapeParams p = csb_v22_shape_for_cell(0x00, 0, d, lat);
            check(p.type >= 0 && p.type < CSB_V22_SHAPE_COUNT,
                  "9sq cell valid");
        }
    }
    /* Plus DOOR + PRISON = 11 variants. */
    check(CSB_V22_WALL_VARIANT_COUNT == 11, "9 wall + DOOR + PRISON = 11");
}

int main(void) {
    printf("=== CSB V2.2 shape book test ===\n");
    t_init();
    t_material_count();
    t_material_oob();
    t_material_in_range();
    t_wall_shape_in_range();
    t_wall_shape_oob();
    t_floor_shape_basic();
    t_shape_for_cell();
    t_shape_clamp();
    t_shape_for_view_square();
    t_prison_door_open_progress();
    t_chaos_rune_colors();
    t_dsa_scroll();
    t_evidence();
    t_9square_3x3_coverage();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
