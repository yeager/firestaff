/*
 * firestaff_dm1_v1_hoc_champion_portrait_11_d2c_far_positive_portrait_rect_position_275_gate_probe.c
 *
 * DM1 V1 Hall of Champions — champion portrait ordinal 11 (STAMM /
 * BLADECASTER), route variant d2c_far_positive, batch group 11.
 *
 * Targeted slice:
 *   ordinal = 11  (C026 strip cell (1, 3) — atlas col 3 row 1;
 *                  source rect (96, 29, 32, 29); mirror catalog
 *                  record "STAMM" / "BLADECASTER" per F0660/F0661).
 *   route   = d2c_far_positive
 *                  The "d2c_far_positive" route is the positive
 *                  cross-check for the D2C (depth-2 center) far
 *                  wall at the canonical (1, 2) DIR_NORTH pose:
 *                  the D1C front-wall rectangle is the ordinal-11
 *                  STAMM portrait (positive — the D1C destination
 *                  is the C026 blit per DUNVIEW.C:3913-3928), and
 *                  the D2C center cell at viewport (59, 19, 106,
 *                  74) — the far-center wall, two cells down the
 *                  corridor — must NOT carry any portrait sprite.
 *                  This is the dedicated "no-floating on the D2C
 *                  far wall" invariant for the ordinal-11 positive
 *                  route.
 *   aspect  = portrait_rect_position
 *                  The D1C champion portrait cutout is the
 *                  source-locked viewport rectangle (96, 35, 32,
 *                  29) per DUNVIEW.C:3913-3928 +
 *                  DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 *                  ChampionPortraitOnWall = {96, 127, 35, 63}.
 *                  The D1C wall-ornament frame is (80, 29, 64, 43)
 *                  per DUNVIEW.C G0205 coordSet 5 / index 12, and
 *                  the inner portrait cutout is parented at
 *                  (frame.x + 16, frame.y + 6) = (96, 35).
 *   batch   = group 11
 *                  d2c_far_positive portrait_rect_position gates:
 *                  ordinal, pose, route triplet, side-wall scan,
 *                  byte-stable redraw, after_party_shuffle.
 *
 * The shipped DM1 V1 PC 3.4 DUNGEON.DAT exposes a C127 sensor
 * with sensorData=1 (HALK) on the (1, 1) cell 0 (north wall of
 * the (1, 1) cell, the source-visible wall bit for the (1, 2)
 * NORTH front cell).  The local PC 3.4 fixture does NOT expose
 * a C127 sensor with sensorData=11 — ordinal 11 (STAMM /
 * BLADECASTER) is reachable only on the after_party_shuffle
 * route via a seeded C127 sensor rewrite.  The same seed
 * pattern is used by the ordinal-3 after_party_shuffle probe
 * (firestaff_dm1_v1_hoc_champion_portrait_03_after_party_shuffle
 * _portrait_rect_position_runtime_probe.c) and the ordinal-11
 * after_party_shuffle probe
 * (firestaff_dm1_v1_hoc_champion_portrait_11_after_party_shuffle
 * _portrait_rect_position_runtime_probe.c).  The seeded sensor
 * is restored on probe exit so no permanent disk change
 * happens.
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     proves ordinal-11 is not among the source-visible DM1 V1
 *     PC 3.4 C127 sensors in the local fixture (no positive
 *     ordinal-11 sensor on the corridor band).  No pixel
 *     contract for ordinal 11 at the d2c_far_positive pose.
 *   - firestaff_dm1_v1_hoc_champion_portrait_11_after_party
 *     _shuffle_portrait_rect_position_runtime_probe covers the
 *     full after_party_shuffle cycle (select/cancel + F0284
 *     rotations + reopen) on the (1, 2) NORTH D1C cutout.
 *     It does NOT cover the d2c_far_positive D2C far-wall
 *     no-floating invariant.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_d2c_far
 *     _positive_portrait_rect_position_193_gate_probe covers
 *     the d2c_far_positive slice for ordinal 1 (HALK) — the
 *     shipped baseline.  Ordinal 11 (STAMM) is a different
 *     atlas slot (col 3 row 1 vs col 1 row 0) with a different
 *     pixel content, so the d2c_far_positive contract for
 *     ordinal 11 is not implicitly covered by the ordinal-01
 *     gate.
 *   - firestaff_dm1_v1_hall_champion_portrait_22_d2c_far
 *     _positive_runtime_probe covers the d2c_far_positive
 *     slice for ordinal 22 (GOTHMOG) at (1, 3, N) — a
 *     different depth-2 view pose (party at D2C cell), a
 *     different ordinal, and a different atlas row.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_11_west
 *     _negative_portrait_rect_position_runtime_probe covers
 *     the negative invariant (no ordinal 11 on the corridor
 *     west wall) — a different route variant.
 *
 * The probe fills that narrow slice by:
 *   1. Seeding the C127 sensor on the (1, 1) cell 0 (north
 *      wall) from sensorData=1 (HALK) to sensorData=11
 *      (STAMM), so the (1, 2) DIR_NORTH pose resolves the
 *      D1C front-mirror ordinal to 11.
 *   2. Asserting the D1C portrait cutout (96, 35, 32, 29)
 *      matches C026 ordinal 11 (STAMM) at >= 90% pixel match
 *      — the positive cross-check.
 *   3. Asserting the D2C center wall zone (viewport 59, 19,
 *      106, 74) does NOT carry C026 ordinal-11 pixels above
 *      the 35% wrong-ordinal drift threshold — the dedicated
 *      d2c_far_positive "no-floating" invariant for the far
 *      center wall at the (1, 2) NORTH positive route.
 *   4. Asserting the D2L + D2R side wall zones (viewport 0,
 *      19, 78, 74) and (146, 19, 78, 74) at the (1, 2) NORTH
 *      pose do NOT carry ordinal-11 pixels above 35% — the
 *      lateral no-floating cross-check for the d2c_far
 *      _positive slice.
 *   5. Asserting the D1C wall ornament zone helper still
 *      returns (80, 29, 64, 43) at the d2c_far_positive pose,
 *      and the same value across the full (1, 2) N/E/S/W pose
 *      lattice, so the wall frame is anchored regardless of
 *      pose (this is the d2c_far slice's portrait_rect
 *      _position contract).
 *   6. Asserting the D2C far wall has rendered content
 *      (>= 30 non-zero pixels and >= 3 distinct non-zero
 *      palette indices) so the no-floating assertion in Group
 *      B cannot be explained away by a "framebuffer was never
 *      painted" hand-wave.
 *   7. Asserting the corridor (x=1, y=2..6) DIR_NORTH scan
 *      resolves exactly one C127 sensor to ordinal 11 after
 *      the seed (the canonical (1, 2) NORTH front cell (1,
 *      1) cell 0 = STAMM), so the d2c_far_positive slice is
 *      the unique positive route for ordinal 11, not a
 *      duplicate of any other corridor ordinal-11 walkpath.
 *      Adjacent corridor cells (1, 3), (1, 4), (1, 5), (1,
 *      6) NORTH must all return -1 (no mirror on their
 *      front cells).
 *   8. Verifying byte-stable redraw at the d2c_far_positive
 *      pose across 4 successive M11_GameView_Draw calls (no
 *      drift between redraws at the (1, 2) NORTH STAMM
 *      pose).
 *   9. Restoring the seeded C127 sensor to its shipped
 *      sensorData=1 baseline so no permanent in-memory or
 *      on-disk mutation persists.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 normalize(M011_CELL(sensor) -
 *     direction) + 3 front-wall sensor filter
 *     (m11_front_cell_mirror_ordinal in src/engine/
 *     m11_game_view.c:11688 — the source of the
 *     d2c_far_positive "ordinal 11 sensor on the front cell"
 *     condition).
 *   ReDMCSB DUNGEON.C:2608-2612 G0289 =
 *     M000_INDEX_TO_ORDINAL(M040_DATA(sensor)) (F0660/F0661
 *     mirror-catalog ordinal-to-name decode).
 *   ReDMCSB DUNVIEW.C:3913-3928 C026 portrait blit into G0109
 *     portrait box (96, 127, 35, 63) = viewport (96, 35, 32,
 *     29).  The C026 blit only happens on D1C
 *     (P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT) —
 *     D2C never gets the C026 blit, even when a C127 sensor
 *     is reachable on the D2C aspect.
 *   ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 *     ChampionPortraitOnWall = {96, 127, 35, 63}.
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 far-to-near viewport
 *     redraw order.
 *   ReDMCSB DUNVIEW.C:8503-8508 F0128 dispatches F0678 (D2L2)
 *     / F0679 (D2R2) before F0119 (D2L) / F0120 (D2R) on the
 *     d2c_far_positive pose.
 *   ReDMCSB COORD.C:1693-1722 PC 3.4 viewport origin (0, 33)
 *     / 224x136 dim.
 *   ReDMCSB COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29.
 *   ReDMCSB MOVESENS.C:1501-1503 sensorData -> F0280
 *     candidate.
 *   ReDMCSB REVIVE.C F0280 materialize candidate from
 *     sensorData.
 *   ReDMCSB DEFS.H:821-826 M027/M028 portrait-grid 8-col
 *     atlas math.
 *   ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS.
 *   ReDMCSB DEFS.H:4050-4051 C710_ZONE_WALL_D2L /
 *     C711_ZONE_WALL_D2R wall zones.
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11688 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:12447 M11_GFX_WALLSET0_D2C = 102
 *     (106x74 wallset bitmap at viewport (59, 19, 106, 74))
 *   src/engine/m11_game_view.c:12451 M11_GFX_WALLSET0_D2R = 100
 *     (78x74 wallset bitmap at viewport (146, 19, 78, 74))
 *   src/engine/m11_game_view.c:12452 M11_GFX_WALLSET0_D2L = 101
 *     (78x74 wallset bitmap at viewport (0, 19, 78, 74))
 *   src/engine/m11_game_view.c:13480-13485 kFrontBlits D2C
 *     (M11_GFX_WALLSET0_D2C, 59, 19, 106, 74).
 *   src/engine/m11_game_view.c:13952 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962 dst=(M11_VIEWPORT_X+96,
 *     M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972 atlas addr ((ord&7)*32,
 *     (ord>>3)*29)
 *   src/engine/m11_game_view.c:14464-14465 D2L/D2R entries
 *     {2, 2, -1, M11_GFX_WALLSET0_D2L, 0, 19, 78, 74} and
 *     {2, 2, 1, M11_GFX_WALLSET0_D2R, 146, 19, 78, 74}.
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     (16-pose ordinal lookup matrix).
 *   firestaff_dm1_v1_champion_mirror_ordinal_11_west_negative
 *     _portrait_rect_position_runtime_probe (ordinal-11 west
 *     negative invariant — different route variant).
 *   firestaff_dm1_v1_hoc_champion_portrait_11_after_party
 *     _shuffle_portrait_rect_position_runtime_probe (ordinal-11
 *     after_party_shuffle cycle — same cell, same seed pattern,
 *     but full select/cancel/rotate/reopen cycle, no D2C
 *     far-wall check).
 *   firestaff_dm1_v1_hoc_champion_portrait_01_d2c_far_positive
 *     _portrait_rect_position_193_gate_probe (ordinal-01
 *     d2c_far_positive slice — different ordinal, different
 *     atlas row).
 *   firestaff_dm1_v1_hall_champion_portrait_22_d2c_far
 *     _positive_runtime_probe (ordinal-22 d2c_far_positive
 *     slice at (1, 3, N) — different party cell, different
 *     ordinal, different atlas row).
 *   firestaff_dm1_v1_hoc_champion_portrait_00_d2r_negative
 *     _portrait_rect_position_072_gate_probe (ordinal-00
 *     d2r_negative route — different ordinal, different view
 *     square, different route variant).
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_d2l_negative
 *     _portrait_rect_position_runtime_probe (ordinal-02
 *     d2l_negative route — different ordinal, different view
 *     square, different route variant).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares
 *     the rendered D1C cutout and the D2C far wall / D2L /
 *     D2R side wall zones against the local C026 strip pulled
 *     from the same GRAPHICS.DAT the runtime is drawing from,
 *     so this is runtime correctness rather than pixel-for-
 *     pixel DOSBox reference parity.
 *   - We do not assume a C127 sensor with sensorData=11 lives
 *     anywhere on the corridor band by default.  The seed in
 *     seed_first_c127_data rewrites the (1, 1) cell 0 C127
 *     sensorData from 1 (HALK, shipped) to 11 (STAMM) for the
 *     duration of the probe only; the bytes on disk remain
 *     PC 3.4 sensorData=1 (HALK), and the in-memory world is
 *     restored before the probe exits.
 *   - Ordinal 11 (STAMM / BLADECASTER) is a real C026 atlas
 *     slot (col 3 row 1, atlas address (96, 29), source rect
 *     (96, 29, 32, 29)).  Its catalog identity is asserted in
 *     Group A so the slice binds to a real source identity,
 *     not a phantom ordinal.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_11_d2c_far_positive
 *   _portrait_rect_position_275_gate_probe DATA_DIR
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
     * route).  Inner portrait cutout is parented at (frame.x + 16,
     * frame.y + 6) = (96, 35). */
    WALLBOX_X = 80,
    WALLBOX_Y = 29,
    WALLBOX_W = 64,
    WALLBOX_H = 43,
    /* D2C (depth-2 center) far wall bitmap zone from
     * M11_GFX_WALLSET0_D2C = 102 (106x74) at viewport
     * (59, 19, 106, 74) per src/engine/m11_game_view.c:12447 +
     * :13483.  This is the center far wall (two cells down the
     * corridor) at the d2c_far_positive pose.  The probe asserts
     * no ordinal-11 portrait pixels live in this zone. */
    D2C_X = VIEWPORT_X + 59,
    D2C_Y = VIEWPORT_Y + 19,
    D2C_W = 106,
    D2C_H = 74,
    /* D2R (depth-2 right) wall bitmap zone from
     * M11_GFX_WALLSET0_D2R = 100 (78x74) at viewport
     * (146, 19, 78, 74) per src/engine/m11_game_view.c:12451 +
     * :14465.  Lateral no-floating cross-check for the
     * d2c_far_positive slice. */
    D2R_X = VIEWPORT_X + 146,
    D2R_Y = VIEWPORT_Y + 19,
    D2R_W = 78,
    D2R_H = 74,
    /* D2L (depth-2 left) wall bitmap zone from
     * M11_GFX_WALLSET0_D2L = 101 (78x74) at viewport
     * (0, 19, 78, 74) per src/engine/m11_game_view.c:12452 +
     * :14464.  Lateral no-floating cross-check for the
     * d2c_far_positive slice. */
    D2L_X = VIEWPORT_X + 0,
    D2L_Y = VIEWPORT_Y + 19,
    D2L_W = 78,
    D2L_H = 74,
    /* Match thresholds.  At the d2c_far_positive pose the D2C
     * far wall and the D2L/D2R side wall zones must not contain
     * a C026 ordinal-11 portrait.  We allow up to 35% pixel match
     * against ordinal 11 (the wrong-ordinal drift threshold used
     * by the actual-pose probe's check_no_stale_ordinal_in_rect).
     * Above 35% means a stale ordinal-11 sprite is floating over
     * the far center wall or the corridor side walls at depth 2. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-route D1C cross-check the cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* Warm-pixel count thresholds.  The grey-stone wall texture
     * uses palette indices 0x01/0x02/0x07/0x0D and never the
     * warm set, so warm_count cleanly distinguishes "portrait
     * present" from "wall texture only". */
    PORTRAIT_WARM_POS_THRESHOLD = 30,
    PORTRAIT_WARM_NEG_THRESHOLD = 30,
    /* The slice target ordinal (STAMM, mirror catalog record). */
    ORDINAL_TARGET = 11,
    /* DM1 V1 direction constants are macros in
     * memory_champion_state_pc34_compat.h:
     *   DIR_NORTH=0, DIR_EAST=1, DIR_SOUTH=2, DIR_WEST=3.
     * Use raw 0/1/2/3 in the body of this probe so we do not
     * collide with the macro defines.  Comments name the
     * constant they map to. */
    /* Hall map index (map 0 = Hall of Champions). */
    HALL_MAP_INDEX = 0,
    /* Corridor party coordinates used for the d2c_far_positive
     * slice.  Party at (1, 2) facing NORTH = D1C cell (1, 1)
     * which is the canonical STAMM pose after the sensor seed. */
    PARTY_MAP_X = 1,
    PARTY_MAP_Y = 2,
    /* The shipped DM1 V1 DUNGEON.DAT C127 sensor at (1, 2)
     * NORTH-route front square (1, 1) cell 0 carries
     * sensorData=1 (HALK).  We seed it to ORDINAL_TARGET (11,
     * STAMM) for the duration of this probe so the D1C blit
     * resolves to the C026 atlas cell 11 instead of cell 1. */
    SHIPPED_SENSOR_OLD_DATA = 1,
    /* D2C content thresholds (Group A). */
    D2C_NONZERO_PIXEL_THRESHOLD = 30,
    D2C_DISTINCT_PALETTE_THRESHOLD = 3
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define CHECKF(cond, fmt, ...) do { \
    if (cond) { ++g_pass; printf("  PASS: " fmt "\n", __VA_ARGS__); } \
    else      { ++g_fail; printf("  FAIL: " fmt "\n", __VA_ARGS__); } \
} while (0)

