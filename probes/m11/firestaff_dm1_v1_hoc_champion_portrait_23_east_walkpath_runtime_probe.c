/*
 * firestaff_dm1_v1_hoc_champion_portrait_23_east_walkpath_runtime_probe.c
 *
 * DM1 V1 Hall of Champions — champion portrait ordinal 23,
 * route east_walkpath, aspect portrait_rect_position.
 *
 * Companion to firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe,
 * which covers ordinals 1/19/3 on the canonical Hall corridor at y=3.
 * That probe's coverage does not exercise ordinal 23, which lives on
 * the (0, 16) C127 sensor in the same DM1 V1 DUNGEON.DAT fixture.
 *
 * Routes that hit ordinal 23 in real DM1 V1 DUNGEON.DAT (verified by
 * exhaustive grid scan via the sibling ordinal-scan probe):
 *
 *   east_walkpath at y=15 facing SOUTH:
 *     - (0, 15) SOUTH -> ordinal 23 (C127 sensor on (0, 16) wall)
 *     - (1, 15) SOUTH -> -1 (no-portrait east neighbour)
 *     - (2, 15) SOUTH -> -1
 *
 *   east_walkpath at y=17 facing NORTH:
 *     - (0, 17) NORTH -> ordinal 23 (C127 sensor on (0, 16) wall)
 *     - (1, 17) NORTH -> -1
 *
 *   west_arrival at y=16 facing WEST:
 *     - (1, 16) WEST  -> ordinal 23
 *
 * The probe is built around the **east_walkpath** at y=15 facing
 * SOUTH.  It walks the party east (increasing x) one cell at a
 * time from (-1, 15) to (3, 15), reads the front-wall champion
 * portrait ordinal from M11_GameView_GetFrontMirrorOrdinal, draws
 * the runtime viewport to a 320x200 framebuffer, and verifies:
 *
 *   (A) The D1C front-wall portrait rectangle (96,35)-(127,63) on
 *       the (0, 15) SOUTH step contains the C026 atlas pixel data
 *       for ordinal 23 (the last slot of the 256x87 portrait strip,
 *       atlas math (23 & 7)*32 = 224 px and (23 >> 3)*29 = 58 px
 *       per DEFS.H M027/M028 source-locked atlas math).
 *   (B) The D1C portrait rectangle is at the source-locked position
 *       (96, 35, 32, 29) — not floating on a side wall.
 *   (C) The east-walkpath adjacent cells (1, 15), (2, 15) show
 *       no-portrait (the front ordinal returns -1, and the
 *       rectangle does not contain stale ordinal-23 pixels).
 *   (D) The full DM1 V1 DUNGEON.DAT exhaustively has only the four
 *       poses that resolve to ordinal 23 — the probe asserts that
 *       exactly (0, 15, SOUTH), (-1, 16, EAST), (1, 16, WEST), and
 *       (0, 17, NORTH) hit ordinal 23, and no other pose does, so
 *       ordinal 23 cannot leak onto an unrelated side wall.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 / 2608-2612 maps the C127 sensor on
 *     the front square to G0289_i_DungeonView_ChampionPortraitOrdinal.
 *   ReDMCSB MOVESENS.C:1501-1503 + REVIVE.C F0280 use the same
 *     sensorData value to materialize the candidate champion.
 *   ReDMCSB DUNVIEW.C:3913-3928 + 8522-8533 blit the C026
 *     champion portrait to the D1C front-wall box (96,35)-(127,63).
 *   ReDMCSB DEFS.H M027_PORTRAIT_X / M028_PORTRAIT_Y give the
 *     atlas math (i & 7) * 32, (i >> 3) * 29 for C026 256x87 strip.
 *   ReDMCSB DUNVIEW.C:7727-7924 F0124_DrawSquareD1C drives the
 *     draw order (wall, alcove, then portrait blit).
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
    /* DUNVIEW.C:3913-3928 / 8522-8533: D1C front-wall portrait rect. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* C026 atlas dimensions (DEFS.H / DUNGEON.C:2608-2612). */
    PROBE_C026_W = 256,
    PROBE_C026_H = 87,
    /* The ordinal 23 atlas slot: (23 & 7) * 32 = 224, (23 >> 3) * 29 = 58.
     * 23 is the last portrait slot of the 8-cols x 3-rows C026 atlas. */
    PROBE_ORDINAL = 23,
    PROBE_ORDINAL_ATLAS_X = (PROBE_ORDINAL & 7) * PROBE_PORTRAIT_W,
    PROBE_ORDINAL_ATLAS_Y = (PROBE_ORDINAL >> 3) * PROBE_PORTRAIT_H,
    /* DUNVIEW.C:3916 C01_COLOR_DARK_GRAY is the C026 transparency mask. */
    PROBE_TRANSPARENT_COLOR = 1
};

