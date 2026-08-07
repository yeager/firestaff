#include "csb_v1_fmtowns_dyna_buttons.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_alias(void) {
    /* Verify aliased lookup returns the same labels DM1 does. */
    assert(csb_v1_fmtowns_dyna_button_label_pc34(1) != NULL);
    assert(strcmp(csb_v1_fmtowns_dyna_button_label_pc34(1), "BLOCK") == 0);
    assert(strcmp(csb_v1_fmtowns_dyna_button_label_pc34(20), "FIREBALL") == 0);
    assert(strcmp(csb_v1_fmtowns_dyna_button_label_pc34(43), "FUSE") == 0);
    /* Vaddr accessor */
    assert(csb_v1_fmtowns_dyna_buttons_vaddr_in_chtwe_pc34() == 0x29d50u);
}

static void test_real_data(void) {
    const char *path = getenv("FIRESTAFF_CSB_FMTOWNS_CHTWE_EXP");
    FILE *fp;
    uint8_t csb_buf[500];
    uint8_t dm1_buf[500];
    if (!path || !path[0]) { puts("SKIP: no CHTWE.EXP"); return; }
    /* Read CSB first 500 bytes of DYNA_BUTTONS. */
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open CSB"); return; }
    if (fseek(fp, 0x200 + CSB_V1_FMTOWNS_DYNA_BUTTONS_VADDR, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek"); return;
    }
    if (fread(csb_buf, 1, 500, fp) != 500) {
        fclose(fp); puts("SKIP: read"); return;
    }
    fclose(fp);
    /* Read DM1's DYNA_BUTTONS from EDM.EXP. */
    const char *dm1_path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    if (!dm1_path || !dm1_path[0]) { puts("SKIP: no EDM.EXP for DM1 comparison"); return; }
    fp = fopen(dm1_path, "rb");
    if (!fp) { puts("SKIP: cannot open DM1"); return; }
    if (fseek(fp, 0x200 + 0x24194, SEEK_SET) != 0) {
        fclose(fp); puts("SKIP: seek"); return;
    }
    if (fread(dm1_buf, 1, 500, fp) != 500) {
        fclose(fp); puts("SKIP: read"); return;
    }
    fclose(fp);
    /* Byte-identity. */
    assert(memcmp(csb_buf, dm1_buf, 500) == 0);
    puts("PASS: CSB CHTWE.EXP DYNA_BUTTONS 500 bytes byte-identical to DM1 EDM.EXP");
}

int main(void) {
    test_alias();
    test_real_data();
    puts("All csb_v1_fmtowns_dyna_buttons tests passed.");
    return 0;
}
