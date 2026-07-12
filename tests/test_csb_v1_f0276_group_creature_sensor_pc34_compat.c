#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_combat_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
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

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

static void add_group_move(CSB_V1_RuntimeProfile *profile,
                           unsigned short group_thing,
                           int target_x,
                           int target_y)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_MOVE_GROUP_SILENT;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    event.b_mapX = (unsigned char)target_x;
    event.b_mapY = (unsigned char)target_y;
    event.c_cell = (unsigned char)(group_thing & 0xffu);
    event.c_effect = (unsigned char)(group_thing >> 8);
    CHECK(csb_v1_runtime_add_timeline_event(profile, &event) >= 0,
          "C04 move enters the live CSB timeline");
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[132];
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
    dungeon.thing_type_counts[4] = 2;
    raw[square_offset(0, 0)] |= 0x10u;
    raw[square_offset(1, 0)] |= 0x10u;
    raw[square_offset(2, 0)] = (unsigned char)(6u << 5);
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
    add_group_move(&profile, group_thing, 1, 0);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C04 group move dispatches through F0267");
    CHECK(get_le16(raw, 66) == 0xfffeu &&
              get_le16(raw, 68) == group_thing,
          "F0267 relinks the C04 group onto the C007 square");
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].b_mapX == 2 &&
              profile.timeline_queue.events[0].b_mapY == 0 &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET,
          "C007 group addition publishes the ReDMCSB F0268 fakewall event");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
