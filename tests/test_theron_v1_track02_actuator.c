#include "theron_v1_track02_actuator.h"
#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_thing_data.h"
#include "theron_v1_track02_item_id_map.h"
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

static void test_decode_basic(void) {
    /* 8-byte record: next_ref(2) + type/value(2) + effect(2) + target(2) */
    uint8_t raw[8] = {0};
    raw[0] = 0xFE; raw[1] = 0xFF; /* next = 0xFFFE (REFERENCE_NONE) */
    raw[2] = 0x05; raw[3] = 0x00; /* type=5, value=0 */

    Theron_Actuator act;
    assert(theron_v1_track02_actuator_decode(raw, &act) == 0);
    assert(act.next_ref == 0xFFFE);
    assert(act.type == 5);
    assert(act.value == 0);
    printf("  Basic decode OK\n");
}

static void test_value_fix(void) {
    assert(theron_v1_track02_actuator_needs_value_fix(TQ_ACT_WALL_ALCOVE_ITEM, 1));
    assert(theron_v1_track02_actuator_needs_value_fix(TQ_ACT_WALL_ITEM_EATER, 1));
    assert(!theron_v1_track02_actuator_needs_value_fix(TQ_ACT_WALL_TRIGGER, 1));
    assert(theron_v1_track02_actuator_needs_value_fix(TQ_ACT_FLOOR_CARRIED_ITEM, 0));
    assert(!theron_v1_track02_actuator_needs_value_fix(TQ_ACT_FLOOR_PARTY, 0));
    printf("  Value fix check OK\n");
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

        unsigned int num_act = dd.object_counts[3];
        unsigned int type_counts[128] = {0};
        unsigned int value_fix_count = 0;
        unsigned int none_refs = 0;

        for (unsigned int i = 0; i < num_act; i++) {
            Theron_Actuator act;
            assert(theron_v1_track02_actuator_decode(
                &td->items[3][i * 8], &act) == 0);
            if (act.type < 128) type_counts[act.type]++;
            if (act.next_ref == 0xFFFE) none_refs++;

            if (theron_v1_track02_actuator_needs_value_fix(act.type, 1)) {
                uint8_t translated = theron_v1_track02_translate_item_id(
                    (uint8_t)(act.value & 0xFF));
                if (translated != (act.value & 0xFF))
                    value_fix_count++;
            }
        }

        printf("  %s: %u actuators (%u end-of-chain)", names[d], num_act, none_refs);
        if (value_fix_count > 0)
            printf(" (%u need value fix)", value_fix_count);

        printf(" types:");
        for (int t = 0; t < 128; t++) {
            if (type_counts[t] > 0)
                printf(" %d×%u", t, type_counts[t]);
        }
        printf("\n");

        if (d == 0) {
            assert(num_act == 180);
            assert(type_counts[TQ_ACT_WALL_TRIGGER] > 0);
        }

        free(td);
    }
}

int main(void) {
    printf("test_theron_v1_track02_actuator\n");
    test_decode_basic();
    test_value_fix();

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
