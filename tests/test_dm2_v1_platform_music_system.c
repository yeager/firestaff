/*
 * test_dm2_v1_platform_music_system.c
 *
 * Validates the DM2 platform → music system classification.
 */

#include "dm2_v1_boot.h"

#include <assert.h>
#include <stdio.h>

int main(void) {
    printf("DM2 platform music system classification tests:\n");

    assert(dm2_v1_platform_music_system(DM2_PLATFORM_PC_EN)
           == DM2_MUSIC_SYSTEM_HMP_SONGLIST);
    assert(dm2_v1_platform_music_system(DM2_PLATFORM_PC_FR)
           == DM2_MUSIC_SYSTEM_HMP_SONGLIST);
    assert(dm2_v1_platform_music_system(DM2_PLATFORM_PC_JEWEL)
           == DM2_MUSIC_SYSTEM_HMP_SONGLIST);
    printf("  PASS: PC platforms -> HMP_SONGLIST\n");

    assert(dm2_v1_platform_music_system(DM2_PLATFORM_MAC_EN)
           == DM2_MUSIC_SYSTEM_HMP_MAP176);
    assert(dm2_v1_platform_music_system(DM2_PLATFORM_MAC_FR)
           == DM2_MUSIC_SYSTEM_HMP_MAP176);
    printf("  PASS: Mac platforms -> HMP_MAP176\n");

    assert(dm2_v1_platform_music_system(DM2_PLATFORM_AMIGA_EN)
           == DM2_MUSIC_SYSTEM_MOD_MAP176);
    printf("  PASS: Amiga -> MOD_MAP176\n");

    assert(dm2_v1_platform_music_system(DM2_PLATFORM_FMTOWNS_JA)
           == DM2_MUSIC_SYSTEM_CDDA_COORD);
    assert(dm2_v1_platform_music_system(DM2_PLATFORM_MEGACD_JA)
           == DM2_MUSIC_SYSTEM_CDDA_COORD);
    assert(dm2_v1_platform_music_system(DM2_PLATFORM_PC9821_JA)
           == DM2_MUSIC_SYSTEM_CDDA_COORD);
    printf("  PASS: FM Towns/Mega CD/PC-9821 -> CDDA_COORD\n");

    printf("  Platform count: %d\n", DM2_PLATFORM_COUNT);
    assert(DM2_PLATFORM_COUNT == 9);
    printf("  PASS: 9 platforms defined\n");

    printf("\nAll platform music system tests passed.\n");
    return 0;
}
