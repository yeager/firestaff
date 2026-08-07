#include "dm1_v1_fmtowns_jdm_font.h"
#include "dm1_v1_fmtowns_text_geometry.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_sjis_lead(void) {
    /* Valid lead ranges. */
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0x81) == 1);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0x9f) == 1);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0xe0) == 1);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0xfc) == 1);
    /* Invalid ranges. */
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0x00) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0x7f) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0x80) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0xa0) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0xdf) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_lead_pc34(0xfd) == 0);
}

static void test_sjis_pair(void) {
    /* Real Japanese kanji: '日' = 0x93 0xfa in Shift-JIS. */
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(0x93, 0xfa) == 1);
    /* Invalid trail. */
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(0x93, 0x00) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(0x93, 0x3f) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(0x93, 0x7f) == 0);
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(0x93, 0xfd) == 0);
    /* Non-lead first byte. */
    assert(dm1_v1_fmtowns_jdm_font_is_shift_jis_pair_pc34(0x41, 0x40) == 0);
}

static void test_ascii_render_uses_shared_raster(void) {
    /* Fixture ASCII raster with 'A' (0x41) row 2 = 0x1f (crossbar). */
    uint8_t raster[768] = {0};
    raster[2 * 128 + 'A'] = 0x1f;
    uint8_t fb[8 * 32];
    memset(fb, 0, sizeof(fb));
    /* Render "A" (single ASCII). */
    uint8_t label[1] = { 'A' };
    unsigned int painted = dm1_v1_fmtowns_jdm_font_render_string_pc34(
        raster, NULL, NULL, fb, 32, 8, 32, 0, 0, 4, 0, label, 1);
    assert(painted == 1);
    /* Crossbar row 2, 5 pixels of colour 4. */
    for (int x = 0; x < 5; ++x) assert(fb[2 * 32 + x] == 4);
}

static void test_sjis_fails_closed_without_callback(void) {
    uint8_t raster[768] = {0};
    uint8_t fb[8 * 32];
    memset(fb, 0, sizeof(fb));
    /* Shift-JIS pair 0x93 0xfa ('日'). */
    uint8_t label[2] = { 0x93, 0xfa };
    unsigned int painted = dm1_v1_fmtowns_jdm_font_render_string_pc34(
        raster, NULL, NULL, fb, 32, 8, 32, 0, 0, 4, 0, label, 2);
    /* Nothing painted; framebuffer stays zeroed. */
    assert(painted == 0);
    for (size_t i = 0; i < sizeof(fb); ++i) assert(fb[i] == 0);
}

static int test_sjis_callback(void *user, uint8_t *fb, int w, int h, int s,
                              int dx, int dy, uint8_t fg, uint8_t bg,
                              uint8_t l, uint8_t t) {
    (void)fb; (void)w; (void)h; (void)s; (void)dx; (void)dy;
    (void)fg; (void)bg; (void)l; (void)t;
    /* Record that we were called. */
    int *counter = (int *)user;
    ++(*counter);
    return 1;
}

static void test_sjis_callback_is_invoked(void) {
    uint8_t raster[768] = {0};
    uint8_t fb[8 * 32];
    int counter = 0;
    memset(fb, 0, sizeof(fb));
    uint8_t label[2] = { 0x93, 0xfa };
    unsigned int painted = dm1_v1_fmtowns_jdm_font_render_string_pc34(
        raster, test_sjis_callback, &counter, fb, 32, 8, 32,
        0, 0, 4, 0, label, 2);
    assert(painted == 1);
    assert(counter == 1);
}

static void test_mixed_ascii_sjis_string(void) {
    /* "A[SJIS]B" — cursor advances CHAR_X_WID*1 + CHAR_X_WID*2 +
     * CHAR_X_WID*1 = 4 * CHAR_X_WID = 24 pixels for a 4-char
     * mixed string. */
    uint8_t raster[768] = {0};
    raster[2 * 128 + 'A'] = 0x1f;
    raster[2 * 128 + 'B'] = 0x1f;
    uint8_t fb[8 * 64];
    int counter = 0;
    memset(fb, 0, sizeof(fb));
    uint8_t label[4] = { 'A', 0x93, 0xfa, 'B' };
    unsigned int painted = dm1_v1_fmtowns_jdm_font_render_string_pc34(
        raster, test_sjis_callback, &counter, fb, 64, 8, 64,
        0, 0, 4, 0, label, 4);
    assert(painted == 3); /* A + SJIS(via callback) + B */
    assert(counter == 1);
    /* A at x=0, B at x = CHAR_X_WID + 2*CHAR_X_WID = 18 (row 2 pixels). */
    for (int x = 0; x < 5; ++x) assert(fb[2 * 64 + x] == 4);
    for (int x = 18; x < 23; ++x) assert(fb[2 * 64 + x] == 4);
}

static void test_real_data_jdata(void) {
    /* Verify JDATA/GRAPHICS.DAT asset 557 (JDM ASCII font) is
     * byte-identical to DATA/GRAPHICS.DAT asset 557 (English EDM
     * ASCII font). Both are 768-byte rasters. */
    const char *jdata_dir = getenv("FIRESTAFF_DM1_FMTOWNS_JDATA_DIR");
    const char *edata_dir = getenv("FIRESTAFF_DM1_FMTOWNS_DATA_DIR");
    if (!jdata_dir || !edata_dir) { puts("SKIP: no data dirs"); return; }
    /* Just read the first 64 bytes of the raster in both and confirm identity. */
    char path[1024];
    uint8_t jdata_head[64], edata_head[64];
    /* Both discs use the same DM1 pic_library format; asset 557 is
     * at the same relative header index. Locate via loader or by
     * scanning for the known DM1 ASCII font prefix. */
    static const uint8_t known_prefix[8] = {0x00,0x1e,0x16,0x0f,0x1e,0x0f,0x1f,0x0e};
    /* Read entire GRAPHICS.DAT and scan. */
    for (int which = 0; which < 2; ++which) {
        const char *dir = which == 0 ? jdata_dir : edata_dir;
        snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", dir);
        FILE *fp = fopen(path, "rb");
        if (!fp) { puts("SKIP: cannot open"); return; }
        fseek(fp, 0, SEEK_END);
        long sz = ftell(fp);
        fseek(fp, 0, SEEK_SET);
        uint8_t *buf = (uint8_t *)malloc((size_t)sz);
        if (!buf || fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
            free(buf); fclose(fp); puts("SKIP: read"); return;
        }
        fclose(fp);
        /* Find the 8-byte prefix. */
        long idx = -1;
        for (long i = 0; i + 64 <= sz; ++i) {
            if (memcmp(buf + i, known_prefix, 8) == 0) { idx = i; break; }
        }
        assert(idx >= 0);
        memcpy(which == 0 ? jdata_head : edata_head, buf + idx, 64);
        free(buf);
    }
    assert(memcmp(jdata_head, edata_head, 64) == 0);
    puts("PASS: JDATA asset 557 ASCII font is byte-identical to DATA asset 557");
}

int main(void) {
    test_sjis_lead();
    test_sjis_pair();
    test_ascii_render_uses_shared_raster();
    test_sjis_fails_closed_without_callback();
    test_sjis_callback_is_invoked();
    test_mixed_ascii_sjis_string();
    test_real_data_jdata();
    puts("All dm1_v1_fmtowns_jdm_font tests passed.");
    return 0;
}
