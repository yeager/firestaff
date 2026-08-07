#include "dm2_v1_fmtowns_cross_game_shared_ranges.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_shape(void) {
    for (int i = 0; i < DM2_V1_FMTOWNS_DM1_TO_DM2_RANGE_COUNT; ++i) {
        const dm2_v1_fmtowns_cross_range_t *r =
            &dm2_v1_fmtowns_dm1_to_dm2_ranges[i];
        assert(r->length_bytes > 0);
        assert(r->src_vaddr + 0x200u == r->src_file_offset);
        assert(r->dm2_vaddr + 0x200u == r->dm2_file_offset);
    }
    for (int i = 0; i < DM2_V1_FMTOWNS_CSB_TO_DM2_RANGE_COUNT; ++i) {
        const dm2_v1_fmtowns_cross_range_t *r =
            &dm2_v1_fmtowns_csb_to_dm2_ranges[i];
        assert(r->length_bytes > 0);
    }
}

static void test_translation(void) {
    uint32_t out = 0;
    /* First DM1 range: base translates to DM2 base. */
    assert(dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(0x029788u, &out) == 1);
    assert(out == 0x006928u);
    /* 100 bytes into it. */
    assert(dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(0x029788u + 100u, &out) == 1);
    assert(out == 0x006928u + 100u);
    /* Just past end of last DM1 range. */
    uint32_t last = 0x040347u + 597u;
    assert(dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(last, &out) == 0);
    /* Gap. */
    assert(dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(0x00u, &out) == 0);
    /* CSB translation smoke check. */
    assert(dm2_v1_fmtowns_csb_to_dm2_file_offset_pc34(0x02d678u, &out) == 1);
    assert(out == 0x00693cu);
    /* NULL out. */
    assert(dm2_v1_fmtowns_dm1_to_dm2_file_offset_pc34(0x029788u, NULL) == 0);
}

static int verify_ranges_against_files(const char *src_path,
                                       const char *dm2_path,
                                       const dm2_v1_fmtowns_cross_range_t *tbl,
                                       int n,
                                       const char *label) {
    FILE *fa = fopen(src_path, "rb"), *fb = fopen(dm2_path, "rb");
    if (!fa || !fb) { if(fa)fclose(fa); if(fb)fclose(fb); return -1; }
    fseek(fa, 0, SEEK_END); long sa = ftell(fa); fseek(fa, 0, SEEK_SET);
    fseek(fb, 0, SEEK_END); long sb = ftell(fb); fseek(fb, 0, SEEK_SET);
    uint8_t *ba = (uint8_t*)malloc((size_t)sa), *bb = (uint8_t*)malloc((size_t)sb);
    if (fread(ba,1,sa,fa)!=(size_t)sa || fread(bb,1,sb,fb)!=(size_t)sb) {
        free(ba); free(bb); fclose(fa); fclose(fb); return -1;
    }
    fclose(fa); fclose(fb);
    for (int i = 0; i < n; ++i) {
        assert((long)(tbl[i].src_file_offset + tbl[i].length_bytes) <= sa);
        assert((long)(tbl[i].dm2_file_offset + tbl[i].length_bytes) <= sb);
        assert(memcmp(ba + tbl[i].src_file_offset,
                      bb + tbl[i].dm2_file_offset,
                      tbl[i].length_bytes) == 0);
    }
    free(ba); free(bb);
    printf("PASS: %s ranges byte-verified\n", label);
    return 0;
}

static void test_real_data(void) {
    const char *dm1 = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    const char *csb = getenv("FIRESTAFF_CSB_FMTOWNS_CHTWE_EXP");
    const char *dm2 = getenv("FIRESTAFF_DM2_FMTOWNS_SKULL_EXP");
    if (!dm2) { puts("SKIP: no DM2 SKULL.EXP path"); return; }
    if (dm1) verify_ranges_against_files(
        dm1, dm2, dm2_v1_fmtowns_dm1_to_dm2_ranges,
        DM2_V1_FMTOWNS_DM1_TO_DM2_RANGE_COUNT, "DM1->DM2");
    if (csb) verify_ranges_against_files(
        csb, dm2, dm2_v1_fmtowns_csb_to_dm2_ranges,
        DM2_V1_FMTOWNS_CSB_TO_DM2_RANGE_COUNT, "CSB->DM2");
}

int main(void) {
    test_shape();
    test_translation();
    test_real_data();
    puts("All dm2_v1_fmtowns_cross_game_shared_ranges tests passed.");
    return 0;
}
