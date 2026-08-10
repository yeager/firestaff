#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState game;
    char name[64], title[64];
    int i;
    const struct DungeonThings_Compat* things;
    if (argc < 2) return 2;
    M12_StartupMenu_InitWithDataDir(&menu, argv[1], NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) { fprintf(stderr,"open failed\n"); return 1; }
    for (i = 0; i < 32; ++i) {
        int rn = M11_GameView_GetMirrorNameByOrdinal(&game, i, name, sizeof(name));
        int rt = M11_GameView_GetMirrorTitleByOrdinal(&game, i, title, sizeof(title));
        printf("ord %2d  name='%s' (rc=%d)  title='%s' (rc=%d)\n", i, name, rn, title, rt);
    }
    printf("\n---mirror catalog textStringIndex -> mirrorOrdinal map---\n");
    {
        struct ChampionMirrorCatalog_Compat cat;
        int k;
        things = game.world.things;
        F0652_CHAMPION_BuildMirrorCatalog_Compat(things, &cat);
        printf("catalog count=%d textStringCount=%d\n", cat.count, things ? things->textStringCount : -1);
        for (k = 0; k < cat.count; ++k) {
            printf("  ord %2d <- textString %3d  name='%s' title='%s'\n",
                   cat.records[k].mirrorOrdinal,
                   cat.records[k].textStringIndex,
                   cat.records[k].nameText,
                   cat.records[k].titleText);
        }
        printf("\n---text string index -> raw decoded (name/title only)---\n");
        for (k = 0; things && k < things->textStringCount && k < 50; ++k) {
            char decoded[256];
            int len = F0507_DUNGEON_DecodeTextAtOffset_Compat(
                things->textData,
                things->textDataWordCount,
                things->textStrings[k].textDataWordOffset,
                decoded,
                (int)sizeof(decoded));
            int j;
            printf("ts %2d len=%d :", k, len);
            for (j = 0; j < len && j < 60; ++j) {
                unsigned char c = (unsigned char)decoded[j];
                if (c == '\n') printf("|");
                else if (c >= 32 && c < 127) putchar(c);
                else printf(".");
            }
            printf("\n");
        }
    }
    M11_GameView_Shutdown(&game);
    return 0;
}
