#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void) {
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    const char *demo_zip = getenv("FIRESTAFF_DM2_MAC_DEMO_ARCHIVE");
    DM2_V1_BootProfile profile;
    const DM2_V1_DungeonData *dungeon;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 77;
    }
    /* The workspace contains both the full retail disc and the unrelated
     * First Chapter demo. Scan the demo while its sibling retail media is
     * still visible: a failed explicit selection must not silently become a
     * verified FM Towns (or any other sibling) boot profile. */
    if (demo_zip && demo_zip[0]) {
        int demo_rc;
        dm2_v1_boot_profile_init(&profile);
        demo_rc = dm2_v1_boot_scan_assets(&profile, demo_zip);
        if (demo_rc == 0 || profile.assets_verified) {
            fprintf(stderr,
                    "DM2 Mac demo selection fell through to another edition: "
                    "rc=%d platform=%d version=%s graphics=%s\n",
                    demo_rc, profile.platform, profile.version_id,
                    profile.graphics_path);
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
        dm2_v1_boot_cleanup(&profile);
    }
    dm2_v1_boot_profile_init(&profile);
    if (dm2_v1_boot_scan_assets(&profile, zip) != 0 ||
        !profile.assets_verified || profile.platform != DM2_PLATFORM_MAC_EN ||
        strcmp(profile.version_id, "mac-en-retail") != 0 ||
        profile.graphics_mem_size != 8157169u ||
        profile.dungeon_mem_size != 39411u ||
        !profile.music_map_verified || profile.music_map_size != 176u ||
        (profile.mac_movie_present_mask != 0x1du ||
                   profile.mac_movie_resource_present_mask != 0x1du ||
                   profile.mac_movie_moov_present_mask != 0x1du ||
                   profile.mac_movie_moov_size[DM2_V1_MAC_MOVIE_TITLE] !=
                       3286u) ||
        profile.mac_sound_resource_fork_present_mask != 0x7u ||
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
    {
        if (!profile.mac_application_data ||
            profile.mac_application_data_size != 484944u ||
            !profile.mac_application_resource ||
            profile.mac_application_resource_size != 5046234u) {
            fprintf(stderr, "DM2 Mac application forks were not retained\n");
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
        /* The canonical Mac File_header uses the same 44-map layout as the
         * retail PC family.  Validate every authentic map, not just the
         * entrance maps, so a shifted header cannot hide later roots. */
        for (int map = 0; map < dungeon->level_count; ++map) {
            DM2_V1_FileHeaderRuntimeMapReceipt receipt;
            memset(&receipt, 0, sizeof(receipt));
            if (!dm2_v1_dungeon_validate_file_header_runtime_map(
                    dungeon, map, &receipt) || !receipt.committed ||
                receipt.root_count < 0 || receipt.record_count < receipt.root_count) {
                fprintf(stderr,
                        "DM2 Mac retail map %d File_header gate failed: roots=%d records=%d\n",
                        map, receipt.root_count, receipt.record_count);
                dm2_v1_boot_cleanup(&profile);
                return 1;
            }
        }
    }
    dm2_v1_boot_cleanup(&profile);
    puts("PASS: DM2 Macintosh retail boots from the original ZIP in RAM");
    return 0;
}
