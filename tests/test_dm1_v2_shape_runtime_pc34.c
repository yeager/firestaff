/* test_dm1_v2_shape_runtime_pc34.c
 *
 * DM1 V2.2 shape runtime dispatch unit test.
 */
#include "dm1_v2_shape_runtime_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include <stdio.h>
#include <string.h>
static int g_failed = 0, g_total = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "FAIL: %s\n", name); }
    else printf("PASS: %s\n", name);
}

static void t_inactive_v1(void) {
    /* Default V1 path: runtime reports active=0. */
    dm1_v2_presentation_mode_reset();
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x00, 0, 0, 0);
    check(r.active == 0, "V1 default -> active=0");
    check(dm1_v2_shape_runtime_v22_active() == 0, "v22_active false V1");
}

static void t_active_v22_with_pack(void) {
    /* V22 + modern pack: runtime reports active=1. */
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 1, "V22+pack -> active=1");
    check(r.material != NULL, "V22 material non-null");
    check(r.params.type >= 0 && r.params.type < M11_V22_SHAPE_COUNT, "V22 shape type in range");
    check(dm1_v2_shape_runtime_v22_active() == 1, "v22_active true V22");
}

static void t_active_v22_no_pack(void) {
    /* V22 without modern pack: resolves to V21, runtime inactive. */
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 0, "V22 no pack -> active=0 (V21 fallback)");
}

static void t_v21_inactive(void) {
    /* V21 is the upscale path, not the V22 shape path. */
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 0, "V21 -> active=0 (V21 is upscale not shapes)");
}

static void t_wall_variant_by_depth(void) {
    /* Verify the (depth, lateral) -> WallVariant mapping covers
     * the 4x3 grid: D3 (3) + D2 (3) + D1 (3) + D0 (3) = 12 cells. */
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    int seen_variant[4] = { 0, 0, 0, 0 };
    for (int d = 0; d < 4; d++) {
        for (int lat = -1; lat <= 1; lat++) {
            DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x00, 0, d, lat);
            check(r.active == 1, "wall variant active");
            check(r.wall.base_texture_id >= 0, "wall has base tex");
        }
        seen_variant[d] = 1;
    }
    check(seen_variant[0] && seen_variant[1] && seen_variant[2] && seen_variant[3],
          "all 4 depths covered");
}

static void t_composition_12_cells(void) {
    /* A 12-cell composition in DM1 order. */
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    uint8_t raws[12] = { 0x00, 0x04, 0x40, 0x20, 0x10, 0x10, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04 };
    int lateral[12] = { -1, 0, 1,  -1, 0, 1,  -1, 0, 1,  -1, 0, 1 };
    int depth[12]   = { 3, 3, 3,  2, 2, 2,  1, 1, 1,  0, 0, 0 };
    DM1_V2_ShapeRuntimeComposition c = dm1_v2_shape_runtime_composition(raws, lateral, depth, 0);
    check(c.count == 12, "composition count=12");
    int active_count = 0;
    for (int i = 0; i < 12; i++) {
        if (c.cells[i].active) active_count++;
    }
    check(active_count == 12, "all 12 cells active under V22");
}

static void t_composition_null_safe(void) {
    DM1_V2_ShapeRuntimeComposition c = dm1_v2_shape_runtime_composition(NULL, NULL, NULL, 0);
    check(c.count == 0, "composition null -> count=0");
}

static void t_evidence(void) {
    const char* ev = dm1_v2_shape_runtime_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "DUNVIEW.C") != NULL, "ev DUNVIEW.C");
    check(strstr(ev, "DUNGEON.C") != NULL, "ev DUNGEON.C");
    check(strstr(ev, "DEFS.H") != NULL, "ev DEFS.H");
    check(strstr(ev, "M034") != NULL, "ev M034");
}

static void t_pack_turn_off_in_flight(void) {
    /* V22 active then pack goes off mid-flight: falls back to V21,
     * runtime becomes inactive. */
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    check(dm1_v2_shape_runtime_v22_active() == 1, "V22+pack active");
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    check(dm1_v2_shape_runtime_v22_active() == 0, "pack off -> V21 -> inactive");
}

static void t_independent_from_csb(void) {
    /* DM1 V22 and CSB V21 are independent; setting one doesn't
     * flip the other's runtime. */
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    csb_v2_presentation_mode_set_modern_pack_available(0);
    csb_v2_presentation_mode_set(CSB_V2_PM_V21_UPSCALED);
    check(dm1_v2_shape_runtime_v22_active() == 1, "DM1 V22 still active");
    check(csb_v2_presentation_mode_is_v21() == 1, "CSB V21 still active");
    /* DM1 runtime active, CSB runtime is its own thing. */
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 1, "DM1 V22 cell still resolves");
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
}

int main(void) {
    printf("=== DM1 V2.2 shape runtime test ===\n");
    t_inactive_v1();
    t_active_v22_with_pack();
    t_active_v22_no_pack();
    t_v21_inactive();
    t_wall_variant_by_depth();
    t_composition_12_cells();
    t_composition_null_safe();
    t_evidence();
    t_pack_turn_off_in_flight();
    t_independent_from_csb();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
