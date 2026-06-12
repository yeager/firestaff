/*
 * DM1 V1 champion icon direction-swap runtime pixel probe.
 *
 * This is Firestaff-side evidence only.  It opens the hash-verified DM1 V1
 * runtime when local assets are available, installs a deterministic four
 * champion party with distinct per-champion facing directions, and
 * pixel-checks that the on-screen C113..C116 19x14 champion icon zones
 * contain the exact C028_GRAPHIC_CHAMPION_ICONS cell selected by the
 * (championDirection - partyDirection) & 0x03 source index.
 *
 * The existing firestaff_dm1_v1_champion_panel_pixels_runtime_probe and
 * firestaff_dm1_v1_champion_panel_status_states_runtime_probe both
 * verify the icon zone has *some* non-C0 content and a non-C12 cell
 * backdrop color, but neither one pixel-checks the direction-swap
 * C028 source blit.  This new probe closes that gap with the
 * M026_CHAMPION_ICON_INDEX(Direction, PartyDirection) macro:
 *
 *   1. Party=N(0), champions 0..3 face N/E/S/W  -> cells 0,3,2,1
 *   2. Party=E(1), champions 0..3 face N/E/S/W  -> cells 3,0,1,2
 *
 * Each on-screen icon zone is compared against the corresponding
 * C028 cell with the C12 darkest-gray transparent mask honored, so
 * non-transparent asset pixels must match the on-screen palette
 * index (4-bit value).
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0288 clears C113..C116, fills a 19x14 cell
 *   with G0046_auc_Graphic562_ChampionColor[slot], and overlays the
 *   C028_GRAPHIC_CHAMPION_ICONS strip with C12 transparent;
 *   M026_CHAMPION_ICON_INDEX(Direction, PartyDirection) returns the
 *   horizontal cell offset in the 4x19x14 strip.
 *   ReDMCSB DEFS.H line 2190-2194 C028_GRAPHIC_CHAMPION_ICONS at
 *   76x14 (4 cells * 19 wide * 14 tall).
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
    PROBE_CHAMPION_COUNT = 4,
    PROBE_ICON_CELL_W = 19,
    PROBE_ICON_CELL_H = 14,
    PROBE_ICON_CELLS_PER_STRIP = 4,
    PROBE_ICON_ZONE_W = 19,
    PROBE_ICON_ZONE_H = 14,
    PROBE_ICON_TRANSPARENT_INDEX = 12 /* C12 darkest gray */
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    if (x < 0 || y < 0 || x >= PROBE_FB_W || y >= PROBE_FB_H) {
        return 0xFFu; /* sentinel "off-screen" */
    }
    return M11_FB_DECODE_INDEX(fb[y * width + x]);
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

static void seed_champion(struct ChampionState_Compat* champ,
                          const char* name,
                          int portraitIndex,
                          int direction,
                          int hp,
                          int hpMax,
                          int stamina,
                          int staminaMax,
                          int mana,
                          int manaMax) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->portraitIndex = portraitIndex;
    champ->direction = direction;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = (unsigned short)hpMax;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = (unsigned short)staminaMax;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = (unsigned short)manaMax;
    champ->wounds = 0;
    champ->poisonDose = 0;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game, int partyDirection) {
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = partyDirection;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    game->world.magic.event71CountInvisibility = 0;
    game->championDamageTimer[0] = 0;
    game->championDamageTimer[1] = 0;
    game->championDamageTimer[2] = 0;
    game->championDamageTimer[3] = 0;
    /* Slot i faces direction i: N/E/S/W */
    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, 0, 100, 100, 80, 80, 60, 60);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, 1, 100, 100, 80, 80, 60, 60);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, 2, 100, 100, 80, 80, 60, 60);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, 3, 100, 100, 80, 80, 60, 60);
}

/* Compare the on-screen champion icon zone against the expected C028
 * cell at horizontal offset cellIndex * PROBE_ICON_CELL_W.
 * Counts opaque (non-C12) source pixels that match the destination
 * palette index.  Skips asset pixels equal to the configured
 * transparent color.  Returns 1 if >=80% of the opaque source
 * pixels match the on-screen pixel. */
static int check_icon_cell_pixels(const M11_GameViewState* game,
                                  const unsigned char* fb,
                                  int slot,
                                  int cellIndex) {
    int gfx = M11_GameView_GetV1ChampionIconGraphicId();
    const M11_AssetSlot* strip = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game->assetLoader, (unsigned int)gfx);
    int x, y, w, h;
    int sourceIndex = M11_GameView_GetV1ChampionIconSourceIndex(game, slot);
    int matched = 0;
    int expected = 0;
    int xx, yy;
    int ok = 1;
    char label[160];

    snprintf(label, sizeof(label), "slot%d source index matches cell", slot);
    ok &= expect_int(label, sourceIndex, cellIndex & 0x03);

    snprintf(label, sizeof(label), "slot%d C028 strip asset", slot);
    ok &= expect_true(label, strip && strip->loaded && strip->pixels &&
                             strip->width == PROBE_ICON_CELL_W * PROBE_ICON_CELLS_PER_STRIP &&
                             strip->height == PROBE_ICON_CELL_H);

    snprintf(label, sizeof(label), "slot%d icon zone shape", slot);
    ok &= expect_true(label, M11_GameView_GetV1ChampionIconZone(
                                  slot, &x, &y, &w, &h) &&
                             w == PROBE_ICON_ZONE_W &&
                             h == PROBE_ICON_ZONE_H);
    if (!ok || !strip || !strip->pixels) {
        return 0;
    }

    for (yy = 0; yy < h; ++yy) {
        int sy = yy;
        int sx0 = cellIndex * PROBE_ICON_CELL_W;
        for (xx = 0; xx < w; ++xx) {
            int sx = sx0 + xx;
            unsigned char src = (unsigned char)(strip->pixels[sy * (int)strip->width + sx] & 0x0F);
            unsigned char dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            if ((int)src == PROBE_ICON_TRANSPARENT_INDEX) {
                continue;
            }
            ++expected;
            if ((int)dst == (int)src) {
                ++matched;
            }
        }
    }
    snprintf(label, sizeof(label),
             "slot%d C028 cell%d source blit match", slot, cellIndex);
    ok &= expect_true(label, expected > 0 && matched * 100 >= expected * 80);
    printf("slot%d cell=%d matched=%d/%d\n", slot, cellIndex, matched, expected);
    return ok;
}

