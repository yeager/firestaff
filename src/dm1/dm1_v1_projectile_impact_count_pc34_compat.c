#include "dm1_v1_projectile_impact_count_pc34_compat.h"

int F0218_PROJECTILE_GetImpactCount_Compat(
    const struct ProjectileList_Compat *projectiles,
    int mapIndex,
    int mapX,
    int mapY,
    int cell)
{
    int count = 0;
    int slot;

    if (!projectiles || cell < 0 || cell > 3) {
        return 0;
    }

    /* F0218 scans the square's projectile chain.  The compatibility list
     * has no square-local chain, so scan its bounded live records instead. */
    for (slot = 0; slot < PROJECTILE_LIST_CAPACITY; ++slot) {
        const struct ProjectileInstance_Compat *projectile =
            &projectiles->entries[slot];

        if (projectile->slotIndex < 0 || projectile->mapIndex != mapIndex ||
            projectile->mapX != mapX || projectile->mapY != mapY ||
            projectile->cell != cell) {
            continue;
        }
        ++count;
    }

    return count;
}

int dm1_v1_f0218_count_authenticated_c14_at_cell_pc34(
    const struct DungeonDatState_Compat* dungeon,
    const struct DungeonThings_Compat* things,
    const struct ProjectileList_Compat* projectiles,
    int mapIndex, int mapX, int mapY, int cell, int* outCount)
{
    unsigned short thing;
    int count = 0;
    int safety = 0;

    if (outCount) *outCount = 0;
    if (!dungeon || !things || !projectiles || !outCount ||
        cell < 0 || cell > 3 || mapIndex < 0 ||
        mapIndex >= dungeon->header.mapCount) return 0;
    if (!things->loaded || !things->projectiles ||
        !things->rawThingData[THING_TYPE_PROJECTILE]) return 1;
    thing = F0511_DUNGEON_GetSquareFirstThing_Compat(
        dungeon, things, mapIndex, mapX, mapY);
    if (thing == THING_ENDOFLIST) return 0;
    while (thing != THING_NONE && thing != THING_ENDOFLIST && safety++ < 64) {
        int type = THING_GET_TYPE(thing);
        int index = THING_GET_INDEX(thing);
        if (type == THING_TYPE_PROJECTILE) {
            const unsigned char* raw;
            const struct DungeonProjectile_Compat* source;
            const struct ProjectileInstance_Compat* live;
            if (index < 0 || index >= things->projectileCount ||
                index >= things->thingCounts[THING_TYPE_PROJECTILE] ||
                index >= PROJECTILE_LIST_CAPACITY) {
                return 0;
            }
            source = &things->projectiles[index];
            raw = things->rawThingData[THING_TYPE_PROJECTILE] + (size_t)index * 8u;
            live = &projectiles->entries[index];
            if ((unsigned short)(raw[0] | ((unsigned short)raw[1] << 8)) != source->next ||
                (unsigned short)(raw[2] | ((unsigned short)raw[3] << 8)) != source->slot ||
                raw[4] != source->kineticEnergy || raw[5] != source->attack ||
                (unsigned short)(raw[6] | ((unsigned short)raw[7] << 8)) != source->eventIndex) return 0;
            if (THING_GET_CELL(thing) == cell &&
                (live->slotIndex != index || live->reserved3 == 0 ||
                 live->mapIndex != mapIndex || live->mapX != mapX ||
                live->mapY != mapY || live->cell != cell)) return 0;
            if (THING_GET_CELL(thing) == cell) ++count;
        }
        thing = F0512_DUNGEON_GetThingNext_Compat(things, thing);
    }
    if (safety >= 64) return 0;
    *outCount = count;
    return 1;
}
