/* One-off dev probe: dump map 0 element-type grid for the Hall of Champions. */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include <stdio.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState game;
    M12_StartupMenu_InitWithDataDir(&menu, argv[1], NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) return 1;
    struct { int x; int y; int d; const char* label; } poses[] = {
        /* Try various cells for ordinal 12 (cellBit=2 = west wall of (2,9)) */
        {2, 8, 0, "(2,8) N"}, {2, 8, 1, "(2,8) E"}, {2, 8, 2, "(2,8) S"}, {2, 8, 3, "(2,8) W"},
        {2, 10, 0, "(2,10) N"}, {2, 10, 1, "(2,10) E"}, {2, 10, 2, "(2,10) S"}, {2, 10, 3, "(2,10) W"},
        {1, 9, 0, "(1,9) N"}, {1, 9, 1, "(1,9) E"}, {1, 9, 2, "(1,9) S"}, {1, 9, 3, "(1,9) W"},
        {3, 9, 0, "(3,9) N"}, {3, 9, 1, "(3,9) E"}, {3, 9, 2, "(3,9) S"}, {3, 9, 3, "(3,9) W"},
    };
    int i;
    /* Print just the y=9 row to see cell layout */
    int w = game.world.dungeon->maps[0].width;
    int h = game.world.dungeon->maps[0].height;
    int x, y;
    printf("Map 0: %d x %d\n", w, h);
    printf("    ");
    for (x = 0; x < w; ++x) printf("%d", x % 10);
    printf("\n");
    for (y = 7; y <= 10; ++y) {
        printf("y=%2d ", y);
        for (x = 0; x < w; ++x) {
            int sqIdx = x * h + y;
            unsigned char sq = game.world.dungeon->tiles[0].squareData[sqIdx];
            int type = sq >> 5;
            char ch = '?';
            if (type == 0) ch = '#';
            else if (type == 1) ch = '.';
            else if (type == 2) ch = 'P';
            else if (type == 3) ch = 'S';
            else if (type == 4) ch = 'D';
            else if (type == 5) ch = 'T';
            else if (type == 6) ch = 'F';
            printf("%c", ch);
        }
        printf("\n");
    }
    printf("\nOrdinal lookups:\n");
    for (i = 0; i < (int)(sizeof(poses)/sizeof(poses[0])); ++i) {
        int got;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = poses[i].x;
        game.world.party.mapY = poses[i].y;
        game.world.party.direction = poses[i].d;
        got = M11_GameView_GetFrontMirrorOrdinal(&game);
        printf("  %s -> ordinal=%d\n", poses[i].label, got);
    }
    M11_GameView_Shutdown(&game);
    return 0;
}

