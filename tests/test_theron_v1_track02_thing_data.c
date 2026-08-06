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
    size_t raw_size = (size_t)fsize;
    if (raw_size % SECTOR_SIZE != 0u) { fclose(fp); return NULL; }
    uint8_t *raw = malloc(raw_size);
    if (!raw) { fclose(fp); return NULL; }
    fread(raw, 1, raw_size, fp);
    fclose(fp);
    size_t sectors = raw_size / SECTOR_SIZE;
    size_t ud_size = sectors * UD_PER_SECTOR;
    uint8_t *ud = calloc(1, ud_size);
    if (!ud) { free(raw); return NULL; }
    for (size_t s = 0; s < sectors; s++)
        memcpy(ud + s * UD_PER_SECTOR, raw + s * SECTOR_SIZE + SYNC_OFFSET, UD_PER_SECTOR);
    free(raw);
    *out_size = ud_size;
    return ud;
}

static const char *find_track02(void) {
    const char *home = getenv("HOME");
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    static char path[512];
    const char *candidates[3];

    candidates[0] = explicit_path;
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQUS02.bin",
                 home);
        candidates[1] = path;
        snprintf(path + 256, sizeof(path) - 256,
                 "%s/.firestaff/data/theron/raw-us/"
                 "Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                 home);
        candidates[2] = path + 256;
    } else {
        candidates[1] = NULL;
        candidates[2] = NULL;
    }
    for (unsigned int i = 0; i < 3u; ++i) {
        FILE *fp;
        if (!candidates[i] || !candidates[i][0]) continue;
        fp = fopen(candidates[i], "rb");
        if (fp) { fclose(fp); return candidates[i]; }
    }
    return NULL;
}

static void test_ground_ref_count(void) {
    uint8_t tiles[] = { 0x00, 0x10, 0x20, 0x30, 0x1F, 0x0F };
    unsigned int count = theron_v1_track02_compute_ground_ref_count(tiles, 6);
    assert(count == 3);
    printf("  ground_ref_count helper OK\n");
}

static void test_ground_ref_count_bound(void) {
    uint8_t byte = 0;
    uint16_t object_counts[THERON_ITEM_CATEGORY_COUNT] = {0};
    Theron_ThingData data;

    assert(!theron_v1_track02_thing_data_load(
        &byte, sizeof(byte), 0, object_counts,
        THERON_MAX_GROUND_REFS + 1u, &data));
    printf("  ground_ref_count bound rejects overflow OK\n");
}

static void test_truncated_source_rejected(void) {
    uint8_t byte = 0;
    uint16_t object_counts[THERON_ITEM_CATEGORY_COUNT] = {0};
    Theron_ThingData data;

    /* The first real ground-reference offset is far beyond this buffer.
     * A short source must be rejected before any pointer arithmetic can
     * wrap or manufacture an object record. */
    assert(!theron_v1_track02_thing_data_load(
        &byte, sizeof(byte), 0, object_counts, 0, &data));
    printf("  truncated source rejects out-of-range records OK\n");
}

static void test_truncated_map_source_rejected(void) {
    uint8_t byte = 0;
    Theron_DungeonData dungeon;

    assert(!theron_v1_track02_dungeon_map_load(&byte, sizeof(byte), 0,
                                               &dungeon));
    printf("  truncated map source rejects out-of-range tables OK\n");
}

static void test_source_category_layout(void) {
    assert(THERON_CAT_MONSTER == 4);
    assert(THERON_CAT_WEAPON == 5);
    assert(THERON_CAT_CLOTHING == 6);
    assert(THERON_CAT_SCROLL == 7);
    assert(THERON_CAT_POTION == 8);
    assert(THERON_CAT_CONTAINER == 9);
    assert(THERON_CAT_MISC == 10);
    assert(THERON_CAT_MISSILE == 14);
    assert(THERON_CAT_CLOUD == 15);
    assert(theron_item_bytes[THERON_CAT_MONSTER] == 16u);
    assert(theron_item_bytes[THERON_CAT_CONTAINER] == 8u);
    assert(theron_item_bytes[THERON_CAT_MISSILE] == 8u);
    printf("  source category order and record widths OK\n");
}