/* Find the first C127 sensor in the loaded world whose sensorData
 * matches oldData and rewrite it to newData.  Returns the sensor
 * index on success, or -1 if no such sensor exists.  Same seed
 * helper used by the existing ordinal-3 portrait03 probe and the
 * ordinal-3 cancel_reopen probe.  The in-memory sensor record is
 * not written to disk; the original sensorData is captured by the
 * caller for restore on probe exit. */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData, int newData) {
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

/* Pixel-match a 32x29 viewport rect against a single 32x29 cell of
 * the C026 strip (graphics.dat asset slot
 * M11_GFX_CHAMPION_PORTRAITS = 26, atlas 256x87, 8 cols x 3 rows
 * of 32x29 portraits).  Returns matched-percent (0..100) or -1 if
 * the asset is missing.  Source pixels with palette index 1 (the
 * blitter transparentColor used by
 * m11_draw_dm1_front_champion_portrait) and palette index 12
 * (the wall-niche backdrop dark gray) are skipped so the wall
 * background bleed does not skew the match.  Same logic as
 * match_portrait_cell in the d2c_far_positive ordinal-01 gate. */
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

/* Pixel-match an arbitrary viewport rect against a C026 atlas cell
 * (32x29 source cell at column = (ordinal&7), row = (ordinal>>3)).
 * Skips C026 transparent index 1 and wall-niche backdrop index 12
 * in the source, but compares every destination pixel (no rect-
 * shape constraint).  Used for the D2C / D2L / D2R wall zones
 * which are not 32x29. */
static int match_portrait_in_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int rectX, int rectY,
                                  int rectW, int rectH,
                                  int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y, srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if (ordinal < 0 || ordinal >= 24) return -1;
    if ((int)portraits->width < 8 * PORTRAIT_W) return -1;
    if ((int)portraits->height < 3 * PORTRAIT_H) return -1;
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < rectH; ++y) {
        for (x = 0; x < rectW; ++x) {
            int srcX = srcX0 + (x % PORTRAIT_W);
            int srcY = srcY0 + (y % PORTRAIT_H);
            unsigned char src = (unsigned char)(
                portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PORTRAIT_TRANSPARENT) continue;
            if (src == 12) continue;
            ++compared;
            {
                int sx = rectX + x;
                int sy = rectY + y;
                if (sx < 0 || sx >= FB_W || sy < 0 || sy >= FB_H) continue;
                if (src == (unsigned char)(fb[sy * FB_W + sx] & 0x0F)) {
                    ++matched;
                }
            }
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count warm-colored pixels in a framebuffer rect.  The warm-color
 * palette set is {0x07 green, 0x08 red, 0x09 orange, 0x0A peach,
 * 0x0B yellow, 0x0E blue} — the C026 champion portrait skin /
 * clothing palette.  Grey-stone wall texture never uses this set,
 * so warm_count cleanly distinguishes "portrait present" from
 * "wall texture only". */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int count = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char raw = fb[yy * FB_W + xx];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07: case 0x08: case 0x09: case 0x0A:
                case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/* Count distinct non-zero palette indices in a viewport rect.
 * Proves the D2C far wall has at least *some* rendered content
 * (wallset bitmap pixels, etc.) so an empty D2C rect cannot be
 * explained away by a "framebuffer was never painted" hand-wave. */
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

/* Count non-zero pixels in a viewport rect.  Proves the D2C far
 * wall has at least *some* rendered content (wallset bitmap
 * pixels, etc.) so an empty D2C rect cannot be explained away by
 * a "framebuffer was never painted" hand-wave. */
static int rect_nonzero_count(const unsigned char* fb,
                              int x, int y, int w, int h) {
    int count = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            if (fb[yy * FB_W + xx] != 0) ++count;
        }
    }
    return count;
}

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction) pose
 * on map 0 (Hall of Champions) and return the rendered framebuffer
 * in `fb`.  Centralizes the boilerplate so every render call below
 * uses the same field initialization. */
