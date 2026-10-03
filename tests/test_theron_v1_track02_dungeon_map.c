#include "theron_v1_track02_dungeon_map.h"
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

    size_t raw_size = (size_t)fsize;
    uint8_t *raw = malloc(raw_size);
    if (!raw) { fclose(fp); return NULL; }
    if (fread(raw, 1, raw_size, fp) != raw_size) {
        free(raw);
        fclose(fp);
        return NULL;
    }
    fclose(fp);

    size_t sectors = raw_size / SECTOR_SIZE;
    size_t ud_size = sectors * UD_PER_SECTOR;
    uint8_t *ud = calloc(1, ud_size);
    if (!ud) { free(raw); return NULL; }

    for (size_t s = 0; s < sectors; s++) {
        memcpy(ud + s * UD_PER_SECTOR,
               raw + s * SECTOR_SIZE + SYNC_OFFSET,
               UD_PER_SECTOR);
    }
    free(raw);
    *out_size = ud_size;
    return ud;
}

static void test_quest_block_offsets(void) {
    Theron_QuestBlockOffsets qb;
    assert(theron_v1_track02_dungeon_map_quest_block_offsets(0, &qb));
    assert(qb.dims_offset == 0x0002A0F1);
    assert(qb.map_data_offset == 0x0002A2D7);
    assert(!theron_v1_track02_dungeon_map_quest_block_offsets(7, &qb));
}

static void test_map_counts(void) {
    assert(theron_v1_track02_dungeon_map_count(0) == 4);
    assert(theron_v1_track02_dungeon_map_count(1) == 8);
    assert(theron_v1_track02_dungeon_map_count(2) == 5);
    assert(theron_v1_track02_dungeon_map_count(3) == 6);
    assert(theron_v1_track02_dungeon_map_count(4) == 3);
    assert(theron_v1_track02_dungeon_map_count(5) == 4);
    assert(theron_v1_track02_dungeon_map_count(6) == 4);
    assert(theron_v1_track02_dungeon_map_count(7) == 0);
}

static void test_tile_helpers(void) {
    assert(theron_tile_type(0x00) == THERON_TILE_WALL);
    assert(theron_tile_type(0x20) == THERON_TILE_OPEN);
    assert(theron_tile_type(0x40) == THERON_TILE_PIT);
    assert(theron_tile_type(0x60) == THERON_TILE_STAIRS);
    assert(theron_tile_type(0x80) == THERON_TILE_DOOR);
    assert(theron_tile_type(0xA0) == THERON_TILE_TELEPORTER);
    assert(theron_tile_type(0xC0) == THERON_TILE_FAKEWALL);
    assert(theron_tile_type(0xE0) == THERON_TILE_TYPE7);
    assert(theron_tile_has_things(0x10) == 1);
    assert(theron_tile_has_things(0x0F) == 0);
    assert(theron_tile_attributes(0x3F) == 0x0F);
}

static const char *find_track02(const char *region,
                                Theron_Track02Variant *variant,
                                const char **expected_md5) {
    const char *home = getenv("HOME");
    static char paths[2][512];
    const char *override;

    if (strcmp(region, "jp") == 0) {
        static char jp_path[512];
        *variant = THERON_TRACK02_VARIANT_JP_BIN;
        *expected_md5 = THERON_TRACK02_MD5_JP_BIN;
        override = getenv("FIRESTAFF_THERON_TRACK02_JP_RAW");
        if (override && override[0]) return override;
        if (!home || !home[0]) return NULL;
        int written = snprintf(jp_path, sizeof(jp_path),
                              "%s/.firestaff/data/theron/TQJP02.bin", home);
        if (written < 0 || (size_t)written >= sizeof(jp_path)) {
            fprintf(stderr, "FAIL: HOME-derived JP Track 02 path is too long\n");
            jp_path[0] = '\0';
            return jp_path;
        }
        FILE *fp = fopen(jp_path, "rb");
        if (!fp) return NULL;
        fclose(fp);
        return jp_path;
    }

    override = getenv("FIRESTAFF_THERON_TRACK02_CLONECD_RAW");
    *variant = THERON_TRACK02_VARIANT_US_CLONECD_RAW;
    *expected_md5 = THERON_TRACK02_MD5_US_CLONECD_BIN;
    if (override && override[0]) return override;

    override = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    *variant = THERON_TRACK02_VARIANT_US_BIN;
    *expected_md5 = THERON_TRACK02_MD5_US_BIN;
    if (override && override[0]) return override;
    if (!home || !home[0]) return NULL;

    int written = snprintf(paths[0], sizeof(paths[0]),
                           "%s/.firestaff/data/theron/TQUS02.bin", home);
    if (written < 0 || (size_t)written >= sizeof(paths[0])) {
        fprintf(stderr, "FAIL: HOME-derived US Track 02 path is too long\n");
        paths[0][0] = '\0';
        return paths[0];
    }
    FILE *fp = fopen(paths[0], "rb");
    if (fp) {
        fclose(fp);
        return paths[0];
    }

    written = snprintf(paths[1], sizeof(paths[1]),
                       "%s/.firestaff/data/theron/raw-us/"
                       "Dungeon Master - Theron's Quest (USA) (Track 02).bin",
                       home);
    if (written < 0 || (size_t)written >= sizeof(paths[1])) {
        fprintf(stderr, "FAIL: HOME-derived US raw Track 02 path is too long\n");
        paths[1][0] = '\0';
        return paths[1];
    }
    fp = fopen(paths[1], "rb");
    if (!fp) return NULL;
    fclose(fp);
    return paths[1];
}

