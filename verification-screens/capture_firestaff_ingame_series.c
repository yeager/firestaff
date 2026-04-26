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

static int write_ppm_crop_from_indices(const unsigned char* fb,
                                       int width,
                                       int height,
                                       int cropX,
                                       int cropY,
                                       int cropW,
                                       int cropH,
                                       unsigned int paletteLevel,
                                       const char* path) {
    FILE* f;
    int x;
    int y;
    if (!fb || !path || cropX < 0 || cropY < 0 || cropW <= 0 || cropH <= 0 ||
        cropX + cropW > width || cropY + cropH > height) {
        return 0;
    }
    f = fopen(path, "wb");
    if (!f) return 0;
    fprintf(f, "P6\n%d %d\n255\n", cropW, cropH);
    for (y = 0; y < cropH; ++y) {
        for (x = 0; x < cropW; ++x) {
            unsigned char raw = fb[(cropY + y) * width + (cropX + x)];
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
    char cropPath[1024];
    int vpX = 0;
    int vpY = 0;
    int vpW = 0;
    int vpH = 0;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    snprintf(path, sizeof(path), "%s/%s.ppm", outDir, name);
    if (!write_ppm_from_indices(fb, FB_W, FB_H, paletteLevel, path)) {
        return 0;
    }
    if (!M11_GameView_GetViewportRect(&vpX, &vpY, &vpW, &vpH) ||
        vpX != 0 || vpY != 33 || vpW != 224 || vpH != 136) {
        return 0;
    }
    snprintf(cropPath, sizeof(cropPath), "%s/%s_viewport_224x136.ppm", outDir, name);
    return write_ppm_crop_from_indices(fb, FB_W, FB_H,
                                       vpX, vpY, vpW, vpH,
                                       paletteLevel, cropPath);
}

static void ensure_deterministic_capture_champion(M11_GameViewState* game) {
    struct ChampionState_Compat* c;
    unsigned short weaponThing;
    int invSlot;
    if (!game) return;
    c = &game->world.party.champions[0];
    memset(c, 0, sizeof(*c));
    for (invSlot = 0; invSlot < CHAMPION_SLOT_COUNT; ++invSlot) {
        c->inventory[invSlot] = THING_NONE;
    }
    c->present = 1;
    memcpy(c->name, "HALK\0\0\0\0", 8);
    c->hp.current = 42; c->hp.maximum = 60;
    c->stamina.current = 35; c->stamina.maximum = 50;
    c->mana.current = 20; c->mana.maximum = 40;
    c->food = 80; c->water = 70;
    c->portraitIndex = 0;
    game->world.party.championCount = 1;
    game->world.party.activeChampionIndex = 0;

    /* Deterministic DAGGER fixture: this makes both the right-side
     * action icon cells and the inventory-panel slot box exercise the
     * source object-icon path instead of whatever subtype the dungeon
     * happened to load at weapon index 0. */
    if (game->world.things && game->world.things->weapons &&
        game->world.things->weaponCount > 0) {
        weaponThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 0);
        game->world.things->weapons[0].type = 8; /* dagger -> icon 32 */
        game->world.things->weapons[0].chargeCount = 0;
        game->world.things->weapons[0].lit = 0;
        c->inventory[CHAMPION_SLOT_HAND_RIGHT] = weaponThing;
    }
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
    /* Match the real launcher path: once DM1 opens, main_loop_m11.c
     * resets the renderer palette level to 0 so indexed in-game pixels
     * are not globally dimmed by a startup-menu setting. */
    paletteLevel = 0U;
    ensure_deterministic_capture_champion(&game);

    if (!save_game(outDir, "01_ingame_start_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_RIGHT);
    if (!save_game(outDir, "02_ingame_turn_right_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_UP);
    if (!save_game(outDir, "03_ingame_move_forward_latest", &game, paletteLevel)) return 1;
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_RUNE_1);
    if (!save_game(outDir, "04_ingame_spell_panel_latest", &game, paletteLevel)) return 1;
    /* Complete a deterministic low-power Ful Ir (fireball) sequence before
     * casting: row0 power rune, row1 Ful, row2 Ir.  The previous fixture
     * tried to cast after only one rune, so SPELL_CAST was ignored while the
     * screenshot name claimed "after_cast". */
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_RUNE_4);
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_RUNE_4);
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_CAST);
    if (!save_game(outDir, "05_ingame_after_cast_latest", &game, paletteLevel)) return 1;

    /* Ensure the spell panel is closed, then open the inventory panel. */
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_SPELL_CLEAR);
    ensure_deterministic_capture_champion(&game);
    (void)M11_GameView_HandleInput(&game, M12_MENU_INPUT_INVENTORY_TOGGLE);
    if (!save_game(outDir, "06_ingame_inventory_panel_latest", &game, paletteLevel)) return 1;

    M11_GameView_Shutdown(&game);
    printf("wrote 6 in-game screenshots and 6 viewport crops to %s\n", outDir);
    return 0;
}
