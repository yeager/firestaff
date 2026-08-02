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

    assert(r.entries[0].type_flag == 0x06);
    assert(r.entries[1].type_flag == 0x03);
    assert(r.entries[2].type_flag == 0x02);

    assert(r.entries[0].byte0 == 0x06);
    assert(r.entries[1].byte0 == 0x23);

    assert(dm2_v1_fmtowns_cd_dat_audio_track_count(&r) == 9);
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
    test_parse_null();
    test_parse_short();
    test_audio_count_null();
    printf("PASS: dm2_v1_fmtowns_cd_dat\n");
    return 0;
}