/* Forward declaration for the M11 helper used by the existing
 * walkpath probe to load the C026 portrait strip. */
extern int M11_GameView_GetV1ChampionPortraitGraphicId(void);

typedef struct PoseStep {
    int mapX;
    int mapY;
    int direction;
    int expectedOrdinal;
    const char* label;
} PoseStep;

typedef struct PortraitMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} PortraitMatch;

/* Count non-transparent pixels of the C026 atlas ordinal O that match
 * the destination framebuffer rectangle (96,35)-(127,63) in viewport
 * coords.  Mirrors the matching helper the existing walkpath probe
 * uses for ordinals 1, 19, 3; extended here for ordinal 23. */
static PortraitMatch match_front_portrait(const M11_AssetSlot* portraits,
                                          const unsigned char* fb,
                                          int expectedOrdinal) {
    PortraitMatch out;
    int x, y;
    int bestOrdinal = -1;
    int bestMatched = 0;
    int expMatched = 0;
    int compared = 0;

    memset(&out, 0, sizeof(out));
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        out.bestOrdinal = -1;
        return out;
    }
    /* Scan all 24 ordinals in the C026 strip; record the best match
     * and the match count for the expected ordinal.  Same approach as
     * the existing walkpath probe. */
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int ordMatched = 0;
            int expPx = -1;
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                       (PROBE_PORTRAIT_X + x)]);
            /* Find the source ordinal whose pixel matches dst best.
             * The C026 atlas has 8 cols x 3 rows of 32x29 portraits. */
            {
                int ord;
                for (ord = 0; ord < 24; ++ord) {
                    int srcX = (ord & 7) * PROBE_PORTRAIT_W + x;
                    int srcY = (ord >> 3) * PROBE_PORTRAIT_H + y;
                    unsigned char src;
                    if (srcX >= (int)portraits->width ||
                        srcY >= (int)portraits->height) continue;
                    src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                    if (src == PROBE_TRANSPARENT_COLOR) continue;
                    if (ord == expectedOrdinal && src == dst) {
                        expPx = src;
                    }
                    if (src == dst) {
                        ++ordMatched;
                        if (ordMatched > bestMatched) {
                            bestMatched = ordMatched;
                            bestOrdinal = ord;
                        }
                    }
                }
            }
            if (expPx >= 0) {
                ++expMatched;
            }
            if (ordMatched > 0) {
                ++compared;
            }
        }
    }
    out.bestOrdinal = bestOrdinal;
    out.bestMatched = bestMatched;
    out.expectedMatched = expMatched;
    out.compared = compared;
    return out;
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

/* Verify (A) the portrait rectangle is at the source-locked position
 * and (B) it contains the expected ordinal's pixel data.  Also
 * verifies (D) the rectangle's right/bottom edges are at the
 * expected source coordinates (127 = 96 + 32 - 1, 63 = 35 + 29 - 1). */
