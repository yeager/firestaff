#include "csb_v1_fmtowns_oicon_descriptor.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_alias_consistency(void) {
    /* The aliases must map to DM1's underlying tables byte-for-byte. */
    assert(CSB_V1_FMTOWNS_OICON_KIND_COUNT == 224);
    for (uint16_t i = 0; i < 224; ++i) {
        assert(csb_v1_fmtowns_oicon_kind[i] == dm1_v1_fmtowns_oicon_kind[i]);
    }
    /* Vaddr accessor. */
    assert(csb_v1_fmtowns_oicon_vaddr_in_chtwe_pc34() == 0x27f77u);
}

static void test_alias_accessors(void) {
    /* Same lookup semantics as DM1. */
    assert(csb_v1_fmtowns_oicon_kind_at_pc34(0) == 0);
    assert(csb_v1_fmtowns_oicon_kind_at_pc34(5) == 42);
    assert(csb_v1_fmtowns_oicon_is_thing_pc34(5) == 1);
    assert(csb_v1_fmtowns_oicon_is_thing_pc34(0) == 0);
    assert(csb_v1_fmtowns_oicon_kind_at_pc34(224) == 0xff);
}

static void test_real_data_csb(void) {
    const char *path = getenv("FIRESTAFF_CSB_FMTOWNS_CHTWE_EXP");
    FILE *fp;
    uint8_t buf[1344];
    if (!path || !path[0]) { puts("SKIP: no CHTWE.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    if (fseek(fp, 0x200 + CSB_V1_FMTOWNS_OICON_VADDR, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek"); return;
    }
    if (fread(buf, 1, 1344, fp) != 1344) {
        fclose(fp); puts("SKIP: read"); return;
    }
    fclose(fp);
    /* Compare every one of 1344 bytes to DM1's shipped table. */
    for (uint16_t i = 0; i < 224; ++i) {
        for (unsigned j = 0; j < 6; ++j) {
            assert(buf[i*6 + j] == dm1_v1_fmtowns_oicon_descriptor[i][j]);
        }
    }
    puts("PASS: CSB CHTWE.EXP OICON is byte-identical to DM1 EDM.EXP OICON");
}

int main(void) {
    test_alias_consistency();
    test_alias_accessors();
    test_real_data_csb();
    puts("All csb_v1_fmtowns_oicon_descriptor tests passed.");
    return 0;
}
