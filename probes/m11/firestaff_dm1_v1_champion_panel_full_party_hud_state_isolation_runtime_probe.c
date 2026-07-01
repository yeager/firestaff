/*
 * DM1 V1 champion panel single-champion state isolation runtime pixel probe.
 *
 * This is Firestaff-side evidence only. It opens the hash-verified DM1 V1
 * runtime when local assets are available, installs a deterministic four
 * champion party, renders one V1 frame (frame A) into a fresh framebuffer,
 * mutates only slot 1's HP/stamina/mana and re-renders into a fresh
 * framebuffer (frame B), and then pixel-checks the resulting frames for the
 * following invariants that the existing single-frame and zero-party probes
 * do not cover:
 *
 *   1. Frame A is byte-stable across two renders with the same party state
 *      (proves the M11 draw stack is deterministic; covers the lower
 *      "pre-mutation" baseline before the single-champion mutation).
 *
 *   2. Slot 1's bar graphs have a different fill count in frame B than in
 *      frame A (proves the M11 draw stack routes the HP/stamina/mana
 *      mutation through the slot-1 status box only, not through the other
 *      three slots' bar zones).
 *
 *   3. Slots 0, 2, 3 status box, name text, status bar, status hand slot,
 *      champion icon, and action icon cell zones are byte-stable between
 *      frame A and frame B (proves single-champion state mutation does not
 *      bleed into other slots and the F0292 per-champion draw dispatch is
 *      isolated).
 *
 *   4. Slot 1's status box zone is NOT byte-stable between frame A and
 *      frame B (proves the mutation actually reached the framebuffer, not
 *      just the synthetic state struct; this is the negative complement
 *      of invariant 3 for the mutated slot).
 *
 *   5. After mutation, all four status box zones still contain the
 *      C12 dark-gray fill pixels (proves the party HUD is still showing
 *      four champions and the slot-1 mutation did not collapse the
 *      four-champion layout into a three-champion layout).
 *
 *   6. The status box gutter between adjacent slots (two black pixels,
 *      width = 2 px on V1 status box stride = 69 px) remains
 *      byte-stable across the mutation (proves the per-slot stride is
 *      independent of the per-slot mutation).
 *
 *   7. The leader name color cascade (slot 0 leader C11 yellow, slots
 *      1..3 non-leader C09 orange) remains correct after the slot-1
 *      mutation (proves the leader rotation code path was not disturbed
 *      by the single-champion state mutation; this is independent of the
 *      bar / status box isolation checks because the leader name color
 *      is keyed on activeChampionIndex, not on HP/stamina/mana values).
 *
 * The probe does not claim original DOS screenshot parity. All assertions
 * are Firestaff-side evidence that the F0292 per-champion redraw dispatch
 * isolates the state mutation to the mutated slot.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292_CHAMPION_DrawState:816-842 draws C008 for
 *     dead champions and exits before F0287 bar graphs and F0291 hand
 *     slots can overdraw C008.
 *   ReDMCSB CHAMDRAW.C F0293_CHAMPION_DrawAllChampionStates:1117-1143
 *     iterates from C00_CHAMPION_FIRST while championIndex <
 *     G0305_ui_PartyChampionCount and calls F0292 once per active
 *     champion (so per-champion isolation is owned by F0292, not F0293).
 *   ReDMCSB CHAMDRAW.C F0287 draws bottom-anchored HP/stamina/mana bar
 *     graphs from champion.HP/Stamina/Mana current/maximum, so the
 *     bar fill count is the most direct visible signal of a HP/stamina/
 *     mana mutation reaching the framebuffer.
 *   ReDMCSB CHAMDRAW.C F0291 draws the 18x18 hand slot box at
 *     champIdx*69+4 / +24, which is unaffected by HP/stamina/mana mutation.
 *   ReDMCSB CHAMDRAW.C F0292: lines 879-905 refreshes live status box
 *     name strip and bars before drawing the name text; the F0292
 *     status-box refresh writes the full 67x29 rectangle for each
 *     active champion, which lets the mutated slot's bar zone change
 *     while the unmutated slots' bar zones stay byte-stable.
 *   ReDMCSB COORD.C/layout-696 C151..C154 status box stride 69 px with
 *     67-wide status box (two black gutter pixels between adjacent
 *     status boxes).
 *   ReDMCSB COORD.C M026_CHAMPION_ICON_INDEX(Direction, PartyDirection)
 *     macro = (Direction - PartyDirection) & 0x03 is independent of
 *     HP/stamina/mana.
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

    /* Source-locked DM PC VGA color indices. */
    PROBE_FILL_COLOR = 12, /* C12 = M11_COLOR_DARK_GRAY */
    PROBE_BLANK_BAR_COLOR = 12, /* C12 = M11_COLOR_DARK_GRAY */
    PROBE_LEADER_NAME_COLOR = 11, /* C11 = M11_COLOR_YELLOW */
    PROBE_NONLEADER_NAME_COLOR = 9, /* C09 = M11_COLOR_ORANGE */
    PROBE_BLACK = 0
};

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

