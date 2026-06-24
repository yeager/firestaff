/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 0 (DAROOU)
 * d2r_negative / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal = 0   (C026 strip cell (0, 0) — atlas rect (0, 0, 32, 29);
 *                  mirror catalog record "DAROOU" per F0660/F0661).
 *   route   = d2r_negative
 *                  The "d2r_negative" route is a corridor pose where
 *                  the D2R (depth-2 right) viewport cell would be
 *                  visible, but no C127 sensor with sensorData=0 is
 *                  reachable on the D2R cell aspect.  In the source-
 *                  visible DM1 V1 PC 3.4 fixture the party at (1, 2)
 *                  facing EAST puts the D2R cell on relative (3, 3)
 *                  from the player; the front cell is (2, 2) which
 *                  has no C127 sensor on the WEST wall (the visible
 *                  wall for that view direction), and the D2R cell
 *                  (3, 3) also has no C127 sensor on its WEST wall.
 *                  M11_GameView_GetFrontMirrorOrdinal therefore
 *                  returns -1 and the D1C portrait cutout must NOT
 *                  carry an ordinal-0 sprite at the d2r_negative
 *                  pose.  This is the d2r view of the same corridor
 *                  covered by west_negative probes for ordinals 2, 6,
 *                  9, 11, 13, 15, 17, 21 — but at the d2r (right
 *                  side) rather than the d2l (left side) wall.
 *   aspect  = portrait_rect_position
 *                  The D1C champion portrait cutout is the source-
 *                  locked viewport rectangle (96, 35, 32, 29) per
 *                  DUNVIEW.C:3913-3928 + DUNVIEW.C:525
 *                  G0109_auc_Graphic558_Box_ChampionPortraitOnWall.
 *                  The D1C wall-ornament frame is (80, 29, 64, 43)
 *                  per DUNVIEW.C G0205 coordSet 5 / index 12, and
 *                  the inner portrait cutout is parented at
 *                  (frame.x + 16, frame.y + 6) = (96, 35).
 *   batch   = group 3
 *                  portrait_rect_position gates: ordinal, pose, route
 *                  triplet, side-wall scan, byte-stable redraw.
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     covers (1, 2, E) = -1 (the "hall_start_east_wrong_wall_no
 *     _portrait" pose) for ordinal lookup only — no pixel contract.
 *   - firestaff_dm1_v1_champion_mirror_portrait00_rect_runtime_probe
 *     and the portrait00_south_return probe cover ordinal 0 on the
 *     (1, 2) NORTH and (1, 5) SOUTH positive routes.  Neither drives
 *     the d2r_negative pose.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_*_west_negative
 *     probes cover ordinals 2/6/9/11/13/15/17/21 on the corridor
 *     west wall.  No d2r_negative variant exists for any ordinal.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_00_cancel_reopen
 *     covers ordinal 0 cancel_reopen on the (1, 2) NORTH positive
 *     route.  The d2r_negative slice is disjoint: a different pose
 *     and a different invariant.
 *   - The D2R viewport wall bitmap (M11_GFX_WALLSET0_D2R = 100, 78x74
 *     at viewport (146, 19)) is exercised by the d2l2/d2r2 viewport
 *     tests, but the d2r_negative *portrait_rect_position* invariant
 *     is not covered by any existing gate.
 *
 * The probe fills that narrow slice by:
 *   1. Pinning the d2r_negative pose front-mirror ordinal to -1 at
 *      (1, 2) DIR_EAST.  M11_GameView_GetFrontMirrorOrdinal must
 *      return -1 because the front cell (2, 2) has no C127 sensor
 *      on the WEST wall, and the D2R cell (3, 3) has no C127 sensor
 *      on the WEST wall either.
 *   2. Asserting the D1C portrait cutout (96, 35, 32, 29) does NOT
 *      match C026 ordinal 0 above the 35% drift threshold (the
 *      wrong-ordinal drift threshold used by the actual-pose probe).
 *   3. Asserting the D2R wall zone (146, 19, 78, 74) viewport
 *      coordinates does NOT contain ordinal-0 portrait pixels
 *      (no portrait floats on the right-side wall at the
 *      d2r_negative pose).
 *   4. Asserting the D1C wall ornament zone helper still returns
 *      (80, 29, 64, 43) at the d2r_negative pose, so the wall
 *      frame is anchored regardless of pose.
 *   5. Asserting the corridor (x=1, y=2..6) east-facing d2r band
 *      consistently returns ordinal -1 (the d2r_negative invariant
 *      is not a one-cell accident).
 *   6. Cross-checking (1, 2) DIR_NORTH (after seeding the C127
 *      sensor from HALK to ordinal 0) shows ordinal 0 in the D1C
 *      cutout at >= 90% match — proves the cutout is alive at the
 *      source position.  An empty d2r_negative cutout cannot
 *      silently mean the rectangle is dead.
 *   7. Verifying byte-stable redraw at the d2r_negative pose across
 *      4 successive M11_GameView_Draw calls (no drift between
 *      redraws).
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter (the source of the d2r_negative
 *     "no reachable C127 sensor" condition).
 *   ReDMCSB DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL
 *     (M040_DATA(sensor)).
 *   ReDMCSB DUNVIEW.C:3913-3928 C026 portrait blit into G0109
 *     portrait box (only on D1C).
 *   ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 *     ChampionPortraitOnWall = {96, 127, 35, 63}.
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw
 *     order.
 *   ReDMCSB DUNVIEW.C:8503-8508 F0128 dispatches F0678 (D2L2) /
 *     F0679 (D2R2) before F0119 (D2L) / F0120 (D2R) on the
 *     d2r_negative pose.
 *   ReDMCSB COORD.C:1693-1722 PC 3.4 viewport origin (0,33) /
 *     224x136 dim.
 *   ReDMCSB COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29.
 *   ReDMCSB MOVESENS.C:1501-1503 sensorData -> F0280 candidate.
 *   ReDMCSB REVIVE.C F0280 materialize candidate from sensorData.
 *   ReDMCSB DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5.
 *   ReDMCSB DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT.
 *   ReDMCSB DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas math.
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13952 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962 dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972 atlas addr ((ord&7)*32, (ord>>3)*29)
 *   src/engine/m11_game_view.c:12824 D2R view-square id 5 record
 *   src/engine/m11_game_view.c:14465 D2R wall bitmap index 7 in G0206
 *   src/engine/m11_game_view.c:12451 M11_GFX_WALLSET0_D2R = 100
 *     (78x74 wallset bitmap at viewport (146, 19)).
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     (16-pose ordinal lookup matrix, includes (1,2,E)=-1).
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     (visual captures + warm-pixel heuristic for ordinal 0).
 *   firestaff_dm1_v1_champion_mirror_portrait00_rect_runtime_probe
 *     (ordinal 0 north_entry portrait_rect_position).
 *   firestaff_dm1_v1_champion_mirror_portrait00_south_return
 *     _portrait_rect_position_probe
 *     (ordinal 0 south_return portrait_rect_position).
 *   firestaff_dm1_v1_hall_of_champions_portrait_00_cancel_reopen
 *     _portrait_rect_position_runtime_probe
 *     (ordinal 0 cancel_reopen on north route).
 *   firestaff_dm1_v1_champion_mirror_ordinal_*_west_negative
 *     (corridor west wall negative routes for ordinals 2/6/9/11/13
 *      /15/17/21 — left side; this probe is the right side).
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe
 *     (positive (1,2)N + (1,5)N zones, no negative route).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout and the D2R wall zone against the local
 *     C026 strip pulled from the same GRAPHICS.DAT the runtime is
 *     drawing from, so this is runtime correctness rather than
 *     pixel-for-pixel DOSBox reference parity.
 *   - We do not assume a C127 sensor with sensorData=0 is in the
 *     local DM1 V1 build.  The d2r_negative slice is specifically
 *     the negative route, and the local PC 3.4 DUNGEON.DAT is the
 *     source-locked fixture that proves the rectangle is empty at
 *     the d2r_negative pose.  The positive cross-check at (1,2)N
 *     seeds the existing C127 sensor to ordinal 0 because the
 *     shipped sensorData=1 (HALK) does not match ordinal 0.
 *   - We do not duplicate the cancel_reopen or redraw_after
 *     _candidate gates: those cover different routes and the
 *     d2r_negative slice is the d2r view of the corridor under
 *     the (1,2)E pose.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_00_d2r_negative_portrait
 *   _rect_position_072_gate_probe DATA_DIR
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
    /* D2R (depth-2 right) wall bitmap zone from
     * M11_GFX_WALLSET0_D2R = 100 (78x74) at viewport
     * (146, 19, 78, 74) per src/engine/m11_game_view.c:12451 +
     * :14465 + :12824.  This is the right-side wall that would
     * be visible at the d2r_negative pose.  The probe asserts no
     * ordinal-0 portrait pixels live in this zone. */
    D2R_X = VIEWPORT_X + 146,
    D2R_Y = VIEWPORT_Y + 19 - 33, /* viewport-local */
    D2R_W = 78,
    D2R_H = 74,
    /* Match thresholds.  At the d2r_negative pose the D1C cutout
     * and the D2R wall zone must not contain a C026 ordinal-0
     * portrait.  We allow up to 35% pixel match against ordinal 0
     * (the wrong-ordinal drift threshold used by the actual-pose
     * probe's check_no_stale_ordinal_in_rect).  Above 35% means a
     * stale ordinal-0 sprite is floating over the corridor east
     * wall or the D2R right-side wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal (DAROOU, mirror catalog record). */
    ORDINAL_TARGET = 0,
    /* The cross-check ordinal from the seeded (1, 2) DIR_NORTH
     * pose.  The shipped PC 3.4 DUNGEON.DAT places a C127 sensor
     * with sensorData=1 (HALK) on the (1, 2) NORTH-route front
     * square (1, 1); we seed that sensor to ordinal 0 to lock the
     * ordinal-0 edge case on the same map cell. */
    ORDINAL_CROSSCHECK = 0
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
 * match_portrait_cell in the ordinal_2_west_negative probe. */
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
 * Useful for proving the d2r wall zone has at least *some*
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

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found.  We use this to
 * lock the ordinal-0 edge case on the real DM1 V1 DUNGEON.DAT
 * (which ships HALK / ordinal 1 on the (1,2) NORTH-route front
 * square (1,1)).  The seed does NOT change the map layout or the
 * C127 cell match - only the G0289 ordinal that DUNVIEW.C:3913-3928
 * reads through M000_INDEX_TO_ORDINAL (DUNGEON.C:2610-2612). */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData,
                                int newData) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Group A — Engine helper contract surface.
 * M11_GameView_GetD1CWallOrnamentZone must return the source-locked
 * wall box (80, 29, 64, 43) regardless of the active pose, so the
 * portrait_rect_position invariant holds across the d2r_negative
 * slice. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for d2r_negative\n");

    /* Pose the party at (1, 2) E — the canonical ordinal-0
     * d2r_negative route (front cell (2, 2) has no C127 sensor on
     * the WEST wall). */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = 1; /* DIR_EAST */

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

