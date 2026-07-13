/*
 * PC G1 DB3 Actuator::GraphicNumber to WALL_GFX graph gate.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_map.cpp selects the tile record root and c_record.cpp
 *   addresses DB3. DME.h Actuator::GraphicNumber maps through
 *   Map_definitions::WallGraphics before DRAW_WALL_ORNATE reads its GDAT
 *   scalar fields.
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int calls;
} Calls;

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

static int read_scalar(void *userdata, int data_type, int category,
                       int entry, int field, uint16_t *out_value)
{
    Calls *calls = (Calls *)userdata;

    ++calls->calls;
    if (category != 9 || entry != 0x2a)
        return 0;
    if (data_type == 0x0b && field == 4) *out_value = 9;
    else if (data_type == 0x0b && field == 5) *out_value = 3;
    else if (data_type == 0x0b && field == 7) *out_value = 1;
    else if (data_type == 0x0b && field == 0x0a) *out_value = 2;
    else if (data_type == 0x0c && field == 0xfd) *out_value = 0xfe02;
    else return 0;
    return 1;
}

int main(void)
{
    uint8_t raw[66];
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1ActuatorWallGfxRuntimeReceipt receipt;
    Calls calls;

    memset(raw, 0, sizeof(raw));
    memset(&dungeon, 0, sizeof(dungeon));
    for (int db = 0; db < 16; ++db)
        dungeon.thing_data_bases[db] = -1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_bytes = 1;
    dungeon.g1_extension_size = 1;
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 1;
    dungeon.level_heights[0] = 1;
    dungeon.column_index_base = 0;
    dungeon.square_first_thing_base = 2;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[3] = 4;
    dungeon.thing_type_counts[3] = 1;
    dungeon.raw_map_data_base = 64;

    put16le(raw + 2, 0x8c00u);
    put16le(raw + 4, 0xfffeu);
    put16le(raw + 8, 0x1000u); /* DB3 GraphicNumber ordinal 1 */
    put16le(raw + 54, 1u); /* map-0 WallGraphics count */
    raw[64] = 0x10; /* one thing-bearing source tile */
    raw[65] = 0x2a; /* map-local WALL_GFX[0] */

    memset(&calls, 0, sizeof(calls));
    memset(&receipt, 0xa5, sizeof(receipt));
    CHECK(dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_runtime(
              &dungeon, 0, read_scalar, &calls, &receipt) == 0,
          "incomplete G1 graph rejects DB3-to-WALL_GFX materialization");
    CHECK(calls.calls == 0,
          "incomplete G1 graph does not issue WALL_GFX scalar requests");
    CHECK(receipt.valid == 0 && receipt.material_count == 0,
          "rejected DB3 materialization leaves an invalid receipt");

    dungeon.record_graph_complete = 1;
    CHECK(dm2_v1_dungeon_materialize_g1_actuator_wall_gfx_runtime(
              &dungeon, 0, read_scalar, &calls, &receipt) == 1 &&
              receipt.valid && receipt.material_count == 1,
          "complete G1 graph materializes the source-owned DB3 WALL_GFX receipt");
    CHECK(calls.calls == 5 &&
              receipt.materials[0].wall_gfx_index == 0x2a &&
              receipt.materials[0].graphic_ordinal == 1,
          "complete graph retains map-list ordinal and all five GDAT scalars");
    CHECK(receipt.materials[0].colorkey == 9 &&
              receipt.materials[0].position == 3 &&
              receipt.materials[0].image_offset == 0xfe02,
          "complete graph preserves source scalar ownership in the receipt");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
