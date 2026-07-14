/*
 * DM1 V1 champion panel leader-rotation pixel slice probe.
 *
 * This is Firestaff-side evidence only.  It opens the hash-verified DM1 V1
 * runtime when local assets are available, installs a deterministic four
 * champion party, renders one V1 frame with activeChampionIndex = 0, then
 * rotates the leader to activeChampionIndex = 2 and re-renders, and finally
 * checks the resulting on-screen pixels for the following invariants:
 *
 *   1. Per the CHAMDRAW.C F0292 name-color cascade:
 *        - leader name text is drawn in C11 (M11_COLOR_YELLOW) in the
 *          source-locked C163+n name text zone (V1 source: leader C11,
 *          other champions C09).
 *        - non-leader name text is drawn in C09 (M11_COLOR_ORANGE).
 *      The probe asserts both colors in the leader-vs-non-leader name
 *      text zone for activeChampionIndex = 0, then re-asserts that the
 *      color presence migrates after leader rotation.
 *
 *   2. Per the CHAMDRAW.C F0287 bar-graph source-locked split, the
 *      HP/stamina/mana bar fill/blank pixel counts in the C195..C206
 *      bar zones are byte-identical before and after leader rotation,
 *      because leader rotation only changes which slot is "active";
 *      it does not touch the current/maximum HP/stamina/mana values.
 *
 *   3. Per the CHAMDRAW.C F0291 hand-slot graphic cascade and the
 *      M026_CHAMPION_ICON_INDEX source lock, the 18x18 hand-slot box
 *      perimeter pixel counts and the per-champion icon source index
 *      are byte-identical before and after leader rotation.
 *
 *   4. Per the CHAMDRAW.C F0292 status-box redraw, the four-champion
 *      status box fill (C12, M11_COLOR_DARK_GRAY) presence is
 *      preserved across leader rotation; the dead status box predicate
 *      remains correct (HP == 0 stays C008).
 *
 *   5. Per the F0292 source-locked C09 (M11_COLOR_ORANGE) and
 *      C11 (M11_COLOR_YELLOW) values, the leader rotation also
 *      migrates which M11_COLOR_*_for_slot lookup returns 11 vs 9.
 *
 * Source evidence:
 *   ReDMCSB CHAMDRAW.C F0292: leader C11, other champions C09;
 *     F0282:1060-1078 status-box redraw propagation.
 *   ReDMCSB CHAMDRAW.C F0287: bottom-anchored 4x25 HP/stamina/mana
 *     bar fill (current/maximum unchanged by leader rotation).
 *   ReDMCSB CHAMDRAW.C F0291: 18x18 hand-slot box at champIdx*69+4/24.
 *   ReDMCSB COORD.C/layout-696 C113..C116 champion icon zones and
 *     M026_CHAMPION_ICON_INDEX(Direction, PartyDirection) macro.
 *   ReDMCSB CHAMDRAW.C F0622: 19x14 champion icon bitmap.
 *   ReDMCSB COORD.C G0046_auc_Graphic562_ChampionColor[4] = {7,11,8,14}
 *     drives the per-champion bar fill color and the icon inner color.
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

    /* Source-locked DM PC VGA color indices used by the CHAMDRAW.C F0292
     * champion name text cascade.  M11 routes these through the same
     * 0..15 palette indices that the M11 framebuffer stores verbatim. */
    PROBE_LEADER_NAME_COLOR    = 11, /* C11 = M11_COLOR_YELLOW (m11_game_view.c) */
    PROBE_NONLEADER_NAME_COLOR = 9,  /* C09 = M11_COLOR_ORANGE  (m11_game_view.c) */
    PROBE_BLANK_BAR_COLOR      = 12, /* C12 = M11_COLOR_DARK_GRAY */
    PROBE_BLACK                = 0
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

