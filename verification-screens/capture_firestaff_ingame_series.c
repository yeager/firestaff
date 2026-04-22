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

static const unsigned char kPalette[6][16][3] = {
    {
        {0,0,0},{0,0,170},{0,170,0},{0,170,170},{170,0,0},{170,85,0},{170,170,170},{85,85,85},
        {255,85,85},{85,85,255},{85,255,85},{255,255,85},{40,40,40},{200,200,200},{100,100,100},{255,255,255}
    },
    {
        {0,0,0},{0,0,136},{0,136,0},{0,136,136},{136,0,0},{136,68,0},{136,136,136},{68,68,68},
        {204,68,68},{68,68,204},{68,204,68},{204,204,68},{32,32,32},{160,160,160},{80,80,80},{204,204,204}
    },
    {
        {0,0,0},{0,0,102},{0,102,0},{0,102,102},{102,0,0},{102,51,0},{102,102,102},{51,51,51},
        {153,51,51},{51,51,153},{51,153,51},{153,153,51},{24,24,24},{120,120,120},{60,60,60},{153,153,153}
    },
    {
        {0,0,0},{0,0,68},{0,68,0},{0,68,68},{68,0,0},{68,34,0},{68,68,68},{34,34,34},
        {102,34,34},{34,34,102},{34,102,34},{102,102,34},{16,16,16},{80,80,80},{40,40,40},{102,102,102}
    },
    {
        {0,0,0},{0,0,34},{0,34,0},{0,34,34},{34,0,0},{34,17,0},{34,34,34},{17,17,17},
        {51,17,17},{17,17,51},{17,51,17},{51,51,17},{8,8,8},{40,40,40},{20,20,20},{51,51,51}
    },
    {
        {0,0,0},{0,0,17},{0,17,0},{0,17,17},{17,0,0},{17,8,0},{17,17,17},{8,8,8},
        {26,8,8},{8,8,26},{8,26,8},{26,26,8},{4,4,4},{20,20,20},{10,10,10},{26,26,26}
    }
};

static const unsigned char* get_rgb(unsigned char idx, unsigned int paletteLevel) {
    if (paletteLevel >= 6U) {
        paletteLevel = 0;
    }
    return kPalette[paletteLevel][idx & 0x0F];
}

static int write_ppm_from_indices(const unsigned char* fb, int width, int height, unsigned int paletteLevel, const char* path) {
    FILE* f = fopen(path, "wb");
    int x;
    int y;
    if (!f) return 0;
    fprintf(f, "P6\n%d %d\n255\n", width, height);
    for (y = 0; y < height; ++y) {
        for (x = 0; x < width; ++x) {
            const unsigned char* rgb = get_rgb(fb[y * width + x], paletteLevel);
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
    return 1;
}

static int save_game(const char* outDir, const char* name, M11_GameViewState* game, unsigned int paletteLevel) {
    unsigned char fb[FB_W * FB_H];
    char path[1024];
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/%s.ppm", outDir, name);
    return write_ppm_from_indices(fb, FB_W, FB_H, paletteLevel, path);
}

int main(int argc, char** argv) {
    const char* outDir;
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned int paletteLevel;

    if (argc < 3) {
        fprintf(stderr, "usage: %s OUT_DIR DATA_DIR\n", argv[0]);
        return 2;
    }
    outDir = argv[1];
    dataDir = argv[2];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir);
    paletteLevel = (unsigned int)M12_StartupMenu_GetRenderPaletteLevel(&menu);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "failed to open DM1 game view\n");
        return 1;
    }

    if (!save_game(outDir, "01_ingame_start_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    if (!save_game(outDir, "02_ingame_turn_right_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    if (!save_game(outDir, "03_ingame_move_forward_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_RUNE_1);
    if (!save_game(outDir, "04_ingame_spell_panel_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_CAST);
    if (!save_game(outDir, "05_ingame_after_cast_latest", &game, paletteLevel)) return 1;

    /* Close spell panel via SPELL_CLEAR, then open the inventory panel.
     * DM1 starts with no recruited champions, so synthesize one for
     * the screenshot if needed. */
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_CLEAR);
    if (game.world.party.championCount == 0) {
        /* Synthesize a test champion for inventory screenshot */
        struct ChampionState_Compat* c = &game.world.party.champions[0];
        memset(c, 0, sizeof(*c));
        c->present = 1;
        memcpy(c->name, "HALK\0\0\0\0", 8);
        c->hp.current = 42; c->hp.maximum = 60;
        c->stamina.current = 35; c->stamina.maximum = 50;
        c->mana.current = 20; c->mana.maximum = 40;
        c->food = 80; c->water = 70;
        c->portraitIndex = 0;
        /* Give the champion a TORCH in the ready hand (weapon subtype 4) */
        if (game.world.things && game.world.things->weapons &&
            game.world.things->weaponCount > 0) {
            /* Find a weapon thing or create a synthetic thingId */
            unsigned short weaponThing = (THING_TYPE_WEAPON << 10) | 0;
            c->inventory[0] = weaponThing; /* HAND_LEFT = slot 0 */
        }
        game.world.party.championCount = 1;
        game.world.party.activeChampionIndex = 0;
    } else {
        (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_CYCLE_CHAMPION);
    }
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_INVENTORY_TOGGLE);
    if (!save_game(outDir, "06_ingame_inventory_panel_latest", &game, paletteLevel)) return 1;

    M11_GameView_Shutdown(&game);
    printf("wrote 6 in-game screenshots to %s\n", outDir);
    return 0;
}
