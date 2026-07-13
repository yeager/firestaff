/*
 * PC G1 c_record null-link gate.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_record.cpp DM2_GET_NEXT_RECORD_LINK lines 51-54
 *   exposes GenericRecord::w0, while DM2_APPEND_RECORD_TO and
 *   DM2_CUT_RECORD_FROM lines 66-68 and 126-127 reject OBJECT_NULL before
 *   record access.  OBJECT_END_MARKER is the documented chain terminator.
 *
 * The raw bytes below model only the malformed c_record boundary needed to
 * prove that OBJECT_NULL cannot flow from w0 into a dungeon/HUD consumer.
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
    uint8_t raw[8];
    DM2_V1_DungeonData dungeon;

    memset(raw, 0, sizeof(raw));
    memset(&dungeon, 0, sizeof(dungeon));
    for (int type = 0; type < 16; ++type)
        dungeon.thing_data_bases[type] = -1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.thing_data_bases[0] = 0;
    dungeon.thing_type_counts[0] = 2;

    put16le(raw, 0xffffu);
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0000u) == -1,
          "OBJECT_NULL w0 is rejected before it can select DB15/index-1023");
    CHECK(dm2_v1_dungeon_find_thing_of_type(
              &dungeon, 0x0000u, 15, 8) == -1,
          "OBJECT_NULL w0 cannot reach a later record-type consumer");

    put16le(raw, 0xfffeu);
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0000u) == 0xfffe,
          "OBJECT_END_MARKER remains the source-defined chain terminator");
    CHECK(dm2_v1_dungeon_find_thing_of_type(
              &dungeon, 0x0000u, 15, 8) == -1,
          "terminated chain remains unavailable to a nonmatching consumer");

    put16le(raw, 0x0001u);
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0000u) == 0x0001,
          "bounded DB0 link still preserves a valid next ObjectID");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
