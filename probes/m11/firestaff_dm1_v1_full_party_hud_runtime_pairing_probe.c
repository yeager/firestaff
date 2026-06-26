/*
 * DM1 V1 full four-champion party HUD + single-champion status-panel
 * runtime pairing probe.
 *
 * This probe is the Firestaff-side runtime + pixel companion to
 * pass1071 (DM1 V1 champion-panel pairing readiness). pass1071 is the
 * existence-of-original-artifacts gate; this probe is the
 * Firestaff-side runtime + pixel gate that proves the live draw
 * stack:
 *
 *   1. Populates the full four-champion party HUD (C151..C154 status
 *      boxes, C195..C206 stat bars, C113..C116 champion icons,
 *      C211..C218 hand slot boxes).
 *   2. Populates the single-champion status panel (only slot 0).
 *   3. Actively clears slot 1/2/3 when the party shrinks below 4.
 *   4. Repopulates slot 1/2/3 when the party grows back to 4 after
 *      a previous single-champion draw.
 *   5. Emits a per-terminal-state 64-bit FNV-1a panel-region
 *      fingerprint so the Python verifier can detect drift across
 *      runs / probes / asset shuffles.
 *
 * Compared to the existing
 * firestaff_dm1_v1_champion_panel_partial_party_pixel_probe this
 * probe (a) covers the maximum four-champion HUD state which the
 * existing probe intentionally walks around, (b) covers the
 * minimum single-champion state as an isolated row instead of just
 * one step of a 1..3 sweep, and (c) verifies the two-way transition
 * between the two terminal cases so the M11 draw stack is provably
 * clearing inactive slots and repopulating them on demand.
 *
 * Honesty boundary:
 *   - This probe does not compare Firestaff against original DM1
 *     PC 3.4 pixels.
 *   - This probe does not claim full four-champion HUD or
 *     single-champion status-panel parity.
 *   - The actual same-state original pairing is still BLOCKED in
 *     pass1071; this probe only proves the live V1 draw stack
 *     matches the source-locked ReDMCSB geometry + active-loop
 *     bound under real DM1 V1 GRAPHICS.DAT assets.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292 lines 771-789 STATUS_BOX fill:
 *     alive status boxes for slot i draw C151+i with C12 background.
 *   ReDMCSB CHAMDRAW.C F0293 lines 1134-1138 active-champion loop
 *     visits only indices < G0305_ui_PartyChampionCount.
 *   ReDMCSB CHAMDRAW.C F0292 lines 893-905 draws the HP/stamina/
 *     mana bars for live champions.
 *   ReDMCSB CHAMDRAW.C F0622 lines 41-58 prepares the C113..C116
 *     19x14 champion icon composite.
 *   ReDMCSB CHAMDRAW.C F0291 lines 632-646 renders hand slot boxes
 *     C211..C218 with C033/C034/C035 for normal/wounded/acting.
 *   ReDMCSB DEFS.H:2157 anchors the 69px status-box stride.
 *   ReDMCSB DEFS.H:3800-3807 anchors the C211..C218 status-hand
 *     zone IDs.
 *   ReDMCSB layout-696 C151..C154 status-box zone IDs.
 *   ReDMCSB layout-696 C195..C206 status-bar zone IDs.
 *   ReDMCSB layout-696 C113..C116 champion-icon zone IDs.
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
    PROBE_BLACK = 0,

    /* Lower-bound pixel counts for an alive / populated sub-region.
     * Status box 67x29 = 1943 px total; champion icon 19x14 = 266 px;
     * hand slot box 18x18 = 324 px; stat bar 4x25 = 100 px. The
     * minimum thresholds below are deliberately conservative so the
     * probe survives minor palette / index noise from real GRAPHICS.DAT
     * assets without becoming flaky. */
    PROBE_STATUS_BOX_MIN_NONBLACK = 500,
    PROBE_STAT_BAR_NONBLACK_FULL  = 100, /* 4*25 == 100 */
    PROBE_HAND_BOX_MIN_NONBLACK = 60,
    PROBE_CHAMPION_ICON_MIN_NONBLACK = 20,
    PROBE_NAME_COLOR_MIN_COUNT = 1,

    /* Panel region covers all four status boxes side by side:
     *   x = 0..(4*69 - 2) = 274
     *   y = 0..29         = 29
     * Total panel width 274 px, height 29 px. */
    PROBE_PANEL_X = 0,
    PROBE_PANEL_Y = 0,
    PROBE_PANEL_W = 274,
    PROBE_PANEL_H = 29,
};

/* FNV-1a 64-bit over the panel-region FNV-1a (status boxes only).
 * The hash is content-only (no width / height anchors) so two
 * different framings of the same pixel content produce the same
 * fingerprint. */
static unsigned long long fnv1a64(const unsigned char* fb, int width,
                                   int x, int y, int w, int hPixels) {
    unsigned long long fp = 0xcbf29ce484222325ULL;
    int yy;
    if (x < 0 || y < 0 || x + w > width || y + hPixels > 200 || !fb) {
        return 0;
    }
    for (yy = 0; yy < hPixels; ++yy) {
        const unsigned char* row = fb + (y + yy) * width + x;
        int xx;
        for (xx = 0; xx < w; ++xx) {
            fp ^= (unsigned long long)row[xx];
            fp *= 0x100000001b3ULL;
        }
    }
    return fp;
}

