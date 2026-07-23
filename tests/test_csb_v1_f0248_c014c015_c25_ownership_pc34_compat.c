/* ReDMCSB TIMELINE.C F0247/F0248 C014/C015 and PROJEXPL.C F0213 C25. */
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"
#include "memory_projectile_pc34_compat.h"

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
    return (unsigned short)(bytes[offset] |
                            ((unsigned short)bytes[offset + 1] << 8));
}

static int square_offset(int x, int y)
{
    return x * 3 + y;
}

static void init_real_dungeon(CSB_V1_RuntimeProfile *profile,
                              CSB_V1_DungeonData *dungeon,
                              unsigned char raw[128])
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
    dungeon->thing_data_bases[5] = 82;
    dungeon->thing_type_counts[5] = 1;

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->dungeon_seed = 0xC5B14C25u;
}

static int queue_c06(CSB_V1_RuntimeProfile *profile)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    event.b_mapX = 0;
    event.b_mapY = 1;
    event.c_cell = 0;
    event.c_effect = DM1_EFFECT_SET;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0;
}

static int queue_c25(CSB_V1_RuntimeProfile *profile,
                     int slot,
                     int map_x,
                     int map_y,
                     int cell,
                     int explosion_type,
                     unsigned int fire_at)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_EXPLOSION;
    event.priority = (unsigned char)slot;
    event.map_time = DM1_MAP_TIME_MAKE(0, fire_at);
    event.b_mapX = (unsigned char)map_x;
    event.b_mapY = (unsigned char)map_y;
    event.c_cell = (unsigned char)cell;
    event.c_effect = (unsigned char)explosion_type;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0;
}

static void test_c014_transfers_only_loaded_source_object(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];

    init_real_dungeon(&profile, &dungeon, raw);
    /* Wall C014 at (0,1), then its same-cell weapon. F0247 launches north. */
    raw[square_offset(0, 1)] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, (unsigned short)(5u << 10));
    put_le16(raw, 70, (unsigned short)DM1_SENSOR_WALL_SINGLE_PROJ_LAUNCHER_SQUARE_OBJ);
    put_le16(raw, 72, (unsigned short)(1u << 2));
    put_le16(raw, 74, (unsigned short)(7u | (9u << 8)));
    put_le16(raw, 82, 0xfffeu);
    put_le16(raw, 84, 27u);

    CHECK(queue_c06(&profile), "C014 fixture queues a source-shaped C06 event");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "F0248 dispatches C014 from loaded DUNGEON bytes");
    CHECK(profile.projectiles.count == 1 &&
              profile.projectiles.entries[0].ownerKind == PROJECTILE_OWNER_LAUNCHER &&
              (unsigned short)profile.projectiles.entries[0].reserved1 ==
                  (unsigned short)(5u << 10) &&
              read_le16(raw, 68) == 0xfffeu &&
              (read_le16(raw, 70) & 0x007fu) == 0u,
          "C014 unlinks one real source object before its C49-owned projectile");
}

static int create_smoke(CSB_V1_RuntimeProfile *profile, int *out_slot)
{
    struct ExplosionCreateInput_Compat input;
    struct TimelineEvent_Compat first;

    memset(&input, 0, sizeof(input));
    memset(&first, 0, sizeof(first));
    input.explosionType = C040_EXPLOSION_SMOKE;
    input.attack = 100;
    input.mapIndex = 0;
    input.mapX = 0;
    input.mapY = 0;
    input.cell = 0;
    input.currentTick = (int)profile->game_time;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = -1;
    input.creatorProjectileSlot = -1;
    return F0821_EXPLOSION_Create_Compat(
        &input, &profile->explosions, out_slot, &first);
}

static void test_c25_rejects_stale_or_aliased_ownership(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    int slot = -1;

    init_real_dungeon(&profile, &dungeon, raw);
    CHECK(create_smoke(&profile, &slot), "C25 fixture creates one active C15 explosion");
    CHECK(queue_c25(&profile, slot, 1, 0, 0, C040_EXPLOSION_SMOKE, 1u),
          "stale C25 slot event is structurally queueable");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1,
          "C25 reaches its scheduled F0261 tick");
    CHECK(profile.explosions.entries[slot].reserved0 != 0 &&
              profile.explosions.entries[slot].currentFrame == 0 &&
              profile.explosions.entries[slot].scheduledAtTick == 1,
          "C25 rejects an aliased location without advancing a live explosion slot");
}

static void test_c25_accepts_matching_real_square_event(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    int slot = -1;

    init_real_dungeon(&profile, &dungeon, raw);
    CHECK(create_smoke(&profile, &slot), "matching C25 fixture creates one active C15 explosion");
    CHECK(queue_c25(&profile, slot, 0, 0, 0, C040_EXPLOSION_SMOKE, 1u),
          "matching C25 retains the runtime explosion identity");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1 &&
              csb_v1_runtime_tick_v1(&profile) == 1,
          "matching C25 reaches F0220 on its source tick");
    CHECK(profile.explosions.entries[slot].reserved0 == 0 ||
              profile.explosions.entries[slot].currentFrame > 0 ||
              profile.explosions.entries[slot].scheduledAtTick > 1,
          "matching C25 alone may advance the real owned explosion state");
}

int main(void)
{
    test_c014_transfers_only_loaded_source_object();
    test_c25_rejects_stale_or_aliased_ownership();
    test_c25_accepts_matching_real_square_event();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
