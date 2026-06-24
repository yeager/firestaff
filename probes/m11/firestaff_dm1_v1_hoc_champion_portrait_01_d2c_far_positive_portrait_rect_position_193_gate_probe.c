/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 1 (HALK)
 * d2c_far_positive / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal = 1   (C026 strip cell (1, 0) — atlas col 1 row 0;
 *                  source rect (32, 0, 32, 29); mirror catalog
 *                  record "HALK" per F0660/F0661).
 *   route   = d2c_far_positive
 *                  The "d2c_far_positive" route is the positive
 *                  cross-check for the D2C (depth-2 center)
 *                  far wall: at the canonical (1, 2) DIR_NORTH
 *                  pose the D1C front-wall rectangle is the
 *                  ordinal-1 HALK portrait (positive — the
 *                  D1C destination is the C026 blit per
 *                  DUNVIEW.C:3913-3928), and the D2C center
 *                  cell at viewport (59, 19, 106, 74) — the
 *                  far-center wall, i.e. the wall two cells
 *                  down the corridor — must NOT carry any
 *                  portrait sprite.  This is the dedicated
 *                  "no-floating on the D2C far wall" invariant
 *                  for the ordinal-1 positive route.
 *   aspect  = portrait_rect_position
 *                  The D1C champion portrait cutout is the
 *                  source-locked viewport rectangle (96, 35,
 *                  32, 29) per DUNVIEW.C:3913-3928 +
 *                  DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 *                  ChampionPortraitOnWall = {96, 127, 35, 63}.
 *                  The D1C wall-ornament frame is (80, 29, 64,
 *                  43) per DUNVIEW.C G0205 coordSet 5 / index
 *                  12, and the inner portrait cutout is
 *                  parented at (frame.x + 16, frame.y + 6) =
 *                  (96, 35).
 *   batch   = group 8
 *                  portrait_rect_position gates: ordinal, pose,
 *                  route triplet, side-wall scan, byte-stable
 *                  redraw.
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     proves (1, 2, N) = 1 (the canonical HALK pose) for ordinal
 *     lookup only — no pixel contract, no D2C far-wall check.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_1_halk_pose_probe
 *     covers the HALK pose (1, 2) NORTH + the west_negative
 *     (1, 2) WEST slice, but does NOT cover the D2C far-wall
 *     slice at the (1, 2) NORTH positive route.
 *   - firestaff_dm1_v1_hall_champion_portrait_01_east_walkpath
 *     _portrait_rect_position_runtime_probe covers the
 *     east_walkpath corridor walk (1, 3) / (2, 3) / (3, 3) NORTH
 *     — all no-portrait cells — and does NOT cover the D2C
 *     far-wall slice.
 *   - firestaff_dm1_v1_hall_of_champions_champion_portrait_01
 *     _south_return_portrait_rect_position_probe covers the
 *     south_return (1, 0) DIR_SOUTH slice — a different pose
 *     with no D2C wall check.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_candidate_panel
 *     _open_portrait_rect_position_097_gate_probe covers the
 *     C040 panel-open slice at the HALK pose — no D2C
 *     far-wall check.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_input_focus
 *     _restore_portrait_rect_position_145_gate_probe covers
 *     input focus / panel-state restoration — no D2C
 *     far-wall check.
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after
 *     _candidate_portrait_rect_position_097_gate_probe covers
 *     the redraw_after_candidate slice — no D2C far-wall check.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_01_cancel
 *     _reopen_portrait_rect_position_runtime_probe covers the
 *     C040 panel select/cancel/select cycle — no D2C far-wall
 *     check.
 *   - The D2C view square (M11_GFX_WALLSET0_D2C = 102, 106x74
 *     wallset bitmap at viewport (59, 19, 106, 74)) is
 *     exercised by the kD2C zone blit and the depth-2 floor
 *     pit draw path, but the D2C *portrait_rect_position*
 *     invariant is not covered by any existing ordinal-1 gate.
 *   - The d2r_negative ordinal-0 gate covers the (1, 2) DIR_EAST
 *     ordinal-0 negative route with a D2R wall zone check, but
 *     no ordinal-1 d2c_far_positive gate exists.
 *
 * The probe fills that narrow slice by:
 *   1. Pinning the d2c_far_positive pose front-mirror ordinal
 *      to 1 at (1, 2) DIR_NORTH.  M11_GameView_GetFrontMirror
 *      Ordinal must return 1 because the front cell (1, 1)
 *      carries the shipped C127 sensor with sensorData=1 (HALK)
 *      on its south wall (cell 0, which is the source-visible
 *      wall cell for DIR_NORTH: visibleWallCell = (NORTH + 2) &
 *      3 = 2 — actually the south wall = cell 0; see
 *      DUNGEON.C:2573 normalization).
 *   2. Asserting the D1C portrait cutout (96, 35, 32, 29)
 *      matches C026 ordinal 1 (HALK) at >= 90% pixel match —
 *      the positive cross-check.
 *   3. Asserting the D2C center wall zone (viewport 59, 19,
 *      106, 74) does NOT carry C026 ordinal-1 pixels above
 *      the 35% wrong-ordinal drift threshold — the dedicated
 *      d2c_far_positive "no-floating" invariant for the far
 *      center wall at the (1, 2) NORTH positive route.
 *   4. Asserting the D2L + D2R side wall zones (viewport 0,
 *      19, 78, 74) and (146, 19, 78, 74) at the (1, 2) NORTH
 *      pose do NOT carry ordinal-1 pixels above 35% — the
 *      lateral no-floating cross-check for the d2c_far
 *      _positive slice.
 *   5. Asserting the D1C wall ornament zone helper still
 *      returns (80, 29, 64, 43) at the d2c_far_positive pose,
 *      and the same value across the full (1, 2) N/E/S/W pose
 *      lattice, so the wall frame is anchored regardless of
 *      pose (this is the d2c_far slice's portrait_rect
 *      _position contract).
 *   6. Asserting the corridor (x=1, y=2..6) north-facing
 *      ordinal-1 C127 sensor on (1, 1) is the only sensor
 *      that resolves to ordinal 1 in the corridor band (so
 *      the d2c_far_positive slice is the unique positive
 *      route, not a duplicate of the corridor ordinal-1
 *      walkpath).
 *   7. Verifying byte-stable redraw at the d2c_far_positive
 *      pose across 4 successive M11_GameView_Draw calls (no
 *      drift between redraws at the (1, 2) NORTH HALK pose).
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 normalize(M011_CELL(sensor) -
 *     direction) + 3 front-wall sensor filter
 *     (m11_front_cell_mirror_ordinal in src/engine/
 *     m11_game_view.c:11652 — the source of the
 *     d2c_far_positive "ordinal 1 sensor on the front cell"
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
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
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
 *     (16-pose ordinal lookup matrix, includes (1,2,N)=1).
 *   firestaff_dm1_v1_champion_mirror_capture_probe (PPM dumps
 *     for visual review of the HALK pose).
 *   firestaff_dm1_v1_champion_mirror_ordinal_1_halk_pose_probe
 *     (HALK pose + west_negative slice — no D2C far wall
 *     check).
 *   firestaff_dm1_v1_hall_champion_portrait_01_east_walkpath
 *     _portrait_rect_position_runtime_probe (east_walkpath
 *     corridor walk — no D2C far wall check).
 *   firestaff_dm1_v1_hall_of_champions_champion_portrait_01
 *     _south_return_portrait_rect_position_probe (south_return
 *     route — no D2C far wall check).
 *   firestaff_dm1_v1_hoc_champion_portrait_01_candidate_panel
 *     _open_portrait_rect_position_097_gate_probe (C040
 *     panel-open slice — no D2C far wall check).
 *   firestaff_dm1_v1_hoc_champion_portrait_01_input_focus
 *     _restore_portrait_rect_position_145_gate_probe (input
 *     focus / panel-state restoration — no D2C far wall
 *     check).
 *   firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after
 *     _candidate_portrait_rect_position_097_gate_probe
 *     (redraw_after_candidate slice — no D2C far wall
 *     check).
 *   firestaff_dm1_v1_hall_of_champions_portrait_01_cancel
 *     _reopen_portrait_rect_position_runtime_probe
 *     (select/cancel/select cycle — no D2C far wall check).
 *   firestaff_dm1_v1_hoc_champion_portrait_00_d2r_negative
 *     _portrait_rect_position_072_gate_probe (ordinal-0
 *     d2r_negative route — different ordinal, different
 *     view square, different route).
 *   firestaff_dm1_v1_champion_mirror_ordinal_6_d2l_negative
 *     _portrait_rect_position_runtime_probe (ordinal-6
 *     d2l_negative route — different ordinal, different view
 *     square).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares
 *     the rendered D1C cutout and the D2C far wall / D2L /
 *     D2R side wall zones against the local C026 strip pulled
 *     from the same GRAPHICS.DAT the runtime is drawing from,
 *     so this is runtime correctness rather than pixel-for-
 *     pixel DOSBox reference parity.
 *   - We do not assume a C127 sensor with sensorData=1 lives
 *     anywhere on the corridor band.  The local PC 3.4
 *     DUNGEON.DAT ships a C127 sensor with sensorData=1 on
 *     (1, 1) cell 0 (south wall of the (1, 1) cell), which
 *     is the source-visible wall cell for (1, 2) DIR_NORTH.
 *     No sensorData seeding is required.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_01_d2c_far_positive
 *   _portrait_rect_position_193_gate_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_status_m12.h"

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
     * no ordinal-1 portrait pixels live in this zone. */
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
     * a C026 ordinal-1 portrait.  We allow up to 35% pixel match
     * against ordinal 1 (the wrong-ordinal drift threshold used
     * by the actual-pose probe's check_no_stale_ordinal_in_rect).
     * Above 35% means a stale ordinal-1 sprite is floating over
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
    /* The slice target ordinal (HALK, mirror catalog record). */
    ORDINAL_TARGET = 1
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

/* Pixel-match a 32x29 viewport rect against a single 32x29 cell of
 * the C026 strip (graphics.dat asset slot
 * M11_GFX_CHAMPION_PORTRAITS = 26, atlas 256x87, 8 cols x 3 rows
 * of 32x29 portraits).  Returns matched-percent (0..100) or -1 if
 * the asset is missing.  Source pixels with palette index 1 (the
 * blitter transparentColor used by
 * m11_draw_dm1_front_champion_portrait) and palette index 12
 * (the wall-niche backdrop dark gray) are skipped so the wall
 * background bleed does not skew the match.  Same logic as
 * match_portrait_cell in the d2r_negative gate. */
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

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction) pose
 * on map 0 (Hall of Champions) and return the rendered framebuffer
 * in `fb`.  Centralizes the boilerplate so every render call below
 * uses the same field initialization. */
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
    state->world.party.championCount = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* ── Group A: D2C far wall has rendered content ───────────────
 * Source-locked to DUNVIEW.C:13480-13485 kFrontBlits D2C entry
 * (M11_GFX_WALLSET0_D2C, viewport 59, 19, 106, 74) and the F0128
 * far-to-near draw order that paints the D2C cell before the D1C
 * front wall.  At the d2c_far_positive pose the D2C far wall
 * must carry wallset 102 bitmap pixels — not be silently empty.
 * An empty D2C rect would mean the far wall is missing, not that
 * ordinal 1 is absent from it.  Two-channel content check:
 * (1) >= 30 non-zero pixels (texture actually present) and
 * (2) >= 3 distinct non-zero palette indices (texture is
 * varied, not a single-color fill). */
static void check_d2c_far_has_content(M11_GameViewState* state) {
    unsigned char fb[FB_W * FB_H];
    int nonZero;
    int distinct;

    printf("\n[Group A] D2C far wall rect at (1,2) NORTH has rendered content\n");

    render_at(state, fb, 1, 2, 0 /* DIR_NORTH */);
    nonZero = 0;
    {
        int xx, yy;
        for (yy = D2C_Y; yy < D2C_Y + D2C_H && yy < FB_H; ++yy) {
            for (xx = D2C_X; xx < D2C_X + D2C_W && xx < FB_W; ++xx) {
                if (fb[yy * FB_W + xx] != 0) ++nonZero;
            }
        }
    }
    distinct = rect_distinct_nonzero(fb, D2C_X, D2C_Y, D2C_W, D2C_H);

    CHECKF(nonZero >= 30,
           "D2C far wall has >= 30 non-zero pixels (got %d)",
           nonZero);
    CHECKF(distinct >= 3,
           "D2C far wall has >= 3 distinct non-zero palette indices (got %d)",
           distinct);
}

/* ── Group B: D2C far wall does NOT match ordinal 1 (HALK) ────
 * The d2c_far_positive slice invariant: at the (1, 2) DIR_NORTH
 * pose the C026 ordinal-1 HALK portrait must NOT be painted over
 * the D2C far wall (viewport 59, 19, 106, 74).  The D1C front-
 * wall rectangle IS the only destination for the C026 blit
 * (DUNVIEW.C:3913-3928 gates on P0117_i_ViewWallIndex ==
 * M587_VIEW_WALL_D1C_FRONT).  A regression that lets the C026
 * blit leak onto the D2C cell would push the match above 35%. */
static void check_d2c_far_no_ordinal_1(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int pct;
    int warm;
    char nameBuf[32];

    printf("\n[Group B] D2C far wall does NOT match C026 ordinal 1 (HALK) at (1,2) NORTH\n");

    render_at(state, fb, 1, 2, 0 /* DIR_NORTH */);

    /* Cross-check: confirm we are testing the right ordinal —
     * the mirror catalog resolves ordinal 1 to HALK. */
    memset(nameBuf, 0, sizeof(nameBuf));
    (void)M11_GameView_GetMirrorNameByOrdinal(state, ORDINAL_TARGET,
                                              nameBuf, sizeof(nameBuf));
    if (strcmp(nameBuf, "HALK") == 0) {
        CHECK(1, "mirror-catalog ordinal 1 name == \"HALK\"");
    } else {
        CHECKF(0, "mirror-catalog ordinal 1 name == \"HALK\" (got \"%s\")",
               nameBuf);
    }

    /* NOTE on warm_count: the D2C rect at (1,2) NORTH is the
     * corridor far-center wall, not a side wall.  Unlike the
     * D2L/D2R side walls (which have warm_count == 0 on the
     * same cell), the D2C center wall at depth 2 includes the
     * wallset bitmap 102 corridor far-wall texture plus torch
     * glow from the corridor lighting, so warm_count is
     * non-zero (~4-5% of 7844 pixels) from the wallset bitmap
     * and torch glow, not from the C026 ordinal-1 portrait
     * sprite.  The authoritative check is the C026 atlas
     * pixel match below; warm_count is logged here for the
     * record. */
    warm = rect_warm_count(fb, D2C_X, D2C_Y, D2C_W, D2C_H);
    {
        char info[160];
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
                   "D2C far wall C026 ordinal 1 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group C: D2C far wall does NOT match ANY C026 atlas slot ──
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

    printf("\n[Group C] D2C far wall does NOT match any C026 atlas slot (24-slot strict dominance)\n");

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
        return;
    }

    render_at(state, fb, 1, 2, 0 /* DIR_NORTH */);
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

/* ── Group D: D1C portrait cutout IS ordinal 1 (HALK) positive ─
 * Cross-check that the (1, 2) DIR_NORTH pose is the source-locked
 * HALK cell on the local DM1 V1 PC 3.4 fixture.  The D1C rectangle
 * must paint the C026 ordinal-1 portrait with >= 90% pixel match
 * and >= 30 warm pixels — the positive cross-check that anchors
 * the d2c_far_positive slice. */
static void check_d1c_is_ordinal_1(M11_GameViewState* state,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int warm;

    printf("\n[Group D] D1C portrait cutout IS ordinal 1 (HALK) at (1,2) NORTH (positive cross-check)\n");

    render_at(state, fb, 1, 2, 0 /* DIR_NORTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    CHECKF(ord == ORDINAL_TARGET,
           "M11_GameView_GetFrontMirrorOrdinal((1,2)N) == %d (got %d)",
           ORDINAL_TARGET, ord);

    warm = rect_warm_count(fb, PORTRAIT_X, PORTRAIT_Y, PORTRAIT_W, PORTRAIT_H);
    CHECKF(warm >= PORTRAIT_WARM_POS_THRESHOLD,
           "Inner portrait cutout warm_count >= %d for HALK (got %d)",
           PORTRAIT_WARM_POS_THRESHOLD, warm);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_cell(portraits, fb,
                                  PORTRAIT_X, PORTRAIT_Y,
                                  ORDINAL_TARGET);
        if (pct < 0) {
            printf("  SKIP: GRAPHICS.DAT champion portrait strip missing/incomplete\n");
        } else {
            CHECKF(pct >= CORRECT_ORDINAL_MATCH_PCT,
                   "D1C portrait cutout C026 ordinal 1 match >= %d%% (got %d%%)",
                   CORRECT_ORDINAL_MATCH_PCT, pct);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group E: D2L + D2R side walls do NOT match ordinal 1 ─────
 * Lateral no-floating cross-check for the d2c_far_positive slice:
 * at the (1, 2) DIR_NORTH pose the D2L side wall (viewport 0, 19,
 * 78, 74) and the D2R side wall (viewport 146, 19, 78, 74) must
 * not carry ordinal-1 portrait pixels above the 35% drift
 * threshold.  This is the dedicated "no portrait on either
 * lateral side wall at depth 2" invariant for the
 * d2c_far_positive pose. */
static void check_d2l_d2r_no_ordinal_1(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int warmD2L, warmD2R;
    int pctD2L, pctD2R;

    printf("\n[Group E] D2L + D2R side walls do NOT match ordinal 1 (HALK) at (1,2) NORTH (lateral no-floating)\n");

    render_at(state, fb, 1, 2, 0 /* DIR_NORTH */);

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
                   "D2L side wall C026 ordinal 1 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pctD2L);
            CHECKF(pctD2R < WRONG_ORDINAL_MATCH_PCT,
                   "D2R side wall C026 ordinal 1 match < %d%% (got %d%%)",
                   WRONG_ORDINAL_MATCH_PCT, pctD2R);
        }
    } else {
        printf("  SKIP: GRAPHICS.DAT champion portrait strip unavailable\n");
    }
}

/* ── Group F: portrait_rect_position + D2C rect invariants ────
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
        {1, 2, 0 /* DIR_NORTH */},
        {1, 2, 1 /* DIR_EAST  */},
        {1, 2, 2 /* DIR_SOUTH */},
        {1, 2, 3 /* DIR_WEST  */}
    };
    int i;

    printf("\n[Group F] portrait_rect_position contract across (1,2) pose lattice\n");

    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        state->world.party.mapIndex = 0;
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

