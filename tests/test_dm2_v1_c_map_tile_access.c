/*
 * Source-locked c_map tile accessor gate.
 *
 * Covers skproject/SKULLWIN/c_map.cpp:
 *   DM2_GET_ADDRESS_OF_TILE_RECORD
 *   DM2_IS_TILE_PASSAGE
 *   DM2_GET_TILE_VALUE
 *
 * This is a bounded map/table/address test. It does not walk GenericRecord::w0
 * and does not promote any GDAT, HUD, save, or renderer fallback.
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

static void put16le(uint8_t *p, uint16_t v)
{
    p[0] = (uint8_t)(v & 0xffu);
    p[1] = (uint8_t)(v >> 8);
}

static size_t build_fixture(uint8_t *buf, size_t cap)
{
    const size_t header_size = 44u;
    const size_t map_desc_size = 16u;
    const size_t column_base = header_size + map_desc_size;
    const size_t stack_base = column_base + 4u;
    const size_t db0_base = stack_base + 6u;
    const size_t raw_map_base = db0_base + 12u;
    uint8_t *desc;

    if (cap < raw_map_base + 6u) return 0;
    memset(buf, 0, cap);
    buf[4] = 1;
    put16le(buf + 10, 3);
    put16le(buf + 12, 3);

    desc = buf + header_size;
    put16le(desc + 0, 0);
    put16le(desc + 8, (uint16_t)(((2u - 1u) << 6) | ((3u - 1u) << 11)));

    put16le(buf + column_base, 0);
    put16le(buf + column_base + 2u, 1);
    put16le(buf + stack_base, 0x0000);
    put16le(buf + stack_base + 2u, 0x0001);
    put16le(buf + stack_base + 4u, 0x0002);
    put16le(buf + db0_base, 0xfffe);
    put16le(buf + db0_base + 4u, 0xfffe);
    put16le(buf + db0_base + 8u, 0xfffe);

    buf[raw_map_base + 0u] = 0x20;
    buf[raw_map_base + 1u] = 0x00;
    buf[raw_map_base + 2u] = 0xe0;
    buf[raw_map_base + 3u] = 0x90;
    buf[raw_map_base + 4u] = 0x20;
    buf[raw_map_base + 5u] = 0x10;
    return raw_map_base + 6u;
}

int main(void)
{
    uint8_t raw[96];
    DM2_V1_DungeonData dungeon;
    DM2_V1_SkprojectTileValueReceipt value_receipt;
    DM2_V1_SkprojectTilePassageReceipt passage_receipt;
    DM2_V1_SkprojectTileRecordAddressReceipt address_receipt;
    DM2_V1_SkprojectObjectIndexReceipt index_receipt;
    DM2_V1_SkprojectObjectIndexReceipt object_index_receipt;
    DM2_V1_SkprojectChangeCurrentMapReceipt change_map_receipt;
    uint16_t object_id = 0xffffu;
    int record_offset = -1;
    const uint8_t *record;
    size_t size = build_fixture(raw, sizeof(raw));

    CHECK(size == 88u, "synthetic c_map fixture is complete");
    CHECK(dm2_v1_dungeon_load(&dungeon, raw, (int)size) == 0,
          "skproject byte-map layout loads");
    CHECK(dm2_v1_dungeon_c_map_get_tile_value(&dungeon, 0, 0, 0) == 0x20,
          "GET_TILE_VALUE returns in-bounds raw tile byte");
    CHECK(dm2_v1_skproject_get_tile_value(&dungeon, 0, 0, 0,
                                          &value_receipt) &&
              value_receipt.valid &&
              value_receipt.raw_tile == 0x20 &&
              value_receipt.tile_value == 1 &&
              strcmp(value_receipt.source_symbol,
                     "DM2_GET_TILE_VALUE") == 0,
          "GET_TILE_VALUE receipt retains source symbol and tile class");
    CHECK(dm2_v1_dungeon_c_map_is_tile_passage(&dungeon, 0, 0, 0) == 1,
          "IS_TILE_PASSAGE accepts nonzero, non-seven tile classes");
    CHECK(dm2_v1_skproject_is_tile_passage(&dungeon, 0, 0, 0,
                                           &passage_receipt) &&
              passage_receipt.valid && passage_receipt.is_passage == 1 &&
              strcmp(passage_receipt.source_symbol,
                     "DM2_IS_TILE_PASSAGE") == 0,
          "IS_TILE_PASSAGE receipt admits the source floor class");
    CHECK(dm2_v1_dungeon_c_map_is_tile_passage(&dungeon, 0, 0, 1) == 0,
          "IS_TILE_PASSAGE rejects wall class zero");
    CHECK(dm2_v1_dungeon_c_map_is_tile_passage(&dungeon, 0, 0, 2) == 0,
          "IS_TILE_PASSAGE rejects tile class seven");
    CHECK(dm2_v1_skproject_get_object_index_from_tile(
              &dungeon, 0, 1, 0, &object_index_receipt) &&
              object_index_receipt.valid &&
              object_index_receipt.object_index == 1 &&
              object_index_receipt.column_base_index == 1 &&
              object_index_receipt.preceding_root_count == 0 &&
              strcmp(object_index_receipt.source_symbol,
                     "DM2_GET_OBJECT_INDEX_FROM_TILE") == 0,
          "GET_OBJECT_INDEX_FROM_TILE returns the column base root index");
    CHECK(dm2_v1_skproject_get_object_index_from_tile(
              &dungeon, 0, 1, 2, &object_index_receipt) &&
              object_index_receipt.valid &&
              object_index_receipt.object_index == 2 &&
              object_index_receipt.column_base_index == 1 &&
              object_index_receipt.preceding_root_count == 1,
          "GET_OBJECT_INDEX_FROM_TILE counts earlier root-marked rows");
    CHECK(!dm2_v1_skproject_get_object_index_from_tile(
              &dungeon, 0, 0, 0, &object_index_receipt) &&
              object_index_receipt.blocked_no_tile_record_link,
          "GET_OBJECT_INDEX_FROM_TILE rejects tiles without the 0x10 root bit");

    record = dm2_v1_dungeon_c_map_get_address_of_tile_record(
        &dungeon, 0, 1, 0, &object_id, &record_offset);
    CHECK(record == dungeon.raw_data + 74 && object_id == 0x0001u &&
          record_offset == 74,
          "GET_ADDRESS_OF_TILE_RECORD selects the c_map ground-stack root");
    CHECK(dm2_v1_skproject_get_address_of_tile_record(
              &dungeon, 0, 1, 0, &address_receipt) &&
              address_receipt.valid &&
              address_receipt.object_id == 0x0001u &&
              address_receipt.type == 0 &&
              address_receipt.index == 1 &&
              address_receipt.record_size == 4 &&
              strcmp(address_receipt.source_symbol,
                     "DM2_GET_ADDRESS_OF_TILE_RECORD") == 0,
          "GET_ADDRESS_OF_TILE_RECORD receipt exposes only the root address");
    CHECK(dm2_v1_skproject_get_object_index_from_tile(
              &dungeon, 0, 1, 0, &index_receipt) &&
              index_receipt.valid &&
              index_receipt.object_index == 1 &&
              index_receipt.column_base_index == 1 &&
              index_receipt.preceding_root_count == 0 &&
              index_receipt.column_index_offset == 62 &&
              index_receipt.object_index_offset == 66 &&
              index_receipt.object_id == 0x0001u &&
              strcmp(index_receipt.source_symbol,
                     "DM2_GET_OBJECT_INDEX_FROM_TILE") == 0,
          "GET_OBJECT_INDEX_FROM_TILE receipt exposes the c_map stack index");
    CHECK(dm2_v1_dungeon_c_map_get_address_of_tile_record(
              &dungeon, 0, 0, 0, &object_id, &record_offset) == NULL,
          "GET_ADDRESS_OF_TILE_RECORD rejects tiles without the 0x10 root bit");
    CHECK(!dm2_v1_skproject_get_object_index_from_tile(
              &dungeon, 0, 0, 0, &index_receipt) &&
              index_receipt.blocked_no_tile_record_link,
          "GET_OBJECT_INDEX_FROM_TILE fails closed without the root bit");
    CHECK(!dm2_v1_skproject_get_address_of_tile_record(
              &dungeon, 0, 0, 0, &address_receipt) &&
              address_receipt.blocked_no_tile_record_link,
          "GET_ADDRESS_OF_TILE_RECORD receipt fails closed without a root");
    CHECK(dm2_v1_dungeon_c_map_get_tile_value(&dungeon, 0, -1, 0) == 0x04,
          "GET_TILE_VALUE preserves west-edge passage sentinel");
    CHECK(dm2_v1_dungeon_c_map_get_tile_value(&dungeon, 0, 2, 0) == 0x01,
          "GET_TILE_VALUE preserves east-edge passage sentinel");
    CHECK(dm2_v1_dungeon_c_map_get_tile_value(&dungeon, 0, 0, -1) == 0x02,
          "GET_TILE_VALUE preserves north-edge passage sentinel");
    CHECK(dm2_v1_dungeon_c_map_get_tile_value(&dungeon, 0, 0, 3) == 0xe0,
          "GET_TILE_VALUE keeps blocked south edge unavailable");
    CHECK(dm2_v1_dungeon_c_map_get_tile_value(&dungeon, 0, -2, 0) == 0xe0,
          "GET_TILE_VALUE rejects far out-of-range coordinates");
    CHECK(dm2_v1_skproject_change_current_map_to(
              &dungeon, 0, 0, 4, 5, 0, 2, &change_map_receipt) &&
              change_map_receipt.valid &&
              change_map_receipt.unchanged &&
              change_map_receipt.current_map == 0,
          "CHANGE_CURRENT_MAP_TO keeps same-map requests unchanged");
    CHECK(dm2_v1_skproject_change_current_map_to(
              &dungeon, 0, -1, 4, 5, 0, 2, &change_map_receipt) == 0 &&
              change_map_receipt.blocked_negative_map,
          "CHANGE_CURRENT_MAP_TO rejects negative map index");
    CHECK(dm2_v1_skproject_change_current_map_to(
              &dungeon, 0, 1, 4, 5, 0, 2, &change_map_receipt) == 0 &&
              change_map_receipt.blocked_map_range,
          "CHANGE_CURRENT_MAP_TO rejects out-of-range map index");

    dm2_v1_dungeon_free(&dungeon);
    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
