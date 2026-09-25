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
    if (fsize <= 0) { fclose(fp); return NULL; }

    size_t raw_size = (size_t)fsize;
    uint8_t *raw = malloc(raw_size);
    if (!raw) { fclose(fp); return NULL; }
    fread(raw, 1, raw_size, fp);
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

static const char *find_track02(void) {
    const char *clonecd_path = getenv("FIRESTAFF_THERON_TRACK02_CLONECD_RAW");
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_RAW");
    const char *home = getenv("HOME");
    static char path[512];
    const char *candidates[3] = {
        clonecd_path && clonecd_path[0] ? clonecd_path : explicit_path,
        NULL, NULL
    };
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

static const char *find_jp_track02(void) {
    const char *explicit_path = getenv("FIRESTAFF_THERON_TRACK02_JP_RAW");
    static char path[512];
    const char *home = getenv("HOME");
    if (explicit_path && explicit_path[0]) return explicit_path;
    if (!home || !home[0]) return NULL;
    snprintf(path, sizeof(path), "%s/.firestaff/data/theron/TQJP02.bin", home);
    FILE *fp = fopen(path, "rb");
    if (!fp) return NULL;
    fclose(fp);
    return path;
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
    static const char *const names[] = {
        "AKUTUBA", "DRATOR", "FORMICIA", "SARMON",
        "SHADODAN", "THIEVES", "DEMON"
    };
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
    printf("  %s: %u authentic stair-class tiles; direction and destination unresolved\n",
           region, total);
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

int main(void) {
    printf("test_theron_v1_track02_dungeon_map\n");

    test_quest_block_offsets();
    test_map_counts();
    test_tile_helpers();
    printf("  Static tests OK\n");

    const char *track02_path = find_track02();
    const char *clonecd_env = getenv("FIRESTAFF_THERON_TRACK02_CLONECD_RAW");
    Theron_Track02Variant track02_variant =
        clonecd_env && clonecd_env[0]
            ? THERON_TRACK02_VARIANT_US_CLONECD_RAW
            : THERON_TRACK02_VARIANT_US_BIN;
    const char *expected_us_md5 =
        track02_variant == THERON_TRACK02_VARIANT_US_CLONECD_RAW
            ? THERON_TRACK02_MD5_US_CLONECD_BIN
            : THERON_TRACK02_MD5_US_BIN;
    if (!track02_path) {
        printf("  SKIP: Track 02 BIN not found\n");
        return 77;
    }
    char actual_md5[33] = {0};
    if (!m12_file_md5_hex(track02_path, actual_md5) ||
        strcmp(actual_md5, expected_us_md5) != 0) {
        fprintf(stderr, "FAIL: US Track 02 identity is not authenticated: %s\n",
                actual_md5);
        return 1;
    }

    size_t ud_size = 0;
    uint8_t *ud = load_track02_ud(track02_path, &ud_size);
    if (!ud) {
        printf("  SKIP: could not load Track 02\n");
        return 77;
    }

    test_akutuba_maps(ud, ud_size, track02_variant);
    test_drator_maps(ud, ud_size, track02_variant);
    test_all_dungeons(ud, ud_size, track02_variant);
    (void)report_authentic_stair_candidates(
        "US", ud, ud_size, track02_variant);

    free(ud);

    const char *jp_path = find_jp_track02();
    if (jp_path) {
        if (!m12_file_md5_hex(jp_path, actual_md5) ||
            strcmp(actual_md5, THERON_TRACK02_MD5_JP_BIN) != 0) {
            fprintf(stderr, "FAIL: JP Track 02 identity is not authenticated: %s\n",
                    actual_md5);
            return 1;
        }
        size_t jp_ud_size = 0;
        uint8_t *jp_ud = load_track02_ud(jp_path, &jp_ud_size);
        if (jp_ud) {
            test_jp_maps(jp_ud, jp_ud_size);
            free(jp_ud);
        }
    } else {
        printf("  SKIP: Japanese Track 02 BIN not found\n");
    }
    printf("PASS\n");
    return 0;
}
