/* ReDMCSB TIMELINE.C F0248 C006 -> SENSOR.C F0270/F0271 runtime path. */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_f0248_local_effect_runtime_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static unsigned short get_le16(const unsigned char *bytes, int offset)
{
    return (unsigned short)(bytes[offset] | ((unsigned short)bytes[offset + 1] << 8));
}

static void put_local_c006(unsigned char *raw, int offset,
                           unsigned short next, int local_effect)
{
    put_le16(raw, offset, next);
    put_le16(raw, offset + 2, (unsigned short)((1u << 7) |
                                                DM1_SENSOR_WALL_COUNTDOWN));
    put_le16(raw, offset + 4, (unsigned short)(1u << 11));
    put_le16(raw, offset + 6, (unsigned short)local_effect);
}

static void make_dungeon(CSB_V1_DungeonData *dungeon,
                         unsigned char *raw, size_t raw_size)
{
    memset(dungeon, 0, sizeof(*dungeon));
    memset(raw, 0, raw_size);
    dungeon->level_count = 1;
    dungeon->level_widths[0] = 3;
    dungeon->level_heights[0] = 3;
    dungeon->square_bytes = 1;
    dungeon->raw_data = raw;
    dungeon->raw_size = (int)raw_size;
    dungeon->square_first_thing_base = 66;
    dungeon->square_first_thing_count = 1;
    dungeon->thing_data_bases[3] = 68;
    dungeon->thing_type_counts[3] = 3;
    raw[0] = 0x10u;
    put_le16(raw, 60, 0);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_local_c006(raw, 68, (unsigned short)((3u << 10) | 1u),
                   DM1_EFFECT_TOGGLE);
    put_local_c006(raw, 76, (unsigned short)((3u << 10) | 2u),
                   DM1_EFFECT_SET);
    put_local_c006(raw, 84, 0xfffeu,
                   DM1_EFFECT_ADD_300XP_STEAL_SKILL);
}

static int test_receipt(void)
{
    struct SensorTriggerResult_Compat result;
    CSB_V1_F0248LocalEffectReceipt_PC34 receipt;

    memset(&result, 0, sizeof(result));
    result.triggered = 1;
    result.isLocal = 1;
    result.localEffectValue = DM1_EFFECT_ADD_300XP_STEAL_SKILL;
    if (!csb_v1_f0248_local_effect_consume_pc34_compat(
            &result, 4, 5, 2, &receipt) || !receipt.valid ||
        !receipt.award_steal_experience || !receipt.leader_only ||
        csb_v1_f0248_local_effect_consume_pc34_compat(
            &result, 4, 5, 4, &receipt)) {
        fputs("FAIL: F0270 receipt did not enforce local C10 cell semantics\n",
              stderr);
        return 0;
    }
    return 1;
}

static int test_runtime(void)
{
    unsigned char raw[128];
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 event;

    make_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.party_state_valid = 1;
    profile.party_state.ChampionCount = 2;
    profile.party_state.Champions[0].CurrentHealth = 80;
    profile.party_state.Champions[1].CurrentHealth = 80;
    profile.leader_index = 0;
    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.b_mapX = 0;
    event.b_mapY = 0;
    event.c_cell = 0;
    event.c_effect = DM1_EFFECT_CLEAR;
    if (csb_v1_runtime_add_timeline_event(&profile, &event) < 0 ||
        csb_v1_runtime_tick_v1(&profile) != 1 ||
        profile.party_state.Champions[0].SkillExperience[8] != 300u ||
        profile.party_state.Champions[1].SkillExperience[8] != 0u ||
        get_le16(raw, 68) != (unsigned short)((3u << 10) | 1u) ||
        get_le16(raw, 76) != (unsigned short)((3u << 10) | 2u) ||
        get_le16(raw, 84) != 0xfffeu) {
        fputs("FAIL: C006 F0270/F0271 local effects lost source ordering\n",
              stderr);
        return 0;
    }
    return 1;
}

int main(void)
{
    if (!test_receipt() || !test_runtime()) return 1;
    puts("ok: F0248 C006 consumes F0270/F0271 local effect ordering");
    return 0;
}
