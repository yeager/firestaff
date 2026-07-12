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

static void make_dungeon(CSB_V1_DungeonData *dungeon,
                         unsigned char *raw,
                         int sensor_x,
                         int sensor_y)
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
    raw[sensor_x * 3 + sensor_y] |= 0x10u;
    raw[2 * 3] = (unsigned char)(6u << 5);
    put_le16(raw, 60 + sensor_x * 2, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT);
    put_le16(raw, 72, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 74, (unsigned short)(2u << 6));
}

static void prepare_party(CSB_V1_RuntimeProfile *profile,
                          CSB_V1_DungeonData *dungeon,
                          int x,
                          int y,
                          int direction)
{
    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->current_level = 0;
    profile->party_x = x;
    profile->party_y = y;
    profile->party_dir = direction;
    profile->champion_count = 1;
    profile->party_state_valid = 1;
    profile->party_state.ChampionCount = 1;
    profile->party_state.Champions[0].CurrentHealth = 100;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[96];

    make_dungeon(&dungeon, raw, 0, 1);
    prepare_party(&profile, &dungeon, 0, 0, CSB_V1_DIR_SOUTH);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) == 1,
          "MOVE_FORWARD queues the live C001 party route");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "MOVE_FORWARD reaches the CSB F0267 runtime boundary");
    CHECK(result.movement_step_applied == 1 &&
              profile.party_x == 0 && profile.party_y == 1 &&
              result.sensor_trigger_count == 1 &&
              result.sensor_last_type == DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE_OBJECT,
          "party enters an empty C001 floor-sensor square");
    CHECK(result.sensor_event_count == 1 &&
              result.sensor_last_event_type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.eventCount == 1,
          "C001 party addition publishes the source F0272/F0268 event");

    make_dungeon(&dungeon, raw, 1, 1);
    prepare_party(&profile, &dungeon, 1, 1, CSB_V1_DIR_NORTH);
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              &queue, DM1_V1_COMMAND_TURN_RIGHT, 0, 0) == 1,
          "TURN_RIGHT queues a same-square C001 pass");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1 &&
              result.sensor_trigger_count == 0 &&
              profile.timeline_queue.eventCount == 0,
          "PartySquare suppresses C001 during a source F0284 turn");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
