#include "dm1_v1_fmtowns_text_geometry.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

static void test_real_data_edm(void) {
    /* Verify every CHAR_* constant reads back byte-exact from EDM.EXP
     * at the SYM1-recorded vaddrs. Skipped without env var. */
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    uint8_t w[2];
    if (!path || !path[0]) { puts("SKIP: no EDM.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    const struct {
        const char *name;
        long vaddr;
        int expected;
    } cases[] = {
        { "CHAR_X_SIZE",    0x26c8a, DM1_V1_FMTOWNS_CHAR_X_SIZE },
        { "CHAR_Y_SIZE",    0x26c8c, DM1_V1_FMTOWNS_CHAR_Y_SIZE },
        { "CHAR_X_SPC",     0x26c8e, DM1_V1_FMTOWNS_CHAR_X_SPC },
        { "CHAR_Y_SPC",     0x26c90, DM1_V1_FMTOWNS_CHAR_Y_SPC },
        { "CHAR_DESCENDER", 0x26c92, DM1_V1_FMTOWNS_CHAR_DESCENDER },
        { "CHAR_X_WID",     0x26c94, DM1_V1_FMTOWNS_CHAR_X_WID },
        { "CHAR_Y_HYT",     0x26c96, DM1_V1_FMTOWNS_CHAR_Y_HYT }
    };
    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        if (fseek(fp, 0x200 + cases[i].vaddr, SEEK_SET) != 0) {
            fclose(fp); puts("SKIP: seek"); return;
        }
        if (fread(w, 1, 2, fp) != 2) { fclose(fp); puts("SKIP: read"); return; }
        int got = (int)(w[0] | (w[1] << 8));
        assert(got == cases[i].expected);
    }
    fclose(fp);
    puts("PASS: real EDM.EXP CHAR_* constants match shipped values");
}

int main(void) {
    test_glyph_constants();
    test_screen_and_icon_constants();
    test_pixel_width();
    test_pixel_height();
    test_vaddr_lookup();
    test_real_data_edm();
    printf("All dm1_v1_fmtowns_text_geometry tests passed.\n");
    return 0;
}
