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
        !dungeon->source_words_big_endian ||
        dungeon->g1_w0_chains_disabled || !dungeon->initial_party_pose_valid) {
        fprintf(stderr, "DM2 Mac dungeon graph/endian gate failed: graph=%d source_be=%d pose=%d\n",
                dungeon ? dungeon->record_graph_complete : 0,
                dungeon ? dungeon->source_words_big_endian : 0,
                dungeon ? dungeon->initial_party_pose_valid : 0);
        dm2_v1_boot_cleanup(&profile);
        return 1;
    }
    {
        int total_columns = 0;
        int ordinal = 0;
        int checked = 0;
        for (int map = 0; map < dungeon->level_count; ++map)
            total_columns += dungeon->level_widths[map];
        if (dungeon->raw_map_data_base != 26806 ||
            dungeon->g1_extension_size != 0 || total_columns != 725) {
            fprintf(stderr, "Mac map layout mismatch: base=%d extension=%d columns=%d\n",
                    dungeon->raw_map_data_base, dungeon->g1_extension_size,
                    total_columns);
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
        /* File_header column prefixes count tiles with a record root.
         * The last column has no following prefix; all other 724 can be
         * checked against the independently owned source map bytes. */
        for (int map = 0; map < dungeon->level_count; ++map) {
            for (int x = 0; x < dungeon->level_widths[map]; ++x, ++ordinal) {
                int roots = 0;
                int base = dungeon->column_index_base + ordinal * 2;
                int next = base + 2;
                int prefix = (dungeon->raw_data[base] << 8) |
                             dungeon->raw_data[base + 1];
                int next_prefix;
                if (ordinal + 1 == total_columns) continue;
                next_prefix = (dungeon->raw_data[next] << 8) |
                              dungeon->raw_data[next + 1];
                for (int y = 0; y < dungeon->level_heights[map]; ++y) {
                    int tile = dm2_v1_dungeon_get_tile_raw(dungeon, map, x, y);
                    if (tile < 0) {
                        dm2_v1_boot_cleanup(&profile);
                        return 1;
                    }
                    roots += (tile & 0x10) != 0;
                }
                if (next_prefix - prefix != roots) {
                    fprintf(stderr, "Mac map column %d root mismatch: prefix=%d tile=%d\n",
                            ordinal, next_prefix - prefix, roots);
                    dm2_v1_boot_cleanup(&profile);
                    return 1;
                }
                ++checked;
            }
        }
        if (checked != 724) {
            dm2_v1_boot_cleanup(&profile);
            return 1;
        }
    }
    {
        /* Original retail Dungeon.dat, SKWIN/SkWinCore.cpp:2718-2730:
         * GET_NEXT_RECORD_LINK reads GenericRecord::w0 in source order. */
        static const struct {
            int map, x, y;
            uint16_t links[6];
            int count;
        } chains[] = {
            {10, 4, 9, {0xe80f, 0x2810, 0x6811, 0xa812, 0xe813, 0xfffe}, 6},
            {15, 10, 6, {0x2848, 0x6849, 0xfffe}, 3},
        };
        for (size_t c = 0; c < sizeof(chains) / sizeof(chains[0]); ++c) {
            int thing = dm2_v1_dungeon_get_first_thing(
                dungeon, chains[c].map, chains[c].x, chains[c].y);
            for (int i = 0; i < chains[c].count; ++i) {
                if (thing != (int)chains[c].links[i]) {
                    fprintf(stderr,
                            "Mac DB10 chain mismatch map=%d x=%d y=%d step=%d got=0x%04x expected=0x%04x\n",
                            chains[c].map, chains[c].x, chains[c].y, i,
                            (unsigned int)(thing & 0xffff), chains[c].links[i]);
                    dm2_v1_boot_cleanup(&profile);
                    return 1;
                }
                if (i + 1 < chains[c].count)
                    thing = dm2_v1_dungeon_get_next_thing(dungeon, (uint16_t)thing);
            }
        }
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
                receipt.root_count < 0 || receipt.record_count < receipt.root_count ||
                ((map == 10 || map == 15) &&
                 receipt.record_count <= receipt.root_count)) {
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
