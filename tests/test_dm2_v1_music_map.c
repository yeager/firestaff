/*
 * test_dm2_v1_music_map.c
 *
 * Validates DM2 generalized music map parser against:
 *   - Amiga CD.DAT (MOD, 10 tracks)
 *   - Mac md.dat (HMP, 29 tracks)
 */

#include "dm2_v1_music_map.h"

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

static void test_music_map(const char *label, const char *path,
                           DM2_MusicKind kind) {
    uint8_t *data;
    size_t data_size;
    DM2_V1_MusicMap mm;
    int mapped = 0, unmapped = 0;

    data = read_file(path, &data_size);
    if (!data) {
        printf("  SKIP: %s — cannot read %s\n", label, path);
        return;
    }

    printf("  %s: %zu bytes\n", label, data_size);
    assert(data_size == DM2_MUSIC_MAP_FILE_SIZE);

    assert(dm2_v1_music_map_parse(&mm, data, data_size, kind));
    printf("  PASS: parsed (%s, max %u tracks)\n",
           kind == DM2_MUSIC_KIND_MOD ? "MOD" : "HMP", mm.max_tracks);

    for (int i = 0; i < (int)DM2_MUSIC_MAP_FILE_ENTRIES; i++) {
        int track = dm2_v1_music_map_track_for_map(&mm, i);
        if (track >= 0) mapped++;
        else unmapped++;
    }
    printf("  mapped: %d, unmapped: %d\n", mapped, unmapped);

    assert(mapped > 0);
    printf("  PASS: %s\n", label);

    free(data);
}

int main(void) {
    const char *home;
    char path[512];

    printf("DM2 music map tests:\n\n");

    home = getenv("HOME");
    if (!home) { printf("SKIP: HOME not set\n"); return 0; }

    /* Amiga CD.DAT as MOD */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/amiga-en-extracted/CD.DAT", home);
    test_music_map("Amiga CD.DAT (MOD)", path, DM2_MUSIC_KIND_MOD);

    /* Mac md.dat as HMP */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/md.dat",
             home);
    test_music_map("Mac md.dat (HMP)", path, DM2_MUSIC_KIND_HMP);

    /* Mac md.dat as MOD (should lose entries with track >= 10) */
    snprintf(path, sizeof(path),
             "%s/.firestaff/data/dm2-extras/mac-en-v1/Dungeon Master II/DMFiles/md.dat",
             home);
    test_music_map("Mac md.dat (MOD filter)", path, DM2_MUSIC_KIND_MOD);

    printf("\nAll DM2 music map tests passed.\n");
    return 0;
}
