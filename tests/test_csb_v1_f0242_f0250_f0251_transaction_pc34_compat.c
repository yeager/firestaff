/* ReDMCSB TIMELINE.C F0242/F0250/F0251 real-square transaction guards. */
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static int passed;
static int failed;

#define CHECK(condition, message) do { \
    if (condition) { ++passed; printf("  PASS: %s\n", message); } \
    else { ++failed; printf("  FAIL: %s\n", message); } \
} while (0)

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

static void make_dungeon(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[64])
{
    int i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 64u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 64;
    for (i = 0; i < 9; ++i) raw[i] = (unsigned char)(1u << 5);

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
}

static int queue_event(CSB_V1_RuntimeProfile *profile,
                       int type,
                       int effect,
                       int x,
                       int y)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = (unsigned char)type;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    event.b_mapX = (unsigned char)x;
    event.b_mapY = (unsigned char)y;
    event.c_effect = (unsigned char)effect;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0;
}

static void test_fakewall_clear_defers_then_commits(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[64];
    int offset = square_offset(0, 1);

    make_dungeon(&profile, &dungeon, raw);
    raw[offset] = (unsigned char)((6u << 5) | 0x04u);
    profile.current_level = 0;
    profile.party_x = 0;
    profile.party_y = 1;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 1;

    CHECK(queue_event(&profile, DM1_EVENT_FAKEWALL, DM1_EFFECT_CLEAR, 0, 1),
          "C07 fixture queues a real fakewall clear");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "F0242 sees the party occupancy on its source tick");
    CHECK((raw[offset] & 0x04u) != 0u && profile.timeline_queue.eventCount == 1,
          "F0242 leaves the fakewall open and requeues exactly one clear");

    profile.party_x = 1;
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "the deferred F0242 event reaches the next source tick");
    CHECK((raw[offset] & 0x04u) == 0u && profile.timeline_queue.eventCount == 0,
          "F0242 clears only after the source square is unoccupied");
}

static void test_teleporter_and_pit_state_are_real_square_only(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[64];
    int teleporter = square_offset(1, 1);
    int pit = square_offset(2, 1);
    unsigned char before;

    make_dungeon(&profile, &dungeon, raw);
    raw[teleporter] = (unsigned char)(5u << 5);
    raw[pit] = (unsigned char)(2u << 5);
    CHECK(queue_event(&profile, DM1_EVENT_TELEPORTER, DM1_EFFECT_SET, 1, 1) &&
              csb_v1_runtime_tick_v1(&profile) == 1,
          "F0250 processes a loaded teleporter square");
    CHECK((raw[teleporter] & 0x08u) != 0u,
          "F0250 commits OPEN before optional resident movement");
    CHECK(queue_event(&profile, DM1_EVENT_PIT, DM1_EFFECT_TOGGLE, 2, 1) &&
              csb_v1_runtime_tick_v1(&profile) == 1,
          "F0251 processes a loaded pit square");
    CHECK((raw[pit] & 0x08u) != 0u,
          "F0251 resolves TOGGLE then commits OPEN");

    before = raw[teleporter];
    CHECK(queue_event(&profile, DM1_EVENT_PIT, DM1_EFFECT_CLEAR, 1, 1) &&
              csb_v1_runtime_tick_v1(&profile) == 1,
          "mismatched C09 event remains queueable and dispatchable");
    CHECK(raw[teleporter] == before,
          "mismatched event type never mutates a real teleporter square");
}

int main(void)
{
    test_fakewall_clear_defers_then_commits();
    test_teleporter_and_pit_state_are_real_square_only();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
