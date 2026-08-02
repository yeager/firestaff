/*
 * test_dm2_v1_cdda_cd_dat.c
 *
 * Validates DM2 CDDA Red Book music trigger format (40 bytes)
 * against FM Towns, Mega CD, and PC-9821 CD.DAT files.
 * All three are byte-identical.
 */

#include "dm2_v1_cdda_cd_dat.h"

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

static void test_cdda_file(const char *label, const char *path,
                           const uint8_t *reference, size_t ref_size) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_CddaCdDat cd;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: %s — cannot read %s\n", label, path);
        return;
    }

    printf("  %s: %zu bytes\n", label, data_size);
    assert(data_size == DM2_CDDA_CD_DAT_SIZE);

    if (reference) {
        assert(memcmp(data, reference, ref_size) == 0);
        printf("  PASS: byte-identical to reference\n");
    }

    assert(dm2_v1_cdda_cd_dat_parse(&cd, data, data_size));
    printf("  PASS: parsed %u entries\n", DM2_CDDA_CD_DAT_ENTRY_COUNT);

    printf("  Music trigger table:\n");
    for (int i = 0; i < (int)DM2_CDDA_CD_DAT_ENTRY_COUNT; i++) {
        printf("    [%d] x=%u y=%u level=%u track=%u\n",
               i, cd.entries[i].x, cd.entries[i].y,
               cd.entries[i].level, cd.entries[i].track);
    }

    free(data);
    printf("  PASS: %s\n\n", label);
}

static void test_lookup(void) {
    /* Known entry 0: x=6, y=6, level=10, track=6 */
    const uint8_t sample[40] = {
        0x06,0x06,0x0a,0x06, 0x23,0x1a,0x06,0x03,
        0x0f,0x24,0x06,0x02, 0x09,0x24,0x06,0x02,
        0x07,0x1e,0x06,0x02, 0x0f,0x1a,0x06,0x02,
        0x25,0x39,0x06,0x02, 0x53,0x24,0x06,0x02,
        0x54,0x1a,0x06,0x02, 0x54,0x11,0x06,0x02
    };
    DM2_V1_CddaCdDat cd;
    int track;

    assert(dm2_v1_cdda_cd_dat_parse(&cd, sample, sizeof(sample)));

    track = dm2_v1_cdda_cd_dat_track_at(&cd, 10, 6, 6);
    assert(track == 6);
    printf("  PASS: lookup (10, 6, 6) -> track 6\n");

    track = dm2_v1_cdda_cd_dat_track_at(&cd, 6, 0x23, 0x1a);
    assert(track == 3);
    printf("  PASS: lookup (6, 0x23, 0x1a) -> track 3\n");

    track = dm2_v1_cdda_cd_dat_track_at(&cd, 0, 0, 0);
    assert(track == -1);
    printf("  PASS: lookup miss -> -1\n");
}

int main(void) {
    const char *home;
    char path_fmtowns[512], path_megacd[512], path_pc9821[512];
    uint8_t *ref_data = NULL;
    size_t ref_size = 0;

    printf("DM2 CDDA CD.DAT music trigger tests:\n\n");

    home = getenv("HOME");
    if (!home) { printf("SKIP: HOME not set\n"); return 0; }

    snprintf(path_fmtowns, sizeof(path_fmtowns),
             "%s/.firestaff/data/dm2-extras/fm-towns-ja/extracted/CD.DAT", home);
    snprintf(path_megacd, sizeof(path_megacd),
             "%s/.firestaff/data/dm2-extras/mega-cd-jp-extracted/CD.DAT", home);
    snprintf(path_pc9821, sizeof(path_pc9821),
             "%s/.firestaff/data/dm2-extras/pc9821-jp-extracted/CD.DAT", home);

    ref_data = read_file(path_fmtowns, &ref_size);
    test_cdda_file("FM Towns CD.DAT", path_fmtowns, NULL, 0);
    test_cdda_file("Mega CD CD.DAT", path_megacd, ref_data, ref_size);
    test_cdda_file("PC-9821 CD.DAT", path_pc9821, ref_data, ref_size);

    printf("Coordinate lookup tests:\n");
    test_lookup();

    free(ref_data);
    printf("\nAll DM2 CDDA CD.DAT tests passed.\n");
    return 0;
}
