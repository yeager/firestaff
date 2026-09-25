#include "theron_v1_track02_door.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02.h"
#include "asset_status_m12.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define SECTOR_SIZE 2352
#define UD_PER_SECTOR 2048
#define SYNC_OFFSET 16

static uint8_t *load_track02_ud(const char *path, size_t *out_size) {
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fseek(fp, 0, SEEK_END);
    long fsize = ftell(fp);
    fseek(fp, 0, SEEK_SET);
    if (fsize <= 0 || fsize % SECTOR_SIZE != 0) {
        fclose(fp);
        return NULL;
    }
    uint8_t *raw = malloc((size_t)fsize);
    if (!raw) { fclose(fp); return NULL; }
    if (fread(raw, 1, (size_t)fsize, fp) != (size_t)fsize) {
        free(raw);
        fclose(fp);
        return NULL;
    }
    fclose(fp);
    size_t sectors = (size_t)fsize / SECTOR_SIZE;
    size_t ud_size = sectors * UD_PER_SECTOR;
    uint8_t *ud = calloc(1, ud_size);
    if (!ud) { free(raw); return NULL; }
    for (size_t s = 0; s < sectors; s++)
        memcpy(ud + s * UD_PER_SECTOR, raw + s * SECTOR_SIZE + SYNC_OFFSET, UD_PER_SECTOR);
    free(raw);
    *out_size = ud_size;
    return ud;
}

static void test_door_decode_basic(void) {
    uint8_t raw[4] = {0xFE, 0xFF, 0x03, 0x00};
    Theron_Door door;
    assert(theron_v1_track02_door_decode(raw, &door) == 0);
    assert(door.next_ref == 0xFFFE);
    assert(door.type == 1);
    assert(door.ornate == 1);
    printf("  Door basic decode OK\n");
}

static void test_teleporter_decode_basic(void) {
    uint8_t raw[6] = {0xFE, 0xFF, 0x00, 0x00, 0x00, 0x00};
    Theron_Teleporter tp;
    assert(theron_v1_track02_teleporter_decode(raw, &tp) == 0);
    assert(tp.next_ref == 0xFFFE);
    assert(tp.x_dest == 0);
    assert(tp.y_dest == 0);
    printf("  Teleporter basic decode OK\n");
}

static void test_teleporter_level_destination_uses_six_bits(void) {
    /* w2 bits 8..13 are the source ldest field.  Values above 15 are outside
     * the current seven-level corpus but must remain lossless at decode. */
    const uint8_t raw[6] = {0, 0, 0, 0, 0, 0x2A};
    Theron_Teleporter tp;
    assert(theron_v1_track02_teleporter_decode(raw, &tp) == 0);
    assert(tp.level_dest == 42u);
}

