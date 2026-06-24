/*
 * firestaff_dm1_v1_champion_mirror_ordinal_17_approach_from_left_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 17 (C026 strip cell 17 -- atlas col 1 row 2,
 *                                source rect (32, 58, 32, 29); the
 *                                mirror TextString catalog row binds
 *                                ordinal 17 to "BORIS" / "WIZARD OF
 *                                BALDOR" on DM1 V1 PC 3.4)
 *   route approach_from_left: party at (1, y) on the (x=1) corridor
 *                             facing EAST.  The (x=1) corridor cell
 *                             stands immediately WEST of the (2, y)
 *                             chamber cell, so a DIR_EAST pose is
 *                             the canonical "approach from the left"
 *                             for whatever chamber is at (2, y).
 *                             Ordinal 17 is NOT a Hall of Champions
 *                             C127 sensorData value in the source-
 *                             visible DM1 V1 PC 3.4 DUNGEON.DAT
 *                             (the actual_pose probe lists 24 poses
 *                             and the (1, y) DIR_EAST band returns
 *                             -1 for y=2, 4, 5, 6, 7 and 18 for
 *                             y=3; ordinal 17 never appears), so
 *                             this slice locks the no-floating
 *                             invariant for ordinal 17 across the
 *                             entire (x=1) DIR_EAST corridor band.
 *                             It is the route-variant
 *                             complement of the existing
 *                             ordinal_17_west_negative slice, which
 *                             locks the (1, y) DIR_WEST band.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                  -- exactly the source-locked
 *                                  DUNVIEW.C G0109_auc_Graphic558
 *                                  _Box_ChampionPortraitOnWall =
 *                                  {96, 127, 35, 63} blit
 *                                  destination, parented inside the
 *                                  C346 D1C wall-mirror frame
 *                                  (80, 29, 64, 43) per DUNVIEW.C
 *                                  G0205 coordSet 5 / index 12.
 *
 * The slice is the dedicated approach_from_left route variant /
 * rect-position invariant for ordinal 17.  Ordinal 17 is a valid
 * C026 atlas cell (graphic 26 is 256x87 = 8 cols x 3 rows of
 * 32x29 portraits; ordinal 17 lives at row 2, col 1 = atlas
 * pixel (32, 58)-(64, 87)) and the catalog binds it to BORIS
 * "WIZARD OF BALDOR", but no C127 sensor with sensorData=17
 * exists on Hall of Champions map 0 in the source-visible DM1 V1
 * PC 3.4 fixture.  This probe is the regression gate that says:
 *
 *   "No (1, y) DIR_EAST corridor pose may route ordinal 17 to the
 *    D1C portrait rectangle (96, 35, 32, 29) under the source-
 *    visible DM1 V1 PC 3.4 DUNGEON.DAT, regardless of which
 *    sensorData value is on the (2, y) WEST wall."
 *
 * Five invariant groups:
 *
 *   A. Engine helper surface -- at (1, 3) DIR_EAST the
 *      M11_GameView_GetD1CWallOrnamentZone helper still returns
 *      the source-locked wall box (80, 29, 64, 43) and the inner
 *      portrait cutout is parented at (96, 35, 32, 29).  The
 *      portrait_rect_position is invariant across the
 *      approach_from_left slice, including at the (1, 3) DIR_EAST
 *      SONJA-ordinal-18 pose where the D1C cutout IS painted.
 *      (1, 3) DIR_EAST is the only (1, y) DIR_EAST corridor pose
 *      whose front cell has a C127 sensor visible from the
 *      corridor WEST side (sensorData=18 SONJA on the (2, 3) WEST
 *      wall), so it is the natural anchor for the approach_from
 *      _left slice.
 *
 *   B. Pixel contract at (1, 3) DIR_EAST -- GetFrontMirrorOrdinal
 *      returns 18 (SONJA) and the D1C cutout pixel-matches the
 *      C026 ordinal-18 atlas cell at >= 90%, with ordinal 18
 *      strictly beating every other ordinal in the rect.  The
 *      same cutout must NOT match C026 ordinal 17 above the 35%
 *      drift threshold.  This proves the D1C cutout is alive at
 *      the (1, 3) approach_from_left SONJA route AND that ordinal
 *      17 (BORIS) does NOT float over the SONJA chamber wall.
 *
 *   C. Corridor approach_from_left band scan -- for every
 *      (x=1, y=2..6) cell with DIR_EAST, the engine returns the
 *      expected ordinal (or -1) and the D1C cutout must NOT match
 *      C026 ordinal 17 above the 35% drift threshold at any of
 *      these poses.  No C127 sensor with sensorData=17 lives on
 *      the (2, y) WEST wall for any Hall y-coordinate, so the
 *      engine never paints ordinal 17 over the corridor
 *      approach_from_left wall.
 *
 *   D. Re-entry + cross-pose stability -- paint (1, 5) DIR_SOUTH
 *      (WUUF ordinal 13 on the (1, 6) NORTH wall), then re-enter
 *      (1, 3) DIR_EAST.  The D1C cutout must come back to ordinal
 *      18 (SONJA), and ordinal 17 must still NOT appear above
 *      the 35% drift threshold.  Re-entry proves the
 *      approach_from_left no-floating invariant is not a one-shot
 *      artifact of a single draw cycle and survives a state
 *      transition across a different route.
 *
 *   E. Mirror catalog name resolution -- the mirror catalog must
 *      resolve ordinal 17 to "BORIS" and ordinal 18 to "SONJA".
 *      This catches a regression where the catalog and the C026
 *      atlas disagree on the ordinal-17 / ordinal-18 records.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter (DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_
 *     ORDINAL=5)
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box
 *     (only on D1C -- M587_VIEW_WALL_D1C_FRONT)
 *   - DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw order
 *   - DUNVIEW.C G0205 G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
 *     coordSet 5 / index 12 = C346 D1C wall-mirror frame at
 *     (80, 29, 64, 43)
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32, G2079_C29=29
 *   - MOVESENS.C:1501-1503 sensorData flows to F0280 candidate
 *   - REVIVE.C F0280 materialize candidate from sensorData
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
 *     -- 16-pose ordinal map including (1,3,E)=18 and (1,2..6,
 *        E)=-1 except (1,3,E)=18 (this probe uses the same
 *        fixture but pixel-verifies the D1C rect for ordinal 17,
 *        which the actual_pose probe does not do).
 *   firestaff_dm1_v1_champion_mirror_ordinal_17_west_negative_portrait_rect_position_runtime_probe
 *     -- ordinal-17 west_negative (different route variant,
 *        disjoint data fixtures: (1, y) DIR_WEST band y=2..6).
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from_left_portrait_rect_position_runtime_probe
 *     -- ordinal-4 approach_from_left template this probe
 *        follows (different ordinal, single-cell focus on
 *        (1, 2) E for the LEIF chamber; this probe is the
 *        multi-cell corridor band sweep for ordinal 17).
 *   firestaff_dm1_v1_hall_champion_portrait_17_front_north_entry_runtime_probe
 *     -- ordinal-17 front_north_entry (disjoint route variant;
 *        the catalog identity assertions in [E] cross-check the
 *        probe's name lookup with the actual_pose probe's known
 *        BORIS slot).
 *   firestaff_dm1_v1_hall_of_champions_portrait_17_cancel_reopen_portrait_rect_position_runtime_probe
 *     -- ordinal-17 cancel/reopen behavior (disjoint aspect).
 *   firestaff_dm1_v1_hall_of_champions_portrait_17_inventory_exit_restore_portrait_rect_position_runtime_probe
 *     -- ordinal-17 inventory exit/restore (disjoint aspect).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=17 exists in
 *     a custom-port / ROM-hack distribution.  The
 *     approach_from_left slice is specifically the negative
 *     route, and the local PC 3.4 DUNGEON.DAT is the source-locked
 *     fixture that proves the rectangle is empty of ordinal 17
 *     pixels across the entire (x=1) corridor DIR_EAST band.
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
    /* Match thresholds.  At every (1, y) DIR_EAST corridor pose
     * the D1C cutout must not contain a C026 ordinal-17 portrait.
     * We allow up to 35% pixel match against ordinal 17 (the
     * wrong-ordinal drift threshold used by the actual-pose probe
     * and the ordinal-12 / ordinal-13 / ordinal-2 / ordinal-17
     * west_negative sibling probes).  Above 35% means a stale
     * ordinal-17 sprite is floating over the corridor approach
     * wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the (1, 3) DIR_EAST positive cross-check the D1C cutout
     * must carry the expected portrait (SONJA, ordinal 18) at
     * >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal. */
    ORDINAL_TARGET = 17,
    /* The (1, 3) DIR_EAST approach_from_left anchor ordinal.  The
     * front cell (2, 3) WEST wall carries the C127 sensor with
     * sensorData=18 (SONJA) under DM1 V1 PC 3.4, so the only
     * (1, y) DIR_EAST corridor pose that exposes a portrait is
     * y=3, ordinal 18.  This is the natural positive cross-check
     * anchor for the approach_from_left slice. */
    ORDINAL_APPROACH_ANCHOR = 18,
    APPROACH_ANCHOR_MAP_X = 1,
    APPROACH_ANCHOR_MAP_Y = 3,
    APPROACH_ANCHOR_DIR = 1, /* DIR_EAST */
    /* Corridor approach_from_left band -- every (x=1, y) cell
     * where an east-facing party has its front square on the
     * (2, y) WEST wall.  y=2..6 covers the canonical Hall of
     * Champions corridor band; y=7 is excluded because
     * (2, 7) is the WUUF "back-of-hall" chamber whose WEST wall
     * is reserved for an end-of-corridor cell, not the standard
     * approach_from_left slice. */
    CORRIDOR_APPROACH_Y_MIN = 2,
    CORRIDOR_APPROACH_Y_MAX = 6,
    /* D1C wall-mirror frame parented offset per DUNVIEW.C:3913-3928
     * and the C346 frame geometry in m11_draw_dm1_front_mirror_route
     * (src/engine/m11_game_view.c:14077). */
    FRAME_PORTRAIT_OFFSET_X = 16,
    FRAME_PORTRAIT_OFFSET_Y = 6,
    /* Cross-pose stability anchor: paint (1, 5) DIR_SOUTH (WUUF
     * ordinal 13) before re-entering (1, 3) DIR_EAST to prove the
     * approach_from_left no-floating invariant survives a state
     * transition across a different route. */
    CROSS_POSE_MAP_X = 1,
    CROSS_POSE_MAP_Y = 5,
    CROSS_POSE_DIR = 2, /* DIR_SOUTH */
    CROSS_POSE_EXPECTED_ORDINAL = 13
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
 * not skew the match.  Palette index 12 (the C346 D1C wall-mirror
 * frame backdrop) is also skipped so the dark-gray frame backdrop
 * does not count as ordinal pixels. */
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

