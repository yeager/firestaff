/*
 * DM1 V1 champion-panel HUD/status top-row geometry runtime probe.
 *
 * Firestaff-side evidence only.  This loads the local hash-verified DM1
 * GRAPHICS.DAT/DUNGEON.DAT path, seeds a deterministic four-champion party,
 * renders one V1 frame, and checks the source-locked C151..C218 top-row
 * status-box/name/bar/hand geometry against live framebuffer pixels.
 *
 * Source evidence:
 *   ReDMCSB DEFS.H lines 3779-3803 names C151..C218 top-row zones.
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-905 draws live status boxes,
 *   name clear/text zones, bar graphs, and status-hand slots.
 *   ReDMCSB CHAMDRAW.C F0287 lines 307-346 draws C195/C199/C203
 *   bottom-anchored HP/stamina/mana bars.
 *   ReDMCSB CHAMDRAW.C F0293 lines 1117-1139 walks all party champions.
 *   ReDMCSB TIMELINE.C F0260 lines 1817-1830 marks MASK0x1000_STATUS_BOX
 *   before refreshing champion status boxes.
 *
 * This does not claim original DOS screenshot parity.
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
    PROBE_PARTY_COUNT = 4,
    PROBE_STALE_PIXEL = M11_FB_ENCODE(0, 15)
};

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return M11_FB_DECODE_INDEX(fb[y * width + x]);
}

static int count_color(const unsigned char* fb,
                       int width,
                       int x,
                       int y,
                       int w,
                       int h,
                       int color) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if ((int)px_index(fb, width, x + xx, y + yy) == color) {
                ++count;
            }
        }
    }
    return count;
}

static int count_raw_pixel(const unsigned char* fb,
                           int width,
                           int x,
                           int y,
                           int w,
                           int h,
                           unsigned char rawPixel) {
    int count = 0;
    int yy;
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            if (fb[(y + yy) * width + x + xx] == rawPixel) {
                ++count;
            }
        }
    }
    return count;
}

static int expected_fill_height(int current, int maximum, int fullHeight) {
    long scaled;
    if (current <= 0 || maximum <= 0 || fullHeight <= 0) {
        return 0;
    }
    scaled = (long)fullHeight * (long)current / (long)maximum;
    if (scaled < 1) {
        scaled = 1;
    }
    if (scaled > fullHeight) {
        scaled = fullHeight;
    }
    return (int)scaled;
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
                          int manaMax,
                          unsigned short wounds) {
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
    champ->wounds = wounds;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game) {
    game->world.party.championCount = PROBE_PARTY_COUNT;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 2;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 100, 80, 80, 60, 60, 0);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 50, 100, 40, 80, 30, 60, 0x0001u);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 25, 100, 20, 80, 15, 60, 0x0002u);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 1, 100, 1, 80, 1, 60, 0);
}

static int check_rect(const char* label,
                      int gotOk,
                      int gotX,
                      int gotY,
                      int gotW,
                      int gotH,
                      int wantX,
                      int wantY,
                      int wantW,
                      int wantH) {
    int ok = 1;
    char sub[160];
    snprintf(sub, sizeof(sub), "%s available", label);
    ok &= expect_true(sub, gotOk);
    snprintf(sub, sizeof(sub), "%s x", label);
    ok &= expect_int(sub, gotX, wantX);
    snprintf(sub, sizeof(sub), "%s y", label);
    ok &= expect_int(sub, gotY, wantY);
    snprintf(sub, sizeof(sub), "%s w", label);
    ok &= expect_int(sub, gotW, wantW);
    snprintf(sub, sizeof(sub), "%s h", label);
    ok &= expect_int(sub, gotH, wantH);
    return ok;
}

static int check_slot_top_row_geometry(const M11_GameViewState* game,
                                       const unsigned char* fb,
                                       int slot) {
    const struct ChampionState_Compat* champ =
        &game->world.party.champions[slot];
    const int slotX = slot * 69;
    const int barX[3] = {46, 53, 60};
    const int cur[3] = {
        (int)champ->hp.current,
        (int)champ->stamina.current,
        (int)champ->mana.current
    };
    const int max[3] = {
        (int)champ->hp.maximum,
        (int)champ->stamina.maximum,
        (int)champ->mana.maximum
    };
    int ok = 1;
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int stat;
    int hand;
    char label[160];

    snprintf(label, sizeof(label), "slot%d C151 status-box id", slot);
    ok &= expect_int(label, M11_GameView_GetV1StatusBoxZoneId(slot), 151 + slot);
    snprintf(label, sizeof(label), "slot%d C151 status-box rect", slot);
    ok &= check_rect(label,
                     M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h),
                     x, y, w, h,
                     slotX, 0, 67, 29);
    snprintf(label, sizeof(label), "slot%d C151 status-box touched", slot);
    ok &= expect_int(label,
                     count_raw_pixel(fb, PROBE_FB_W, slotX, 0, 67, 29,
                                     (unsigned char)PROBE_STALE_PIXEL),
                     0);

    snprintf(label, sizeof(label), "slot%d C159 name-clear id", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1StatusNameClearZoneId(slot),
                     159 + slot);
    snprintf(label, sizeof(label), "slot%d C159 name-clear rect", slot);
    ok &= check_rect(label,
                     M11_GameView_GetV1StatusNameZone(slot, &x, &y, &w, &h),
                     x, y, w, h,
                     slotX, 0, 43, 7);
    snprintf(label, sizeof(label), "slot%d C159 name-clear dark pixels", slot);
    ok &= expect_true(label,
                      count_color(fb, PROBE_FB_W, slotX, 0, 43, 7,
                                  M11_GameView_GetV1StatusNameClearColor()) > 180);

    snprintf(label, sizeof(label), "slot%d C163 name-text id", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1StatusNameTextZoneId(slot),
                     163 + slot);
    snprintf(label, sizeof(label), "slot%d C163 name-text rect", slot);
    ok &= check_rect(label,
                     M11_GameView_GetV1StatusNameTextZone(slot, &x, &y, &w, &h),
                     x, y, w, h,
                     slotX + 1, 0, 42, 7);
    snprintf(label, sizeof(label), "slot%d C163 name-text color", slot);
    ok &= expect_true(label,
                      count_color(fb, PROBE_FB_W, slotX + 1, 0, 42, 7,
                                  M11_GameView_GetV1StatusNameColor(game, slot)) > 0);

    snprintf(label, sizeof(label), "slot%d C187 bar-graph id", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1StatusBarGraphZoneId(slot),
                     187 + slot);
    for (stat = 0; stat < 3; ++stat) {
        int fillHeight;
        int blankHeight;
        int valueZone = 195 + slot + stat * 4;
        snprintf(label, sizeof(label), "slot%d stat%d C195/C199/C203 root id",
                 slot, stat);
        ok &= expect_int(label,
                         M11_GameView_GetV1StatusBarZoneId(stat),
                         195 + stat * 4);
        snprintf(label, sizeof(label), "slot%d stat%d value-zone id", slot, stat);
        ok &= expect_int(label,
                         M11_GameView_GetV1StatusBarValueZoneId(slot, stat),
                         valueZone);
        snprintf(label, sizeof(label), "slot%d stat%d value-zone rect", slot, stat);
        ok &= check_rect(label,
                         M11_GameView_GetV1StatusBarZone(slot, stat,
                                                          &x, &y, &w, &h),
                         x, y, w, h,
                         slotX + barX[stat], 4, 4, 25);
        fillHeight = expected_fill_height(cur[stat], max[stat], 25);
        blankHeight = 25 - fillHeight;
        snprintf(label, sizeof(label), "slot%d stat%d value-zone blank top",
                 slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     slotX + barX[stat], 4,
                                     4, blankHeight,
                                     M11_GameView_GetV1StatusBarBlankColor()),
                         4 * blankHeight);
        snprintf(label, sizeof(label), "slot%d stat%d value-zone color bottom",
                 slot, stat);
        ok &= expect_int(label,
                         count_color(fb, PROBE_FB_W,
                                     slotX + barX[stat], 4 + blankHeight,
                                     4, fillHeight,
                                     M11_GameView_GetV1ChampionBarColor(slot)),
                         4 * fillHeight);
    }

    snprintf(label, sizeof(label), "slot%d C207 hand-parent id", slot);
    ok &= expect_int(label,
                     M11_GameView_GetV1StatusHandParentZoneId(slot),
                     207 + slot);
    for (hand = 0; hand < 2; ++hand) {
        const int handX = slotX + (hand == 0 ? 4 : 24);
        const int handZoneId = 211 + slot * 2 + hand;
        snprintf(label, sizeof(label), "slot%d hand%d C211/C212 id", slot, hand);
        ok &= expect_int(label,
                         M11_GameView_GetV1StatusHandZoneId(slot, hand),
                         handZoneId);
        snprintf(label, sizeof(label), "slot%d hand%d C211/C212 rect",
                 slot, hand);
        ok &= check_rect(label,
                         M11_GameView_GetV1StatusHandZone(slot, hand,
                                                           &x, &y, &w, &h),
                         x, y, w, h,
                         handX, 10, 16, 16);
        snprintf(label, sizeof(label), "slot%d hand%d slot-box blit rect",
                 slot, hand);
        ok &= check_rect(label,
                         M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                                  &x, &y, &w, &h),
                         x, y, w, h,
                         handX, 10, 18, 18);
        snprintf(label, sizeof(label), "slot%d hand%d live pixels", slot, hand);
        ok &= expect_int(label,
                         count_raw_pixel(fb, PROBE_FB_W,
                                         handX, 10, 18, 18,
                                         (unsigned char)PROBE_STALE_PIXEL),
                         0);
    }

    return ok;
}

static int check_invalid_guards(void) {
    int ok = 1;
    ok &= expect_int("invalid status-box low", M11_GameView_GetV1StatusBoxZoneId(-1), 0);
    ok &= expect_int("invalid status-box high", M11_GameView_GetV1StatusBoxZoneId(4), 0);
    ok &= expect_int("invalid name-clear high", M11_GameView_GetV1StatusNameClearZoneId(4), 0);
    ok &= expect_int("invalid name-text low", M11_GameView_GetV1StatusNameTextZoneId(-1), 0);
    ok &= expect_int("invalid bar graph high", M11_GameView_GetV1StatusBarGraphZoneId(4), 0);
    ok &= expect_int("invalid bar stat low", M11_GameView_GetV1StatusBarZoneId(-1), 0);
    ok &= expect_int("invalid bar stat high", M11_GameView_GetV1StatusBarZoneId(3), 0);
    ok &= expect_int("invalid bar value slot", M11_GameView_GetV1StatusBarValueZoneId(4, 0), 0);
    ok &= expect_int("invalid bar value stat", M11_GameView_GetV1StatusBarValueZoneId(0, 3), 0);
    ok &= expect_int("invalid hand parent high", M11_GameView_GetV1StatusHandParentZoneId(4), 0);
    ok &= expect_int("invalid hand zone low", M11_GameView_GetV1StatusHandZoneId(-1, 0), 0);
    ok &= expect_int("invalid hand zone hand", M11_GameView_GetV1StatusHandZoneId(0, 2), 0);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int slot;
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

    seed_party(&game);
    memset(fb, PROBE_STALE_PIXEL, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= check_invalid_guards();
    for (slot = 0; slot < PROBE_PARTY_COUNT; ++slot) {
        ok &= check_slot_top_row_geometry(&game, fb, slot);
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion-panel HUD/status top-row geometry runtime probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
