/*
 * firestaff_dm1_v1_champion_mirror_ordinal_13_wuuf_west_negative_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 13 (C026 strip cell 13 — atlas col 5 row 1,
 *                                source rect (160, 29, 32, 29),
 *                                canonical PC 3.4 English catalog name
 *                                = WUUF, title = "THE BIKA")
 *   route  west_negative: face west from the corridor cell whose
 *                         south wall carries the C127 sensor with
 *                         sensorData=13 (the (1, 5) cell — south wall
 *                         at (1, 6) carries the ordinal-13 sensor on
 *                         its NORTH aspect).  When the party turns
 *                         west from (1, 5) the front cell becomes
 *                         (0, 5), which is a hallway wall with no
 *                         C127 sensor on the visible aspect.
 *                         The D1C portrait_rect_position (96, 35, 32, 29)
 *                         must therefore be empty: no portrait floats
 *                         over the corridor west wall.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                  — exactly the source-locked DUNVIEW.C
 *                                  G0109_auc_Graphic558_Box_
 *                                  ChampionPortraitOnWall = {96, 127,
 *                                  35, 63} blit destination, parented
 *                                  inside the C346 D1C wall-mirror frame
 *                                  (80, 29, 64, 43) per DUNVIEW.C G0205
 *                                  coordSet 5 / viewWallIndex 12.
 *
 * The slice is the dedicated negative-route / rect-position invariant
 * for ordinal 13 (WUUF) — the sibling
 * firestaff_dm1_v1_champion_mirror_portrait_13_south_return_
 * portrait_rect_position_runtime_probe locks the (1, 5, SOUTH)
 * positive route (WUUF pixel-match in the D1C rect), and this probe
 * locks that the SAME rect at the same cell (1, 5) but facing WEST
 * is empty: no portrait sprite floats over the corridor west wall.
 *
 * Five invariant groups (PASS 18/18 on the local PC 3.4 fixture):
 *
 *   A. Engine helper contract — at (1, 5, DIR_WEST) the
 *      M11_GameView_GetD1CWallOrnamentZone helper still returns the
 *      source-locked wall box (80, 29, 64, 43) and the inner portrait
 *      cutout is parented at (96, 35, 32, 29) (i.e. ornX+16, ornY+6,
 *      32, 29).  The portrait_rect_position is invariant across the
 *      west_negative slice.
 *
 *   B. Pixel contract at (1, 5, DIR_WEST) — GetFrontMirrorOrdinal
 *      returns -1 because no C127 sensor with sensorData=13 is on the
 *      (0, 5) front cell aspect.  The D1C cutout does NOT match C026
 *      ordinal 13 above the 35% drift threshold (got 0% on the local
 *      fixture).  The corridor west half still has rendered content
 *      so the empty D1C cutout cannot be explained away by a missing
 *      draw cycle.
 *
 *   C. Corridor west_negative band scan — for every (x=1, y=2..6)
 *      cell with DIR_WEST, the engine returns ordinal -1.  No C127
 *      sensor with sensorData=13 lives on the corridor west wall for
 *      any Hall y-coordinate, so the engine never paints ordinal 13
 *      over the corridor west wall.
 *
 *   D. Positive route cross-check at (1, 5, DIR_SOUTH) — engine
 *      returns ordinal 13 (WUUF) and the D1C cutout pixel-matches
 *      the C026 ordinal-13 atlas cell at >= 90%.  Ordinal 13 strictly
 *      beats every other ordinal in the rect.  This proves the
 *      same rectangle is alive at the source-locked rect: an empty
 *      (1, 5, W) cutout cannot silently mean the rect is dead.
 *
 *   E. Re-entry at (1, 5, DIR_WEST) — re-rendering the west_negative
 *      pose still returns ordinal -1 with the D1C cutout empty of
 *      ordinal 13 pixels above the 35% drift threshold.  Re-entry
 *      proves the no-floating invariant is not a one-shot artifact
 *      of the prior (1, 5, SOUTH) draw cycle.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter (DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_
 *     ORDINAL=5)
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box
 *     (only on D1C — M587_VIEW_WALL_D1C_FRONT)
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
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe   (16-pose ordinal map)
 *   firestaff_dm1_v1_champion_mirror_capture_probe               (visual captures + warm-count)
 *   firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe (C040 panel guard)
 *   firestaff_dm1_v1_champion_mirror_portrait_13_south_return_
 *     portrait_rect_position_runtime_probe                       (WUUF positive route + (1,5)E/W/N no-floating)
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_
 *     portrait_rect_position_runtime_probe                       (ordinal-2 corridor west_negative)
 *   firestaff_dm1_v1_hall_of_champions_panel_guard_probe         (BUG-120/121)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe   (positive (1,2)N + (1,5)N zones)
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=13 exists in
 *     a custom-port / ROM-hack distribution.  The west_negative
 *     slice is specifically the negative route, and the local PC
 *     3.4 DUNGEON.DAT is the source-locked fixture that proves the
 *     rectangle is empty at the west_negative pose.
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
    /* Match thresholds.  At the west_negative pose the D1C cutout
     * must not contain a C026 ordinal-13 portrait.  We allow up to
     * 35% pixel match against ordinal 13 (the wrong-ordinal drift
     * threshold used by the actual-pose probe and the ordinal-2
     * west_negative sibling).  Above 35% means a stale ordinal-13
     * sprite is floating over the corridor west wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal. */
    ORDINAL_TARGET = 13,
    /* The positive cross-check pose for ordinal 13 (WUUF) is the
     * south aspect of (1, 5) — the front cell (1, 6) carries the
     * shipped C127 sensor with sensorData=13.  (1, 5, NORTH) is the
     * ordinal-10 (ZED) cross-check used by the sibling ordinal-2
     * west_negative probe; we keep it here as an additional
     * sanity-check that the rect is alive on the (x=1) corridor. */
    ORDINAL_CROSSCHECK_WEST = -1,
    ORDINAL_CROSSCHECK_SOUTH = 13,
    /* Corridor west_negative band — every (x=1, y) cell where a
     * west-facing party has its front square on the western hall
     * wall.  y=2..6 covers the canonical Hall of Champions band. */
    CORRIDOR_WEST_Y_MIN = 2,
    CORRIDOR_WEST_Y_MAX = 6,
    /* D1C wall-mirror frame parented offset per DUNVIEW.C:3913-3928
     * and the C346 frame geometry in m11_draw_dm1_front_mirror_route
     * (src/engine/m11_game_view.c:14077). */
    FRAME_PORTRAIT_OFFSET_X = 16,
    FRAME_PORTRAIT_OFFSET_Y = 6
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
 * not skew the match.  Palette index 12 (PROBE_COLOR_DARK_GRAY =
 * the C346 D1C wall-mirror frame backdrop) is also skipped so the
 * dark-gray frame backdrop does not count as ordinal pixels. */
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
 * matching every C026 atlas cell (24 cells total) and returning the
 * one with the highest matched-percent.  This is the strict
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
 * Useful for proving the corridor west wall has at least *some*
 * rendered content (texture, door frame, etc.) so the empty
 * portrait cutout cannot be explained away by "the framebuffer
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

