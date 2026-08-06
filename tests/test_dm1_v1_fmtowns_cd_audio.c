#include "dm1_v1_fmtowns_cd_audio.h"
#include "firestaff_fmtowns_disc.h"
#include <assert.h>
#include <stdio.h>
#include <string.h>
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

static void test_cue_parse_dm1_fmtowns(void) {
    static const char cue[] =
        "FILE \"Dungeon Master (1987)(FTL)(Jp-En).bin\" BINARY\n"
        "  TRACK 01 MODE1/2048\n"
        "    INDEX 01 00:00:00\n"
        "  TRACK 02 AUDIO\n"
        "    PREGAP 00:02:00\n"
        "    INDEX 01 00:28:50\n"
        "  TRACK 03 AUDIO\n"
        "    INDEX 00 02:26:50\n"
        "    INDEX 01 02:28:50\n"
        "  TRACK 05 AUDIO\n"
        "    INDEX 00 06:18:50\n"
        "    INDEX 01 06:20:50\n"
        "  TRACK 20 AUDIO\n"
        "    INDEX 00 29:46:52\n"
        "    INDEX 01 29:48:52\n";
    uint32_t starts[24];
    int count;
    memset(starts, 0, sizeof(starts));
    count = fmtowns_cue_parse_track_starts(cue, strlen(cue), starts, 24);
    assert(count >= 20);
    assert(starts[1] == 0);
    assert(starts[2] == 28u * 75u + 50u);
    assert(starts[3] == 2u * 60u * 75u + 28u * 75u + 50u);
    assert(starts[5] == 6u * 60u * 75u + 20u * 75u + 50u);
    assert(starts[20] == 29u * 60u * 75u + 48u * 75u + 52u);
}

static void test_cdda_byte_offset_calculation(void) {
    uint32_t data_track_end = 28u * 75u + 50u;
    uint32_t track5_sector = 6u * 60u * 75u + 20u * 75u + 50u;
    size_t data_bytes = (size_t)data_track_end * 2048u;
    size_t audio_offset = data_bytes +
        (size_t)(track5_sector - data_track_end) * FMTOWNS_CDDA_SECTOR_SIZE;
    assert(data_bytes == 4403200u);
    assert(audio_offset == 66496000u);
}

int main(void) {
    test_track_for_map();
    test_track_for_event();
    test_track_info();
    test_all_tracks_covered();
    test_cue_parse_dm1_fmtowns();
    test_cdda_byte_offset_calculation();
    printf("All dm1_v1_fmtowns_cd_audio tests passed.\n");
    return 0;
}
