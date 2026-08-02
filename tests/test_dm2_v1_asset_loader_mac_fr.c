/*
 * test_dm2_v1_asset_loader_mac_fr.c
 *
 * Validates DM2 GDAT asset loader against Mac French Graphics.dat.
 * Mac 68k BE, GDAT v5, ~8.16MB.
 */

#include "dm2_v1_asset_loader.h"

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
    DM2_V1_AssetLoader loader;

    printf("DM2 Mac FR GDAT asset loader tests:\n");

    home = getenv("HOME");
    if (!home) { printf("  SKIP: HOME not set\n"); return 0; }

    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mac-fr/Dungeon Master II/DMFiles/Graphics.dat",
             home);
    data = read_file(path, &data_size);
    if (!data) { printf("  SKIP: cannot read %s\n", path); return 0; }

    printf("  Mac FR Graphics.dat: %zu bytes\n", data_size);
    assert(data_size == 8157169);

    /* BE: first two bytes should be 0x80 0x05 */
    assert(data[0] == 0x80 && data[1] == 0x05);
    printf("  PASS: magic 0x8005 (GDAT v5 BE)\n");

    int rc = dm2_v1_asset_loader_init(&loader, data, data_size);
    assert(rc == 0);
    printf("  PASS: loader init OK\n");

    assert(loader.gdat_version == 5);
    printf("  PASS: GDAT version 5\n");

    assert(loader.big_endian == 1);
    printf("  PASS: big-endian\n");

    assert(loader.raw_data_count > 0);
    printf("  PASS: raw_data_count = %u\n", loader.raw_data_count);

    dm2_v1_asset_loader_free(&loader);
    free(data);
    printf("\nAll Mac FR GDAT tests passed.\n");
    return 0;
}
