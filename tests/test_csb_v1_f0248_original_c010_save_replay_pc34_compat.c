/* ReDMCSB TIMELINE.C F0248 C010: authenticated PC34 route only. */
#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "dm1_v1_sensor_trigger_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSB_PC34_DUNGEON_MD5 "6695d2acebce49f95db1d8f3a5c733de"
typedef struct { int level, x, y, cell; } C010Route;

static unsigned short read_u16(const unsigned char *p)
{
    return (unsigned short)(p[0] | ((unsigned short)p[1] << 8));
}

static int find_c010(const CSB_V1_DungeonData *d, C010Route *route)
{
    int level;
    for (level = 0; level < d->level_count; ++level) {
        int x;
        for (x = 0; x < d->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < d->level_heights[level]; ++y) {
                int raw = csb_v1_dungeon_get_raw_square(d, level, x, y);
                int thing = csb_v1_dungeon_get_first_thing(d, level, x, y);
                int guard;
                if (raw < 0 || ((raw >> 5) & 7) != 0) continue;
                for (guard = 0; guard < 128 && thing != 0xfffe && thing != 0xffff;
                     ++guard) {
                    const unsigned char *record;
                    int type, size;
                    record = csb_v1_dungeon_get_thing_record(
                        d, (unsigned short)thing, &type, NULL, &size);
                    if (!record || size < 2) break;
                    if (type == 3 && size >= 8 &&
                        (read_u16(record + 2) & 0x007f) ==
                            DM1_SENSOR_WALL_DOUBLE_PROJ_LAUNCHER_EXPLOSION) {
                        route->level = level; route->x = x; route->y = y;
                        route->cell = thing & 3;
                        return 1;
                    }
                    thing = (int)read_u16(record);
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    const char *save_path = "/tmp/firestaff_csb_c010_original_replay.sav";
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile source, replay;
    struct DM1_Event_V1 event;
    C010Route route;

    if (!path || !*path || !asset_file_matches_md5(path, CSB_PC34_DUNGEON_MD5)) {
        puts("SKIP: authenticated FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        !find_c010(&dungeon, &route)) {
        puts("SKIP: authenticated Dungeon.dat has no C010 route");
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }
    csb_v1_runtime_init(&source, NULL);
    source.dungeon_handle = &dungeon;
    source.chaos_magic.magic_initialized = 1;
    source.dungeon_game_id = 0x0731u;
    source.game_time = 113u;
    source.timeline_queue.gameTick = source.game_time;
    if (csb_v1_runtime_save_game_to_path(&source, save_path) != 0) goto fail;
    csb_v1_runtime_init(&replay, NULL);
    replay.dungeon_handle = &dungeon;
    if (csb_v1_runtime_load_game_from_path(&replay, save_path) != 0 ||
        replay.game_time != 113u || replay.timeline_queue.gameTick != 113u) goto fail;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(route.level, replay.game_time);
    event.type = DM1_EVENT_WALL;
    event.b_mapX = (unsigned char)route.x;
    event.b_mapY = (unsigned char)route.y;
    event.c_cell = (unsigned char)route.cell;
    event.c_effect = DM1_EFFECT_SET;
    if (csb_v1_runtime_add_timeline_event(&replay, &event) < 0 ||
        csb_v1_runtime_tick_v1(&replay) != 1 || replay.game_time != 114u ||
        replay.timeline_queue.gameTick != 114u ||
        csb_v1_runtime_save_game_to_path(&replay, save_path) != 0) goto fail;
    remove(save_path);
    csb_v1_dungeon_free(&dungeon);
    puts("ok: authenticated original C010 event replays across native save/load");
    return 0;
fail:
    fputs("FAIL: authenticated original C010 replay/save identity failed\n", stderr);
    remove(save_path);
    csb_v1_dungeon_free(&dungeon);
    return 1;
}
