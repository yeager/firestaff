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

typedef struct {
    int count;
    uint16_t things[4];
    int types[4];
    int indexes[4];
    int levels[4];
    int xs[4];
    int ys[4];
    uint16_t w0s[4];
} WalkCapture;

static int capture_walk_thing(void *user,
                              uint16_t thing,
                              int type,
                              int index,
                              const uint8_t *record,
                              int record_size,
                              int level,
                              int x,
                              int y)
{
    WalkCapture *capture = (WalkCapture *)user;
    if (!capture || !record || record_size < 2 ||
        capture->count >= (int)(sizeof(capture->things) /
                                sizeof(capture->things[0]))) {
        return -1;
    }
    capture->things[capture->count] = thing;
    capture->types[capture->count] = type;
    capture->indexes[capture->count] = index;
    capture->levels[capture->count] = level;
    capture->xs[capture->count] = x;
    capture->ys[capture->count] = y;
    capture->w0s[capture->count] =
        (uint16_t)record[0] | ((uint16_t)record[1] << 8);
    capture->count++;
    return 0;
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

static size_t build_skproject_append_existing_chain_fixture(uint8_t *buf,
                                                            size_t cap)
{
    const int w = 2;
    const int h = 2;
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + (size_t)w * 2u;
    const size_t thing_base = sft_base + 2u;
    const size_t door0_base = thing_base;
    const size_t door1_base = door0_base + 4u;
    const size_t text_base = door1_base + 4u;
    const size_t raw_map_base = text_base + 4u;
    uint8_t *desc;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);

    buf[4] = 1;
    put16le(buf + 10, 1);
    put16le(buf + 12, 2);
    put16le(buf + 16, 1);

    desc = buf + header_size;
    put16le(desc + 0, 0);
    put16le(desc + 8, (uint16_t)(((w - 1) << 6) | ((h - 1) << 11)));

    put16le(buf + column_base + 0, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0800);

    put16le(buf + door0_base, 0xfffe);
    put16le(buf + door0_base + 2, 0x0801);
    put16le(buf + door1_base, 0x0000);
    put16le(buf + door1_base + 2, 0x0802);
    put16le(buf + text_base, 0x0000);
    put16le(buf + text_base + 2, 0x0000);

    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static size_t build_skproject_append_empty_tile_fixture(uint8_t *buf,
                                                        size_t cap)
{
    const int w = 2;
    const int h = 2;
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t column_base = header_size + map_desc_size;
    const size_t sft_base = column_base + (size_t)w * 2u;
    const size_t thing_base = sft_base + 4u;
    const size_t door0_base = thing_base;
    const size_t door1_base = door0_base + 4u;
    const size_t raw_map_base = door1_base + 4u;
    uint8_t *desc;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);

    buf[4] = 1;
    put16le(buf + 10, 2);
    put16le(buf + 12, 2);

    desc = buf + header_size;
    put16le(desc + 0, 0);
    put16le(desc + 8, (uint16_t)(((w - 1) << 6) | ((h - 1) << 11)));

    put16le(buf + column_base + 0, 0);
    put16le(buf + column_base + 2, 0);
    put16le(buf + sft_base, 0x0000);
    put16le(buf + sft_base + 2, 0xffff);

    put16le(buf + door0_base, 0xfffe);
    put16le(buf + door0_base + 2, 0x0801);
    put16le(buf + door1_base, 0x0000);
    put16le(buf + door1_base + 2, 0x0802);

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

static size_t build_skproject_map_wall_gfx_list_fixture(uint8_t *buf,
                                                       size_t cap)
{
    size_t size = build_skproject_actuator_wall_gfx_fixture(buf, cap);
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t raw_map_base = header_size + map_desc_size +
                                4u + 2u + 4u + 8u;
    uint8_t *desc;

    if (size == 0 || cap < raw_map_base + 9u) return 0;
    desc = buf + header_size;
    put16le(desc + 10, 4); /* Map_definitions::WallGraphics() */
    put16le(desc + 12, (uint16_t)(1u << 4)); /* CreaturesTypes() */
    buf[raw_map_base + 4u] = 0x7e; /* creature id list entry */
    buf[raw_map_base + 5u] = 0x10;
    buf[raw_map_base + 6u] = 0x20;
    buf[raw_map_base + 7u] = 0x2a;
    buf[raw_map_base + 8u] = 0x30;
    return raw_map_base + 9u;
}

static size_t build_skproject_map_floor_gfx_list_fixture(uint8_t *buf,
                                                         size_t cap)
{
    size_t size = build_skproject_map_wall_gfx_list_fixture(buf, cap);
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t raw_map_base = header_size + map_desc_size +
                                4u + 2u + 4u + 8u;
    uint8_t *desc;

    if (size == 0 || cap < raw_map_base + 11u) return 0;
    desc = buf + header_size;
    /* DME.h Map_definitions::FloorGraphics is w10 bits 8..11. */
    put16le(desc + 10, (uint16_t)(4u | (2u << 8)));
    buf[raw_map_base + 9u] = 0x41;
    buf[raw_map_base + 10u] = 0x57;
    return raw_map_base + 11u;
}

static size_t build_pc_g1_record_evidence_fixture(uint8_t *buf, size_t cap)
{
    const size_t header_size = 44;
    const size_t map_desc_size = 16;
    const size_t extension_size = 256;
    const size_t column_base = header_size + map_desc_size + extension_size;
    const size_t sft_base = column_base + 4u;
    const size_t text_base = sft_base + 2u;
    const size_t pool_base = text_base + 2u;
    const size_t extension_base = pool_base + 8u;
    const size_t raw_map_base = extension_base + 4u;
    uint8_t *desc;

    if (cap < raw_map_base + 4u) return 0;
    memset(buf, 0, cap);
    put16le(buf + 2, 0x3147U);
    put16le(buf + 4, header_size);
    buf[6] = 1;
    put16le(buf + 8, 1);  /* shifted cwTextData */
    put16le(buf + 10, 1); /* shifted cwListSize */
    put16le(buf + 14, 2); /* dbDoor count */

    desc = buf + header_size;
    put16le(desc + 0, 0);
    put16le(desc + 8, (uint16_t)((1u << 6) | (1u << 11)));
    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2u, 0);
    put16le(buf + sft_base, 0x0000); /* DB0 root, index 0 */
    put16le(buf + pool_base, 0xfffe); /* candidate DB0 w0 end marker */
    put16le(buf + pool_base + 4u, 0xfffe); /* unreferenced DB0 record */
    buf[raw_map_base + 0] = 0x20;
    buf[raw_map_base + 1] = 0x20;
    buf[raw_map_base + 2] = 0x90;
    buf[raw_map_base + 3] = 0x20;
    return raw_map_base + 4u;
}

