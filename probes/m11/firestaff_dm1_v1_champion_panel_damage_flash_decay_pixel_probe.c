/*
 * DM1 V1 champion panel damage-flash decay runtime pixel probe.
 *
 * Firestaff-side evidence only. It opens the hash-verified DM1 V1 runtime
 * when local assets are available, renders one alive champion with the
 * C015 damage-to-champion banner active, ticks the per-champion damage
 * timer down to zero, then redraws into the same framebuffer. The gate
 * proves the real M11 V1 draw stack both shows the GRAPHICS.DAT damage
 * feedback while active, draws 1/2/3-digit damage numbers at the PC34
 * F0320 MEDIA009 origins, and clears stale banner pixels when the timer
 * expires. It does not claim original DOS screenshot parity.
 *
 * KNOWN FAILURE (diagnosed 2026-08-09, not yet fixed):
 *   The C015 banner IS implemented -- m11_game_view.c draws it via
 *   dm1_v1_graphic_champion_damage_small_pc34(), placing it with
 *   dm1_v1_champion_damage_indicator_rect_pc34() and blitting with
 *   M11_COLOR_FLESH transparency, matching CHAMPION.C F0320.  It is simply
 *   never reached on this probe's path.
 *
 *   m11_dm1_v1_top_row_receipt_required() is satisfied here
 *   (M11_GAME_SOURCE_BUILTIN_CATALOG counts as a DM1 source kind, debug HUD
 *   off, V1 chrome mode on, no V2 slice), so the champion-panel draw takes
 *   m11_draw_dm1_v1_top_row_receipt() and returns early.  The panel body,
 *   and the damage banner inside it, never execute -- the indicator zone
 *   comes back as 315 pixels of colour 0, and the three PC34 damage-number
 *   origin checks fail with it.
 *
 *   Ruled out while diagnosing: geometry (this probe's
 *   M11_GameView_GetV1DamageIndicatorZone is an inline wrapper around the
 *   very same rect function the draw uses) and asset/font availability
 *   (assetsAvailable and originalFontAvailable are both 1).
 *
 *   ReDMCSB CHAMPION.C:1762 draws this banner over the champion name in the
 *   status box -- i.e. in the top row -- so the top-row receipt path should
 *   render it.  That makes this a parity gap in the receipt path rather
 *   than a defect in this probe.
 *
 *   Correcting the record: commit 9e4fffcc8 claimed the banner was "simply
 *   not implemented".  That was wrong; it was based on grepping for the
 *   unused M11_GFX_DAMAGE_TO_CHAMPION_SMALL enum, which is dead code
 *   sitting alongside a working implementation that uses a different
 *   accessor.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0623 lines 680-699 chooses C015/C167 for a
 *   non-inventory champion damage banner, prints the damage text, and
 *   calls F0292_CHAMPION_DrawState for the same champion.
 *   ReDMCSB CHAMPION.C F0320 lines 1720-1779 applies pending damage,
 *   calls F0623 for nonlethal damage, then lines 1780-1784 schedule
 *   C12_EVENT_HIDE_DAMAGE_RECEIVED five ticks later.
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-815 redraws an alive champion's
 *   C151..C154 status box before name, statistics, wounds, and action
 *   hand overlays are redrawn.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_CHAMPION_COUNT = 1,
    PROBE_SLOT = 0,
    PROBE_DAMAGE_AMOUNT = 37,
    PROBE_COLOR_WHITE = 15
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    printf("PASS %s\n", label);
    return 1;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    printf("PASS %s got=%d\n", label, got);
    return 1;
}

static void seed_champion(struct ChampionState_Compat* champ) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    memcpy(champ->name, "HURT", 4);
    champ->portraitIndex = 0;
    champ->direction = DIR_NORTH;
    champ->hp.current = 63;
    champ->hp.maximum = 100;
    champ->stamina.current = 52;
    champ->stamina.maximum = 80;
    champ->mana.current = 25;
    champ->mana.maximum = 60;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    int slot;
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = PROBE_SLOT;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    for (slot = 0; slot < 4; ++slot) {
        game->championDamageTimer[slot] = 0;
        game->championDamageAmount[slot] = 0;
    }
    seed_champion(&game->world.party.champions[PROBE_SLOT]);
}

static int damage_asset_match_count(const M11_GameViewState* game,
                                    const unsigned char* fb,
                                    int slot,
                                    int* outExpected,
                                    int* outMatched) {
    int x, y, w, h;
    int gfx = M11_GameView_GetV1ChampionSmallDamageGraphicId();
    const M11_AssetSlot* asset = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int expected = 0;
    int matched = 0;
    int yy;

    if (!M11_GameView_GetV1DamageIndicatorZone(slot, 45, 7,
                                               &x, &y, &w, &h) ||
        !asset || !asset->loaded || !asset->pixels ||
        asset->width != 45 || asset->height != 7 ||
        w != 45 || h != 7) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char src =
                (unsigned char)(asset->pixels[yy * (int)asset->width + xx] & 0x0F);
            unsigned char dst;
            if (src == 0) {
                continue;
            }
            dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            ++expected;
            if (dst == src) {
                ++matched;
            }
        }
    }

    if (outExpected) {
        *outExpected = expected;
    }
    if (outMatched) {
        *outMatched = matched;
    }
    return expected > 0;
}

static int check_damage_banner_active(const M11_GameViewState* game,
                                      const unsigned char* fb) {
    int expected = 0;
    int matched = 0;
    int ok = 1;

    ok &= expect_true("active damage asset sampled",
                      damage_asset_match_count(game, fb, PROBE_SLOT,
                                               &expected, &matched));
    ok &= expect_true("active C015 damage banner visible",
                      expected > 0 && matched * 100 >= expected * 70);
    printf("active C015 match=%d/%d\n", matched, expected);
    return ok;
}

static int count_white_pixels(const unsigned char* fb,
                              int x,
                              int y,
                              int w,
                              int h) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (px_index(fb, PROBE_FB_W, x + xx, y + yy) == PROBE_COLOR_WHITE) {
                ++count;
            }
        }
    }
    return count;
}

static int check_pc34_damage_number_origin(M11_GameViewState* game,
                                           unsigned char* fb,
                                           int amount,
                                           int expectedX,
                                           int expectedY) {
    int x = 0;
    int y = 0;
    int ok = 1;
    int oldAssetsAvailable = game->assetsAvailable;
    char label[160];

    game->assetsAvailable = 0;
    game->championDamageTimer[PROBE_SLOT] = 5;
    game->championDamageAmount[PROBE_SLOT] = amount;
    memset(fb, 0, PROBE_FB_W * PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    game->assetsAvailable = oldAssetsAvailable;

    snprintf(label, sizeof(label), "damage %d PC34 origin helper", amount);
    ok &= expect_true(label,
                      M11_GameView_GetV1DamageNumberOriginPc34(
                          PROBE_SLOT, amount, 0, &x, &y));
    snprintf(label, sizeof(label), "damage %d PC34 origin x", amount);
    ok &= expect_int(label, x, expectedX);
    snprintf(label, sizeof(label), "damage %d PC34 origin y", amount);
    ok &= expect_int(label, y, expectedY);
    snprintf(label, sizeof(label), "damage %d white text at PC34 origin", amount);
    ok &= expect_true(label, count_white_pixels(fb, x, y, 18, 7) > 0);
    return ok;
}

static int check_pc34_damage_number_origins(M11_GameViewState* game,
                                            unsigned char* fb) {
    int ok = 1;
    int x = 0;
    int y = 0;
    ok &= check_pc34_damage_number_origin(game, fb, 7, 19, 5);
    ok &= check_pc34_damage_number_origin(game, fb, 37, 16, 5);
    ok &= check_pc34_damage_number_origin(game, fb, 137, 13, 5);
    ok &= expect_true("inventory damage 7 PC34 origin helper",
                      M11_GameView_GetV1DamageNumberOriginPc34(
                          PROBE_SLOT, 7, 1, &x, &y));
    ok &= expect_int("inventory damage 7 PC34 origin x", x, 21);
    ok &= expect_int("inventory damage 7 PC34 origin y", y, 16);
    ok &= expect_true("inventory damage 37 PC34 origin helper",
                      M11_GameView_GetV1DamageNumberOriginPc34(
                          PROBE_SLOT, 37, 1, &x, &y));
    ok &= expect_int("inventory damage 37 PC34 origin x", x, 18);
    ok &= expect_int("inventory damage 37 PC34 origin y", y, 16);
    ok &= expect_true("inventory damage 137 PC34 origin helper",
                      M11_GameView_GetV1DamageNumberOriginPc34(
                          PROBE_SLOT, 137, 1, &x, &y));
    ok &= expect_int("inventory damage 137 PC34 origin x", x, 15);
    ok &= expect_int("inventory damage 137 PC34 origin y", y, 16);
    ok &= expect_true("invalid zero damage PC34 origin rejected",
                      !M11_GameView_GetV1DamageNumberOriginPc34(
                          PROBE_SLOT, 0, 0, &x, &y));
    return ok;
}

static int check_damage_banner_cleared(const M11_GameViewState* game,
                                       const unsigned char* fb,
                                       int activeMatched) {
    int expected = 0;
    int matched = 0;
    int ok = 1;

    ok &= expect_true("cleared damage asset sampled",
                      damage_asset_match_count(game, fb, PROBE_SLOT,
                                               &expected, &matched));
    ok &= expect_true("expired timer cleared stale C015 banner",
                      expected > 0 &&
                      matched * 100 < expected * 35 &&
                      matched * 2 < activeMatched);
    printf("cleared C015 residual match=%d/%d active=%d\n",
           matched, expected, activeMatched);
    return ok;
}

static int tick_damage_timer_to_zero(M11_GameViewState* game) {
    int guard = 64;
    while (game->championDamageTimer[PROBE_SLOT] > 0 && guard-- > 0) {
        M11_GameView_TickAnimation(game);
    }
    return game->championDamageTimer[PROBE_SLOT] == 0;
}

static int probe_file_exists(const char* path) {
    FILE* f = fopen(path, "rb");
    if (!f) return 0;
    fclose(f);
    return 1;
}

static int probe_is_pc34_data_dir(const char* path) {
    char graphicsPath[512];
    char dungeonPath[512];
    if (!path || !path[0]) return 0;
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/GRAPHICS.DAT", path);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", path);
    return probe_file_exists(graphicsPath) && probe_file_exists(dungeonPath);
}

/* ctest passes FIRESTAFF_DM1_WORKSPACE_DATA_DIR (~/.firestaff/data/dm1),
 * the documented workspace root, which holds the distribution archives and
 * an extracted dos_extract/ tree rather than loose DAT files.  Walk the same
 * candidate list the sibling HoC probes use and pick the first directory
 * that really contains GRAPHICS.DAT and DUNGEON.DAT. */