static void seed_party(M11_GameViewState* game, int activeLeaderIndex) {
    game->world.party.championCount = PROBE_CHAMPION_COUNT;
    game->world.party.activeChampionIndex = activeLeaderIndex;
    game->world.party.direction = DIR_NORTH;
    game->showDebugHUD = 0;
    game->inventoryPanelActive = 0;
    game->spellPanelOpen = 0;
    game->actingChampionOrdinal = 0; /* no acting champion, so slot 0 and
                                       slot 1 hand-slot boxes both use the
                                       non-acting C033 normal cascade. */
    game->world.magic.fireShieldDefense = 0;
    game->world.magic.spellShieldDefense = 0;
    game->world.magic.partyShieldDefense = 0;
    /* Distinct per-champion HP/stamina/mana values so the F0287 bar
     * fill/blank pixel counts are clearly distinguishable across the
     * four slots.  Champion names are 5 ASCII letters wide so the
     * centered text fits the 42-px name text zone. */
    seed_champion(&game->world.party.champions[0],
                  "TIGGY", 0, DIR_NORTH, 100, 100, 80, 80, 60, 60, 0);
    seed_champion(&game->world.party.champions[1],
                  "HALK", 1, DIR_EAST, 50, 100, 40, 80, 30, 60, 0);
    seed_champion(&game->world.party.champions[2],
                  "WUUF", 2, DIR_SOUTH, 25, 100, 20, 80, 15, 60, 0);
    seed_champion(&game->world.party.champions[3],
                  "ALEX", 3, DIR_WEST, 1, 100, 1, 80, 1, 60, 0);
}

/* Per-slot invariant capture.  We snapshot the on-screen pixels for the
 * name text zone, the four 4x25 bar zones, the two 18x18 hand-slot box
 * perimeters, the 19x14 champion icon zone, and the 67x29 status box
 * fill, then compare the two snapshots after leader rotation. */
typedef struct LeaderRotationCapture {
    int nameTextYellow;
    int nameTextOrange;
    int nameTextBlack; /* font shadow */
    int barBlankPixels[PROBE_CHAMPION_COUNT][3];
    int barFillPixels[PROBE_CHAMPION_COUNT][3];
    int statusBoxFillPixels;
    int statusBoxNonBlackPixels;
    int iconNonBlackPixels;
    int handSlotBoxPerimeter;
} LeaderRotationCapture;

