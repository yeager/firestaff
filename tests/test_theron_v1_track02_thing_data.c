#include "theron_v1_track02_dungeon_map.h"
#include "theron_v1_track02_item_properties.h"
#include "theron_v1_track02_thing_data.h"
#ifdef NDEBUG
#undef NDEBUG
#endif
#ifdef NDEBUG
#undef NDEBUG
#endif
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

static const char *find_track02_variant(Theron_Track02Variant variant) {
    const char *home = getenv("HOME");
    const char *explicit_path = (variant == THERON_TRACK02_VARIANT_JP_BIN)
        ? getenv("FIRESTAFF_THERON_TRACK02_JP_RAW")
        : getenv("FIRESTAFF_THERON_TRACK02_RAW");
    static char path[512];
    const char *candidates[2];

    candidates[0] = explicit_path;
    if (home && home[0]) {
        snprintf(path, sizeof(path), "%s/.firestaff/data/theron/%s.bin",
                 home, variant == THERON_TRACK02_VARIANT_JP_BIN ? "TQJP02" : "TQUS02");
        candidates[1] = path;
    } else {
        candidates[1] = NULL;
    }
    for (unsigned int i = 0; i < 2u; ++i) {
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
    /* Authentic US AKUTUBA category-14/15 index-0 rows.  test_all_dungeons()
     * also compares these bytes to the loaded Track 02 tables. */
    const uint8_t missile[] = {0xff, 0xff, 0x10, 0x10, 0x01, 0x00, 0x00, 0x00};
    const uint8_t cloud[] = {0xff, 0xff, 0x2e, 0x0e};
    Theron_Track02ItemRecord record;

    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_MISSILE, missile, sizeof(missile), &record));
    assert(record.next_ref == 0xffffu &&
           record.value.missile.unknown1 == 0x10u &&
           record.value.missile.spell == 0x10u &&
           record.value.missile.power == 0x01u &&
           record.value.missile.unknown2 == 0x00u &&
           record.value.missile.zero == 0x00u &&
           record.value.missile.e == 0x00u);
    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_CLOUD, cloud, sizeof(cloud), &record));
    assert(record.next_ref == 0xffffu && record.value.cloud.power == 0x2eu &&
           record.value.cloud.spell == 0x0eu);
    assert(!theron_v1_track02_item_record_decode(
        THERON_CAT_MISSILE, missile, sizeof(missile) - 1u, &record));
    assert(!theron_v1_track02_item_record_decode(
        THERON_CAT_CLOUD, cloud, sizeof(cloud) - 1u, &record));
    printf("  source missile/cloud record layouts OK\n");
}