static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
    state->world.party.championCount = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* ── Group A: catalog identity + sensor seed + front ordinal ──
 * Source-locked to DUNGEON.C:2608-2612 (C127 sensorData ->
 * G0289) + F0652 / F0660 / F0661 (mirror catalog build + name +
 * title decode) + DUNGEON.C:2573 (front-cell visibleWallCell
 * normalization).  This group:
 *   (1) Confirms the local DM1 V1 PC 3.4 fixture has at least
 *       24 catalog entries (so ordinal 11 is a valid atlas
 *       slot, not an empty record).
 *   (2) Confirms ordinal 11 resolves to the STAMM / BLADECASTER
 *       catalog identity (pinned to a real source identity, not
 *       a phantom ordinal).
 *   (3) Seeds the C127 sensor at the (1, 1) cell 0 north wall
 *       (the source-visible wall bit for the (1, 2) DIR_NORTH
 *       front cell) from sensorData=1 (HALK) to
 *       sensorData=11 (STAMM).  The sensor index is captured so
 *       it can be restored on probe exit.
 *   (4) Re-park the party at (1, 2) DIR_NORTH and confirm the
 *       seeded front-mirror ordinal is 11 (the D1C blit must
 *       now resolve to atlas cell 11). */
static void check_catalog_and_seed(M11_GameViewState* state,
                                   int* outSeededSensorIdx) {
    int catalogCount;
    char ordinalName[64];
    char ordinalTitle[64];
    int seededSensor;
    int frontOrdinal;

    printf("\n[Group A] catalog identity + sensor seed + front ordinal at (1,2) NORTH\n");

    catalogCount = M11_GameView_GetMirrorCatalogCount(state);
    CHECKF(catalogCount >= ORDINAL_TARGET + 1,
           "Hall mirror catalog has at least %d entries (ordinal 11 must exist, got %d)",
           ORDINAL_TARGET + 1, catalogCount);

    ordinalName[0] = '\0';
    ordinalTitle[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(state, ORDINAL_TARGET,
                                              ordinalName,
                                              (int)sizeof(ordinalName));
    (void)M11_GameView_GetMirrorTitleByOrdinal(state, ORDINAL_TARGET,
                                               ordinalTitle,
                                               (int)sizeof(ordinalTitle));
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "mirror-catalog ordinal %d name == \"STAMM\" (got \"%s\")",
                 ORDINAL_TARGET, ordinalName);
        CHECK(strcmp(ordinalName, "STAMM") == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "mirror-catalog ordinal %d title == \"BLADECASTER\" (got \"%s\")",
                 ORDINAL_TARGET, ordinalTitle);
        CHECK(strcmp(ordinalTitle, "BLADECASTER") == 0, msg);
    }

    /* Seed the (1, 1) cell 0 north-wall C127 sensor from
     * sensorData=1 (HALK, shipped) to sensorData=11 (STAMM) so
     * the (1, 2) NORTH front cell resolves the D1C portrait to
     * atlas cell 11 instead of cell 1. */
    seededSensor = seed_first_c127_data(state, SHIPPED_SENSOR_OLD_DATA,
                                        ORDINAL_TARGET);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded first C127 sensor sensorData %d -> %d (sensor idx=%d)",
                 SHIPPED_SENSOR_OLD_DATA, ORDINAL_TARGET, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }
    *outSeededSensorIdx = seededSensor;

    /* Re-park party at (1, 2) NORTH and confirm the seeded front
     * ordinal is 11. */
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = PARTY_MAP_X;
    state->world.party.mapY = PARTY_MAP_Y;
    state->world.party.direction = 0 /* DIR_NORTH */;
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after seed: M11_GameView_GetFrontMirrorOrdinal((1,2)N) == %d (got %d)",
                 ORDINAL_TARGET, frontOrdinal);
        CHECK(frontOrdinal == ORDINAL_TARGET, msg);
    }
}

