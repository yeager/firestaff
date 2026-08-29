/*
 * Source-independent unit coverage for the Amiga CD.DAT parser.
 *
 * Retail proof is deliberately kept in test_dm2_v1_amiga_lzx_real_media:
 * original ZIP -> nested ZIP -> ADF -> LZX -> CD.DAT, entirely in RAM.  This
 * parser test must not depend on a manually extracted game file.
 */

#include "dm2_v1_amiga_cd_dat.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_null_safety(void) {
    DM2_V1_AmigaCdDat cd;
    assert(dm2_v1_amiga_cd_dat_parse(NULL, NULL, 0) == 0);
    assert(dm2_v1_amiga_cd_dat_parse(&cd, NULL, 0) == 0);
    assert(dm2_v1_amiga_cd_dat_mod_for_map(NULL, 0) == -1);
    printf("  PASS: null safety\n");
}

static void test_format_contract(void) {
    uint8_t data[DM2_AMIGA_CD_DAT_SIZE];
    DM2_V1_AmigaCdDat cd;
    unsigned int i;

    memset(data, 0xff, sizeof(data));
    for (i = 0u; i < DM2_AMIGA_CD_DAT_MAP_COUNT; ++i) {
        data[i * DM2_AMIGA_CD_DAT_ENTRY_SIZE + 2u] = (uint8_t)i;
        data[i * DM2_AMIGA_CD_DAT_ENTRY_SIZE + 3u] =
            (uint8_t)(i % DM2_AMIGA_MOD_TRACK_COUNT);
    }
    assert(dm2_v1_amiga_cd_dat_parse(&cd, data, sizeof(data)) == 1);
    assert(cd.valid == 1);
    for (i = 0u; i < DM2_AMIGA_CD_DAT_MAP_COUNT; ++i)
        assert(dm2_v1_amiga_cd_dat_mod_for_map(&cd, (int)i) ==
               (int)(i % DM2_AMIGA_MOD_TRACK_COUNT));
    assert(dm2_v1_amiga_cd_dat_mod_for_map(&cd, -1) == -1);
    assert(dm2_v1_amiga_cd_dat_mod_for_map(
               &cd, DM2_AMIGA_CD_DAT_MAP_COUNT) == -1);

    data[3] = DM2_AMIGA_MOD_TRACK_COUNT;
    assert(dm2_v1_amiga_cd_dat_parse(&cd, data, sizeof(data)) == 1);
    assert(dm2_v1_amiga_cd_dat_mod_for_map(&cd, 0) == -1);
    puts("  PASS: bounded 44-entry CD.DAT format contract");
}

int main(void) {
    printf("DM2 Amiga CD.DAT parser tests:\n");
    test_null_safety();
    test_format_contract();
    printf("\nAll Amiga CD.DAT tests passed.\n");
    return 0;
}