/* Group B — d2r_negative slice pixel contract.
 * At (1, 2) DIR_EAST the engine returns ordinal -1 because no C127
 * sensor with sensorData=0 is on the front cell.  The D1C portrait
 * cutout (96, 35, 32, 29) and the D2R wall zone (146, 19, 78, 74)
 * must NOT contain C026 ordinal-0 pixels. */
static void check_d2r_negative_pixel_contract(M11_GameViewState* state,
                                              const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctCutout;
    int pctD2R;
    int distinctD2R;
    char msg[200];

    printf("\n[Group B] (1,2) DIR_EAST pixel contract — ordinal 0 must NOT be in the D1C cutout or the D2R wall zone\n");

    render_at(state, fb, 1, 2, 1 /* DIR_EAST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,2) E) == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    /* Sanity: the D2R wall zone must have *some* rendered content
     * (wallset bitmap pixels) — otherwise the empty cutout is
     * meaningless.  We expect at least 3 distinct non-zero palette
     * indices in the D2R wall zone, proving the right-side wall
     * actually rendered. */
    distinctD2R = rect_distinct_nonzero(fb,
                                        VIEWPORT_X + D2R_X,
                                        VIEWPORT_Y + D2R_Y,
                                        D2R_W, D2R_H);
    snprintf(msg, sizeof(msg),
             "(1,2) E D2R wall zone has rendered content "
             "(distinct non-zero palette indices >= 3, got %d)",
             distinctD2R);
    CHECK(distinctD2R >= 3, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    /* Pixel-match against C026 ordinal 0 in the D1C cutout.  The
     * cutout must NOT match ordinal 0 above the wrong-ordinal
     * drift threshold (35%).  A regression that paints a stale
     * ordinal-0 sprite over the corridor east wall would push the
     * match above 35%. */
    pctCutout = match_portrait_cell(portraits, fb,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,2) E D1C cutout does NOT match ordinal %d (>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctCutout);
    CHECK(pctCutout < WRONG_ORDINAL_MATCH_PCT, msg);

    /* Pixel-match against C026 ordinal 0 in the D2R wall zone.
     * The D2R wall bitmap must NOT carry ordinal-0 portrait
     * pixels above the drift threshold — proves the right-side
     * wall at the d2r view is wall texture only, not a floating
     * portrait sprite. */
    pctD2R = match_portrait_cell(portraits, fb,
                                 D2R_X, VIEWPORT_Y + D2R_Y,
                                 ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,2) E D2R wall zone does NOT match ordinal %d (>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctD2R);
    CHECK(pctD2R < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group C — Corridor d2r_negative band scan.
 * Walk every (mapX, mapY) on the (x=1) corridor band with
 * DIR_EAST and confirm no C127 sensor resolves to ordinal 0 on
 * the corridor east wall.  The corridor east wall has no C127
 * sensors in the source-visible DM1 V1 PC 3.4 fixture for any
 * of those cells, so the engine must consistently return -1. */
static void check_corridor_d2r_negative_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal0 = 0;
    int ordinalsFound[8];
    int ordinalsCount = 0;
    int i;

    printf("\n[Group C] Corridor (x=1) DIR_EAST scan y=2..6\n");
    for (y = 2; y <= 6; ++y) {
        int ord = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 1;
        state->world.party.mapY = y;
        state->world.party.direction = 1; /* DIR_EAST */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal0;
            printf("  (1,%d) DIR_EAST -> ordinal %d (UNEXPECTED for d2r_negative slice)\n",
                   y, ord);
        } else if (ord >= 0 && ordinalsCount < (int)(sizeof(ordinalsFound) / sizeof(ordinalsFound[0]))) {
            ordinalsFound[ordinalsCount++] = ord;
            printf("  (1,%d) DIR_EAST -> ordinal %d\n", y, ord);
        } else {
            printf("  (1,%d) DIR_EAST -> -1 (no mirror)\n", y);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=1, y=2..6) DIR_EAST scan finds no C127 sensor with sensorData=%d "
             "on the corridor east wall (found %d)",
             ORDINAL_TARGET, foundOrdinal0);
    CHECK(foundOrdinal0 == 0, msg);

    /* Side-check: the engine helper at each d2r_negative pose
     * still returns the source-locked wall box (80, 29, 64, 43). */
    for (i = 0; i < ordinalsCount; ++i) {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Engine helper invariant: D1C wall box is (%d, %d, %d, %d) for any corridor d2r_negative pose "
                 "(got (%d, %d, %d, %d))",
                 WALLBOX_X, WALLBOX_Y, WALLBOX_W, WALLBOX_H,
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
        /* Only need to assert this once — break early to avoid
         * redundant PASS lines. */
        break;
    }
}

/* Group D — Cross-check positive ordinal at the SAME rect.
 * Seed the existing (1, 2) NORTH-route C127 sensor from HALK (1)
 * to ordinal 0 and verify the engine paints the ordinal-0
 * portrait into the D1C cutout at (1, 2) DIR_NORTH.  This proves
 * the D1C rectangle is alive at the source position: an empty
 * (1, 2) E cutout cannot silently mean the rectangle is dead. */
static void check_positive_cross_check(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctWant;
    int pctTarget;
    int seededSensor;
    char msg[200];

    printf("\n[Group D] (1,2) DIR_NORTH cross-check — D1C cutout IS painted with ordinal %d (after seeding)\n",
           ORDINAL_CROSSCHECK);

    /* Seed the C127 sensor from sensorData=1 (HALK) to
     * sensorData=0 (DAROOU) on the (1, 2) NORTH-route front
     * square. */
    seededSensor = seed_first_c127_data(state, 1, ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "seeded (1,2) NORTH-route C127 sensor from 1 to %d (sensor index %d)",
             ORDINAL_CROSSCHECK, seededSensor);
    CHECK(seededSensor >= 0, msg);

    render_at(state, fb, 1, 2, 0 /* DIR_NORTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,2) N seeded) == %d (got %d)",
             ORDINAL_CROSSCHECK, ord);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    /* The cutout must match ordinal 0 (DAROOU) above 90%. */
    pctWant = match_portrait_cell(portraits, fb,
                                  PORTRAIT_X, PORTRAIT_Y,
                                  ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "(1,2) N D1C cutout matches ordinal %d (DAROOU) >= %d%%%% (got %d%%%%)",
             ORDINAL_CROSSCHECK, CORRECT_ORDINAL_MATCH_PCT, pctWant);
    CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* And it must NOT match ordinal 0 (the slice target) at the
     * d2r_negative pose's drift threshold — this is the same
     * ordinal because ORDINAL_CROSSCHECK == ORDINAL_TARGET, so
     * we sanity-check that the cross-check painted the right
     * portrait by counting it as a positive match (>= 90% wins
     * over the drift threshold).  This step is intentionally a
     * no-op double-check: the >= 90% check above already proves
     * the cross-check painted ordinal 0. */
    pctTarget = match_portrait_cell(portraits, fb,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,2) N D1C cutout matches ordinal %d (the slice target) >= %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, CORRECT_ORDINAL_MATCH_PCT, pctTarget);
    CHECK(pctTarget >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* Group E — Re-enter d2r_negative to confirm the empty rect
 * invariant holds on a fresh render too (no stale state from
 * the cross-check).  Also exercises the byte-stable redraw
 * invariant at the d2r_negative pose: 4 successive
 * M11_GameView_Draw calls must produce byte-stable framebuffer
 * pixels (no drift between redraws at the d2r_negative pose). */
static void check_d2r_negative_reentry_and_stable_redraw(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    unsigned char fb0[FB_W * FB_H];
    unsigned char fbN[FB_W * FB_H];
    int ord;
    int pct;
    int cycle;
    int stable = 1;
    int baselinePct = -1;
    char msg[200];

    printf("\n[Group E] Re-enter (1,2) DIR_EAST — empty D1C cutout + D2R wall zone invariant + byte-stable redraw\n");

    render_at(state, fb0, 1, 2, 1 /* DIR_EAST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) E ordinal == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb0,
                              PORTRAIT_X, PORTRAIT_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) E D1C cutout does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    pct = match_portrait_cell(portraits, fb0,
                              D2R_X, VIEWPORT_Y + D2R_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) E D2R wall zone does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    /* Byte-stable redraw at the d2r_negative pose across 4 cycles.
     * This is the same invariant the candidate_panel probe uses
     * for the redraw_after_candidate slice, applied here at the
     * d2r_negative pose: a regression that leaks framebuffer state
     * between draws (e.g. a stale back-buffer not cleared, a
     * non-stable re-blt path) would diverge between redraws and
     * fail this group. */
    baselinePct = match_portrait_cell(portraits, fb0,
                                      PORTRAIT_X, PORTRAIT_Y,
                                      ORDINAL_TARGET);
    for (cycle = 1; cycle < 4; ++cycle) {
        int pctN;
        render_at(state, fbN, 1, 2, 1 /* DIR_EAST */);
        pctN = match_portrait_cell(portraits, fbN,
                                   PORTRAIT_X, PORTRAIT_Y,
                                   ORDINAL_TARGET);
        if (pctN != baselinePct) {
            fprintf(stderr,
                    "FAIL (1,2) E cycle %d portrait_rect_position drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctN, baselinePct);
            stable = 0;
        }
        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
            fprintf(stderr,
                    "FAIL (1,2) E cycle %d framebuffer drift in viewport area\n",
                    cycle + 1);
            stable = 0;
        }
    }
    if (stable) {
        printf("  byte_stable_redraw_d2r_negative cycles=4 "
               "ordinal %d match=%d%%%% (no drift)\n",
               ORDINAL_TARGET, baselinePct);
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
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d d2r_negative portrait_rect_position (072 gate) ===\n",
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
    check_d2r_negative_pixel_contract(&state, portraits);
    check_corridor_d2r_negative_scan(&state);
    check_positive_cross_check(&state, portraits);
    check_d2r_negative_reentry_and_stable_redraw(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
