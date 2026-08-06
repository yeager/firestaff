/* Opt-in boot regression for DM2's original Amiga AGA installer.
 *
 * The archive is read through its nested ZIP/ADF/LZX transport in memory.
 * No game member is extracted, copied or materialized on disk. */

#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message)
{
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

int main(void)
{
    const char *root = getenv("FIRESTAFF_DM2_AMIGA_ROOT");
    DM2_V1_BootProfile profile;

    if (!root || root[0] == '\0') {
        puts("SKIP: FIRESTAFF_DM2_AMIGA_ROOT is not set");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    expect(dm2_v1_boot_scan_assets(&profile, root) == 0,
           "the original Amiga installer provides a complete DM2 hash pair");
    expect(profile.assets_verified &&
               profile.platform == DM2_PLATFORM_AMIGA_EN &&
               strcmp(profile.version_id, "amiga-en") == 0,
           "boot admits only the verified Amiga release identity");
    expect(profile.graphics_mem && profile.graphics_mem_size == 3493879u &&
               profile.dungeon_mem && profile.dungeon_mem_size == 39411u,
           "boot retains authenticated GRAPHICS.DAT and DUNGEON.DAT in RAM");
    expect(profile.music_map_verified && profile.music_map_size == 176u,
           "boot admits the original Amiga CD.DAT map in RAM");
    expect(strstr(profile.graphics_path, "::DM2_archive.LZX/GRAPHICS.DAT") != NULL &&
               strstr(profile.dungeon_path, "::DM2_archive.LZX/DUNGEON.DAT") != NULL,
           "boot records nested media provenance instead of a cache path");
    expect(dm2_v1_boot_enter_game(&profile) == 0,
           "the admitted original Amiga buffers complete DM2 boot");
    dm2_v1_boot_cleanup(&profile);
    if (failures != 0) {
        return 1;
    }
    puts("PASS: DM2 Amiga boot remains memory-owned from original installer media");
    return 0;
}
