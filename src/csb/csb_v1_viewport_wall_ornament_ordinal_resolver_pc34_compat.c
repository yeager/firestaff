#include "csb_v1_viewport_wall_ornament_ordinal_resolver_pc34_compat.h"
#include "csb_v1_dungeon_loader_pc34_compat.h"
#include "dm1_v1_random_ornament_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

int csb_v1_viewport_wall_ornament_ordinal_resolve_pc34(
    void *user_data,
    int map_x, int map_y)
{
    const CSB_V1_WallOrnamentOrdinalResolverPc34 *res =
        (const CSB_V1_WallOrnamentOrdinalResolverPc34 *)user_data;
    const CSB_V1_DungeonData *d;
    int level;

    if (!res || !res->dungeon) return -1;

    d = (const CSB_V1_DungeonData *)res->dungeon;
    level = res->level;

    if (level < 0 || level >= d->level_count) return -1;

    {
        int first = csb_v1_dungeon_get_first_thing(d, level, map_x, map_y);
        if (first >= 0) {
            uint16_t thing = (uint16_t)first;
            int safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                   safety < 64) {
                int type = 0, index = 0, size = 0;
                const uint8_t *rec = csb_v1_dungeon_get_thing_record(
                    d, thing, &type, &index, &size);
                if (rec && type == THING_TYPE_SENSOR && size >= 6) {
                    uint16_t bf = (uint16_t)(rec[4] | ((uint16_t)rec[5] << 8));
                    int ord = (int)((bf >> 12) & 0x0Fu);
                    if (ord > 0) {
                        return ord;
                    }
                }
                thing = csb_v1_dungeon_f0159_get_next_thing_pc34(d, thing);
                ++safety;
            }
        }
    }

    if (res->randomWallOrnamentCount > 0) {
        int mapWidth = d->level_widths[level];
        int mapHeight = d->level_heights[level];
        int mapIndex = d->map_levels[level];
        int rndOrd = dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
            1,
            res->randomWallOrnamentCount,
            map_x, map_y,
            mapIndex,
            mapWidth,
            mapHeight,
            res->ornamentRandomSeed,
            30);
        if (rndOrd > 0) {
            return rndOrd;
        }
    }

    return -1;
}
