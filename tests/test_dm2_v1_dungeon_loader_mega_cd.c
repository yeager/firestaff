/*
 * test_dm2_v1_dungeon_loader_mega_cd.c
 *
 * Validates DM2 dungeon loader against Mega CD (Sega CD) DUNGEON.DAT.
 * Mega CD uses 68k big-endian format with magic 0x3093.
 *
 * Mega CD JP: ~/.firestaff/data/dm2-extras/mega-cd-jp-extracted/DUNGEON.DAT (37,957 bytes)
 */

#include "dm2_v1_dungeon_loader.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static uint8_t *read_file(const char *path, size_t *out_size) {
    FILE *f = fopen(path, "rb");
    uint8_t *data;
    long sz;
    if (!f) return NULL;
    fseek(f, 0, SEEK_END);
    sz = ftell(f);
    if (sz <= 0) { fclose(f); return NULL; }
    fseek(f, 0, SEEK_SET);
    data = malloc((size_t)sz);
    if (!data) { fclose(f); return NULL; }
    if (fread(data, 1, (size_t)sz, f) != (size_t)sz) {
        free(data);
        fclose(f);
        return NULL;
    }
    fclose(f);
    *out_size = (size_t)sz;
    return data;
}

int main(void) {
    const char *home;
    char path[512];
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    printf("DM2 Mega CD big-endian dungeon loader tests:\n");

    home = getenv("HOME");
    if (!home) {
        printf("  SKIP: HOME not set\n");
        return 0;
    }

    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mega-cd-jp-extracted/DUNGEON.DAT",
             home);
    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read Mega CD DUNGEON.DAT at %s\n", path);
        return 0;
    }

    printf("  Mega CD DUNGEON.DAT: %zu bytes\n", data_size);
    assert(data_size == 37957);

    /* Magic: 0x30 0x93 at offset 2 (BE encoding of 0x3093) */
    assert(data[2] == 0x30 && data[3] == 0x93);
    printf("  PASS: magic 0x3093 (Mega CD 68k BE)\n");

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    printf("  PASS: dungeon loaded\n");

    /* 28 maps, same as PC/Mac/Amiga */
    assert(dungeon.level_count == 28);
    printf("  PASS: 28 maps\n");

    /* Byte squares */
    assert(dungeon.square_bytes == 1);
    printf("  PASS: byte squares\n");

    /* Map 0 should be OUTDOOR */
    assert(dungeon.level_types[0] == DM2_LEVEL_OUTDOOR);
    printf("  PASS: map 0 is OUTDOOR\n");

    /* Print thing counts for reference */
    printf("  Thing counts:");
    for (int i = 0; i < 5; ++i)
        printf(" [%d]=%d", i, dungeon.thing_type_counts[i]);
    printf("\n");

    /* Map 0 dimensions */
    assert(dungeon.level_widths[0] > 0 && dungeon.level_widths[0] <= 64);
    assert(dungeon.level_heights[0] > 0 && dungeon.level_heights[0] <= 64);
    printf("  PASS: map 0 dimensions: %dx%d\n",
           dungeon.level_widths[0], dungeon.level_heights[0]);

    /* Square type access */
    {
        int sq = dm2_v1_dungeon_get_square_type(&dungeon, 0, 0, 0);
        assert(sq >= 0 && sq <= 7);
        printf("  PASS: square type at (0,0,0) = %d\n", sq);
    }

    free(data);
    printf("\nAll Mega CD dungeon loader tests passed.\n");
    return 0;
}
