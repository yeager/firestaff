/*
 * test_dm2_v1_boot_music_dispatch.c
 *
 * Validates the unified dm2_v1_boot_music_track_for_level dispatcher
 * loads and routes music through the correct system per platform.
 */

#include "dm2_v1_boot.h"

#include <assert.h>
#include <stdio.h>
#include <string.h>

static void test_null_profile(void) {
    int track = 99;
    assert(dm2_v1_boot_music_track_for_level(NULL, 0, 0, 0, &track) == 0);
    assert(track == -1);
    printf("  PASS: NULL profile returns 0\n");
}

static void test_unverified_profile(void) {
    DM2_V1_BootProfile profile;
    int track = 99;
    dm2_v1_boot_profile_init(&profile);
    assert(dm2_v1_boot_music_track_for_level(&profile, 0, 0, 0, &track) == 0);
    printf("  PASS: unverified profile returns 0\n");
}

static void test_songlist_dispatch(void) {
    DM2_V1_BootProfile profile;
    int track = -1;
    dm2_v1_boot_profile_init(&profile);
    profile.platform = DM2_PLATFORM_PC_EN;
    profile.songlist_verified = 1;
    profile.songlist_map[0] = 5;
    profile.songlist_map[3] = 12;
    profile.songlist_map[10] = 0xff;

    assert(dm2_v1_boot_music_track_for_level(&profile, 0, 0, 0, &track) == 1);
    assert(track == 5);

    assert(dm2_v1_boot_music_track_for_level(&profile, 3, 0, 0, &track) == 1);
    assert(track == 12);

    assert(dm2_v1_boot_music_track_for_level(&profile, 10, 0, 0, &track) == 0);

    printf("  PASS: PC songlist dispatch\n");
}

static void test_cdda_dispatch(void) {
    DM2_V1_BootProfile profile;
    int track = -1;
    dm2_v1_boot_profile_init(&profile);
    profile.platform = DM2_PLATFORM_FMTOWNS_JA;
    profile.cdda_cd_dat_verified = 1;
    /* Entry 0: x=5, y=3, level=2, track=7 */
    profile.cdda_cd_dat_data[0] = 5;
    profile.cdda_cd_dat_data[1] = 3;
    profile.cdda_cd_dat_data[2] = 2;
    profile.cdda_cd_dat_data[3] = 7;
    profile.cdda_cd_dat_size = 40;

    assert(dm2_v1_boot_music_track_for_level(&profile, 2, 5, 3, &track) == 1);
    assert(track == 7);

    assert(dm2_v1_boot_music_track_for_level(&profile, 2, 0, 0, &track) == 0);

    printf("  PASS: FM Towns CDDA dispatch\n");
}

static void test_music_system_routing(void) {
    DM2_V1_BootProfile profile;
    int track = -1;
    dm2_v1_boot_profile_init(&profile);

    profile.platform = DM2_PLATFORM_PC_FR;
    profile.songlist_verified = 1;
    profile.songlist_map[1] = 3;
    assert(dm2_v1_boot_music_track_for_level(&profile, 1, 0, 0, &track) == 1);
    assert(track == 3);

    profile.platform = DM2_PLATFORM_MEGACD_JA;
    assert(dm2_v1_boot_music_track_for_level(&profile, 1, 0, 0, &track) == 0);

    printf("  PASS: routing changes with platform\n");
}

int main(void) {
    printf("DM2 boot music dispatch tests:\n");
    test_null_profile();
    test_unverified_profile();
    test_songlist_dispatch();
    test_cdda_dispatch();
    test_music_system_routing();
    printf("\nAll boot music dispatch tests passed.\n");
    return 0;
}
