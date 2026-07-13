/*
 * PC G1 c_record list-to-HUD material gate.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_map.cpp obtains a tile's first ObjectID, then walks
 *   it through c_record.cpp DM2_GET_NEXT_RECORD_LINK. skdungn.cpp consumes
 *   DB2 TextMode wall-GFX and DB3 Actuator::GraphicNumber only through that
 *   record-list route.
 *
 * No pixels, GDAT payloads, or synthetic renderer are involved here. The
 * data-free record bytes prove generic HUD material selection stays closed
 * until the byte-square G1 map-to-record graph is complete.
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { \
        ++passed; \
        printf("  PASS: %s\n", message); \
    } else { \
        ++failed; \
        printf("  FAIL: %s\n", message); \
    } \
} while (0)

static void put16le(uint8_t *p, uint16_t value)
{
    p[0] = (uint8_t)(value & 0xffu);
    p[1] = (uint8_t)(value >> 8);
}

int main(void)
{
    uint8_t raw[12];
    DM2_V1_DungeonData dungeon;
    int wall_index = 55;
    int wall_field = 66;
    int ordinal = 77;

    memset(raw, 0, sizeof(raw));
    memset(&dungeon, 0, sizeof(dungeon));
    for (int db = 0; db < 16; ++db)
        dungeon.thing_data_bases[db] = -1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_bytes = 1;
    dungeon.g1_extension_size = 1;
    dungeon.thing_data_bases[2] = 0;
    dungeon.thing_type_counts[2] = 1;
    dungeon.thing_data_bases[3] = 4;
    dungeon.thing_type_counts[3] = 1;

    put16le(raw + 0, 0xfffeu);
    put16le(raw + 2, 0x0153u); /* visible DB2 TextMode=1, ornate=0x2a */
    put16le(raw + 4, 0xfffeu);
    put16le(raw + 8, 0x1000u); /* DB3 GraphicNumber() ordinal 1 */

    CHECK(dm2_v1_dungeon_find_text_wall_gfx(
              &dungeon, 0x8800u, 0, 2, 8, &wall_index, &wall_field) == -1,
          "incomplete G1 graph cannot promote a direct DB2 wall-GFX record");
    CHECK(wall_index == -1 && wall_field == -1,
          "blocked DB2 wall-GFX leaves HUD material outputs invalid");
    CHECK(dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
              &dungeon, 0x8c00u, 0, 2, 8, &ordinal) == -1,
          "incomplete G1 graph cannot promote a direct DB3 actuator record");
    CHECK(ordinal == -1,
          "blocked DB3 actuator leaves the wall-GFX ordinal invalid");

    dungeon.record_graph_complete = 1;
    CHECK(dm2_v1_dungeon_find_text_wall_gfx(
              &dungeon, 0x8800u, 0, 2, 8, &wall_index, &wall_field) == 0 &&
              wall_index == 0x2a && wall_field == 1,
          "complete G1 graph preserves source-shaped DB2 wall-GFX selection");
    CHECK(dm2_v1_dungeon_find_actuator_wall_gfx_ordinal(
              &dungeon, 0x8c00u, 0, 2, 8, &ordinal) == 0 && ordinal == 1,
          "complete G1 graph preserves source-shaped DB3 graphic ordinal");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
