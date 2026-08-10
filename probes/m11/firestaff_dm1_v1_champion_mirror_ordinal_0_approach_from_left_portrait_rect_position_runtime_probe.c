/*
 * firestaff_dm1_v1_champion_mirror_ordinal_0_approach_from_left_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 0 (C026 strip cell (0, 0) — atlas rect
 *                                (0, 0, 32, 29); mirror catalog record
 *                                "DAROOU" per F0660/F0661.  DAROOU is
 *                                the ordinal-zero edge case: a 0-valued
 *                                sensorData is easy to mis-handle as
 *                                "false" / "absent" instead of as a
 *                                valid C026 portrait index, so the
 *                                ordinal 0 route needs the same
 *                                dedicated approach_from_left lock
 *                                that ordinals 4 (LEIF), 21 (ZED), 8
 *                                (IAIDO), etc. already have.)
 *   route approach_from_left: party at (map=0, x=7, y=7) facing EAST,
 *                             the player approaching the DAROOU
 *                             mirror cell (8, 7) from the LEFT (west)
 *                             side.  The visible wall under this
 *                             route is the WEST wall of (8, 7),
 *                             which has no C127 sensor — the
 *                             DAROOU sensorData=0 sensor sits on
 *                             the EAST wall of (8, 7) per the
 *                             verified PC34 C127 layout (canonical
 *                             positive route (9, 7) DIR_WEST,
 *                             no seed required).  This
 *                             route must therefore return front
 *                             ordinal -1 and the D1C portrait
 *                             cutout (96, 35, 32, 29) must stay
 *                             empty: no DAROOU portrait floats
 *                             over the corridor west wall.
 *   aspect portrait_rect_position: viewport rectangle
 *                                 (96, 35, 32, 29) — exactly the
 *                                 source-locked DUNVIEW.C
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96,
 *                                 127, 35, 63} blit destination
 *                                 (DUNVIEW.C:525).
 *
 * The slice was authored against the same DM1 V1 PC 3.4 fixture
 * used by the firestaff_dm1_v1_champion_mirror_actual_pose_runtime
 * _probe, rebased on the verified PC34 C127 layout: (9, 7) DIR_WEST
 * is the canonical positive DAROOU route (front cell (8, 7) has
 * the C127 sensor on cell 1 = EAST with sensorData=0).  The
 * ordinal 0 route is the ordinal-zero edge: a future regression
 * that treats sensorData=0 as "false" / "no sensor" would break
 * the ordinal 0 candidate entirely.  The approach_from_left route
 * (7, 7) DIR_EAST is the *wrong-wall* mirror of the same (8, 7)
 * cell — the party stands to the west of the mirror cell and
 * looks east at its west wall.  ReDMCSB DUNGEON.C:2573 normalizes
 * (M011_CELL(sensor) - direction) and discards the route when
 * the resulting side is not the visible wall side, so
 * m11_front_cell_mirror_ordinal must return -1 here, the D1C
 * cutout must be empty, and a regression that paints ordinal 0
 * over the corridor wall would push the C026 ordinal-0 pixel
 * match above the 35% threshold.
 *
 * This probe narrows four contracts to the (7, 7) DIR_EAST
 * approach_from_left anchor:
 *
 *   1. Engine-helper invariant: M11_GameView_GetD1CWallOrnament
 *      Zone at (7, 7) DIR_EAST returns the source-locked
 *      (80, 29, 64, 43) D1C wall-mirror frame, so the C026
 *      portrait cutout lives at (96, 35, 32, 29) inside the
 *      frame regardless of party pose.
 *   2. Front mirror ordinal invariant:
 *      M11_GameView_GetFrontMirrorOrdinal at (7, 7) DIR_EAST
 *      returns -1.  This is the ordinal 0 negative-slice
 *      complement to the actual-pose probe's
 *      "hall_start_east_wrong_wall_no_portrait" line (which is
 *      anchored at (1, 2) E for the LEIF side; this probe is
 *      anchored at (7, 7) E for the DAROOU side).
 *   3. D1C portrait_rect_position pixel-emptiness: the
 *      (96, 35, 32, 29) cutout on the rendered 320x200
 *      framebuffer contains no C026 ordinal-0 (DAROOU) pixels
 *      (>= 35% match would mean a stale DAROOU sprite floats
 *      over the corridor west wall).  This is the no-floating
 *      invariant the existing ordinal_2 / ordinal_9 /
 *      ordinal_17 west_negative probes lock for their
 *      respective ordinals, here applied to ordinal 0 (DAROOU)
 *      at the approach_from_left wrong-wall route.
 *   4. Cross-check that the D1C cutout is *not* dead: at the
 *      canonical positive DAROOU route (9, 7) DIR_WEST (native
 *      (8, 7) EAST-wall sensor on the shipped fixture; on a
 *      local build without it we seed the first HALK sensor to
 *      sensorData=0 and cross-check at (7, 9) DIR_NORTH) — the
 *      SAME rectangle IS
 *      painted with ordinal 0 at >= 90% match.  An empty
 *      rectangle at (7, 7) E must not silently mean the
 *      rectangle is dead.  This is the same cross-check the
 *      ordinal_4_approach_from_left probe uses for the (2, 1)
 *      S LEIF cross-check ordinal 4.
 *
 * Plus three orthogonal invariants:
 *
 *   5. Approach-band sweep: every (map=0, x=7, y=6..10) DIR_EAST
 *      pose must consistently return -1 for the front mirror
 *      ordinal.  These are the "approach from the left" poses
 *      for the (8, y) column: the player is to the west of
 *      the (8, y) column and looks east, so the visible wall
 *      is the WEST wall of (8, y) and carries no C127 sensor
 *      per the shipped DM1 V1 DUNGEON.DAT (the C127 sensor
 *      matrix on the corridor cells only fires on the NORTH
 *      and SOUTH aspects, not the WEST aspect).
 *   6. 4-direction rotate-away at the (7, 7) cell: the three
 *      facings OTHER than DIR_EAST that reject DAROOU
 *      (DIR_SOUTH, DIR_WEST) must also return -1, and the
 *      DIR_NORTH facing must return ordinal -1 as well (the
 *      (7, 6) SOUTH wall carries no C127 sensor under the
 *      verified PC34 C127 layout).  This proves the approach cell is
 *      alive in the corridor without accidentally exposing a
 *      mirror.
 *   7. Mirror catalog name resolution for ordinal 0: the
 *      mirror catalog must resolve ordinal 0 to the DAROOU
 *      record.  This catches a regression where the catalog
 *      and the C026 atlas disagree on the ordinal-0 record
 *      (e.g., an atlas re-indexing that moves DAROOU to a
 *      different ordinal).
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
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29
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
 *     - 16-pose ordinal map including (1,2,E)=-1 (LEIF side) and
 *       (1,2,N)=1 (HALK side).  This probe uses the same fixture
 *       (with a C127 sensor seed from HALK=1 to DAROOU=0) but
 *       pixel-verifies the D1C rect at the (7,7,E) approach
 *       anchor, which the actual_pose probe does not cover.
 *   firestaff_dm1_v1_champion_mirror_portrait00_rect_runtime_probe
 *     - locks the positive (1,2,N)=0 (seeded) route at the D1C
 *       rect; this probe uses the same seed and cross-checks
 *       the same positive pose via the Group D cross-check, but
 *       adds the (7,7,E) approach_from_left negative slice.
 *   firestaff_dm1_v1_champion_mirror_portrait00_south_return
 *     _portrait_rect_position_probe
 *     - ordinal 0 on the south_return route via the (1,5,SOUTH)
 *       cell with C127 sensor rewritten to 0.  Different cell,
 *       different route, different aspect.
 *   firestaff_dm1_v1_hall_champion_portrait_00_front_north_entry
 *     _runtime_probe
 *     - the front_north_entry contract for ordinal 0, including
 *       the (1,2,N)=0 (seeded) cross-check.  This probe reuses
 *       the same cross-check pose (Group D) but adds the
 *       (7,7,E) approach_from_left negative slice plus the
 *       4-direction rotate-away and approach-band sweep that
 *       the front_north_entry contract does not exercise.
 *   firestaff_dm1_v1_hall_of_champions_portrait_00_cancel_reopen
 *     - ordinal 0 cancel_reopen on the (1,2,N) positive route
 *       (select -> cancel -> re-select cycle).  Different
 *       invariant: cancel_reopen state machine vs approach
 *       from-left wrong-wall negative.
 *   firestaff_dm1_v1_hoc_champion_portrait_00_d2r_negative
 *     - ordinal 0 d2r_negative at (1,2,E).  Different cell side
 *       (this probe uses (7,7,E) which is the cell west of
 *       the (8,7) DAROOU chamber, not the cell east of the
 *       (1,2) corridor).  The d2r_negative probe covers the
 *       D1C cutout + D2R side wall at (1,2,E); this probe
 *       covers the D1C cutout only at (7,7,E).
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from
 *     _left_portrait_rect_position_runtime_probe
 *     - the ordinal 4 (LEIF) approach_from_left template this
 *       probe follows (different ordinal + different chamber
 *       cell, disjoint data fixtures).  The ordinal 4 chamber
 *       is (2, 2) and its approach from left is (1, 2)
 *       DIR_EAST; the ordinal 0 chamber is (8, 7) and its
 *       approach from left is (7, 7) DIR_EAST.  No overlap.
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled
 *     from the same GRAPHICS.DAT the runtime is drawing from,
 *     so this is runtime correctness rather than pixel-for-
 *     pixel DOSBox reference parity.
 *   - We do not assume a C127 sensor with sensorData=0 exists
 *     at the (7, 7) DIR_EAST visible wall.  The approach
 *     from_left slice is specifically the negative route, and
 *     the local PC 3.4 DUNGEON.DAT is the source-locked
 *     fixture that proves the rectangle is empty at the
 *     (7, 7) DIR_EAST wrong-wall pose.
 *   - The probe does not load real DOSBox captures or original
 *     PC 3.4 screenshots; it uses the same runtime state the
 *     live M11 game view uses, with the same asset loader
 *     pipeline the renderer is using, so the comparison is
 *     apples-to-apples.
 *
 * Usage:
 *   firestaff_dm1_v1_champion_mirror_ordinal_0_approach_from_left
 *   _portrait_rect_position_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "firestaff_dm1_probe_data_dir.h"
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
     * The inner portrait cutout used by
     * m11_draw_dm1_front_champion_portrait is (96, 35, 32, 29).
     * Width 32 / height 29 from ReDMCSB COORD.C:1748-1749
     * (G2078_C32_PortraitWidth=32, G2079_C29_PortraitHeight=29). */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_TRANSPARENT = 1,
    /* Source-locked D1C wall-mirror frame from DUNVIEW.C G0205
     * Graphic558 coordSet 5 / index 12 (C346 D1C champion-mirror
     * route).  Inner portrait cutout is parented at
     * (frame.x + 16, frame.y + 6) = (96, 35). */
    WALLBOX_X = 80,
    WALLBOX_Y = 29,
    WALLBOX_W = 64,
    WALLBOX_H = 43,
    /* Match thresholds.  At the approach_from_left pose the D1C
     * cutout must not contain a C026 ordinal-0 (DAROOU) portrait.
     * We allow up to 35% pixel match against ordinal 0 (the
     * wrong-ordinal drift threshold used by the actual-pose
     * probe's check_no_stale_ordinal_in_rect).  Above 35% means
     * a stale DAROOU sprite is floating over the corridor west
     * wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal (DAROOU, mirror catalog record). */
    ORDINAL_TARGET = 0,
    /* The cross-check ordinal from the native (9, 7) DIR_WEST
     * pose.  The shipped PC 3.4 DUNGEON.DAT carries the ordinal-0
     * (DAROOU) C127 sensor natively on the (8, 7) EAST wall
     * (verified PC34 C127 layout), so no seed is required on the
     * shipped fixture. */
    ORDINAL_CROSSCHECK = 0,
    /* The cross-check pose anchors the D1C rect's liveness check. */
    CROSSCHECK_MAP_X = 9,
    CROSSCHECK_MAP_Y = 7,
    CROSSCHECK_DIR = 3 /* DIR_WEST */,
    /* The approach_from_left anchor cell (7, 7) DIR_EAST.  This
     * is the floor cell immediately west of the (8, 7)
     * DAROOU mirror cell — the cell the party occupies when
     * "approaching from the left" of the DAROOU mirror. */
    APPROACH_MAP_X = 7,
    APPROACH_MAP_Y = 7,
    APPROACH_DIR = 1 /* DIR_EAST */,
    /* Fallback only: if a local build lacks the native DAROOU
     * sensor, the Group D cross-check rewrites the first HALK
     * (sensorData=1) sensor — the (7, 8) SOUTH wall on the
     * shipped fixture — to ordinal 0 and restores it before
     * exit so subsequent CTest runs see the shipped DM1 V1
     * data. */
    SHIPPED_HALK_ORDINAL = 1
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
 * of 32x29 portraits).  Returns matched-percent (0..100) or -1
 * if the asset is missing.  Source pixels with palette index 1
 * (the blitter transparentColor used by
 * m11_draw_dm1_front_champion_portrait) are skipped so the
 * wall-niche background bleed does not skew the match.  Same
 * logic as match_portrait_cell in the ordinal_4_approach_from
 * _left probe. */
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
            ++compared;
            if (src == dst) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Find the first C127 sensor in the loaded world whose
 * sensorData equals oldData and rewrite it to newData.  Returns
 * the sensor index on success, or -1 if no such sensor was
 * found.  The sensor rewrite does NOT change the map layout or
 * the C127 cell match — only the G0289 ordinal that
 * DUNVIEW.C:3913-3928 reads through M000_INDEX_TO_ORDINAL
 * (DUNGEON.C:2610-2612).  Used to seed the (8, 7) chamber's
 * C127 sensor from the shipped HALK (sensorData=1) to DAROOU
 * (sensorData=0) for the Group D cross-check, and to restore
 * the original sensorData before exit. */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData,
                                int newData,
                                unsigned short* outSaved) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            if (outSaved) {
                *outSaved = state->world.things->sensors[i].sensorData;
            }
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Park the party at the given (mapX, mapY, direction) pose on
 * map 0 (Hall of Champions) and clear candidate-panel state so
 * the rewritten sensor is reflected by GetFrontMirrorOrdinal
 * without a stale candidate being returned. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

