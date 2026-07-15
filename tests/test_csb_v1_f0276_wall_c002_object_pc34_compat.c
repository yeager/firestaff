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

static unsigned short sensor_target(int x, int y, int cell)
{
    return (unsigned short)(((cell & 3) << 4) |
                            ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static void make_loaded_wall_chain(CSB_V1_DungeonData *dungeon,
                                   unsigned char *raw)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 96u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 2;
    dungeon->level_heights[0] = 1;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 96;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 1;

    /* Original PC3.4 byte-map: wall plus first-Thing bit at (0,0),
     * fakewall target at (1,0), and the original column count at +60. */
    raw[0] = 0x10u;
    raw[1] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((27u << 7) | 2u));
    put_le16(raw, 72, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 74, sensor_target(1, 0, 3));
}

static void prepare_profile(CSB_V1_RuntimeProfile *profile,
                            CSB_V1_DungeonData *dungeon)
{
    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[96];

    make_loaded_wall_chain(&dungeon, raw);
    prepare_profile(&profile, &dungeon);
    CHECK(csb_v1_runtime_trigger_wall_ornament_click(
              &profile, 0, 0, 0, 27) == 1,
          "loaded C002 wall/object chain reaches F0272");
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET &&
              profile.timeline_queue.events[0].c_cell == 0,
          "F0272 normalizes non-wall target cell before F0268 event queueing");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1 &&
              (raw[1] & 0x04u) != 0,
          "F0268 fakewall SET consumes the source-owned event");

    make_loaded_wall_chain(&dungeon, raw);
    raw[0] = (unsigned char)((1u << 5) | 0x10u);
    prepare_profile(&profile, &dungeon);
    CHECK(csb_v1_runtime_trigger_wall_ornament_click(
              &profile, 0, 0, 0, 27) == 0 &&
              profile.timeline_queue.eventCount == 0,
          "non-wall loaded chains fail closed instead of borrowing C002 wall semantics");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
