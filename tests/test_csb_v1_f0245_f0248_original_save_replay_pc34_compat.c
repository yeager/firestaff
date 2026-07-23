/* ReDMCSB TIMELINE.C F0245/F0248: original PC34 C005/C006 replay only. */
#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSB_PC34_DUNGEON_MD5 "6695d2acebce49f95db1d8f3a5c733de"

typedef struct {
    int level;
    int x;
    int y;
    int cell;
    unsigned short thing;
    unsigned short before_type_data;
} OriginalRoute;

static unsigned short read_u16(const unsigned char *bytes)
{
    return (unsigned short)(bytes[0] | ((unsigned short)bytes[1] << 8));
}

static int find_original_c005_or_c006(const CSB_V1_DungeonData *dungeon,
                                      OriginalRoute *route)
{
    int level;

    if (!dungeon || !route || dungeon->square_bytes != 1) return 0;
    for (level = 0; level < dungeon->level_count; ++level) {
        int x;
        for (x = 0; x < dungeon->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < dungeon->level_heights[level]; ++y) {
                int raw = csb_v1_dungeon_get_raw_square(dungeon, level, x, y);
                int thing;
                int guard;

                if (raw < 0 || ((raw >> 5) & 7) != 0) continue;
                thing = csb_v1_dungeon_get_first_thing(dungeon, level, x, y);
                for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
                     ++guard) {
                    const unsigned char *record;
                    int type;
                    int size;

                    record = csb_v1_dungeon_get_thing_record(
                        dungeon, (unsigned short)thing, &type, NULL, &size);
                    if (!record || size < 2) break;
                    if (type == 3 && size >= 8) {
                        unsigned short type_data = read_u16(record + 2);
                        unsigned short flags = read_u16(record + 4);
                        int sensor_type = type_data & 0x007f;
                        int sensor_data = type_data >> 7;

                        /* C005 has an unfilled cell bit; C006 has a bounded
                         * positive countdown. Local effects are deliberately
                         * excluded: this probes only F0248's source record
                         * mutation and native queue replay. */
                        if ((flags & (1u << 11)) == 0u &&
                            ((sensor_type == DM1_SENSOR_WALL_AND_OR_GATE &&
                              (sensor_data & (1 << (thing & 3))) == 0) ||
                             (sensor_type == DM1_SENSOR_WALL_COUNTDOWN &&
                              sensor_data > 0 && sensor_data < 511))) {
                            route->level = level;
                            route->x = x;
                            route->y = y;
                            route->cell = thing & 3;
                            route->thing = (unsigned short)thing;
                            route->before_type_data = type_data;
                            return 1;
                        }
                    }
                    thing = csb_v1_dungeon_f0159_get_next_thing_pc34(
                        dungeon, (unsigned short)thing);
                }
            }
        }
    }
    return 0;
}

static int replay_route(CSB_V1_RuntimeProfile *profile,
                        const OriginalRoute *route)
{
    struct DM1_Event_V1 event;

    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(route->level, profile->game_time);
    event.type = DM1_EVENT_WALL;
    event.b_mapX = (unsigned char)route->x;
    event.b_mapY = (unsigned char)route->y;
    event.c_cell = (unsigned char)route->cell;
    event.c_effect = DM1_EFFECT_SET;
    return csb_v1_runtime_add_timeline_event(profile, &event) >= 0 &&
        csb_v1_runtime_tick_v1(profile) == 1;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    const char *save_path = "/tmp/firestaff_csb_c005_c006_original_replay.sav";
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile profile;
    CSB_V1_RuntimeProfile resumed;
    OriginalRoute route;
    const unsigned char *record;
    unsigned short after_type_data;
    int type;
    int size;

    if (!path || !*path) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (!asset_file_matches_md5(path, CSB_PC34_DUNGEON_MD5)) {
        puts("SKIP: FIRESTAFF_CSB_DUNGEON_DAT is not authenticated PC34 data");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        !find_original_c005_or_c006(&dungeon, &route)) {
        puts("SKIP: authenticated Dungeon.dat has no replayable C005/C006 route");
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }

    csb_v1_runtime_init(&profile, NULL);
    profile.chaos_magic.magic_initialized = 1;
    profile.dungeon_handle = &dungeon;
    profile.current_level = route.level;
    profile.dungeon_game_id = 0x0731u;
    profile.game_time = 37u;
    profile.timeline_queue.gameTick = profile.game_time;
    if (csb_v1_runtime_save_game_to_path(&profile, save_path) != 0) {
        fputs("FAIL: cannot save the authenticated pre-replay queue boundary\n", stderr);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    csb_v1_runtime_init(&resumed, NULL);
    resumed.dungeon_handle = &dungeon;
    if (csb_v1_runtime_load_game_from_path(&resumed, save_path) != 0 ||
        resumed.game_time != 37u || resumed.timeline_queue.gameTick != 37u ||
        !replay_route(&resumed, &route)) {
        fputs("FAIL: authenticated C005/C006 save/load replay did not retain timeline identity\n", stderr);
        remove(save_path);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    record = csb_v1_dungeon_get_thing_record(
        &dungeon, route.thing, &type, NULL, &size);
    after_type_data = record && type == 3 && size >= 8 ? read_u16(record + 2) : 0;
    if (after_type_data == route.before_type_data || resumed.game_time != 38u ||
        resumed.timeline_queue.gameTick != 38u ||
        csb_v1_runtime_save_game_to_path(&resumed, save_path) != 0) {
        fputs("FAIL: authenticated C005/C006 replay did not persist source mutation/clock\n", stderr);
        remove(save_path);
        csb_v1_dungeon_free(&dungeon);
        return 1;
    }
    remove(save_path);
    csb_v1_dungeon_free(&dungeon);
    puts("ok: authenticated original C005/C006 route replays across native save/load");
    return 0;
}
