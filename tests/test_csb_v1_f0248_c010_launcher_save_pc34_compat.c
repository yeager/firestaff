/* ReDMCSB TIMELINE.C F0247/F0248 C010 -> F0261 -> CSB save handoff. */
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
    "/tmp/firestaff_csb_f0248_c010_launcher_save.fsav";

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

static int count_queued_events(const CSB_V1_RuntimeProfile *profile,
                               unsigned char event_type)
{
    int count = 0;
    int index;

    for (index = 0;
         profile && index < profile->timeline_queue.eventCount;
         ++index) {
        int slot = profile->timeline_queue.timeline[index];
        if (slot >= 0 && slot < DM1_EVENT_MAX_COUNT &&
            profile->timeline_queue.events[slot].type == event_type) {
            ++count;
        }
    }
    return count;
}

static void make_fixture(CSB_V1_RuntimeProfile *profile,
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

    /* C06 source wall at (0,1), its C03 has cell zero. F0247 advances north
     * into (0,0), so both C15-associated projectiles remain in-bounds. */
    raw[square_offset(0, 1)] = 0x10u;
    put_le16(raw, 60, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((2u << 7) |
                                        DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION));
    put_le16(raw, 72, (unsigned short)(1u << 2)); /* OnceOnly */
    put_le16(raw, 74, (unsigned short)(7u | (9u << 8)));

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->dungeon_game_id = 0x0731u;
    profile->dungeon_seed = 0xC5B10C10u;
}

static int queue_native_wall_event(CSB_V1_RuntimeProfile *profile, int cell)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile->game_time);
    event.b_mapX = 0;
    event.b_mapY = 1;
    event.c_cell = (unsigned char)cell;
    event.c_effect = DM1_EFFECT_SET;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0;
}

static void test_c010_source_launcher_roundtrips_runtime_save(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeProfile loaded;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];

    remove(kSavePath);
    make_fixture(&profile, &dungeon, raw);
    CHECK(queue_native_wall_event(&profile, 0),
          "C010 fixture queues the source-shaped C06 wall event");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "F0261 dispatches the loaded C010 wall sensor");
    CHECK((read_le16(raw, 70) & 0x007fu) == 0u &&
              profile.projectiles.count == 2 &&
              profile.projectiles.entries[0].projectileSubtype ==
                  PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
              profile.projectiles.entries[1].projectileSubtype ==
                  PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
              profile.projectiles.entries[0].ownerKind ==
                  PROJECTILE_OWNER_LAUNCHER &&
              profile.projectiles.entries[1].ownerKind ==
                  PROJECTILE_OWNER_LAUNCHER &&
              profile.projectiles.entries[0].mapX == 0 &&
              profile.projectiles.entries[0].mapY == 0 &&
              profile.projectiles.entries[0].cell == 2 &&
              profile.projectiles.entries[1].cell == 3 &&
              count_queued_events(&profile, DM1_EVENT_MOVE_PROJECTILE) == 2,
          "F0247 retains C010's two source-owned C15 launcher projectiles and C49 events");
    CHECK(csb_v1_runtime_save_game_to_path(&profile, kSavePath) == 0,
          "C010 runtime state writes through the existing authenticated save handoff");
    csb_v1_runtime_init(&loaded, NULL);
    loaded.dungeon_handle = &dungeon; /* Original Dungeon remains boot-owned. */
    CHECK(csb_v1_runtime_load_game_from_path(&loaded, kSavePath) == 0,
          "C010 runtime save reloads through the existing native receipt");
    CHECK(loaded.projectiles.count == 2 &&
              loaded.projectiles.entries[0].reserved3 != 0 &&
              loaded.projectiles.entries[1].reserved3 != 0 &&
              loaded.projectiles.entries[0].projectileSubtype ==
                  PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
              loaded.projectiles.entries[1].projectileSubtype ==
                  PROJECTILE_SUBTYPE_LIGHTNING_BOLT &&
              loaded.projectiles.entries[0].ownerKind ==
                  PROJECTILE_OWNER_LAUNCHER &&
              loaded.projectiles.entries[1].ownerKind ==
                  PROJECTILE_OWNER_LAUNCHER &&
              count_queued_events(&loaded, DM1_EVENT_MOVE_PROJECTILE) == 2 &&
              (read_le16(raw, 70) & 0x007fu) == 0u,
          "save reload preserves only the emitted C010 runtime state and source OnceOnly byte");
    remove(kSavePath);
}

static void test_c010_wrong_cell_rejects_without_mutation(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[128];
    unsigned char before[128];

    make_fixture(&profile, &dungeon, raw);
    memcpy(before, raw, sizeof(before));
    CHECK(queue_native_wall_event(&profile, 1),
          "wrong-cell C06 event remains structurally queueable");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "wrong-cell C06 event reaches F0261");
    CHECK(profile.projectiles.count == 0 &&
              profile.timeline_queue.eventCount == 0 &&
              memcmp(raw, before, sizeof(raw)) == 0,
          "C010 rejects a cell identity mismatch without raw or projectile mutation");
}

int main(void)
{
    test_c010_source_launcher_roundtrips_runtime_save();
    test_c010_wrong_cell_rejects_without_mutation();
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