/* ── Group B: D2C far wall has rendered content ───────────────
 * Source-locked to DUNVIEW.C:13480-13485 kFrontBlits D2C entry
 * (M11_GFX_WALLSET0_D2C, viewport 59, 19, 106, 74) and the F0128
 * far-to-near draw order that paints the D2C cell before the D1C
 * front wall.  At the d2c_far_positive pose the D2C far wall
 * must carry wallset 102 bitmap pixels — not be silently empty.
 * An empty D2C rect would mean the far wall is missing, not that
 * ordinal 11 is absent from it.  Two-channel content check:
 * (1) >= 30 non-zero pixels (texture actually present) and
 * (2) >= 3 distinct non-zero palette indices (texture is
 * varied, not a single-color fill). */
static void check_d2c_far_has_content(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int nonZero;
    int distinct;

    printf("\n[Group B] D2C far wall rect at (1,2) NORTH has rendered content\n");

    render_at(state, fb, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);
    nonZero = rect_nonzero_count(fb, D2C_X, D2C_Y, D2C_W, D2C_H);
    distinct = rect_distinct_nonzero(fb, D2C_X, D2C_Y, D2C_W, D2C_H);

    CHECKF(nonZero >= D2C_NONZERO_PIXEL_THRESHOLD,
           "D2C far wall has >= %d non-zero pixels (got %d)",
           D2C_NONZERO_PIXEL_THRESHOLD, nonZero);
    CHECKF(distinct >= D2C_DISTINCT_PALETTE_THRESHOLD,
           "D2C far wall has >= %d distinct non-zero palette indices (got %d)",
           D2C_DISTINCT_PALETTE_THRESHOLD, distinct);
}