/* Expected dimensions from dmbuilder source (stored as dim-1). */
static const uint8_t akutuba_xdims[] = { 5, 18, 18, 12 };
static const uint8_t akutuba_ydims[] = { 7, 12, 16, 11 };
static const uint8_t drator_xdims[]  = { 5, 9, 13, 13, 13, 14, 13, 7 };
static const uint8_t drator_ydims[]  = { 7, 10, 12, 8, 9, 12, 9, 9 };

static void test_akutuba_maps(const uint8_t *ud, size_t ud_size,
                              Theron_Track02Variant variant) {
    Theron_DungeonData dd;
    assert(theron_v1_track02_dungeon_map_load_for_variant(
        ud, ud_size, variant, 0, &dd));
    assert(dd.map_count == 4);
    assert(dd.dungeon_index == 0);

    for (int m = 0; m < 4; m++) {
        assert(dd.maps[m].header.x_dim == akutuba_xdims[m]);
        assert(dd.maps[m].header.y_dim == akutuba_ydims[m]);
    }

    /* Level 0 hub: 6x8 grid. First tile is a teleporter (0xB8 = type 5). */
    assert(theron_tile_type(dd.maps[0].tiles[0][0]) == THERON_TILE_TELEPORTER);

    /* Level 0 hub: tile at (1,0) is open floor (0x20 = type 1). */
    assert(theron_tile_type(dd.maps[0].tiles[1][0]) == THERON_TILE_OPEN);

    /* Level 1: creatures=2, xp=0. */
    assert(dd.maps[1].header.creature_count == 2);
    assert(dd.maps[1].header.xp_modifier == 0);

    /* Level 3: creatures=1, xp=5. */
    assert(dd.maps[3].header.creature_count == 1);
    assert(dd.maps[3].header.xp_modifier == 5);

    assert(dd.creature_gfx_bank[0] == 0x0003);
    assert(dd.cumulative_column_items[0] == 0x0000);
    assert(dd.cumulative_column_items[1] == 0x000A);

    printf("  AKUTUBA: 4 maps OK\n");
}

static void test_drator_maps(const uint8_t *ud, size_t ud_size,
                             Theron_Track02Variant variant) {
    Theron_DungeonData dd;
    assert(theron_v1_track02_dungeon_map_load_for_variant(
        ud, ud_size, variant, 1, &dd));
    assert(dd.map_count == 8);

    for (int m = 0; m < 8; m++) {
        assert(dd.maps[m].header.x_dim == drator_xdims[m]);
        assert(dd.maps[m].header.y_dim == drator_ydims[m]);
    }

    /* Level 0 hub has same layout as AKUTUBA hub. */
    assert(theron_tile_type(dd.maps[0].tiles[0][0]) == THERON_TILE_TELEPORTER);

    printf("  DRATOR: 8 maps OK\n");
}

static void test_all_dungeons(const uint8_t *ud, size_t ud_size,
                              Theron_Track02Variant variant) {
    const uint8_t expected_maps[] = { 4, 8, 5, 6, 3, 4, 4 };
    const char *names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        int ok = theron_v1_track02_dungeon_map_load_for_variant(
            ud, ud_size, variant, d, &dd);
        assert(ok);
        assert(dd.map_count == expected_maps[d]);

        /* All hubs are 6x8 (stored 5x7). */
        assert(dd.maps[0].header.x_dim == 5);
        assert(dd.maps[0].header.y_dim == 7);

        unsigned int total_tiles = 0;
        for (unsigned int m = 0; m < dd.map_count; m++) {
            unsigned int w = dd.maps[m].header.x_dim + 1u;
            unsigned int h = dd.maps[m].header.y_dim + 1u;
            total_tiles += w * h;
        }

        printf("  %s: %u maps, %u tiles OK\n", names[d], dd.map_count, total_tiles);
    }
}

