/*
 * firestaff_dm1_v1_hoc_champion_portrait_01_approach_from_right_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 1 (HALK strip cell 1 — atlas col 1 row 0,
 *                                source rect (32, 0, 32, 29), mirror
 *                                catalog record "HALK" /
 *                                "THE BARBARIAN")
 *   route approach_from_right: party at (2, 1) facing WEST, the player
 *                              approaching the HALK chamber (1, 1) from
 *                              the RIGHT (east) side.  The visible wall
 *                              under this route is the WEST wall of
 *                              (1, 1), which has no C127 sensor — the
 *                              HALK sensorData=1 sensor sits on the
 *                              NORTH wall of (1, 1) per the actual_pose
 *                              probe fixture (party at (1, 2) facing
 *                              NORTH = ordinal 1).  This route must
 *                              therefore return front ordinal -1 and
 *                              the D1C portrait cutout (96, 35, 32, 29)
 *                              must stay empty: no HALK portrait
 *                              floats over the wrong (east-of-chamber)
 *                              wall.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 — exactly the source-locked DUNVIEW.C
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} blit destination.
 *
 * The slice was authored against the same DM1 V1 PC 3.4 fixture used
 * by the firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe:
 * (1, 2) DIR_NORTH is the canonical positive HALK route (front cell
 * is (1, 1) with the C127 sensor on cell 2 = the wall the party
 * visually faces from the south side of the chamber).  The
 * approach_from_right route (2, 1) DIR_WEST is the *wrong-wall* mirror
 * of the same (1, 1) cell — the party stands to the east of the
 * chamber and looks west at its east wall.  ReDMCSB DUNGEON.C:2573
 * normalizes (M011_CELL(sensor) - direction) and the front-cell filter
 * (visibleWallCell = (direction + 2) & 3) rejects the route when
 * the resulting side is not the visible wall side, so
 * m11_front_cell_mirror_ordinal must return -1 here, the D1C cutout
 * must be empty, and a regression that paints ordinal 1 over the
 * corridor east wall would push the C026 ordinal-1 pixel-match above
 * the 35% threshold.
 *
 * This probe narrows four contracts to the (2, 1) DIR_WEST
 * approach_from_right anchor:
 *
 *   1. Engine-helper invariant: M11_GameView_GetFrontMirrorOrdinal
 *      at (2, 1) DIR_WEST returns -1.  This is the same fixture
 *      line the actual_pose probe's "hall_start_east_wrong_wall_
 *      no_portrait" entry covers for the LEIF chamber's approach
 *      from the left, but is locked here as the dedicated
 *      ordinal-1 approach_from_right slice (party is to the east
 *      of the HALK chamber, not to the west of the LEIF chamber).
 *   2. D1C portrait_rect_position pixel-emptiness: the (96, 35, 32, 29)
 *      cutout on the rendered 320x200 framebuffer contains no
 *      C026 ordinal-1 pixels (>= 35% match would mean a stale
 *      HALK sprite floats over the wrong wall).  This is the
 *      no-floating invariant the existing ordinal_2 / ordinal_9 /
 *      ordinal_17 west_negative probes lock for their respective
 *      ordinals and that the ordinal_4 approach_from_left probe
 *      locks for LEIF, here applied to ordinal 1 (HALK) from
 *      the approach_from_right angle.
 *   3. Cross-check that the D1C cutout is *not* dead: at the
 *      canonical positive HALK route (1, 2) DIR_NORTH the SAME
 *      rectangle IS painted with ordinal 1 at >= 90% match.  An
 *      empty rectangle at (2, 1) W must not silently mean the
 *      rectangle is dead.  This is the same cross-check the
 *      ordinal_4 approach_from_left probe uses for (2, 1) S /
 *      ordinal 4.
 *   4. Mirror catalog name resolution: ordinal 1 must round-trip
 *      to the "HALK" mirror catalog record.  This catches a
 *      regression where the catalog and the C026 atlas disagree
 *      on the ordinal-1 record.
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
 *     - 16-pose ordinal map including (1,2,N)=1 (HALK canonical
 *       positive) and (1,2,E)=-1 (LEIF wrong-wall).  This probe
 *       adds the (2,1,W)=-1 HALK approach_from_right line that
 *       the actual_pose probe does not cover at the pixel level.
 *   firestaff_dm1_v1_champion_mirror_ordinal_1_halk_pose_probe
 *     - locks the (1,2)N=1 positive route and the (1,2)W=-1
 *       negative route at the D1C rect, plus the resurrect
 *       round-trip.  This probe extends the (1,2)W=-1 guarantee
 *       with the (2,1)W=-1 approach_from_right anchor (party to
 *       the east of the HALK chamber, not to the west of the
 *       door cell).
 *   firestaff_dm1_v1_hoc_champion_portrait_01_candidate_panel_open_portrait_rect_position_097_gate_probe
 *     - panel-open state at (1,2)N=1 with C040 RR panel + C017
 *       backdrop.  This probe works on the (2,1)W=-1 wrong-wall
 *       pre-candidate pose.
 *   firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate_portrait_rect_position_097_gate_probe
 *     - 4-cycle redraw_after_candidate (pre / panel-open /
 *       post-confirm / post-cancel) at (1,2)N=1.  This probe
 *       works on a single (2,1)W=-1 frame at the same level as
 *       the (1,2)W=-1 line in the actual_pose probe.
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from_left_portrait_rect_position_runtime_probe
 *     - the approach_from_left template this probe mirrors for
 *       HALK ordinal 1 / (2,1)W approach_from_right.  Disjoint
 *       data fixtures (different chamber, different ordinal,
 *       different approach angle).
 *   firestaff_dm1_v1_hall_of_champions_champion_portrait_01_south_return_portrait_rect_position_probe
 *     - south_return contract-portable slice anchored at
 *       (1,0,SOUTH) with both pre-fix and post-fix ordinal
 *       expectations (this probe works on the (2,1,W) side of
 *       the HALK chamber).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=1 exists at
 *     the (2, 1) DIR_WEST visible wall.  The approach_from_right
 *     slice is specifically the negative route, and the local PC 3.4
 *     DUNGEON.DAT is the source-locked fixture that proves the
 *     rectangle is empty at the (2, 1) DIR_WEST wrong-wall pose.
 *   - The probe does not load real DOSBox captures or original
 *     PC 3.4 screenshots; it uses the same runtime state the live
 *     M11 game view uses, with the same asset loader pipeline the
 *     renderer is using, so the comparison is apples-to-apples.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

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
    /* Match thresholds.  At the approach_from_right pose the D1C
     * cutout must not contain a C026 ordinal-1 portrait.  We allow
     * up to 35% pixel match against ordinal 1 (the wrong-ordinal
     * drift threshold used by the actual-pose probe's
     * check_no_stale_ordinal_in_rect).  Above 35% means a stale
     * HALK sprite is floating over the corridor east wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal. */
    ORDINAL_TARGET = 1,
    /* The cross-check ordinal comes from the canonical positive
     * HALK route at (1, 2) DIR_NORTH. */
    ORDINAL_CROSSCHECK = 1,
    /* The cross-check pose anchors the D1C rect's liveness check. */
    CROSSCHECK_MAP_X = 1,
    CROSSCHECK_MAP_Y = 2,
    CROSSCHECK_DIR = 0 /* DIR_NORTH */
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
 * the same box at the (2, 1) DIR_WEST approach_from_right pose so
 * a regression that re-routes the ornament under a wrong-wall
 * pose is caught. */
