/*
 * DM1 V1 Hall of Champions - champion portrait ordinal 9 (ZED)
 * d1l_no_portrait / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal = 9
 *              C026 strip cell 9 (atlas column 1, row 1;
 *              source rect (32, 29, 32, 29); mirror catalog
 *              record "ZED" / title "DUKE OF BANVILLE" per
 *              F0660/F0661 in the PC 3.4 English build).
 *   route   = d1l_no_portrait
 *              The "d1l_no_portrait" route is a corridor pose
 *              where the D1L (depth-1 left) wall bitmap is
 *              visible at viewport (0, 9, 60, 111), but no
 *              C127 sensor with sensorData=9 is reachable on
 *              the visible wall of the D1L cell.  In the
 *              source-visible DM1 V1 PC 3.4 fixture the party
 *              at (1, 2) facing WEST samples the D1L cell
 *              (0, 3) (one step WEST, one step SOUTH from the
 *              party - the diagonal "left-front" cell when
 *              facing west).  The D1L viewDir is NORTH
 *              (relSide=-1 shifts viewDir=(direction+3)%4),
 *              and the visible wall cell is (viewDir+2)%4 =
 *              SOUTH.  The shipped DUNGEON.DAT places no
 *              C127 sensor with sensorData=9 (ZED) on the
 *              (0, 3) SOUTH wall (the wall that the D1L
 *              bitmap paints at (1, 2) DIR_WEST).  The
 *              ordinal-9 C127 sensor lives at (1, 10)
 *              DIR_NORTH on the local PC 3.4 fixture - not
 *              on any corridor west wall or D1L side wall.
 *              M11_GameView_GetFrontMirrorOrdinal therefore
 *              returns -1 at (1, 2) W (no sensor on the
 *              front cell (0, 2) EAST wall), and the D1C
 *              portrait cutout and the D1L side wall bitmap
 *              must NOT carry an ordinal-9 sprite.  This is
 *              the d1l (depth-1 left) counterpart to the
 *              existing d1r (depth-1 right) slice covered
 *              by the 192 gate probe for ordinal 0, and the
 *              "left-side" analog of the corridor west
 *              west_negative slice covered by the ordinal-9
 *              west_negative sibling probe.
 *   aspect  = portrait_rect_position
 *              The D1C champion portrait cutout is the
 *              source-locked viewport rectangle
 *              (96, 35, 32, 29) per DUNVIEW.C:3913-3928 +
 *              DUNVIEW.C:525
 *              G0109_auc_Graphic558_Box_ChampionPortraitOnWall.
 *              The D1C wall-ornament frame is (80, 29, 64, 43)
 *              per DUNVIEW.C G0205 coordSet 5 / index 12, and
 *              the inner portrait cutout is parented at
 *              (frame.x + 16, frame.y + 6) = (96, 35).
 *              The D1L side wall bitmap is at viewport
 *              (0, 9, 60, 111) per
 *              src/engine/m11_game_view.c:14548 (relForward=1,
 *              relSide=-1, M11_GFX_WALLSET0_D1L = 96, 60x111).
 *   batch   = group 11
 *              portrait_rect_position gates: ordinal, pose,
 *              route triplet, side-wall scan, byte-stable
 *              redraw.
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_ordinal_9_west_negative
 *     _portrait_rect_position_runtime_probe covers the corridor
 *     west wall at (1, 4) DIR_WEST but does NOT pin the D1L
 *     side wall bitmap (0, 9, 60, 111) - the D1L bitmap is a
 *     different geometry (left-side wall vs the front-cell west
 *     wall), so a regression that paints a stale ordinal-9
 *     sprite into the D1L bitmap but not the front-cell west
 *     wall would slip past the ordinal-9 west_negative sibling.
 *   - firestaff_dm1_v1_hoc_champion_portrait_00_d1r_no_portrait
 *     _portrait_rect_position_192_gate_probe covers the d1r
 *     (depth-1 right) zone (164, 9, 60, 111) at (1, 2) DIR_EAST
 *     for ordinal 0.  This probe is the d1l (depth-1 left)
 *     zone (0, 9, 60, 111) at the same map cell but with
 *     DIR_WEST - a different view direction and a different
 *     side wall.  The 192 gate is for ordinal 0 (DAROOU); this
 *     probe is for ordinal 9 (ZED), an entirely different
 *     atlas cell (row 1, col 1 vs row 0, col 0).
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     covers (1, 2, W) = -1 (the "hall_start_west_no_portrait"
 *     pose) for ordinal lookup only - no pixel contract and
 *     no D1L bitmap zone check.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_9_south_return
 *     _portrait_rect_position_runtime_probe covers ordinal 9
 *     on the (1, 5) DIR_SOUTH positive route.  This probe is
 *     the d1l_no_portrait negative slice for the same ordinal.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_*_west_negative
 *     and ordinal_*_d2l_negative probes cover ordinals 2/6/9/
 *     11/13/15/17/21 on the corridor west wall and the D2L
 *     side wall.  None drive the d1l (depth-1 left) zone at
 *     the d1l_no_portrait pose.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_09_candidate
 *     _panel_cancel covers the C040 candidate-panel cancel
 *     state machine for ordinal 9 on the (1, 5) DIR_SOUTH
 *     positive route.  The d1l_no_portrait slice is disjoint:
 *     a different pose, a different wall bitmap (D1L vs
 *     D1C), and a different invariant.
 *
 * The probe fills that narrow slice by:
 *   1. Pinning the d1l_no_portrait pose front-mirror ordinal
 *      to -1 at (1, 2) DIR_WEST.  M11_GameView_GetFrontMirror
 *      Ordinal must return -1 because the front cell (0, 2)
 *      has no C127 sensor on the EAST wall (the visible wall
 *      cell for DIR_WEST, per the ReDMCSB front-wall side
 *      filter), and the D1L cell (0, 3) has no C127 sensor
 *      with sensorData=9 on its SOUTH wall (the visible wall
 *      cell for the D1L viewDir at (1, 2) DIR_WEST).
 *   2. Asserting the D1C portrait cutout (96, 35, 32, 29)
 *      does NOT match C026 ordinal 9 above the 35% drift
 *      threshold (the wrong-ordinal drift threshold used by
 *      the actual-pose probe and the ordinal-9 west_negative
 *      sibling).
 *   3. Asserting the D1L wall bitmap (0, 9, 60, 111) viewport
 *      coordinates does NOT contain ordinal-9 portrait pixels
 *      (no portrait floats on the left-side wall at depth-1
 *      at the d1l_no_portrait pose).
 *   4. Asserting the D1C wall ornament zone helper still
 *      returns (80, 29, 64, 43) at the d1l_no_portrait pose,
 *      so the wall frame is anchored regardless of pose.
 *   5. Asserting the corridor (x=1, y=2..6) west-facing d1l
 *      band consistently returns ordinal -1 at the front-cell
 *      level (the d1l_no_portrait invariant at the party-cell
 *      level is not a one-cell accident).
 *   6. Cross-checking (1, 3) DIR_SOUTH (the cell that carries
 *      a C127 sensor with sensorData=10 (ZED-actually-GANDO)
 *      on its NORTH wall, visible to a party at (1, 3) facing
 *      SOUTH) shows ordinal 10 in the D1C cutout at >= 90%
 *      match - proves the cutout is alive at the source
 *      position.  An empty d1l_no_portrait cutout cannot
 *      silently mean the rectangle is dead.  Ordinal 10
 *      (GANDO) shares the C026 row=1 row-band with ordinal 9
 *      (ZED) but lives at column 2, so the cross-check
 *      proves the row=1 portrait band is painted correctly
 *      without colliding with the slice target.
 *   7. Verifying byte-stable redraw at the d1l_no_portrait
 *      pose across 4 successive M11_GameView_Draw calls
 *      (no drift between redraws).
 *   8. The D2L side wall bitmap (0, 19, 78, 74) is re-asserted
 *      as a byte-stable cross-check - a regression that flips
 *      the D1L but not the D2L (or vice versa) can be
 *      localised by comparing this probe's results to the
 *      ordinal-2 d2l_negative gate's results.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 normalize(M011_CELL(sensor) -
 *     direction) + 3 front-wall sensor filter (the source of
 *     the d1l_no_portrait "no reachable C127 sensor" condition
 *     for both the front cell EAST wall and the D1L cell
 *     SOUTH wall).
 *   ReDMCSB DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL
 *     (M040_DATA(sensor)).
 *   ReDMCSB DUNVIEW.C:3913-3928 C026 portrait blit into G0109
 *     portrait box (only on D1C, only when G0289 > 0).
 *   ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 *     ChampionPortraitOnWall = {96, 127, 35, 63}.
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 far-to-near viewport
 *     redraw order.
 *   ReDMCSB DUNVIEW.C:8488-8533 dispatches D1L/D1R before
 *     D1C; D2L2/D2R2 before D1L/D1R.
 *   ReDMCSB COORD.C:1693-1722 PC 3.4 viewport origin (0, 33) /
 *     224x136 dim.
 *   ReDMCSB COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29.
 *   ReDMCSB MOVESENS.C:1501-1503 sensorData -> F0280
 *     candidate.
 *   ReDMCSB REVIVE.C F0280 materialize candidate from
 *     sensorData.
 *   ReDMCSB DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5.
 *   ReDMCSB DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT.
 *   ReDMCSB DEFS.H:821-826 M027/M028 portrait-grid 8-col
 *     atlas math.
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13952 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962 dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972 atlas addr ((ord&7)*32, (ord>>3)*29)
 *   src/engine/m11_game_view.c:12482 M11_GFX_WALLSET0_D1L = 96
 *     (60x111 wallset bitmap).
 *   src/engine/m11_game_view.c:14548 D1L view-square record
 *     {relForward=1, relSide=-1, M11_GFX_WALLSET0_D1L, 0, 9, 60, 111}.
 *   src/engine/m11_game_view.c:10138-10155 m11_direction_vectors
 *     (DIR_WEST: fx=-1, fy=0, rx=0, ry=-1; the D1L cell at
 *     (1, 2) DIR_WEST is (frontX + rx, frontY + ry) = (0 + 0,
 *     2 + (-1)) ... actually relSide=-1 maps to "left when
 *     facing party.direction"; per m11_dm1_side_lane_offsets
 *     the D1L cell is the (relForward=1, relSide=-1) offset
 *     from the party cell, so at (1, 2) DIR_WEST the D1L cell
 *     is (0, 3)).
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     (16-pose ordinal lookup matrix, includes (1,2,W)=-1).
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     (visual captures + warm-pixel heuristic for ordinal 9).
 *   firestaff_dm1_v1_champion_mirror_ordinal_9_south_return
 *     _portrait_rect_position_runtime_probe
 *     (ordinal 9 south_return portrait_rect_position).
 *   firestaff_dm1_v1_champion_mirror_ordinal_9_west_negative
 *     _portrait_rect_position_runtime_probe
 *     (ordinal 9 corridor west wall negative route - this
 *      probe is the d1l (depth-1 left) view of the same
 *      corridor under the d1l_no_portrait pose).
 *   firestaff_dm1_v1_hall_of_champions_portrait_09_candidate
 *     _panel_cancel_portrait_rect_position_runtime_probe
 *     (ordinal 9 C040 candidate-panel cancel state machine).
 *   firestaff_dm1_v1_hoc_champion_portrait_00_d1r_no_portrait
 *     _portrait_rect_position_192_gate_probe
 *     (ordinal 0 d1r_negative portrait_rect_position - same
 *      map cell (1, 2), opposite direction, opposite side
 *      wall, different ordinal).
 *   firestaff_dm1_v1_champion_mirror_ordinal_*_d2l_negative
 *     (corridor d2l (depth-2 left) negative routes for
 *      ordinals 2/6 - deeper side wall; this probe is the
 *      d1l (depth-1 left) view).
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe
 *     (positive (1,2)N + (1,5)N zones, no negative route).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares
 *     the rendered D1C cutout and the D1L wall zone against
 *     the local C026 strip pulled from the same GRAPHICS.DAT
 *     the runtime is drawing from, so this is runtime
 *     correctness rather than pixel-for-pixel DOSBox reference
 *     parity.
 *   - We do not assume a C127 sensor with sensorData=9 is in
 *     the local DM1 V1 build.  The d1l_no_portrait slice is
 *     specifically the negative route, and the local PC 3.4
 *     DUNGEON.DAT is the source-locked fixture that proves
 *     the rectangles are empty at the d1l_no_portrait pose.
 *     The positive cross-check at (1,3) DIR_SOUTH is the
 *     same pose as the actual-pose probe's
 *     hall_zed_from_north_ordinal_10 entry (which reports
 *     ordinal 10 on (1, 3) SOUTH per the source-visible
 *     C127 sensor on the (1, 4) NORTH wall), so the
 *     cross-check ordinal is dictated by the shipped sensor
 *     data and the slice does not need to mutate anything.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_09_d1l_no_portrait
 *   _portrait_rect_position_273_gate_probe DATA_DIR
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
    /* D1L (depth-1 left) wall bitmap zone from
     * M11_GFX_WALLSET0_D1L = 96 (60x111) at viewport
     * (0, 9, 60, 111) per src/engine/m11_game_view.c:12482 +
     * :14548.  This is the left-side wall that would be
     * visible at the d1l_no_portrait pose (the depth-1 left
     * wall of cell (0, 3) at the (1, 2) DIR_WEST pose).  The
     * probe asserts no ordinal-9 portrait pixels live in this
     * zone.  This is the d1l counterpart to the 192 gate
     * d1r_no_portrait probe's D1R wall bitmap (164, 9, 60, 111)
     * - the d1l zone is the same geometry on the opposite
     * side of the viewport, but the trigger pose is different
     * (DIR_WEST vs DIR_EAST), so the probe fills a separate
     * invariant. */
    D1L_X = VIEWPORT_X + 0,
    D1L_Y = VIEWPORT_Y + 9,
    D1L_W = 60,
    D1L_H = 111,
    /* D2L (depth-2 left) wall bitmap zone from
     * M11_GFX_WALLSET0_D2L = 101 (78x74) at viewport
     * (0, 19, 78, 74) per src/engine/m11_game_view.c:12489 +
     * :14547.  We re-assert the d2l_negative invariant here
     * as a byte-stable cross-check: the ordinal-2 d2l_negative
     * gate covers the d2l_negative invariant at the byte-stable
     * cycle, but this probe runs the d2l_zone match on the
     * same redraw cycle that drives the d1l_zone match, so a
     * regression that flips the d2l but not the d1l (or vice
     * versa) can be localised by comparing the two probes'
     * results. */
    D2L_X = VIEWPORT_X + 0,
    D2L_Y = VIEWPORT_Y + 19,
    D2L_W = 78,
    D2L_H = 74,
    /* Match thresholds.  At the d1l_no_portrait pose the D1C
     * cutout and the D1L wall zone must not contain a C026
     * ordinal-9 portrait.  We allow up to 35% pixel match
     * against ordinal 9 (the wrong-ordinal drift threshold
     * used by the actual-pose probe's
     * check_no_stale_ordinal_in_rect and the ordinal-9
     * west_negative sibling).  Above 35% means a stale
     * ordinal-9 sprite is floating over the corridor west
     * wall or the D1L left-side wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal (ZED, mirror catalog record). */
    ORDINAL_TARGET = 9,
    /* The cross-check ordinal from the (1, 3) DIR_SOUTH pose.
     * The shipped PC 3.4 DUNGEON.DAT places a C127 sensor
     * with sensorData=10 (GANDO) on the (1, 4) NORTH wall -
     * visible to a party at (1, 3) facing SOUTH.  This
     * sensor is the actual_pose probe's
     * hall_zed_from_north_ordinal_10 entry.  Ordinal 10
     * (GANDO) shares the C026 row=1 row-band with ordinal 9
     * (ZED) but lives at column 2 - so this cross-check
     * proves the row=1 portrait band is painted correctly
     * at the source-locked rect without colliding with the
     * slice target. */
    ORDINAL_CROSSCHECK = 10,
    /* Corridor d1l_no_portrait band bounds (x=1, y=2..6).  The
     * ordinal-9 C127 sensor lives at (1, 10) DIR_NORTH on the
     * local PC 3.4 DUNGEON.DAT, so no cell in this band
     * should resolve to ordinal 9 on any corridor west wall
     * or D1L side wall. */
    CORRIDOR_X = 1,
    CORRIDOR_Y_MIN = 2,
    CORRIDOR_Y_MAX = 6
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Pixel-match a 32x29 viewport rect against a single 32x29 cell of
 * the C026 strip (graphics.dat asset slot
 * M11_GFX_CHAMPION_PORTRAITS = 26, atlas 256x87, 8 cols x 3 rows
 * of 32x29 portraits).  Returns matched-percent (0..100) or -1 if
 * the asset is missing.  Source pixels with palette index 1 (the
 * blitter transparentColor used by
 * m11_draw_dm1_front_champion_portrait) and palette index 12
 * (the wall-niche backdrop dark gray) are skipped so the wall
 * background bleed does not skew the match.  Same logic as
 * match_portrait_cell in the 192 d1r_no_portrait gate probe. */