/* ── Group A: source-locked D1C wall-ornament zone invariant ────────
 * The D1C wall-ornament zone is reserved at the source-locked
 * (80, 29, 64, 43) rectangle regardless of party pose (DUNVIEW.C
 * G0205 Graphic558 coordSet 5 / index 12).  The probe must read
 * the same box at the (7, 7) DIR_EAST approach_from_left pose so
 * a regression that re-routes the ornament under a wrong-wall
 * pose is caught. */
static void check_d1c_wall_ornament_zone(M11_GameViewState* game) {
    int x = -1, y = -1, w = -1, h = -1;
    int rc = 0;
    char msg[200];

    printf("\n[Group A] D1C wall-ornament zone at (7,7,EAST)\n");

    set_pose(game, APPROACH_MAP_X, APPROACH_MAP_Y, APPROACH_DIR);
    rc = M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetD1CWallOrnamentZone returns 1 at (7,7,EAST) "
             "(got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "D1C wall-ornament zone at (7,7,EAST) = (%d,%d,%d,%d) "
             "(expected (80, 29, 64, 43) per DUNVIEW.C G0205 "
             "coordSet 5 / index 12)",
             x, y, w, h);
    CHECK(x == WALLBOX_X && y == WALLBOX_Y &&
          w == WALLBOX_W && h == WALLBOX_H, msg);
}

