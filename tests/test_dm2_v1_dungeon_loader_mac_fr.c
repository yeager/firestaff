/*
 * test_dm2_v1_dungeon_loader_mac_fr.c
 *
 * Validates DM2 dungeon loader against Mac French Dungeon.dat.
 * Mac 68k BE format, same magic 0x313b as Mac EN.
 */

#include "dm2_v1_dungeon_loader.h"

#include <assert.h>
#include <stdio.h>
#include <stdlib.h>

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

    printf("DM2 Mac FR dungeon loader tests:\n");

    home = getenv("HOME");
    if (!home) { printf("  SKIP: HOME not set\n"); return 0; }

    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mac-fr/Dungeon Master II/DMFiles/Dungeon.dat",
             home);
    data = read_file(path, &data_size);
    if (!data) { printf("  SKIP: cannot read %s\n", path); return 0; }

    printf("  Mac FR Dungeon.dat: %zu bytes\n", data_size);
    assert(data_size == 39411);

    /* Magic 0x313b in BE: bytes 31 3b at offset 2 */
    assert(data[2] == 0x31 && data[3] == 0x3b);
    printf("  PASS: magic 0x313b (Mac 68k BE)\n");

    assert(dm2_v1_dungeon_load(&dungeon, data, (int)data_size) == 0);
    printf("  PASS: dungeon loaded\n");

    assert(dungeon.level_count == 28);
    printf("  PASS: 28 maps\n");

    assert(dungeon.square_bytes == 1);
    printf("  PASS: byte squares\n");

    assert(dungeon.level_types[0] == DM2_LEVEL_OUTDOOR);
    printf("  PASS: map 0 is OUTDOOR\n");

    printf("  Thing counts:");
    for (int i = 0; i < 5; ++i)
        printf(" [%d]=%d", i, dungeon.thing_type_counts[i]);
    printf("\n");

    assert(dungeon.level_widths[0] > 0 && dungeon.level_widths[0] <= 64);
    printf("  PASS: map 0 dimensions: %dx%d\n",
           dungeon.level_widths[0], dungeon.level_heights[0]);

    free(data);
    printf("\nAll Mac FR dungeon loader tests passed.\n");
    return 0;
}
