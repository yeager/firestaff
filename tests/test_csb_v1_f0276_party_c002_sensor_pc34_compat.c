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

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[96];

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, (unsigned char)(1u << 5), sizeof(raw));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 3;
    dungeon.level_heights[0] = 3;
    dungeon.square_bytes = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[3] = 68;
    dungeon.thing_type_counts[3] = 1;
    raw[square_offset(0, 1)] |= 0x10u;
    raw[square_offset(2, 0)] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE);
    put_le16(raw, 72, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 74, (unsigned short)(2u << 6));

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = 0;
    profile.party_x = 0;
    profile.party_y = 0;
    profile.party_dir = CSB_V1_DIR_SOUTH;
    profile.champion_count = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;
    profile.party_state.Champions[0].CurrentHealth = 100;

    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) == 1,
          "MOVE_FORWARD queues the live C002 party route");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "MOVE_FORWARD reaches the CSB F0267 runtime boundary");
    CHECK(result.movement_step_applied == 1 &&
              profile.party_x == 0 && profile.party_y == 1,
          "party enters the C002 floor-sensor square");
    CHECK(result.sensor_trigger_count == 1 &&
              result.sensor_last_type == DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE &&
              result.sensor_event_count == 1 &&
              result.sensor_last_event_type == DM1_EVENT_FAKEWALL &&
              result.sensor_last_effect == DM1_EFFECT_SET,
          "C002 party addition queues the source F0272/F0268 fakewall SET");
    CHECK(profile.timeline_queue.eventCount == 1,
          "C002 owns exactly one queued square-state event");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