static int check_d1c_wall_ornament_zone(M11_GameViewState* game) {
    int x = -1, y = -1, w = -1, h = -1;
    int ok = 1;
    char msg[200];

    set_pose(game, 2, 1, 3 /* DIR_WEST */);
    M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    snprintf(msg, sizeof(msg),
             "D1C wall-ornament zone at (2,1,WEST) = (%d,%d,%d,%d) "
             "(expected (80, 29, 64, 43) per DUNVIEW.C G0205 "
             "coordSet 5 / index 12)",
             x, y, w, h);
    CHECK(x == WALLBOX_X && y == WALLBOX_Y &&
          w == WALLBOX_W && h == WALLBOX_H, msg);

    return ok;
}

/* ── Group B: engine-helper front-ordinal invariant ─────────────────
 * The actual_pose probe prints (1,2,DIR_NORTH) = 1 as the canonical
 * HALK positive route and (1,2,DIR_EAST) = -1 as the wrong-wall
 * anchor for the LEIF chamber.  This probe locks the same kind of
 * wrong-wall invariant for the HALK chamber from the OPPOSITE side:
 * party at (2, 1) DIR_WEST, front cell (1, 1), visible wall cell
 * (W+2)&3 = 1 (the wall facing the party from the east side of the
 * chamber).  The C127 sensor for HALK sits on cell 2 (the wall
 * facing the party from the south side of the chamber), so
 * m11_front_cell_mirror_ordinal must return -1.  A future refactor
 * that misroutes the HALK sensor to the (1, 1) west/east wall is
 * caught by this invariant. */
