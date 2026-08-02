#include "dm1_v1_fmtowns_cd_audio.h"
#include <assert.h>
#include <stdio.h>
#include <stddef.h>

static void test_track_for_map(void) {
    assert(dm1_v1_fmtowns_cd_track_for_map(0) == 6);
    assert(dm1_v1_fmtowns_cd_track_for_map(1) == 6);
    assert(dm1_v1_fmtowns_cd_track_for_map(2) == 8);
    assert(dm1_v1_fmtowns_cd_track_for_map(5) == 5);
    assert(dm1_v1_fmtowns_cd_track_for_map(6) == 10);
    assert(dm1_v1_fmtowns_cd_track_for_map(9) == 12);
    assert(dm1_v1_fmtowns_cd_track_for_map(12) == 16);
    assert(dm1_v1_fmtowns_cd_track_for_map(-1) == 0);
    assert(dm1_v1_fmtowns_cd_track_for_map(16) == 0);
}

static void test_track_for_event(void) {
    assert(dm1_v1_fmtowns_cd_track_for_event(0) == DM1_FMTOWNS_TRACK_TITLE);
    assert(dm1_v1_fmtowns_cd_track_for_event(1) == DM1_FMTOWNS_TRACK_HALL);
    assert(dm1_v1_fmtowns_cd_track_for_event(2) == DM1_FMTOWNS_TRACK_GAME_OVER);
    assert(dm1_v1_fmtowns_cd_track_for_event(3) == DM1_FMTOWNS_TRACK_GAME_WON);
    assert(dm1_v1_fmtowns_cd_track_for_event(-1) == 0);
    assert(dm1_v1_fmtowns_cd_track_for_event(4) == 0);
}

static void test_track_info(void) {
    const DM1_V1_FmtownsCdTrackInfo *info;

    info = dm1_v1_fmtowns_cd_track_info(2);
    assert(info != NULL);
    assert(info->track_number == 2);
    assert(info->is_unused == 0);

    info = dm1_v1_fmtowns_cd_track_info(4);
    assert(info != NULL);
    assert(info->is_unused == 1);

    info = dm1_v1_fmtowns_cd_track_info(13);
    assert(info != NULL);
    assert(info->track_number == 13);

    info = dm1_v1_fmtowns_cd_track_info(18);
    assert(info != NULL);
    assert(info->track_number == 18);

    info = dm1_v1_fmtowns_cd_track_info(20);
    assert(info != NULL);
    assert(info->is_unused == 1);

    assert(dm1_v1_fmtowns_cd_track_info(1) == NULL);
    assert(dm1_v1_fmtowns_cd_track_info(21) == NULL);
}

static void test_all_tracks_covered(void) {
    for (int t = DM1_FMTOWNS_CD_FIRST_AUDIO_TRACK;
         t <= DM1_FMTOWNS_CD_LAST_AUDIO_TRACK; t++) {
        const DM1_V1_FmtownsCdTrackInfo *info = dm1_v1_fmtowns_cd_track_info(t);
        assert(info != NULL);
        assert(info->track_number == t);
        assert(info->description != NULL);
    }
}

int main(void) {
    test_track_for_map();
    test_track_for_event();
    test_track_info();
    test_all_tracks_covered();
    printf("All dm1_v1_fmtowns_cd_audio tests passed.\n");
    return 0;
}