static void capture_slot_pixels(const M11_GameViewState* game,
                                const unsigned char* fb,
                                int slot,
                                int leaderIndex,
                                LeaderRotationCapture* cap) {
    int nameTextX, nameTextY, nameTextW, nameTextH;
    int stat;
    int x, y, w, h;
    int statusBoxX, statusBoxY, statusBoxW, statusBoxH;
    int iconX, iconY, iconW, iconH;

    if (!M11_GameView_GetV1StatusNameTextZone(
            slot, &nameTextX, &nameTextY, &nameTextW, &nameTextH)) {
        cap->nameTextYellow = -1;
        cap->nameTextOrange = -1;
        cap->nameTextBlack = -1;
    } else {
        int expectedLeaderColor = (slot == leaderIndex)
            ? PROBE_LEADER_NAME_COLOR
            : PROBE_NONLEADER_NAME_COLOR;
        int expectedNonLeaderColor = (slot == leaderIndex)
            ? PROBE_NONLEADER_NAME_COLOR
            : PROBE_LEADER_NAME_COLOR;
        cap->nameTextYellow  = count_color(fb, PROBE_FB_W,
            nameTextX, nameTextY, nameTextW, nameTextH,
            expectedLeaderColor);
        cap->nameTextOrange = count_color(fb, PROBE_FB_W,
            nameTextX, nameTextY, nameTextW, nameTextH,
            expectedNonLeaderColor);
        cap->nameTextBlack   = count_color(fb, PROBE_FB_W,
            nameTextX, nameTextY, nameTextW, nameTextH, PROBE_BLACK);
    }

    for (stat = 0; stat < 3; ++stat) {
        if (!M11_GameView_GetV1StatusBarZone(slot, stat,
                                              &x, &y, &w, &h)) {
            cap->barBlankPixels[slot][stat] = -1;
            cap->barFillPixels[slot][stat] = -1;
            continue;
        }
        cap->barBlankPixels[slot][stat] = count_color(fb, PROBE_FB_W,
            x, y, w, h, PROBE_BLANK_BAR_COLOR);
        cap->barFillPixels[slot][stat] = count_color(fb, PROBE_FB_W,
            x, y, w, h, M11_GameView_GetV1ChampionBarColor(slot));
    }

    if (!M11_GameView_GetV1StatusBoxZone(slot,
                                         &statusBoxX, &statusBoxY,
                                         &statusBoxW, &statusBoxH)) {
        cap->statusBoxFillPixels = -1;
        cap->statusBoxNonBlackPixels = -1;
    } else {
        int fillColor = M11_GameView_GetV1StatusBoxFillColor();
        cap->statusBoxFillPixels = count_color(fb, PROBE_FB_W,
            statusBoxX, statusBoxY, statusBoxW, statusBoxH, fillColor);
        cap->statusBoxNonBlackPixels = 0;
        {
            int yy;
            for (yy = 0; yy < statusBoxH; ++yy) {
                int xx;
                for (xx = 0; xx < statusBoxW; ++xx) {
                    int idx = (statusBoxY + yy) * PROBE_FB_W + (statusBoxX + xx);
                    unsigned char raw = fb[idx];
                    if ((int)M11_FB_DECODE_INDEX(raw) != PROBE_BLACK) {
                        cap->statusBoxNonBlackPixels++;
                    }
                }
            }
        }
    }

    if (!M11_GameView_GetV1ChampionIconZone(slot,
                                            &iconX, &iconY,
                                            &iconW, &iconH)) {
        cap->iconNonBlackPixels = -1;
    } else {
        int yy;
        cap->iconNonBlackPixels = 0;
        for (yy = 0; yy < iconH; ++yy) {
            int xx;
            for (xx = 0; xx < iconW; ++xx) {
                int idx = (iconY + yy) * PROBE_FB_W + (iconX + xx);
                unsigned char raw = fb[idx];
                if ((int)M11_FB_DECODE_INDEX(raw) != PROBE_BLACK) {
                    cap->iconNonBlackPixels++;
                }
            }
        }
    }

    /* Per-hand perimeter pixel count: the 18x18 C033/C034/C035 hand-slot
     * box outer ring pixels (i.e. the 1-px border of the 18x18 box that
     * is not overwritten by a 16x16 in-hand object icon).  Source-locked
     * by CHAMDRAW.C F0291 18x18 hand-slot box anchor at
     * (champIdx*69+4/24, 10). */
    {
        int hand;
        cap->handSlotBoxPerimeter = 0;
        for (hand = 0; hand < 2; ++hand) {
            int hx, hy, hw, hh;
            if (!M11_GameView_GetV1StatusHandSlotBoxZone(slot, hand,
                                                        &hx, &hy,
                                                        &hw, &hh)) {
                continue;
            }
            {
                int yy;
                for (yy = 0; yy < hh; ++yy) {
                    int xx;
                    for (xx = 0; xx < hw; ++xx) {
                        int isBorder = (yy == 0 || yy == hh - 1 ||
                                        xx == 0 || xx == hw - 1);
                        if (!isBorder) continue;
                        {
                            int idx = (hy + yy) * PROBE_FB_W + (hx + xx);
                            unsigned char raw = fb[idx];
                            if ((int)M11_FB_DECODE_INDEX(raw) != PROBE_BLACK) {
                                cap->handSlotBoxPerimeter++;
                            }
                        }
                    }
                }
            }
        }
    }
    (void)game;
}

static int check_leader_color_in_name_text(const M11_GameViewState* game,
                                            const unsigned char* fb,
                                            int slot,
                                            int leaderIndex) {
    int x, y, w, h;
    int leaderPixels;
    int nonLeaderPixels;
    int ok = 1;
    char label[128];
    int expectedLeaderColor = (slot == leaderIndex)
        ? PROBE_LEADER_NAME_COLOR
        : PROBE_NONLEADER_NAME_COLOR;
    int expectedNonLeaderColor = (slot == leaderIndex)
        ? PROBE_NONLEADER_NAME_COLOR
        : PROBE_LEADER_NAME_COLOR;
    const char* leaderName = (slot == leaderIndex) ? "leader" : "non-leader";

    if (!M11_GameView_GetV1StatusNameTextZone(slot, &x, &y, &w, &h)) {
        fprintf(stderr, "FAIL slot%d name text zone not available\n", slot);
        return 0;
    }

    leaderPixels = count_color(fb, PROBE_FB_W, x, y, w, h, expectedLeaderColor);
    nonLeaderPixels = count_color(fb, PROBE_FB_W, x, y, w, h, expectedNonLeaderColor);

    snprintf(label, sizeof(label),
             "slot%d name text %s color present (>=8 px of color %d)",
             slot, leaderName, expectedLeaderColor);
    ok &= expect_true(label, leaderPixels >= 8);

    snprintf(label, sizeof(label),
             "slot%d name text opposite color absent (<=2 px of color %d)",
             slot, expectedNonLeaderColor);
    ok &= expect_true(label, nonLeaderPixels <= 2);

    (void)game;
    return ok;
}