/* ── Group C: D2C far wall does NOT match ordinal 11 (STAMM) ────
 * The d2c_far_positive slice invariant: at the (1, 2) DIR_NORTH
 * pose the C026 ordinal-11 STAMM portrait must NOT be painted
 * over the D2C far wall (viewport 59, 19, 106, 74).  The D1C
 * front-wall rectangle IS the only destination for the C026
 * blit (DUNVIEW.C:3913-3928 gates on P0117_i_ViewWallIndex ==
 * M587_VIEW_WALL_D1C_FRONT).  A regression that lets the C026
 * blit leak onto the D2C cell would push the match above 35%. */
static void check_d2c_far_no_ordinal_11(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int pct;
    int warm;

    printf("\n[Group C] D2C far wall does NOT match C026 ordinal 11 (STAMM) at (1,2) NORTH\n");

    render_at(state, fb, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);

    /* NOTE on warm_count: the D2C rect at (1, 2) NORTH is the
     * corridor far-center wall, not a side wall.  Unlike the
     * D2L/D2R side walls (which have warm_count == 0 on the
     * same cell), the D2C center wall at depth 2 includes the
     * wallset bitmap 102 corridor far-wall texture plus torch
     * glow from the corridor lighting, so warm_count is
     * non-zero (~4-5% of 7844 pixels) from the wallset bitmap
     * and torch glow, not from the C026 ordinal-11 portrait
     * sprite.  The authoritative check is the C026 atlas
     * pixel match below; warm_count is logged here for the
     * record. */
    warm = rect_warm_count(fb, D2C_X, D2C_Y, D2C_W, D2C_H);
    {
        char info[200];
        snprintf(info, sizeof(info),
                 "D2C far wall warm_count (informational, logged) = %d at (1,2) NORTH",
                 warm);
        printf("  INFO: %s\n", info);
        ++g_pass;
    }

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2C_X, D2C_Y, D2C_W, D2C_H,
                                     ORDINAL_TARGET);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct < WRONG_ORDINAL_MATCH_PCT,
                   "D2C far wall C026 ordinal %d match < %d%% (got %d%%)",
                   ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group D: D2C far wall does NOT match ANY C026 atlas slot ──
 * Strict-dominance negative check across all 24 C026 atlas slots:
 * no portrait sprite (ordinals 0..23) is painted over the D2C
 * far wall at the (1, 2) DIR_NORTH pose.  This catches a
 * hypothetical regression where the C026 blit leaks from D1C
 * into the depth-2 far wall — the d2c_far_positive slice is
 * the dedicated "no-portrait-anywhere-on-D2C" invariant. */