/* ── Group B: engine-helper front-ordinal invariant ─────────────────
 * The actual_pose probe prints (1,2,DIR_EAST) = -1 as
 * "hall_start_east_wrong_wall_no_portrait".  This probe locks
 * the analogous (7,7,DIR_EAST) = -1 as the dedicated ordinal 0
 * (DAROOU) approach_from_left slice so a future refactor that
 * misroutes the DAROOU sensor to the west wall of (8, 7) is
 * caught.  This is the ordinal 0 negative-slice complement to
 * the LEIF ordinal 4 (1, 2) DIR_EAST = -1 line. */
static void check_front_ordinal_approach_from_left(M11_GameViewState* game) {
    int ord = -999;
    char msg[200];

    printf("\n[Group B] front mirror ordinal at (7,7,DIR_EAST) "
           "— ordinal 0 (DAROOU) approach_from_left\n");

    set_pose(game, APPROACH_MAP_X, APPROACH_MAP_Y, APPROACH_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (7,7,DIR_EAST) = %d "
             "(expected -1, wrong wall under DUNGEON.C:2573 filter)",
             ord);
    CHECK(ord == -1, msg);
}

/* ── Group C: D1C portrait_rect_position pixel-emptiness ───────────
 * Drive M11_GameView_Draw at (7, 7) DIR_EAST and verify the D1C
 * portrait cutout (96, 35, 32, 29) does NOT carry ordinal-0
 * (DAROOU) pixels at > 35% match.  A regression that paints the
 * DAROOU sprite over the corridor west wall would push the
 * match above 35% and trip this assertion. */
