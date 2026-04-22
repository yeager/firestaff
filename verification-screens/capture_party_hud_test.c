#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

#define FB_W 320
#define FB_H 200

static const unsigned char kPalette[16][3] = {
    {0,0,0},{0,0,170},{0,170,0},{0,170,170},{170,0,0},{170,85,0},{170,170,170},{85,85,85},
    {255,85,85},{85,85,255},{85,255,85},{255,255,85},{40,40,40},{200,200,200},{100,100,100},{255,255,255}
};

static int write_ppm(const unsigned char* fb, int w, int h, const char* path) {
    FILE* f = fopen(path, "wb");
    int i;
    if (!f) return 0;
    fprintf(f, "P6\n%d %d\n255\n", w, h);
    for (i = 0; i < w * h; ++i) {
        unsigned char idx = fb[i] & 0x0F;
        fputc(kPalette[idx][0], f);
        fputc(kPalette[idx][1], f);
        fputc(kPalette[idx][2], f);
    }
    fclose(f);
    return 1;
}

int main(int argc, char** argv) {
    const char* outDir;
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[FB_W * FB_H];
    char path[1024];
    int i;

    if (argc < 3) {
        fprintf(stderr, "usage: %s OUT_DIR DATA_DIR\n", argv[0]);
        return 2;
    }
    outDir = argv[1];
    dataDir = argv[2];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "failed to open DM1 game view\n");
        return 1;
    }

    /* Synthesize 4 champions for party HUD test */
    {
        const char* names[4] = {"HALK", "ALEX", "SYRA", "ZYTA"};
        for (i = 0; i < 4; ++i) {
            struct ChampionState_Compat* c = &game.world.party.champions[i];
            memset(c, 0, sizeof(*c));
            c->present = 1;
            memcpy(c->name, names[i], strlen(names[i]));
            c->hp.current = 40 + i * 10; c->hp.maximum = 60 + i * 5;
            c->stamina.current = 30 + i * 5; c->stamina.maximum = 50;
            c->mana.current = 15 + i * 5; c->mana.maximum = 30 + i * 5;
            c->portraitIndex = i;
        }
        game.world.party.championCount = 4;
        game.world.party.activeChampionIndex = 0;
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/07_party_hud_with_champions.ppm", outDir);
    write_ppm(fb, FB_W, FB_H, path);

    /* Also capture with spell panel open */
    M11_GameView_OpenSpellPanel(&game);
    M11_GameView_EnterRune(&game, 0); /* Enter first rune */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/08_spell_panel_with_champions.ppm", outDir);
    write_ppm(fb, FB_W, FB_H, path);

    M11_GameView_Shutdown(&game);
    printf("wrote 2 party-HUD test screenshots to %s\n", outDir);
    return 0;
}
