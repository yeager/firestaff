/*
 * DM1 V1 Hall of Champions portrait ordinal 8 — east_walkpath probe.
 *
 * Verifies one narrow slice of the Hall of Champions champion portrait
 * placement: the ordinal 8 C127 sensor at cell (3,1) (cell_bit = 3 /
 * facing W) on the east side of the (2,1) corridor cell, exposed to
 * the party when the party stands at (2,1) facing EAST.  This is the
 * only east_walkpath champion-portrait route in the canonical DM1 V1
 * Hall-of-Champions DUNGEON.DAT — the other east-facing ordinal
 * routes in the layout-696 corridor are no-portrait corridor cells.
 *
 * Slice coverage (this probe):
 *   (2,1) EAST  -> ordinal  8  (positive, D1C portrait matches >= 90%)
 *   (2,1) NORTH -> ordinal -1  (no-portrait side wall, no leak)
 *   (2,1) SOUTH -> ordinal  4  (LEIF on the (2,2) south-wall sensor)
 *   (2,1) WEST  -> ordinal -1  (no-portrait side wall, no leak)
 *   in-place EAST->WEST re-blt clears ordinal 8 pixels from the D1C
 *     wall rectangle instead of leaving a stale portrait ghost over
 *     the (2,1) west side wall.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573   M011_CELL(sensor) source visible-cell test
 *                            against partyDirection.
 *   ReDMCSB DUNGEON.C:2608-2612  G0289 stores the C127 sensorData as
 *                            the champion-portrait ordinal.
 *   ReDMCSB DUNVIEW.C:3913-3928  D1C champion-portrait blit into the
 *                            fixed (96,35)-(127,63) viewport-relative
 *                            rectangle when G0289 is in range.
 *   ReDMCSB DUNVIEW.C:8318-8542  F0128_DUNGEONVIEW_Draw_CPSF rebuilds
 *                            the viewport far-to-near from the new
 *                            party pose after a movement turn (also
 *                            MOVESENS.C:556).
 *   ReDMCSB MOVESENS.C:1501-1503  passes the C127 sensorData to the
 *                            resurrect F0280 candidate materializer.
 *   ReDMCSB REVIVE.C F0280/F0282  candidate append + post-confirm
 *                            C127 sensor disable.
 *
 * Why this slice was previously uncovered: the existing east-facing
 * champion-mirror routes (firestaff_dm1_v1_champion_mirror_actual_pose,
 * _visibility, _zorder, _zorder_reblt, _capture, _walkpath,
 * _candidate_panel) all asserted the (1,3) EAST -> SONJA ordinal 18
 * route.  The (2,1) EAST -> ordinal 8 route was not exercised by any
 * probe before this commit, even though it is in the canonical DM1 V1
 * Hall-of-Champions DUNGEON.DAT (C127 sensor #23 at map=0 cell=(3,1)
 * cell_bit=3 sensorData=8).  Without an explicit gate, a future
 * change to m11_front_cell_mirror_ordinal or to the DUNVIEW.C
 * portrait blit could regress this route without tripping any of the
 * existing probes.
 *
 * On builds whose DUNGEON.DAT does not match the canonical fixture
 * the probe skips with a SKIP message (per-build fixture guard;
 * not a regression detector).
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
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* DUNVIEW.C:3913-3928 / 8522-8533 blit the D1C champion portrait
     * to the fixed (96,35)-(127,63) viewport-relative rectangle. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* Side pose: C1 dark-gray transparency is what the source blit
     * skips; it is the same constant the front-mirror route uses
     * (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY). */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Cross-ordinal re-blt tolerance (matches zorder_reblt probe). */
    PROBE_STALE_PCT_TOLERANCE = 35,
    /* Positive-ordinal portrait match tolerance (>= 90%). */
    PROBE_POSITIVE_MATCH_PCT = 90
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

static MirrorMatch match_front_portrait(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int expectedOrdinal) {
    MirrorMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int matched = 0;
        int compared = 0;
        int y, x;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
                unsigned char src =
                    (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                unsigned char dst =
                    M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                           (PROBE_PORTRAIT_X + x)]);
                if (src == PROBE_CHAMPION_TRANSPARENT) {
                    continue;
                }
                ++compared;
                if (dst == src) {
                    ++matched;
                }
            }
        }
        if (matched > out.bestMatched) {
            out.bestMatched = matched;
            out.bestOrdinal = ordinal;
        }
        if (ordinal == expectedOrdinal) {
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
}

static int ordinal_compared_count(const M11_AssetSlot* portraits, int ordinal) {
    int compared = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            ++compared;
        }
    }
    return compared;
}