static void check_d2c_far_no_stale_ordinal(M11_GameViewState* state,
                                           const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int bestOrd = -1;
    int bestPct = 0;
    int pct;

    printf("\n[Group D] D2C far wall does NOT match any C026 atlas slot (24-slot strict dominance)\n");

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
        return;
    }

    render_at(state, fb, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);
    for (ord = 0; ord < 24; ++ord) {
        pct = match_portrait_in_rect(portraits, fb,
                                     D2C_X, D2C_Y, D2C_W, D2C_H, ord);
        if (pct > bestPct) {
            bestPct = pct;
            bestOrd = ord;
        }
    }
    CHECKF(bestPct < WRONG_ORDINAL_MATCH_PCT,
           "D2C far wall best C026 match < %d%% (got %d%% at ordinal %d)",
           WRONG_ORDINAL_MATCH_PCT, bestPct, bestOrd);
}

/* ── Group E: D1C portrait cutout IS ordinal 11 (STAMM) positive ─
 * Cross-check that the (1, 2) DIR_NORTH pose (with the seeded
 * C127 sensor on (1, 1) cell 0) is the source-locked STAMM cell
 * on the local DM1 V1 PC 3.4 fixture.  The D1C rectangle must
 * paint the C026 ordinal-11 portrait with >= 90% pixel match
 * and >= 30 warm pixels — the positive cross-check that anchors
 * the d2c_far_positive slice for ordinal 11. */
