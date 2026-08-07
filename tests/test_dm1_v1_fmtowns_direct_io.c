#include "dm1_v1_fmtowns_direct_io.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_null_gate(void) {
    dm1_v1_fmtowns_direct_io_profile_t p;
    uint8_t buf[10] = {0};
    assert(dm1_v1_fmtowns_direct_io_count_pc34(NULL, 10, &p) == 0);
    assert(dm1_v1_fmtowns_direct_io_count_pc34(buf, 10, NULL) == 0);
    assert(dm1_v1_fmtowns_direct_io_count_pc34(buf, 5, &p) == 0);
}

static void test_recovered_profiles(void) {
    /* EDM.EXP: 1 SOUND_INT_REASON only */
    assert(dm1_v1_fmtowns_direct_io_profile_edm_exp_pc34.sound_int_reason == 1);
    assert(dm1_v1_fmtowns_direct_io_profile_edm_exp_pc34.total == 1);
    /* JDM.EXP matches EDM.EXP exactly */
    assert(dm1_v1_fmtowns_direct_io_profile_jdm_exp_pc34.sound_int_reason == 1);
    assert(dm1_v1_fmtowns_direct_io_profile_jdm_exp_pc34.total == 1);
    /* TMENU: 1+1+2+1+2 = 7 total */
    assert(dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34.sound_int_reason == 1);
    assert(dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34.rs232c_modem_control == 1);
    assert(dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34.cmos_boot_dev_flag == 2);
    assert(dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34.cmos_def_boot_dev_type == 1);
    assert(dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34.cmos_def_boot_dev_unit == 2);
    assert(dm1_v1_fmtowns_direct_io_profile_tmenu_exp_pc34.total == 7);
}

static void test_scanner_counts(void) {
    uint8_t img[64];
    dm1_v1_fmtowns_direct_io_profile_t p;
    memset(img, 0x90, sizeof(img));
    unsigned int o = 0;
    /* mov dx, 0x04E9; in al, dx */
    img[o++]=0x66; img[o++]=0xba; img[o++]=0xE9; img[o++]=0x04; img[o++]=0xec;
    /* mov dx, 0x3180; in al, dx */
    img[o++]=0x66; img[o++]=0xba; img[o++]=0x80; img[o++]=0x31; img[o++]=0xec;
    /* mov dx, 0xFFFF; out dx, al  (non-DM1 port, ignored) */
    img[o++]=0x66; img[o++]=0xba; img[o++]=0xff; img[o++]=0xff; img[o++]=0xee;
    /* mov dx, 0x3180; in al, dx  (second hit on same port) */
    img[o++]=0x66; img[o++]=0xba; img[o++]=0x80; img[o++]=0x31; img[o++]=0xec;

    assert(dm1_v1_fmtowns_direct_io_count_pc34(img, sizeof(img), &p) == 1);
    assert(p.sound_int_reason == 1);
    assert(p.cmos_boot_dev_flag == 2);
    assert(p.total == 3); /* non-DM1 port was ignored */
}

static void test_real_data_edm(void) {
    const char *path = getenv("FIRESTAFF_DM1_FMTOWNS_EDM_EXP");
    FILE *fp;
    long size;
    uint8_t *buf;
    dm1_v1_fmtowns_direct_io_profile_t p;
    if (!path || !path[0]) { puts("SKIP: no EDM.EXP"); return; }
    fp = fopen(path, "rb");
    if (!fp) { puts("SKIP: cannot open"); return; }
    fseek(fp, 0, SEEK_END); size = ftell(fp); fseek(fp, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)size);
    if (!buf || fread(buf, 1, (size_t)size, fp) != (size_t)size) {
        free(buf); fclose(fp); puts("SKIP: read failed"); return;
    }
    fclose(fp);
    assert(dm1_v1_fmtowns_direct_io_count_pc34(buf, (unsigned long)size, &p) == 1);
    /* EDM.EXP: only touches SOUND_INT_REASON (0x04E9) once. */
    assert(p.sound_int_reason == 1);
    assert(p.total == 1);
    free(buf);
    puts("PASS: real EDM.EXP direct-I/O profile matches shipped constants");
}

int main(void) {
    test_null_gate();
    test_recovered_profiles();
    test_scanner_counts();
    test_real_data_edm();
    puts("All dm1_v1_fmtowns_direct_io tests passed.");
    return 0;
}
