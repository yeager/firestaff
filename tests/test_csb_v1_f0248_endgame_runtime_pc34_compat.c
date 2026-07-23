/* ReDMCSB TIMELINE.C F0248 C018 -> ENDGAME.C F0666/F0444 runtime route. */
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_f0248_endgame_runtime_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <string.h>

static void put_le16(unsigned char *bytes, int offset, unsigned short value)
{
    bytes[offset] = (unsigned char)(value & 0xffu);
    bytes[offset + 1] = (unsigned char)(value >> 8);
}

static void make_c018_dungeon(CSB_V1_DungeonData *dungeon,
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
    raw[0] = 0x10u;
    put_le16(raw, 60, 0);
    put_le16(raw, 66, (unsigned short)(3u << 10));
    put_le16(raw, 68, 0xfffeu);
    put_le16(raw, 70, (unsigned short)DM1_SENSOR_WALL_END_GAME);
    put_le16(raw, 72, (unsigned short)(3u << 7));
    put_le16(raw, 74, 0);
}

static int test_receipt_consumer(void)
{
    struct DungeonSensor_Compat sensor;
    struct SensorTriggerResult_Compat result;
    CSB_V1_F0248EndgameRuntimeReceipt_PC34 receipt;

    memset(&sensor, 0, sizeof(sensor));
    sensor.sensorType = DM1_SENSOR_WALL_END_GAME;
    sensor.value = 3;
    memset(&result, 0, sizeof(result));
    if (!F0731_SENSOR_EvaluateWallEndGameEvent_Compat(
            &sensor, 0, DM1_EFFECT_SET, 2, &result) ||
        !csb_v1_f0248_endgame_consume_pc34_compat(&result, 0, &receipt) ||
        !receipt.valid || receipt.delay_ticks != 180 ||
        !receipt.palette_curtain || !receipt.restart_disabled ||
        !receipt.game_won || !receipt.f0666_entered ||
        !receipt.startend_endgame_requested ||
        csb_v1_f0248_endgame_consume_pc34_compat(&result, 1, &receipt)) {
        fputs("FAIL: C018 receipt did not consume the source endgame sequence\n",
              stderr);
        return 0;
    }
    return 1;
}

static int test_runtime_consumer(void)
{
    unsigned char raw[128];
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    struct DM1_Event_V1 event;

    make_c018_dungeon(&dungeon, raw, sizeof(raw));
    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.state = CSB_STATE_DUNGEON;
    memset(&event, 0, sizeof(event));
    event.type = DM1_EVENT_WALL;
    event.map_time = DM1_MAP_TIME_MAKE(0, profile.game_time);
    event.b_mapX = 0;
    event.b_mapY = 0;
    event.c_cell = 2;
    event.c_effect = DM1_EFFECT_SET;
    if (csb_v1_runtime_add_timeline_event(&profile, &event) < 0 ||
        csb_v1_runtime_tick_v1(&profile) != 1 || !profile.victory ||
        profile.state != CSB_STATE_VICTORY || profile.game_over ||
        csb_v1_runtime_tick_v1(&profile) != 0) {
        fputs("FAIL: C018 runtime did not promote the endgame receipt\n", stderr);
        return 0;
    }
    return 1;
}

int main(void)
{
    if (!test_receipt_consumer() || !test_runtime_consumer()) return 1;
    puts("ok: F0248 C018 consumes F0731 through ENDGAME.C runtime receipt");
    return 0;
}