static void test_source_projectile_records(void) {
    const uint8_t missile[] = {0x34, 0x12, 0x80, 0x05, 0x09, 0x44, 0x00, 0x7f};
    const uint8_t cloud[] = {0x78, 0x56, 0x09, 0x03};
    Theron_Track02ItemRecord record;

    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_MISSILE, missile, sizeof(missile), &record));
    assert(record.next_ref == 0x1234u &&
           record.value.missile.unknown1 == 0x80u &&
           record.value.missile.spell == 0x05u &&
           record.value.missile.power == 0x09u &&
           record.value.missile.unknown2 == 0x44u &&
           record.value.missile.zero == 0x00u &&
           record.value.missile.e == 0x7fu);
    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_CLOUD, cloud, sizeof(cloud), &record));
    assert(record.next_ref == 0x5678u && record.value.cloud.power == 0x09u &&
           record.value.cloud.spell == 0x03u);
    assert(!theron_v1_track02_item_record_decode(
        THERON_CAT_MISSILE, missile, sizeof(missile) - 1u, &record));
    assert(!theron_v1_track02_item_record_decode(
        THERON_CAT_CLOUD, cloud, sizeof(cloud) - 1u, &record));
    printf("  source missile/cloud record layouts OK\n");
}

static void test_real_item_records(const Theron_ThingData *td,
                                   unsigned int dungeon_index) {
    /* Authenticated object-count words from each real US quest block. */
    static const uint16_t expected[7][11] = {
        {31, 68, 17, 180, 118, 143, 153, 6, 11, 3, 181},
        {38, 38, 11, 199, 106, 123, 159, 1, 8, 0, 176},
        {19, 23, 11, 149, 101, 126, 151, 0, 12, 0, 169},
        {19, 106, 15, 341, 94, 123, 151, 3, 15, 0, 155},
        {5, 127, 13, 181, 107, 116, 153, 4, 14, 1, 149},
        {12, 71, 19, 192, 101, 136, 171, 3, 14, 0, 159},
        {16, 29, 21, 118, 90, 148, 172, 1, 9, 3, 164},
    };

    assert(dungeon_index < 7u);
    for (unsigned int cat = 0; cat <= THERON_CAT_MISC; ++cat) {
        assert(td->object_counts[cat] == expected[dungeon_index][cat]);
        if (td->object_counts[cat] == 0u)
            continue;
        size_t bytes = (size_t)td->object_counts[cat] *
                       theron_item_bytes[cat];
        int has_payload = 0;
        for (size_t i = 0; i < bytes; ++i) {
            if (td->items[cat][i] != 0u) {
                has_payload = 1;
                break;
            }
        }
        assert(has_payload);

        if (cat >= THERON_CAT_MONSTER) {
            for (unsigned int id = 0; id < td->object_counts[cat]; ++id) {
                Theron_Track02ItemRecord record;
                const uint8_t *raw =
                    &td->items[cat][id * theron_item_bytes[cat]];
                assert(theron_v1_track02_item_record_decode(
                    cat, raw, theron_item_bytes[cat], &record));
                assert(record.category == cat);
                assert(record.next_ref ==
                       ((uint16_t)raw[0] | ((uint16_t)raw[1] << 8)));
            }
        }
    }
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size) {
    const char *names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_load(ud, ud_size, d, &dd));

        unsigned int total_tiles = 0;
        uint8_t flat_tiles[4096];
        unsigned int flat_pos = 0;
        for (unsigned int m = 0; m < dd.map_count; m++) {
            unsigned int w = dd.maps[m].header.x_dim + 1u;
            unsigned int h = dd.maps[m].header.y_dim + 1u;
            total_tiles += w * h;
            for (unsigned int x = 0; x < w; x++)
                for (unsigned int y = 0; y < h; y++)
                    flat_tiles[flat_pos++] = dd.maps[m].tiles[x][y];
        }

        unsigned int gref_count =
            theron_v1_track02_compute_ground_ref_count(flat_tiles, total_tiles);

        Theron_ThingData *td = calloc(1, sizeof(Theron_ThingData));
        assert(td);
        int ok = theron_v1_track02_thing_data_load(
            ud, ud_size, d, dd.object_counts, gref_count, td);
        assert(ok);

        assert(td->ground_ref_count == gref_count);
        test_real_item_records(td, d);

        unsigned int total_items = 0;
        for (int c = 0; c < 16; c++)
            total_items += td->object_counts[c];

        printf("  %s: %u ground_refs, %u items, %u text_words OK\n",
               names[d], gref_count, total_items, td->text_data_count);

        if (d == 0) {
            assert(td->object_counts[THERON_CAT_DOOR] == 31);
            assert(td->object_counts[THERON_CAT_TELEPORTER] == 68);
            assert(td->object_counts[THERON_CAT_ACTUATOR] == 180);
            assert(td->text_data_count == 0x013C);
        }

        free(td);
    }
}

int main(void) {
    printf("test_theron_v1_track02_thing_data\n");

    test_ground_ref_count();
    test_ground_ref_count_bound();
    test_truncated_source_rejected();
    test_truncated_map_source_rejected();
    test_source_category_layout();
    test_source_projectile_records();

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
