#include "dm1_v1_c14_layout_pc34_compat.h"
#include <string.h>
#include <stdio.h>

#define OK(x) do { if (!(x)) { fprintf(stderr, "FAIL: %s\n", #x); return 1; } } while (0)
int main(void) {
    struct DungeonThings_Compat things = {0}; struct DungeonDatState_Compat dungeon = {0};
    struct DungeonMapDesc_Compat map = {0}; struct DungeonMapTiles_Compat tiles = {0};
    struct DungeonProjectile_Compat p[2] = {0}, before[2]; DM1_C14PoolReservationPc34 r;
    unsigned char square[1] = { (unsigned char)((DUNGEON_ELEMENT_CORRIDOR << 5) | DUNGEON_SQUARE_MASK_THING_LIST) };
    unsigned short sft[1] = { (unsigned short)(THING_TYPE_PROJECTILE << 10) }, cols[1] = {0};
    unsigned char raw[16] = { 0xfe,0xff, 0x11,0x22,3,4,5,0, 0xff,0xff, 9,8,7,6,5,4 }, saved[16];
    p[0].next=THING_ENDOFLIST; p[0].slot=0x2211; p[0].kineticEnergy=3; p[0].attack=4; p[0].eventIndex=5;
    p[1].next=THING_NONE; p[1].slot=0x0809; p[1].kineticEnergy=7; p[1].attack=6; p[1].eventIndex=0x0405;
    memcpy(saved,raw,sizeof(raw)); memcpy(before,p,sizeof(p));
    things.loaded=1; things.projectiles=p; things.projectileCount=things.thingCounts[THING_TYPE_PROJECTILE]=2;
    things.rawThingData[THING_TYPE_PROJECTILE]=raw; things.squareFirstThings=sft; things.squareFirstThingCount=1;
    dungeon.tilesLoaded=1; dungeon.header.mapCount=1; dungeon.maps=&map; dungeon.tiles=&tiles;
    dungeon.columnsCumulativeSquareFirstThingCount=cols; dungeon.dungeonColumnCount=1;
    map.width=map.height=1; tiles.squareData=square; tiles.squareCount=1;
    OK(dm1_v1_c14_pool_reserve_pc34(&things,&r));
    OK(r.thing == (unsigned short)((THING_TYPE_PROJECTILE<<10)|1));
    OK(raw[8]==0xfe && raw[9]==0xff && p[1].next==THING_ENDOFLIST);
    OK(dm1_v1_c14_pool_initialize_and_link_pc34(&r,&dungeon,0x2022,33,44,55,2,0,0,0));
    OK(raw[10]==0x22 && raw[11]==0x20 && raw[12]==33 && raw[13]==44 && raw[14]==55 && raw[15]==0);
    OK(p[1].slot==0x2022 && p[1].kineticEnergy==33 && p[1].attack==44 && p[1].eventIndex==55);
    OK(p[0].next == (unsigned short)((2u<<14)|(THING_TYPE_PROJECTILE<<10)|1));
    OK(dm1_v1_c14_pool_rollback_pc34(&r)); OK(!memcmp(raw,saved,sizeof(raw)) && !memcmp(p,before,sizeof(p)));
    OK(dm1_v1_c14_pool_reserve_pc34(&things,&r));
    OK(!dm1_v1_c14_pool_initialize_and_link_pc34(&r,&dungeon,0x2022,33,44,55,2,0,1,0));
    OK(!r.active && !memcmp(raw,saved,sizeof(raw)) && !memcmp(p,before,sizeof(p)));
    puts("PASS: DM1 C14 raw transaction"); return 0;
}
