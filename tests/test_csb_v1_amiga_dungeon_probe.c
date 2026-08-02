/*
 * test_csb_v1_amiga_dungeon_probe.c — verify CSB Amiga Dungeon.DAT
 * loads through the FTL decompressor and big-endian byte-swap path.
 *
 * Requires: ~/.firestaff/data/csb-amiga/Dungeon.DAT
 *           (CSB Amiga v3.3 French, MD5 6695d2acebce49f95db1d8f3a5c733de)
 */

#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static const char *amiga_dungeon_path(void)
{
    static char path[1024];
    const char *home = getenv("HOME");
    if (!home) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/csb-amiga/Dungeon.DAT", home);
    return path;
}

static uint8_t *read_file(const char *path, long *out_size)
{
    FILE *f = fopen(path, "rb");
    uint8_t *buf;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    *out_size = ftell(f);
    fseek(f, 0, SEEK_SET);
    buf = (uint8_t *)malloc((size_t)*out_size);
    if (!buf) { fclose(f); return NULL; }
    if ((long)fread(buf, 1, (size_t)*out_size, f) != *out_size) {
        free(buf);
        fclose(f);
        return NULL;
    }
    fclose(f);
    return buf;
}

static void test_compressed_header(void)
{
    const char *path = amiga_dungeon_path();
    long size = 0;
    uint8_t *dat;

    if (!path) { puts("  SKIP: no HOME"); return; }
    dat = read_file(path, &size);
    if (!dat) { puts("  SKIP: Amiga Dungeon.DAT not found"); return; }

    assert(size >= 8);
    assert(dat[0] == 0x81);
    assert(dat[1] == 0x04);

    uint32_t decomp_size = ((uint32_t)dat[2] << 24) | ((uint32_t)dat[3] << 16) |
                           ((uint32_t)dat[4] << 8) | (uint32_t)dat[5];
    printf("  compressed: %ld bytes, decompressed: %u bytes\n",
           size, decomp_size);
    assert(decomp_size > 0 && decomp_size < 65536);

    printf("  PASS: FTL header 0x8104, decomp=%u\n", decomp_size);
    free(dat);
}

static void test_ftl_decompress_and_load(void)
{
    const char *path = amiga_dungeon_path();
    CSB_V1_DungeonData dungeon;

    if (!path) { puts("  SKIP: no HOME"); return; }

    memset(&dungeon, 0, sizeof(dungeon));
    int rc = csb_v1_dungeon_load_from_file(&dungeon, path);
    if (rc != 0) { puts("  SKIP: Amiga Dungeon.DAT not loadable"); return; }

    printf("  levels: %d\n", dungeon.level_count);
    assert(dungeon.level_count == 12);

    int map_index = -1, x = -1, y = -1, direction = -1;
    int pose_ok = csb_v1_dungeon_initial_party_pose_pc34(
        &dungeon, &map_index, &x, &y, &direction);
    assert(pose_ok == 1);
    assert(map_index >= 0 && map_index < 12);
    assert(x >= 0 && y >= 0);
    assert(direction >= 0 && direction <= 3);
    printf("  party: level=%d x=%d y=%d dir=%d\n", map_index, x, y, direction);

    for (int i = 0; i < dungeon.level_count; i++) {
        assert(dungeon.map_width[i] >= 1 && dungeon.map_width[i] <= 32);
        assert(dungeon.map_height[i] >= 1 && dungeon.map_height[i] <= 32);
        printf("  level %2d: %dx%d\n", i, dungeon.map_width[i], dungeon.map_height[i]);
    }

    printf("  PASS: FTL decompress + big-endian swap => %d levels\n",
           dungeon.level_count);
    csb_v1_dungeon_free(&dungeon);
}

int main(void)
{
    puts("test_csb_v1_amiga_dungeon_probe:");
    test_compressed_header();
    test_ftl_decompress_and_load();
    puts("ok: CSB Amiga dungeon probe");
    return 0;
}