static size_t build_pc_g1_extension_record_fixture(uint8_t *buf, size_t cap)
{
    return build_pc_g1_record_evidence_fixture(buf, cap);
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
    WalkCapture capture;
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
    memset(&capture, 0, sizeof(capture));
    CHECK(dm2_v1_dungeon_walk_square_things(
              &dungeon, 0, 1, 0, 8, capture_walk_thing, &capture) == 2,
          "source-owned square-chain walker visits both linked records");
    CHECK(capture.count == 2 &&
              capture.things[0] == 0x0800 && capture.types[0] == 2 &&
              capture.indexes[0] == 0 && capture.w0s[0] == 0x0000 &&
              capture.things[1] == 0x0000 && capture.types[1] == 0 &&
              capture.indexes[1] == 0 && capture.w0s[1] == 0xfffe,
          "source-owned square-chain walker preserves ObjectID order and w0 links");
    CHECK(capture.levels[0] == 0 && capture.xs[0] == 1 &&
              capture.ys[0] == 0 && capture.levels[1] == 0 &&
              capture.xs[1] == 1 && capture.ys[1] == 0,
          "source-owned square-chain walker reports the owning square");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_append_record_to_existing_tile_chain(void)
{
    uint8_t dat[192];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectAppendRecordReceipt receipt;
    size_t size =
        build_skproject_append_existing_chain_fixture(dat, sizeof(dat));
    int thing;

    CHECK(size > 0, "append existing-chain fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts append existing-chain fixture");
    CHECK(dm2_v1_skproject_append_record_to(
              &dungeon, 0x0001, NULL, 0, 1, 0, &receipt) == 1,
          "APPEND_RECORD_TO appends to an existing tile chain");
    CHECK(receipt.valid && receipt.existing_tile_chain_route &&
              receipt.tail_object_id == 0x0000 &&
              receipt.appended_previous_next == 0x0000,
          "APPEND_RECORD_TO receipt names the existing-chain tail route");
    thing = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0);
    CHECK(thing == 0x0800,
          "APPEND_RECORD_TO preserves the tile root ObjectID");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0800) == 0x0000,
          "APPEND_RECORD_TO leaves the text node linked to the original door");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0000) == 0x0001,
          "APPEND_RECORD_TO links the old tail to the appended door");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0001) == 0xfffe,
          "APPEND_RECORD_TO resets appended record w0 to OBJECT_END_MARKER");
    CHECK(dm2_v1_skproject_append_record_to(
              &dungeon, 0xffff, NULL, 0, 1, 0, &receipt) == 0 &&
              receipt.blocked_null_or_end_append,
          "APPEND_RECORD_TO rejects OBJECT_NULL append input");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_append_record_to_empty_tile_inserts_root(void)
{
    uint8_t dat[192];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectAppendRecordReceipt receipt;
    size_t size = build_skproject_append_empty_tile_fixture(dat, sizeof(dat));

    CHECK(size > 0, "append empty-tile fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts append empty-tile fixture");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) ==
              -1,
          "empty-tile fixture starts without a root ObjectID");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0) == 0x0000,
          "empty-tile fixture starts with the second-column door root");
    CHECK(dm2_v1_skproject_append_record_to(
              &dungeon, 0x0001, NULL, 0, 0, 0, &receipt) == 1,
          "APPEND_RECORD_TO inserts a new root into an empty tile");
    CHECK(receipt.valid && receipt.empty_tile_insert_route &&
              receipt.object_index == 0 &&
              receipt.shifted_ground_stack_words == 1 &&
              receipt.incremented_column_offsets == 1,
          "APPEND_RECORD_TO receipt records ground-stack shift and column bump");
    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 0, 0) == 0x30,
          "APPEND_RECORD_TO sets tile bit 0x10 on the empty square");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 0, 0) == 0x0001,
          "APPEND_RECORD_TO installs appended ObjectID at the new root");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0) == 0x0000,
          "APPEND_RECORD_TO preserves later square root after column shift");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0001) == 0xfffe,
          "APPEND_RECORD_TO resets inserted record next link to end marker");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_cut_record_from_single_tile_root(void)
{
    uint8_t dat[128];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectCutRecordReceipt receipt;
    size_t size = build_skproject_layout_fixture(dat, sizeof(dat));

    CHECK(size > 0, "cut single-root fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts cut single-root fixture");
    CHECK(dm2_v1_skproject_cut_record_from(
              &dungeon, 0x0000, NULL, 0, 1, 0, &receipt) == 1,
          "CUT_RECORD_FROM removes a single tile root");
    CHECK(receipt.valid && receipt.tile_single_root_remove_route &&
              receipt.object_index == 0 &&
              receipt.shifted_ground_stack_words == 0 &&
              receipt.decremented_column_offsets == 0 &&
              receipt.source_reset_to_end,
          "CUT_RECORD_FROM receipt names the single-root compaction route");
    CHECK(receipt.source_symbol &&
              strcmp(receipt.source_symbol, "DM2_CUT_RECORD_FROM") == 0 &&
              receipt.source_line == 121,
          "CUT_RECORD_FROM receipt preserves the SKULLWIN c_record source row");
    CHECK(dm2_v1_dungeon_get_tile_raw(&dungeon, 0, 1, 0) == 0x80,
          "CUT_RECORD_FROM clears the tile record-list bit");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0) == -1,
          "CUT_RECORD_FROM leaves the square without a root ObjectID");
    CHECK(dm2_v1_skproject_cut_record_from(
              &dungeon, 0xffff, NULL, 0, 1, 0, &receipt) == 0 &&
              receipt.blocked_null_or_end_cut,
          "CUT_RECORD_FROM rejects OBJECT_NULL input");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_cut_record_from_tile_root_replaces_with_next(void)
{
    uint8_t dat[160];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectCutRecordReceipt receipt;
    size_t size = build_skproject_chained_door_fixture(dat, sizeof(dat));

    CHECK(size > 0, "cut root-replace fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts cut root-replace fixture");
    CHECK(dm2_v1_skproject_cut_record_from(
              &dungeon, 0x8800, NULL, 0, 1, 0, &receipt) == 1,
          "CUT_RECORD_FROM cuts a directed root by its masked ObjectID");
    CHECK(receipt.valid && receipt.tile_root_replace_route &&
              receipt.masked_object_id == 0x0800 &&
              receipt.tile_previous_root == 0x0800 &&
              receipt.tile_new_root == 0x0000 &&
              receipt.source_previous_next == 0x0000,
          "CUT_RECORD_FROM receipt records root replacement with source next");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0) == 0x0000,
          "CUT_RECORD_FROM installs the cut root's next link as square root");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0800) == 0xfffe,
          "CUT_RECORD_FROM resets the cut record next link to end marker");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_cut_record_from_tile_chain_unlinks_middle(void)
{
    uint8_t dat[160];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectCutRecordReceipt receipt;
    size_t size = build_skproject_chained_door_fixture(dat, sizeof(dat));

    CHECK(size > 0, "cut chain fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts cut chain fixture");
    CHECK(dm2_v1_skproject_cut_record_from(
              &dungeon, 0x0000, NULL, 0, 1, 0, &receipt) == 1,
          "CUT_RECORD_FROM unlinks a non-root tile-chain record");
    CHECK(receipt.valid && receipt.tile_chain_unlink_route &&
              receipt.preceding_object_id == 0x0800 &&
              receipt.tile_new_root == 0x0800 &&
              receipt.source_previous_next == 0xfffe,
          "CUT_RECORD_FROM receipt records the predecessor rewrite route");
    CHECK(dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0) == 0x0800,
          "CUT_RECORD_FROM preserves the square root for middle unlink");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0800) == 0xfffe,
          "CUT_RECORD_FROM rewires the predecessor to the cut record's next");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_cut_record_from_parent_link_route(void)
{
    uint8_t dat[160];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectCutRecordReceipt receipt;
    uint16_t parent = 0x0800;
    size_t size = build_skproject_chained_door_fixture(dat, sizeof(dat));

    CHECK(size > 0, "cut parent-link fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts cut parent-link fixture");
    CHECK(dm2_v1_skproject_cut_record_from(
              &dungeon, 0x0000, &parent, 0, -1, 0, &receipt) == 1,
          "CUT_RECORD_FROM follows the source parent-link route");
    CHECK(receipt.valid && receipt.parent_link_route &&
              receipt.parent_previous_link == 0x0800 &&
              receipt.parent_new_link == 0x0800 &&
              receipt.preceding_object_id == 0x0800,
          "CUT_RECORD_FROM receipt records parent predecessor unlink");
    CHECK(parent == 0x0800,
          "CUT_RECORD_FROM preserves the parent head when cutting a child");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0800) == 0xfffe,
          "CUT_RECORD_FROM rewires the parent-owned chain");

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

