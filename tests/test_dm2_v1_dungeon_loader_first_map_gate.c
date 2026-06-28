/*
 * test_dm2_v1_dungeon_loader_first_map_gate.c
 *
 * Synthetic, data-free gate for DM2 V1 dungeon-loader map-0 bring-up.
 *
 * Source-lock:
 *   ReDMCSB DEFS.H lines 989-998: DUNGEON_HEADER.MapCount and header fields.
 *   ReDMCSB DEFS.H lines 1049-1116: MAP.RawMapDataByteOffset and descriptor.
 *   dm2_v1_dungeon_loader.c: tile data starts after the fixed 28 descriptors
 *   and is read column-major as uint16 words.
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

#define DM2_TEST_HEADER_SIZE 44
#define DM2_TEST_MAP_DESC_SIZE 16
#define DM2_TEST_DESCRIPTOR_COUNT 28
#define DM2_TEST_TILE_DATA_START \
    (DM2_TEST_HEADER_SIZE + DM2_TEST_DESCRIPTOR_COUNT * DM2_TEST_MAP_DESC_SIZE)

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { \
        passed++; \
        printf("  PASS: %s\n", msg); \
    } else { \
        failed++; \
        printf("  FAIL: %s\n", msg); \
    } \
} while (0)

static void put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffU);
    p[1] = (uint8_t)((v >> 8) & 0xffU);
}

static size_t build_first_map_fixture(uint8_t *buf, size_t cap)
{
    const int w = 3;
    const int h = 2;
    uint8_t *desc;
    uint8_t *tiles;
    const uint16_t raw_tiles[6] = {
        0x0021U, /* x=0,y=0: type 1 with flags */
        0x0002U, /* x=0,y=1: type 2 */
        0x0003U, /* x=1,y=0: type 3 */
        0x001fU, /* x=1,y=1: type 31 */
        0x0004U, /* x=2,y=0: type 4 */
        0x0005U  /* x=2,y=1: type 5 */
    };

    if (cap < DM2_TEST_TILE_DATA_START + sizeof(raw_tiles))
        return 0;

    memset(buf, 0, cap);

    put16le(buf + 2, 0x4731U); /* "G1" */
    put16le(buf + 4, DM2_TEST_HEADER_SIZE);
    buf[6] = 1; /* DUNGEON_HEADER.MapCount */

    desc = buf + DM2_TEST_HEADER_SIZE;
    put16le(desc + 0, 0);             /* map-0 raw tile offset */
    put16le(desc + 4, (uint16_t)((w - 1) << 5 | (h - 1)));
    put16le(desc + 12, (uint16_t)w);  /* DM2 width override */
    put16le(desc + 14, (uint16_t)h);  /* DM2 height override */

    tiles = buf + DM2_TEST_TILE_DATA_START;
    for (int i = 0; i < 6; i++)
        put16le(tiles + i * 2, raw_tiles[i]);

    return DM2_TEST_TILE_DATA_START + sizeof(raw_tiles);
}

static void test_first_map_metadata_and_tiles(void)
{
    uint8_t dat[DM2_TEST_TILE_DATA_START + 12];
    DM2_V1_DungeonData dungeon;
    size_t size = build_first_map_fixture(dat, sizeof(dat));

    CHECK(size == sizeof(dat), "synthetic map-0 fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts complete first-map fixture");
    CHECK(dungeon.level_count == 1, "level_count is read from header byte 6");
    CHECK(dungeon.level_types[0] == DM2_LEVEL_OUTDOOR,
          "first level is marked outdoor");
    CHECK(dungeon.level_widths[0] == 3, "map-0 width override is preserved");
    CHECK(dungeon.level_heights[0] == 2, "map-0 height override is preserved");
    CHECK(dungeon.level_offsets[0] == 0, "map-0 raw offset is zero");
    CHECK(dm2_v1_dungeon_is_outdoor(&dungeon, 0) == 1,
          "is_outdoor reports map 0");

    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 0, 0) == 0x0021,
          "raw first tile keeps high flag bits");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 0, 0) == 1,
          "square type masks the first tile to low 5 bits");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 1, 0) == 3,
          "column-major lookup reaches x=1,y=0");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 1, 1) == 31,
          "column-major lookup reaches x=1,y=1");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 2, 1) == 5,
          "last in-bounds map-0 tile is readable");

    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 3, 0) == -1,
          "x past map-0 width is rejected");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 0, 2) == -1,
          "y past map-0 height is rejected");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 1, 0, 0) == -1,
          "level past map_count is rejected");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_truncated_first_map_rejected(void)
{
    uint8_t dat[DM2_TEST_TILE_DATA_START + 12];
    DM2_V1_DungeonData dungeon;
    size_t size = build_first_map_fixture(dat, sizeof(dat));

    CHECK(size == sizeof(dat), "truncation fixture starts complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size - 1) == -1,
          "loader rejects missing final byte of map-0 tile span");
}

int main(void)
{
    printf("=== DM2 V1 Dungeon Loader First-Map Gate ===\n\n");

    test_first_map_metadata_and_tiles();
    test_truncated_first_map_rejected();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
