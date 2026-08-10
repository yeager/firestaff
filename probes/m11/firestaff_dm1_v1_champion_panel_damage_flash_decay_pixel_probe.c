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
 *   This is NOT a missing feature and NOT a parity gap.  The C015 banner is
 *   implemented twice over:
 *     - the legacy champion-panel painter draws it via
 *       dm1_v1_graphic_champion_damage_small_pc34(), placed with
 *       dm1_v1_champion_damage_indicator_rect_pc34() and blitted with
 *       M11_COLOR_FLESH, matching CHAMPION.C F0320; and
 *     - the source-owned top-row receipt path draws it as a
 *       DM1_V1_CHAMPION_REDRAW_DAMAGE_PC34 operation together with the
 *       F0320 damage number.
 *   The receipt chain is wired too: m11_dm1_v1_party_inventory_handoff_from_frame
 *   populates redrawState.pendingDamage/pendingDamageAmount straight from
 *   state->championDamageTimer/championDamageAmount.
 *
 *   What actually happens here: m11_dm1_v1_top_row_receipt_required() is
 *   satisfied on this probe's path (M11_GAME_SOURCE_BUILTIN_CATALOG is a DM1
 *   source kind, debug HUD off, V1 chrome mode on, no V2 slice), so the
 *   source-owned atomic top row governs.  Its receipt then REJECTS, and the
 *   caller deliberately blanks the top-row and status-bar zones and returns
 *   rather than let the legacy painter republish a partial HUD -- see the
 *   "A source-owned top row is atomic" comment in m11_game_view.c.  The
 *   315-pixel all-zero indicator zone this probe reports IS that documented
 *   fail-closed blanking, working as designed.
 *
 *   The open question is therefore narrower than it looked: which material
 *   receipt rejects in this probe's environment, and whether the probe should
 *   be satisfying it or should assert the fail-closed blank instead.
 *
 *   Ruled out while diagnosing: geometry (this probe's
 *   M11_GameView_GetV1DamageIndicatorZone is an inline wrapper around the
 *   very same rect function the legacy draw uses) and asset/font
 *   availability (assetsAvailable and originalFontAvailable are both 1).
 *
 *   Correcting the record twice over: commit 9e4fffcc8 claimed the banner was
 *   "simply not implemented" (wrong -- that came from grepping the unused
 *   M11_GFX_DAMAGE_TO_CHAMPION_SMALL enum, dead code beside a working
 *   implementation), and the follow-up called the residual failure a parity
 *   gap in the receipt path (also wrong -- that path implements the banner;
 *   the blanking is intentional atomicity).
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
#include "firestaff_dm1_probe_portrait_seed.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <string.h>
#include "firestaff_dm1_probe_data_dir.h"

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
    char label[160];

    /* Prior revisions of this helper set state->assetsAvailable = 0 here,
     * apparently to isolate the F0320 damage-number placement from the
     * C015 backing surface. It does not: with assets disabled the DM1
     * source-locked runtime declines to publish EITHER the C015 backing
     * surface OR the M653 source-font damage-number text (m11_draw_text
     * refuses a host-font fallback for a DM1 source frame -- "bygg inget
     * syntetiskt"), and the top-row receipt bails with a BLACK clear-
     * zones over every champion status rect. The resulting all-zero
     * check window made the assertion unsatisfiable while the runtime
     * was behaving correctly. Keep assets enabled so the source-owned
     * atomic top-row publishes the C015 surface AND the F0320 damage-
     * number text at their source-locked origins. */
    game->championDamageTimer[PROBE_SLOT] = 5;
    game->championDamageAmount[PROBE_SLOT] = amount;
    memset(fb, 0, PROBE_FB_W * PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    snprintf(label, sizeof(label), "damage %d PC34 origin helper", amount);
    ok &= expect_true(label,
                      M11_GameView_GetV1DamageNumberOriginPc34(
                          PROBE_SLOT, amount, 0, &x, &y));
    snprintf(label, sizeof(label), "damage %d PC34 origin x", amount);
    ok &= expect_int(label, x, expectedX);
    snprintf(label, sizeof(label), "damage %d PC34 origin y", amount);
    ok &= expect_int(label, y, expectedY);
    snprintf(label, sizeof(label), "damage %d white text at PC34 origin", amount);
    /* CHAMPION.C F0320:1775 hands F0053 the "Y" value 5 for the non-inventory
     * damage-number banner. F0053 forwards it to F0040 which does `subq.w
     * #4,D0` -- the caller's Y is treated as a baseline that sits 4 pixels
     * below the glyph top, and the glyph fills M11_FONT_CHAR_VISIBLE_H (6)
     * rows top-down. So text pixels land at rows (Y-4..Y+1). The prior check
     * window (x, Y, 18, 7) reached from row Y downward and completely missed
     * the glyph. Cover (Y-4..Y+2), the 7 rows that hold the glyph plus its
     * one-row descender guard band. */
    ok &= expect_true(label, count_white_pixels(fb, x, y - 4, 18, 7) > 0);
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
    dataDir = firestaff_dm1_probe_narrow_data_dir(argv[1], narrowedDataDir,
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
    if (!firestaff_dm1_probe_seed_original_portraits(&game, PROBE_CHAMPION_COUNT)) {
        fprintf(stderr, "SKIP could not load DM1 champion portrait atlas from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 0;
    }
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
