#include "dm1_v1_fmtowns_egb_rect.h"
#include <assert.h>
#include <stdio.h>

static void test_null_gate(void) {
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(1, NULL) ==
           DM1_V1_FMTOWNS_EGB_RECT_ERR_NULL);
}

static void test_unknown_id(void) {
    dm1_v1_fmtowns_egb_rect_t r;
    /* 0 is not in any block; 18..64 gap; 9999 far above. */
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(0, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_ERR_UNKNOWN_ID);
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(9999, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_ERR_UNKNOWN_ID);
}

static void test_root_size_region(void) {
    /* Region 1: type 9, size 320x200. Origin at (0,0). */
    dm1_v1_fmtowns_egb_rect_t r;
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(1, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_OK);
    assert(r.x1 == 0 && r.y1 == 0);
    assert(r.x2 == 319 && r.y2 == 199);
    assert(r.width == 320 && r.height == 200);
}

static void test_menu_panel_rect(void) {
    /* Region 10: type 9 size 87x45 (parent 2 = origin at 0,0).
     * This is the SPC_BLOT panel size — resolved rect origin (0,0). */
    dm1_v1_fmtowns_egb_rect_t r;
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(10, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_OK);
    assert(r.width == 87 && r.height == 45);
    assert(r.x1 == 0 && r.y1 == 0);
    assert(r.x2 == 86 && r.y2 == 44);
}

static void test_menu_clear_area_anchor(void) {
    /* Region 11: type 2, parent 10, anchor (319, 77) = bottom-right.
     * So (x2, y2) = (0 + 319, 0 + 77) = (319, 77) and rect is 87x45
     * -> x1 = 319 - 87 + 1 = 233, y1 = 77 - 45 + 1 = 33. */
    dm1_v1_fmtowns_egb_rect_t r;
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(11, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_OK);
    assert(r.width == 87 && r.height == 45);
    assert(r.x1 == 233 && r.y1 == 33);
    assert(r.x2 == 319 && r.y2 == 77);
}

static void test_viewport_size(void) {
    /* Region 3: type 9 size 224x136 (viewport). Parent 0 -> origin. */
    dm1_v1_fmtowns_egb_rect_t r;
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(3, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_OK);
    assert(r.width == 224 && r.height == 136);
    assert(r.x1 == 0 && r.y1 == 0);
}

static void test_message_bar(void) {
    /* Region 14: type 9 size 320x31 (message bar). */
    dm1_v1_fmtowns_egb_rect_t r;
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(14, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_OK);
    assert(r.width == 320 && r.height == 31);
    /* Region 15: type 4, parent 14, anchor (0, 199) = bottom-left.
     * So (x1, y2) = (0, 199), width 320 height 31
     * -> x2 = 0 + 320 - 1 = 319, y1 = 199 - 31 + 1 = 169. */
    assert(dm1_v1_fmtowns_egb_rect_resolve_pc34(15, &r) ==
           DM1_V1_FMTOWNS_EGB_RECT_OK);
    assert(r.x1 == 0 && r.x2 == 319);
    assert(r.y1 == 169 && r.y2 == 199);
    assert(r.width == 320 && r.height == 31);
}

int main(void) {
    test_null_gate();
    test_unknown_id();
    test_root_size_region();
    test_menu_panel_rect();
    test_menu_clear_area_anchor();
    test_viewport_size();
    test_message_bar();
    puts("All dm1_v1_fmtowns_egb_rect tests passed.");
    return 0;
}
