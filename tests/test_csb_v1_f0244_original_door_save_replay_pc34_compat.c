/* ReDMCSB TIMELINE.C F0244/F0241: no fixture or replacement dungeon path. */
#include "asset_find_by_hash.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "csb_v1_runtime_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define CSB_PC34_DUNGEON_MD5 "6695d2acebce49f95db1d8f3a5c733de"

typedef struct { int level, x, y; unsigned char before; } DoorRoute;

static int complete_chain(const CSB_V1_DungeonData *d, int level, int x, int y)
{
    int thing = csb_v1_dungeon_get_first_thing(d, level, x, y);
    int guard;
    if (thing < 0) return 1;
    for (guard = 0; guard < 128; ++guard) {
        const unsigned char *record;
        int type, size;
        if (thing == 0xfffe) return 1;
        if (thing == 0xffff) return 0;
        record = csb_v1_dungeon_get_thing_record(
            d, (unsigned short)thing, &type, NULL, &size);
        if (!record || size < 2 || (type == 2 && size < 4) ||
            (type == 3 && size < 8)) return 0;
        thing = (int)(record[0] | ((unsigned short)record[1] << 8));
    }
    return 0;
}

static int find_door(const CSB_V1_DungeonData *d, DoorRoute *route)
{
    int level;
    for (level = 0; level < d->level_count; ++level) {
        int x;
        for (x = 0; x < d->level_widths[level]; ++x) {
            int y;
            for (y = 0; y < d->level_heights[level]; ++y) {
                int raw = csb_v1_dungeon_get_raw_square(d, level, x, y);
                if (raw >= 0 && ((raw >> 5) & 7) == 4 &&
                    (raw & 7) > 0 && (raw & 7) < 5 &&
                    complete_chain(d, level, x, y)) {
                    route->level = level; route->x = x; route->y = y;
                    route->before = (unsigned char)raw;
                    return 1;
                }
            }
        }
    }
    return 0;
}

int main(void)
{
    const char *path = getenv("FIRESTAFF_CSB_DUNGEON_DAT");
    const char *save_path = "/tmp/firestaff_csb_f0244_original_replay.sav";
    CSB_V1_DungeonData dungeon;
    CSB_V1_RuntimeProfile before, resumed;
    struct DM1_Event_V1 event;
    DoorRoute route;
    int after;

    if (!path || !*path || !asset_file_matches_md5(path, CSB_PC34_DUNGEON_MD5)) {
        puts("SKIP: authenticated FIRESTAFF_CSB_DUNGEON_DAT is not configured");
        return 0;
    }
    if (csb_v1_dungeon_load_from_file(&dungeon, path) != 0 ||
        !find_door(&dungeon, &route)) {
        puts("SKIP: authenticated Dungeon.dat has no replayable F0244 door");
        csb_v1_dungeon_free(&dungeon);
        return 0;
    }
    csb_v1_runtime_init(&before, NULL);
    before.dungeon_handle = &dungeon;
    before.chaos_magic.magic_initialized = 1;
    before.dungeon_game_id = 0x0731u;
    before.game_time = 91u;
    before.timeline_queue.gameTick = before.game_time;
    if (csb_v1_runtime_save_game_to_path(&before, save_path) != 0) goto fail;
    csb_v1_runtime_init(&resumed, NULL);
    resumed.dungeon_handle = &dungeon;
    if (csb_v1_runtime_load_game_from_path(&resumed, save_path) != 0 ||
        resumed.game_time != 91u || resumed.timeline_queue.gameTick != 91u) goto fail;
    memset(&event, 0, sizeof(event));
    event.map_time = DM1_MAP_TIME_MAKE(route.level, resumed.game_time);
    event.type = DM1_EVENT_DOOR;
    event.b_mapX = (unsigned char)route.x;
    event.b_mapY = (unsigned char)route.y;
    event.c_effect = DM1_EFFECT_SET;
    if (csb_v1_runtime_add_timeline_event(&resumed, &event) < 0 ||
        csb_v1_runtime_tick_v1(&resumed) != 1) goto fail;
    after = csb_v1_dungeon_get_raw_square(&dungeon, route.level, route.x, route.y);
    if (after < 0 || (after & 7) >= (route.before & 7) ||
        resumed.game_time != 92u || resumed.timeline_queue.gameTick != 92u ||
        csb_v1_runtime_save_game_to_path(&resumed, save_path) != 0) goto fail;
    remove(save_path);
    csb_v1_dungeon_free(&dungeon);
    puts("ok: authenticated original F0244 door replays across native save/load");
    return 0;
fail:
    fputs("FAIL: authenticated original F0244 door replay/save identity failed\n", stderr);
    remove(save_path);
    csb_v1_dungeon_free(&dungeon);
    return 1;
}