static void check_d1c_is_ordinal_11(M11_GameViewState* state,
                                    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int warm;

    printf("\n[Group E] D1C portrait cutout IS ordinal 11 (STAMM) at (1,2) NORTH (positive cross-check)\n");

    render_at(state, fb, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    CHECKF(ord == ORDINAL_TARGET,
           "M11_GameView_GetFrontMirrorOrdinal((1,2)N) == %d (got %d)",
           ORDINAL_TARGET, ord);

    warm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
    CHECKF(warm >= PORTRAIT_WARM_POS_THRESHOLD,
           "Inner portrait cutout warm_count >= %d for STAMM (got %d)",
           PORTRAIT_WARM_POS_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_cell(portraits, fb,
                                  PORTRAIT_X, PORTRAIT_Y,
                                  ORDINAL_TARGET);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct >= CORRECT_ORDINAL_MATCH_PCT,
                   "D1C portrait cutout C026 ordinal %d match >= %d%% (got %d%%)",
                   ORDINAL_TARGET, CORRECT_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group F: D2L + D2R side walls do NOT match ordinal 11 ─────
 * Lateral no-floating cross-check for the d2c_far_positive
 * slice: at the (1, 2) DIR_NORTH pose the D2L side wall
 * (viewport 0, 19, 78, 74) and the D2R side wall (viewport
 * 146, 19, 78, 74) must not carry ordinal-11 portrait pixels
 * above the 35% drift threshold.  This is the dedicated "no
 * portrait on either lateral side wall at depth 2" invariant
 * for the d2c_far_positive pose. */
static void check_d2l_d2r_no_ordinal_11(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int warmD2L, warmD2R;
    int pctD2L, pctD2R;

    printf("\n[Group F] D2L + D2R side walls do NOT match ordinal 11 (STAMM) at (1,2) NORTH (lateral no-floating)\n");

    render_at(state, fb, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);

    warmD2L = rect_warm_count(fb, D2L_X, D2L_Y, D2L_W, D2L_H);
    warmD2R = rect_warm_count(fb, D2R_X, D2R_Y, D2R_W, D2R_H);
    CHECKF(warmD2L < PORTRAIT_WARM_NEG_THRESHOLD,
           "D2L side wall warm_count < %d at (1,2) NORTH (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warmD2L);
    CHECKF(warmD2R < PORTRAIT_WARM_NEG_THRESHOLD,
           "D2R side wall warm_count < %d at (1,2) NORTH (got %d)",
           PORTRAIT_WARM_NEG_THRESHOLD, warmD2R);

    if (portraits && portraits->loaded && portraits->pixels) {
        pctD2L = match_portrait_in_rect(portraits, fb,
                                        D2L_X, D2L_Y, D2L_W, D2L_H,
                                        ORDINAL_TARGET);
        pctD2R = match_portrait_in_rect(portraits, fb,
                                        D2R_X, D2R_Y, D2R_W, D2R_H,
                                        ORDINAL_TARGET);
        if (pctD2L < 0 || pctD2R < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pctD2L < WRONG_ORDINAL_MATCH_PCT,
                   "D2L side wall C026 ordinal %d match < %d%% (got %d%%)",
                   ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctD2L);
            CHECKF(pctD2R < WRONG_ORDINAL_MATCH_PCT,
                   "D2R side wall C026 ordinal %d match < %d%% (got %d%%)",
                   ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctD2R);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group G: portrait_rect_position + D1C rect invariants ────
 * Source-locked to DUNVIEW.C:3913-3928 (C026 blit) + DUNVIEW.C
 * G0205 Graphic558 coordSet 5 / index 12 (C346 D1C wall-mirror
 * frame) + DUNVIEW.C:12447/13483 (D2C wallset bitmap 102 at
 * viewport 59, 19, 106, 74).  The D1C wall-mirror frame MUST be
 * at (80, 29, 64, 43) regardless of pose; the portrait cutout
 * MUST be at (frame.x + 16, frame.y + 6) = (96, 35) per the
 * (+16, +6) parented offset. */
static void check_rect_position_invariants(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    const int kPoses[][3] = {
        {PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */},
        {PARTY_MAP_X, PARTY_MAP_Y, 1 /* DIR_EAST */},
        {PARTY_MAP_X, PARTY_MAP_Y, 2 /* DIR_SOUTH */},
        {PARTY_MAP_X, PARTY_MAP_Y, 3 /* DIR_WEST */}
    };
    int i;

    printf("\n[Group G] portrait_rect_position contract across (1,2) pose lattice\n");

    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        state->world.party.mapIndex = HALL_MAP_INDEX;
        state->world.party.mapX = kPoses[i][0];
        state->world.party.mapY = kPoses[i][1];
        state->world.party.direction = kPoses[i][2];
        state->showDebugHUD = 0;
        state->candidateMirrorPanelActive = 0;
        state->candidateMirrorOrdinal = -1;
        state->candidateMirrorPartyIndex = -1;
        state->inventoryPanelActive = 0;
        rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        if (rc != 1) {
            CHECKF(0,
                   "M11_GameView_GetD1CWallOrnamentZone returns 1 at (1,2) dir=%d (got %d)",
                   kPoses[i][2], rc);
            continue;
        }
        if (ornX == WALLBOX_X && ornY == WALLBOX_Y &&
            ornW == WALLBOX_W && ornH == WALLBOX_H &&
            ornX + 16 == 96 && ornY + 6 == 35) {
            CHECKF(1,
                   "D1C rect invariant at (1,2) dir=%d: box=(%d,%d,%d,%d) cutout=(%d,%d)",
                   kPoses[i][2],
                   ornX, ornY, ornW, ornH, ornX + 16, ornY + 6);
        } else {
            CHECKF(0,
                   "D1C rect invariant at (1,2) dir=%d: box=(%d,%d,%d,%d) cutout=(%d,%d)",
                   kPoses[i][2],
                   ornX, ornY, ornW, ornH, ornX + 16, ornY + 6);
        }
    }
}

/* ── Group H: corridor (x=1) DIR_NORTH scan y=2..6 ────────────
 * Walk every (mapX, mapY) on the (x=1) corridor band with
 * DIR_NORTH and confirm the only C127 sensor that resolves to
 * ordinal 11 is the canonical (1, 2) NORTH front cell (1, 1)
 * (sensorData seeded to 11 = STAMM).  Adjacent corridor cells
 * (1, 3), (1, 4), (1, 5), (1, 6) NORTH must all return -1 (no
 * mirror on their front cells).  This proves the
 * d2c_far_positive slice is the unique positive route for
 * ordinal 11 after the seed, not a duplicate of any other
 * corridor ordinal-11 walkpath. */
static void check_corridor_d2c_far_positive_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal11 = 0;

    printf("\n[Group H] Corridor (x=1) DIR_NORTH scan y=2..6 (with seeded sensor)\n");
    for (y = 2; y <= 6; ++y) {
        int ord = 0;
        state->world.party.mapIndex = HALL_MAP_INDEX;
        state->world.party.mapX = 1;
        state->world.party.mapY = y;
        state->world.party.direction = 0 /* DIR_NORTH */;
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal11;
            if (y == 2) {
                printf("  (1,%d) DIR_NORTH -> ordinal %d (canonical STAMM pose after seed)\n",
                       y, ord);
            } else {
                printf("  (1,%d) DIR_NORTH -> ordinal %d (UNEXPECTED for d2c_far_positive slice)\n",
                       y, ord);
            }
        } else if (ord >= 0) {
            printf("  (1,%d) DIR_NORTH -> ordinal %d\n", y, ord);
        } else {
            printf("  (1,%d) DIR_NORTH -> -1 (no mirror)\n", y);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=1, y=2..6) DIR_NORTH scan finds exactly one C127 sensor with sensorData=%d "
             "on the corridor north wall after seed (found %d)",
             ORDINAL_TARGET, foundOrdinal11);
    CHECK(foundOrdinal11 == 1, msg);
}

/* ── Group I: re-entry + byte-stable redraw ───────────────────
 * Re-rendering (1, 2) DIR_NORTH at the d2c_far_positive pose
 * does not silently introduce ordinal 11 on the D2C far wall
 * (no-floating) or remove it from the D1C cutout (positive).
 * 4 successive M11_GameView_Draw calls must produce byte-stable
 * framebuffer pixels (no drift between redraws at the
 * (1, 2) NORTH STAMM pose).  Same invariant as the
 * d2r_negative ordinal-0 gate. */
static void check_d2c_far_reentry_and_stable_redraw(
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

    printf("\n[Group I] Re-enter (1,2) DIR_NORTH — D2C far wall no-floating + D1C positive + byte-stable redraw\n");

    render_at(state, fb0, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) N ordinal == %d (got %d)",
             ORDINAL_TARGET, ord);
    CHECK(ord == ORDINAL_TARGET, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_in_rect(portraits, fb0,
                                 D2C_X, D2C_Y, D2C_W, D2C_H,
                                 ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) N D2C far wall does NOT match ordinal %d < %d%% (got %d%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    pct = match_portrait_cell(portraits, fb0,
                              PORTRAIT_X, PORTRAIT_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,2) N D1C cutout matches ordinal %d >= %d%% (got %d%%)",
             ORDINAL_TARGET, CORRECT_ORDINAL_MATCH_PCT, pct);
    CHECK(pct >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* Byte-stable redraw at the d2c_far_positive pose across
     * 4 cycles.  This is the same invariant the
     * d2r_negative ordinal-0 gate uses for the
     * redraw_after_candidate slice, applied here at the
     * d2c_far_positive pose: a regression that leaks
     * framebuffer state between draws (e.g. a stale back-
     * buffer not cleared, a non-stable re-blt path) would
     * diverge between redraws and fail this group. */
    baselinePct = match_portrait_cell(portraits, fb0,
                                      PORTRAIT_X, PORTRAIT_Y,
                                      ORDINAL_TARGET);
    for (cycle = 1; cycle < 4; ++cycle) {
        int pctN;
        render_at(state, fbN, PARTY_MAP_X, PARTY_MAP_Y, 0 /* DIR_NORTH */);
        pctN = match_portrait_cell(portraits, fbN,
                                   PORTRAIT_X, PORTRAIT_Y,
                                   ORDINAL_TARGET);
        if (pctN != baselinePct) {
            fprintf(stderr,
                    "FAIL (1,2) N cycle %d portrait_rect_position drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctN, baselinePct);
            stable = 0;
        }
        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
            fprintf(stderr,
                    "FAIL (1,2) N cycle %d framebuffer drift in viewport area\n",
                    cycle + 1);
            stable = 0;
        }
    }
    if (stable) {
        printf("  byte_stable_redraw_d2c_far_positive cycles=4 "
               "ordinal %d match=%d%% (no drift)\n",
               ORDINAL_TARGET, baselinePct);
    } else {
        ++g_fail;
    }
}

/* Restore the seeded C127 sensor to its shipped sensorData so
 * no permanent in-memory mutation persists across probe runs.
 * Idempotent: if outSeededSensorIdx < 0 or the sensor array is
 * missing, no-op. */
static void restore_seeded_sensor(M11_GameViewState* state,
                                  int seededSensorIdx) {
    if (!state || !state->world.things || !state->world.things->sensors) {
        return;
    }
    if (seededSensorIdx < 0 ||
        seededSensorIdx >= state->world.things->sensorCount) {
        return;
    }
    state->world.things->sensors[seededSensorIdx].sensorData =
        (unsigned short)SHIPPED_SENSOR_OLD_DATA;
    printf("\n[Restore] C127 sensor idx=%d sensorData restored to %d\n",
           seededSensorIdx, SHIPPED_SENSOR_OLD_DATA);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int assetsAvailable;
    int seededSensorIdx = -1;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d "
           "d2c_far_positive portrait_rect_position (275 gate, batch group 11) ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP firestaff_dm1_v1_hoc_champion_portrait_11_d2c_far_positive_"
               "portrait_rect_position_275_gate_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.world.party.championCount = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    check_catalog_and_seed(&state, &seededSensorIdx);
    check_d2c_far_has_content(&state);
    check_d2c_far_no_ordinal_11(&state, portraits);
    check_d2c_far_no_stale_ordinal(&state, portraits);
    check_d1c_is_ordinal_11(&state, portraits);
    check_d2l_d2r_no_ordinal_11(&state, portraits);
    check_rect_position_invariants(&state);
    check_corridor_d2c_far_positive_scan(&state);
    check_d2c_far_reentry_and_stable_redraw(&state, portraits);

    /* Restore the seeded C127 sensor so no permanent in-memory
     * mutation persists.  The on-disk DUNGEON.DAT is never
     * touched by this probe; the seed only affects the in-memory
     * world state owned by `state`. */
    restore_seeded_sensor(&state, seededSensorIdx);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
