#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

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

static int write_ppm_from_indices(const unsigned char* fb,
                                  int width,
                                  int height,
                                  unsigned int paletteLevel,
                                  const char* path) {
    FILE* f;
    int x;
    int y;
    if (!fb || !path) {
        return 0;
    }
    f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
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

static int save_menu(const char* outDir,
                     const char* name,
                     M12_StartupMenuState* menu,
                     unsigned int paletteLevel) {
    unsigned char fb[FB_W * FB_H];
    char path[1024];
    memset(fb, 0, sizeof(fb));
    M12_StartupMenu_Draw(menu, fb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/%s.ppm", outDir, name);
    return write_ppm_from_indices(fb, FB_W, FB_H, paletteLevel, path);
}

static int save_game(const char* outDir,
                     const char* name,
                     M11_GameViewState* game,
                     unsigned int paletteLevel) {
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
    if (!save_menu(outDir, "01_start_menu", &menu, paletteLevel)) {
        fprintf(stderr, "failed to save start menu\n");
        return 1;
    }

    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&menu, M12_MENU_INPUT_ACCEPT);
    if (!save_menu(outDir, "02_settings", &menu, paletteLevel)) {
        fprintf(stderr, "failed to save settings\n");
        return 1;
    }

    M12_StartupMenu_InitWithDataDir(&menu, dataDir);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "failed to open DM1 game view\n");
        return 1;
    }

    if (!save_game(outDir, "03_ingame_start", &game, paletteLevel)) {
        fprintf(stderr, "failed to save game start\n");
        return 1;
    }

    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    if (!save_game(outDir, "04_ingame_turn_right", &game, paletteLevel)) {
        fprintf(stderr, "failed to save turned game\n");
        return 1;
    }

    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    if (!save_game(outDir, "05_ingame_move_forward", &game, paletteLevel)) {
        fprintf(stderr, "failed to save moved game\n");
        return 1;
    }

    M11_GameView_Shutdown(&game);
    printf("wrote 5 screenshots to %s\n", outDir);
    return 0;
}
