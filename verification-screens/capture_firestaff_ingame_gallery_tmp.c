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

/*
 * DM/CSB PC 3.4 VGA palette - authoritative source.
 *
 * Mirrors G9010_auc_VgaPaletteAll_Compat in vga_palette_pc34_compat.c,
 * which itself reproduces VIDEODRV.C / ReDMCSB G8149/G8151-G8156 exactly.
 * The previous array here used the IBM VGA 16-color legacy palette, which
 * produced a "neon CGA" look on PPM capture: floors read as saturated
 * lime green, reds/blues were mis-toned, and screenshots did not match
 * what classic DM players recognize.  Using the real DM PC VGA palette
 * gives the muted, earthy, stone-colored look the game is supposed to
 * have.
 */
static const unsigned char kPalette[6][16][3] = {
    /* LIGHT0: brightest */
    {
        {  0,   0,   0}, {109, 109, 109}, {146, 146, 146}, {109,  36,   0},
        {  0, 219, 219}, {146,  73,   0}, {  0, 146,   0}, {  0, 219,   0},
        {255,   0,   0}, {255, 182,   0}, {219, 146, 109}, {255, 255,   0},
        { 73,  73,  73}, {182, 182, 182}, {  0,   0, 255}, {255, 255, 255}
    },
    /* LIGHT1 - G8152 */
    {
        {  0,   0,   0}, { 73,  73,  73}, {109, 109, 109}, {109,  36,   0},
        {  0, 219, 219}, {146,  36,   0}, {  0, 109,   0}, {  0, 182,   0},
        {219,   0,   0}, {219, 146,   0}, {182, 109,  73}, {255, 219,   0},
        { 36,  36,  36}, {146, 146, 146}, {  0,   0, 219}, {219, 219, 219}
    },
    /* LIGHT2 - G8153 */
    {
        {  0,   0,   0}, { 36,  36,  36}, { 73,  73,  73}, { 73,  36,   0},
        {  0, 219, 219}, {109,  36,   0}, {  0,  73,   0}, {  0, 146,   0},
        {182,   0,   0}, {182, 109,   0}, {146,  73,  36}, {255, 182,   0},
        {  0,   0,   0}, {109, 109, 109}, {  0,   0, 182}, {182, 182, 182}
    },
    /* LIGHT3 - G8154 */
    {
        {  0,   0,   0}, {  0,   0,   0}, { 36,  36,  36}, { 36,   0,   0},
        {  0, 219, 219}, { 73,  36,   0}, {  0,  36,   0}, {  0, 109,   0},
        {146,   0,   0}, {146,  73,   0}, {109,  36,   0}, {219, 146,   0},
        {  0,   0,   0}, { 73,  73,  73}, {  0,   0, 146}, {146, 146, 146}
    },
    /* LIGHT4 - G8155 */
    {
        {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
        {  0, 219, 219}, { 36,   0,   0}, {  0,   0,   0}, {  0,  73,   0},
        {109,   0,   0}, {109,  36,   0}, { 73,   0,   0}, {182, 109,   0},
        {  0,   0,   0}, { 36,  36,  36}, {  0,   0, 109}, {109, 109, 109}
    },
    /* LIGHT5: near-black - G8156 */
    {
        {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
        {  0, 219, 219}, {  0,   0,   0}, {  0,   0,   0}, {  0,  36,   0},
        { 73,   0,   0}, { 73,   0,   0}, { 36,   0,   0}, {109,  73,   0},
        {  0,   0,   0}, {  0,   0,   0}, {  0,   0,  73}, { 73,  73,  73}
    }
};

static const unsigned char* get_rgb(unsigned char idx, unsigned int paletteLevel) {
    if (paletteLevel >= 6U) paletteLevel = 0;
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

    if (!save_game(outDir, "01_ingame_start", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_MAP_TOGGLE);
    if (!save_game(outDir, "02_ingame_map_overlay", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_MAP_TOGGLE);
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_INVENTORY_TOGGLE);
    if (!save_game(outDir, "03_ingame_inventory_overlay", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_INVENTORY_TOGGLE);
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_RUNE_1);
    if (!save_game(outDir, "04_ingame_spell_panel", &game, paletteLevel)) return 1;

    M11_GameView_Shutdown(&game);
    printf("wrote gallery screenshots to %s\n", outDir);
    return 0;
}