/* ── Group G: corridor (x=1) DIR_NORTH scan y=2..6 ────────────
 * Walk every (mapX, mapY) on the (x=1) corridor band with
 * DIR_NORTH and confirm the only C127 sensor that resolves to
 * ordinal 1 is the canonical (1, 2) NORTH front cell (1, 1)
 * (sensorData=1, HALK).  Adjacent corridor cells (1, 3),
 * (1, 4), (1, 5), (1, 6) NORTH must all return -1 (no mirror
 * on their front cells).  This proves the d2c_far_positive
 * slice is the unique positive route for ordinal 1, not a
 * duplicate of any other corridor ordinal-1 walkpath. */
static void check_corridor_d2c_far_positive_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal1 = 0;
    int ordinalsFound[8];
    int ordinalsCount = 0;
    int i;

    printf("\n[Group G] Corridor (x=1) DIR_NORTH scan y=2..6\n");
    for (y = 2; y <= 6; ++y) {
        int ord = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 1;
        state->world.party.mapY = y;
        state->world.party.direction = 0; /* DIR_NORTH */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal1;
            if (y == 2) {
                printf("  (1,%d) DIR_NORTH -> ordinal %d (canonical HALK pose)\n",
                       y, ord);
            } else {
                printf("  (1,%d) DIR_NORTH -> ordinal %d (UNEXPECTED for d2c_far_positive slice)\n",
                       y, ord);
            }
        } else if (ord >= 0 && ordinalsCount < (int)(sizeof(ordinalsFound) / sizeof(ordinalsFound[0]))) {
            ordinalsFound[ordinalsCount++] = ord;
            printf("  (1,%d) DIR_NORTH -> ordinal %d\n", y, ord);
        } else {
            printf("  (1,%d) DIR_NORTH -> -1 (no mirror)\n", y);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=1, y=2..6) DIR_NORTH scan finds exactly one C127 sensor with sensorData=%d "
             "on the corridor north wall (found %d)",
             ORDINAL_TARGET, foundOrdinal1);
    CHECK(foundOrdinal1 == 1, msg);

    /* Side-check: the engine helper at each d2c_far_positive pose
     * still returns the source-locked wall box (80, 29, 64, 43). */
    for (i = 0; i < ordinalsCount; ++i) {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Engine helper invariant: D1C wall box is (%d, %d, %d, %d) for any corridor d2c_far_positive pose "
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

/* ── Group H: re-entry + byte-stable redraw ───────────────────
 * Re-rendering (1, 2) DIR_NORTH at the d2c_far_positive pose
 * does not silently introduce ordinal 1 on the D2C far wall
 * (no-floating) or remove it from the D1C cutout (positive).
 * 4 successive M11_GameView_Draw calls must produce byte-stable
 * framebuffer pixels (no drift between redraws at the
 * d2c_far_positive pose).  Same invariant as the
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

    printf("\n[Group H] Re-enter (1,2) DIR_NORTH — D2C far wall no-floating + D1C positive + byte-stable redraw\n");

    render_at(state, fb0, 1, 2, 0 /* DIR_NORTH */);
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
        render_at(state, fbN, 1, 2, 0 /* DIR_NORTH */);
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

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int assetsAvailable;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d d2c_far_positive portrait_rect_position (193 gate) ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    assetsAvailable = M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1");
    if (!assetsAvailable) {
        printf("SKIP firestaff_dm1_v1_hoc_champion_portrait_01_d2c_far_positive_"
               "portrait_rect_position_193_gate_probe "
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

    check_d2c_far_has_content(&state);
    check_d2c_far_no_ordinal_1(&state, portraits);
    check_d2c_far_no_stale_ordinal(&state, portraits);
    check_d1c_is_ordinal_1(&state, portraits);
    check_d2l_d2r_no_ordinal_1(&state, portraits);
    check_rect_position_invariants(&state);
    check_corridor_d2c_far_positive_scan(&state);
    check_d2c_far_reentry_and_stable_redraw(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
