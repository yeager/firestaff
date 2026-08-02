/*
 * test_dm2_v1_dungeon_loader_mac_amiga.c
 *
 * Validates DM2 dungeon loader against real Mac and Amiga DUNGEON.DAT files.
 * Both platforms use 68k big-endian format with magic 0x313b.
 *
 * Mac EN:   ~/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/Dungeon.dat (39,411 bytes)
 * Amiga EN: ~/.firestaff/data/dm2-extras/amiga-en-extracted/DUNGEON.DAT (39,411 bytes)
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

static void test_be_load(const char *path, const char *platform) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read %s DUNGEON.DAT at %s\n", platform, path);
        return;
    }

    printf("  %s DUNGEON.DAT: %zu bytes\n", platform, data_size);
    assert(data_size == 39411);

    /* Magic should be 0x313b (BE) = bytes 31 3b at offset 2 */
    assert(data[2] == 0x31 && data[3] == 0x3b);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    printf("  PASS: %s DUNGEON.DAT loaded successfully\n", platform);

    /* 28 maps, same as PC */
    assert(dungeon.level_count == 28);
    printf("  PASS: 28 maps\n");

    /* Byte-sized squares */
    assert(dungeon.square_bytes == 1);
    printf("  PASS: byte squares\n");

    /* Thing type counts — Mac/Amiga may differ slightly from PC */
    printf("  Thing counts: doors=%d teleporters=%d text=%d actuators=%d creatures=%d\n",
           dungeon.thing_type_counts[0], dungeon.thing_type_counts[1],
           dungeon.thing_type_counts[2], dungeon.thing_type_counts[3],
           dungeon.thing_type_counts[4]);

    /* thing_type_counts read from header offset 14 + i*2 */
    assert(dungeon.thing_type_counts[0] == 217);
    assert(dungeon.thing_type_counts[1] == 576);
    assert(dungeon.thing_type_counts[2] == 1020);
    assert(dungeon.thing_type_counts[3] == 299);
    assert(dungeon.thing_type_counts[4] == 173);
    printf("  PASS: thing type counts\n");

    /* Map 0 should be OUTDOOR */
    assert(dungeon.level_types[0] == DM2_LEVEL_OUTDOOR);
    printf("  PASS: map 0 is OUTDOOR\n");

    /* Verify structural fields were parsed */
    assert(dungeon.raw_map_data_base >= 0);
    assert(dungeon.column_index_base >= 0);
    printf("  PASS: raw_map_data_base=%d column_index_base=%d\n",
           dungeon.raw_map_data_base, dungeon.column_index_base);

    /* Map 0 dimensions should match PC (derived from bitfield, not endian) */
    assert(dungeon.level_widths[0] > 0 && dungeon.level_widths[0] <= 64);
    assert(dungeon.level_heights[0] > 0 && dungeon.level_heights[0] <= 64);
    printf("  PASS: map 0 dimensions: %dx%d\n",
           dungeon.level_widths[0], dungeon.level_heights[0]);

    /* Square type access should work for map 0 tile (0,0) */
    {
        int sq = dm2_v1_dungeon_get_square_type(&dungeon, 0, 0, 0);
        assert(sq >= 0 && sq <= 7);
        printf("  PASS: square type at (0,0,0) = %d\n", sq);
    }

    printf("  PASS: %s dungeon loader complete\n", platform);
    free(data);
}

int main(void) {
    const char *home;
    char mac_path[512], amiga_path[512], pc_path[512];

    printf("DM2 Mac/Amiga big-endian dungeon loader tests:\n");

    home = getenv("HOME");
    if (!home) {
        printf("  SKIP: HOME not set\n");
        return 0;
    }

    snprintf(mac_path, sizeof(mac_path),
             "%s/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/Dungeon.dat",
             home);
    test_be_load(mac_path, "Mac");

    snprintf(amiga_path, sizeof(amiga_path),
             "%s/.firestaff/data/dm2-extras/amiga-en-extracted/DUNGEON.DAT",
             home);
    test_be_load(amiga_path, "Amiga");

    /* Verify PC still works */
    {
        uint8_t *data;
        size_t data_size;
        DM2_V1_DungeonData dungeon;

        snprintf(pc_path, sizeof(pc_path),
                 "%s/.firestaff/data/dm2/DUNGEON.DAT", home);
        data = read_file(pc_path, &data_size);
        if (data) {
            assert(dm2_v1_dungeon_load(&dungeon, data, (int)data_size) == 0);
            assert(dungeon.level_count == 28);
            printf("  PASS: PC DUNGEON.DAT still loads\n");
            free(data);
        }
    }

    printf("\nAll Mac/Amiga dungeon loader tests passed.\n");
    return 0;
}
