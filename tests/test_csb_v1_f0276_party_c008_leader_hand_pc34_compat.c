/* ReDMCSB MOVESENS.C F0274/F0276 C008 leader-hand possession route. */
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

enum {
    kObjectType = 27,
    kSensorOffset = 68,
    kWeaponOffset = 80,
    kContainerOffset = 88
};

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
                         unsigned char raw[128],
                         unsigned short leader_hand)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, (unsigned char)(1u << 5), 128u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 128;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = kSensorOffset;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[THING_TYPE_WEAPON] = kWeaponOffset;
    dungeon->thing_type_counts[THING_TYPE_WEAPON] = 1;
    dungeon->thing_data_bases[THING_TYPE_CONTAINER] = kContainerOffset;
    dungeon->thing_type_counts[THING_TYPE_CONTAINER] = 1;

    /* The destination's source C03 sensor is in level-zero column x=0. */
    put_le16(raw, 60, 0u);
    raw[1] |= 0x10u;
    raw[6] = (unsigned char)(6u << 5); /* F0268 fakewall target at (2,0). */
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, kSensorOffset, 0xfffeu);
    put_le16(raw, kSensorOffset + 2,
             (unsigned short)((kObjectType << 7) |
                              DM1_SENSOR_FLOOR_PARTY_POSSESSION));
    put_le16(raw, kSensorOffset + 4, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, kSensorOffset + 6, sensor_target(2, 0, 0));

    /* C05 WEAPON subtype 27 is the possession value checked by F0274. */
    put_le16(raw, kWeaponOffset, 0xfffeu);
    put_le16(raw, kWeaponOffset + 2, kObjectType);

    /* C09 CONTAINER Next, Slot, Type: Slot points only at the C05 chain. */
    put_le16(raw, kContainerOffset, 0xfffeu);
    put_le16(raw, kContainerOffset + 2, (unsigned short)(THING_TYPE_WEAPON << 10));
    put_le16(raw, kContainerOffset + 4, 0u);

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
    profile->party_state.LeaderHandThing = leader_hand;
}

static void run_case(unsigned short leader_hand,
                     int expect_sensor,
                     const char *case_name)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct Dm1V1InputCommandQueuePc34Compat queue;
    CSB_V1_InputCommandRuntimeResult result;
    unsigned char raw[128];
    unsigned char before[128];

    make_fixture(&profile, &dungeon, raw, leader_hand);
    memcpy(before, raw, sizeof(before));
    DM1_V1_InputCommandQueue_InitPc34Compat(&queue);
    CHECK(DM1_V1_InputCommandQueue_EnqueueCommandPc34Compat(
              &queue, DM1_V1_COMMAND_MOVE_FORWARD, 0, 0) == 1,
          "MOVE_FORWARD queues the C008 possession route");
    CHECK(csb_v1_runtime_process_input_queue(
              &profile, &queue, 0, 0, 0, &result) == 1,
          "C008 possession route reaches the live CSB runtime");
    CHECK(result.movement_step_applied == 1 &&
              profile.party_x == 0 && profile.party_y == 1,
          "party enters the source-owned C008 sensor square");
    if (expect_sensor) {
        CHECK(result.sensor_trigger_count == 1 &&
                  result.sensor_last_type == DM1_SENSOR_FLOOR_PARTY_POSSESSION &&
                  result.sensor_last_data == kObjectType &&
                  result.sensor_event_count == 1 &&
                  result.sensor_last_event_type == DM1_EVENT_FAKEWALL &&
                  profile.timeline_queue.eventCount == 1,
              case_name);
        CHECK(memcmp(raw + kWeaponOffset, before + kWeaponOffset, 4) == 0 &&
                  memcmp(raw + kContainerOffset, before + kContainerOffset, 8) == 0,
              "F0274 possession scan does not mutate source object/container bytes");
    } else {
        CHECK(result.sensor_trigger_count == 0 && result.sensor_event_count == 0 &&
                  profile.timeline_queue.eventCount == 0 &&
                  memcmp(raw, before, sizeof(raw)) == 0,
              case_name);
    }
}

int main(void)
{
    run_case((unsigned short)(THING_TYPE_WEAPON << 10), 1,
             "F0274 admits the exact source-owned leader-hand object");
    run_case((unsigned short)(THING_TYPE_CONTAINER << 10), 1,
             "F0274 admits only the leader-hand C144 Slot chain");
    run_case(0xffffu, 0,
             "missing leader-hand identity rejects without raw or timeline mutation");
    run_case((unsigned short)((THING_TYPE_WEAPON << 10) | 1u), 0,
             "stale leader-hand identity rejects without raw or timeline mutation");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
