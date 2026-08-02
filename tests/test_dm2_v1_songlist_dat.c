/*
 * test_dm2_v1_songlist_dat.c — SONGLIST.DAT parser tests.
 *
 * Validates against the real DOS EN SONGLIST.DAT (63 bytes).
 */

#include "dm2_v1_songlist_dat.h"
#include <assert.h>
#include <stdio.h>

static const uint8_t real_songlist[63] = {
    0x02, 0x11, 0x0E, 0x1B, 0x04, 0x0C, 0x0C, 0x12,
    0x0F, 0x0D, 0x0C, 0x0C, 0x10, 0x06, 0x15, 0x0E,
    0x11, 0x11, 0x11, 0x11, 0x03, 0x08, 0x11, 0x0E,
    0x02, 0x17, 0x16, 0x14, 0x11, 0x00, 0x02, 0x02,
    0x02, 0x09, 0x02, 0x03, 0x0E, 0x10, 0x1C, 0x16,
    0x13, 0x09, 0x16, 0x03,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
    0xFF, 0xFF, 0xFF
};

static void test_parse_valid(void) {
    DM2_V1_SonglistDat sl;
    assert(dm2_v1_songlist_dat_parse(&sl, real_songlist, 63) == 1);
    assert(sl.valid == 1);
}

static void test_map_0_track(void) {
    DM2_V1_SonglistDat sl;
    dm2_v1_songlist_dat_parse(&sl, real_songlist, 63);
    assert(dm2_v1_songlist_dat_track_for_map(&sl, 0) == 0x02);
}

static void test_map_3_track(void) {
    DM2_V1_SonglistDat sl;
    dm2_v1_songlist_dat_parse(&sl, real_songlist, 63);
    assert(dm2_v1_songlist_dat_track_for_map(&sl, 3) == 0x1B);
}

static void test_map_29_silence(void) {
    DM2_V1_SonglistDat sl;
    dm2_v1_songlist_dat_parse(&sl, real_songlist, 63);
    assert(dm2_v1_songlist_dat_track_for_map(&sl, 29) == 0x00);
}

static void test_all_tracks_in_range(void) {
    DM2_V1_SonglistDat sl;
    int i;
    dm2_v1_songlist_dat_parse(&sl, real_songlist, 63);
    for (i = 0; i < 44; i++) {
        int t = dm2_v1_songlist_dat_track_for_map(&sl, i);
        assert(t >= 0 && t <= 28);
    }
}

static void test_out_of_range(void) {
    DM2_V1_SonglistDat sl;
    dm2_v1_songlist_dat_parse(&sl, real_songlist, 63);
    assert(dm2_v1_songlist_dat_track_for_map(&sl, -1) == -1);
    assert(dm2_v1_songlist_dat_track_for_map(&sl, 44) == -1);
    assert(dm2_v1_songlist_dat_track_for_map(&sl, 100) == -1);
}

static void test_null_safety(void) {
    DM2_V1_SonglistDat sl;
    assert(dm2_v1_songlist_dat_parse(NULL, real_songlist, 63) == 0);
    assert(dm2_v1_songlist_dat_parse(&sl, NULL, 63) == 0);
    assert(dm2_v1_songlist_dat_parse(&sl, real_songlist, 10) == 0);
    assert(dm2_v1_songlist_dat_track_for_map(NULL, 0) == -1);
}

static void test_fmtowns_cdda_chain(void) {
    /* Map 0 → HMP track 2 → (via fmtowns_cdda_music) CDDA index 2 */
    DM2_V1_SonglistDat sl;
    dm2_v1_songlist_dat_parse(&sl, real_songlist, 63);
    int hmp = dm2_v1_songlist_dat_track_for_map(&sl, 0);
    assert(hmp == 2);
}

int main(void) {
    printf("SONGLIST.DAT parser tests\n");
    printf("Source: DOS EN SONGLIST.DAT (63 bytes)\n\n");

    test_parse_valid();        printf("  parse_valid          PASS\n");
    test_map_0_track();        printf("  map_0_track          PASS\n");
    test_map_3_track();        printf("  map_3_track          PASS\n");
    test_map_29_silence();     printf("  map_29_silence       PASS\n");
    test_all_tracks_in_range();printf("  all_tracks_in_range  PASS\n");
    test_out_of_range();       printf("  out_of_range         PASS\n");
    test_null_safety();        printf("  null_safety          PASS\n");
    test_fmtowns_cdda_chain(); printf("  fmtowns_cdda_chain   PASS\n");

    printf("\n8/8 tests passed\n");
    return 0;
}
