/* ReDMCSB MOVESENS.C F0275 C011 -> F0272/F0268 -> CSB save handoff. */
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

static const char *const kSavePath =
    "/tmp/firestaff_csb_f0275_c011_wall_click_save.fsav";

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static unsigned short read_le16(const unsigned char *bytes, int offset)
{
    return (unsigned short)(bytes[offset] |
                            ((unsigned short)bytes[offset + 1] << 8));
}

static unsigned short sensor_target(int x, int y, int cell)
{
    return (unsigned short)(((cell & 3) << 4) |
                            ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

static int count_queued_fakewall_events(const CSB_V1_RuntimeProfile *profile)
{
    int count = 0;
    int index;

    for (index = 0;
         profile && index < profile->timeline_queue.eventCount;
         ++index) {
        int slot = profile->timeline_queue.timeline[index];
        if (slot >= 0 && slot < DM1_EVENT_MAX_COUNT &&
            profile->timeline_queue.events[slot].type == DM1_EVENT_FAKEWALL) {
            ++count;
        }
    }
    return count;
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[128],
                         int sensor_object_type)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 5;
    dungeon->thing_data_bases[5] = 108;
    dungeon->thing_type_counts[5] = 1;

    /* A wall at (0,0) owns a C011 in cell zero. The preceding C000 keeps
     * rotation observable, while the C011 consumes the live type-8 hand. */
    raw[square_offset(0, 0)] = 0x10u;
    raw[square_offset(2, 0)] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, (unsigned short)((3u << 10) | 4u));
    put_le16(raw, 70, 0u);
    put_le16(raw, 72, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 74, sensor_target(2, 0, 0));
    put_le16(raw, 100, 0xfffeu);
    put_le16(raw, 102, (unsigned short)((sensor_object_type << 7) |
                                         DM1_SENSOR_WALL_CLICK_OBJ_REMOVED_ROTATE));
    put_le16(raw, 104, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 106, sensor_target(2, 0, 0));
    put_le16(raw, 108, 0xfffeu);
    put_le16(raw, 110, 8u); /* Loaded C05 object's source type. */

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->dungeon_game_id = 0x0731u;
    profile->dungeon_seed = 0xC5B11C01u;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.Champions[0].CurrentHealth = 100;
    profile->party_state.LeaderHandThing = (unsigned short)(5u << 10);
    profile->csbwin_gameblock2_summary_valid = 1;
    profile->csbwin_object_in_hand = profile->party_state.LeaderHandThing;
}

static void test_c011_runtime_hand_roundtrips_save(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeProfile loaded;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];

    remove(kSavePath);
    make_fixture(&profile, &dungeon, raw, 8);
    CHECK(csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
              &profile, 0, 0, 0) == 1,
          "C011 consumes the matching source-owned runtime hand");
    CHECK(profile.party_state.LeaderHandThing == 0xffffu &&
              profile.csbwin_object_in_hand == 0xffffu &&
              read_le16(raw, 108) == 0xffffu &&
              read_le16(raw, 66) == (unsigned short)((3u << 10) | 4u) &&
              read_le16(raw, 100) == (unsigned short)(3u << 10) &&
              count_queued_fakewall_events(&profile) == 1,
          "F0275 clears only the hand object, rotates C011, and queues F0268");
    CHECK(csb_v1_runtime_save_game_to_path(&profile, kSavePath) == 0,
          "C011 live hand and pending F0268 write through native runtime save");
    csb_v1_runtime_init(&loaded, NULL);
    loaded.dungeon_handle = &dungeon; /* Raw Dungeon stays boot-owned. */
    loaded.csbwin_gameblock2_summary_valid = 1;
    loaded.csbwin_object_in_hand = (unsigned short)(5u << 10);
    CHECK(csb_v1_runtime_load_game_from_path(&loaded, kSavePath) == 0,
          "C011 native runtime save reloads through the existing handoff");
    CHECK(loaded.party_state_valid &&
              loaded.party_state.LeaderHandThing == 0xffffu &&
              loaded.csbwin_object_in_hand == 0xffffu &&
              count_queued_fakewall_events(&loaded) == 1 &&
              read_le16(raw, 108) == 0xffffu,
          "reload retains the emitted runtime hand and F0268 state without a copy");
    CHECK(csb_v1_runtime_tick_v1(&loaded) == 1 &&
              (raw[square_offset(2, 0)] & 0x04u) != 0u,
          "reloaded C011 F0268 event reaches the existing F0261 fakewall owner");
    remove(kSavePath);
}

static void test_c011_wrong_object_type_rejects_without_mutation(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    unsigned char before[128];

    make_fixture(&profile, &dungeon, raw, 7);
    memcpy(before, raw, sizeof(before));
    CHECK(csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
              &profile, 0, 0, 0) == 0,
          "C011 rejects a live hand whose loaded type misses Sensor.Data");
    CHECK(profile.party_state.LeaderHandThing == (unsigned short)(5u << 10) &&
              profile.csbwin_object_in_hand == (unsigned short)(5u << 10) &&
              profile.timeline_queue.eventCount == 0 &&
              memcmp(raw, before, sizeof(raw)) == 0,
          "C011 mismatch leaves the hand, raw chain, and timeline unchanged");
}

int main(void)
{
    test_c011_runtime_hand_roundtrips_save();
    test_c011_wrong_object_type_rejects_without_mutation();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
