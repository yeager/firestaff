/* ReDMCSB GROUP1.C F0175 + PROJEXPL.C F0217/F0215 through a C05 chain. */
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_f0217_group_lookup_pc34_compat.h"

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

static void make_fixture(CSB_V1_RuntimeProfile *profile,
                         CSB_V1_DungeonData *dungeon,
                         unsigned char raw[208])
{
    int i;

    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, 208u);
    dungeon->level_count = 2;
    dungeon->level_widths[0] = dungeon->level_widths[1] = 3;
    dungeon->level_heights[0] = dungeon->level_heights[1] = 3;
    dungeon->level_offsets[0] = 0;
    dungeon->level_offsets[1] = 9;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = 208;
    dungeon->square_first_thing_base = 100;
    dungeon->square_first_thing_count = 2;
    dungeon->thing_data_bases[1] = 140;
    dungeon->thing_type_counts[1] = 1;
    dungeon->thing_data_bases[3] = 146;
    dungeon->thing_type_counts[3] = 1;
    dungeon->thing_data_bases[4] = 160;
    dungeon->thing_type_counts[4] = 1;
    dungeon->thing_data_bases[5] = 180;
    dungeon->thing_type_counts[5] = 1;
    for (i = 0; i < 18; ++i) raw[i] = (unsigned char)(1u << 5);

    /* Open source C05: map 0 (1,0) -> map 1 (0,0). */
    raw[3] = (unsigned char)((5u << 5) | 0x18u);
    put_le16(raw, 78, 0u);
    put_le16(raw, 100, (unsigned short)(1u << 10));
    put_le16(raw, 140, 0xfffeu);
    put_le16(raw, 142, 0u);
    put_le16(raw, 144, 0x0100u);

    /* Target corridor retains a C03 before C04, as F0175 supports. */
    raw[9] = (unsigned char)((1u << 5) | 0x10u);
    put_le16(raw, 82, 1u);
    put_le16(raw, 102, (unsigned short)(3u << 10));
    put_le16(raw, 146, (unsigned short)(4u << 10));
    put_le16(raw, 160, 0xfffeu);
    put_le16(raw, 162, 0xfffeu);
    raw[164] = 3u;
    raw[165] = 0xffu;
    put_le16(raw, 166, 500u);
    put_le16(raw, 174, 0u);
    put_le16(raw, 180, 0xfffeu);
    put_le16(raw, 182, 27u);

    csb_v1_runtime_init(profile, NULL);
    profile->chaos_magic.magic_initialized = 1;
    profile->dungeon_handle = dungeon;
    profile->dungeon_seed = 0xF0247017u;
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    unsigned char raw[208];
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat first_move;
    struct DM1_Event_V1 event;
    int slot = -1;
    uint16_t group_thing = 0xffffu;

    make_fixture(&profile, &dungeon, raw);
    CHECK(csb_v1_f0217_find_group_thing_pc34_compat(
              &dungeon, 1, 0, 0, &group_thing) == 1 &&
              group_thing == (uint16_t)(4u << 10),
          "F0175 scans past target C03 to the real C04");
    memset(&input, 0, sizeof(input));
    input.category = PROJECTILE_CATEGORY_KINETIC;
    input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 1;
    input.cell = 0;
    input.direction = 0;
    input.kineticEnergy = 20;
    input.attack = 20;
    input.launcherStrength = 20;
    input.stepEnergy = 1;
    input.associatedThing = (int)(5u << 10);
    CHECK(F0810_PROJECTILE_Create_Compat(
              &input, &profile.projectiles, &slot, &first_move) == 1 &&
              slot == 0,
          "C49 fixture creates an arrow-associated C14");
    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_MOVE_PROJECTILE;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.priority = (unsigned char)first_move.aux0;
    event.b_mapX = (unsigned char)first_move.mapX;
    event.b_mapY = (unsigned char)first_move.mapY;
    event.c_cell = (unsigned char)first_move.cell;
    event.c_effect = (unsigned char)first_move.aux3;
    CHECK(csb_v1_runtime_add_timeline_event(&profile, &event) >= 0,
          "C49 fixture queues a source-shaped projectile event");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "C49 follows C05 before scanning the destination Thing chain");
    CHECK(profile.projectiles.count == 0 &&
              read_le16(raw, 102) == (unsigned short)(3u << 10) &&
              read_le16(raw, 160) == (unsigned short)(0xd400u) &&
              read_le16(raw, 180) == 0xfffeu,
          "F0215 retains the arrow once in the destination C03/C04 Thing chain");
    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