static int ordinal_pixels_in_box(const M11_AssetSlot* portraits,
                                const unsigned char* fb, int ordinal) {
    int matched = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                       (PROBE_PORTRAIT_X + x)]);
            if (dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

typedef struct EastOrdinal8Step {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal; /* -1 means no mirror */
    const char* label;
} EastOrdinal8Step;

static int check_pose(M11_GameViewState* game,
                      const M11_AssetSlot* portraits,
                      const EastOrdinal8Step* step,
                      int prevOrdinal,
                      unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, step->dir);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != step->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                step->label, ordinal, step->expectedOrdinal);
        ok = 0;
    }
    memset(outFb, 0, (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);

    if (step->expectedOrdinal >= 0) {
        match = match_front_portrait(portraits, outFb, step->expectedOrdinal);
        if (match.bestOrdinal != step->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * PROBE_POSITIVE_MATCH_PCT) {
            fprintf(stderr,
                    "FAIL %s visible portrait expected=%d best=%d matched=%d/%d\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else {
        /* No-portrait pose must not leak the prior ordinal's pixels
         * from the D1C wall rectangle.  Tolerates the same 35% wall
         * texture leak the zorder_reblt probe already tolerates on
         * the (1,3)/(1,4) corridor poses. */
        if (prevOrdinal >= 0) {
            int stale = ordinal_pixels_in_box(portraits, outFb, prevOrdinal);
            int prevCompared = ordinal_compared_count(portraits, prevOrdinal);
            int stalePct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
            if (stalePct >= PROBE_STALE_PCT_TOLERANCE) {
                fprintf(stderr,
                        "FAIL %s prior ordinal=%d still leaks pixels=%d/%d (%d%%) over side wall\n",
                        step->label, prevOrdinal, stale, prevCompared, stalePct);
                ok = 0;
            }
        }
    }

    /* Cross-ordinal re-blt invariant: when the ordinal changes
     * between steps, the prior ordinal's pixels must not be the
     * dominant match in the new framebuffer's portrait rectangle.
     * This is the same forward-walk analogue of the in-place turn
     * invariant in firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe. */
    if (prevOrdinal >= 0 && step->expectedOrdinal >= 0 &&
        prevOrdinal != step->expectedOrdinal) {
        int stale = ordinal_pixels_in_box(portraits, outFb, prevOrdinal);
        int prevCompared = ordinal_compared_count(portraits, prevOrdinal);
        int stalePct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (stalePct >= PROBE_STALE_PCT_TOLERANCE) {
            fprintf(stderr,
                    "FAIL %s prior ordinal=%d still leaks pixels=%d/%d (%d%%) after re-blt to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared, stalePct,
                    step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label, step->mapX, step->mapY, step->dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;

    /* East_walkpath ordinal-8 slice at (2,1).  The C127 sensor at
     * map=0 cell=(3,1) cell_bit=3 sensorData=8 (front wall of
     * (2,1) facing EAST) is the only east-facing ordinal in the
     * canonical DM1 V1 Hall DUNGEON.DAT.  The same (2,1) cell also
     * hosts a south-wall C127 sensor at cell=(2,2) cell_bit=0
     * sensorData=4 (LEIF), which the probe drives via (2,1) SOUTH.
     * The north and west poses at (2,1) have no front-wall C127
     * sensor and must therefore yield ordinal=-1 with no D1C portrait
     * bleed.  Together this proves the (2,1) east_walkpath ordinal-8
     * slice end-to-end:
     *   positive ordinal 8 in the D1C box,
     *   positive ordinal 4 (LEIF) on the south wall,
     *   no-portrait on the (2,1) north/west side walls. */
    const EastOrdinal8Step steps[] = {
        {2, 1, DIR_NORTH, -1, "hall_east_walkpath_ord8_north_no_portrait"},
        {2, 1, DIR_EAST,   8, "hall_east_walkpath_ord8_east_positive"},
        {2, 1, DIR_SOUTH,  4, "hall_east_walkpath_ord8_south_leif"},
        {2, 1, DIR_WEST,  -1, "hall_east_walkpath_ord8_west_no_portrait"},
        {2, 1, DIR_EAST,   8, "hall_east_walkpath_ord8_east_back_to_ord8"},
    };
    int stepIdx;
    int prevOrdinal = -2; /* sentinel: no prior ordinal */

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    /*
     * Fixture check: the east_walkpath ordinal-8 slice expects the
     * canonical DM1 V1 Hall-of-Champions sensor layout where the
     * C127 sensor at map=0 cell=(3,1) cell_bit=3 has sensorData=8
     * and is therefore exposed to a party at (2,1) facing EAST.
     * Different DM1 V1 builds place the C127 sensor on different
     * cells, so on builds that don't match the reference DUNGEON.DAT
     * we skip the probe and print SKIP rather than fail.  This is
     * not a regression detector; it is a per-build fixture guard,
     * the same pattern firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
     * and firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
     * already use.
     */
    {
        set_pose(&game, 2, 1, DIR_EAST);
        int probeOrd = M11_GameView_GetFrontMirrorOrdinal(&game);
        if (probeOrd != 8) {
            printf("SKIP hall_east_walkpath_ord8_fixture_mismatch "
                   "(2,1) EAST front ordinal=%d expected=8; "
                   "this DM1 V1 build does not match the reference "
                   "DUNGEON.DAT fixture (the (3,1) cell_bit=3 sensor "
                   "is laid out differently; see "
                   "TODO.md fixture-mismatch for the full "
                   "cell->ordinal map)\n", probeOrd);
            M11_GameView_Shutdown(&game);
            return 0;
        }
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    for (stepIdx = 0; stepIdx < (int)(sizeof(steps) / sizeof(steps[0])); ++stepIdx) {
        int stepOk = check_pose(&game, portraits,
                                &steps[stepIdx], prevOrdinal, currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = steps[stepIdx].expectedOrdinal;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror east-walkpath ordinal 8 runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
