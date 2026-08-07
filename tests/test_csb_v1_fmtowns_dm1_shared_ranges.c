#include "csb_v1_fmtowns_dm1_shared_ranges.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_table_shape(void) {
    for (int i = 0; i < CSB_V1_FMTOWNS_DM1_SHARED_RANGE_COUNT; ++i) {
        const csb_v1_fmtowns_dm1_shared_range_t *r =
            &csb_v1_fmtowns_dm1_shared_ranges[i];
        assert(r->length_bytes > 0);
        assert(r->dm1_vaddr + 0x200u == r->dm1_file_offset);
        assert(r->csb_vaddr + 0x200u == r->csb_file_offset);
    }
}

static void test_translation(void) {
    uint32_t out = 0;
    /* OICON descriptor: DM1 file 0x226db (33 bytes into range 0). */
    assert(csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(0x226dbu, &out) == 1);
    assert(out == 0x28144u + (0x226dbu - 0x226a8u));

    /* Boundary — first byte of range 3 (large). */
    assert(csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(0x29776u, &out) == 1);
    assert(out == 0x2d666u);

    /* Boundary — last byte of range 3. */
    uint32_t last = 0x29776u + 33194u - 1u;
    assert(csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(last, &out) == 1);

    /* One past range 3 end: no mapping. */
    assert(csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(0x29776u + 33194u, &out) == 0);

    /* Gap between ranges: no mapping. */
    assert(csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(0x22ffcu, &out) == 0);

    /* NULL out rejected. */
    assert(csb_v1_fmtowns_dm1_to_csb_file_offset_pc34(0x226dbu, NULL) == 0);
}

static void test_real_data(void) {
    /* If both binaries are available, verify the ranges are
     * byte-identical for every byte. Skip otherwise. */
    const char *dm1_path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    const char *csb_path = getenv("FIRESTAFF_CSB_FMTOWNS_CHTWE_EXP");
    if (!dm1_path || !csb_path) { puts("SKIP: no bin paths"); return; }
    FILE *fa = fopen(dm1_path, "rb"), *fb = fopen(csb_path, "rb");
    if (!fa || !fb) { if(fa)fclose(fa); if(fb)fclose(fb); puts("SKIP: open"); return; }
    fseek(fa, 0, SEEK_END); long sa = ftell(fa); fseek(fa, 0, SEEK_SET);
    fseek(fb, 0, SEEK_END); long sb = ftell(fb); fseek(fb, 0, SEEK_SET);
    uint8_t *ba = (uint8_t*)malloc((size_t)sa), *bb = (uint8_t*)malloc((size_t)sb);
    if (fread(ba,1,sa,fa)!=(size_t)sa || fread(bb,1,sb,fb)!=(size_t)sb) {
        free(ba); free(bb); fclose(fa); fclose(fb); puts("SKIP: read"); return;
    }
    fclose(fa); fclose(fb);
    for (int i = 0; i < CSB_V1_FMTOWNS_DM1_SHARED_RANGE_COUNT; ++i) {
        const csb_v1_fmtowns_dm1_shared_range_t *r =
            &csb_v1_fmtowns_dm1_shared_ranges[i];
        assert((long)(r->dm1_file_offset + r->length_bytes) <= sa);
        assert((long)(r->csb_file_offset + r->length_bytes) <= sb);
        assert(memcmp(ba + r->dm1_file_offset,
                      bb + r->csb_file_offset,
                      r->length_bytes) == 0);
    }
    free(ba); free(bb);
    puts("PASS: all shared ranges byte-verified");
}

int main(void) {
    test_table_shape();
    test_translation();
    test_real_data();
    puts("All csb_v1_fmtowns_dm1_shared_ranges tests passed.");
    return 0;
}
