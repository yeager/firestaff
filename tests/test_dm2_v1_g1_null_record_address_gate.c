/*
 * PC G1 c_record null-address gate.
 *
 * Source-lock:
 *   skproject/SKULLWIN/c_record.cpp DM2_GET_ADDRESS_OF_RECORD lines 45-52
 *   decodes DB from bits 10..13 and the 10-bit record index.
 *   DM2_APPEND_RECORD_TO lines 67-68 rejects OBJECT_END_MARKER and
 *   OBJECT_NULL before it calls that address transform.
 *
 * This deliberately malformed DB15 pool is a resolver-boundary regression,
 * not a materialized dungeon fixture.  It proves OBJECT_NULL (0xffff) cannot
 * be reinterpreted as the otherwise valid DB15/index-1023 address.
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

int main(void) {
    enum { DB15_RECORD_SIZE = 4, DB15_MAX_INDEX = 1023 };
    uint8_t raw[DB15_RECORD_SIZE * (DB15_MAX_INDEX + 1)];
    DM2_V1_DungeonData dungeon;
    const uint8_t *record;
    int type = 77;
    int index = 88;
    int size = 99;

    memset(raw, 0xa5, sizeof(raw));
    memset(&dungeon, 0, sizeof(dungeon));
    for (int i = 0; i < 16; ++i)
        dungeon.thing_data_bases[i] = -1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.thing_data_bases[15] = 0;
    dungeon.thing_type_counts[15] = DB15_MAX_INDEX + 1;

    record = dm2_v1_dungeon_get_thing_record(
        &dungeon, 0xffffu, &type, &index, &size);
    CHECK(record == NULL,
          "OBJECT_NULL never materializes as DB15/index-1023 record bytes");
    CHECK(type == -1,
          "OBJECT_NULL leaves the public record type invalid");
    CHECK(index == -1,
          "OBJECT_NULL leaves the public record index invalid");
    CHECK(size == 0,
          "OBJECT_NULL leaves the public record size empty");

    record = dm2_v1_dungeon_get_thing_record(
        &dungeon, 0xfffeu, &type, &index, &size);
    CHECK(record == NULL && type == -1 && index == -1 && size == 0,
          "OBJECT_END_MARKER remains equally non-addressable");

    record = dm2_v1_dungeon_get_thing_record(
        &dungeon, 0x3fffu, &type, &index, &size);
    CHECK(record == raw + DB15_MAX_INDEX * DB15_RECORD_SIZE,
          "a real DB15/index-1023 ObjectID remains addressable");
    CHECK(type == 15 && index == DB15_MAX_INDEX && size == DB15_RECORD_SIZE,
          "real DB15 address preserves c_record type/index/stride metadata");

    printf("%d passed, %d failed\n", passed, failed);
    return failed != 0;
}