static int zone_byte_equal(const unsigned char* fbA,
                           const unsigned char* fbB,
                           int width,
                           int x,
                           int y,
                           int w,
                           int h) {
    int yy;
    if (x < 0 || y < 0 || x + w > width || y + h > 200) {
        return 0;
    }
    for (yy = 0; yy < h; ++yy) {
        int xx;
        for (xx = 0; xx < w; ++xx) {
            unsigned char a = fbA[(y + yy) * width + (x + xx)];
            unsigned char b = fbB[(y + yy) * width + (x + xx)];
            if (a != b) {
                return 0;
            }
        }
    }
    return 1;
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

static void seed_full_party(M11_GameViewState* game) {
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = 0; /* slot 0 is the leader */
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->resting = 0;
    game->candidateMirrorOrdinal = 0;
    game->candidateMirrorPanelActive = 0;
    game->actingChampionOrdinal = 2; /* slot 1 action hand uses C035 */
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
                  "ALEX", 3, DIR_WEST, 10, 100, 10, 80, 10, 60, 0);
}

static void mutate_slot1_state(M11_GameViewState* game) {
    /* Slot 1 (HALK): shift all three stat bars to a clearly different
     * fill count so the bar-graph zone differs from frame A.  ReDMCSB
     * CHAMPION.C clamps current to maximum and rejects negative current,
     * so this mutation is in the legal range. */
    game->world.party.champions[1].hp.current = 95;
    game->world.party.champions[1].stamina.current = 70;
    game->world.party.champions[1].mana.current = 50;
    /* A wound flag flip is also acceptable as a mutation signal but
     * keeps the per-champion isolation semantics identical because
     * F0292 routes wounds via the C195..C206 bar zones the same way
     * it routes HP/stamina/mana. */
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fbA1[PROBE_FB_W * PROBE_FB_H];
    unsigned char fbA2[PROBE_FB_W * PROBE_FB_H];
    unsigned char fbB [PROBE_FB_W * PROBE_FB_H];
    int slot;
    int stat;
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
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!game.assetsAvailable) {
        fprintf(stderr,
                "FAIL DM1 V1 GRAPHICS.DAT assets unavailable from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ---- Frame A baseline: two renders must be byte-identical ---- */
    seed_full_party(&game);
    memset(fbA1, 0, sizeof(fbA1));
    M11_GameView_Draw(&game, fbA1, PROBE_FB_W, PROBE_FB_H);
    memset(fbA2, 0, sizeof(fbA2));
    M11_GameView_Draw(&game, fbA2, PROBE_FB_W, PROBE_FB_H);
    {
        int i;
        int aEqual = 1;
        for (i = 0; i < (int)sizeof(fbA1); ++i) {
            if (fbA1[i] != fbA2[i]) {
                aEqual = 0;
                break;
            }
        }
        ok &= expect_true("frame A baseline render A==render B (deterministic M11 draw stack)",
                          aEqual);
    }

    /* All four slots' status boxes populated before mutation. */
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int x, y, w, h;
        int fillsCount;
        char label[128];
        snprintf(label, sizeof(label),
                 "frame A slot%d status box filled", slot);
        fillsCount = M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h)
            ? count_color(fbA1, PROBE_FB_W, x, y, w, h, PROBE_FILL_COLOR)
            : -1;
        ok &= expect_true(label, fillsCount > 0);
    }

    /* Pre-mutation slot-1 bar fill counts. */
    int preFill[3] = { 0, 0, 0 };
    for (stat = 0; stat < 3; ++stat) {
        int x, y, w, h;
        int preBar;
        if (!M11_GameView_GetV1StatusBarZone(1, stat, &x, &y, &w, &h)) {
            ok &= expect_true("frame A slot1 stat bar zone lookup", 0);
            continue;
        }
        preBar = count_color(fbA1, PROBE_FB_W, x, y, w, h,
                             M11_GameView_GetV1ChampionBarColor(1));
        preFill[stat] = preBar;
        char label[128];
        snprintf(label, sizeof(label),
                 "frame A slot1 stat%d bar fill count recorded (=%d)", stat, preBar);
        ok &= expect_true(label, preBar > 0);
    }

    /* Leader name color cascade at frame A. */
    ok &= expect_int("frame A slot0 leader name color C11",
                     M11_GameView_GetV1StatusNameColor(&game, 0),
                     PROBE_LEADER_NAME_COLOR);
    ok &= expect_int("frame A slot1 non-leader name color C09",
                     M11_GameView_GetV1StatusNameColor(&game, 1),
                     PROBE_NONLEADER_NAME_COLOR);
    ok &= expect_int("frame A slot2 non-leader name color C09",
                     M11_GameView_GetV1StatusNameColor(&game, 2),
                     PROBE_NONLEADER_NAME_COLOR);
    ok &= expect_int("frame A slot3 non-leader name color C09",
                     M11_GameView_GetV1StatusNameColor(&game, 3),
                     PROBE_NONLEADER_NAME_COLOR);

    /* ---- Mutate only slot 1 and render frame B ---- */
    mutate_slot1_state(&game);
    memset(fbB, 0, sizeof(fbB));
    M11_GameView_Draw(&game, fbB, PROBE_FB_W, PROBE_FB_H);

    /* Slot 1's three bar fill counts must have changed (mutation reached
     * the framebuffer through the F0292 slot-1 redraw path). */
    for (stat = 0; stat < 3; ++stat) {
        int x, y, w, h;
        int postBar;
        if (!M11_GameView_GetV1StatusBarZone(1, stat, &x, &y, &w, &h)) {
            ok &= expect_true("frame B slot1 stat bar zone lookup", 0);
            continue;
        }
        postBar = count_color(fbB, PROBE_FB_W, x, y, w, h,
                              M11_GameView_GetV1ChampionBarColor(1));
        char label[128];
        snprintf(label, sizeof(label),
                 "frame B slot1 stat%d bar fill count differs from frame A "
                 "(pre=%d post=%d)", stat, preFill[stat], postBar);
        ok &= expect_true(label, postBar != preFill[stat]);
    }

    /* Slots 0/2/3 status box, name, bar, hand slot box, champion icon,
     * action icon cell zones are byte-stable between frame A and frame B. */
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        if (slot == 1) {
            continue;
        }
        int x, y, w, h;
        int nx, ny, nw, nh;
        int ntx, nty, ntw, nth;
        int bx, by, bw, bh;
        int handX, handY, handW, handH;
        int iconX, iconY, iconW, iconH;
        int cellX, cellY, cellW, cellH;
        char label[256];

        snprintf(label, sizeof(label),
                 "slot%d status box zone byte-stable across slot1 mutation",
                 slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                          zone_byte_equal(fbA1, fbB, PROBE_FB_W, x, y, w, h));

        snprintf(label, sizeof(label),
                 "slot%d name clear zone byte-stable across slot1 mutation",
                 slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusNameZone(slot, &nx, &ny, &nw, &nh) &&
                          zone_byte_equal(fbA1, fbB, PROBE_FB_W, nx, ny, nw, nh));

        snprintf(label, sizeof(label),
                 "slot%d name text zone byte-stable across slot1 mutation",
                 slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1StatusNameTextZone(slot,
                                                                &ntx, &nty,
                                                                &ntw, &nth) &&
                          zone_byte_equal(fbA1, fbB, PROBE_FB_W, ntx, nty, ntw, nth));

        for (stat = 0; stat < 3; ++stat) {
            snprintf(label, sizeof(label),
                     "slot%d stat%d bar zone byte-stable across slot1 mutation",
                     slot, stat);
            ok &= expect_true(label,
                              M11_GameView_GetV1StatusBarZone(slot, stat,
                                                             &bx, &by,
                                                             &bw, &bh) &&
                              zone_byte_equal(fbA1, fbB, PROBE_FB_W, bx, by, bw, bh));
        }

        for (stat = 0; stat < 2; ++stat) {
            snprintf(label, sizeof(label),
                     "slot%d hand%d slot box zone byte-stable across slot1 mutation",
                     slot, stat);
            ok &= expect_true(label,
                              M11_GameView_GetV1StatusHandSlotBoxZone(slot, stat,
                                                                      &handX, &handY,
                                                                      &handW, &handH) &&
                              zone_byte_equal(fbA1, fbB, PROBE_FB_W, handX, handY, handW, handH));
        }

        snprintf(label, sizeof(label),
                 "slot%d champion icon zone byte-stable across slot1 mutation",
                 slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1ChampionIconZone(slot,
                                                             &iconX, &iconY,
                                                             &iconW, &iconH) &&
                          zone_byte_equal(fbA1, fbB, PROBE_FB_W, iconX, iconY, iconW, iconH));

        snprintf(label, sizeof(label),
                 "slot%d action icon cell zone byte-stable across slot1 mutation",
                 slot);
        ok &= expect_true(label,
                          M11_GameView_GetV1ActionIconCellZone(slot,
                                                               &cellX, &cellY,
                                                               &cellW, &cellH) &&
                          zone_byte_equal(fbA1, fbB, PROBE_FB_W, cellX, cellY, cellW, cellH));
    }

    /* Slot 1's status box zone must NOT be byte-stable: the mutation
     * reached slot 1's bars and the surrounding 67x29 status box. */
    {
        int x, y, w, h;
        int slot1Stable;
        char label[128];
        ok &= expect_true("slot1 status box zone lookup",
                          M11_GameView_GetV1StatusBoxZone(1, &x, &y, &w, &h));
        slot1Stable = zone_byte_equal(fbA1, fbB, PROBE_FB_W, x, y, w, h);
        snprintf(label, sizeof(label),
                 "slot1 status box zone changed across slot1 mutation (stable=%d)",
                 slot1Stable);
        ok &= expect_true(label, !slot1Stable);
    }

    /* After mutation, all four slots still have status box fill pixels
     * (the four-champion layout did not collapse). */
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        int x, y, w, h;
        int fillsCount;
        char label[128];
        snprintf(label, sizeof(label),
                 "frame B slot%d status box still filled (four-champion layout preserved)",
                 slot);
        fillsCount = M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h)
            ? count_color(fbB, PROBE_FB_W, x, y, w, h, PROBE_FILL_COLOR)
            : -1;
        ok &= expect_true(label, fillsCount > 0);
    }

    /* Status box gutter (between adjacent status boxes) is byte-stable
     * across the mutation.  The gutter is two black pixels wide on the
     * V1 status box stride (69 px). */
    for (slot = 0; slot < PROBE_CHAMPION_COUNT - 1; ++slot) {
        int x, y, w, h;
        int nextX, nextY, nextW, nextH;
        int gutterX, gutterY, gutterW, gutterH;
        char label[128];
        ok &= expect_true("gutter slot zone lookup",
                          M11_GameView_GetV1StatusBoxZone(slot, &x, &y, &w, &h) &&
                          w == 67 && h == 29);
        ok &= expect_true("gutter next slot zone lookup",
                          M11_GameView_GetV1StatusBoxZone(slot + 1,
                                                          &nextX, &nextY,
                                                          &nextW, &nextH) &&
                          nextY == y && nextW == 67 && nextH == 29);
        gutterX = x + w;
        gutterY = y;
        gutterW = nextX - gutterX;
        gutterH = h;
        snprintf(label, sizeof(label),
                 "slot%d status box gutter width = 2 (V1 stride 69 - status box 67)",
                 slot);
        ok &= expect_true(label, gutterW == 2);
        snprintf(label, sizeof(label),
                 "slot%d status box gutter byte-stable across slot1 mutation",
                 slot);
        ok &= expect_true(label,
                          zone_byte_equal(fbA1, fbB, PROBE_FB_W,
                                          gutterX, gutterY, gutterW, gutterH));
    }

    /* Leader name color cascade is preserved after the mutation. */
    ok &= expect_int("frame B slot0 leader name color C11",
                     M11_GameView_GetV1StatusNameColor(&game, 0),
                     PROBE_LEADER_NAME_COLOR);
    ok &= expect_int("frame B slot1 non-leader name color C09",
                     M11_GameView_GetV1StatusNameColor(&game, 1),
                     PROBE_NONLEADER_NAME_COLOR);
    ok &= expect_int("frame B slot2 non-leader name color C09",
                     M11_GameView_GetV1StatusNameColor(&game, 2),
                     PROBE_NONLEADER_NAME_COLOR);
    ok &= expect_int("frame B slot3 non-leader name color C09",
                     M11_GameView_GetV1StatusNameColor(&game, 3),
                     PROBE_NONLEADER_NAME_COLOR);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel single-champion state isolation "
           "runtime pixel probe (Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