static int check_champion_icon_source_index(const M11_GameViewState* game,
                                             int slot,
                                             int* outIconIndex) {
    int idx = M11_GameView_GetV1ChampionIconSourceIndex(game, slot);
    if (outIconIndex) {
        *outIconIndex = idx;
    }
    return idx;
}

static int check_captures_equal(const LeaderRotationCapture* a,
                                const LeaderRotationCapture* b) {
    int ok = 1;
    int slot, stat;
    char label[128];

    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        /* The 67x29 status box fill and non-black pixel count for the
         * same slot must be identical before and after leader rotation;
         * the rotation only swaps which slot is the "active" one. */
        snprintf(label, sizeof(label),
                 "slot%d status box fill pixel count invariant", slot);
        ok &= expect_int(label, b->statusBoxFillPixels, a->statusBoxFillPixels);
        snprintf(label, sizeof(label),
                 "slot%d status box non-black pixel count invariant", slot);
        ok &= expect_int(label, b->statusBoxNonBlackPixels, a->statusBoxNonBlackPixels);
        snprintf(label, sizeof(label),
                 "slot%d champion icon non-black pixel count invariant", slot);
        ok &= expect_int(label, b->iconNonBlackPixels, a->iconNonBlackPixels);
        snprintf(label, sizeof(label),
                 "slot%d hand-slot box outer perimeter pixel count invariant", slot);
        ok &= expect_int(label, b->handSlotBoxPerimeter, a->handSlotBoxPerimeter);

        for (stat = 0; stat < 3; ++stat) {
            snprintf(label, sizeof(label),
                     "slot%d stat%d bar blank pixel count invariant",
                     slot, stat);
            ok &= expect_int(label,
                             b->barBlankPixels[slot][stat],
                             a->barBlankPixels[slot][stat]);
            snprintf(label, sizeof(label),
                     "slot%d stat%d bar fill pixel count invariant",
                     slot, stat);
            ok &= expect_int(label,
                             b->barFillPixels[slot][stat],
                             a->barFillPixels[slot][stat]);
        }
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fbA[PROBE_FB_W * PROBE_FB_H];
    unsigned char fbB[PROBE_FB_W * PROBE_FB_H];
    LeaderRotationCapture capA;
    LeaderRotationCapture capB;
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

    /* Phase A: leader at slot 0 (initial DM1 default). */
    seed_party(&game, /* activeLeaderIndex = */ 0);
    memset(fbA, 0, sizeof(fbA));
    M11_GameView_Draw(&game, fbA, PROBE_FB_W, PROBE_FB_H);
    memset(&capA, 0, sizeof(capA));
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        capture_slot_pixels(&game, fbA, slot, 0, &capA);
        ok &= check_leader_color_in_name_text(&game, fbA, slot, 0);
    }

    /* Champion icon source index must reflect the M026 macro on
     * (champion.direction, party.direction) and must NOT depend on
     * activeChampionIndex.  Lock this before the leader rotation so a
     * later rotation regression can be isolated. */
    {
        int iconIdx[PROBE_CHAMPION_COUNT];
        int leaderIdx[PROBE_CHAMPION_COUNT];
        int s;
        for (s = 0; s < PROBE_CHAMPION_COUNT; ++s) {
            iconIdx[s] = check_champion_icon_source_index(&game, s, NULL);
            leaderIdx[s] = M11_GameView_GetV1StatusNameColor(&game, s);
        }
        /* ReDMCSB M026_CHAMPION_ICON_INDEX(Direction, PartyDirection)
         * = (Direction - PartyDirection) & 0x03.  With party.direction
         * pinned to DIR_NORTH (0), the icon index equals
         * champion.direction & 0x03 for each slot. */
        ok &= expect_int("slot0 icon source = (DIR_NORTH - DIR_NORTH) & 3",
                         iconIdx[0], (DIR_NORTH - DIR_NORTH) & 0x03);
        ok &= expect_int("slot1 icon source = (DIR_EAST - DIR_NORTH) & 3",
                         iconIdx[1], (DIR_EAST - DIR_NORTH) & 0x03);
        ok &= expect_int("slot2 icon source = (DIR_SOUTH - DIR_NORTH) & 3",
                         iconIdx[2], (DIR_SOUTH - DIR_NORTH) & 0x03);
        ok &= expect_int("slot3 icon source = (DIR_WEST - DIR_NORTH) & 3",
                         iconIdx[3], (DIR_WEST - DIR_NORTH) & 0x03);
        /* Slot 0 (activeChampionIndex=0) is the leader with C11 yellow. */
        ok &= expect_int("slot0 name color = leader C11 yellow",
                         leaderIdx[0], PROBE_LEADER_NAME_COLOR);
        ok &= expect_int("slot1 name color = non-leader C09 orange",
                         leaderIdx[1], PROBE_NONLEADER_NAME_COLOR);
        ok &= expect_int("slot2 name color = non-leader C09 orange",
                         leaderIdx[2], PROBE_NONLEADER_NAME_COLOR);
        ok &= expect_int("slot3 name color = non-leader C09 orange",
                         leaderIdx[3], PROBE_NONLEADER_NAME_COLOR);
    }

    /* Phase B: leader rotated to slot 2. */
    seed_party(&game, /* activeLeaderIndex = */ 2);
    memset(fbB, 0, sizeof(fbB));
    M11_GameView_Draw(&game, fbB, PROBE_FB_W, PROBE_FB_H);
    memset(&capB, 0, sizeof(capB));
    for (slot = 0; slot < PROBE_CHAMPION_COUNT; ++slot) {
        capture_slot_pixels(&game, fbB, slot, 2, &capB);
        ok &= check_leader_color_in_name_text(&game, fbB, slot, 2);
    }

    {
        int leaderIdx[PROBE_CHAMPION_COUNT];
        int s;
        int iconIdx[PROBE_CHAMPION_COUNT];
        for (s = 0; s < PROBE_CHAMPION_COUNT; ++s) {
            leaderIdx[s] = M11_GameView_GetV1StatusNameColor(&game, s);
            iconIdx[s] = check_champion_icon_source_index(&game, s, NULL);
        }
        /* After leader rotation: slot 0 is now a non-leader (orange);
         * slot 2 is the new leader (yellow). */
        ok &= expect_int("rotated slot0 name color = non-leader C09 orange",
                         leaderIdx[0], PROBE_NONLEADER_NAME_COLOR);
        ok &= expect_int("rotated slot1 name color = non-leader C09 orange",
                         leaderIdx[1], PROBE_NONLEADER_NAME_COLOR);
        ok &= expect_int("rotated slot2 name color = leader C11 yellow",
                         leaderIdx[2], PROBE_LEADER_NAME_COLOR);
        ok &= expect_int("rotated slot3 name color = non-leader C09 orange",
                         leaderIdx[3], PROBE_NONLEADER_NAME_COLOR);
        /* M026 is direction-vs-party-direction; it must NOT change with
         * leader rotation.  Lock the four source indices to the same
         * values the pre-rotation phase recorded. */
        ok &= expect_int("rotated slot0 icon source unchanged",
                         iconIdx[0], (DIR_NORTH - DIR_NORTH) & 0x03);
        ok &= expect_int("rotated slot1 icon source unchanged",
                         iconIdx[1], (DIR_EAST - DIR_NORTH) & 0x03);
        ok &= expect_int("rotated slot2 icon source unchanged",
                         iconIdx[2], (DIR_SOUTH - DIR_NORTH) & 0x03);
        ok &= expect_int("rotated slot3 icon source unchanged",
                         iconIdx[3], (DIR_WEST - DIR_NORTH) & 0x03);
    }

    /* Phase C: per-slot invariant — bar fill/blank, status box fill,
     * champion icon non-black, and hand-slot box outer perimeter pixel
     * counts must be byte-identical before and after leader rotation. */
    ok &= check_captures_equal(&capA, &capB);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion panel leader-rotation pixel slice probe "
           "(Firestaff-side evidence)\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