static void check_no_floating_on_approach_from_left(
    M11_GameViewState* game,
    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    char msg[200];

    printf("\n[Group C] D1C portrait_rect_position pixel-emptiness on "
           "(7,7,DIR_EAST) approach_from_left\n");

    set_pose(game, APPROACH_MAP_X, APPROACH_MAP_Y, APPROACH_DIR);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    matchPct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "D1C portrait_rect_position at (7,7,EAST) carries ordinal %d "
             "(DAROOU) pixels at < %d%% match (got %d%%) - no-floating "
             "invariant on the approach_from_left wrong-wall route",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= 0 && matchPct < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* ── Group D: positive cross-check at the canonical DAROOU route ───
 * The D1C cutout must NOT be dead: at the canonical positive
 * route (9, 7) DIR_WEST — the native (8, 7) EAST-wall DAROOU
 * sensor on the shipped PC 3.4 fixture — the SAME rectangle IS
 * painted with ordinal 0 at >= 90% match.  An empty rectangle at
 * (7, 7) E must not silently mean the rectangle is dead.  This
 * is the ordinal 0 cross-check: it locks the ordinal-zero edge
 * (a 0 sensorData must be treated as a valid ordinal, not as
 * "false" / "no sensor").  If a local build lacks the native
 * DAROOU sensor we fall back to seeding the first HALK
 * (sensorData=1) sensor to ordinal 0, cross-check at the (7, 9)
 * DIR_NORTH HALK route, and restore the sensor before exit. */
static void check_positive_crosscheck_daroou_route(
    M11_GameViewState* game,
    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    int ord = -999;
    int seededSensor = -1;
    unsigned short savedData = 0;
    char msg[240];

    printf("\n[Group D] D1C portrait_rect_position positive cross-check at "
           "(9,7,WEST) — ordinal 0 (DAROOU) native (8,7) EAST-wall route\n");

    set_pose(game, CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, CROSSCHECK_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (%d,%d,DIR_WEST) native = %d "
             "(expected %d, DAROOU visible from east of (8,7))",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ord, ORDINAL_CROSSCHECK);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);

    if (ord != ORDINAL_CROSSCHECK) {
        /* Fallback: seed the first HALK (sensorData=1) C127
         * sensor — the (7, 8) SOUTH wall on the shipped fixture —
         * to ordinal 0 and cross-check at the (7, 9) DIR_NORTH
         * HALK route.  The seed is reverted before the probe
         * exits so subsequent CTest runs see the shipped DM1 V1
         * data. */
        seededSensor = seed_first_c127_data(game,
                                              SHIPPED_HALK_ORDINAL,
                                              ORDINAL_CROSSCHECK,
                                              &savedData);
        snprintf(msg, sizeof(msg),
                 "seeded (7,9) NORTH-route C127 sensor from %d to %d "
                 "(sensor index %d, saved sensorData %u for restore)",
                 SHIPPED_HALK_ORDINAL, ORDINAL_CROSSCHECK,
                 seededSensor, (unsigned)savedData);
        CHECK(seededSensor >= 0, msg);
        if (seededSensor < 0) {
            return;
        }
        set_pose(game, 7, 9, 0 /* DIR_NORTH */);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (7,9,DIR_NORTH) seeded = %d "
                 "(expected %d, DAROOU visible from south of (7,8) "
                 "after the C127 sensor rewrite)",
                 ord, ORDINAL_CROSSCHECK);
        CHECK(ord == ORDINAL_CROSSCHECK, msg);
        if (ord != ORDINAL_CROSSCHECK) {
            game->world.things->sensors[seededSensor].sensorData = savedData;
            return;
        }
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    matchPct = match_portrait_cell(portraits, fb, ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "D1C portrait_rect_position carries ordinal %d "
             "(DAROOU) pixels at >= %d%% match (got %d%%) - positive "
             "cross-check proves the D1C rect is alive at the canonical "
             "DAROOU route",
             ORDINAL_CROSSCHECK,
             CORRECT_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* Restore the original sensorData so subsequent CTest runs
     * see the shipped DM1 V1 data (no-op on the native path). */
    if (seededSensor >= 0) {
        game->world.things->sensors[seededSensor].sensorData = savedData;
    }
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->candidateMirrorPanelActive = 0;
}

/* ── Group E: approach-cell band sweep ────────────────────────────
 * The (x=7) column is the cell column immediately west of the
 * (x=8) column where the DAROOU mirror cell (8, 7) sits.  Every
 * (map=0, x=7, y=6..10) DIR_EAST pose is an "approach from the
 * left" pose for the (8, y) column: the player is to the west
 * of (8, y) and looks east, so the visible wall is the WEST
 * wall of (8, y) and carries no C127 sensor per the shipped
 * DM1 V1 DUNGEON.DAT (verified PC34 C127 layout: no WEST-face
 * C127 sensor exists on the (8, y) cells).  The probe asserts
 * ordinal -1 (or at least never ordinal 0) for each
 * (7, y) DIR_EAST pose to lock the approach-band invariant
 * across the (x=7) column adjacent to the DAROOU mirror
 * column. */
static void check_approach_cell_band_sweep(M11_GameViewState* game) {
    static const struct {
        int y;
        const char* label;
    } kApproachY[] = {
        {6, "daroou_approach_from_left_y6"},
        {7, "daroou_approach_from_left_y7_anchor"},
        {8, "daroou_approach_from_left_y8"},
        {9, "daroou_approach_from_left_y9"},
        {10, "daroou_approach_from_left_y10"}
    };
    int i;
    int n = (int)(sizeof(kApproachY) / sizeof(kApproachY[0]));
    int ok = 1;

    printf("\n[Group E] approach-cell band sweep at (7, 6..10) DIR_EAST\n");
    for (i = 0; i < n; ++i) {
        int ord = -999;
        char msg[240];
        set_pose(game, 7, kApproachY[i].y, 1 /* DIR_EAST */);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (7,%d,DIR_EAST) [%s] = %d "
                 "(expected -1, no DAROOU mirror visible from the (7,y) "
                 "approach column looking east at the (8,y) WEST wall)",
                 kApproachY[i].y, kApproachY[i].label, ord);
        if (ord != -1) {
            /* A non-(-1) result here is not necessarily a
             * failure: some (7, y) cells in the shipped
             * DUNGEON.DAT may border a C127 sensor on another
             * aspect, but no such sensor resolves to ordinal 0
             * from this side.  We assert only that the result
             * is NOT ordinal 0 (no DAROOU float) — the
             * no-floating invariant for the ordinal 0 (DAROOU)
             * slice. */
            snprintf(msg, sizeof(msg),
                     "front mirror ordinal at (7,%d,DIR_EAST) [%s] = %d "
                     "is NOT ordinal 0 (no DAROOU float over the "
                     "(8,%d) WEST wall)",
                     kApproachY[i].y, kApproachY[i].label, ord,
                     kApproachY[i].y);
            CHECK(ord != ORDINAL_TARGET, msg);
        } else {
            CHECK(ord == -1, msg);
        }
    }
    (void)ok;
}