static int check_front_ordinal_approach_from_right(M11_GameViewState* game) {
    int ord = -999;
    int ok = 1;
    char msg[200];

    set_pose(game, 2, 1, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (2,1,DIR_WEST) = %d "
             "(expected -1, wrong wall under DUNGEON.C:2573 filter)",
             ord);
    CHECK(ord == -1, msg);
    return ok;
}

/* ── Group C: D1C portrait_rect_position pixel-emptiness ───────────
 * Drive M11_GameView_Draw at (2, 1) DIR_WEST and verify the D1C
 * portrait cutout (96, 35, 32, 29) does NOT carry ordinal-1
 * (HALK) pixels at > 35% match.  A regression that paints the
 * HALK sprite over the corridor east wall would push the match
 * above 35% and trip this assertion. */
static int check_no_floating_on_approach_from_right(
    M11_GameViewState* game,
    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    int ok = 1;
    char msg[200];

    set_pose(game, 2, 1, 3 /* DIR_WEST */);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    matchPct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "D1C portrait_rect_position at (2,1,WEST) carries ordinal %d "
             "(HALK) pixels at < %d%% match (got %d%%) - no-floating "
             "invariant on the approach_from_right wrong-wall route",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= 0 && matchPct < WRONG_ORDINAL_MATCH_PCT, msg);

    return ok;
}

/* ── Group D: positive cross-check at the canonical HALK route ─────
 * The D1C cutout must NOT be dead: at the canonical positive HALK
 * route (1, 2) DIR_NORTH the SAME rectangle IS painted with
 * ordinal 1 at >= 90% match.  An empty rectangle at (2, 1) W must
 * not silently mean the rectangle is dead. */
static int check_positive_crosscheck_halk_route(
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
             "front mirror ordinal at (%d,%d,NORTH) = %d "
             "(expected %d, HALK visible from south of (1,1))",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ord, ORDINAL_CROSSCHECK);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);
    if (ord != ORDINAL_CROSSCHECK) {
        return 0;
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    matchPct = match_portrait_cell(portraits, fb, ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "D1C portrait_rect_position at (%d,%d,NORTH) carries ordinal %d "
             "pixels at >= %d%% match (got %d%%) - positive cross-check "
             "proves the D1C rect is alive at the canonical HALK route",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ORDINAL_CROSSCHECK,
             CORRECT_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= CORRECT_ORDINAL_MATCH_PCT, msg);

    return ok;
}

/* ── Group E: mirror catalog name resolution for ordinal 1 ─────────
 * The mirror catalog must resolve ordinal 1 to the HALK record.
 * This catches a regression where the catalog and the C026 atlas
 * disagree on the ordinal-1 record (e.g., an atlas re-indexing
 * that moves HALK to a different ordinal).  Same pattern the
 * ordinal_5 / ordinal_19 cancel_reopen probes use for their
 * respective ordinals. */
static int check_ordinal_1_catalog_resolution(M11_GameViewState* game) {
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
             "mirror catalog name for ordinal %d = \"%s\" (expected \"HALK\")",
             ORDINAL_TARGET, nameBuf[0] ? nameBuf : "");
    CHECK(nameRc > 0 && strcmp(nameBuf, "HALK") == 0, msg);

    (void)M11_GameView_GetMirrorTitleByOrdinal(game,
                                                ORDINAL_TARGET,
                                                titleBuf,
                                                (int)sizeof(titleBuf));
    /* The HALK title is less important than the name — the catalog
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
    printf("=== DM1 V1 Hall of Champions portrait-01 (HALK) / approach_from_right / "
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
    printf("\n[Group A] D1C wall-ornament zone at (2,1,WEST)\n");
    if (!check_d1c_wall_ornament_zone(&game)) ok = 0;

    /* Group B — engine-helper front-ordinal invariant. */
    printf("\n[Group B] front mirror ordinal at (2,1,WEST)\n");
    if (!check_front_ordinal_approach_from_right(&game)) ok = 0;

    /* Group C — D1C portrait_rect_position pixel-emptiness on the
     * wrong wall. */
    printf("\n[Group C] D1C portrait_rect_position pixel-emptiness on "
           "approach_from_right\n");
    if (!check_no_floating_on_approach_from_right(&game, portraits)) ok = 0;

    /* Group D — positive cross-check at the canonical HALK route. */
    printf("\n[Group D] D1C portrait_rect_position positive cross-check at "
           "(1,2,NORTH)\n");
    if (!check_positive_crosscheck_halk_route(&game, portraits)) ok = 0;

    /* Group E — mirror catalog name resolution. */
    printf("\n[Group E] ordinal 1 mirror catalog name resolution\n");
    if (!check_ordinal_1_catalog_resolution(&game)) ok = 0;

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
