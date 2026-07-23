#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int failed;

#define CHECK(condition, message) do { \
    if (!(condition)) { ++failed; fprintf(stderr, "FAIL: %s\n", message); } \
} while (0)

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static unsigned short get_le16(const unsigned char *bytes, int offset)
{
    return (unsigned short)(bytes[offset] |
                            ((unsigned short)bytes[offset + 1] << 8));
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct DM1_Event_V1 move;
    unsigned char raw[112];
    unsigned short group_thing = (unsigned short)(4u << 10);

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, (unsigned char)(1u << 5), sizeof(raw));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 3;
    dungeon.level_heights[0] = 3;
    dungeon.square_bytes = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 2;
    dungeon.thing_data_bases[3] = 72;
    dungeon.thing_type_counts[3] = 1;
    dungeon.thing_data_bases[4] = 80;
    dungeon.thing_type_counts[4] = 1;
    raw[0] |= 0x10u;
    raw[3] |= 0x10u;
    raw[6] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 66, group_thing);
    put_le16(raw, 68, (unsigned short)(3u << 10));
    put_le16(raw, 72, 0xfffeu);
    put_le16(raw, 74, DM1_SENSOR_FLOOR_CREATURE);
    put_le16(raw, 76, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 78, (unsigned short)(2u << 6));
    put_le16(raw, 80, 0xfffeu);
    raw[84] = 9u;
    put_le16(raw, 94, 0u);

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    memset(&move, 0, sizeof(move));
    move.type = DM1_EVENT_MOVE_GROUP_SILENT;
    move.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    move.b_mapX = 1;
    move.b_mapY = 0;
    move.c_cell = (unsigned char)(group_thing & 0xffu);
    move.c_effect = (unsigned char)(group_thing >> 8);
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &move) >= 0,
          "C04 move enters the live CSB timeline");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "F0267 commits the C04 group relocation");
    CHECK(get_le16(raw, 66) == 0xfffeu &&
              get_le16(raw, 68) == (unsigned short)(3u << 10) &&
              get_le16(raw, 72) == group_thing,
          "C04 is relinked after the loaded C007 sensor prefix");
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].b_mapX == 2 &&
              profile.timeline_queue.events[0].b_mapY == 0 &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET,
          "C007 group addition publishes the ReDMCSB F0272/F0268 event");
    return failed ? 1 : 0;
}
