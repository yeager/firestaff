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

static int square_offset(int x, int y)
{
    return x * 3 + y;
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
          "C49 move enters the live CSB timeline");
}

int main(void)
{
    CSB_V1_RuntimeProfile profile;
    CSB_V1_DungeonData dungeon;
    struct ProjectileCreateInput_Compat input;
    struct TimelineEvent_Compat first_move;
    unsigned char raw[112];
    int slot = -1;

    memset(&dungeon, 0, sizeof(dungeon));
    memset(raw, 0, sizeof(raw));
    dungeon.level_count = 1;
    dungeon.level_widths[0] = 3;
    dungeon.level_heights[0] = 3;
    dungeon.square_bytes = 1;
    dungeon.raw_data = raw;
    dungeon.raw_size = (int)sizeof(raw);
    dungeon.square_first_thing_base = 66;
    dungeon.square_first_thing_count = 1;
    dungeon.thing_data_bases[3] = 72;
    dungeon.thing_type_counts[3] = 2;
    dungeon.thing_data_bases[5] = 88;
    dungeon.thing_type_counts[5] = 1;
    memset(raw, (unsigned char)(1u << 5), 9u);
    raw[square_offset(1, 0)] = (unsigned char)((1u << 5) | 0x10u);
    put_le16(raw, 60 + 1 * 2, 0u);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 72, (unsigned short)((3u << 10) | 1u));
    put_le16(raw, 74,
             (unsigned short)((27u << 7) | DM1_SENSOR_FLOOR_OBJECT));
    put_le16(raw, 76, (unsigned short)((DM1_EFFECT_SET << 3) | (1u << 11)));
    put_le16(raw, 78, 10u);
    put_le16(raw, 80, 0xfffeu);
    put_le16(raw, 82, DM1_SENSOR_FLOOR_THERON_PARTY_CREATURE);
    put_le16(raw, 84, 0u);
    put_le16(raw, 86, 0u);
    put_le16(raw, 88, 0xfffeu);
    put_le16(raw, 90, 27u);

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.champion_count = 2;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 2;
    profile.party_state.Champions[0].CurrentHealth = 100;
    profile.party_state.Champions[0].SkillExperienceValid = 1u;
    profile.party_state.Champions[0].SkillExperience[8] = 40u;
    profile.party_state.Champions[0].SkillExperience[1] = 80u;
    profile.party_state.Champions[1].CurrentHealth = 100;
    profile.party_state.Champions[1].SkillExperienceValid = 1u;
    profile.party_state.Champions[1].SkillExperience[8] = 50u;
    profile.party_state.Champions[1].SkillExperience[1] = 90u;

    memset(&input, 0, sizeof(input));
    memset(&first_move, 0, sizeof(first_move));
    input.category = PROJECTILE_CATEGORY_KINETIC;
    input.subtype = PROJECTILE_SUBTYPE_KINETIC_ARROW;
    input.ownerKind = PROJECTILE_OWNER_LAUNCHER;
    input.ownerIndex = 7;
    input.mapIndex = 0;
    input.mapX = 1;
    input.mapY = 0;
    input.direction = 0;
    input.kineticEnergy = 10;
    input.attack = 20;
    input.launcherStrength = 20;
    input.stepEnergy = 1;
    input.associatedThing = (int)(5u << 10);
    CHECK(F0810_PROJECTILE_Create_Compat(&input, &profile.projectiles,
                                         &slot, &first_move) == 1 && slot == 0,
          "C49 projectile owns an object for C004 C10 placement");
    queue_projectile_move(&profile, &first_move);
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "first C49 boundary advances the projectile");
    CHECK(csb_v1_runtime_tick_v1(&profile) == 1,
          "second C49 boundary materializes the object on C10");
    CHECK(profile.party_state.Champions[0].SkillExperience[8] == 190u &&
              profile.party_state.Champions[1].SkillExperience[8] == 200u,
          "C10 divides 300 Steal XP by both party champions");
    CHECK(profile.party_state.Champions[0].SkillExperience[1] == 230u &&
              profile.party_state.Champions[1].SkillExperience[1] == 240u,
          "C10 credits the ReDMCSB hidden-skill base Ninja XP");
    CHECK(profile.party_state.Champions[0].SkillTemporaryExperience[8] == 18 &&
              profile.party_state.Champions[1].SkillTemporaryExperience[8] == 18,
          "C10 applies F0304 temporary experience to Steal");
    CHECK(profile.timeline_queue.eventCount == 0,
          "C10 LocalEffect does not queue an F0268 remote event");

    printf("PASSED: %d\nFAILED: %d\n", passed, failed);
    return failed ? 1 : 0;
}
