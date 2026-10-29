/*
 * test_csb_v1_amiga_dungeon_probe.c — verify original CSB Dungeon.DAT media
 * through the FTL decompressor and big-endian byte-swap path. Pass --atari
 * to use the Atari ST archive member and FIRESTAFF_CSB_ATARI_DUNGEON.
 *
 * Default mode requires FIRESTAFF_CSB_AMIGA_DUNGEON; --atari requires
 * FIRESTAFF_CSB_ATARI_DUNGEON. Nested archive members are read directly in
 * RAM and are never materialized beside the user's game data.
 */

#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_compressed_header(const char *path, const char *media_name)
{
    char md5[33];
    size_t size = 0u;
    uint8_t *dat = NULL;

    if (!asset_read_virtual_path_alloc(path, &dat, &size)) dat = NULL;
    if (!dat) {
        fprintf(stderr, "FAIL: cannot read configured %s Dungeon.DAT: %s\n",
                media_name, path);
        assert(dat != NULL);
    }

    /* ReDMCSB Atari ST and A31/A35 original game media use this same CSB
     * dungeon payload. Retain the digest gate so an arbitrary FTL-shaped
     * fixture cannot become evidence for the native startup route. */
    assert(asset_file_md5_hex(path, md5));
    assert(strcmp(md5, "6695d2acebce49f95db1d8f3a5c733de") == 0);

    assert(size >= 8);
    assert(dat[0] == 0x81);
    assert(dat[1] == 0x04);

    uint32_t decomp_size = ((uint32_t)dat[2] << 24) | ((uint32_t)dat[3] << 16) |
                           ((uint32_t)dat[4] << 8) | (uint32_t)dat[5];
    printf("  compressed: %zu bytes, decompressed: %u bytes\n",
           size, decomp_size);
    assert(decomp_size > 0 && decomp_size < 65536);

    printf("  PASS: FTL header 0x8104, decomp=%u\n", decomp_size);
    free(dat);
}

static void test_ftl_decompress_and_load(const char *path,
                                        const char *media_name)
{
    CSB_V1_DungeonData dungeon;

    memset(&dungeon, 0, sizeof(dungeon));
    int rc = csb_v1_dungeon_load_from_file(&dungeon, path);
    if (rc != 0) {
        fprintf(stderr, "FAIL: configured %s Dungeon.DAT is not loadable: %s\n",
                media_name, path);
        assert(rc == 0);
    }

    printf("  levels: %d\n", dungeon.level_count);
    /* v3.3 French (Meynaf hack) has only 2 levels; original has 12.
     * Accept any valid count > 0. */
    assert(dungeon.level_count > 0 && dungeon.level_count <= 12);

    int map_index = -1, x = -1, y = -1, direction = -1;
    int pose_ok = csb_v1_dungeon_initial_party_pose_pc34(
        &dungeon, &map_index, &x, &y, &direction);
    assert(pose_ok == 1);
    assert(map_index >= 0 && map_index < dungeon.level_count);
    assert(x >= 0 && y >= 0);
    assert(direction >= 0 && direction <= 3);
    printf("  party: level=%d x=%d y=%d dir=%d\n", map_index, x, y, direction);

    /* ReDMCSB DUNGEON.C F0173 stores G0261 wall ornament selectors after
     * each map's square and creature-type bytes. Validate the accessor
     * against the authenticated expanded source body. */
    {
        int verified_wall_ornaments = 0;
        uint8_t ignored_global_index = 0u;
        for (int i = 0; i < dungeon.level_count; ++i) {
            const size_t square_count =
                (size_t)dungeon.level_widths[i] *
                (size_t)dungeon.level_heights[i];
            const size_t wall_table_offset = (size_t)dungeon.level_offsets[i] +
                square_count + (size_t)dungeon.map_creature_type_count[i];
            assert(dungeon.map_wall_ornament_count[i] >= 0 &&
                   dungeon.map_wall_ornament_count[i] <= 16);
            for (int j = 0; j < dungeon.map_wall_ornament_count[i]; ++j) {
                uint8_t global_index = 0u;
                assert(wall_table_offset + (size_t)j <
                       (size_t)dungeon.raw_size);
                assert(csb_v1_dungeon_get_wall_ornament_index(
                    &dungeon, i, j, &global_index));
                assert(global_index ==
                       dungeon.raw_data[wall_table_offset + (size_t)j]);
                ++verified_wall_ornaments;
            }
            assert(!csb_v1_dungeon_get_wall_ornament_index(
                &dungeon, i, dungeon.map_wall_ornament_count[i],
                &ignored_global_index));
        }
        assert(!csb_v1_dungeon_get_wall_ornament_index(
            &dungeon, dungeon.level_count, 0, &ignored_global_index));
        assert(!csb_v1_dungeon_get_wall_ornament_index(
            &dungeon, 0, -1, &ignored_global_index));
        assert(!csb_v1_dungeon_get_wall_ornament_index(
            &dungeon, 0, 0, NULL));
        assert(verified_wall_ornaments > 0);
        printf("  wall ornament selectors: %d original entries verified\n",
               verified_wall_ornaments);
    }

    for (int i = 0; i < dungeon.level_count; i++) {
        assert(dungeon.level_widths[i] >= 1 && dungeon.level_widths[i] <= 32);
        assert(dungeon.level_heights[i] >= 1 && dungeon.level_heights[i] <= 32);
        printf("  level %2d: %dx%d\n", i, dungeon.level_widths[i], dungeon.level_heights[i]);
    }

    printf("  PASS: FTL decompress + big-endian swap => %d levels\n",
           dungeon.level_count);
    csb_v1_dungeon_free(&dungeon);
}

int main(int argc, char **argv)
{
    const int atari = argc > 1 && strcmp(argv[1], "--atari") == 0;
    const char *media_name = atari ? "Atari" : "Amiga";
    const char *path = getenv(atari ? "FIRESTAFF_CSB_ATARI_DUNGEON" :
                                      "FIRESTAFF_CSB_AMIGA_DUNGEON");

    if (!path || !path[0]) {
        printf("skip: FIRESTAFF_CSB_%s_DUNGEON is not set\n",
               atari ? "ATARI" : "AMIGA");
        return 77;
    }
    printf("test_csb_v1_%s_dungeon_probe:\n", atari ? "atari" : "amiga");
    test_compressed_header(path, media_name);
    test_ftl_decompress_and_load(path, media_name);
    printf("ok: CSB %s dungeon probe\n", media_name);
    return 0;
}
