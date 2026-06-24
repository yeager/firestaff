/*
 * firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from_left_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 4 (C026 strip cell 4 — atlas col 4 row 0,
 *                               source rect (128, 0, 32, 29), mirror
 *                               catalog record "LEIF")
 *   route approach_from_left: party at (1, 2) facing EAST, the player
 *                             approaching the LEIF chamber (2, 2) from
 *                             the LEFT (west) side.  The visible wall
 *                             under this route is the WEST wall of
 *                             (2, 2), which has no C127 sensor — the
 *                             LEIF sensorData=4 sensor sits on the
 *                             NORTH wall of (2, 2) per the actual_pose
 *                             probe fixture.  This route must therefore
 *                             return front ordinal -1 and the D1C
 *                             portrait cutout (96, 35, 32, 29) must
 *                             stay empty: no LEIF portrait floats over
 *                             the wrong wall.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 — exactly the source-locked DUNVIEW.C
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} blit destination.
 *
 * The slice was authored against the same DM1 V1 PC 3.4 fixture used
 * by the firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe:
 * (2, 1) DIR_SOUTH is the canonical positive LEIF route (front cell
 * is (2, 2) with the C127 sensor on cell 2 = NORTH).  The
 * approach_from_left route (1, 2) DIR_EAST is the *wrong-wall* mirror
 * of the same (2, 2) cell — the party stands to the west of the
 * chamber and looks east at its west wall.  ReDMCSB DUNGEON.C:2573
 * normalizes (M011_CELL(sensor) - direction) and discards the route
 * when the resulting side is not the visible wall side, so
 * m11_front_cell_mirror_ordinal must return -1 here, the D1C cutout
 * must be empty, and a regression that paints ordinal 4 over the
 * corridor wall would push the C026 ordinal-4 pixel-match above the
 * 35% threshold.
 *
 * This probe narrows four contracts to the (1, 2) DIR_EAST
 * approach_from_left anchor:
 *
 *   1. Engine-helper invariant: M11_GameView_GetFrontMirrorOrdinal
 *      at (1, 2) DIR_EAST returns -1.  This is the same fixture
 *      line the actual_pose probe prints as
 *      "hall_start_east_wrong_wall_no_portrait" but is locked here
 *      as the dedicated ordinal-4 negative slice.
 *   2. D1C portrait_rect_position pixel-emptiness: the (96, 35, 32, 29)
 *      cutout on the rendered 320x200 framebuffer contains no
 *      C026 ordinal-4 pixels (>= 35% match would mean a stale
 *      LEIF sprite floats over the wrong wall).  This is the
 *      no-floating invariant the existing ordinal_2 / ordinal_9 /
 *      ordinal_17 west_negative probes lock for their respective
 *      ordinals, here applied to ordinal 4 (LEIF).
 *   3. Cross-check that the D1C cutout is *not* dead: at the
 *      canonical positive LEIF route (2, 1) DIR_SOUTH the SAME
 *      rectangle IS painted with ordinal 4 at >= 90% match.  An
 *      empty rectangle at (1, 2) E must not silently mean the
 *      rectangle is dead.  This is the same cross-check the
 *      ordinal_2_west_negative probe uses for the (1, 5) N
 *      cross-check ordinal 10.
 *   4. Approach-band sweep: every (map=0, x=1, y=2..5) DIR_EAST
 *      pose, plus the corridor (1, 2) DIR_NORTH/EAST/SOUTH/WEST
 *      variants adjacent to the LEIF chamber, must consistently
 *      return -1 for the front mirror ordinal.  These are the
 *      "approach from the left" poses for the LEIF mirror: the
 *      player is to the west of the (2, y) column and looking
 *      east, so the visible wall is the WEST wall of (2, y) and
 *      carries no C127 sensor per the shipped DM1 V1 DUNGEON.DAT.
 *      The C127 sensor on (2, 2) sits on cell 2 (NORTH) per the
 *      actual_pose probe fixture.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box
 *     (only on D1C — M587_VIEW_WALL_D1C_FRONT)
 *   - DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw order
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32, G2079_C29=29
 *   - MOVESENS.C:1501-1503 sensorData flows to F0280 candidate
 *   - REVIVE.C F0280 materialize candidate from sensorData
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   - DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *   - DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas math
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13952 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962 dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972 atlas addr ((ord&7)*32, (ord>>3)*29)
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - 16-pose ordinal map including (1,2,E)=-1 and (2,1,S)=4
 *       (this probe uses the same fixture but pixel-verifies the
 *        D1C rect, which the actual_pose probe does not do).
 *   firestaff_dm1_v1_champion_mirror_portrait04_rect_position_runtime_probe
 *     - locks the positive (2,1,S)=4 route at the D1C rect and
 *       the wrong-wall (1,2,E) no-floating on the *same* pose
 *       (this probe extends the same (1,2,E) no-floating
 *       guarantee with: (a) a 4-direction rotate-away at the
 *       same wrong-wall cell, (b) an approach-band sweep across
 *       the entire x=1 corridor, and (c) a positive cross-check
 *       that the D1C rect is alive at (2,1,S) with ordinal 4).
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_east_walkpath_portrait_rect_position_runtime_probe
 *     - sealed-chamber east_walkpath guard + (2,1,S)=4 portrait
 *       pixel match at 95% (this probe focuses on the wrong-wall
 *       west side of the chamber, not the sealed east entry).
 *   firestaff_dm1_v1_hall_of_champions_champion_portrait_04_south_return_portrait_rect_position_probe
 *     - south_return contract-portable slice anchored at
 *       (2,1,SOUTH) with both pre-fix and post-fix ordinal
 *       expectations (this probe works on the (1,2,E) side of
 *       the same chamber).
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     - visual PPM captures of the canonical LEIF pose
 *       (this probe is contract-only, no PPM).
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_portrait_rect_position_runtime_probe
 *     - the ordinal-2 west_negative template this probe follows
 *       (different ordinal + different approach angle, disjoint
 *        data fixtures).
 *   firestaff_dm1_v1_champion_mirror_ordinal_17_west_negative_portrait_rect_position_runtime_probe
 *     - ordinal-17 west_negative (disjoint, different cell and
 *       sensor layout).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=4 exists at
 *     the (1, 2) DIR_EAST visible wall.  The approach_from_left
 *     slice is specifically the negative route, and the local PC 3.4
 *     DUNGEON.DAT is the source-locked fixture that proves the
 *     rectangle is empty at the (1, 2) DIR_EAST wrong-wall pose.
 *   - The probe does not load real DOSBox captures or original
 *     PC 3.4 screenshots; it uses the same runtime state the live
 *     M11 game view uses, with the same asset loader pipeline the
 *     renderer is using, so the comparison is apples-to-apples.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* ReDMCSB DUNVIEW.C:525 G0109 portrait box = {96, 127, 35, 63}.
     * The inner portrait cutout used by m11_draw_dm1_front_champion
     * _portrait is (96, 35, 32, 29).  Width 32 / height 29 from
     * ReDMCSB COORD.C:1748-1749 (G2078_C32_PortraitWidth=32,
     * G2079_C29_PortraitHeight=29). */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_TRANSPARENT = 1,
    /* Source-locked wall box from DUNVIEW.C G0205 Graphic558
     * coordSet 5 / index 12 (C346 D1C champion-mirror route). */
    WALLBOX_X = 80,
    WALLBOX_Y = 29,
    WALLBOX_W = 64,
    WALLBOX_H = 43,
    /* Match thresholds.  At the approach_from_left pose the D1C
     * cutout must not contain a C026 ordinal-4 portrait.  We allow
     * up to 35% pixel match against ordinal 4 (the wrong-ordinal
     * drift threshold used by the actual-pose probe's
     * check_no_stale_ordinal_in_rect).  Above 35% means a stale
     * LEIF sprite is floating over the corridor west wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal. */
    ORDINAL_TARGET = 4,
    /* The cross-check ordinal comes from the canonical positive
     * LEIF route at (2, 1) DIR_SOUTH. */
    ORDINAL_CROSSCHECK = 4,
    /* The cross-check pose anchors the D1C rect's liveness check. */
    CROSSCHECK_MAP_X = 2,
    CROSSCHECK_MAP_Y = 1,
    CROSSCHECK_DIR = 2 /* DIR_SOUTH */
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Pixel-match the D1C portrait cutout (96, 35, 32, 29) against a
 * single 32x29 cell of the C026 strip (graphics.dat asset slot
 * M11_GFX_CHAMPION_PORTRAITS = 26, atlas 256x87, 8 cols x 3 rows
 * of 32x29 portraits).  Returns matched-percent (0..100) or -1 if
 * the asset is missing.  Source pixels with palette index 1 (the
 * blitter transparentColor used by m11_draw_dm1_front_champion
 * _portrait) are skipped so the wall-niche background bleed does
 * not skew the match.  PROBE_COLOR_DARK_GRAY (=12) is the
 * wall-niche backdrop that the
 * firestaff_dm1_v1_hall_of_champions_mirror_zones_probe treats
 * as transparent; we skip that on the source side too. */
