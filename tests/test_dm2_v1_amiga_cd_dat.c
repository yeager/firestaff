/*
 * test_dm2_v1_amiga_cd_dat.c — Validates Amiga CD.DAT parser against real data.
 *
 * Amiga EN: ~/.firestaff/data/dm2-extras/amiga-en-extracted/CD.DAT (176 bytes)
 */

#include "dm2_v1_amiga_cd_dat.h"

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

static void test_null_safety(void) {
    DM2_V1_AmigaCdDat cd;
    assert(dm2_v1_amiga_cd_dat_parse(NULL, NULL, 0) == 0);
    assert(dm2_v1_amiga_cd_dat_parse(&cd, NULL, 0) == 0);
    assert(dm2_v1_amiga_cd_dat_mod_for_map(NULL, 0) == -1);
    printf("  PASS: null safety\n");
}

static void test_real_data(void) {
    const char *home;
    char path[512];
    uint8_t *data;
    size_t size;
    DM2_V1_AmigaCdDat cd;
    int track;

    home = getenv("HOME");
    if (!home) { printf("  SKIP: HOME not set\n"); return; }

    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/amiga-en-extracted/CD.DAT", home);
    data = read_file(path, &size);
    if (!data) { printf("  SKIP: cannot read %s\n", path); return; }

    printf("  CD.DAT: %zu bytes\n", size);
    assert(size == 176);

    assert(dm2_v1_amiga_cd_dat_parse(&cd, data, size) == 1);
    assert(cd.valid == 1);
    printf("  PASS: parse succeeded\n");

    /* Spot-check known mappings from hex dump analysis */
    track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, 0);
    assert(track == 3);
    printf("  PASS: map 0 -> SK03.MOD (track=%d)\n", track);

    track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, 1);
    assert(track == 1);
    printf("  PASS: map 1 -> SK01.MOD\n");

    track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, 5);
    assert(track == 9);
    printf("  PASS: map 5 -> SK09.MOD\n");

    track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, 15);
    assert(track == 0);
    printf("  PASS: map 15 -> SK00.MOD\n");

    track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, 43);
    assert(track == 2);
    printf("  PASS: map 43 -> SK02.MOD\n");

    /* All 44 maps should have a valid MOD track (0-9) */
    for (int i = 0; i < 44; ++i) {
        track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, i);
        assert(track >= 0 && track <= 9);
    }
    printf("  PASS: all 44 maps have valid MOD tracks\n");

    /* Out of bounds should return -1 */
    assert(dm2_v1_amiga_cd_dat_mod_for_map(&cd, -1) == -1);
    assert(dm2_v1_amiga_cd_dat_mod_for_map(&cd, 44) == -1);
    printf("  PASS: out-of-bounds returns -1\n");

    free(data);
    printf("  PASS: Amiga CD.DAT complete\n");
}

int main(void) {
    printf("DM2 Amiga CD.DAT parser tests:\n");
    test_null_safety();
    test_real_data();
    printf("\nAll Amiga CD.DAT tests passed.\n");
    return 0;
}
