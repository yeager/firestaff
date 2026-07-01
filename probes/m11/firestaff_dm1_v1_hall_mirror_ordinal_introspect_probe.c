/*
 * DM1 V1 Hall of Champions mirror ordinal introspector.
 *
 * Brute-force scans the Hall of Champions cell map for every front-cell
 * mirror ordinal reachable from M11_GameView_GetFrontMirrorOrdinal,
 * and prints the (map, x, y, dir) -> ordinal map for runtime fixture
 * auditing.  This is a non-CTest diagnostic helper that supports the
 * `front_north_entry` slice family for ordinal 0..23 by reporting the
 * full pose table so individual gate probes can pin a specific cell.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2570-2573 maps sensor cell to front-wall aspect
 *     via M021_NORMALIZE for the PC 3.x side filter;
 *   ReDMCSB DUNGEON.C:2608-2612 sets G0289 for C127 champion portraits;
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 at the fixed G0109 D1C wall
 *     box (left=96, top=35, width=32, height=29).
 *
 * Usage: firestaff_dm1_v1_hall_mirror_ordinal_introspect_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static const char* dir_name(int dir) {
    switch (dir) {
        case 0: return "NORTH";
        case 1: return "EAST";
        case 2: return "SOUTH";
        case 3: return "WEST";
        default: return "?";
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int mapX;
    int mapY;
    int dir;
    int seen[24] = {0};

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Map 0 is the Hall of Champions.  Brute-force every (x, y, dir)
     * cell, report any non-negative front-cell mirror ordinal, and
     * group the cells by ordinal so a slice gate can pick exactly one
     * (x, y, dir) pose per ordinal. */
    printf("=== DM1 V1 Hall of Champions mirror ordinal pose table ===\n");
    printf("(map=0, x, y, dir, ordinal)\n");
    for (dir = 0; dir < 4; ++dir) {
        for (mapY = 0; mapY < (int)game.world.dungeon->maps[0].height; ++mapY) {
            for (mapX = 0; mapX < (int)game.world.dungeon->maps[0].width; ++mapX) {
                int ord;
                game.world.party.mapIndex = 0;
                game.world.party.mapX = mapX;
                game.world.party.mapY = mapY;
                game.world.party.direction = dir;
                ord = M11_GameView_GetFrontMirrorOrdinal(&game);
                if (ord >= 0 && ord < 24) {
                    printf("  map=0 pose=(%d,%d) dir=%s ord=%d\n",
                        mapX, mapY, dir_name(dir), ord);
                    if (ord >= 0 && ord < 24) ++seen[ord];
                }
            }
        }
    }

    printf("=== ordinal coverage summary ===\n");
    for (int i = 0; i < 24; ++i) {
        printf("ordinal=%d seen=%d\n", i, seen[i]);
    }

    M11_GameView_Shutdown(&game);
    return 0;
}
