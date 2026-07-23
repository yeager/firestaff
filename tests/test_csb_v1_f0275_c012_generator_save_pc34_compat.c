/* ReDMCSB MOVESENS.C F0275 C012 -> F0167 -> F0272/F0268 -> save. */
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
    "/tmp/firestaff_csb_f0275_c012_generator_save.fsav";

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
    return (unsigned short)(((cell & 3) << 4) | ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static int square_offset(int x, int y) { return x * 3 + y; }

static int queued_fakewall_events(const CSB_V1_RuntimeProfile *profile)
{
    int index;
    int count = 0;
    for (index = 0; profile && index < profile->timeline_queue.eventCount;
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
                         unsigned char raw[128], unsigned short hand)
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
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[5] = 76;
    dungeon->thing_type_counts[5] = 1;

    raw[square_offset(0, 0)] = 0x10u;
    raw[square_offset(2, 0)] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((51u << 7) |
                                        DM1_SENSOR_WALL_OBJECT_GENERATOR_ROTATE));
    put_le16(raw, 72, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 74, sensor_target(2, 0, 0));
    put_le16(raw, 76, 0xffffu);
    put_le16(raw, 78, 0u);

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->dungeon_game_id = 0x0731u;
    profile->dungeon_seed = 0xC5B12C02u;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.Champions[0].CurrentHealth = 100;
    profile->party_state.LeaderHandThing = hand;
    profile->csbwin_gameblock2_summary_valid = 1;
    profile->csbwin_object_in_hand = hand;
}

static void test_c012_generator_roundtrips_runtime_save(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeProfile loaded;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];

    remove(kSavePath);
    make_fixture(&profile, &dungeon, raw, 0xffffu);
    profile.game_time = 19u;
    profile.timeline_queue.gameTick = profile.game_time;
    CHECK(csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
              &profile, 0, 0, 0) == 1,
          "C012 allocates only through the live empty runtime hand");
    CHECK(profile.party_state.LeaderHandThing == (unsigned short)(5u << 10) &&
              profile.csbwin_object_in_hand == (unsigned short)(5u << 10) &&
              read_le16(raw, 76) == 0xfffeu && read_le16(raw, 78) == 27u &&
              queued_fakewall_events(&profile) == 1,
          "C012 materializes the F0167 arrow and queues the source F0268 event");
    CHECK(csb_v1_runtime_save_game_to_path(&profile, kSavePath) == 0,
          "C012 generated hand and pending event write through native save");
    csb_v1_runtime_init(&loaded, NULL);
    loaded.dungeon_handle = &dungeon;
    loaded.csbwin_gameblock2_summary_valid = 1;
    CHECK(csb_v1_runtime_load_game_from_path(&loaded, kSavePath) == 0,
          "C012 native save reloads through the existing runtime handoff");
    CHECK(loaded.party_state.LeaderHandThing == (unsigned short)(5u << 10) &&
              loaded.csbwin_object_in_hand == (unsigned short)(5u << 10) &&
              loaded.game_time == 19u &&
              loaded.timeline_queue.gameTick == loaded.game_time &&
              queued_fakewall_events(&loaded) == 1,
          "reload preserves the generated hand and source timeline clock");
    CHECK(csb_v1_runtime_tick_v1(&loaded) == 1 &&
              (raw[square_offset(2, 0)] & 0x04u) != 0u,
          "reloaded C012 event reaches the existing F0261 fakewall owner");
    remove(kSavePath);
}

static void test_c012_rejects_nonempty_hand_without_mutation(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    unsigned char before[128];

    make_fixture(&profile, &dungeon, raw, (unsigned short)(5u << 10));
    memcpy(before, raw, sizeof(before));
    CHECK(csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
              &profile, 0, 0, 0) == 0,
          "C012 rejects a nonempty leader hand before F0167 allocation");
    CHECK(profile.party_state.LeaderHandThing == (unsigned short)(5u << 10) &&
              profile.csbwin_object_in_hand == (unsigned short)(5u << 10) &&
              profile.timeline_queue.eventCount == 0 &&
              memcmp(raw, before, sizeof(raw)) == 0,
          "C012 rejection leaves the hand, raw allocator record, and timeline unchanged");
}

static void test_c012_fails_closed_when_f0167_has_no_free_record(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    unsigned char before[128];

    make_fixture(&profile, &dungeon, raw, 0xffffu);
    /* No C05 records are available for the source F0167 allocation. */
    dungeon.thing_type_counts[5] = 0;
    memcpy(before, raw, sizeof(before));
    CHECK(csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
              &profile, 0, 0, 0) == 0,
          "C012 fails closed when F0167 cannot allocate its source object");
    CHECK(profile.party_state.LeaderHandThing == 0xffffu &&
              profile.csbwin_object_in_hand == 0xffffu &&
              profile.timeline_queue.eventCount == 0 &&
              memcmp(raw, before, sizeof(raw)) == 0,
          "failed C012 leaves the sensor, fakewall timeline, and hand unchanged");
}

static void test_c012_rejects_a_stale_save_clock(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    unsigned char before[128];

    remove(kSavePath);
    make_fixture(&profile, &dungeon, raw, 0xffffu);
    profile.game_time = 19u;
    profile.timeline_queue.gameTick = 18u;
    memcpy(before, raw, sizeof(before));
    CHECK(csb_v1_runtime_trigger_wall_ornament_click_runtime_hand(
              &profile, 0, 0, 0) == 0,
          "C012 rejects a stale save/timeline identity before F0167");
    CHECK(profile.party_state.LeaderHandThing == 0xffffu &&
              profile.timeline_queue.eventCount == 0 &&
              memcmp(raw, before, sizeof(raw)) == 0 &&
              csb_v1_runtime_save_game_to_path(&profile, kSavePath) != 0,
          "stale C012 state cannot materialize or cross the native save boundary");
    remove(kSavePath);
}

int main(void)
{
    test_c012_generator_roundtrips_runtime_save();
    test_c012_rejects_nonempty_hand_without_mutation();
    test_c012_fails_closed_when_f0167_has_no_free_record();
    test_c012_rejects_a_stale_save_clock();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