static unsigned int report_authentic_stair_candidates(
    const char *region, const uint8_t *ud, size_t ud_size,
    Theron_Track02Variant variant) {
    static const unsigned int expected_attributes[2][16] = {
        [0] = {
            22u, 10u, 5u, 1u, 21u, 8u, 7u, 4u,
            25u, 10u, 3u, 6u, 21u, 10u, 9u, 9u
        },
        [1] = {
            18u, 6u, 9u, 10u, 22u, 3u, 3u, 6u,
            32u, 4u, 8u, 4u, 19u, 1u, 11u, 14u
        }
    };
    static const char *const names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };
    const unsigned int region_index =
        variant == THERON_TRACK02_VARIANT_JP_BIN ? 1u : 0u;
    unsigned int attribute_counts[16] = {0u};
    unsigned int total = 0u;

    for (unsigned int dungeon = 0u; dungeon < THERON_TRACK02_DUNGEON_COUNT;
         ++dungeon) {
        Theron_DungeonData data;
        if (!theron_v1_track02_dungeon_map_load_for_variant(
                ud, ud_size, variant, dungeon, &data)) {
            fprintf(stderr, "FAIL: %s authenticated dungeon %u failed to load\n",
                    region, dungeon);
            exit(1);
        }
        for (unsigned int map = 0u; map < data.map_count; ++map) {
            const unsigned int width =
                (unsigned int)data.maps[map].header.x_dim + 1u;
            const unsigned int height =
                (unsigned int)data.maps[map].header.y_dim + 1u;
            for (unsigned int x = 0u; x < width; ++x) {
                for (unsigned int y = 0u; y < height; ++y) {
                    const uint8_t raw = data.maps[map].tiles[x][y];
                    if (theron_tile_type(raw) != THERON_TILE_STAIRS) continue;
                    ++total;
                    ++attribute_counts[theron_tile_attributes(raw)];
                    printf("  source-only stair candidate region=%s dungeon=%s "
                           "map=%u x=%u y=%u raw=%02x attributes=%x\n",
                           region, names[dungeon], map, x, y,
                           (unsigned int)raw,
                           (unsigned int)theron_tile_attributes(raw));
                }
            }
        }
    }
    const unsigned int expected =
        variant == THERON_TRACK02_VARIANT_JP_BIN ? 170u : 171u;
    if (total != expected) {
        fprintf(stderr, "FAIL: %s authentic stair-class tile count %u, expected %u\n",
                region, total, expected);
        exit(1);
    }
    for (unsigned int attributes = 0u; attributes < 16u; ++attributes) {
        if (attribute_counts[attributes] !=
            expected_attributes[region_index][attributes]) {
            fprintf(stderr,
                    "FAIL: %s authentic stair attribute 0x%x count %u, "
                    "expected %u\n",
                    region, attributes, attribute_counts[attributes],
                    expected_attributes[region_index][attributes]);
            exit(1);
        }
    }
    printf("  %s: %u authentic stair-class tiles; direction and destination unresolved\n",
           region, total);
    for (unsigned int attributes = 0u; attributes < 16u; ++attributes) {
        if (attribute_counts[attributes] != 0u)
            printf("  source-only stair attribute region=%s value=0x%x count=%u; "
                   "semantics unresolved\n",
                   region, attributes, attribute_counts[attributes]);
    }
    return total;
}

static int quest_offsets_equal(const Theron_QuestBlockOffsets *left,
                               const Theron_QuestBlockOffsets *right) {
    return left && right &&
        left->dims_offset == right->dims_offset &&
        left->map_data_offset == right->map_data_offset &&
        left->ground_refs_offset == right->ground_refs_offset &&
        left->items_part1_offset == right->items_part1_offset &&
        left->items_part2_offset == right->items_part2_offset &&
        left->text_data_offset == right->text_data_offset;
}