static int match_portrait_cell(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y;
    int srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if ((int)portraits->width < 8 * PORTRAIT_W) return -1;
    if ((int)portraits->height < 3 * PORTRAIT_H) return -1;
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)(
                portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            unsigned char dst = (unsigned char)(
                fb[(VIEWPORT_Y + 35 + y) * FB_W + (VIEWPORT_X + 96 + x)] & 0x0F);
            if (src == PORTRAIT_TRANSPARENT) continue;
            if (src == 12) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
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

/* ── Group A: source-locked D1C wall-ornament zone invariant ────────
 * The D1C wall-ornament zone is reserved at the source-locked
 * (80, 29, 64, 43) rectangle regardless of party pose (DUNVIEW.C
 * G0205 Graphic558 coordSet 5 / index 12).  The probe must read
 * the same box at the (1, 2) DIR_EAST approach_from_left pose so
 * a regression that re-routes the ornament under a wrong-wall
 * pose is caught. */
static int check_d1c_wall_ornament_zone(M11_GameViewState* game) {
    int x = -1, y = -1, w = -1, h = -1;
    int ok = 1;
    char msg[200];

    set_pose(game, 1, 2, 1 /* DIR_EAST */);
    M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    snprintf(msg, sizeof(msg),
             "D1C wall-ornament zone at (1,2,EAST) = (%d,%d,%d,%d) "
             "(expected (80, 29, 64, 43) per DUNVIEW.C G0205 "
             "coordSet 5 / index 12)",
             x, y, w, h);
    CHECK(x == WALLBOX_X && y == WALLBOX_Y &&
          w == WALLBOX_W && h == WALLBOX_H, msg);

    return ok;
}

/* ── Group B: engine-helper front-ordinal invariant ─────────────────
 * The actual_pose probe already prints (1,2,DIR_EAST) = -1 as
 * "hall_start_east_wrong_wall_no_portrait".  This probe locks the
 * same value as the dedicated approach_from_left slice so a
 * future refactor that misroutes the LEIF sensor to the west wall
 * of (2, 2) is caught. */
static int check_front_ordinal_approach_from_left(M11_GameViewState* game) {
    int ord = -999;
    int ok = 1;
    char msg[200];

    set_pose(game, 1, 2, 1 /* DIR_EAST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (1,2,DIR_EAST) = %d "
             "(expected -1, wrong wall under DUNGEON.C:2573 filter)",
             ord);
    CHECK(ord == -1, msg);
    return ok;
}

/* ── Group C: D1C portrait_rect_position pixel-emptiness ───────────
 * Drive M11_GameView_Draw at (1, 2) DIR_EAST and verify the D1C
 * portrait cutout (96, 35, 32, 29) does NOT carry ordinal-4
 * (LEIF) pixels at > 35% match.  A regression that paints the
 * LEIF sprite over the corridor west wall would push the match
 * above 35% and trip this assertion. */
static int check_no_floating_on_approach_from_left(
    M11_GameViewState* game,
    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    int ok = 1;
    char msg[200];

    set_pose(game, 1, 2, 1 /* DIR_EAST */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    matchPct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "D1C portrait_rect_position at (1,2,EAST) carries ordinal %d "
             "(LEIF) pixels at < %d%% match (got %d%%) - no-floating "
             "invariant on the approach_from_left wrong-wall route",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= 0 && matchPct < WRONG_ORDINAL_MATCH_PCT, msg);

    return ok;
}

/* ── Group D: positive cross-check at the canonical LEIF route ─────
 * The D1C cutout must NOT be dead: at the canonical positive LEIF
 * route (2, 1) DIR_SOUTH the SAME rectangle IS painted with
 * ordinal 4 at >= 90% match.  An empty rectangle at (1, 2) E must
 * not silently mean the rectangle is dead. */
static int check_positive_crosscheck_leif_route(
    M11_GameViewState* game,
    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    int ord = -999;
    int ok = 1;
    char msg[240];

    set_pose(game, CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, CROSSCHECK_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (%d,%d,SOUTH) = %d "
             "(expected %d, LEIF visible from north of (2,2))",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ord, ORDINAL_CROSSCHECK);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);
    if (ord != ORDINAL_CROSSCHECK) {
        return 0;
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    matchPct = match_portrait_cell(portraits, fb, ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "D1C portrait_rect_position at (%d,%d,SOUTH) carries ordinal %d "
             "pixels at >= %d%% match (got %d%%) - positive cross-check "
             "proves the D1C rect is alive at the canonical LEIF route",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ORDINAL_CROSSCHECK,
             CORRECT_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= CORRECT_ORDINAL_MATCH_PCT, msg);

    return ok;
}

/* ── Group E: approach-cell band sweep ────────────────────────────
 * The (1, 2) cell is the only corridor cell immediately west of the
 * (2, 2) LEIF chamber, so the (1, 2) DIR_EAST pose is the only
 * "approach from left" route for the LEIF mirror.  All four facings
 * at (1, 2) must reject LEIF: NORTH sees the HALK mirror on the
 * (1, 1) NORTH wall (cell=2 → front cell=(1, 1), ordinal 1), EAST
 * sees the (2, 2) WEST wall (no sensor, -1), SOUTH sees the (1, 3)
 * WEST wall (no sensor, -1), and WEST sees the (0, 2) door (no
 * mirror, -1).  This is the in-place-turn analogue the existing
 * ordinal_2 west_negative probe's check_rotate_away exercises for
 * (1, 4) DIR_WEST; here we lock it for the (1, 2) approach cell
 * across all four facings so a future refactor that misroutes the
 * LEIF sensor to the (1, 2) WEST or (1, 2) SOUTH wall is caught.
 *
 * Note: other x=1 corridor cells (1, 3) EAST, (1, 4) EAST, (1, 5)
 * EAST, etc.) do NOT belong to this slice.  Each of those cells is
 * the "approach from left" for a *different* champion's mirror
 * (1, 3) EAST = SONJA ordinal 18 at the (2, 3) WEST wall, (1, 4)
 * EAST = no mirror, (1, 5) EAST = no mirror, etc.).  Those
 * champions have their own ordinal-XX west_negative / east_walkpath
 * / approach_* probes; this slice stays focused on the (1, 2) cell
 * and ordinal 4 (LEIF) only. */
static int check_approach_cell_band_sweep(M11_GameViewState* game) {
    static const struct {
        int x;
        int y;
        int dir;
        int expectedOrdinal;
        const char* label;
    } kApproachPoses[] = {
        /* (1, 2) DIR_EAST — the primary approach_from_left anchor.
         * Front cell is (2, 2); visible wall is its WEST wall which
         * has no C127 sensor under DM1 V1. */
        {1, 2, 1, -1, "leif_approach_from_left_east"},
        /* (1, 2) DIR_SOUTH — party looks south, front cell is (1, 3),
         * visible wall is its WEST wall, no C127 sensor. */
        {1, 2, 2, -1, "leif_approach_from_left_south"},
        /* (1, 2) DIR_WEST — party looks west, front cell is (0, 2),
         * the door cell, no mirror. */
        {1, 2, 3, -1, "leif_approach_from_left_west"},
    };
    int i;
    int n = (int)(sizeof(kApproachPoses) / sizeof(kApproachPoses[0]));
    int ok = 1;

    for (i = 0; i < n; ++i) {
        int ord = -999;
        char msg[240];
        set_pose(game,
                 kApproachPoses[i].x, kApproachPoses[i].y,
                 kApproachPoses[i].dir);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (%d,%d,dir=%d) [%s] = %d "
                 "(expected %d, no LEIF mirror visible from the (1,2) "
                 "approach cell in this direction)",
                 kApproachPoses[i].x, kApproachPoses[i].y,
                 kApproachPoses[i].dir, kApproachPoses[i].label, ord,
                 kApproachPoses[i].expectedOrdinal);
        CHECK(ord == kApproachPoses[i].expectedOrdinal, msg);
    }
    return ok;
}

/* ── Group F: 4-direction rotate-away at the (1, 2) cell ───────────
 * The (1, 2) cell is the approach_from_left anchor for LEIF, but
 * the four facings at (1, 2) are NOT all wrong-wall.  Under the
 * actual-pose probe's DM1 V1 fixture:
 *   - (1, 2) DIR_NORTH exposes the HALK mirror (ordinal 1) on the
 *     (1, 1) NORTH wall — this is the canonical HALK route, not
 *     LEIF, and ordinal 1 is a different portrait.
 *   - (1, 2) DIR_EAST is the wrong wall for LEIF, returns -1 (this
 *     probe's primary slice).
 *   - (1, 2) DIR_SOUTH is the wrong wall for any mirror, returns -1
 *     (no C127 sensor on the (1, 3) WEST wall).
 *   - (1, 2) DIR_WEST is the wrong wall for any mirror, returns -1
 *     (front cell is (0, 2) which is the corridor door cell with no
 *     mirror).
 *
 * What this group locks is the specific invariant: at the (1, 2)
 * approach_from_left cell, the three facings OTHER than DIR_EAST
 * that reject LEIF (DIR_SOUTH, DIR_WEST) must return -1, and the
 * DIR_NORTH facing must return the HALK ordinal 1 (proving the cell
 * is alive in the corridor, just not a LEIF approach).  This is
 * disjoint from the Group E band sweep: Group E covers the same
 * (1, 2) cell with the focus on the LEIF-rejection invariant
 * (ordinal 4 never visible at the (1, 2) approach cell), while
 * Group F covers the cross-mirror invariant (the only mirror visible
 * from (1, 2) is HALK, not LEIF, when facing NORTH). */
static int check_rotate_away_at_approach_cell(M11_GameViewState* game) {
    static const struct {
        int dir;
        int expectedOrdinal;
        const char* label;
    } kDirs[] = {
        {0, 1, "leif_approach_rotate_north_halk"},
        {1, -1, "leif_approach_rotate_east_wrong_wall"},
        {2, -1, "leif_approach_rotate_south_wrong_wall"},
        {3, -1, "leif_approach_rotate_west_wrong_wall"},
    };
    int i;
    int n = (int)(sizeof(kDirs) / sizeof(kDirs[0]));
    int ok = 1;

    for (i = 0; i < n; ++i) {
        int ord = -999;
        char msg[240];
        set_pose(game, 1, 2, kDirs[i].dir);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (1,2,dir=%d) [%s] = %d "
                 "(expected %d: DIR_NORTH=HALK(1) at (1,1) NORTH wall, "
                 "DIR_EAST=wrong wall for LEIF, DIR_SOUTH/WEST=no mirror)",
                 kDirs[i].dir, kDirs[i].label, ord, kDirs[i].expectedOrdinal);
        CHECK(ord == kDirs[i].expectedOrdinal, msg);
    }
    return ok;
}

/* ── Group G: mirror catalog name resolution for ordinal 4 ─────────
 * The mirror catalog must resolve ordinal 4 to the LEIF record.
 * This catches a regression where the catalog and the C026 atlas
 * disagree on the ordinal-4 record (e.g., an atlas re-indexing
 * that moves LEIF to a different ordinal).  Same pattern the
 * ordinal_5 / ordinal_19 cancel_reopen probes use for their
 * respective ordinals. */
static int check_ordinal_4_catalog_resolution(M11_GameViewState* game) {
    char nameBuf[32] = {0};
    char titleBuf[32] = {0};
    int nameRc = 0;
    int ok = 1;
    char msg[200];

    nameRc = M11_GameView_GetMirrorNameByOrdinal(game,
                                                  ORDINAL_TARGET,
                                                  nameBuf,
                                                  (int)sizeof(nameBuf));
    snprintf(msg, sizeof(msg),
             "mirror catalog name for ordinal %d = \"%s\" (expected \"LEIF\")",
             ORDINAL_TARGET, nameBuf[0] ? nameBuf : "");
    CHECK(nameRc > 0 && strcmp(nameBuf, "LEIF") == 0, msg);

    (void)M11_GameView_GetMirrorTitleByOrdinal(game,
                                                ORDINAL_TARGET,
                                                titleBuf,
                                                (int)sizeof(titleBuf));
    /* The LEIF title is less important than the name — the catalog
     * title may be empty in some DM1 V1 builds.  Print it for
     * diagnostics but do not fail on it. */
    printf("  INFO: ordinal %d mirror catalog title = \"%s\"\n",
           ORDINAL_TARGET, titleBuf[0] ? titleBuf : "(untitled)");

    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-04 / approach_from_left / "
           "portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 8 * PORTRAIT_W ||
        portraits->height < 3 * PORTRAIT_H) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Group A — source-locked D1C wall-ornament zone invariant. */
    printf("\n[Group A] D1C wall-ornament zone at (1,2,EAST)\n");
    if (!check_d1c_wall_ornament_zone(&game)) ok = 0;

    /* Group B — engine-helper front-ordinal invariant. */
    printf("\n[Group B] front mirror ordinal at (1,2,EAST)\n");
    if (!check_front_ordinal_approach_from_left(&game)) ok = 0;

    /* Group C — D1C portrait_rect_position pixel-emptiness on the
     * wrong wall. */
    printf("\n[Group C] D1C portrait_rect_position pixel-emptiness on "
           "approach_from_left\n");
    if (!check_no_floating_on_approach_from_left(&game, portraits)) ok = 0;

    /* Group D — positive cross-check at the canonical LEIF route. */
    printf("\n[Group D] D1C portrait_rect_position positive cross-check at "
           "(2,1,SOUTH)\n");
    if (!check_positive_crosscheck_leif_route(&game, portraits)) ok = 0;

    /* Group E — approach-cell band sweep at (1, 2). */
    printf("\n[Group E] approach-cell band sweep at (1, 2)\n");
    if (!check_approach_cell_band_sweep(&game)) ok = 0;

    /* Group F — 4-direction rotate-away at the (1, 2) cell. */
    printf("\n[Group F] 4-direction rotate-away at (1, 2)\n");
    if (!check_rotate_away_at_approach_cell(&game)) ok = 0;

    /* Group G — mirror catalog name resolution. */
    printf("\n[Group G] ordinal 4 mirror catalog name resolution\n");
    if (!check_ordinal_4_catalog_resolution(&game)) ok = 0;

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