static void test_source_control_record_fields(void) {
    /* Authentic US AKUTUBA category rows: door 0, teleporter 0, text 0 and
     * the B4 square's actuator 5.  test_all_dungeons() below also compares
     * these constants to the loaded Track 02 tables before using them as
     * focused decoder regressions. */
    const uint8_t door[] = {0xFE, 0xFF, 0x21, 0x00};
    const uint8_t teleporter[] = {0x00, 0x08, 0x62, 0x74, 0x00, 0x00};
    const uint8_t text[] = {0xFE, 0xFF, 0x00, 0x00};
    /* Authentic US/JP AKUTUBA M0 (2,1), category-3 index 5.  This is the
     * party actuator chained after the closed B4 teleporter, retained from
     * raw Track 02 as feff0300a4078018. */
    const uint8_t actuator[] = {0xFE, 0xFF, 0x03, 0x00, 0xA4, 0x07,
                                0x80, 0x18};
    Theron_Track02ItemRecord record;

    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_DOOR, door, sizeof(door), &record));
    assert(record.next_ref == 0xFFFEu && record.value.door.type == 1u &&
           record.value.door.ornate == 0u && record.value.door.opens_up == 1u &&
           record.value.door.button == 0u && record.value.door.destroyable == 0u &&
           record.value.door.bashable == 0u);
    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_TELEPORTER, teleporter, sizeof(teleporter), &record));
    assert(record.next_ref == 0x0800u &&
           record.value.teleporter.xdest == 2u &&
           record.value.teleporter.ydest == 3u &&
           record.value.teleporter.rotation == 1u &&
           record.value.teleporter.absolute == 1u &&
           record.value.teleporter.scope == 3u &&
           record.value.teleporter.sound == 0u &&
           record.value.teleporter.ldest == 0u &&
           record.value.teleporter.unused == 0u);
    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_TEXT, text, sizeof(text), &record));
    assert(record.next_ref == 0xFFFEu &&
           record.value.text.visible == 0u && record.value.text.flag2 == 0u &&
           record.value.text.flag3 == 0u && record.value.text.offset == 0u);
    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_ACTUATOR, actuator, sizeof(actuator), &record));
    assert(record.next_ref == 0xFFFEu &&
           record.value.actuator.type == 3u &&
           record.value.actuator.value == 0u &&
           record.value.actuator.unreferenced_bit0 == 0u &&
           record.value.actuator.unreferenced_bit1 == 0u &&
           record.value.actuator.once == 1u &&
           record.value.actuator.effect == 0u &&
           record.value.actuator.revert_effect == 1u &&
           record.value.actuator.sound == 0u &&
           record.value.actuator.delay == 15u &&
           record.value.actuator.local_effect == 0u &&
           record.value.actuator.graphism == 0u &&
           record.value.actuator.target_x == 2u &&
           record.value.actuator.target_y == 3u &&
           record.value.actuator.facing == 0u &&
           record.value.actuator.local_multiple == 0x0880u);
    printf("  source door/teleporter/text/actuator fields OK\n");
}

static void test_source_monster_chested_field(void) {
    /* Authentic US AKUTUBA category-4 index-0 row: generic next reference,
     * followed by the 14-byte dm_monster payload. */
    const uint8_t monster[] = {
        0xfe, 0xff, 0xfe, 0xff, 0x0a, 0x01, 0x3b, 0x00,
        0x25, 0x00, 0x24, 0x00, 0x25, 0x00, 0x20, 0x04
    };
    Theron_Track02ItemRecord record;

    assert(theron_v1_track02_item_record_decode(
        THERON_CAT_MONSTER, monster, sizeof(monster), &record));
    assert(record.next_ref == 0xfffeu);
    assert(record.value.monster.chested == -2);
    assert(record.value.monster.type == 0x0au);
    assert(record.value.monster.position == 0x01u);
    assert(record.value.monster.health[0] == 0x003bu);
    assert(record.value.monster.health[1] == 0x0025u);
    assert(record.value.monster.health[2] == 0x0024u);
    assert(record.value.monster.health[3] == 0x0025u);
    assert(record.value.monster.flags_word == 0x0420u);
    assert(record.value.monster.number == 1u);
    assert(record.value.monster.unknown_word == 0u);
    assert(record.value.monster.direction_flags == 0x04u);
    printf("  source monster chested field layout OK\n");
}