static unsigned char px_index(const unsigned char* fb, int width, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * width + x]);
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
    if (x < 0 || y < 0 || x + w > width || y + h > 200) {
        return -1;
    }
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

static int count_nonblack(const unsigned char* fb,
                          int width,
                          int x,
                          int y,
                          int w,
                          int h) {
    if (x < 0 || y < 0 || x + w > width || y + h > 200) {
        return -1;
    }
    return w * h - count_color(fb, width, x, y, w, h, PROBE_BLACK);
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
                          int direction,
                          int hp,
                          int hpMax,
                          int stamina,
                          int mana,
                          unsigned short wounds) {
    int i;
    memset(champ, 0, sizeof(*champ));
    champ->present = 1;
    memset(champ->name, ' ', sizeof(champ->name));
    for (i = 0; name[i] && i < 8; ++i) {
        champ->name[i] = name[i];
    }
    champ->direction = direction;
    champ->portraitIndex = i & 3;
    champ->hp.current = (unsigned short)hp;
    champ->hp.maximum = (unsigned short)hpMax;
    champ->stamina.current = (unsigned short)stamina;
    champ->stamina.maximum = 80;
    champ->mana.current = (unsigned short)mana;
    champ->mana.maximum = 60;
    champ->wounds = wounds;
    for (i = 0; i < CHAMPION_SLOT_COUNT; ++i) {
        champ->inventory[i] = THING_NONE;
    }
}

static void seed_party(M11_GameViewState* game, int championCount) {
    int slot;
    game->world.party.championCount = championCount;
    game->world.party.activeChampionIndex = 0;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0;
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;

    seed_champion(&game->world.party.champions[0],
                  "TIGGY", DIR_NORTH, 100, 100, 80, 60, 0);
    seed_champion(&game->world.party.champions[1],
                  "HALK", DIR_EAST, 65, 100, 44, 20, 0);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", DIR_SOUTH, 25, 100, 20, 10,
                  (unsigned short)(1u << 3));
    seed_champion(&game->world.party.champions[3],
                  "ALEX", DIR_WEST, 10, 100, 8, 3,
                  (unsigned short)((1u << 2) | (1u << 4)));
    for (slot = championCount; slot < PROBE_CHAMPION_COUNT; ++slot) {
        memset(&game->world.party.champions[slot], 0,
               sizeof(game->world.party.champions[slot]));
    }
}

/* Asserts that every alive-champion zone for `slot` is populated. */
static int check_alive_slot(const M11_GameViewState* game,
                            const unsigned char* fb,
                            int slot,
                            const char* partyLabel) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    int hand;
    char label[128];

    snprintf(label, sizeof(label), "%s slot%d status box populated",
             partyLabel, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        w == 67 && h == 29 &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) >=
            PROBE_STATUS_BOX_MIN_NONBLACK);

    snprintf(label, sizeof(label), "%s slot%d status-box stride 69",
             partyLabel, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        x == slot * 69);

    snprintf(label, sizeof(label), "%s slot%d name color present",
             partyLabel, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusNameTextZone(slot, &x, &y, &w, &h) &&
        count_color(fb, PROBE_FB_W, x, y, w, h,
                    M11_GameView_GetV1StatusNameColor(game, slot)) >=
            PROBE_NAME_COLOR_MIN_COUNT);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label), "%s slot%d stat%d bar populated",
                 partyLabel, slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            w == 4 && h == 25 &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) ==
                PROBE_STAT_BAR_NONBLACK_FULL);
    }

    snprintf(label, sizeof(label), "%s slot%d champion icon populated",
             partyLabel, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) >=
            PROBE_CHAMPION_ICON_MIN_NONBLACK);

    for (hand = 0; hand < 2; ++hand) {
        snprintf(label, sizeof(label),
                 "%s slot%d hand%d slot-box graphic available",
                 partyLabel, slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotGraphic(game, slot, hand) != 0);
        snprintf(label, sizeof(label),
                 "%s slot%d hand%d slot-box populated",
                 partyLabel, slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h) &&
            w == 18 && h == 18 &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) >=
                PROBE_HAND_BOX_MIN_NONBLACK);
    }
    return ok;
}

/* Asserts that slot >= partyCount is fully empty: no stale pixels
 * from a previous draw, no name color available. */
