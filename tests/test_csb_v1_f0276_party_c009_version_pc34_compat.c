/* ReDMCSB MOVESENS.C F0276 C009 -> F0272/F0268 -> F0261 PC34 route. */
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_input_command_queue_pc34_compat.h"
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

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[96],
                         int version_data)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, (unsigned char)(1u << 5), 96u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 96;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 1;

    /* The loaded C03 record sits on the entered level-zero column x=0. */
    put_le16(raw, 60, 0u);
    raw[1] |= 0x10u;
    raw[6] = (unsigned char)(6u << 5); /* F0268 fakewall target at (2,0). */
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((version_data << 7) |
                                        DM1_SENSOR_FLOOR_VERSION_CHECKER));
    put_le16(raw, 72, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 74, sensor_target(2, 0, 0));

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
    profile->party_x = 0;
    profile->party_y = 0;
    profile->party_dir = CSB_V1_DIR_SOUTH;
    profile->champion_count = 1;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.Champions[0].CurrentHealth = 100;
}

static void run_case(int version_data, int expect_publish, const char *message)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[96];
    unsigned char before[96];

    make_fixture(&profile, &dungeon, raw, version_data);
    memcpy(before, raw, sizeof(before));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) == 1,
          "MOVE_FORWARD queues the source C009 route");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "C009 route reaches the live F0276 runtime boundary");
    if (expect_publish) {
        CHECK(result.movement_step_applied == 1 &&
                  result.sensor_trigger_count == 1 &&
                  result.sensor_last_type == DM1_SENSOR_FLOOR_VERSION_CHECKER &&
                  result.sensor_last_data == version_data &&
                  result.sensor_event_count == 1 &&
                  result.sensor_last_event_type == DM1_EVENT_FAKEWALL &&
                  profile.timeline_queue.eventCount == 1,
              message);
        CHECK(csb_v1_runtime_tick_v1(&profile) == 1 &&
                  profile.timeline_queue.eventCount == 0 &&
                  (raw[6] & 0x04u) != 0u,
              "published C009 effect reaches the existing F0261 fakewall consumer");
    } else {
        CHECK(result.movement_step_applied == 1 &&
                  result.sensor_trigger_count == 0 &&
                  result.sensor_event_count == 0 &&
                  profile.timeline_queue.eventCount == 0 &&
                  memcmp(raw, before, sizeof(raw)) == 0,
              message);
    }
}

static void test_c009_rejects_a_stale_save_clock(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[96];
    unsigned char before[96];

    make_fixture(&profile, &dungeon, raw, 34);
    profile.game_time = 19u;
    profile.timeline_queue.gameTick = 18u;
    memcpy(before, raw, sizeof(before));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) == 1 &&
              csb_v1_runtime_process_input_queue(
                  &profile, &queue, 0, 0, 0, &result) == 1,
          "stale-clock C009 reaches the F0267/F0276 boundary");
    CHECK(result.sensor_trigger_count == 0 &&
              result.sensor_event_count == 0 &&
              profile.timeline_queue.eventCount == 0 &&
              memcmp(raw, before, sizeof(raw)) == 0,
          "C009 rejects a stale save/timeline identity before F0272/F0268");
}

int main(void)
{
    run_case(34, 1,
             "C009 accepts the exact ReDMCSB PC34 engine boundary");
    run_case(35, 0,
             "C009 rejects an over-bound source Data value without mutation");
    test_c009_rejects_a_stale_save_clock();

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
