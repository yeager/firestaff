/*
 * test_csb_v1_amiga_dungeon_probe.c — verify CSB Amiga Dungeon.DAT
 * loads through the FTL decompressor and big-endian byte-swap path.
 *
 * Requires FIRESTAFF_CSB_AMIGA_DUNGEON to name the hash-verified DUNGEON.DAT
 * member selected from an A31/A35 package.  Nested ZIP→ADF members are read
 * directly in RAM and are never materialized beside the user's game data.
 */

#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void test_compressed_header(const char *path)
{
    char md5[33];
    size_t size = 0u;
    uint8_t *dat = NULL;

    if (!asset_read_virtual_path_alloc(path, &dat, &size)) dat = NULL;
    if (!dat) {
        fprintf(stderr, "FAIL: cannot read configured Amiga Dungeon.DAT: %s\n",
                path);
        assert(dat != NULL);
    }

    /* ReDMCSB COMPILE.H's A31/A35 media use this original CSB dungeon
     * payload.  Retain the digest gate so an arbitrary FTL-shaped fixture
     * cannot become evidence for the native startup route. */
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

static void test_ftl_decompress_and_load(const char *path)
{
    CSB_V1_DungeonData dungeon;

    memset(&dungeon, 0, sizeof(dungeon));
    int rc = csb_v1_dungeon_load_from_file(&dungeon, path);
    if (rc != 0) {
        fprintf(stderr, "FAIL: configured Amiga Dungeon.DAT is not loadable: %s\n",
                path);
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

    for (int i = 0; i < dungeon.level_count; i++) {
        assert(dungeon.level_widths[i] >= 1 && dungeon.level_widths[i] <= 32);
        assert(dungeon.level_heights[i] >= 1 && dungeon.level_heights[i] <= 32);
        printf("  level %2d: %dx%d\n", i, dungeon.level_widths[i], dungeon.level_heights[i]);
    }

    printf("  PASS: FTL decompress + big-endian swap => %d levels\n",
           dungeon.level_count);
    csb_v1_dungeon_free(&dungeon);
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_AMIGA_DUNGEON");

    if (!path || !path[0]) {
        puts("skip: FIRESTAFF_CSB_AMIGA_DUNGEON is not set");
        return 77;
    }
    puts("test_csb_v1_amiga_dungeon_probe:");
    test_compressed_header(path);
    test_ftl_decompress_and_load(path);
    puts("ok: CSB Amiga dungeon probe");
    return 0;
}