static int check_empty_slot(const M11_GameViewState* game,
                            const unsigned char* fb,
                            int slot,
                            const char* partyLabel) {
    int ok = 1;
    int x, y, w, h;
    int stat;
    int hand;
    char label[128];

    snprintf(label, sizeof(label), "%s slot%d status box empty",
             partyLabel, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    snprintf(label, sizeof(label), "%s slot%d name color unavailable",
             partyLabel, slot);
    ok &= expect_int(label, M11_GameView_GetV1StatusNameColor(game, slot), -1);

    snprintf(label, sizeof(label), "%s slot%d champion icon empty",
             partyLabel, slot);
    ok &= expect_true(label,
        M11_GameView_GetV1ChampionIconZone(slot, &x, &y, &w, &h) &&
        count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);

    for (stat = 0; stat < 3; ++stat) {
        snprintf(label, sizeof(label), "%s slot%d stat%d bar empty",
                 partyLabel, slot, stat);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusBarZone(slot, stat, &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);
    }
    for (hand = 0; hand < 2; ++hand) {
        snprintf(label, sizeof(label),
                 "%s slot%d hand%d slot-box graphic unavailable",
                 partyLabel, slot, hand);
        ok &= expect_int(label,
                         M11_GameView_GetV1StatusHandSlotGraphic(game,
                                                                 slot,
                                                                 hand),
                         0);
        snprintf(label, sizeof(label), "%s slot%d hand%d zone empty",
                 partyLabel, slot, hand);
        ok &= expect_true(label,
            M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                    &x, &y, &w, &h) &&
            count_nonblack(fb, PROBE_FB_W, x, y, w, h) == 0);
    }
    return ok;
}

/* Run the four terminal / transition cases plus the 2-/3-champion
 * intermediate cases. Each terminal case emits its own panel-region
 * FNV-1a64 fingerprint so the Python verifier can detect drift. */
static int run_terminal_pair(M11_GameViewState* game,
                             unsigned char* fb) {
    int ok = 1;
    int slot;
    unsigned long long fp;

    /* Case 1: full4 HUD. All four status boxes populated. */
    seed_party(game, PROBE_CHAMPION_COUNT);
    memset(fb, 0x00, (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_alive_slot(game, fb, slot, "full4");
    }
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO full4_panel_fnv1a64=0x%016llx\n", fp);

    /* Case 2: single1 status panel. Only slot 0 populated. */
    seed_party(game, 1);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_alive_slot(game, fb, 0, "single1");
    for (slot = 1; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_empty_slot(game, fb, slot, "single1");
    }
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO single1_panel_fnv1a64=0x%016llx\n", fp);

    /* Case 3: two-champion intermediate. */
    seed_party(game, 2);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_alive_slot(game, fb, 0, "two2");
    ok &= check_alive_slot(game, fb, 1, "two2");
    for (slot = 2; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_empty_slot(game, fb, slot, "two2");
    }
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO two2_panel_fnv1a64=0x%016llx\n", fp);

    /* Case 4: three-champion intermediate. */
    seed_party(game, 3);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 0; slot < 3; ++slot) {
        ok &= check_alive_slot(game, fb, slot, "three3");
    }
    ok &= check_empty_slot(game, fb, 3, "three3");
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO three3_panel_fnv1a64=0x%016llx\n", fp);

    /* Case 5: single1 -> full4 transition. Slot 1/2/3 must be
     * repopulated by the post-draw M11 stack. */
    seed_party(game, 1);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    seed_party(game, PROBE_CHAMPION_COUNT);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 1; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_alive_slot(game, fb, slot, "single1->full4");
    }
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO single1_to_full4_panel_fnv1a64=0x%016llx\n", fp);

    /* Case 6: full4 -> single1 transition. Slot 1/2/3 must be
     * cleared by the post-draw M11 stack. */
    seed_party(game, PROBE_CHAMPION_COUNT);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    seed_party(game, 1);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    for (slot = 1; slot < PROBE_CHAMPION_COUNT; ++slot) {
        ok &= check_empty_slot(game, fb, slot, "full4->single1");
    }
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO full4_to_single1_panel_fnv1a64=0x%016llx\n", fp);

    /* Case 7: full4 -> two2 -> full4 transition. Slot 2/3 must
     * be cleared, then repopulated. */
    seed_party(game, PROBE_CHAMPION_COUNT);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    seed_party(game, 2);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_empty_slot(game, fb, 2, "full4->two2");
    ok &= check_empty_slot(game, fb, 3, "full4->two2");
    seed_party(game, PROBE_CHAMPION_COUNT);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    ok &= check_alive_slot(game, fb, 2, "two2->full4");
    ok &= check_alive_slot(game, fb, 3, "two2->full4");
    fp = fnv1a64(fb, PROBE_FB_W, PROBE_PANEL_X, PROBE_PANEL_Y,
                 PROBE_PANEL_W, PROBE_PANEL_H);
    printf("INFO two2_to_full4_panel_fnv1a64=0x%016llx\n", fp);

    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "SKIP dm1 v1 full_party_hud_runtime_pairing_probe: "
                "could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 2;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr,
                "SKIP dm1 v1 full_party_hud_runtime_pairing_probe: "
                "GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 2;
    }

    /* Warm the framebuffer with a known party so the first terminal
     * case has consistent prior content for the empty-slot checks. */
    seed_party(&game, PROBE_CHAMPION_COUNT);
    memset(fb, 0x0F, sizeof(fb) / sizeof(fb[0]));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= run_terminal_pair(&game, fb);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 full_party_hud_runtime_pairing_probe "
           "(Firestaff-side evidence, no original DOS claim)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