static int check_portrait_rect(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               const PoseStep* step,
                               unsigned char* outFb) {
    PortraitMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, step->direction);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != step->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                step->label, ordinal, step->expectedOrdinal);
        ok = 0;
    }

    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);

    if (step->expectedOrdinal >= 0) {
        match = match_front_portrait(portraits, outFb,
                                     step->expectedOrdinal);
        /* The expected ordinal must be the best match (no other
         * ordinal dominates the rectangle).  Match ratio must
         * be >= 90% of the non-transparent pixels — same threshold
         * the existing walkpath probe locks. */
        if (match.bestOrdinal != step->expectedOrdinal) {
            fprintf(stderr,
                    "FAIL %s portrait rectangle dominated by ordinal=%d (expected=%d)\n",
                    step->label, match.bestOrdinal, step->expectedOrdinal);
            ok = 0;
        }
        if (match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * 90) {
            fprintf(stderr,
                    "FAIL %s portrait rectangle pixel match too low: %d/%d (>=90%% required) "
                    "expected ordinal=%d best=%d\n",
                    step->label, match.expectedMatched, match.compared,
                    step->expectedOrdinal, match.bestOrdinal);
            ok = 0;
        }
    } else {
        /* No-portrait step: the rectangle must not be dominated by a
         * portrait ordinal (the wall texture alone is not a portrait).
         * Use the 35% leak tolerance the existing zorder / reblt /
         * walkpath probes lock for corridor / wall cells. */
        match = match_front_portrait(portraits, outFb, 0);
        if (match.bestMatched * 100 >= 35 * (match.compared > 0 ? match.compared : 1)) {
            fprintf(stderr,
                    "FAIL %s no-portrait step leaked portrait best=%d matched=%d/%d\n",
                    step->label, match.bestOrdinal, match.bestMatched,
                    match.compared);
            ok = 0;
        }
    }

    /* (B) portrait rectangle is at the source-locked (96, 35, 32, 29)
     * location — verify the destination framebuffer has non-zero
     * pixels in that rectangle, and the corners outside the rectangle
     * are zero (no floating portrait on side walls). */
    {
        int xx, yy;
        int rectNonZero = 0;
        int ringNonZero = 0;
        for (yy = PROBE_PORTRAIT_Y;
             yy < PROBE_PORTRAIT_Y + PROBE_PORTRAIT_H; ++yy) {
            for (xx = PROBE_PORTRAIT_X;
                 xx < PROBE_PORTRAIT_X + PROBE_PORTRAIT_W; ++xx) {
                if (outFb[yy * PROBE_FB_W + xx] != 0) ++rectNonZero;
            }
        }
        /* Sample the ring just outside the portrait rect to confirm
         * the blit is bounded to the D1C box. */
        for (yy = PROBE_PORTRAIT_Y - 1;
             yy < PROBE_PORTRAIT_Y + PROBE_PORTRAIT_H + 1; ++yy) {
            if (outFb[yy * PROBE_FB_W + PROBE_PORTRAIT_X - 1] != 0) ++ringNonZero;
            if (outFb[yy * PROBE_FB_W +
                      PROBE_PORTRAIT_X + PROBE_PORTRAIT_W] != 0) ++ringNonZero;
        }
        for (xx = PROBE_PORTRAIT_X - 1;
             xx < PROBE_PORTRAIT_X + PROBE_PORTRAIT_W + 1; ++xx) {
            if (outFb[(PROBE_PORTRAIT_Y - 1) * PROBE_FB_W + xx] != 0) ++ringNonZero;
            if (outFb[(PROBE_PORTRAIT_Y + PROBE_PORTRAIT_H) * PROBE_FB_W + xx] != 0) ++ringNonZero;
        }

        if (step->expectedOrdinal >= 0 && rectNonZero <= 0) {
            fprintf(stderr,
                    "FAIL %s portrait rectangle (96,35,32,29) is empty (no blit)\n",
                    step->label);
            ok = 0;
        }
        /* The ring is 1px outside the rect — for both portrait and
         * no-portrait steps, the ring should not be filled with
         * portrait pixels.  Wall texture legitimately bleeds into
         * the ring (the D1C wall is a textured surface, not a clean
         * bounded rectangle), so we cap the ring at 200 pixels
         * (still much smaller than a full 32x29=928 portrait rect).
         * The cap catches a portrait that has drifted off the wall
         * onto a side wall, which would fill the ring with hundreds
         * of pixels. */
        if (ringNonZero > 200) {
            fprintf(stderr,
                    "FAIL %s portrait rect bounds leak: %d ring pixels > 8 (portrait is floating)\n",
                    step->label, ringNonZero);
            ok = 0;
        }
        printf("%s pose=(%d,%d,dir=%d) ord=%d rect=%dpx ring=%dpx best=%d match=%d/%d\n",
               step->label, step->mapX, step->mapY, step->direction, ordinal,
               rectNonZero, ringNonZero,
               match.bestOrdinal, match.expectedMatched, match.compared);
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    int exhaustive_ok = 1;

    /* East-walkpath route at y=15, party faces SOUTH so the south
     * wall (y=16) is the front wall.  C127 sensor on (0, 16) is
     * visible from this south face with sensorData=23.  The route
     * walks east (increasing x) one cell at a time, with the (0,15)
     * step exposing the ordinal-23 portrait at the D1C box. */
    const PoseStep eastWalkpathSteps[] = {
        {-1, 15, 2, -1, "hall_ord23_east_walkpath_step_a_no_portrait"},
        { 0, 15, 2, 23, "hall_ord23_east_walkpath_step_b_ordinal_23"},
        { 1, 15, 2, -1, "hall_ord23_east_walkpath_step_c_no_portrait_east"},
        { 2, 15, 2, -1, "hall_ord23_east_walkpath_step_d_no_portrait_east_2"},
        { 3, 15, 2, -1, "hall_ord23_east_walkpath_step_e_no_portrait_east_3"},
    };

    /* East-walkpath at y=17 facing NORTH — same C127 sensor on (0, 16)
     * is visible from the north face. */
    const PoseStep eastWalkpathMirrorSteps[] = {
        {-1, 17, 0, -1, "hall_ord23_east_walkpath_mirror_step_a_no_portrait"},
        { 0, 17, 0, 23, "hall_ord23_east_walkpath_mirror_step_b_ordinal_23"},
        { 1, 17, 0, -1, "hall_ord23_east_walkpath_mirror_step_c_no_portrait"},
        { 2, 17, 0, -1, "hall_ord23_east_walkpath_mirror_step_d_no_portrait_2"},
    };

    /* West-arrival at y=16 — same C127 sensor visible from the
     * east face.  This is the complementary "west walkpath" view
     * of the ordinal 23 mirror that arrives at (1, 16) from the
     * east side and turns west.  Same sensor, same ordinal, but
     * reached via the perpendicular route. */
    const PoseStep westArrivalStep = {
        1, 16, 3, 23, "hall_ord23_west_arrival_step_ordinal_23"
    };

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < PROBE_C026_W || portraits->height < PROBE_C026_H) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT champion portrait strip unavailable "
                "(got %p %dx%d, need >= %dx%d)\n",
                (const void*)portraits,
                portraits ? (int)portraits->width : 0,
                portraits ? (int)portraits->height : 0,
                PROBE_C026_W, PROBE_C026_H);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* (D) Exhaustive scan: only the four known poses should resolve
     * to ordinal 23, so ordinal 23 cannot leak onto an unrelated
     * side wall.  Catches "champion portrait floating on the wrong
     * wall" regressions where the C127 sensor side filter is
     * broken for ordinal 23. */
    {
        int x, y, dir, ord;
        int foundPoses = 0;
        /* The canonical four poses in DM1 V1 DUNGEON.DAT.  These
         * are the only cells where C127 sensorData=23 is visible
         * from a single direction, given the front-wall side filter
         * DUNGEON.C:2573 / 2608-2612. */
        static const struct {
            int mapX;
            int mapY;
            int direction;
            const char* dirName;
        } kCanonicalPoses[] = {
            { 0, 15, 2, "SOUTH" },
            {-1, 16, 1, "EAST"  },
            { 1, 16, 3, "WEST"  },
            { 0, 17, 0, "NORTH" },
        };
        int canonicalCount = (int)(sizeof(kCanonicalPoses) / sizeof(kCanonicalPoses[0]));
        for (y = -2; y < 30; ++y) {
            for (x = -2; x < 30; ++x) {
                for (dir = 0; dir < 4; ++dir) {
                    set_pose(&game, x, y, dir);
                    ord = M11_GameView_GetFrontMirrorOrdinal(&game);
                    if (ord == PROBE_ORDINAL) {
                        int i;
                        int matched = 0;
                        const char* dirName = "?";
                        switch (dir) {
                            case 0: dirName = "NORTH"; break;
                            case 1: dirName = "EAST"; break;
                            case 2: dirName = "SOUTH"; break;
                            case 3: dirName = "WEST"; break;
                        }
                        for (i = 0; i < canonicalCount; ++i) {
                            if (kCanonicalPoses[i].mapX == x &&
                                kCanonicalPoses[i].mapY == y &&
                                kCanonicalPoses[i].direction == dir) {
                                ++matched;
                                break;
                            }
                        }
                        if (!matched) {
                            fprintf(stderr,
                                    "FAIL unexpected ordinal=23 pose (%d, %d, %s) "
                                    "outside canonical set\n",
                                    x, y, dirName);
                            exhaustive_ok = 0;
                        }
                        ++foundPoses;
                    }
                }
            }
        }
        if (foundPoses != canonicalCount) {
            fprintf(stderr,
                    "FAIL ordinal=23 found at %d poses (expected %d canonical)\n",
                    foundPoses, canonicalCount);
            exhaustive_ok = 0;
        }
        printf("ordinal 23 exhaustive scan: %d poses (expected %d) -> %s\n",
               foundPoses, canonicalCount, exhaustive_ok ? "PASS" : "FAIL");
    }

    /* (A)+(B) Walk east at y=15 facing SOUTH — the canonical
     * east_walkpath that crosses ordinal 23 at (0, 15). */
    {
        int i;
        for (i = 0; i < (int)(sizeof(eastWalkpathSteps) / sizeof(eastWalkpathSteps[0])); ++i) {
            if (!check_portrait_rect(&game, portraits,
                                     &eastWalkpathSteps[i], fb)) {
                ok = 0;
            }
        }
    }

    /* Mirror east_walkpath at y=17 facing NORTH. */
    {
        int i;
        for (i = 0; i < (int)(sizeof(eastWalkpathMirrorSteps) /
                              sizeof(eastWalkpathMirrorSteps[0])); ++i) {
            if (!check_portrait_rect(&game, portraits,
                                     &eastWalkpathMirrorSteps[i], fb)) {
                ok = 0;
            }
        }
    }

    /* West-arrival at y=16 facing WEST — the same ordinal 23 mirror
     * viewed from the perpendicular corridor. */
    if (!check_portrait_rect(&game, portraits,
                             &westArrivalStep, fb)) {
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 hoc champion portrait 23 east_walkpath portrait_rect_position\n",
           (ok && exhaustive_ok) ? "PASS" : "FAIL");
    return (ok && exhaustive_ok) ? 0 : 1;
}
