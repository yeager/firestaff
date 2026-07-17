#include "dm1_v1_c14_layout_pc34_compat.h"

#include <string.h>

int dm1_v1_c14_pool_reserve_pc34(struct DungeonThings_Compat* things,
                                  DM1_C14PoolReservationPc34* out)
{
    int index;
    unsigned short thing;
    unsigned char* raw;
    if (!things || !out || !things->loaded || !things->projectiles ||
        !things->rawThingData[THING_TYPE_PROJECTILE] ||
        things->projectileCount != things->thingCounts[THING_TYPE_PROJECTILE]) return 0;
    memset(out, 0, sizeof(*out));
    for (index = 0; index < things->projectileCount; ++index) {
        raw = things->rawThingData[THING_TYPE_PROJECTILE] + (size_t)index * 8u;
        if (raw[0] == 0xff && raw[1] == 0xff) break;
    }
    if (index >= things->projectileCount) return 0;
    memcpy(out->raw, raw, 8); out->decoded = things->projectiles[index];
    thing = F0516_DUNGEON_GetUnusedThing_Compat(things, THING_TYPE_PROJECTILE, NULL, NULL);
    if (thing == THING_NONE || THING_GET_INDEX(thing) != index) return 0;
    out->things = things; out->thing = thing; out->active = 1;
    return 1;
}

int dm1_v1_c14_pool_rollback_pc34(DM1_C14PoolReservationPc34* r)
{
    int index;
    unsigned char* raw;
    if (!r || !r->active || !r->things) return 0;
    if (r->linked && (!r->dungeon || !F0515_DUNGEON_UnlinkThingFromList_Compat(
            r->dungeon, r->things, r->thing, THING_ENDOFLIST, r->mapIndex, r->mapX, r->mapY))) return 0;
    index = THING_GET_INDEX(r->thing);
    if (index < 0 || index >= r->things->projectileCount) return 0;
    raw = r->things->rawThingData[THING_TYPE_PROJECTILE] + (size_t)index * 8u;
    memcpy(raw, r->raw, 8); r->things->projectiles[index] = r->decoded;
    r->active = r->linked = 0;
    return 1;
}

int dm1_v1_c14_pool_initialize_and_link_pc34(DM1_C14PoolReservationPc34* r,
    struct DungeonDatState_Compat* dungeon, unsigned short slot, int kinetic, int attack,
    unsigned short event_index, int cell, int map, int x, int y)
{
    int index; unsigned char* raw; unsigned short thing;
    if (!r || !r->active || !dungeon || !dungeon->tilesLoaded || kinetic < 0 || kinetic > 255 ||
        attack < 0 || attack > 255 || cell < 0 || cell > 3) goto fail;
    index = THING_GET_INDEX(r->thing);
    if (index < 0 || index >= r->things->projectileCount) goto fail;
    raw = r->things->rawThingData[THING_TYPE_PROJECTILE] + (size_t)index * 8u;
    raw[2]=(unsigned char)slot; raw[3]=(unsigned char)(slot>>8); raw[4]=(unsigned char)kinetic;
    raw[5]=(unsigned char)attack; raw[6]=(unsigned char)event_index; raw[7]=(unsigned char)(event_index>>8);
    r->things->projectiles[index].slot=slot; r->things->projectiles[index].kineticEnergy=(unsigned char)kinetic;
    r->things->projectiles[index].attack=(unsigned char)attack; r->things->projectiles[index].eventIndex=event_index;
    thing=(unsigned short)(r->thing | ((unsigned short)cell<<14));
    if (!F0514_DUNGEON_LinkThingToList_Compat(dungeon,r->things,thing,THING_ENDOFLIST,map,x,y)) goto fail;
    r->thing=thing; r->dungeon=dungeon; r->mapIndex=map; r->mapX=x; r->mapY=y; r->linked=1; return 1;
fail:
    if (r && r->active) (void)dm1_v1_c14_pool_rollback_pc34(r);
    return 0;
}
