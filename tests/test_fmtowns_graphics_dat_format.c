#include "fmtowns_graphics_dat_format.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_null_gate(void) {
    fmtowns_graphics_dat_identity_t id;
    uint8_t buf[10] = {0};
    assert(fmtowns_graphics_dat_identify_pc34(NULL, 10, &id) == 0);
    assert(fmtowns_graphics_dat_identify_pc34(buf, 10, NULL) == 0);
    assert(fmtowns_graphics_dat_identify_pc34(buf, 3, &id) == 0);
}

static void test_legacy_dm1(void) {
    /* DM1: word0 = 575 = 0x023f */
    uint8_t buf[4] = { 0x3f, 0x02, 0x00, 0x00 };
    fmtowns_graphics_dat_identity_t id;
    assert(fmtowns_graphics_dat_identify_pc34(buf, 4, &id) == 1);
    assert(id.format == FMTOWNS_GRAPHICS_DAT_FORMAT_LEGACY);
    assert(id.asset_count == 575);
    assert(id.record_size_bytes == 2);
    assert(id.header_size_bytes == 2 + 575 * 2);
    assert(id.signature == 0x023f);
}

static void test_ext_v1_csb(void) {
    /* CSB: word0=0x8001, word1=0x02d8=728 */
    uint8_t buf[4] = { 0x01, 0x80, 0xd8, 0x02 };
    fmtowns_graphics_dat_identity_t id;
    assert(fmtowns_graphics_dat_identify_pc34(buf, 4, &id) == 1);
    assert(id.format == FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1);
    assert(id.asset_count == 728);
    /* CSB ext_v1 uses DM1 legacy layout under a sig prefix: 2-byte
     * per size-table entry, twin primary+secondary tables. */
    assert(id.record_size_bytes == 2);
    assert(id.header_size_bytes == 4 + 728 * 4);
}

static void test_ext_v4_dm2(void) {
    /* DM2: word0=0x8004, word1=0x0d4f=3407 */
    uint8_t buf[4] = { 0x04, 0x80, 0x4f, 0x0d };
    fmtowns_graphics_dat_identity_t id;
    assert(fmtowns_graphics_dat_identify_pc34(buf, 4, &id) == 1);
    assert(id.format == FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4);
    assert(id.asset_count == 3407);
    assert(id.header_size_bytes == 4 + 3407 * 4);
}

static void test_unknown_extended(void) {
    uint8_t buf[4] = { 0xff, 0x8f, 0x00, 0x00 };
    fmtowns_graphics_dat_identity_t id;
    /* Sig 0x8fff is not one of the shipping formats -> UNKNOWN. */
    assert(fmtowns_graphics_dat_identify_pc34(buf, 4, &id) == 0);
    assert(id.format == FMTOWNS_GRAPHICS_DAT_FORMAT_UNKNOWN);
    assert(id.signature == 0x8fff);
}

static void test_expected_counts(void) {
    assert(fmtowns_graphics_dat_expected_asset_count_pc34(
        FMTOWNS_GRAPHICS_DAT_FORMAT_LEGACY) == 575);
    assert(fmtowns_graphics_dat_expected_asset_count_pc34(
        FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1) == 728);
    assert(fmtowns_graphics_dat_expected_asset_count_pc34(
        FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4) == 3407);
    assert(fmtowns_graphics_dat_expected_asset_count_pc34(
        FMTOWNS_GRAPHICS_DAT_FORMAT_UNKNOWN) == 0);
}

static void test_real_data_all_three_games(void) {
    /* Test each game's GRAPHICS.DAT if env vars supply the path. */
    const struct {
        const char *env;
        fmtowns_graphics_dat_format_t expected_format;
        uint16_t expected_count;
    } cases[] = {
        { "FIRESTAFF_DM1_FMTOWNS_DATA_DIR",
          FMTOWNS_GRAPHICS_DAT_FORMAT_LEGACY,  575 },
        { "FIRESTAFF_CSB_FMTOWNS_CDATA_DIR",
          FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V1,  728 },
        { "FIRESTAFF_DM2_FMTOWNS_DATA_DIR",
          FMTOWNS_GRAPHICS_DAT_FORMAT_EXT_V4, 3407 }
    };
    int ran = 0;
    for (unsigned i = 0; i < sizeof(cases)/sizeof(cases[0]); ++i) {
        const char *dir = getenv(cases[i].env);
        if (!dir || !dir[0]) continue;
        char path[1024];
        snprintf(path, sizeof(path), "%s/GRAPHICS.DAT", dir);
        FILE *fp = fopen(path, "rb");
        if (!fp) continue;
        uint8_t hdr[4];
        if (fread(hdr, 1, 4, fp) != 4) { fclose(fp); continue; }
        fclose(fp);
        fmtowns_graphics_dat_identity_t id;
        assert(fmtowns_graphics_dat_identify_pc34(hdr, 4, &id) == 1);
        assert(id.format == cases[i].expected_format);
        assert(id.asset_count == cases[i].expected_count);
        ++ran;
    }
    if (ran == 0) puts("SKIP: no GRAPHICS.DAT env var set");
    else printf("PASS: %d real GRAPHICS.DAT header(s) classified correctly\n", ran);
}

int main(void) {
    test_null_gate();
    test_legacy_dm1();
    test_ext_v1_csb();
    test_ext_v4_dm2();
    test_unknown_extended();
    test_expected_counts();
    test_real_data_all_three_games();
    puts("All fmtowns_graphics_dat_format tests passed.");
    return 0;
}