static void test_real_item_records(const Theron_ThingData *td,
                                   unsigned int dungeon_index,
                                   int require_us_counts) {
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
    for (unsigned int cat = 0; cat < THERON_ITEM_CATEGORY_COUNT; ++cat) {
        if (require_us_counts && cat <= THERON_CAT_MISC)
            assert(td->object_counts[cat] == expected[dungeon_index][cat]);
        if (cat == THERON_CAT_MISSILE)
            assert(td->object_counts[cat] == 60u);
        if (cat == THERON_CAT_CLOUD)
            assert(td->object_counts[cat] == 50u);
        if (theron_item_bytes[cat] == 0u) {
            assert(td->object_counts[cat] == 0u);
            continue;
        }
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
                if (cat == THERON_CAT_MONSTER) {
                    assert(record.next_ref ==
                           ((uint16_t)raw[0] | ((uint16_t)raw[1] << 8)));
                    assert(record.value.monster.chested ==
                           (int16_t)((uint16_t)raw[2] |
                                     ((uint16_t)raw[3] << 8)));
                } else
                    assert(record.next_ref ==
                           ((uint16_t)raw[0] | ((uint16_t)raw[1] << 8)));
            }
        }
    }
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size,
                              Theron_Track02Variant variant,
                              const char *label) {
    const char *names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };

    assert(theron_v1_track02_item_properties_match_source(
        ud, ud_size, variant == THERON_TRACK02_VARIANT_JP_BIN));
    {
        uint8_t source_row[THERON_TRACK02_ITEM_PROPERTY_SIZE];
        size_t source_offset = 0u;
        assert(theron_v1_track02_item_property_source_row(
            ud, ud_size, variant == THERON_TRACK02_VARIANT_JP_BIN, 4u,
            source_row, &source_offset));
        assert(source_offset != 0u);
        assert(memcmp(source_row, theron_v1_track02_item_property(4u),
                      sizeof(source_row)) == 0);
    }
    printf("  %s Track 02 item property table matches source bytes OK\n",
           label);

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_load_for_variant(
            ud, ud_size, variant, d, &dd));
        assert(dd.object_counts[THERON_CAT_MISSILE] == 60u);
        assert(dd.object_counts[THERON_CAT_CLOUD] == 50u);

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
        int ok = theron_v1_track02_thing_data_load_for_variant(
            ud, ud_size, variant, d, dd.object_counts, gref_count, td);
        assert(ok);

        if (variant == THERON_TRACK02_VARIANT_US_BIN && d == 0) {
            static const uint8_t real_control_rows[4][8] = {
                {0xfe, 0xff, 0x21, 0x00},
                {0x00, 0x08, 0x62, 0x74, 0x00, 0x00},
                {0xfe, 0xff, 0x00, 0x00},
                {0xfe, 0xff, 0x03, 0x00, 0xa4, 0x07, 0x80, 0x18}
            };
            const unsigned int ids[] = {0u, 0u, 0u, 5u};
            for (unsigned int cat = 0; cat < 4u; ++cat)
                assert(memcmp(&td->items[cat][ids[cat] * theron_item_bytes[cat]],
                              real_control_rows[cat],
                              theron_item_bytes[cat]) == 0);
            {
                static const uint8_t real_monster[] = {
                    0xfe, 0xff, 0xfe, 0xff, 0x0a, 0x01, 0x3b, 0x00,
                    0x25, 0x00, 0x24, 0x00, 0x25, 0x00, 0x20, 0x04
                };
                static const uint8_t real_missile[] = {
                    0xff, 0xff, 0x10, 0x10, 0x01, 0x00, 0x00, 0x00
                };
                static const uint8_t real_cloud[] = {
                    0xff, 0xff, 0x2e, 0x0e
                };
                assert(memcmp(td->items[THERON_CAT_MONSTER], real_monster,
                              sizeof(real_monster)) == 0);
                assert(memcmp(td->items[THERON_CAT_MISSILE], real_missile,
                              sizeof(real_missile)) == 0);
                assert(memcmp(td->items[THERON_CAT_CLOUD], real_cloud,
                              sizeof(real_cloud)) == 0);
            }
        }

        assert(td->ground_ref_count == gref_count);
        test_real_item_records(td, d,
                               variant == THERON_TRACK02_VARIANT_US_BIN);

        unsigned int total_items = 0;
        for (int c = 0; c < 16; c++)
            total_items += td->object_counts[c];

        printf("  %s %s: %u ground_refs, %u items, %u text_words OK\n",
               label, names[d], gref_count, total_items, td->text_data_count);

        if (variant == THERON_TRACK02_VARIANT_US_BIN && d == 0) {
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
    test_source_control_record_fields();
    test_source_monster_chested_field();

    const char *path = find_track02_variant(THERON_TRACK02_VARIANT_US_BIN);
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

    test_all_dungeons(ud, ud_size, THERON_TRACK02_VARIANT_US_BIN, "US");
    free(ud);

    path = find_track02_variant(THERON_TRACK02_VARIANT_JP_BIN);
    if (!path) {
        printf("  SKIP: JP Track 02 BIN not found\n");
        return 0;
    }
    ud = load_track02_ud(path, &ud_size);
    assert(ud);
    test_all_dungeons(ud, ud_size, THERON_TRACK02_VARIANT_JP_BIN, "JP");
    free(ud);
    printf("PASS\n");
    return 0;
}