static void test_skproject_map_wall_gfx_list(void)
{
    uint8_t dat[192];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_map_wall_gfx_list_fixture(dat, sizeof(dat));
    uint8_t list[4] = { 0 };
    int thing;
    int index = -1;
    int field = -1;
    int count;

    CHECK(size > 0, "skproject map wall-gfx list fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts a skproject map-local wall-gfx list");
    count = dm2_v1_dungeon_get_map_wall_gfx_list(&dungeon, 0, list, 4);
    CHECK(count == 4 && list[0] == 0x10 && list[1] == 0x20 &&
          list[2] == 0x2a && list[3] == 0x30,
          "loader exposes wall-gfx list after map creature ids");
    thing = dm2_v1_dungeon_get_first_thing(&dungeon, 0, 1, 0);
    CHECK(dm2_v1_dungeon_resolve_actuator_wall_gfx(
              &dungeon, (uint16_t)thing, 0, 2, 8,
              list, count, &index, &field) == 0 &&
              index == 0x2a && field == 1,
          "loader-resolved map wall-gfx list drives actuator ordinal");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_map_floor_gfx_list(void)
{
    uint8_t dat[192];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_map_floor_gfx_list_fixture(dat, sizeof(dat));
    uint8_t list[2] = { 0 };
    int count;

    CHECK(size > 0, "skproject map floor-gfx list fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts a skproject map-local floor-gfx list");
    count = dm2_v1_dungeon_get_map_floor_gfx_list(&dungeon, 0, list, 2);
    CHECK(count == 2 && list[0] == 0x41 && list[1] == 0x57,
          "loader exposes FLOOR_GFX list after creature and wall lists");

    dm2_v1_dungeon_free(&dungeon);
}

static void test_skproject_map_door_type_flags(void)
{
    uint8_t dat[192];
    DM2_V1_DungeonData dungeon;
    size_t size = build_skproject_map_floor_gfx_list_fixture(dat, sizeof(dat));
    uint8_t *desc = dat + 44;

    /* DME.h Map_definitions::UseDoor0/1 reads w2 bits 7/8, while DoorType0/1
     * are w14 nibbles 8/12.  Slot one remains deliberately disabled. */
    put16le(desc + 2, (uint16_t)(1u << 7));
    put16le(desc + 14, (uint16_t)((3u << 8) | (11u << 12)));
    CHECK(size > 0 && dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts skproject map door header flags");
    CHECK(dungeon.map_use_door0[0] == 1 && dungeon.map_door_set0[0] == 3 &&
              dungeon.map_use_door1[0] == 0 && dungeon.map_door_set1[0] == 11,
          "loader retains UseDoor-gated DoorType0/1 independently");
    dm2_v1_dungeon_free(&dungeon);
}

static void test_pc_g1_record_pool_ownership_and_bounded_traversal(void)
{
    uint8_t dat[384];
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1RecordPoolEvidence evidence;
    size_t size = build_pc_g1_record_evidence_fixture(dat, sizeof(dat));

    CHECK(size > 0, "PC G1 record-evidence fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts bounded PC G1 record-evidence fixture");
    CHECK(dm2_v1_dungeon_collect_g1_record_pool_evidence(
              &dungeon, &evidence) == 1 && evidence.available == 1,
          "G1 evidence receipt is available from non-tail tables");
    CHECK(evidence.text_end == 324 && evidence.candidate_base == 324 &&
              evidence.candidate_end == 332 && evidence.candidate_bytes == 8,
          "G1 candidate span starts after the proven text table");
    CHECK(evidence.candidate_pool_bases[0] == 324 &&
              evidence.candidate_record_count == 2 &&
              evidence.candidate_first_link_end_markers == 2,
          "c_record DB0 provenance includes its terminating first link");
    CHECK(evidence.root_count == 1 && evidence.root_shape_valid == 1 &&
              evidence.root_shape_invalid == 0,
          "ground-stack roots resolve to the declared c_record shape");
    CHECK(evidence.tail_pool_base == 328 && evidence.tail_pool_base_rejected,
          "tail-aligned record placement is rejected by the text anchor");
    CHECK(dungeon.thing_data_bases[0] == 324 &&
              dungeon.record_graph_complete == 1 &&
              dm2_v1_dungeon_validate_record_graph(&dungeon) == 1 &&
              dm2_v1_dungeon_get_thing_record(&dungeon, 0x0000,
                                               NULL, NULL, NULL) != NULL,
          "source-ordered G1 c_record ownership enables bounded traversal");
    CHECK(dm2_v1_dungeon_walk_square_things(
              &dungeon, 0, 1, 0, 8, capture_walk_thing, &(WalkCapture){0}) == 1,
          "complete G1 graph can be walked through the loader-owned route");

    put16le(dat + 328, 0x0002); /* Unreachable DB0 index 2 exceeds count. */
    dm2_v1_dungeon_free(&dungeon);
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts malformed direct-link fixture for diagnostics");
    CHECK(dm2_v1_dungeon_collect_g1_record_pool_evidence(
              &dungeon, &evidence) == 1 &&
              evidence.candidate_first_link_shape_invalid == 1,
          "evidence reports an invalid unreachable direct link");
    CHECK(dungeon.record_graph_complete == 1 &&
              dm2_v1_dungeon_validate_record_pools(&dungeon) == 1 &&
              dm2_v1_dungeon_validate_record_graph(&dungeon) == 1,
          "unreachable pool words do not become inferred G1 links");
    dm2_v1_dungeon_free(&dungeon);

    put16le(dat + 324, 0x0002); /* w0 is game data in G1, not a link. */
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts fixture with arbitrary w0 game data");
    CHECK(dungeon.record_graph_complete == 1 &&
              dm2_v1_dungeon_get_next_thing(&dungeon, 0x0000) ==
                  (int)DM2_THING_END_MARKER,
          "G1 get_next_thing returns END_MARKER (w0 is game data, not link)");
    CHECK(dm2_v1_dungeon_walk_square_things(
              &dungeon, 0, 1, 0, 8, capture_walk_thing, &(WalkCapture){0}) == 1,
          "G1 walk visits root only (no w0 chain)");
    dm2_v1_dungeon_free(&dungeon);

    put16le(dat + 324, 0x0001); /* DB0[0] w0=0x0001 — game data, not link. */
    put16le(dat + 328, 0x0000); /* DB0[1] w0=0x0000 — game data, not link. */
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts fixture with w0 values that would form a cycle");
    CHECK(dungeon.record_graph_complete == 1 &&
              dm2_v1_dungeon_validate_record_graph(&dungeon) == 1,
          "G1 validator ignores w0 (game data, not chain links)");
    CHECK(dm2_v1_dungeon_walk_square_things(
              &dungeon, 0, 1, 0, 8, capture_walk_thing, &(WalkCapture){0}) == 1,
          "G1 walk visits root only even with cycle-shaped w0 values");
    dm2_v1_dungeon_free(&dungeon);
}

static __attribute__((unused)) void test_pc_g1_ground_stack_map_corpus_receipt(void)
{
    uint8_t dat[384];
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1GroundStackMapCorpusReceipt receipt;
    DM2_V1_G1MapCorpusReceipt map_receipt;
    size_t size = build_pc_g1_record_evidence_fixture(dat, sizeof(dat));

    CHECK(size > 0 && dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "PC G1 corpus-receipt fixture loads");
    CHECK(dm2_v1_dungeon_collect_g1_ground_stack_map_corpus_receipt(
              &dungeon, &receipt) == 1 && receipt.available == 1 &&
              receipt.g1_layout_absent == 0 && receipt.raw_only == 1,
          "G1 corpus receipt accepts only verified raw table bounds");
    CHECK(receipt.column_index_base == 316 &&
              receipt.column_index_word_count == 2 &&
              receipt.column_index_byte_count == 4u &&
              receipt.ground_stack_base == 320 &&
              receipt.ground_stack_word_count == 1 &&
              receipt.ground_stack_byte_count == 2u &&
              receipt.map_data_base == 336 && receipt.map_data_byte_count == 4u,
          "G1 corpus receipt retains column, ground-stack, and map byte ranges");
    CHECK(receipt.column_index_hash != 0u && receipt.ground_stack_hash != 0u &&
              receipt.map_data_hash != 0u &&
              receipt.column_index_semantics_unresolved == 1 &&
              receipt.ground_stack_semantics_unresolved == 1,
          "G1 corpus receipt hashes bytes without assigning c_map semantics");
    CHECK(dm2_v1_dungeon_collect_g1_map_corpus_receipt(
              &dungeon, &map_receipt) == 1 && map_receipt.available == 1 &&
              map_receipt.raw_only == 1 &&
              map_receipt.tile_semantics_unresolved == 1 &&
              map_receipt.map_count == 1 && map_receipt.map_data_base == 336 &&
              map_receipt.map_data_byte_count == 4u &&
              map_receipt.map_data_hash != 0u,
          "G1 map receipt retains the trailing map-data block as raw evidence");
    CHECK(map_receipt.maps[0].map == 0 &&
              map_receipt.maps[0].descriptor_base == 44 &&
              map_receipt.maps[0].map_data_offset == 0 &&
              map_receipt.maps[0].width == 2 && map_receipt.maps[0].height == 2 &&
              map_receipt.maps[0].map_byte_count == 4u &&
              map_receipt.maps[0].descriptor_hash != 0u &&
              map_receipt.maps[0].map_hash != 0u,
          "G1 map receipt hashes each descriptor-bounded raw map span");
    dm2_v1_dungeon_free(&dungeon);

    memset(&dungeon, 0, sizeof(dungeon));
    CHECK(dm2_v1_dungeon_collect_g1_ground_stack_map_corpus_receipt(
              &dungeon, &receipt) == 1 && receipt.available == 0 &&
              receipt.g1_layout_absent == 1,
          "missing G1 layout is reported as absence without fallback decoding");
    CHECK(dm2_v1_dungeon_collect_g1_map_corpus_receipt(
              &dungeon, &map_receipt) == 1 && map_receipt.available == 0 &&
              map_receipt.g1_layout_absent == 1,
          "missing G1 map layout is reported as absence without tile decoding");
}

static __attribute__((unused)) void test_pc_g1_extension_record_transform(void)
{
    uint8_t dat[14000];
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1RecordPoolEvidence evidence;
    size_t size = build_pc_g1_extension_record_fixture(dat, sizeof(dat));

    CHECK(size > 0, "PC G1 DB3/DB4 extension fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "loader accepts the exact G1 extension profile");
    CHECK(dungeon.g1_extension_record_bases[3] == dungeon.g1_extension_base &&
              dungeon.g1_extension_record_counts[3] == 725 &&
              dungeon.g1_extension_record_bases[4] ==
                  dungeon.g1_extension_base + 725 * 8 &&
              dungeon.g1_extension_record_counts[4] == 127,
          "G1 extension keeps DB3 stride through the ObjectID ceiling then DB4 stride");
    CHECK(dm2_v1_dungeon_get_thing_record(&dungeon, 0x0d2b,
                                           NULL, NULL, NULL) != NULL &&
              dm2_v1_dungeon_get_thing_record(&dungeon, 0x10ad,
                                               NULL, NULL, NULL) != NULL,
          "extended DB3 and DB4 map roots resolve by c_record stride");
    CHECK(dm2_v1_dungeon_collect_g1_record_pool_evidence(
              &dungeon, &evidence) == 1 &&
              evidence.map_root_extension_shape_valid == 2 &&
              evidence.root_shape_invalid == 0 &&
              dungeon.record_graph_complete == 1,
          "extension ownership does not invent a new ObjectID encoding");
    dm2_v1_dungeon_free(&dungeon);
}

static __attribute__((unused)) void test_pc_g1_partial_boot_is_transactional(void)
{
    uint8_t dat[14000];
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1PartialMapBootReceipt receipt;
    DM2_V1_G1FirstMapRuntimeReceipt first_map_receipt;
    size_t size = build_pc_g1_extension_record_fixture(dat, sizeof(dat));

    CHECK(size > 0 && dm2_v1_dungeon_load(&dungeon, dat, (int)size) == 0,
          "partial G1 boot fixture loads");
    memset(&receipt, 0, sizeof(receipt));
    receipt.committed = -12345;
    CHECK(dm2_v1_dungeon_materialize_g1_partial_map_boot(&dungeon, &receipt) == 0 &&
              receipt.committed == -12345,
          "partial boot leaves its receipt untouched when the corpus contract is incomplete");
    memset(&first_map_receipt, 0, sizeof(first_map_receipt));
    first_map_receipt.committed = -54321;
    CHECK(dm2_v1_dungeon_materialize_g1_first_map_runtime(
              &dungeon, &first_map_receipt) == 0 &&
              first_map_receipt.committed == -54321,
          "first-map runtime receipt leaves its output untouched without the full G1 contract");
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
    test_skproject_append_record_to_existing_tile_chain();
    test_skproject_append_record_to_empty_tile_inserts_root();
    test_skproject_cut_record_from_single_tile_root();
    test_skproject_cut_record_from_tile_root_replaces_with_next();
    test_skproject_cut_record_from_tile_chain_unlinks_middle();
    test_skproject_cut_record_from_parent_link_route();
    test_skproject_text_wall_gfx_metadata();
    test_skproject_actuator_wall_gfx_ordinal();
    test_skproject_map_wall_gfx_list();
    test_skproject_map_floor_gfx_list();
    test_skproject_map_door_type_flags();
    test_pc_g1_record_pool_ownership_and_bounded_traversal();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