static int match_portrait_cell(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int rectX, int rectY,
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
            int sx = rectX + x;
            int sy = rectY + y;
            unsigned char src;
            unsigned char dst;
            if (sx < 0 || sx >= FB_W || sy < 0 || sy >= FB_H) continue;
            src = (unsigned char)(
                portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            dst = (unsigned char)(
                fb[sy * FB_W + sx] & 0x0F);
            if (src == PORTRAIT_TRANSPARENT) continue;
            if (src == 12) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count distinct non-zero palette indices in a viewport rect.
 * Useful for proving the d1l wall zone has at least *some*
 * rendered content (texture, door frame, etc.) so the empty
 * D1C cutout cannot be explained away by "the framebuffer was
 * never painted". */
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

/* Group A - Engine helper contract surface.
 * M11_GameView_GetD1CWallOrnamentZone must return the source-locked
 * wall box (80, 29, 64, 43) regardless of the active pose, so the
 * portrait_rect_position invariant holds across the d1l_no_portrait
 * slice. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for d1l_no_portrait\n");

    /* Pose the party at (1, 2) W - the canonical ordinal-9
     * d1l_no_portrait route (front cell (0, 2) has no C127 sensor
     * on the EAST wall, and the D1L cell (0, 3) has no C127
     * sensor with sensorData=9 on its SOUTH wall - the visible
     * wall cell for the D1L viewDir at (1, 2) DIR_WEST). */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = 3; /* DIR_WEST */

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

    /* Inner portrait cutout = (ornX+16, ornY+6, 32, 29) = (96, 35,
     * 32, 29). */
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout X == 96 (got %d)", ornX + 16);
    CHECK(ornX + 16 == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == 35 (got %d)", ornY + 6);
    CHECK(ornY + 6 == 35, msg);
}

/* Group B - d1l_no_portrait slice pixel contract.
 * At (1, 2) DIR_WEST the engine returns ordinal -1 because no C127
 * sensor with sensorData=9 is on the front cell (0, 2) EAST wall,
 * and no C127 sensor with sensorData=9 is on the D1L cell (0, 3)
 * SOUTH wall (the visible wall cell for the D1L viewDir).  The
 * D1C portrait cutout (96, 35, 32, 29) and the D1L wall bitmap
 * (0, 9, 60, 111) must NOT contain C026 ordinal-9 pixels. */
static void check_d1l_no_portrait_pixel_contract(M11_GameViewState* state,
                                                 const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctCutout;
    int pctD1L;
    int distinctD1L;
    char msg[200];

    printf("\n[Group B] (1,2) DIR_WEST pixel contract - ordinal 9 must NOT be in the D1C cutout or the D1L wall zone\n");

    render_at(state, fb, 1, 2, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,2) W) == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    /* Sanity: the D1L wall zone must have *some* rendered content
     * (wallset bitmap pixels) - otherwise the empty cutout is
     * meaningless.  We expect at least 3 distinct non-zero palette
     * indices in the D1L wall zone, proving the left-side wall
     * actually rendered. */
    distinctD1L = rect_distinct_nonzero(fb,
                                        D1L_X, D1L_Y,
                                        D1L_W, D1L_H);
    snprintf(msg, sizeof(msg),
             "(1,2) W D1L wall zone has rendered content "
             "(distinct non-zero palette indices >= 3, got %d)",
             distinctD1L);
    CHECK(distinctD1L >= 3, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing - pixel-match group skipped\n");
        return;
    }

    /* Pixel-match against C026 ordinal 9 in the D1C cutout.  The
     * cutout must NOT match ordinal 9 above the wrong-ordinal
     * drift threshold (35%).  A regression that paints a stale
     * ordinal-9 sprite over the corridor west wall would push the
     * match above 35%. */
    pctCutout = match_portrait_cell(portraits, fb,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,2) W D1C cutout does NOT match ordinal %d (>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctCutout);
    CHECK(pctCutout < WRONG_ORDINAL_MATCH_PCT, msg);

    /* Pixel-match against C026 ordinal 9 in the D1L wall zone.
     * The D1L wall bitmap must NOT carry ordinal-9 portrait
     * pixels above the drift threshold - proves the left-side
     * wall at the d1l view is wall texture only, not a floating
     * portrait sprite.  This is the d1l counterpart to the 192
     * d1r_no_portrait gate's D1R wall zone check. */
    pctD1L = match_portrait_cell(portraits, fb,
                                 D1L_X, D1L_Y,
                                 ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,2) W D1L wall zone does NOT match ordinal %d (>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctD1L);
    CHECK(pctD1L < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group C - Corridor d1l_no_portrait band scan.
 * Walk every (mapX, mapY) on the (x=1) corridor band with
 * DIR_WEST and confirm no C127 sensor resolves to ordinal 9 on
 * the corridor west wall at the front-cell level.  The corridor
 * west wall has no C127 sensors in the source-visible DM1 V1
 * PC 3.4 fixture for the front cell of any of those cells, so
 * the engine must consistently return -1.  This is the party-
 * cell level counterpart to the ordinal-9 west_negative sibling
 * - the d1l_no_portrait slice is the d1l (depth-1 left) view of
 * the same corridor under the d1l_no_portrait pose. */
static void check_corridor_d1l_no_portrait_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal9 = 0;
    int ordinalsFound[8];
    int ordinalsCount = 0;
    int i;

    printf("\n[Group C] Corridor (x=%d) DIR_WEST scan y=%d..%d\n",
           CORRIDOR_X, CORRIDOR_Y_MIN, CORRIDOR_Y_MAX);
    for (y = CORRIDOR_Y_MIN; y <= CORRIDOR_Y_MAX; ++y) {
        int ord = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = CORRIDOR_X;
        state->world.party.mapY = y;
        state->world.party.direction = 3; /* DIR_WEST */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal9;
            printf("  (1,%d) DIR_WEST -> ordinal %d (UNEXPECTED for d1l_no_portrait slice)\n",
                   y, ord);
        } else if (ord >= 0 && ordinalsCount < (int)(sizeof(ordinalsFound) / sizeof(ordinalsFound[0]))) {
            ordinalsFound[ordinalsCount++] = ord;
            printf("  (1,%d) DIR_WEST -> ordinal %d (not ordinal %d)\n",
                   y, ord, ORDINAL_TARGET);
        } else {
            printf("  (1,%d) DIR_WEST -> -1 (no mirror)\n", y);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=%d, y=%d..%d) DIR_WEST scan finds no C127 sensor with sensorData=%d "
             "on the corridor west wall (found %d)",
             CORRIDOR_X, CORRIDOR_Y_MIN, CORRIDOR_Y_MAX, ORDINAL_TARGET, foundOrdinal9);
    CHECK(foundOrdinal9 == 0, msg);

    /* Side-check: the engine helper at each d1l_no_portrait pose
     * still returns the source-locked wall box (80, 29, 64, 43). */
    for (i = 0; i < ordinalsCount; ++i) {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Engine helper invariant: D1C wall box is (%d, %d, %d, %d) for any corridor d1l_no_portrait pose "
                 "(got (%d, %d, %d, %d))",
                 WALLBOX_X, WALLBOX_Y, WALLBOX_W, WALLBOX_H,
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
        /* Only need to assert this once - break early to avoid
         * redundant PASS lines. */
        break;
    }
}

/* Group D - Cross-check positive ordinal at the SAME rect.
 * At (1, 3) DIR_SOUTH the C127 sensor on the front cell (1, 4)
 * carries sensorData=10 (GANDO) - the actual_pose probe's
 * hall_zed_from_north_ordinal_10 entry.  The engine paints the
 * ordinal-10 portrait into the D1C cutout at this pose, proving
 * the D1C rectangle is alive at the source position: an empty
 * (1, 2) W cutout cannot silently mean the rectangle is dead.
 * Ordinal 10 (GANDO) shares the C026 row=1 row-band with ordinal
 * 9 (ZED) but lives at column 2, so this cross-check proves the
 * row=1 portrait band is painted correctly without colliding
 * with the slice target. */
static void check_positive_cross_check(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctWant;
    int pctTarget;
    char msg[200];

    printf("\n[Group D] (1,3) DIR_SOUTH cross-check - D1C cutout IS painted with ordinal %d (not %d)\n",
           ORDINAL_CROSSCHECK, ORDINAL_TARGET);

    render_at(state, fb, 1, 3, 2 /* DIR_SOUTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,3) S) == %d (got %d)",
             ORDINAL_CROSSCHECK, ord);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing - pixel-match group skipped\n");
        return;
    }

    /* The cutout must match ordinal 10 (GANDO) above 90%. */
    pctWant = match_portrait_cell(portraits, fb,
                                  PORTRAIT_X, PORTRAIT_Y,
                                  ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "(1,3) S D1C cutout matches ordinal %d (GANDO) >= %d%%%% (got %d%%%%)",
             ORDINAL_CROSSCHECK, CORRECT_ORDINAL_MATCH_PCT, pctWant);
    CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* And it must NOT match ordinal 9 (the slice target) above
     * the wrong-ordinal drift threshold - proves the cross-check
     * painted the right portrait, not ordinal 9 by accident. */
    pctTarget = match_portrait_cell(portraits, fb,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,3) S D1C cutout does NOT match ordinal %d (the slice target) < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctTarget);
    CHECK(pctTarget < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group E - Re-enter d1l_no_portrait to confirm the empty rect
 * invariant holds on a fresh render too (no stale state from
 * the cross-check).  Also exercises the byte-stable redraw
 * invariant at the d1l_no_portrait pose: 4 successive
 * M11_GameView_Draw calls must produce byte-stable framebuffer
 * pixels (no drift between redraws at the d1l_no_portrait pose).
 * This re-asserts the d2l_negative invariant as a cross-check
 * so a regression that flips the d2l but not the d1l (or vice
 * versa) can be localised by comparing this probe's results to
 * the ordinal-2 d2l_negative gate's results. */
static void check_d1l_no_portrait_reentry_and_stable_redraw(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    unsigned char fb0[FB_W * FB_H];
    unsigned char fbN[FB_W * FB_H];
    int ord;
    int pct;
    int cycle;
    int stable = 1;
    int baselinePctD1L = -1;
    int baselinePctD2L = -1;
    char msg[200];

    printf("\n[Group E] Re-enter (1,2) DIR_WEST - empty D1C cutout + D1L wall zone invariant + byte-stable redraw\n");

    render_at(state, fb0, 1, 2, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) W ordinal == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing - pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb0,
                              PORTRAIT_X, PORTRAIT_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) W D1C cutout does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    pct = match_portrait_cell(portraits, fb0,
                              D1L_X, D1L_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) W D1L wall zone does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    /* Byte-stable redraw at the d1l_no_portrait pose across 4
     * cycles.  This is the same invariant the candidate_panel
     * probe uses for the redraw_after_candidate slice, applied
     * here at the d1l_no_portrait pose: a regression that leaks
     * framebuffer state between draws (e.g. a stale back-buffer
     * not cleared, a non-stable re-blt path) would diverge
     * between redraws and fail this group.  We compare both the
     * D1L wall zone (the d1l slice invariant) and the D2L wall
     * zone (the ordinal-2 d2l_negative cross-check) so a
     * regression that flips one zone but not the other can be
     * localised. */
    baselinePctD1L = match_portrait_cell(portraits, fb0,
                                         D1L_X, D1L_Y,
                                         ORDINAL_TARGET);
    baselinePctD2L = match_portrait_cell(portraits, fb0,
                                         D2L_X, D2L_Y,
                                         ORDINAL_TARGET);
    for (cycle = 1; cycle < 4; ++cycle) {
        int pctND1L;
        int pctND2L;
        render_at(state, fbN, 1, 2, 3 /* DIR_WEST */);
        pctND1L = match_portrait_cell(portraits, fbN,
                                      D1L_X, D1L_Y,
                                      ORDINAL_TARGET);
        if (pctND1L != baselinePctD1L) {
            fprintf(stderr,
                    "FAIL (1,2) W cycle %d D1L wall zone drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctND1L, baselinePctD1L);
            stable = 0;
        }
        pctND2L = match_portrait_cell(portraits, fbN,
                                      D2L_X, D2L_Y,
                                      ORDINAL_TARGET);
        if (pctND2L != baselinePctD2L) {
            fprintf(stderr,
                    "FAIL (1,2) W cycle %d D2L wall zone drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctND2L, baselinePctD2L);
            stable = 0;
        }
        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
            fprintf(stderr,
                    "FAIL (1,2) W cycle %d framebuffer drift in viewport area\n",
                    cycle + 1);
            stable = 0;
        }
    }
    if (stable) {
        printf("  byte_stable_redraw_d1l_no_portrait cycles=4 "
               "ordinal %d D1L match=%d%%%% D2L match=%d%%%% (no drift)\n",
               ORDINAL_TARGET, baselinePctD1L, baselinePctD2L);
    } else {
        ++g_fail;
    }
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
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d d1l_no_portrait portrait_rect_position (273 gate) ===\n",
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
    check_d1l_no_portrait_pixel_contract(&state, portraits);
    check_corridor_d1l_no_portrait_scan(&state);
    check_positive_cross_check(&state, portraits);
    check_d1l_no_portrait_reentry_and_stable_redraw(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