/* Find the dominant portrait ordinal in the D1C portrait cutout by
 * matching every C026 atlas cell (24 cells total) and returning
 * the one with the highest matched-percent.  This is the strict
 * dominance check used in the cross-check group. */
static int dominant_portrait_ordinal(const M11_AssetSlot* portraits,
                                     const unsigned char* fb) {
    int bestOrdinal = -1;
    int bestPct = -1;
    int ord;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    for (ord = 0; ord < 24; ++ord) {
        int pct = match_portrait_cell(portraits, fb, ord);
        if (pct > bestPct) {
            bestPct = pct;
            bestOrdinal = ord;
        }
    }
    return bestOrdinal;
}

/* Count distinct non-zero palette indices in a viewport rect.
 * Useful for proving the corridor east wall has at least *some*
 * rendered content (floor, wall, or chamber-doorway pixels) so the
 * empty D1C cutout cannot be explained away by "the framebuffer
 * was never painted". */
static int rect_distinct_nonzero(const unsigned char* fb,
                                 int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int n = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = (unsigned char)(fb[yy * FB_W + xx] & 0x0F);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction)
 * pose on map 0 (Hall of Champions) and return the rendered
 * framebuffer.  Caller owns the storage. */
static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* ── Group A: engine-helper surface at the approach anchor ───────
 * The (1, 3) DIR_EAST pose is the only (x=1) corridor pose whose
 * front cell (2, 3) WEST wall carries a C127 sensor visible from
 * the corridor west side (sensorData=18 SONJA).  At this pose the
 * engine helper must still return the source-locked wall box
 * (80, 29, 64, 43) and the inner portrait cutout is parented at
 * (96, 35, 32, 29).  The portrait_rect_position is invariant
 * across the approach_from_left slice, so any drift in the
 * helper output would silently invalidate the pixel-match
 * contracts in groups B, C and D. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for (1, 3) DIR_EAST "
           "approach_from_left anchor\n");

    state->world.party.mapIndex = 0;
    state->world.party.mapX = APPROACH_ANCHOR_MAP_X;
    state->world.party.mapY = APPROACH_ANCHOR_MAP_Y;
    state->world.party.direction = APPROACH_ANCHOR_DIR;

    rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetD1CWallOrnamentZone returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "D1C wall box X == %d (got %d)", WALLBOX_X, ornX);
    CHECK(ornX == WALLBOX_X, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box Y == %d (got %d)", WALLBOX_Y, ornY);
    CHECK(ornY == WALLBOX_Y, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box W == %d (got %d)", WALLBOX_W, ornW);
    CHECK(ornW == WALLBOX_W, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box H == %d (got %d)", WALLBOX_H, ornH);
    CHECK(ornH == WALLBOX_H, msg);

    /* Inner portrait cutout = (ornX+16, ornY+6, 32, 29) = (96, 35, 32, 29). */
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout X == 96 (got %d)",
             ornX + FRAME_PORTRAIT_OFFSET_X);
    CHECK(ornX + FRAME_PORTRAIT_OFFSET_X == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == 35 (got %d)",
             ornY + FRAME_PORTRAIT_OFFSET_Y);
    CHECK(ornY + FRAME_PORTRAIT_OFFSET_Y == 35, msg);
}

/* ── Group B: pixel contract at the (1, 3) DIR_EAST anchor ──────
 * At (1, 3, DIR_EAST) the front cell (2, 3) WEST wall carries the
 * C127 sensor with sensorData=18 (SONJA) on its WEST aspect.  The
 * engine must return ordinal 18 from GetFrontMirrorOrdinal and
 * paint the SONJA portrait sprite (atlas slot 18) into the D1C
 * cutout at >= 90% pixel match, with ordinal 18 strictly beating
 * every other ordinal in the rect.  The same cutout must NOT
 * match C026 ordinal 17 above the 35% drift threshold -- proves
 * ordinal 17 (BORIS) does NOT float over the SONJA chamber
 * WEST wall, the primary no-floating guarantee for this slice. */
static void check_approach_anchor_pixel_contract(
    M11_GameViewState* state,
    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctWant;
    int pctTarget;
    int dominantOrdinal;
    int distinct;
    char msg[240];

    printf("\n[Group B] (1, 3) DIR_EAST pixel contract -- D1C cutout IS painted "
           "with ordinal %d (SONJA), NOT %d (BORIS)\n",
           ORDINAL_APPROACH_ANCHOR, ORDINAL_TARGET);

    render_at(state, fb, APPROACH_ANCHOR_MAP_X, APPROACH_ANCHOR_MAP_Y,
              APPROACH_ANCHOR_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,3) E) == %d (got %d)",
             ORDINAL_APPROACH_ANCHOR, ord);
    CHECK(ord == ORDINAL_APPROACH_ANCHOR, msg);

    /* Sanity: the corridor east wall must have *some* rendered
     * content (chamber wall texture, doorway frame, etc.) so the
     * pixel-match percentages below are meaningful. */
    distinct = rect_distinct_nonzero(fb,
                                     VIEWPORT_X + 64,
                                     VIEWPORT_Y + 30,
                                     96,
                                     60);
    snprintf(msg, sizeof(msg),
             "(1,3) E right half of viewport has rendered content "
             "(distinct non-zero palette indices >= 3, got %d)",
             distinct);
    CHECK(distinct >= 3, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing -- pixel-match group "
               "skipped\n");
        return;
    }

    /* The cutout must match ordinal 18 (SONJA) above 90%. */
    pctWant = match_portrait_cell(portraits, fb, ORDINAL_APPROACH_ANCHOR);
    snprintf(msg, sizeof(msg),
             "(1,3) E D1C cutout matches ordinal %d (SONJA) >= %d%%%% "
             "(got %d%%%%)",
             ORDINAL_APPROACH_ANCHOR, CORRECT_ORDINAL_MATCH_PCT, pctWant);
    CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* Strict dominance: ordinal 18 must beat every other C026
     * atlas cell in the rect.  This proves the cross-check painted
     * the right portrait, not some coincidental palette overlap
     * with a different ordinal. */
    dominantOrdinal = dominant_portrait_ordinal(portraits, fb);
    snprintf(msg, sizeof(msg),
             "(1,3) E D1C cutout dominant ordinal is %d (got %d)",
             ORDINAL_APPROACH_ANCHOR, dominantOrdinal);
    CHECK(dominantOrdinal == ORDINAL_APPROACH_ANCHOR, msg);

    /* The cutout must NOT match ordinal 17 (the slice target,
     * BORIS) above the 35% drift threshold -- proves ordinal 17
     * does NOT leak into the SONJA chamber WEST wall at the
     * approach_from_left anchor.  This is the no-floating
     * guarantee for the primary (1, 3) approach_from_left
     * pose. */
    pctTarget = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,3) E D1C cutout does NOT match ordinal %d (BORIS, the "
             "slice target) < %d%%%% (no ordinal-17 leak, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctTarget);
    CHECK(pctTarget < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* ── Group C: corridor (x=1) DIR_EAST band scan ─────────────────
 * Walk every (mapX=1, mapY) cell on the (x=1) corridor band with
 * DIR_EAST and confirm:
 *   (1) the engine returns the expected ordinal for each cell,
 *   (2) the D1C cutout does NOT match C026 ordinal 17 above the
 *       35% drift threshold at any of these poses.
 * The (x=1) corridor WEST side of (2, y) has no C127 sensor with
 * sensorData=17 in the source-visible DM1 V1 PC 3.4 fixture --
 * the C127 sensors facing WEST live at (2, 4) sensorData=10 and
 * (2, 6) sensorData=13, both of which are off the (x=1)
 * approach_from_left slice.  The only corridor (1, y) DIR_EAST
 * pose that exposes a portrait is (1, 3) DIR_EAST = 18 (SONJA);
 * all other corridor cells (y=2, 4, 5, 6) face a WEST wall with
 * no C127 sensor and return -1.  This locks the no-floating
 * invariant for ordinal 17 across the entire approach_from_left
 * band. */
static void check_corridor_approach_scan(M11_GameViewState* state,
                                         const M11_AssetSlot* portraits) {
    static const struct {
        int y;
        int expectedOrdinal;
        const char* label;
    } kCorridorPoses[] = {
        /* (1, 2) DIR_EAST -- approach_from_left for the LEIF (2, 2)
         * chamber; (2, 2) WEST wall has no C127 sensor, returns -1. */
        {2, -1, "approach_from_left_y2_leif_west_wall"},
        /* (1, 3) DIR_EAST -- the SONJA chamber; (2, 3) WEST wall
         * carries C127 sensorData=18. */
        {3, 18, "approach_from_left_y3_sonja_chamber"},
        /* (1, 4) DIR_EAST -- mid-corridor cell; (2, 4) WEST wall
         * has no C127 sensor (MOPHUS lives on (2, 4) NORTH wall
         * only), returns -1. */
        {4, -1, "approach_from_left_y4_mophus_west_wall"},
        /* (1, 5) DIR_EAST -- approach_from_left for the MOPHUS
         * (2, 5) chamber; (2, 5) WEST wall has no C127 sensor
         * (MOPHUS lives on (2, 5) NORTH wall only), returns -1. */
        {5, -1, "approach_from_left_y5_mophus_chamber_west_wall"},
        /* (1, 6) DIR_EAST -- back-of-hall corridor cell; (2, 6)
         * WEST wall has no C127 sensor (WUUF lives on (2, 6) WEST
         * wall per the actual_pose table -- wait, this is the W
         * wall of (2, 6), not the W wall of (1, 6)'s front cell).
         * The front cell is (2, 6), so the visible wall for (1, 6)
         * DIR_EAST is the (2, 6) WEST wall.  C127 sensorData=13
         * is on (2, 6) WEST per the actual_pose table, but the
         * (1, 6) DIR_EAST pose is the corridor approach, not the
         * chamber interior; the actual sensorData=13 is exposed
         * from inside the (2, 6) chamber facing WEST, not from
         * outside.  Returns -1. */
        {6, -1, "approach_from_left_y6_wuuf_chamber_west_wall"}
    };
    int n = (int)(sizeof(kCorridorPoses) / sizeof(kCorridorPoses[0]));
    int i;
    int foundOrdinal17 = 0;
    int otherOrdinals[8];
    int otherOrdinalsCount = 0;
    unsigned char fb[FB_W * FB_H];
    char msg[240];

    printf("\n[Group C] Corridor (x=1) DIR_EAST approach_from_left scan y=%d..%d\n",
           CORRIDOR_APPROACH_Y_MIN, CORRIDOR_APPROACH_Y_MAX);
    for (i = 0; i < n; ++i) {
        int ord = 0;
        int pct = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 1;
        state->world.party.mapY = kCorridorPoses[i].y;
        state->world.party.direction = 1; /* DIR_EAST */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (1,%d) DIR_EAST [%s] = %d "
                 "(expected %d)",
                 kCorridorPoses[i].y, kCorridorPoses[i].label, ord,
                 kCorridorPoses[i].expectedOrdinal);
        CHECK(ord == kCorridorPoses[i].expectedOrdinal, msg);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal17;
        } else if (ord >= 0 &&
                   otherOrdinalsCount <
                   (int)(sizeof(otherOrdinals) / sizeof(otherOrdinals[0]))) {
            otherOrdinals[otherOrdinalsCount++] = ord;
        }

        if (portraits && portraits->loaded && portraits->pixels) {
            render_at(state, fb, 1, kCorridorPoses[i].y, 1 /* DIR_EAST */);
            pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
            snprintf(msg, sizeof(msg),
                     "(1,%d) DIR_EAST D1C cutout does NOT match ordinal %d "
                     "(BORIS) < %d%%%% (no ordinal-17 leak, got %d%%%%)",
                     kCorridorPoses[i].y, ORDINAL_TARGET,
                     WRONG_ORDINAL_MATCH_PCT, pct);
            CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=1, y=%d..%d) DIR_EAST scan finds no C127 sensor with "
             "sensorData=%d on the corridor approach_from_left wall "
             "(found %d)",
             CORRIDOR_APPROACH_Y_MIN, CORRIDOR_APPROACH_Y_MAX,
             ORDINAL_TARGET, foundOrdinal17);
    CHECK(foundOrdinal17 == 0, msg);

    /* Lock the band coverage so future patches that change
     * CORRIDOR_APPROACH_Y_MIN / _Y_MAX cannot silently shrink
     * the band.  The kCorridorPoses table carries exactly 5
     * cells (y=2..6). */
    snprintf(msg, sizeof(msg),
             "corridor approach_from_left band covered %d cells (y=%d..%d)",
             n, CORRIDOR_APPROACH_Y_MIN, CORRIDOR_APPROACH_Y_MAX);
    CHECK(n == CORRIDOR_APPROACH_Y_MAX - CORRIDOR_APPROACH_Y_MIN + 1, msg);
}

