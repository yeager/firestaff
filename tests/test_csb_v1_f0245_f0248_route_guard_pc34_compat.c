/* ReDMCSB TIMELINE.C F0245/F0248: source square kind gates and C006 order. */
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

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

static void make_c006_corridor(CSB_V1_DungeonData *dungeon,
                               unsigned char raw[128])
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 1;
    dungeon->level_heights[0] = 1;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[4] = 76;
    dungeon->thing_type_counts[4] = 1;

    raw[0] = (unsigned char)((DM1_SQUARE_CORRIDOR << 5) | 0x10u);
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((1u << 7) | 6u));
    put_le16(raw, 72, (unsigned short)((1u << 6) | (2u << 7)));
    put_le16(raw, 74, (unsigned short)((2u << 4) | 1u));
    put_le16(raw, 76, 0xffffu);
}

static int tick_event(CSB_V1_RuntimeProfile *profile, int event_type)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = (unsigned char)event_type;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0 &&
           csb_v1_runtime_tick_v1(profile) == 1;
}

static void test_f0245_generator_source_order(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    unsigned char raw[128];

    make_c006_corridor(&dungeon, raw);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.dungeon_seed = 0xC5B10006u;

    CHECK(tick_event(&profile, DM1_EVENT_CORRIDOR),
          "F0245 consumes a real-format C006 corridor event");
    CHECK((get_le16(raw, 70) & 0x007fu) == 0u &&
              get_le16(raw, 66) == (unsigned short)(4u << 10) &&
              profile.audio_runtime.pendingSoundIndex == CSB_V1_SOUND_BUZZ &&
              profile.audio_runtime.totalRequests == 1u &&
              profile.timeline_queue.eventCount == 2,
          "F0245 orders F0185 materialization, audible buzz, then C006/C65 lifecycle");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1 &&
              (get_le16(raw, 70) & 0x007fu) == 0u &&
              csb_v1_runtime_tick_v1(&profile) == 1 &&
              (get_le16(raw, 70) & 0x007fu) == 6u,
          "C65 re-enables the disabled C006 sensor at its source tick");
}

static void test_f0245_f0248_reject_wrong_square_kind(void)
{
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    unsigned char raw[128];
    unsigned char before[128];

    make_c006_corridor(&dungeon, raw);
    raw[0] = 0x10u;
    memcpy(before, raw, sizeof(before));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    CHECK(tick_event(&profile, DM1_EVENT_CORRIDOR) &&
              memcmp(raw, before, sizeof(raw)) == 0 &&
              profile.audio_runtime.totalRequests == 0u &&
              profile.timeline_queue.eventCount == 0,
          "F0245 rejects a C05 event whose loaded square is not a corridor");

    make_c006_corridor(&dungeon, raw);
    memcpy(before, raw, sizeof(before));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    CHECK(tick_event(&profile, DM1_EVENT_WALL) &&
              memcmp(raw, before, sizeof(raw)) == 0 &&
              profile.audio_runtime.totalRequests == 0u &&
              profile.timeline_queue.eventCount == 0,
          "F0248 rejects a C06 event whose loaded square is not a wall");
}

int main(void)
{
    test_f0245_generator_source_order();
    test_f0245_f0248_reject_wrong_square_kind();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
