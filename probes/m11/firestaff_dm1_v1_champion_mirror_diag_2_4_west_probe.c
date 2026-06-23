/* Diagnostic probe: enumerate front-mirror ordinals for cells (1,4)
 * and (2,4) of map 0 (Hall of Champions) for every direction, plus
 * dump the mirror catalog names + titles.
 *
 * Source: ReDMCSB DUNGEON.C:2573 + 2608-2612 + 11652 in
 * src/engine/m11_game_view.c, the M011_CELL(sensor) front-wall
 * filter for C127 champion-portrait sensors.
 *
 * Used to author the ordinal-6 west_negative slice; not a finished
 * gate (no asserts, prints observations only).  Kept here so the
 * next slice worker can re-run this against a different DM1 V1 build
 * to spot any sensor layout drift.
 *
 * Usage: firestaff_dm1_v1_champion_mirror_diag_2_4_west_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "asset_status_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    int di;
    int dirs[4] = {0, 1, 2, 3};
    const char* dirNames[4] = {"N", "E", "S", "W"};
    int cnt, i;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 (1,4)/(2,4) Hall front-mirror diagnostic ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    for (di = 0; di < 4; ++di) {
        state.world.party.mapIndex = 0;
        state.world.party.mapX = 1;
        state.world.party.mapY = 4;
        state.world.party.direction = dirs[di];
        printf("pose (1,4) facing %s -> ordinal=%d\n",
               dirNames[di], M11_GameView_GetFrontMirrorOrdinal(&state));
    }

    for (di = 0; di < 4; ++di) {
        state.world.party.mapIndex = 0;
        state.world.party.mapX = 2;
        state.world.party.mapY = 4;
        state.world.party.direction = dirs[di];
        printf("pose (2,4) facing %s -> ordinal=%d\n",
               dirNames[di], M11_GameView_GetFrontMirrorOrdinal(&state));
    }

    cnt = M11_GameView_GetMirrorCatalogCount(&state);
    printf("Mirror catalog count: %d\n", cnt);
    for (i = 0; i < cnt && i < 24; ++i) {
        char name[32], title[32];
        memset(name, 0, sizeof(name));
        memset(title, 0, sizeof(title));
        M11_GameView_GetMirrorNameByOrdinal(&state, i, name, sizeof(name));
        M11_GameView_GetMirrorTitleByOrdinal(&state, i, title, sizeof(title));
        printf("  ordinal %2d: name='%s' title='%s'\n", i, name, title);
    }

    M11_GameView_Shutdown(&state);
    return 0;
}
