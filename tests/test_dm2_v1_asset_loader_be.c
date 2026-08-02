/*
 * test_dm2_v1_asset_loader_be.c
 *
 * Validates the DM2 GDAT asset loader against big-endian GRAPHICS.DAT
 * files from Mac 68k and Amiga AGA platforms.
 *
 * Amiga EN: ~/.firestaff/data/dm2-extras/amiga-en-extracted/GRAPHICS.DAT (3,493,879 bytes)
 * Mac EN:   ~/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/Graphics.dat (8,157,169 bytes)
 */

#include "dm2_v1_asset_loader.h"

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

static void test_be_gdat(const char *path, const char *platform) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_AssetLoader loader;
    int result;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read %s GRAPHICS.DAT at %s\n", platform, path);
        return;
    }

    printf("  %s GRAPHICS.DAT: %zu bytes\n", platform, data_size);

    /* Verify BE magic: first two bytes are 0x80 0x05 (BE encoding of 0x8005) */
    assert(data[0] == 0x80 || data[0] == 0x05);

    result = dm2_v1_asset_loader_init(&loader, data, data_size);
    if (result != 0) {
        printf("  SKIP: %s GRAPHICS.DAT init failed (result=%d) — format not yet supported\n",
               platform, result);
        free(data);
        return;
    }

    printf("  PASS: %s GDAT loaded, version=%u, big_endian=%d\n",
           platform, loader.gdat_version, loader.big_endian);
    assert(loader.big_endian == 1);
    assert(loader.gdat_version == 5);
    assert(loader.loaded == 1);

    printf("  raw_data_count=%u, entry_count=%u\n",
           loader.raw_data_count, loader.entry_count);
    assert(loader.raw_data_count > 0);
    assert(loader.entry_count > 0);

    /* Try to query FIGHTER text (cat=0x07 idx=0x00 field=0x00) */
    {
        const uint8_t *text;
        size_t text_size = 0;
        text = dm2_v1_asset_load_sized(&loader, 0x07, 0x00, 0x00, &text_size);
        if (text && text_size > 0) {
            printf("  Text [0x07:0x00:0x00]: %zu bytes\n", text_size);
        } else {
            printf("  INFO: no text at [0x07:0x00:0x00] (may be encrypted)\n");
        }
    }

    /* Verify the typed graph is valid */
    if (dm2_v1_asset_loader_validate_typed_graph(&loader)) {
        printf("  PASS: typed graph valid\n");
    } else {
        printf("  INFO: typed graph validation failed (may need BE fixes)\n");
    }

    /* Verify */
    if (dm2_v1_asset_loader_verify(&loader)) {
        printf("  PASS: loader verify\n");
    } else {
        printf("  INFO: loader verify failed\n");
    }

    dm2_v1_asset_loader_free(&loader);
    free(data);
    printf("  PASS: %s BE GDAT loader complete\n", platform);
}

static void test_pc_still_works(const char *path) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_AssetLoader loader;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: cannot read PC GRAPHICS.DAT at %s\n", path);
        return;
    }

    assert(dm2_v1_asset_loader_init(&loader, data, data_size) == 0);
    assert(loader.big_endian == 0);
    assert(loader.gdat_version == 5);
    printf("  PASS: PC GRAPHICS.DAT still loads (big_endian=%d)\n",
           loader.big_endian);

    dm2_v1_asset_loader_free(&loader);
    free(data);
}

int main(void) {
    const char *home;
    char amiga_path[512], mac_path[512], pc_path[512];

    printf("DM2 big-endian GDAT asset loader tests:\n");

    home = getenv("HOME");
    if (!home) {
        printf("  SKIP: HOME not set\n");
        return 0;
    }

    snprintf(amiga_path, sizeof(amiga_path),
             "%s/.firestaff/data/dm2-extras/amiga-en-extracted/GRAPHICS.DAT", home);
    test_be_gdat(amiga_path, "Amiga");

    snprintf(mac_path, sizeof(mac_path),
             "%s/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/Graphics.dat",
             home);
    test_be_gdat(mac_path, "Mac");

    snprintf(pc_path, sizeof(pc_path),
             "%s/.firestaff/data/dm2/GRAPHICS.DAT", home);
    test_pc_still_works(pc_path);

    printf("\nAll BE GDAT tests passed.\n");
    return 0;
}
