#include "dm1_v1_fmtowns_text_geometry.h"
#include <assert.h>
#include <stdio.h>

static void test_glyph_constants(void) {
    /* Byte-verified reads of EDM.EXP initialised data. */
    assert(DM1_V1_FMTOWNS_CHAR_X_SIZE    == 5);
    assert(DM1_V1_FMTOWNS_CHAR_Y_SIZE    == 6);
    assert(DM1_V1_FMTOWNS_CHAR_X_SPC     == 1);
    assert(DM1_V1_FMTOWNS_CHAR_Y_SPC     == 1);
    assert(DM1_V1_FMTOWNS_CHAR_DESCENDER == 1);
    assert(DM1_V1_FMTOWNS_CHAR_X_WID     == 6);
    assert(DM1_V1_FMTOWNS_CHAR_Y_HYT     == 7);
    /* Verify the advance folds in body + spacing exactly. */
    assert(DM1_V1_FMTOWNS_CHAR_X_WID ==
           DM1_V1_FMTOWNS_CHAR_X_SIZE + DM1_V1_FMTOWNS_CHAR_X_SPC);
    assert(DM1_V1_FMTOWNS_CHAR_Y_HYT ==
           DM1_V1_FMTOWNS_CHAR_Y_SIZE + DM1_V1_FMTOWNS_CHAR_Y_SPC);
}

static void test_screen_and_icon_constants(void) {
    assert(DM1_V1_FMTOWNS_SCR_X_SIZE  == 320);
    assert(DM1_V1_FMTOWNS_ICON_SIZE   == 256);
    assert(DM1_V1_FMTOWNS_ICON_X_SIZE == 16);
    assert(DM1_V1_FMTOWNS_ICON_Y_SIZE == 16);
    /* 16 x 16 icons at 1 byte per pixel = 256 bytes. */
    assert(DM1_V1_FMTOWNS_ICON_X_SIZE * DM1_V1_FMTOWNS_ICON_Y_SIZE ==
           DM1_V1_FMTOWNS_ICON_SIZE);
}

static void test_pixel_width(void) {
    assert(dm1_v1_fmtowns_text_pixel_width_pc34(0)  == 0);
    assert(dm1_v1_fmtowns_text_pixel_width_pc34(-3) == 0);
    assert(dm1_v1_fmtowns_text_pixel_width_pc34(1)  == 6);
    /* "BLOCK" = 5 chars => 30 px. */
    assert(dm1_v1_fmtowns_text_pixel_width_pc34(5)  == 30);
    /* "SPELLSHIELD" = 11 chars => 66 px. Fits in 87px panel. */
    assert(dm1_v1_fmtowns_text_pixel_width_pc34(11) == 66);
}

static void test_pixel_height(void) {
    assert(dm1_v1_fmtowns_text_pixel_height_pc34(0)  == 0);
    assert(dm1_v1_fmtowns_text_pixel_height_pc34(-1) == 0);
    assert(dm1_v1_fmtowns_text_pixel_height_pc34(1)  == 7);
    /* Three-button DYNAMENU panel: 3 rows @ 7px = 21 px vertical. */
    assert(dm1_v1_fmtowns_text_pixel_height_pc34(3)  == 21);
    /* Full 45px panel holds floor(45/7) = 6 rows. */
    assert(dm1_v1_fmtowns_text_pixel_height_pc34(6)  == 42);
}

static void test_vaddr_lookup(void) {
    assert(dm1_v1_fmtowns_text_geometry_vaddr_pc34("CHAR_X_SIZE") == 0x26c8au);
    assert(dm1_v1_fmtowns_text_geometry_vaddr_pc34("CHAR_Y_HYT")  == 0x26c96u);
    assert(dm1_v1_fmtowns_text_geometry_vaddr_pc34("SCR_X_SIZE")  == 0x26c68u);
    assert(dm1_v1_fmtowns_text_geometry_vaddr_pc34("ICON_X_SIZE") == 0x26c78u);
    assert(dm1_v1_fmtowns_text_geometry_vaddr_pc34("nope") == 0u);
    assert(dm1_v1_fmtowns_text_geometry_vaddr_pc34(NULL) == 0u);
}

int main(void) {
    test_glyph_constants();
    test_screen_and_icon_constants();
    test_pixel_width();
    test_pixel_height();
    test_vaddr_lookup();
    printf("All dm1_v1_fmtowns_text_geometry tests passed.\n");
    return 0;
}
