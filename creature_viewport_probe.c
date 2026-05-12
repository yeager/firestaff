/*
 * Probe: dump all things on viewport-visible squares at game start.
 * Party at map 0, (1,3), facing South.
 * Viewport cells: forward 0-3, side -2..+2 relative to party.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "memory_tick_orchestrator_pc34_compat.h"
#include "memory_dungeon_dat_pc34_compat.h"

static const char* thing_type_name(int type) {
    static const char* names[] = {
        "Door", "Teleporter", "TextString", "Sensor",
        "Group", "Weapon", "Armour", "Scroll",
        "Potion", "Container", "Junk", "Projectile",
        "Explosion", "Type13", "Type14", "Type15"
    };
    if (type >= 0 && type < 16) return names[type];
    return "Unknown";
}

int main(int argc, char** argv) {
    const char* dungeonPath = argv[1];
    struct GameWorld_Compat world;
    int fx, fy, rx, ry;
    int fwd, side;

    (void)argc;

    if (!dungeonPath) { fprintf(stderr, "Usage: %s DUNGEON.DAT\n", argv[0]); return 1; }

    memset(&world, 0, sizeof(world));
    if (F0882_WORLD_InitFromDungeonDat_Compat(dungeonPath, 1234, &world) != 1) {
        fprintf(stderr, "Failed to load\n"); return 2;
    }

    printf("Party: map=%d x=%d y=%d dir=%d\n",
           world.party.mapIndex, world.party.mapX, world.party.mapY,
           world.party.direction);
    printf("Map 0 size: %dx%d\n",
           world.dungeon->maps[0].width, world.dungeon->maps[0].height);

    /* Direction vectors: 0=N(0,-1), 1=E(1,0), 2=S(0,1), 3=W(-1,0) */
    int dirs[4][4] = { {0,-1,1,0}, {1,0,0,1}, {0,1,-1,0}, {-1,0,0,-1} };
    fx = dirs[world.party.direction][0];
    fy = dirs[world.party.direction][1];
    rx = dirs[world.party.direction][2];
    ry = dirs[world.party.direction][3];

    printf("\n=== Viewport cells (fwd, side) -> (mapX, mapY) ===\n");
    for (fwd = 0; fwd <= 3; fwd++) {
        for (side = -2; side <= 2; side++) {
            int mx = world.party.mapX + fwd * fx + side * rx;
            int my = world.party.mapY + fwd * fy + side * ry;
            unsigned char sq = 0;
            int validSq = 0;

            printf("\n[fwd=%d side=%+d] -> (%d,%d): ", fwd, side, mx, my);

            if (mx < 0 || my < 0 ||
                mx >= (int)world.dungeon->maps[0].width ||
                my >= (int)world.dungeon->maps[0].height) {
                printf("OUT OF BOUNDS\n");
                continue;
            }

            /* Get square byte */
            {
                int idx = mx * (int)world.dungeon->maps[0].height + my;
                if (world.dungeon->tiles[0].squareData && idx < world.dungeon->tiles[0].squareCount) {
                    sq = world.dungeon->tiles[0].squareData[idx];
                    validSq = 1;
                }
            }
            if (!validSq) { printf("NO SQUARE DATA\n"); continue; }

            int elemType = (sq >> 5) & 0x07;
            printf("square=0x%02x elem=%d (%s)\n",
                   sq, elemType,
                   elemType == 0 ? "WALL" :
                   elemType == 1 ? "OPEN" :
                   elemType == 2 ? "PIT" :
                   elemType == 3 ? "STAIRS" :
                   elemType == 4 ? "DOOR" :
                   elemType == 5 ? "TELEPORTER" :
                   elemType == 6 ? "FAKE_WALL" :
                   elemType == 7 ? "TRICK_WALL" : "?");

            /* Walk thing list */
            if (world.things && world.things->squareFirstThings) {
                int base = 0, mi;
                for (mi = 0; mi < 0; mi++) {
                    base += world.dungeon->maps[mi].width * world.dungeon->maps[mi].height;
                }
                int sqIdx = base + mx * (int)world.dungeon->maps[0].height + my;
                if (sqIdx >= 0 && sqIdx < world.things->squareFirstThingCount) {
                    unsigned short thing = world.things->squareFirstThings[sqIdx];
                    int safety = 0;
                    while (thing != THING_ENDOFLIST && thing != THING_NONE && safety < 32) {
                        int type = THING_GET_TYPE(thing);
                        int index = THING_GET_INDEX(thing);
                        printf("  THING: raw=0x%04x type=%d(%s) index=%d",
                               thing, type, thing_type_name(type), index);
                        if (type == 4 && world.things->groups) { /* Group */
                            if (index < world.things->groupCount) {
                                printf(" creatureType=%d count=%d dir=%d",
                                       world.things->groups[index].creatureType,
                                       world.things->groups[index].count + 1,
                                       world.things->groups[index].direction);
                            }
                        }
                        printf("\n");
                        /* next thing */
                        if (type < 0 || type >= 16 || !world.things->rawThingData[type]) break;
                        int byteCount[] = {4,4,4,8,16,4,4,4,4,4,4,8,4,4,4,4};
                        unsigned char* raw = world.things->rawThingData[type] + index * byteCount[type];
                        thing = (unsigned short)(raw[0] | ((unsigned short)raw[1] << 8));
                        safety++;
                    }
                    if (safety == 0 && thing == THING_ENDOFLIST) printf("  (no things)\n");
                }
            }
        }
    }

    F0883_WORLD_Free_Compat(&world);
    return 0;
}
