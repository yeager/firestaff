/*
 * PC G1 DB4 CreatureType to GDAT map-chip graph gate.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_map.cpp resolves the tile record list through
 *   c_record.cpp. DME.h Creature::CreatureType selects CREATURES/type/F9 in
 *   QUERY_DUNGEON_MAP_CHIP_PICT. This production boundary must not fetch GDAT
 *   for a DB4 root until the PC G1 record graph is complete.
 */

#include "dm2_v1_dungeon_loader.h"

#include <stdint.h>
#include <stdio.h>
#include <string.h>

typedef struct {
    int raw_calls;
    int metadata_calls;
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

static int read_raw(void *userdata, int data_type, int category,
                    int entry, int field, const uint8_t **out_data,
                    uint32_t *out_size)
{
    static const uint8_t map_chip[] = { 0x5au };
    Calls *calls = (Calls *)userdata;

    ++calls->raw_calls;
    if (data_type != 1 || category != 0x0f || entry != 7 || field != 0xf9)
        return 0;
    *out_data = map_chip;
    *out_size = (uint32_t)sizeof(map_chip);
    return 1;
}

static int read_metadata(void *userdata, int category, int entry, int field,
                         int *out_width, int *out_height, int *out_format)
{
    Calls *calls = (Calls *)userdata;

    ++calls->metadata_calls;
    if (category != 0x0f || entry != 7 || field != 0xf9)
        return 0;
    *out_width = 1;
    *out_height = 1;
    *out_format = 8;
    return 1;
}

int main(void)
{
    uint8_t raw[21];
    DM2_V1_DungeonData dungeon;
    DM2_V1_G1CreatureMapChipRuntimeReceipt receipt;
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
    dungeon.thing_data_bases[4] = 4;
    dungeon.thing_type_counts[4] = 1;
    dungeon.raw_map_data_base = 20;

    put16le(raw + 2, 0x1000u);
    put16le(raw + 4, 0xfffeu);
    raw[8] = 7; /* DB4 Creature::CreatureType */
    raw[20] = 0x10; /* one thing-bearing source tile */

    memset(&calls, 0, sizeof(calls));
    memset(&receipt, 0xa5, sizeof(receipt));
    CHECK(dm2_v1_dungeon_materialize_g1_creature_map_chip_runtime(
              &dungeon, 0, read_raw, read_metadata, &calls, &receipt) == 0,
          "incomplete G1 graph rejects DB4-to-GDAT map-chip materialization");
    CHECK(calls.raw_calls == 0 && calls.metadata_calls == 0,
          "incomplete G1 graph does not issue any GDAT material request");
    CHECK(receipt.valid == 0 && receipt.material_count == 0,
          "rejected DB4 materialization leaves an invalid receipt");

    dungeon.record_graph_complete = 1;
    CHECK(dm2_v1_dungeon_materialize_g1_creature_map_chip_runtime(
              &dungeon, 0, read_raw, read_metadata, &calls, &receipt) == 1 &&
              receipt.valid && receipt.material_count == 1,
          "complete G1 graph materializes the source-owned DB4 map-chip receipt");
    CHECK(calls.raw_calls == 1 && calls.metadata_calls == 1 &&
              receipt.materials[0].creature_type == 7 &&
              receipt.materials[0].raw_byte_count == 1,
          "complete graph retains the verified F9 request and provenance");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
