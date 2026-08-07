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

static void test_block_1_full_table(void) {
    /* Byte-exact transcription from parity-evidence disassembly.
     * If a record drifts, this test catches it before any consumer. */
    static const DM1_V1_FmtownsRegionRecord expected[17] = {
        { 9,  0, 320, 200 }, { 1,  1,   0,   0 }, { 9,  0, 224, 136 },
        { 1,  3,   0,   0 }, { 10, 2,   0,   0 }, { 10, 4,   0,   0 },
        { 1,  3,   0,  33 }, { 9,  2,  87,  45 }, { 3,  8, 319, 168 },
        { 9,  2,  87,  45 }, { 2, 10, 319,  77 }, { 9,  2,  87,  33 },
        { 3, 12, 319,  74 }, { 9,  2, 320,  31 }, { 4, 14,   0, 199 },
        { 9,  2,  87,   6 }, { 1, 16, 233,  33 },
    };
    for (int i = 0; i < 17; ++i) {
        const DM1_V1_FmtownsRegionRecord *got =
            &dm1_v1_fmtowns_region_block_1[i];
        assert(got->type   == expected[i].type);
        assert(got->parent == expected[i].parent);
        assert(got->a      == expected[i].a);
        assert(got->b      == expected[i].b);
    }
}

static void test_lookup_generic(void) {
    DM1_V1_FmtownsRegionRecord r;
    /* Every ID 1..17 must resolve to the byte-exact record. */
    for (uint16_t id = 1; id <= 17; ++id) {
        assert(dm1_v1_fmtowns_region_lookup_pc34(id, &r) == 1);
    }
    /* Region 10 must match the dedicated helper. */
    assert(dm1_v1_fmtowns_region_lookup_pc34(10, &r) == 1);
    assert(r.type == 9 && r.parent == 2 && r.a == 87 && r.b == 45);
    /* Region 11 too. */
    assert(dm1_v1_fmtowns_region_lookup_pc34(11, &r) == 1);
    assert(r.type == 2 && r.parent == 10 && r.a == 319 && r.b == 77);
    /* IDs outside every block's range fail closed. Block 1 covers
     * 400..419 so 399 must fail; block 22 (the tail) covers 110..116.
     * Total decoded coverage: 994 records across 23 disjoint ranges. */
    assert(dm1_v1_fmtowns_region_lookup_pc34(0, &r) == 0);
    assert(dm1_v1_fmtowns_region_lookup_pc34(18, &r) == 0);   /* gap after block 0 */
    assert(dm1_v1_fmtowns_region_lookup_pc34(399, &r) == 0);  /* just below block 1 */
    assert(dm1_v1_fmtowns_region_lookup_pc34(9999, &r) == 0);
    /* 400 is now a valid ID (block 1). */
    assert(dm1_v1_fmtowns_region_lookup_pc34(400, &r) == 1);
    /* NULL out gate. */
    assert(dm1_v1_fmtowns_region_lookup_pc34(10, NULL) == 0);
}

static void test_all_blocks_decoded(void) {
    /* Sanity: every block header's range should resolve every id inside it. */
    DM1_V1_FmtownsRegionRecord r;
    unsigned int total_hits = 0;
    for (unsigned int b = 0; b < DM1_V1_FMTOWNS_REGION_BLOCK_COUNT; ++b) {
        const DM1_V1_FmtownsRegionBlock *blk =
            &dm1_v1_fmtowns_region_blocks[b];
        for (uint16_t id = blk->id_min; id <= blk->id_max; ++id) {
            assert(dm1_v1_fmtowns_region_lookup_pc34(id, &r) == 1);
            ++total_hits;
        }
    }
    assert(total_hits == DM1_V1_FMTOWNS_REGION_TOTAL_RECORDS);
    assert(total_hits == 994);
}

int main(void) {
    test_menu_panel_region();
    test_menu_clear_area_region();
    test_scale_coord_identity();
    test_scale_coord_arithmetic();
    test_parent_inheritance();
    test_block_1_full_table();
    test_lookup_generic();
    test_all_blocks_decoded();
    printf("All dm1_v1_fmtowns_menu_regions tests passed.\n");
    return 0;
}