static void test_jp_maps(const uint8_t *ud, size_t ud_size) {
    const uint8_t expected_maps[] = { 4, 8, 5, 6, 3, 4, 4 };
    static const Theron_QuestBlockOffsets expected_offsets[] = {
        { 0x0002991D, 0x00029B03, 0x0002A05D, 0x0002A53B,
          0x0002B000, 0 },
        { 0x00069D50, 0x0006A006, 0x0006A5C2, 0x0006AA4E,
          0x0006B000, 0 },
        { 0x000AA261, 0x000AA422, 0x000AA8DA, 0x000AAD6E,
          0x000AB000, 0 },
        { 0x000E9B47, 0x000E9CD4, 0x000EA188, 0x000EA5FA,
          0x000EB000, 0 },
        { 0x0012A3CB, 0x0012A544, 0x0012AABE, 0x0012AF6C,
          0x0012B000, 0 },
        { 0x00169860, 0x00169A4C, 0x0016A118, 0x0016A618,
          0x0016B000, 0 },
        { 0x001AA043, 0x001AA1FE, 0x001AA857, 0x001AACCF,
          0x001AB000, 0 },
    };
    Theron_QuestBlockOffsets qb;

    for (unsigned int d = 0; d < 7; d++) {
        Theron_DungeonData dd;
        assert(theron_v1_track02_dungeon_map_quest_block_offsets_for_variant(
            THERON_TRACK02_VARIANT_JP_BIN, d, &qb));
        assert(quest_offsets_equal(&qb, &expected_offsets[d]));
        /* JP text ownership is unresolved. A zero offset is a deliberate
         * admission barrier, not a license to decode an adjacent table. */
        assert(qb.text_data_offset == 0);
        if (!theron_v1_track02_dungeon_map_load_for_variant(
                ud, ud_size, THERON_TRACK02_VARIANT_JP_BIN, d, &dd)) {
            fprintf(stderr, "FAIL: JP authenticated dungeon %u failed to load\n", d);
            exit(1);
        }
        assert(dd.map_count == expected_maps[d]);
        assert(dd.maps[0].header.x_dim == 5);
        assert(dd.maps[0].header.y_dim == 7);
        printf("  JP dungeon %u authentic layout: dims=%06x maps=%06x "
               "grefs=%06x items=%06x/%06x text=unbound\n",
               d + 1u, qb.dims_offset, qb.map_data_offset,
               qb.ground_refs_offset, qb.items_part1_offset,
               qb.items_part2_offset);
    }
    printf("  JP Track 02: all dungeon maps OK\n");
    (void)report_authentic_stair_candidates(
        "JP", ud, ud_size, THERON_TRACK02_VARIANT_JP_BIN);
}

int main(int argc, char **argv) {
    if (argc != 2 ||
        (strcmp(argv[1], "us") != 0 && strcmp(argv[1], "jp") != 0)) {
        fprintf(stderr, "usage: %s us|jp\n", argv[0]);
        return 2;
    }

    const char *region = argv[1];
    printf("test_theron_v1_track02_dungeon_map_%s\n", region);

    test_quest_block_offsets();
    test_map_counts();
    test_tile_helpers();
    printf("  Static tests OK\n");

    Theron_Track02Variant track02_variant;
    const char *expected_md5;
    const char *track02_path = find_track02(
        region, &track02_variant, &expected_md5);
    if (!track02_path) {
        printf("  SKIP: %s Track 02 BIN not found\n",
               strcmp(region, "us") == 0 ? "US" : "JP");
        return 77;
    }
    char actual_md5[33] = {0};
    if (!m12_file_md5_hex(track02_path, actual_md5) ||
        strcmp(actual_md5, expected_md5) != 0) {
        fprintf(stderr, "FAIL: %s Track 02 identity is not authenticated: %s\n",
                strcmp(region, "us") == 0 ? "US" : "JP", actual_md5);
        return 1;
    }

    size_t ud_size = 0;
    uint8_t *ud = load_track02_ud(track02_path, &ud_size);
    if (!ud) {
        fprintf(stderr, "FAIL: could not load authenticated %s Track 02\n",
                strcmp(region, "us") == 0 ? "US" : "JP");
        return 1;
    }

    if (strcmp(region, "us") == 0) {
        test_akutuba_maps(ud, ud_size, track02_variant);
        test_drator_maps(ud, ud_size, track02_variant);
        test_all_dungeons(ud, ud_size, track02_variant);
        (void)report_authentic_stair_candidates(
            "US", ud, ud_size, track02_variant);
    } else {
        test_jp_maps(ud, ud_size);
    }
    free(ud);
    printf("PASS: %s real media\n",
           strcmp(region, "us") == 0 ? "US" : "JP");
    return 0;
}
