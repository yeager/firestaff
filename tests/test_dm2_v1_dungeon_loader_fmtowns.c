/*
 * test_dm2_v1_dungeon_loader_fmtowns.c
 *
 * Validates DM2 dungeon loader against real FM Towns DUNGEON.DAT.
 * Source: ~/.firestaff/data/dm2-fmtowns-ja/DUNGEON.DAT (37,954 bytes)
 *
 * The FM Towns DUNGEON.DAT uses magic 0x3094 at offset 2 instead of
 * PC's 0x3147 ('G1'), but has the same header layout, map definition
 * format, and byte-square tile data.
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

static void test_fmtowns_load(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read FM Towns DUNGEON.DAT at %s\n", path);
        return;
    }

    printf("  FM Towns DUNGEON.DAT: %zu bytes\n", data_size);
    assert(data_size == 37954);

    /* Magic should be 0x3094 */
    assert(data[2] == 0x94 && data[3] == 0x30);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    printf("  PASS: FM Towns DUNGEON.DAT loaded successfully\n");

    /* 28 maps, same as PC */
    assert(dungeon.level_count == 28);
    printf("  PASS: 28 maps\n");

    /* Byte-sized squares */
    assert(dungeon.square_bytes == 1);
    printf("  PASS: byte squares\n");

    /* Thing type counts from header */
    assert(dungeon.thing_type_counts[0] == 209);  /* doors */
    assert(dungeon.thing_type_counts[1] == 448);  /* teleporters */
    assert(dungeon.thing_type_counts[2] == 1020); /* text */
    assert(dungeon.thing_type_counts[3] == 280);  /* actuators */
    assert(dungeon.thing_type_counts[4] == 169);  /* creatures */
    printf("  PASS: thing type counts match header\n");

    /* Verify some map data was parsed — raw_map_data_base should be set */
    assert(dungeon.raw_map_data_base >= 0);
    printf("  PASS: raw map data base set (%d)\n", dungeon.raw_map_data_base);

    /* Column index should be set */
    assert(dungeon.column_index_base >= 0);
    printf("  PASS: column index base set (%d)\n", dungeon.column_index_base);

    printf("  PASS: FM Towns dungeon loader\n");
    free(data);
}

static void test_pc_still_works(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_DungeonData dungeon;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read PC DUNGEON.DAT at %s\n", path);
        return;
    }

    /* Magic should be 0x3147 = 'G1' */
    assert(data[2] == 0x47 && data[3] == 0x31);

    result = dm2_v1_dungeon_load(&dungeon, data, (int)data_size);
    assert(result == 0);
    assert(dungeon.level_count == 28);
    printf("  PASS: PC DUNGEON.DAT still loads\n");

    free(data);
}

int main(void) {
    const char *home;
    char fm_path[512], pc_path[512];

    printf("DM2 FM Towns dungeon loader tests:\n");

    home = getenv("HOME");
    if (!home) {
        printf("  SKIP: HOME not set\n");
        return 0;
    }

    snprintf(fm_path, sizeof(fm_path),
             "%s/.firestaff/data/dm2-fmtowns-ja/DUNGEON.DAT", home);
    test_fmtowns_load(fm_path);

    snprintf(pc_path, sizeof(pc_path),
             "%s/.firestaff/data/dm2/DUNGEON.DAT", home);
    test_pc_still_works(pc_path);

    printf("\nAll FM Towns dungeon loader tests passed.\n");
    return 0;
}
