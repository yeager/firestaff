#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_combat_pc34_compat.h"

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

static unsigned short sensor_target(int x, int y, int cell)
{
    return (unsigned short)(((cell & 3) << 4) |
                            ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static void queue_projectile_move(CSB_V1_RuntimeProfile *profile,
                                  const struct TimelineEvent_Compat *event)
{
    struct DM1_Event_V1 queued;

    memset(&queued, 0, sizeof(queued));
    queued.type = DM1_EVENT_MOVE_PROJECTILE;
    queued.map_time = DM1_MAP_TIME_MAKE(event->mapIndex, event->fireAtTick);
    queued.priority = (unsigned char)event->aux0;
    queued.b_mapX = (unsigned char)event->mapX;
    queued.b_mapY = (unsigned char)event->mapY;
    queued.c_cell = (unsigned char)event->cell;
    queued.c_effect = (unsigned char)event->aux3;
    CHECK(csb_v1_runtime_add_timeline_event(profile, &queued) >= 0,
          "C49 object move enters the live CSB timeline");
}

static void make_loaded_chain(CSB_V1_DungeonData *dungeon,
                              unsigned char *raw,
                              int wall,
                              int sensor_type,
                              int sensor_data)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 112u);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 112;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 72;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[5] = 80;
    dungeon->thing_type_counts[5] = 1;
    memset(raw, (unsigned char)(1u << 5), 9u);
    raw[0] = (unsigned char)(6u << 5);
    raw[3] = (unsigned char)((wall ? 0u : 1u << 5) | 0x10u);
    put_le16(raw, 60 + 1 * 2, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 72, 0xfffeu);
    put_le16(raw, 74,
             (unsigned short)((sensor_data << 7) | (sensor_type & 0x7f)));
    put_le16(raw, 76, (unsigned short)(DM1_EFFECT_SET << 3));
    put_le16(raw, 78, sensor_target(0, 0, 3));
    put_le16(raw, 80, 0xfffeu);
    put_le16(raw, 82, 27u);
}

static void run_object_move(CSB_V1_RuntimeProfile *profile)
{
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat first_move;
    int slot = -1;

    memset(&input, 0, sizeof(input));
    memset(&first_move, 0, sizeof(first_move));
    input.category = PROJECTILE_CATEGORY_KINETIC;
    input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = 6;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 0;
    input.direction = 0;
    input.kineticEnergy = 10;
    input.attack = 20;
    input.launcherStrength = 20;
    input.stepEnergy = 1;
    input.associatedThing = (int)(5u << 10);
    CHECK(F0810_PROJECTILE_Create_Compat(&input, &profile->projectiles,
                                         &slot, &first_move) == 1 && slot == 0,
          "C49 projectile owns the loaded object Thing");
    queue_projectile_move(profile, &first_move);
    CHECK(csb_v1_runtime_tick_v1(profile) == 1,
          "first C49 boundary advances the projectile");
    CHECK(csb_v1_runtime_tick_v1(profile) == 1,
          "second C49 boundary materializes the object");
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[112];

    make_loaded_chain(&dungeon, raw, 1, 3, 26);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET &&
              profile.timeline_queue.events[0].c_cell == 0u,
          "F0276 C003 object arrival reaches F0272/F0268 from its wall cell");

    make_loaded_chain(&dungeon, raw, 0, 3, 26);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 0,
          "non-wall object arrival cannot borrow C003 wall sensor semantics");

    make_loaded_chain(&dungeon, raw, 1, 3, 27);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 0,
          "C003 rejects the matching object type before Revert/HOLD handling");

    make_loaded_chain(&dungeon, raw, 1, 3, 26);
    put_le16(raw, 76, (unsigned short)((DM1_EFFECT_SET << 3) |
                                        (1u << 2) | (3u << 7)));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].map_time ==
                  DM1_MAP_TIME_MAKE(0, 4u) &&
              get_le16(raw, 74) == (unsigned short)(26u << 7),
          "C003 OnceOnly writes before its three-tick F0272/F0268 delay");

    make_loaded_chain(&dungeon, raw, 1, 1, 0);
    put_le16(raw, 76, (unsigned short)((DM1_EFFECT_SET << 3) | (1u << 6)));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.party_x = 1;
    profile.party_y = 0;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_SET,
          "F0276 C001 object arrival consumes its loaded wall C03 record");
    CHECK(profile.audio_runtime.pendingSoundIndex == CSB_V1_SOUND_SWITCH &&
              profile.audio_runtime.pendingVolume == 3 &&
              profile.audio_runtime.pendingPriority == 15u &&
              profile.audio_runtime.totalRequests == 1u,
          "Audible C001 requests the original prioritized switch sound");

    make_loaded_chain(&dungeon, raw, 1, 1, 0);
    put_le16(raw, 76, (unsigned short)((DM1_EFFECT_HOLD << 3) | (1u << 5)));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              profile.timeline_queue.events[0].c_effect == DM1_EFFECT_CLEAR,
          "C001 AddThing xor Revert maps HOLD arrival to F0268 CLEAR");

    make_loaded_chain(&dungeon, raw, 1, 2, 27);
    put_le16(raw, 76, (unsigned short)((DM1_EFFECT_SET << 3) | (1u << 2)));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 1 &&
              profile.timeline_queue.events[0].type == DM1_EVENT_FAKEWALL &&
              get_le16(raw, 74) == (unsigned short)(27u << 7),
          "C002 OnceOnly clears its type before publishing F0272/F0268");

    make_loaded_chain(&dungeon, raw, 1, 0, 0);
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    run_object_move(&profile);
    CHECK(profile.timeline_queue.eventCount == 0 && raw[74] == 0u &&
              raw[75] == 0u && profile.audio_runtime.totalRequests == 0u,
          "C000 wall sensor stays disabled before object side effects or audio");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