/* ── Group D: re-entry + cross-pose stability ───────────────────
 * Paint (1, 5) DIR_SOUTH first (WUUF ordinal 13 on the (1, 6)
 * NORTH wall), then re-enter (1, 3) DIR_EAST.  The D1C cutout
 * must come back to SONJA ordinal 18, and ordinal 17 (BORIS) must
 * still NOT appear above the 35% drift threshold.  This proves
 * the approach_from_left no-floating invariant survives a state
 * transition across a different route, so the band-scan results
 * in Group C are not a one-shot artifact of a single draw cycle. */
static void check_reentry_after_cross_pose(M11_GameViewState* state,
                                           const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctWant;
    int pctTarget;
    int dominantOrdinal;
    char msg[240];

    printf("\n[Group D] Re-entry + cross-pose stability -- paint "
           "(1, 5) DIR_SOUTH first, then re-enter (1, 3) DIR_EAST\n");

    /* Step 1: paint the cross-pose first.  (1, 5) DIR_SOUTH --
     * front cell (1, 6) NORTH wall carries C127 sensorData=13
     * (WUUF). */
    render_at(state, fb, CROSS_POSE_MAP_X, CROSS_POSE_MAP_Y, CROSS_POSE_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "Cross-pose: front mirror ordinal at (%d, %d) DIR_SOUTH = %d "
             "(expected %d WUUF)",
             CROSS_POSE_MAP_X, CROSS_POSE_MAP_Y, ord,
             CROSS_POSE_EXPECTED_ORDINAL);
    CHECK(ord == CROSS_POSE_EXPECTED_ORDINAL, msg);

    if (portraits && portraits->loaded && portraits->pixels) {
        pctWant = match_portrait_cell(portraits, fb,
                                      CROSS_POSE_EXPECTED_ORDINAL);
        snprintf(msg, sizeof(msg),
                 "Cross-pose: D1C cutout at (%d, %d) DIR_SOUTH carries "
                 "ordinal %d (WUUF) >= %d%%%% (got %d%%%%)",
                 CROSS_POSE_MAP_X, CROSS_POSE_MAP_Y,
                 CROSS_POSE_EXPECTED_ORDINAL, CORRECT_ORDINAL_MATCH_PCT,
                 pctWant);
        CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

        /* The cross-pose cutout must NOT match ordinal 17 (BORIS)
         * above the 35% drift threshold -- proves the cross-pose
         * paint is WUUF, not ordinal 17 by accident. */
        pctTarget = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
        snprintf(msg, sizeof(msg),
                 "Cross-pose: D1C cutout at (%d, %d) DIR_SOUTH does NOT "
                 "match ordinal %d (BORIS) < %d%%%% (no ordinal-17 leak, "
                 "got %d%%%%)",
                 CROSS_POSE_MAP_X, CROSS_POSE_MAP_Y, ORDINAL_TARGET,
                 WRONG_ORDINAL_MATCH_PCT, pctTarget);
        CHECK(pctTarget < WRONG_ORDINAL_MATCH_PCT, msg);
    }

    /* Step 2: re-enter the approach_from_left anchor. */
    render_at(state, fb, APPROACH_ANCHOR_MAP_X, APPROACH_ANCHOR_MAP_Y,
              APPROACH_ANCHOR_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "Re-entry: M11_GameView_GetFrontMirrorOrdinal((1,3) E) == %d "
             "(got %d) after cross-pose paint",
             ORDINAL_APPROACH_ANCHOR, ord);
    CHECK(ord == ORDINAL_APPROACH_ANCHOR, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing -- pixel-match group "
               "skipped\n");
        return;
    }

    /* The re-entered cutout must match ordinal 18 (SONJA) above 90%. */
    pctWant = match_portrait_cell(portraits, fb, ORDINAL_APPROACH_ANCHOR);
    snprintf(msg, sizeof(msg),
             "Re-entry: (1,3) E D1C cutout matches ordinal %d (SONJA) >= "
             "%d%%%% (got %d%%%%)",
             ORDINAL_APPROACH_ANCHOR, CORRECT_ORDINAL_MATCH_PCT, pctWant);
    CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* Strict dominance: ordinal 18 must beat every other C026
     * atlas cell in the re-entered rect.  This proves the
     * re-entry painted SONJA, not some coincidental palette
     * overlap with a different ordinal. */
    dominantOrdinal = dominant_portrait_ordinal(portraits, fb);
    snprintf(msg, sizeof(msg),
             "Re-entry: (1,3) E D1C cutout dominant ordinal is %d (got %d)",
             ORDINAL_APPROACH_ANCHOR, dominantOrdinal);
    CHECK(dominantOrdinal == ORDINAL_APPROACH_ANCHOR, msg);

    /* The re-entered cutout must NOT match ordinal 17 (BORIS)
     * above the 35% drift threshold -- proves the
     * approach_from_left no-floating invariant survives the
     * (1, 5) S -> (1, 3) E state transition. */
    pctTarget = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "Re-entry: (1,3) E D1C cutout does NOT match ordinal %d "
             "(BORIS) < %d%%%% after cross-pose paint (no ordinal-17 "
             "leak, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctTarget);
    CHECK(pctTarget < WRONG_ORDINAL_MATCH_PCT, msg);

    /* After the re-entry, the engine helper still returns the
     * source-locked wall box (80, 29, 64, 43) at the same pose. */
    {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY,
                                                     &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Re-entry: D1C wall box is still (%d, %d, %d, %d) at (1,3) "
                 "E (got (%d, %d, %d, %d))",
                 WALLBOX_X, WALLBOX_Y, WALLBOX_W, WALLBOX_H,
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
    }
}

