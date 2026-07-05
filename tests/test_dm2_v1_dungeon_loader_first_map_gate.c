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

static size_t build_skproject_layout_fixture(uint8_t *buf, size_t cap)
{
    const int w = 2;
    const int h = 2;
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + (size_t)w * 2u;
    const size_t thing_base = sft_base + 2u;
    const size_t raw_map_base = thing_base + 4u;
    uint8_t *desc;
    uint16_t door_bits;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);

    buf[4] = 1; /* skproject File_header.nMaps */
    put16le(buf + 10, 1); /* File_header.cwListSize */
    put16le(buf + 12, 1); /* nRecords[dbDoor] */

    desc = buf + header_size;
    put16le(desc + 0, 0); /* map data offset */
    desc[6] = 0;
    desc[7] = 0;
    put16le(desc + 8, (uint16_t)(((w - 1) << 6) | ((h - 1) << 11)));

    put16le(buf + column_base + 0, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0000); /* ObjectID: dbDoor, index 0 */

    put16le(buf + thing_base, 0xfffe); /* next link/end marker */
    door_bits = (uint16_t)((1u << 6) | (1u << 11) | (1u << 5) | 1u);
    put16le(buf + thing_base + 2, door_bits);

    buf[raw_map_base + 0] = 0x20; /* x=0,y=0: floor */
    buf[raw_map_base + 1] = 0x20; /* x=0,y=1: floor */
    buf[raw_map_base + 2] = 0x90; /* x=1,y=0: ttDoor + thing-list flag */
    buf[raw_map_base + 3] = 0x20; /* x=1,y=1: floor */
    return raw_map_base + 4u;
}

static size_t build_skproject_chained_door_fixture(uint8_t *buf, size_t cap)
{
    const int w = 2;
    const int h = 2;
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + (size_t)w * 2u;
    const size_t thing_base = sft_base + 2u;
    const size_t door_base = thing_base;
    const size_t text_base = door_base + 4u;
    const size_t raw_map_base = text_base + 4u;
    uint8_t *desc;
    uint16_t door_bits;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);

    buf[4] = 1; /* skproject File_header.nMaps */
    put16le(buf + 10, 1); /* File_header.cwListSize */
    put16le(buf + 12, 1); /* nRecords[dbDoor] */
    put16le(buf + 16, 1); /* nRecords[dbText] */

    desc = buf + header_size;
    put16le(desc + 0, 0);
    put16le(desc + 8, (uint16_t)(((w - 1) << 6) | ((h - 1) << 11)));

    put16le(buf + column_base + 0, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0800); /* ObjectID: dbText, index 0 */

    put16le(buf + door_base, 0xfffe);
    door_bits = (uint16_t)((1u << 6) | (1u << 11) | (1u << 5) | 1u);
    put16le(buf + door_base + 2, door_bits);

    put16le(buf + text_base, 0x0000); /* text.next -> dbDoor index 0 */
    put16le(buf + text_base + 2, 0x0000);

    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static size_t build_skproject_text_wall_gfx_fixture(uint8_t *buf, size_t cap)
{
    size_t size = build_skproject_chained_door_fixture(buf, cap);
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + 4u;
    const size_t thing_base = sft_base + 2u;
    const size_t door_base = thing_base;
    const size_t text_base = door_base + 4u;

    if (size == 0) return 0;
    put16le(buf + sft_base, 0x8800); /* dir=S, dbText, index 0 */
    put16le(buf + text_base, 0x0000);
    put16le(buf + text_base + 2, (uint16_t)((1u << 1) | (0x2au << 3)));
    return size;
}

static size_t build_skproject_actuator_wall_gfx_fixture(uint8_t *buf, size_t cap)
{
    const int w = 2;
    const int h = 2;
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + (size_t)w * 2u;
    const size_t thing_base = sft_base + 2u;
    const size_t door_base = thing_base;
    const size_t actuator_base = door_base + 4u;
    const size_t raw_map_base = actuator_base + 8u;
    uint8_t *desc;
    uint16_t door_bits;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);

    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 12, 1); /* nRecords[dbDoor] */
    put16le(buf + 18, 1); /* nRecords[dbActuator] */

    desc = buf + header_size;
    put16le(desc + 0, 0);
    put16le(desc + 8, (uint16_t)(((w - 1) << 6) | ((h - 1) << 11)));

    put16le(buf + column_base + 0, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x8c00); /* dir=S, dbActuator, index 0 */

    put16le(buf + door_base, 0xfffe);
    door_bits = (uint16_t)((1u << 5) | 1u);
    put16le(buf + door_base + 2, door_bits);

    put16le(buf + actuator_base, 0x0000); /* actuator.next -> dbDoor */
    put16le(buf + actuator_base + 2, 0x0000);
    put16le(buf + actuator_base + 4, (uint16_t)(3u << 12));
    put16le(buf + actuator_base + 6, 0x0000);

    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
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

static void test_shifted_first_map_offset_rejected(void)
{
    uint8_t dat[DM2_TEST_TILE_DATA_START + 12];
    DM2_V1_DungeonData dungeon;
    size_t size = build_first_map_fixture(dat, sizeof(dat));

    CHECK(size == sizeof(dat), "offset fixture starts complete");
    put16le(dat + DM2_TEST_HEADER_SIZE, 2);
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == -1,
          "loader rejects map-0 raw offset that overruns tile data");
    CHECK(dungeon.raw_data == NULL,
          "rejected map-0 offset does not retain raw dungeon data");
}