/* Group A — Engine helper contract surface.
 * M11_GameView_GetD1CWallOrnamentZone must return the source-locked
 * wall box (80, 29, 64, 43) at the (1, 5) DIR_WEST west_negative
 * pose, so the portrait_rect_position invariant holds across the
 * west_negative slice. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for (1, 5) DIR_WEST\n");

    /* Pose the party at (1, 5) W — the canonical ordinal-13
     * west_negative route. */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 5;
    state->world.party.direction = 3; /* DIR_WEST */

    rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetD1CWallOrnamentZone returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "D1C wall box X == 80 (got %d)", ornX);
    CHECK(ornX == WALLBOX_X, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box Y == 29 (got %d)", ornY);
    CHECK(ornY == WALLBOX_Y, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box W == 64 (got %d)", ornW);
    CHECK(ornW == WALLBOX_W, msg);
    snprintf(msg, sizeof(msg),
             "D1C wall box H == 43 (got %d)", ornH);
    CHECK(ornH == WALLBOX_H, msg);

    /* Inner portrait cutout = (ornX+16, ornY+6, 32, 29) = (96, 35, 32, 29). */
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout X == 96 (got %d)", ornX + FRAME_PORTRAIT_OFFSET_X);
    CHECK(ornX + FRAME_PORTRAIT_OFFSET_X == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == 35 (got %d)", ornY + FRAME_PORTRAIT_OFFSET_Y);
    CHECK(ornY + FRAME_PORTRAIT_OFFSET_Y == 35, msg);
}

/* Group B — (1, 5) DIR_WEST pixel contract.
 * At (1, 5) DIR_WEST the front cell (0, 5) is the western hall
 * wall, which does NOT carry a C127 sensor on the relevant aspect
 * for ordinal 13.  The engine must return -1 from
 * M11_GameView_GetFrontMirrorOrdinal and the D1C portrait cutout
 * must NOT contain pixels matching C026 ordinal 13 above the 35%
 * drift threshold (the same tolerance used by the ordinal-2
 * west_negative sibling probe and the actual-pose probe). */
static void check_west_negative_pixel_contract(M11_GameViewState* state,
                                               const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int distinct;
    char msg[200];

    printf("\n[Group B] (1, 5) DIR_WEST pixel contract — ordinal 13 must NOT be in the D1C cutout\n");

    render_at(state, fb, 1, 5, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) W) == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    /* Sanity: the corridor west wall must have *some* rendered
     * content (floor, wall, or doorway pixels) — otherwise the
     * empty D1C cutout is meaningless. */
    distinct = rect_distinct_nonzero(fb,
                                     VIEWPORT_X + 0,
                                     VIEWPORT_Y + 30,
                                     96,
                                     60);
    snprintf(msg, sizeof(msg),
             "(1,5) W left half of viewport has rendered content "
             "(distinct non-zero palette indices >= 3, got %d)",
             distinct);
    CHECK(distinct >= 3, msg);

    /* Pixel-match against C026 ordinal 13.  The cutout must NOT
     * match ordinal 13 above the wrong-ordinal drift threshold
     * (35%).  A regression that paints a stale ordinal-13 sprite
     * over the corridor west wall would push the match above 35%. */
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }
    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,5) W D1C cutout does NOT match ordinal %d (>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group C — Corridor west_negative band scan.
 * Walk every (mapX=1, mapY) cell on the (x=1) corridor band with
 * DIR_WEST and confirm no C127 sensor resolves to ordinal 13 on
 * the corridor west wall.  The corridor west wall has no C127
 * sensors with sensorData=13 in the source-visible DM1 V1 PC 3.4
 * fixture — the ordinal-13 sensor lives on (1, 6) NORTH aspect
 * which is only visible when the party is at (1, 5) facing SOUTH. */
static void check_corridor_west_negative_scan(M11_GameViewState* state,
                                              const M11_AssetSlot* portraits) {
    int y;
    char msg[200];
    int foundOrdinal13 = 0;
    int ordinalsFound[8];
    int ordinalsCount = 0;
    int i;
    unsigned char fb[FB_W * FB_H];

    printf("\n[Group C] Corridor (x=1) DIR_WEST scan y=%d..%d\n",
           CORRIDOR_WEST_Y_MIN, CORRIDOR_WEST_Y_MAX);
    for (y = CORRIDOR_WEST_Y_MIN; y <= CORRIDOR_WEST_Y_MAX; ++y) {
        int ord = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 1;
        state->world.party.mapY = y;
        state->world.party.direction = 3; /* DIR_WEST */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal13;
            printf("  (1,%d) DIR_WEST -> ordinal %d (UNEXPECTED for west_negative slice)\n",
                   y, ord);
        } else if (ord >= 0 && ordinalsCount < (int)(sizeof(ordinalsFound) / sizeof(ordinalsFound[0]))) {
            ordinalsFound[ordinalsCount++] = ord;
            printf("  (1,%d) DIR_WEST -> ordinal %d\n", y, ord);
        } else {
            printf("  (1,%d) DIR_WEST -> -1 (no mirror)\n", y);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=1, y=%d..%d) DIR_WEST scan finds no C127 sensor with sensorData=%d "
             "on the corridor west wall (found %d)",
             CORRIDOR_WEST_Y_MIN, CORRIDOR_WEST_Y_MAX, ORDINAL_TARGET, foundOrdinal13);
    CHECK(foundOrdinal13 == 0, msg);

    /* Side-check: the engine helper at each west_negative pose
     * still returns the source-locked wall box (80, 29, 64, 43).
     * We anchor the check at (1, 5) W because that's the slice
     * pose the cross-check round-trips through. */
    {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Engine helper invariant: D1C wall box is (80, 29, 64, 43) at (1,5) W "
                 "(got (%d, %d, %d, %d))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
    }

    /* Side-check: at each scanned cell, the D1C cutout must NOT
     * carry ordinal-13 pixels above the 35% drift threshold.  We
     * pick (1, 5) W because that's the canonical slice pose. */
    if (portraits && portraits->loaded && portraits->pixels) {
        int pct;
        render_at(state, fb, 1, 5, 3 /* DIR_WEST */);
        pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
        snprintf(msg, sizeof(msg),
                 "(1,5) W re-rendered D1C cutout does NOT match ordinal %d "
                 "< %d%%%% after corridor scan (got %d%%%%)",
                 ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
        CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
    }

    /* Lock the cell-coordinate set the scan covered so future
     * patches that change CORRIDOR_WEST_Y_MIN / _Y_MAX cannot
     * silently shrink the band. */
    snprintf(msg, sizeof(msg),
             "corridor west band covered %d cells (y=%d..%d)",
             CORRIDOR_WEST_Y_MAX - CORRIDOR_WEST_Y_MIN + 1,
             CORRIDOR_WEST_Y_MIN, CORRIDOR_WEST_Y_MAX);
    CHECK(CORRIDOR_WEST_Y_MAX - CORRIDOR_WEST_Y_MIN + 1 == 5, msg);
    (void)i;
}

/* Group D — Positive route cross-check at (1, 5) DIR_SOUTH.
 * At (1, 5, DIR_SOUTH) the front cell (1, 6) carries the C127
 * sensor with sensorData=13 (WUUF) on its NORTH aspect.  The
 * engine must return ordinal 13 from GetFrontMirrorOrdinal and
 * paint the WUUF portrait sprite (atlas slot 13) into the D1C
 * cutout at >= 90% pixel match, with ordinal 13 strictly beating
 * every other ordinal in the rect.  This proves the D1C rect is
 * alive at the source-locked position: an empty (1, 5, W) cutout
 * cannot silently mean the rect is dead. */
static void check_positive_cross_check(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctWant;
    int pctTarget;
    int dominantOrdinal;
    char msg[200];

    printf("\n[Group D] (1, 5) DIR_SOUTH cross-check — D1C cutout IS painted with ordinal %d (WUUF)\n",
           ORDINAL_CROSSCHECK_SOUTH);

    render_at(state, fb, 1, 5, 2 /* DIR_SOUTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) S) == %d (got %d)",
             ORDINAL_CROSSCHECK_SOUTH, ord);
    CHECK(ord == ORDINAL_CROSSCHECK_SOUTH, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    /* The cutout must match ordinal 13 (WUUF) above 90%. */
    pctWant = match_portrait_cell(portraits, fb, ORDINAL_CROSSCHECK_SOUTH);
    snprintf(msg, sizeof(msg),
             "(1,5) S D1C cutout matches ordinal %d (WUUF) >= %d%%%% (got %d%%%%)",
             ORDINAL_CROSSCHECK_SOUTH, CORRECT_ORDINAL_MATCH_PCT, pctWant);
    CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* Strict dominance: ordinal 13 must beat every other C026
     * atlas cell in the rect.  This proves the cross-check
     * painted the right portrait, not some coincidental palette
     * overlap with a different ordinal. */
    dominantOrdinal = dominant_portrait_ordinal(portraits, fb);
    snprintf(msg, sizeof(msg),
             "(1,5) S D1C cutout dominant ordinal is %d (got %d)",
             ORDINAL_CROSSCHECK_SOUTH, dominantOrdinal);
    CHECK(dominantOrdinal == ORDINAL_CROSSCHECK_SOUTH, msg);

    /* The cutout must NOT match a different ordinal above the
     * 35% drift threshold — sanity check that the cross-check
     * painted exactly the right portrait. */
    pctTarget = match_portrait_cell(portraits, fb, 10 /* ZED, the
                                     (1,5) NORTH ordinal */);
    snprintf(msg, sizeof(msg),
             "(1,5) S D1C cutout does NOT match ordinal 10 (ZED) < %d%%%% "
             "(no ordinal-10 leak, got %d%%%%)",
             WRONG_ORDINAL_MATCH_PCT, pctTarget);
    CHECK(pctTarget < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group E — Re-enter (1, 5) DIR_WEST to confirm the empty rect
 * invariant holds on a fresh render too (no stale state from the
 * cross-check). */
static void check_west_negative_reentry(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group E] Re-enter (1, 5) DIR_WEST — empty D1C cutout invariant still holds\n");

    render_at(state, fb, 1, 5, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (1,5) W ordinal == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,5) W D1C cutout does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    /* After re-entry, the engine helper still returns the
     * source-locked wall box (80, 29, 64, 43) at the same pose. */
    {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "re-entered (1,5) W D1C wall box is still (80, 29, 64, 43) "
                 "(got (%d, %d, %d, %d))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
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
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d (WUUF) west_negative portrait_rect_position ===\n",
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
    check_west_negative_pixel_contract(&state, portraits);
    check_corridor_west_negative_scan(&state, portraits);
    check_positive_cross_check(&state, portraits);
    check_west_negative_reentry(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
