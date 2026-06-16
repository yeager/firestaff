/**
 * firestaff_dm1_v2_shape_runtime_probe.c
 *
 * DM1 V2.2 shape runtime dispatch headless probe.
 */
#include "dm1_v2_shape_runtime_pc34.h"
#include "dm1_v2_presentation_mode_pc34.h"
#include "csb_v2_presentation_mode_pc34.h"
#include "csb_v22_shapes.h"
#include <stdio.h>
#include <string.h>
static int g_total = 0, g_failed = 0;
static void check(int cond, const char* name) {
    g_total++;
    if (!cond) { g_failed++; fprintf(stderr, "[FAIL] %s\n", name); }
    else printf("[PASS] %s\n", name);
}

static void p_inactive_v1(void) {
    dm1_v2_presentation_mode_reset();
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 0, "V1 default -> inactive");
}

static void p_active_v22(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 1, "V22+pack -> active");
    check(r.material != NULL, "V22 material present");
}

static void p_fallback_when_no_pack(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(0);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 0, "V22 no pack -> V21 -> inactive");
}

static void p_v21_inactive(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set(DM1_V2_PM_V21_UPSCALED);
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 0, "V21 is upscale, not shapes");
}

static void p_composition(void) {
    dm1_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    uint8_t raws[12] = { 0,0,0, 0,4,0, 0,4,0, 0,4,0 };
    int lat[12] = { -1,0,1,  -1,0,1,  -1,0,1,  -1,0,1 };
    int dep[12] = { 3,3,3,  2,2,2,  1,1,1,  0,0,0 };
    DM1_V2_ShapeRuntimeComposition c = dm1_v2_shape_runtime_composition(raws, lat, dep, 0);
    check(c.count == 12, "composition count=12");
    int n_active = 0;
    for (int i = 0; i < 12; i++) if (c.cells[i].active) n_active++;
    check(n_active == 12, "all 12 active under V22");
}

static void p_evidence(void) {
    const char* ev = dm1_v2_shape_runtime_source_evidence();
    check(ev != NULL && strlen(ev) > 50, "ev non-trivial");
    check(strstr(ev, "DUNVIEW.C") != NULL, "ev DUNVIEW.C");
    check(strstr(ev, "M034") != NULL, "ev M034");
}

static void p_independent_from_csb(void) {
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
    dm1_v2_presentation_mode_set_modern_pack_available(1);
    dm1_v2_presentation_mode_set(DM1_V2_PM_V22_MODERN);
    csb_v2_presentation_mode_set(CSB_V2_PM_V21_UPSCALED);
    check(dm1_v2_shape_runtime_v22_active() == 1, "DM1 V22 active");
    DM1_V2_ShapeRuntimeResult r = dm1_v2_shape_runtime_for_cell(0x04, 0, 1, 0);
    check(r.active == 1, "DM1 cell resolves");
    dm1_v2_presentation_mode_reset();
    csb_v2_presentation_mode_reset();
}

int main(void) {
    printf("=== DM1 V2.2 shape runtime probe ===\n");
    p_inactive_v1(); p_active_v22(); p_fallback_when_no_pack();
    p_v21_inactive(); p_composition(); p_evidence();
    p_independent_from_csb();
    printf("--- %d / %d passed ---\n", g_total - g_failed, g_total);
    return g_failed == 0 ? 0 : 1;
}
