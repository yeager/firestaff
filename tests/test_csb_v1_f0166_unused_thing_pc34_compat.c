#include "csb_v1_dungeon_loader_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int s_failures;

#define CHECK(condition) do { \
    if (!(condition)) { \
        fprintf(stderr, "failed: %s (%s:%d)\n", #condition, __FILE__, __LINE__); \
        s_failures++; \
    } \
} while (0)

static void write_u16(uint8_t *bytes, int offset, uint16_t value)
{
    bytes[offset] = (uint8_t)value;
    bytes[offset + 1] = (uint8_t)(value >> 8);
}

static uint16_t return_weapon_one(uint16_t type, void *user)
{
    int *called = (int *)user;
    if (called) *called += 1;
    return type == 5 ? 0x1401u : 0xffffu;
}

int main(void)
{
    CSB_V1_DungeonData dungeon;
    uint8_t bytes[96];
    int discard_called = 0;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(bytes, 0, sizeof(bytes));
    dungeon.square_bytes = 1;
    dungeon.raw_data = bytes;
    dungeon.raw_size = (int)sizeof(bytes);
    dungeon.thing_data_bases[5] = 16;
    dungeon.thing_type_counts[5] = 2;
    dungeon.thing_data_bases[10] = 48;
    dungeon.thing_type_counts[10] = 4;

    write_u16(bytes, 16, 0xffffu);
    write_u16(bytes, 20, 0xffffu);
    CHECK(csb_v1_dungeon_f0166_get_unused_thing_pc34(
        &dungeon, 5, NULL, NULL) == 0x1400u);
    CHECK(bytes[16] == 0xfe && bytes[17] == 0xff);
    CHECK(bytes[18] == 0 && bytes[19] == 0);

    write_u16(bytes, 16, 0xfffeu);
    write_u16(bytes, 20, 0xfffeu);
    CHECK(csb_v1_dungeon_f0166_get_unused_thing_pc34(
        &dungeon, 5, return_weapon_one, &discard_called) == 0x1401u);
    CHECK(discard_called == 1);
    CHECK(bytes[20] == 0xfe && bytes[21] == 0xff);

    write_u16(bytes, 48, 0xfffeu);
    write_u16(bytes, 52, 0xffffu);
    write_u16(bytes, 56, 0xffffu);
    write_u16(bytes, 60, 0xffffu);
    CHECK(csb_v1_dungeon_f0166_get_unused_thing_pc34(
        &dungeon, 10, NULL, NULL) == 0xffffu);
    CHECK(csb_v1_dungeon_f0166_get_unused_thing_pc34(
        &dungeon, 0x800au, NULL, NULL) == 0x2801u);
    CHECK(bytes[52] == 0xfe && bytes[53] == 0xff);
    CHECK(csb_v1_dungeon_f0166_get_unused_thing_pc34(
        &dungeon, 0x8005u, NULL, NULL) == 0xffffu);

    if (s_failures != 0) return 1;
    puts("test_csb_v1_f0166_unused_thing_pc34_compat: PASS");
    return 0;
}
