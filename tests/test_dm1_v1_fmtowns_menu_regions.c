#include "dm1_v1_fmtowns_menu_regions.h"
#include <assert.h>
#include <stdio.h>

static void test_menu_panel_region(void) {
    DM1_V1_FmtownsRegionRecord r;
    assert(dm1_v1_fmtowns_region_menu_panel_pc34(&r) == 1);
    /* Region 10 record from EDM.EXP registry: byte-verified via
     * parity-evidence/dm1_fmtowns_region_table.md. */
    assert(r.type   == 9);           /* SIZE node */
    assert(r.parent == 2);
    assert(r.a      == 87);          /* width  */
    assert(r.b      == 45);          /* height */
    assert(dm1_v1_fmtowns_region_menu_panel_pc34(NULL) == 0);
}

static void test_menu_clear_area_region(void) {
    DM1_V1_FmtownsRegionRecord r;
    assert(dm1_v1_fmtowns_region_menu_clear_area_pc34(&r) == 1);
    /* Region 11 record: parent is region 10, so its effective size
     * inherits 87x45; a/b are the anchor pair (319, 77). */
    assert(r.type   == 2);
    assert(r.parent == 10);
    assert(r.a      == 319);
    assert(r.b      == 77);
    assert(dm1_v1_fmtowns_region_menu_clear_area_pc34(NULL) == 0);
}

static void test_scale_coord_identity(void) {
    /* scale == 10000 is the GET_RGN_COORD 1:1 fast path. */
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(87, 10000) == 87);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(45, 10000) == 45);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(319, 10000) == 319);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(0, 10000) == 0);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(-1, 10000) == -1);
}

static void test_scale_coord_arithmetic(void) {
    /* Exact source * scale / 10000 (signed integer div, truncation). */
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(87, 5000) == 43);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(87, 20000) == 174);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(45, 2500) == 11);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(45, 15000) == 67);
    /* Zero-scale path degenerates to zero, which then triggers the
     * post-conversion <=0 gate at EDM 0x194ce / 0x194d5. */
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(87, 0) == 0);
}

static void test_parent_inheritance(void) {
    DM1_V1_FmtownsRegionRecord panel, clear;
    assert(dm1_v1_fmtowns_region_menu_panel_pc34(&panel) == 1);
    assert(dm1_v1_fmtowns_region_menu_clear_area_pc34(&clear) == 1);
    /* Region 11's parent pointer must equal region 10's ID -- that
     * is the source-lifted inheritance the FM Towns menu draw relies
     * on to get the 87x45 clear box behind SPC_BLOT. */
    assert(clear.parent == DM1_V1_FMTOWNS_REGION_ID_MENU_PANEL_SIZE);
    assert(panel.type == 9);
    /* Applying the identity scale to the parent's size returns the
     * effective clear-area size for region 11. */
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(panel.a, 10000) ==
           DM1_V1_FMTOWNS_REGION_MENU_PANEL_WIDTH_PX);
    assert(dm1_v1_fmtowns_region_scale_coord_pc34(panel.b, 10000) ==
           DM1_V1_FMTOWNS_REGION_MENU_PANEL_HEIGHT_PX);
}

int main(void) {
    test_menu_panel_region();
    test_menu_clear_area_region();
    test_scale_coord_identity();
    test_scale_coord_arithmetic();
    test_parent_inheritance();
    printf("All dm1_v1_fmtowns_menu_regions tests passed.\n");
    return 0;
}