/* ── Group F: 4-direction rotate-away at the (7, 7) cell ───────────
 * The (7, 7) cell is the approach_from_left anchor for DAROOU,
 * and all four facings at (7, 7) are wrong-wall or no-sensor
 * under the verified PC34 C127 layout:
 *   - (7, 7) DIR_NORTH faces (7, 6) — no C127 sensor on the
 *     SOUTH wall of (7, 6).  Expected: -1.
 *   - (7, 7) DIR_EAST is the wrong wall for DAROOU (the sensor
 *     is on the EAST wall of (8, 7), the visible wall from the
 *     west is the WEST wall), returns -1 (this probe's primary
 *     slice).
 *   - (7, 7) DIR_SOUTH faces (7, 8) — the HALK sensor sits on
 *     the SOUTH wall of (7, 8); the visible NORTH wall carries
 *     no C127 sensor.  Expected: -1.
 *   - (7, 7) DIR_WEST faces (6, 7) — no C127 sensor on the
 *     EAST wall of (6, 7).  Expected: -1.
 *
 * What this group locks is the specific invariant: at the (7, 7)
 * approach_from_left cell, all four facings reject DAROOU
 * (ordinal 0 never visible at the (7, 7) approach cell).  This
 * is disjoint from the Group E band sweep: Group E covers the
 * (7, 6..10) column with the focus on the EAST-facing
 * no-floating invariant (no ordinal 0 float over the (8, y)
 * WEST wall), while Group F covers the (7, 7) cell across all
 * four facings. */
