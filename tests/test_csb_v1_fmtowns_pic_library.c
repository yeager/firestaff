#include "csb_v1_fmtowns_pic_library.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_null_gate(void) {
    dm1_v1_fmtowns_pic_library_view_t v;
    uint8_t buf[10] = {0};
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(NULL, 10, &v) == 0);
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, 10, NULL) == 0);
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, 3, &v) == 0);
}

static void test_wrong_sig_rejects(void) {
    dm1_v1_fmtowns_pic_library_view_t v;
    /* sig != 0x8001. */
    uint8_t buf[10] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, 10, &v) == 0);
}

static void test_real_data_csb(void) {
    const char *dir = getenv("FIRESTAFF_CSB_FMTOWNS_CDATA_DIR");
    FILE *fp;
    if (!dir || !dir[0]) { puts("SKIP: no CDATA dir"); return; }
    char path[1024];
    snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", dir);
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    fseek(fp, 0, SEEK_END);
    long sz = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    uint8_t *buf = (uint8_t *)malloc((size_t)sz);
    if (!buf || fread(buf, 1, (size_t)sz, fp) != (size_t)sz) {
        free(buf); fclose(fp); puts("SKIP: read failed"); return;
    }
    fclose(fp);
    dm1_v1_fmtowns_pic_library_view_t v;
    assert(csb_v1_fmtowns_pic_library_open_ext_v1_pc34(buf, (size_t)sz, &v) == 1);
    /* CSB has 728 assets, first asset size = 0xef (239). */
    assert(v.asset_count == 728);
    uint16_t first_size = 0;
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&v, 0, &first_size) ==
           DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(first_size == 0xef);
    /* Asset 10 = size 0x38 (56 bytes, byte-verified earlier). */
    uint16_t sz10 = 0;
    assert(dm1_v1_fmtowns_pic_library_asset_size_pc34(&v, 9, &sz10) ==
           DM1_V1_FMTOWNS_PIC_LIB_OK);
    assert(sz10 == 0x38);
    free(buf);
    puts("PASS: CSB CDATA/GRAPHICS.DAT ext_v1 opens as DM1 pic_library view with 728 assets");
}

int main(void) {
    test_null_gate();
    test_wrong_sig_rejects();
    test_real_data_csb();
    puts("All csb_v1_fmtowns_pic_library tests passed.");
    return 0;
}
