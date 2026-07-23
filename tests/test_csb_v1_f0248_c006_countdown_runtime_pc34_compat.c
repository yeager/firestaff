/* ReDMCSB TIMELINE.C F0248 C005/C006 -> F0730/F0729 -> delayed F0268 path. */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static int square_offset(int x, int y) { return x * 3 + y; }

static unsigned short sensor_target(int x, int y, int cell)
{
    return (unsigned short)(((cell & 3) << 4) | ((x & 0x1f) << 6) |
                            ((y & 0x1f) << 11));
}

static void make_c006_dungeon(CSB_V1_DungeonData *dungeon,
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
    dungeon->thing_type_counts[3] = 1;

    raw[square_offset(0, 0)] = 0x10u;
    raw[square_offset(1, 0)] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)((1u << 7) |
                                        DM1_SENSOR_WALL_COUNTDOWN));
    put_le16(raw, 72, (unsigned short)((DM1_EFFECT_SET << 3) |
                                        (1u << 6) | (2u << 7)));
    put_le16(raw, 74, sensor_target(1, 0, 0));
}

static void make_c005_dungeon(CSB_V1_DungeonData *dungeon,
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
    dungeon->thing_type_counts[3] = 1;

    raw[square_offset(0, 0)] = 0x10u;
    raw[square_offset(1, 0)] = (unsigned char)(6u << 5);
    put_le16(raw, 60, 0);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    /* C005: reference mask 0001, current mask 0000.  SET on cell 0
     * matches it, triggers F0272 after Remote.Value=2, and is once-only. */
    put_le16(raw, 70, (unsigned short)((0x10u << 7) |
                                        DM1_SENSOR_WALL_AND_OR_GATE));
    put_le16(raw, 72, (unsigned short)((1u << 2) | (1u << 6) |
                                        (2u << 7)));
    put_le16(raw, 74, sensor_target(1, 0, 0));
}

static int run_c005_gate_regression(void)
{
    unsigned char raw[128];
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 event;

    make_c005_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.b_mapX = 0;
    event.b_mapY = 0;
    event.c_cell = 0;
    event.c_effect = DM1_EFFECT_SET;
    if (csb_v1_runtime_add_timeline_event(&profile, &event) < 0 ||
        csb_v1_runtime_tick_v1(&profile) != 1 ||
        (raw[square_offset(1, 0)] & 0x04u) != 0 ||
        (raw[70] & 0x007fu) != 0 ||
        profile.audio_runtime.pendingSoundIndex != CSB_V1_SOUND_SWITCH ||
        profile.audio_runtime.totalRequests != 1u) {
        fputs("FAIL: C005 did not consume F0730 live target/mutation receipt\n",
              stderr);
        return 0;
    }
    if (csb_v1_runtime_tick_v1(&profile) != 1 ||
        (raw[square_offset(1, 0)] & 0x04u) != 0) {
        fputs("FAIL: C005 dispatched its F0268 target too early\n", stderr);
        return 0;
    }
    if (csb_v1_runtime_tick_v1(&profile) != 1 ||
        (raw[square_offset(1, 0)] & 0x04u) == 0) {
        fputs("FAIL: C005 did not dispatch its F0268 target at Remote.Value\n",
              stderr);
        return 0;
    }
    return 1;
}

int main(void)
{
    unsigned char raw[128];
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 event;

    make_c006_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.b_mapX = 0;
    event.b_mapY = 0;
    event.c_cell = 0;
    event.c_effect = DM1_EFFECT_CLEAR;
    if (csb_v1_runtime_add_timeline_event(&profile, &event) < 0 ||
        csb_v1_runtime_tick_v1(&profile) != 1 ||
        (raw[square_offset(1, 0)] & 0x04u) != 0 ||
        ((raw[70] >> 7) & 0x01u) != 0 ||
        profile.audio_runtime.pendingSoundIndex != CSB_V1_SOUND_SWITCH ||
        profile.audio_runtime.totalRequests != 1u) {
        fputs("FAIL: C006 did not apply F0729 raw mutation/audio/delay\n", stderr);
        return 1;
    }
    if (csb_v1_runtime_tick_v1(&profile) != 1 ||
        (raw[square_offset(1, 0)] & 0x04u) != 0) {
        fputs("FAIL: C006 dispatched its delayed F0268 target too early\n", stderr);
        return 1;
    }
    if (csb_v1_runtime_tick_v1(&profile) != 1 ||
        (raw[square_offset(1, 0)] & 0x04u) == 0) {
        fputs("FAIL: C006 did not dispatch its F0268 target at Remote.Value\n", stderr);
        return 1;
    }
    if (!run_c005_gate_regression()) return 1;
    puts("ok: F0248 C005/C006 consume F0730/F0729 live delay/audio receipts");
    return 0;
}
