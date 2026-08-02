/*
 * test_dm2_v1_fmtowns_cdda_music.c — FM Towns HMP-to-CDDA mapping tests.
 *
 * Validates the 29-entry mapping table extracted from SKULL.EXP
 * against known properties of the FM Towns DM2 disc layout.
 */

#include "dm2_v1_fmtowns_cdda_music.h"
#include <assert.h>
#include <stdio.h>

static void test_silence_tracks(void) {
    assert(dm2_v1_fmtowns_hmp_to_cdda(0) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(1) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(2) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(3) == 0);
}

static void test_first_music_block(void) {
    assert(dm2_v1_fmtowns_hmp_to_cdda(4) == 1);
    assert(dm2_v1_fmtowns_hmp_to_cdda(5) == 2);
    assert(dm2_v1_fmtowns_hmp_to_cdda(6) == 3);
    assert(dm2_v1_fmtowns_hmp_to_cdda(7) == 4);
}

static void test_all_nine_cdda_used(void) {
    int used[10] = {0};
    const uint8_t *table = dm2_v1_fmtowns_cdda_map_table();
    for (int i = 0; i < DM2_FMTOWNS_HMP_TRACK_COUNT; i++) {
        if (table[i] > 0 && table[i] <= 9)
            used[table[i]] = 1;
    }
    for (int t = 1; t <= 9; t++)
        assert(used[t] == 1);
}

static void test_out_of_range(void) {
    assert(dm2_v1_fmtowns_hmp_to_cdda(-1) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(29) == 0);
    assert(dm2_v1_fmtowns_hmp_to_cdda(100) == 0);
}

static void test_table_not_null(void) {
    assert(dm2_v1_fmtowns_cdda_map_table() != NULL);
}

int main(void) {
    printf("FM Towns CDDA music mapping tests\n");
    printf("Source: SKULL.EXP offset 0x3dac (HME-242)\n\n");

    test_silence_tracks();     printf("  silence_tracks       PASS\n");
    test_first_music_block();  printf("  first_music_block    PASS\n");
    test_all_nine_cdda_used(); printf("  all_nine_cdda_used   PASS\n");
    test_out_of_range();       printf("  out_of_range         PASS\n");
    test_table_not_null();     printf("  table_not_null       PASS\n");

    printf("\n5/5 tests passed\n");
    return 0;
}