static const char *find_track02(const char *environment,
                                const char *filename) {
    const char *explicit_path = getenv(environment);
    const char *home = getenv("HOME");
    static char paths[2][512];
    static unsigned int path_index;
    char *path = paths[path_index++ % 2u];
    const char *candidates[3] = { explicit_path, NULL, NULL };
    if (home && home[0]) {
        snprintf(path, sizeof(paths[0]), "%s/.firestaff/data/theron/%s",
                 home, filename);
        candidates[1] = path;
    }
    for (unsigned int i = 0; i < 2u; ++i) {
        FILE *fp;
        if (!candidates[i] || !candidates[i][0]) continue;
        fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

static int test_all_dungeons(const uint8_t *ud, size_t ud_size,
                             Theron_Track02Variant variant) {
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        if (!theron_v1_track02_dungeon_map_load_for_variant(
                ud, ud_size, variant, d, &dd)) {
            fprintf(stderr, "FAIL: dungeon %s map data did not load\n", names[d]);
            return 0;
        }

        unsigned int total_tiles = 0;
        uint8_t flat_tiles[8192];
        unsigned int fp2 = 0;
        for (unsigned int m = 0; m < dd.map_count; m++) {
            unsigned int w = dd.maps[m].header.x_dim + 1u;
            unsigned int h = dd.maps[m].header.y_dim + 1u;
            total_tiles += w * h;
            for (unsigned int x = 0; x < w; x++)
                for (unsigned int y = 0; y < h; y++)
                    flat_tiles[fp2++] = dd.maps[m].tiles[x][y];
        }

        unsigned int gref_count =
            theron_v1_track02_compute_ground_ref_count(flat_tiles, total_tiles);
        Theron_ThingData *td = calloc(1, sizeof(Theron_ThingData));
        if (!td) return 0;
        if (!theron_v1_track02_thing_data_load_for_variant(
                ud, ud_size, variant, d, dd.object_counts, gref_count, td)) {
            fprintf(stderr, "FAIL: dungeon %s thing data did not load\n", names[d]);
            free(td);
            return 0;
        }

        unsigned int num_doors = dd.object_counts[0];
        unsigned int num_telep = dd.object_counts[1];

        unsigned int wooden = 0, iron = 0, buttons = 0, bashable = 0;
        for (unsigned int i = 0; i < num_doors; i++) {
            Theron_Door door;
            assert(theron_v1_track02_door_decode(
                &td->items[0][i * 4], &door) == 0);
            if (door.type == 0) wooden++;
            else iron++;
            if (door.button) buttons++;
            if (door.bashable) bashable++;
        }

        unsigned int abs_tp = 0, sound_tp = 0;
        unsigned int max_level = 0;
        for (unsigned int i = 0; i < num_telep; i++) {
            Theron_Teleporter tp;
            assert(theron_v1_track02_teleporter_decode(
                &td->items[1][i * 6], &tp) == 0);
            if (tp.absolute) abs_tp++;
            if (tp.sound) sound_tp++;
            if (tp.level_dest > max_level) max_level = tp.level_dest;
        }

        printf("  %s: %u doors (wood=%u iron=%u btn=%u bash=%u) "
               "%u teleporters (abs=%u snd=%u maxlvl=%u)\n",
               names[d], num_doors, wooden, iron, buttons, bashable,
               num_telep, abs_tp, sound_tp, max_level);

        free(td);
    }
    return 1;
}

static int test_real_variant(const char *label, const char *path,
                             const char *expected_md5,
                             Theron_Track02Variant variant) {
    char actual_md5[33] = {0};
    size_t ud_size = 0;
    uint8_t *ud;

    if (!path) return 77;
    if (!m12_file_md5_hex(path, actual_md5) ||
        strcmp(actual_md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: %s Track 02 identity is not authenticated: %s\n",
                label, actual_md5);
        return 1;
    }
    ud = load_track02_ud(path, &ud_size);
    if (!ud) {
        fprintf(stderr, "FAIL: could not normalize authenticated %s Track 02\n",
                label);
        return 1;
    }
    printf("  %s Track 02 hash verified: %s\n", label, actual_md5);
    int ok = test_all_dungeons(ud, ud_size, variant);
    free(ud);
    return ok ? 0 : 1;
}

int main(void) {
    printf("test_theron_v1_track02_door\n");
    test_door_decode_basic();
    test_teleporter_decode_basic();
    test_teleporter_level_destination_uses_six_bits();

    const char *us_path = find_track02("FIRESTAFF_THERON_TRACK02_RAW",
                                       "TQUS02.bin");
    const char *jp_path = find_track02("FIRESTAFF_THERON_TRACK02_JP_RAW",
                                       "TQJP02.bin");
    int us_result = test_real_variant("US", us_path,
                                      THERON_TRACK02_MD5_US_BIN,
                                      THERON_TRACK02_VARIANT_US_BIN);
    if (us_result != 0) return us_result;
    if (jp_path) {
        int jp_result = test_real_variant("JP", jp_path,
                                          THERON_TRACK02_MD5_JP_BIN,
                                          THERON_TRACK02_VARIANT_JP_BIN);
        if (jp_result != 0) return jp_result;
    } else {
        printf("  SKIP: Japanese Track 02 BIN not found\n");
    }
    printf("PASS\n");
    return 0;
}
