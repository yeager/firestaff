/*
 * PC G1 c_record unmaterialized-base gate.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_record.cpp init_global_records lines 33-38 clears
 *   every recordptr[] before loading. DM2_GET_ADDRESS_OF_RECORD lines 46-52
 *   then adds the 10-bit-index stride to that selected base.
 *
 * The test supplies no dungeon, GDAT, or render fixture. It checks the public
 * resolver boundary with an absent DB0 base, where the old -1 plus index*4
 * arithmetic could accidentally become an in-range byte address.
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

int main(void)
{
    uint8_t raw[8];
    DM2_V1_DungeonData dungeon;
    const uint8_t *record;
    int type = 17;
    int index = 18;
    int size = 19;

    memset(raw, 0, sizeof(raw));
    memset(&dungeon, 0, sizeof(dungeon));
    for (int db = 0; db < 16; ++db)
        dungeon.thing_data_bases[db] = -1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.thing_type_counts[0] = 2;

    record = dm2_v1_dungeon_get_thing_record(
        &dungeon, 0x0001u, &type, &index, &size);
    CHECK(record == NULL,
          "unmaterialized DB0 base cannot become an index-1 byte address");
    CHECK(type == -1 && index == -1 && size == 0,
          "unmaterialized base keeps public resolver outputs invalid");
    CHECK(dm2_v1_dungeon_get_next_thing(&dungeon, 0x0001u) == -1,
          "unmaterialized base cannot expose a record-chain link");

    dungeon.thing_data_bases[0] = 0;
    record = dm2_v1_dungeon_get_thing_record(
        &dungeon, 0x0001u, &type, &index, &size);
    CHECK(record == raw + 4,
          "materialized DB0 base retains c_record index-one stride");
    CHECK(type == 0 && index == 1 && size == 4,
          "materialized DB0 record retains public type/index/stride metadata");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