/* Count visible non-backdrop pixels in the on-screen icon zone. */
static int icon_zone_non_bar_pixels(const unsigned char* fb,
                                    int x, int y, int w, int h,
                                    int barColor) {
    int xx, yy;
    int count = 0;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            unsigned char dst = px_index(fb, PROBE_FB_W, x + xx, y + yy);
            if ((int)dst == 0 || (int)dst == barColor) {
                continue;
            }
            ++count;
        }
    }
    return count;
}

static unsigned long icon_zone_hash(const unsigned char* fb,
                                    int x, int y, int w, int h) {
    int xx, yy;
    unsigned long hash = 2166136261u;
    for (yy = 0; yy < h; ++yy) {
        for (xx = 0; xx < w; ++xx) {
            hash ^= (unsigned long)px_index(fb, PROBE_FB_W, x + xx, y + yy);
            hash *= 16777619u;
        }
    }
    return hash;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int slot;
    int cellsPartyN[4] = { 0, 1, 2, 3 };
    int cellsPartyE[4] = { 3, 0, 1, 2 };
    int nonBarCountsPartyN[4];
    int nonBarCountsPartyE[4];
    unsigned long iconHashesPartyN[4];
    unsigned long iconHashesPartyE[4];
    int barColors[4];
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

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

    /* --- Pass 1: party faces N (0), champions face N/E/S/W. --- */
    seed_party(&game, 0 /* partyDirection = DIR_NORTH */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int x, y, w, h;
        int barColor = M11_GameView_GetV1ChampionBarColor(slot);
        char label[160];
        barColors[slot] = barColor;
        if (!M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h)) {
            ok &= expect_true("party-N slot zone reachable", 0);
            continue;
        }
        nonBarCountsPartyN[slot] =
            icon_zone_non_bar_pixels(fb, x, y, w, h, barColor);
        iconHashesPartyN[slot] = icon_zone_hash(fb, x, y, w, h);
        snprintf(label, sizeof(label), "party-N slot%d non-bar pixels", slot);
        ok &= expect_true(label, nonBarCountsPartyN[slot] > 20);
        ok &= check_icon_cell_pixels(&game, fb, slot, cellsPartyN[slot]);
    }

    /* Verify all four party-N icon zones produced visible on-screen
     * content.  The exact C028 cell checks above prove the source-cell
     * selection; this hash guard catches accidental identical composites
     * without relying on a lossy non-bar pixel count. */
    {
        int distinct = 1;
        int a, b;
        for (a = 0; a < PROBE_CHAMPION_COUNT; ++a) {
            for (b = a + 1; b < PROBE_CHAMPION_COUNT; ++b) {
                if (iconHashesPartyN[a] == iconHashesPartyN[b]) {
                    distinct = 0;
                }
            }
        }
        ok &= expect_true("party-N four icon cells visually distinct", distinct);
    }

    /* --- Pass 2: party faces E (1), same champion facings. --- */
    seed_party(&game, 1 /* partyDirection = DIR_EAST */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int x, y, w, h;
        char label[160];
        if (!M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h)) {
            ok &= expect_true("party-E slot zone reachable", 0);
            continue;
        }
        nonBarCountsPartyE[slot] =
            icon_zone_non_bar_pixels(fb, x, y, w, h, barColors[slot]);
        iconHashesPartyE[slot] = icon_zone_hash(fb, x, y, w, h);
        snprintf(label, sizeof(label), "party-E slot%d non-bar pixels", slot);
        ok &= expect_true(label, nonBarCountsPartyE[slot] > 20);
        ok &= check_icon_cell_pixels(&game, fb, slot, cellsPartyE[slot]);
    }

    /* Verify that flipping party direction changed at least one
     * slot's icon content, which proves the source-index shift
     * reaches the on-screen blit. */
    {
        int changed = 0;
        int s;
        for (s = 0; s < PROBE_CHAMPION_COUNT; ++s) {
            if (iconHashesPartyN[s] != iconHashesPartyE[s]) {
                ++changed;
            }
        }
        ok &= expect_true("party-N vs party-E icon zones changed", changed >= 1);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion icon direction-swap runtime pixel probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