static const char* narrow_dm1_data_dir(const char* dataDir,
                                       char* out,
                                       size_t outSize) {
    static const char* const pc34RelativePaths[] = {
        "DATA",
        "dm1/DATA",
        "dos_extract/Dungeon-Master_DOS_EN_Version-34/DATA",
        "dos_extract/Dungeon-Master_DOS_EN_Version-34",
        "dm1/dos_extract/Dungeon-Master_DOS_EN_Version-34/DATA",
        "dm1/dos_extract/Dungeon-Master_DOS_EN_Version-34",
        NULL
    };
    size_t i;
    if (!dataDir || !out || outSize == 0U) return dataDir;
    if (probe_is_pc34_data_dir(dataDir)) {
        snprintf(out, outSize, "%s", dataDir);
        return out;
    }
    for (i = 0; pc34RelativePaths[i] != NULL; ++i) {
        char candidate[512];
        snprintf(candidate, sizeof(candidate), "%s/%s", dataDir,
                 pc34RelativePaths[i]);
        if (probe_is_pc34_data_dir(candidate)) {
            snprintf(out, outSize, "%s", candidate);
            return out;
        }
    }
    return dataDir;
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowedDataDir[512];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int expected = 0;
    int activeMatched = 0;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = narrow_dm1_data_dir(argv[1], narrowedDataDir,
                                 sizeof(narrowedDataDir));

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr, "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    seed_party(&game);
    M11_GameView_NotifyChampionDamage(&game, PROBE_SLOT, PROBE_DAMAGE_AMOUNT);
    ok &= expect_true("notify sets champion damage timer",
                      game.championDamageTimer[PROBE_SLOT] > 0);
    ok &= expect_true("notify records damage amount",
                      game.championDamageAmount[PROBE_SLOT] == PROBE_DAMAGE_AMOUNT);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_damage_banner_active(&game, fb);
    (void)damage_asset_match_count(&game, fb, PROBE_SLOT,
                                   &expected, &activeMatched);
    ok &= check_pc34_damage_number_origins(&game, fb);

    ok &= expect_true("tick animation expires champion damage timer",
                      tick_damage_timer_to_zero(&game));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_damage_banner_cleared(&game, fb, activeMatched);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel damage-flash decay pixel probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