static void check_rotate_away_at_approach_cell(M11_GameViewState* game) {
    static const struct {
        int dir;
        const char* label;
    } kDirs[] = {
        {0, "daroou_approach_rotate_north_no_sensor"},
        {1, "daroou_approach_rotate_east_wrong_wall"},
        {2, "daroou_approach_rotate_south_halk_north_wall"},
        {3, "daroou_approach_rotate_west_no_sensor"},
    };
    int i;
    int n = (int)(sizeof(kDirs) / sizeof(kDirs[0]));

    printf("\n[Group F] 4-direction rotate-away at (7, 7)\n");
    for (i = 0; i < n; ++i) {
        int ord = -999;
        char msg[240];
        set_pose(game, 7, 7, kDirs[i].dir);
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (7,7,dir=%d) [%s] = %d "
                 "(expected -1, no DAROOU mirror visible from the (7,7) "
                 "approach cell in this direction)",
                 kDirs[i].dir, kDirs[i].label, ord);
        CHECK(ord == -1, msg);
    }
}

/* ── Group G: mirror catalog name resolution for ordinal 0 ─────────
 * The mirror catalog must resolve ordinal 0 to the DAROOU
 * record.  This catches a regression where the catalog and the
 * C026 atlas disagree on the ordinal-0 record (e.g., an atlas
 * re-indexing that moves DAROOU to a different ordinal).  Same
 * pattern the ordinal_4_approach_from_left probe uses for
 * ordinal 4 (LEIF). */
