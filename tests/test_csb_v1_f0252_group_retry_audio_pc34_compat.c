/* ReDMCSB TIMELINE.C F0252 C60/C61 raw group-retry audio. */
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_audio_runtime_pc34_compat.h"

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

static void make_dungeon(CSB_V1_DungeonData *dungeon, unsigned char *raw,
                         size_t raw_size)
{
    const unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 2;
    dungeon->level_heights[0] = 1;
    dungeon->square_bytes = 1;
    dungeon->square_first_thing_base = 64;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[THING_TYPE_GROUP] = 72;
    dungeon->thing_type_counts[THING_TYPE_GROUP] = 1;
    raw[0] = (unsigned char)((1u << 5) | 0x10u);
    raw[1] = (unsigned char)((1u << 5) | 0x10u);
    put_le16(raw, 60, 0u);
    put_le16(raw, 62, 1u);
    put_le16(raw, 64, group);
    put_le16(raw, 66, THING_ENDOFLIST);
    put_le16(raw, 72, THING_ENDOFLIST);
    raw[76] = 9u;
    raw[77] = 0xffu;
    put_le16(raw, 78, 40u);
    put_le16(raw, 86, 7u);
}

static void run_retry(int audible)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct DM1_Event_V1 event;
    unsigned char raw[112];
    const unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);
    uint32_t requests_before;

    make_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.party_x = 0;
    profile.party_y = 0;
    memset(&event, 0, sizeof(event));
    event.type = audible ? DM1_EVENT_MOVE_GROUP_AUDIBLE
                         : DM1_EVENT_MOVE_GROUP_SILENT;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.b_mapX = 1;
    event.b_mapY = 0;
    event.c_cell = (unsigned char)(group & 0xffu);
    event.c_effect = (unsigned char)(group >> 8);
    requests_before = profile.audio_runtime.totalRequests;

    CHECK(csb_v1_runtime_add_timeline_event(&profile, &event) >= 0,
          "raw C60/C61 timer enters the live CSB timeline");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "F0252 consumes the raw C60/C61 timer");
    CHECK(get_le16(raw, 64) == THING_ENDOFLIST &&
              get_le16(raw, 66) == group,
          "F0252 commits the linked raw C04 relocation before audio");
    if (audible) {
        CHECK(profile.audio_runtime.totalRequests == requests_before + 2u &&
                  profile.audio_runtime.pendingSoundIndex == CSB_V1_SOUND_BUZZ,
              "C61 publishes the source buzz alongside F0267 movement audio");
    } else {
        CHECK(profile.audio_runtime.totalRequests == requests_before + 1u &&
                  profile.audio_runtime.pendingSoundIndex != CSB_V1_SOUND_BUZZ,
              "C60 retains F0267 movement audio without the C61 buzz");
    }
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct DM1_Event_V1 event;
    unsigned char raw[112];
    const unsigned short group = (unsigned short)(THING_TYPE_GROUP << 10);

    run_retry(0);
    run_retry(1);

    make_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    put_le16(raw, 64, THING_ENDOFLIST);
    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_MOVE_GROUP_AUDIBLE;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.b_mapX = 1;
    event.b_mapY = 0;
    event.c_cell = (unsigned char)(group & 0xffu);
    event.c_effect = (unsigned char)(group >> 8);
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &event) >= 0 &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              profile.audio_runtime.totalRequests == 0u,
          "stale C61 owner fails closed without publishing buzz audio");
    return failed ? 1 : 0;
}
