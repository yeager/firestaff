#include "theron_v1_track02_door.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
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
    if (fsize <= 0) { fclose(fp); return NULL; }
    uint8_t *raw = malloc((size_t)fsize);
    if (!raw) { fclose(fp); return NULL; }
    fread(raw, 1, (size_t)fsize, fp);
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

static const char *find_track02(void) {
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    const char *candidates[3] = { explicit_path, NULL, NULL };
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin", home);
        candidates[1] = path;
        snprintf(path + 256, sizeof(path) - 256,
                 "%s/.firestaff/data/theron/raw-us/"
                 "Dungeon Master - Theron's Quest (USA) (Track 02).bin", home);
        candidates[2] = path + 256;
    }
    for (unsigned int i = 0; i < 3u; ++i) {
        FILE *fp;
        if (!candidates[i] || !candidates[i][0]) continue;
        fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA","DRATOR","FORMICIA","SARMON","SHADODAN","THIEVES","DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_load(ud, ud_size, d, &dd));

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
        assert(td);
        assert(theron_v1_track02_thing_data_load(
            ud, ud_size, d, dd.object_counts, gref_count, td));

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
}

int main(void) {
    printf("test_theron_v1_track02_door\n");
    test_door_decode_basic();
    test_teleporter_decode_basic();

    const char *path = find_track02();
    if (!path) {
        printf("  SKIP: Track 02 BIN not found\n");
        return 0;
    }
    size_t ud_size = 0;
    uint8_t *ud = load_track02_ud(path, &ud_size);
    if (!ud) {
        printf("  SKIP: could not load Track 02\n");
        return 0;
    }
    test_all_dungeons(ud, ud_size);
    free(ud);
    printf("PASS\n");
    return 0;
}
