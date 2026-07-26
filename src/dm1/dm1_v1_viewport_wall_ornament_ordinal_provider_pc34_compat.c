#include "dm1_v1_viewport_wall_ornament_ordinal_provider_pc34_compat.h"
#include "dm1_v1_random_ornament_pc34_compat.h"

/*
 * Resolve the wall ornament ordinal for a dungeon square.
 *
 * Phase 1: scan sensor things at (map_x, map_y) for an explicit ordinal.
 * Phase 2: compute random ornament from map metadata if no sensor found.
 *
 * Returns: 1-based ordinal, or -1 if no ornament.
 * Source: ReDMCSB DUNGEON.C F0172 (sensor scan) + F0170/F0171 (random).
 */
int dm1_v1_viewport_wall_ornament_ordinal_resolve_pc34(
    void *user_data,
    int map_x, int map_y)
{
    const DM1_V1_WallOrnamentOrdinalProviderPc34 *prov =
        (const DM1_V1_WallOrnamentOrdinalProviderPc34 *)user_data;

    if (!prov || !prov->things) return -1;

    /* Phase 1: sensor-placed ornaments override random ones. */
    if (prov->things->sensors &&
        prov->things->squareFirstThings &&
        map_x >= 0 && map_y >= 0 &&
        prov->mapWidth > 0 && prov->mapHeight > 0) {
        int squareIndex = map_y * prov->mapWidth + map_x;
        if (squareIndex >= 0 &&
            squareIndex < prov->things->squareFirstThingCount) {
            uint16_t thing = prov->things->squareFirstThings[squareIndex];
            int safety = 0;
            while (thing != THING_ENDOFLIST && thing != THING_NONE &&
                   safety < 64) {
                if (THING_GET_TYPE(thing) == THING_TYPE_SENSOR) {
                    int sIdx = (int)THING_GET_INDEX(thing);
                    if (sIdx >= 0 && sIdx < prov->things->sensorCount) {
                        int ord = (int)prov->things->sensors[sIdx].ornamentOrdinal;
                        if (ord > 0) {
                            return ord;
                        }
                    }
                }
                /* Walk the thing list via nextThing links. */
                {
                    int tType = (int)THING_GET_TYPE(thing);
                    int tIdx = (int)THING_GET_INDEX(thing);
                    uint16_t next = THING_NONE;
                    switch (tType) {
                        case THING_TYPE_DOOR:
                            if (prov->things->doors &&
                                tIdx < prov->things->doorCount)
                                next = prov->things->doors[tIdx].next;
                            break;
                        case THING_TYPE_TELEPORTER:
                            if (prov->things->teleporters &&
                                tIdx < prov->things->teleporterCount)
                                next = prov->things->teleporters[tIdx].next;
                            break;
                        case THING_TYPE_TEXTSTRING:
                            if (prov->things->textStrings &&
                                tIdx < prov->things->textStringCount)
                                next = prov->things->textStrings[tIdx].next;
                            break;
                        case THING_TYPE_SENSOR:
                            if (prov->things->sensors &&
                                tIdx < prov->things->sensorCount)
                                next = prov->things->sensors[tIdx].next;
                            break;
                        default:
                            next = THING_NONE;
                            break;
                    }
                    thing = next;
                }
                ++safety;
            }
        }
    }

    /* Phase 2: random ornament from map metadata. */
    if (prov->randomWallOrnamentCount > 0) {
        int rndOrd = dm1_v1_dungeon_get_random_ornament_ordinal_pc34(
            1,
            prov->randomWallOrnamentCount,
            map_x, map_y,
            prov->mapIndex,
            prov->mapWidth,
            prov->mapHeight,
            prov->ornamentRandomSeed,
            30);
        if (rndOrd > 0) {
            return rndOrd;
        }
    }

    return -1;
}
