#include "dm2_v1_fmtowns_cd_dat.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>

static const uint8_t cd_dat_ref[40] = {
    0x06, 0x06, 0x0a, 0x06,
    0x23, 0x1a, 0x06, 0x03,
    0x0f, 0x24, 0x06, 0x02,
    0x09, 0x24, 0x06, 0x02,
    0x07, 0x1e, 0x06, 0x02,
    0x0f, 0x1a, 0x06, 0x02,
    0x25, 0x39, 0x06, 0x02,
    0x53, 0x24, 0x06, 0x02,
    0x54, 0x1a, 0x06, 0x02,
    0x54, 0x11, 0x06, 0x02,
};

static void test_parse_reference(void) {
    DM2_V1_FmtownsCdDatReceipt r;
    assert(dm2_v1_fmtowns_cd_dat_parse(cd_dat_ref, 40, &r) == 0);
    assert(r.valid == 1);
    assert(r.data_entries == 1);
    assert(r.audio_entries == 9);

    assert(r.entries[0].type_flag == DM2_FMTOWNS_CD_TYPE_DATA);
    assert(r.entries[1].type_flag == DM2_FMTOWNS_CD_TYPE_AUDIO_FIRST);
    assert(r.entries[2].type_flag == DM2_FMTOWNS_CD_TYPE_AUDIO);

    assert(r.entries[0].tbios_param0 == 0x06);
    assert(r.entries[1].tbios_param0 == 0x23);
    assert(r.entries[1].tbios_param2 == 0x06);

    assert(dm2_v1_fmtowns_cd_dat_audio_track_count(&r) == 9);
}

static void test_disc_track_mapping(void) {
    DM2_V1_FmtownsCdDatReceipt r;
    assert(dm2_v1_fmtowns_cd_dat_parse(cd_dat_ref, 40, &r) == 0);

    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, 0) == 2);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, 1) == 3);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, 6) == 8);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, 7) == 9);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, 8) == 10);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, 9) == 0);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(&r, -1) == 0);
    assert(dm2_v1_fmtowns_cd_dat_disc_track(NULL, 0) == 0);
}

static void test_parse_null(void) {
    DM2_V1_FmtownsCdDatReceipt r;
    assert(dm2_v1_fmtowns_cd_dat_parse(NULL, 0, &r) == -1);
    assert(r.valid == 0);
}

static void test_parse_short(void) {
    DM2_V1_FmtownsCdDatReceipt r;
    assert(dm2_v1_fmtowns_cd_dat_parse(cd_dat_ref, 20, &r) == -1);
}

static void test_audio_count_null(void) {
    assert(dm2_v1_fmtowns_cd_dat_audio_track_count(NULL) == 0);
}

int main(void) {
    test_parse_reference();
    test_disc_track_mapping();
    test_parse_null();
    test_parse_short();
    test_audio_count_null();
    printf("PASS: dm2_v1_fmtowns_cd_dat\n");
    return 0;
}
