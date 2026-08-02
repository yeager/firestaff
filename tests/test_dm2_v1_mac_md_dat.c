/*
 * test_dm2_v1_mac_md_dat.c
 *
 * Validates that Mac md.dat uses the same 176-byte format as Amiga CD.DAT.
 * Both map dungeon maps to music tracks via 44 x 4-byte entries.
 * Mac maps to HMP/MIDI tracks; Amiga maps to MOD tracks.
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

int main(void) {
    const char *home;
    char path[512];
    uint8_t *data;
    size_t data_size;
    DM2_V1_AmigaCdDat cd;

    printf("DM2 Mac md.dat music mapping tests:\n");

    home = getenv("HOME");
    if (!home) { printf("  SKIP: HOME not set\n"); return 0; }

    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/md.dat",
             home);
    data = read_file(path, &data_size);
    if (!data) { printf("  SKIP: cannot read %s\n", path); return 0; }

    printf("  Mac md.dat: %zu bytes\n", data_size);
    assert(data_size == DM2_AMIGA_CD_DAT_SIZE);
    printf("  PASS: size matches Amiga CD.DAT format (176 bytes)\n");

    /* Parse with the Amiga parser — same 4-byte entry format.
     * Note: Mac track numbers map to HMP files, not MOD files,
     * so track indices may exceed DM2_AMIGA_MOD_TRACK_COUNT (10).
     * The parser will skip entries with track >= 10. */
    assert(dm2_v1_amiga_cd_dat_parse(&cd, data, data_size));
    printf("  PASS: parsed with Amiga CD.DAT parser\n");

    /* Count mapped and unmapped maps */
    int mapped = 0, unmapped = 0;
    for (int i = 0; i < (int)DM2_AMIGA_CD_DAT_MAP_COUNT; i++) {
        int track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, i);
        if (track >= 0) mapped++;
        else unmapped++;
    }
    printf("  mapped: %d, unmapped: %d (of 44 maps)\n", mapped, unmapped);

    /* Print full mapping table for reference */
    printf("  Map-to-track table:\n");
    for (int i = 0; i < (int)DM2_AMIGA_CD_DAT_MAP_COUNT; i++) {
        uint8_t raw_track = data[i * 4 + 3];
        int parsed_track = dm2_v1_amiga_cd_dat_mod_for_map(&cd, i);
        printf("    map %2d -> raw track %2u (parsed: %d)\n",
               i, raw_track, parsed_track);
    }

    /* Mac has 29 HMP tracks (0-28), so many entries have track >= 10
     * and get filtered by the MOD-specific parser. This is expected. */
    printf("  NOTE: %d maps filtered (track >= 10, valid for HMP not MOD)\n",
           unmapped);

    free(data);
    printf("\nAll Mac md.dat tests passed.\n");
    return 0;
}
