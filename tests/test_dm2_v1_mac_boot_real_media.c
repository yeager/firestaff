#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int demo = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP") != NULL;
    DM2_V1_BootProfile profile;
    const DM2_V1_DungeonData *dungeon;

    if (demo) zip = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP");
    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 0;
    }
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, zip) != 0 ||
        !profile.assets_verified || profile.platform != DM2_PLATFORM_MAC_EN ||
        strcmp(profile.version_id, demo ? "mac-en-demo" : "mac-en") != 0 ||
        profile.graphics_mem_size != (demo ? 3110116u : 8157169u) ||
        profile.dungeon_mem_size != (demo ? 6535u : 39411u) ||
        (!demo && (!profile.music_map_verified ||
                   profile.music_map_size != 176u)) ||
        (!demo && (profile.mac_movie_present_mask != 0x1du ||
                   profile.mac_movie_resource_present_mask != 0x1du ||
                   profile.mac_movie_moov_present_mask != 0x1du ||
                   profile.mac_movie_moov_size[DM2_V1_MAC_MOVIE_TITLE] !=
                       3286u)) ||
        (!demo && profile.mac_sound_resource_fork_present_mask != 0x7u) ||
        dm2_v1_boot_enter_game(&profile) != 0) {
        fprintf(stderr,
                "DM2 Mac boot failed: platform=%d version=%s verified=%d g=%zu d=%zu\n",
                profile.platform, profile.version_id, profile.assets_verified,
                profile.graphics_mem_size, profile.dungeon_mem_size);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    dungeon = (const DM2_V1_DungeonData *)profile.dungeon_data;
    if (!dungeon || !dungeon->record_graph_complete ||
        !dungeon->source_words_big_endian || !dungeon->initial_party_pose_valid) {
        fprintf(stderr, "DM2 Mac dungeon graph/endian gate failed: graph=%d source_be=%d pose=%d\n",
                dungeon ? dungeon->record_graph_complete : 0,
                dungeon ? dungeon->source_words_big_endian : 0,
                dungeon ? dungeon->initial_party_pose_valid : 0);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    if (!demo) {
        /* Maps 0-5 cover the source-verified DB0..DB4/DB3 continuation.
         * Later Mac-only DB10/DB14 roots remain deliberately fail-closed
         * until their original pool layout is proven. */
        for (int map = 0; map <= 5 && map < dungeon->level_count; ++map) {
            DM2_V1_FileHeaderRuntimeMapReceipt receipt;
            memset(&receipt, 0, sizeof(receipt));
            if (!dm2_v1_dungeon_validate_file_header_runtime_map(
                    dungeon, map, &receipt) || !receipt.committed ||
                receipt.root_count <= 0 || receipt.record_count < receipt.root_count) {
                fprintf(stderr,
                        "DM2 Mac retail map %d File_header gate failed: roots=%d records=%d\n",
                        map, receipt.root_count, receipt.record_count);
                dm2_v1_boot_cleanup(&profile);
                return 1;
            }
        }
    }
    dm2_v1_boot_cleanup(&profile);
    puts(demo ? "PASS: DM2 Macintosh demo boots from the original ZIP in RAM"
              : "PASS: DM2 Macintosh retail boots from the original ZIP in RAM");
    return 0;
}
