#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

#define FB_W 320
#define FB_H 200

/*
 * DM/CSB PC 3.4 VGA palette — authoritative source.
 *
 * Mirrors G9010_auc_VgaPaletteAll_Compat in vga_palette_pc34_compat.c,
 * which itself reproduces VIDEODRV.C / ReDMCSB G8149/G8151–G8156 exactly.
 * The previous array here used the IBM VGA 16-color legacy palette (the
 * standard EGA-compatible DAC defaults), which produced a "neon CGA"
 * look on PPM capture: floors read as saturated lime green, reds and
 * blues were mis-toned, and the screenshots did not match what classic
 * DM players recognize.  Using the real DM PC VGA palette gives the
 * muted, earthy, stone-colored look the game is supposed to have.
 *
 * VGA 6-bit → 8-bit: rgb8 = (vga6 << 2) | (vga6 >> 4).
 */
static const unsigned char kPalette[6][16][3] = {
    /* LIGHT0: brightest (title/menu/max brightness viewport) */
    {
        {  0,   0,   0}, {109, 109, 109}, {146, 146, 146}, {109,  36,   0},
        {  0, 219, 219}, {146,  73,   0}, {  0, 146,   0}, {  0, 219,   0},
        {255,   0,   0}, {255, 182,   0}, {219, 146, 109}, {255, 255,   0},
        { 73,  73,  73}, {182, 182, 182}, {  0,   0, 255}, {255, 255, 255}
    },
    /* LIGHT1 — G8152 */
    {
        {  0,   0,   0}, { 73,  73,  73}, {109, 109, 109}, {109,  36,   0},
        {  0, 219, 219}, {146,  36,   0}, {  0, 109,   0}, {  0, 182,   0},
        {219,   0,   0}, {219, 146,   0}, {182, 109,  73}, {255, 219,   0},
        { 36,  36,  36}, {146, 146, 146}, {  0,   0, 219}, {219, 219, 219}
    },
    /* LIGHT2 — G8153 */
    {
        {  0,   0,   0}, { 36,  36,  36}, { 73,  73,  73}, { 73,  36,   0},
        {  0, 219, 219}, {109,  36,   0}, {  0,  73,   0}, {  0, 146,   0},
        {182,   0,   0}, {182, 109,   0}, {146,  73,  36}, {255, 182,   0},
        {  0,   0,   0}, {109, 109, 109}, {  0,   0, 182}, {182, 182, 182}
    },
    /* LIGHT3 — G8154 */
    {
        {  0,   0,   0}, {  0,   0,   0}, { 36,  36,  36}, { 36,   0,   0},
        {  0, 219, 219}, { 73,  36,   0}, {  0,  36,   0}, {  0, 109,   0},
        {146,   0,   0}, {146,  73,   0}, {109,  36,   0}, {219, 146,   0},
        {  0,   0,   0}, { 73,  73,  73}, {  0,   0, 146}, {146, 146, 146}
    },
    /* LIGHT4 — G8155 */
    {
        {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
        {  0, 219, 219}, { 36,   0,   0}, {  0,   0,   0}, {  0,  73,   0},
        {109,   0,   0}, {109,  36,   0}, { 73,   0,   0}, {182, 109,   0},
        {  0,   0,   0}, { 36,  36,  36}, {  0,   0, 109}, {109, 109, 109}
    },
    /* LIGHT5: near-black — G8156 (residual light on 8 colors) */
    {
        {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0}, {  0,   0,   0},
        {  0, 219, 219}, {  0,   0,   0}, {  0,   0,   0}, {  0,  36,   0},
        { 73,   0,   0}, { 73,   0,   0}, { 36,   0,   0}, {109,  73,   0},
        {  0,   0,   0}, {  0,   0,   0}, {  0,   0,  73}, { 73,  73,  73}
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
            unsigned char raw = fb[y * width + x];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            int level = M11_FB_DECODE_LEVEL(raw);
            const unsigned char* rgb;
            if (level < 0 || level >= 6) {
                level = (int)paletteLevel;
            }
            rgb = get_rgb(idx, (unsigned int)level);
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
