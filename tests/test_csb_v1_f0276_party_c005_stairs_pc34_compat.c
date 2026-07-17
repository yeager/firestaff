/* ReDMCSB MOVESENS.C F0267/F0276 C005 -> F0272/F0268 runtime route. */
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

static unsigned short read_le16(const unsigned char *bytes, int offset)
{
    return (unsigned short)(bytes[offset] | ((unsigned short)bytes[offset + 1] << 8));
}

static unsigned short sensor_target(int x, int y, int cell)
{
    return (unsigned short)(((cell & 3) << 4) |
                            ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char *raw,
                         int stairs)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, (unsigned char)(1u << 5), 128u);
    dungeon->level_count = 2;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->level_widths[1] = 3;
    dungeon->level_heights[1] = 3;
    dungeon->level_offsets[1] = 9;
    dungeon->map_levels[0] = 0;
    dungeon->map_levels[1] = 1;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 88;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 90;
    dungeon->thing_type_counts[3] = 1;

    /* Header column count for level-zero column x=0.  The target at (0,1)
     * is the first listed square in that column. */
    put_le16(raw, 76, 0u);
    raw[1] = (unsigned char)(((stairs ? 3u : 1u) << 5) | 0x10u);
    raw[6] = (unsigned char)(6u << 5); /* F0268 fakewall target at (2,0). */
    raw[10] = (unsigned char)((3u << 5) | 0x04u);
    put_le16(raw, 88, (unsigned short)(3u << 10));
    put_le16(raw, 90, 0xfffeu);
    put_le16(raw, 92, DM1_SENSOR_FLOOR_PARTY_ON_STAIRS);
    put_le16(raw, 94, (unsigned short)((1u << 2) | (DM1_EFFECT_SET << 3) |
                                        (1u << 6) | (3u << 7)));
    put_le16(raw, 96, sensor_target(2, 0, 3));

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

static void queue_forward(struct Dm1V1InputCommandQueuePc34Compat *queue)
{
    DM1_V1_InputCommandQueue_InitPc34Compat(queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) == 1,
          "MOVE_FORWARD enters the C005 source route");
}

static void test_stairs_addition_publishes_source_effect(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[128];

    make_fixture(&profile, &dungeon, raw, 1);
    queue_forward(&queue);
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "C005 staircase command reaches the live CSB runtime");
    CHECK(result.sensor_destination_add_checked == 1 &&
              result.sensor_trigger_count == 1 &&
              result.sensor_last_type == DM1_SENSOR_FLOOR_PARTY_ON_STAIRS &&
              result.sensor_event_count == 1 &&
              result.sensor_last_event_type == DM1_EVENT_FAKEWALL,
          "C005 evaluates the entered stairs square before F0364 changes level");
    CHECK(profile.current_level == 1 && result.stair_transition_applied == 1,
          "C005 leaves the authentic stairs transition intact");
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].map_time ==
                  DM1_MAP_TIME_MAKE(0, 3) &&
              profile.timeline_queue.events[0].c_cell == 0u &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET,
          "C005 preserves F0272 Value and non-wall target-cell routing");
    CHECK((read_le16(raw, 92) & 0x007fu) == 0u,
          "C005 OnceOnly clears the loaded source sensor type before publish");
    CHECK(profile.audio_runtime.pendingSoundIndex == CSB_V1_SOUND_SWITCH &&
              profile.audio_runtime.totalRequests == 1u,
          "C005 Audible reaches the source CSB audio owner");
}

static void test_non_stairs_rejects_without_mutation(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[128];

    make_fixture(&profile, &dungeon, raw, 0);
    queue_forward(&queue);
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "non-stairs command remains a valid runtime movement");
    CHECK(result.sensor_trigger_count == 0 && result.sensor_event_count == 0 &&
              profile.timeline_queue.eventCount == 0 &&
              (read_le16(raw, 92) & 0x007fu) ==
                  DM1_SENSOR_FLOOR_PARTY_ON_STAIRS &&
              profile.audio_runtime.totalRequests == 0u,
          "C005 rejects a non-stairs square without queue, audio, or raw mutation");
}

int main(void)
{
    test_stairs_addition_publishes_source_effect();
    test_non_stairs_rejects_without_mutation();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
