/*
 * DM1 V1 Hall of Champions mirror ordinal -> champion name lookup.
 *
 * Helper that prints, for every ordinal in the DM1 V1 mirror catalog,
 * the decoded champion name and title.  Useful for filling out the
 * ordinal -> champion mapping in slice-gate docstrings.
 *
 * Source evidence:
 *   ReDMCSB REVIVE.C:63 F0280_CHAMPION_AddCandidateChampionToParty
 *   decodes HP/stamina/mana/statistics from mirror text; the Firestaff
 *   mirror catalog (`ChampionMirrorCatalog_Compat` in
 *   src/memory/memory_champion_state_pc34_compat.c) is the
 *   Firestaff-side reflection of the same decoded records.
 *
 * Usage: firestaff_dm1_v1_hall_mirror_ordinal_champion_name_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int i;

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

    printf("=== DM1 V1 Hall of Champions mirror ordinal -> champion name ===\n");
    for (i = 0; i < game.mirrorCatalog.count; ++i) {
        char name[CHAMPION_NAME_TEXT_CAPACITY];
        char title[CHAMPION_TITLE_TEXT_CAPACITY];
        int ord = game.mirrorCatalog.records[i].mirrorOrdinal;
        name[0] = '\0';
        title[0] = '\0';
        if (F0660_CHAMPION_MirrorCatalogGetName_Compat(&game.mirrorCatalog, ord, name, sizeof(name)) <= 0) {
            snprintf(name, sizeof(name), "<lookup failed>");
        }
        if (F0661_CHAMPION_MirrorCatalogGetTitle_Compat(&game.mirrorCatalog, ord, title, sizeof(title)) <= 0) {
            snprintf(title, sizeof(title), "<untitled>");
        }
        printf("ordinal=%d name=%-8s title=%s\n", ord, name, title);
    }

    M11_GameView_Shutdown(&game);
    return 0;
}