static void test_skproject_layout_first_thing_and_door_record(void)
{
    uint8_t dat[128];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_layout_fixture(dat, sizeof(dat));
    int thing;
    int type = -1;
    int index = -1;
    int record_size = 0;
    const uint8_t *record;

    CHECK(size > 0, "skproject layout fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts skproject byte-square layout");
    CHECK(dungeon.square_bytes == 1,
          "skproject layout uses byte-sized map squares");
    CHECK(dungeon.level_widths[0] == 2 && dungeon.level_heights[0] == 2,
          "skproject map dimensions come from Map_definitions.w8");
    CHECK(dm2_v1_dungeon_get_square_type(&dungeon, 0, 1, 0) == 4,
          "byte-square door type is read from the high three bits");
    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 1, 0) == 0x90,
          "byte-square raw tile preserves the thing-list flag");
    thing = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0);
    CHECK(thing == 0x0000,
          "skproject first-thing lookup returns the door ObjectID");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, (uint16_t)thing) == 0xfffe,
          "skproject ObjectID chain reads the record w0 next link");
    CHECK(dm2_v1_dungeon_find_thing_of_type(
              &dungeon, (uint16_t)thing, 0, 8) == 0x0000,
          "skproject ObjectID chain search finds the DB0 door record");
    CHECK(dm2_v1_dungeon_find_thing_of_type(
              &dungeon, (uint16_t)thing, 3, 8) == -1,
          "skproject ObjectID chain search stops at the end marker");
    record = dm2_v1_dungeon_get_thing_record(
        &dungeon, (uint16_t)thing, &type, &index, &record_size);
    CHECK(record != NULL && type == 0 && index == 0 && record_size == 4,
          "door ObjectID resolves to DB0 record 0");
    CHECK(record != NULL &&
              (((uint16_t)record[2] | ((uint16_t)record[3] << 8)) &
               ((1u << 6) | (1u << 11) | (1u << 5) | 1u)) ==
                  ((1u << 6) | (1u << 11) | (1u << 5) | 1u),
          "door record exposes button, pushed state, opening dir and type bits");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_chained_first_thing_finds_door_record(void)
{
    uint8_t dat[160];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_chained_door_fixture(dat, sizeof(dat));
    int thing;

    CHECK(size > 0, "skproject chained door fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts a skproject tile chain with text before door");
    thing = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0);
    CHECK(thing == 0x0800,
          "skproject chained tile starts with DB2 text ObjectID");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, (uint16_t)thing) == 0x0000,
          "skproject chained tile text record links to DB0 door");
    CHECK(dm2_v1_dungeon_find_thing_of_type(
              &dungeon, (uint16_t)thing, 0, 8) == 0x0000,
          "skproject chained tile search finds the door after text");
    CHECK(dm2_v1_dungeon_find_thing_of_type(
              &dungeon, (uint16_t)thing, 0, 1) == -1,
          "skproject chained tile search respects the bounded step limit");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_text_wall_gfx_metadata(void)
{
    uint8_t dat[160];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_text_wall_gfx_fixture(dat, sizeof(dat));
    int thing;
    int index = -1;
    int field = -1;

    CHECK(size > 0, "skproject text wall-gfx fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts a skproject text wall-gfx chain");
    thing = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0);
    CHECK(thing == 0x8800,
          "text wall-gfx fixture carries direction on the DB2 ObjectID");
    CHECK(dm2_v1_dungeon_find_text_wall_gfx(
              &dungeon, (uint16_t)thing, 0, 2, 8, &index, &field) == 0 &&
              index == 0x2a && field == 1,
          "text wall-gfx helper exposes skproject tfoi[2] index and field");
    CHECK(dm2_v1_dungeon_find_text_wall_gfx(
              &dungeon, (uint16_t)thing, 1, 2, 8, &index, &field) == -1,
          "text wall-gfx helper requires the requested relative side");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_actuator_wall_gfx_ordinal(void)
{
    uint8_t dat[176];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_actuator_wall_gfx_fixture(dat, sizeof(dat));
    int thing;
    int ordinal = -1;
    int index = -1;
    int field = -1;
    static const uint8_t wall_gfx_list[4] = { 0x10, 0x20, 0x2a, 0x30 };

    CHECK(size > 0, "skproject actuator wall-gfx fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts a skproject actuator wall-gfx chain");
    thing = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0);
    CHECK(thing == 0x8c00,
          "actuator wall-gfx fixture carries direction on the DB3 ObjectID");
    CHECK(dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
              &dungeon, (uint16_t)thing, 0, 2, 8, &ordinal) == 0 &&
              ordinal == 3,
          "actuator wall-gfx helper exposes skproject GraphicNumber ordinal");
    CHECK(dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
              &dungeon, (uint16_t)thing, 1, 2, 8, &ordinal) == -1,
          "actuator wall-gfx helper requires the requested relative side");
    CHECK(dm2_v1_dungeon_resolve_actuator_wall_gfx(
              &dungeon, (uint16_t)thing, 0, 2, 8,
              wall_gfx_list, 4, &index, &field) == 0 &&
              index == 0x2a && field == 1,
          "actuator wall-gfx resolver maps one-based ordinal through map list");
    CHECK(dm2_v1_dungeon_resolve_actuator_wall_gfx(
              &dungeon, (uint16_t)thing, 0, 2, 8,
              wall_gfx_list, 2, &index, &field) == -1,
          "actuator wall-gfx resolver rejects missing map-list ordinal");

    dm2_v1_dungeon_free(&dungeon);
}

int main(void)
{
    printf("=== DM2 V1 Dungeon Loader First-Map Gate ===\n\n");

    test_first_map_metadata_and_tiles();
    test_truncated_first_map_rejected();
    test_shifted_first_map_offset_rejected();
    test_skproject_layout_first_thing_and_door_record();
    test_skproject_chained_first_thing_finds_door_record();
    test_skproject_text_wall_gfx_metadata();
    test_skproject_actuator_wall_gfx_ordinal();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