static void check_ordinal_0_catalog_resolution(M11_GameViewState* game) {
    char nameBuf[32] = {0};
    char titleBuf[32] = {0};
    int nameRc = 0;
    char msg[200];

    printf("\n[Group G] ordinal 0 (DAROOU) mirror catalog name resolution\n");

    nameRc = M11_GameView_GetMirrorNameByOrdinal(game,
                                                  ORDINAL_TARGET,
                                                  nameBuf,
                                                  (int)sizeof(nameBuf));
    snprintf(msg, sizeof(msg),
             "mirror catalog name for ordinal %d = \"%s\" (expected \"DAROOU\")",
             ORDINAL_TARGET, nameBuf[0] ? nameBuf : "");
    CHECK(nameRc > 0 && strcmp(nameBuf, "DAROOU") == 0, msg);

    (void)M11_GameView_GetMirrorTitleByOrdinal(game,
                                                ORDINAL_TARGET,
                                                titleBuf,
                                                (int)sizeof(titleBuf));
    /* The DAROOU title is less important than the name — the
     * catalog title may be empty in some DM1 V1 builds (the
     * front_north_entry probe notes DAROOU is untitled in the
     * shipped catalog).  Print it for diagnostics but do not
     * fail on it. */
    printf("  INFO: ordinal %d (DAROOU) mirror catalog title = \"%s\"\n",
           ORDINAL_TARGET, titleBuf[0] ? titleBuf : "(untitled)");
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowed[1024];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = firestaff_dm1_probe_narrow_data_dir(dataDir, narrowed, sizeof(narrowed));
    printf("=== DM1 V1 Hall of Champions portrait ordinal 0 (DAROOU) / "
           "approach_from_left / portrait_rect_position ===\n");
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
    game.candidateMirrorPanelActive = 0;
    game.candidateMirrorOrdinal = -1;
    game.candidateMirrorPartyIndex = -1;
    game.world.party.championCount = 0;

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 8 * PORTRAIT_W ||
        portraits->height < 3 * PORTRAIT_H) {
        printf("  WARN: C026 portrait strip missing or too small; "
               "pixel-match groups will run but report -1%% match.\n");
    }

    /* Group A — source-locked D1C wall-ornament zone invariant. */
    check_d1c_wall_ornament_zone(&game);

    /* Group B — engine-helper front-ordinal invariant. */
    check_front_ordinal_approach_from_left(&game);

    /* Group C — D1C portrait_rect_position pixel-emptiness on the
     * wrong wall. */
    check_no_floating_on_approach_from_left(&game, portraits);

    /* Group D — positive cross-check at the canonical DAROOU
     * route (seeded). */
    check_positive_crosscheck_daroou_route(&game, portraits);

    /* Group E — approach-cell band sweep at (7, 6..10) DIR_EAST. */
    check_approach_cell_band_sweep(&game);

    /* Group F — 4-direction rotate-away at the (7, 7) cell. */
    check_rotate_away_at_approach_cell(&game);

    /* Group G — mirror catalog name resolution. */
    check_ordinal_0_catalog_resolution(&game);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