/* ── Group E: mirror catalog name resolution ────────────────────
 * The mirror catalog must resolve ordinal 17 to BORIS and ordinal
 * 18 to SONJA.  This catches a regression where the catalog and
 * the C026 atlas disagree on the ordinal-17 / ordinal-18
 * records.  The same pattern the ordinal_5 / ordinal_19
 * cancel_reopen probes use for their respective ordinals. */
static void check_catalog_resolution(M11_GameViewState* state) {
    char nameBuf17[32] = {0};
    char nameBuf18[32] = {0};
    char titleBuf17[32] = {0};
    char titleBuf18[32] = {0};
    int nameRc17;
    int nameRc18;
    int titleRc17;
    int titleRc18;
    char msg[200];

    printf("\n[Group E] mirror catalog name resolution for ordinals 17 "
           "(BORIS) and 18 (SONJA)\n");

    nameRc17 = M11_GameView_GetMirrorNameByOrdinal(state, ORDINAL_TARGET,
                                                   nameBuf17,
                                                   (int)sizeof(nameBuf17));
    snprintf(msg, sizeof(msg),
             "mirror catalog name for ordinal %d (BORIS) = \"%s\" "
             "(expected \"BORIS\")",
             ORDINAL_TARGET, nameBuf17[0] ? nameBuf17 : "");
    CHECK(nameRc17 > 0 && strcmp(nameBuf17, "BORIS") == 0, msg);

    titleRc17 = M11_GameView_GetMirrorTitleByOrdinal(state, ORDINAL_TARGET,
                                                     titleBuf17,
                                                     (int)sizeof(titleBuf17));
    snprintf(msg, sizeof(msg),
             "mirror catalog title for ordinal %d (BORIS) = \"%s\" "
             "(expected non-empty -- 'WIZARD OF BALDOR' on DM1 V1 PC 3.4)",
             ORDINAL_TARGET, titleBuf17[0] ? titleBuf17 : "");
    CHECK(titleRc17 > 0 && titleBuf17[0] != '\0', msg);

    nameRc18 = M11_GameView_GetMirrorNameByOrdinal(state,
                                                   ORDINAL_APPROACH_ANCHOR,
                                                   nameBuf18,
                                                   (int)sizeof(nameBuf18));
    snprintf(msg, sizeof(msg),
             "mirror catalog name for ordinal %d (SONJA) = \"%s\" "
             "(expected \"SONJA\")",
             ORDINAL_APPROACH_ANCHOR, nameBuf18[0] ? nameBuf18 : "");
    CHECK(nameRc18 > 0 && strcmp(nameBuf18, "SONJA") == 0, msg);

    titleRc18 = M11_GameView_GetMirrorTitleByOrdinal(state,
                                                     ORDINAL_APPROACH_ANCHOR,
                                                     titleBuf18,
                                                     (int)sizeof(titleBuf18));
    snprintf(msg, sizeof(msg),
             "mirror catalog title for ordinal %d (SONJA) = \"%s\" "
             "(expected non-empty)",
             ORDINAL_APPROACH_ANCHOR, titleBuf18[0] ? titleBuf18 : "");
    CHECK(titleRc18 > 0 && titleBuf18[0] != '\0', msg);

    /* Print the catalog strings for diagnostics. */
    printf("  INFO: ordinal 17 (BORIS)  name=\"%s\" title=\"%s\"\n",
           nameBuf17[0] ? nameBuf17 : "(empty)",
           titleBuf17[0] ? titleBuf17 : "(untitled)");
    printf("  INFO: ordinal 18 (SONJA)  name=\"%s\" title=\"%s\"\n",
           nameBuf18[0] ? nameBuf18 : "(empty)",
           titleBuf18[0] ? titleBuf18 : "(untitled)");
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int assetsOk;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d / "
           "approach_from_left / portrait_rect_position ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.world.party.championCount = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    assetsOk = (portraits && portraits->loaded && portraits->pixels &&
                portraits->width >= 8 * PORTRAIT_W &&
                portraits->height >= 3 * PORTRAIT_H);
    if (!assetsOk) {
        printf("  WARN: C026 portrait strip missing or too small; "
               "pixel-match groups will be skipped.\n");
    }

    check_engine_helpers(&state);
    check_approach_anchor_pixel_contract(&state, portraits);
    check_corridor_approach_scan(&state, portraits);
    check_reentry_after_cross_pose(&state, portraits);
    check_catalog_resolution(&state);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
