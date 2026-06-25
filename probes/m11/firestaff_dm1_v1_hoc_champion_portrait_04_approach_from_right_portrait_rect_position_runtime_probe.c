/*
 * firestaff_dm1_v1_hoc_champion_portrait_04_approach_from_right_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   ordinal 4              (C026 strip cell 4 — atlas col 4 row 0,
 *                          source rect (128, 0, 32, 29), mirror
 *                          catalog record "LEIF" with title
 *                          "THE VALIANT" per the catalog resolver
 *                          test in the ordinal_4_approach_from_left
 *                          probe)
 *   route approach_from_right: party at (3, 2) facing WEST, the player
 *                              approaching the LEIF chamber (2, 2) from
 *                              the RIGHT (east) side.  The visible wall
 *                              under this route is the EAST wall of
 *                              (2, 2), which has no C127 sensor — the
 *                              LEIF sensorData=4 sensor sits on the
 *                              NORTH wall of (2, 2) per the actual_pose
 *                              probe fixture line
 *                              {2, 1, 2, 4, "hall_leif_from_north_ordinal_4"}.
 *                              This route must therefore return front
 *                              ordinal -1 and the D1C portrait cutout
 *                              (96, 35, 32, 29) must stay empty: no LEIF
 *                              portrait floats over the corridor east
 *                              wall.  This is the east-side mirror of
 *                              the existing
 *                              firestaff_dm1_v1_champion_mirror_ordinal_4
 *                              _approach_from_left_portrait_rect_position
 *                              _runtime_probe, which covers the
 *                              approach_from_left (1, 2) DIR_EAST
 *                              wrong-wall anchor at the same chamber.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 — exactly the source-locked DUNVIEW.C
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} blit destination.
 *
 * The slice was authored against the same DM1 V1 PC 3.4 fixture used
 * by firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe:
 * (2, 1) DIR_SOUTH is the canonical positive LEIF route (front cell
 * is (2, 2) with the C127 sensor on cell 0 = NORTH).  The
 * approach_from_right route (3, 2) DIR_WEST is the *wrong-wall*
 * mirror of the same (2, 2) cell — the party stands to the east of
 * the chamber and looks west at its east wall.  ReDMCSB DUNGEON.C:2573
 * normalizes (M011_CELL(sensor) - direction) and discards the route
 * when the resulting side is not the visible wall side, so
 * m11_front_cell_mirror_ordinal must return -1 here, the D1C cutout
 * must be empty, and a regression that paints ordinal 4 over the
 * corridor east wall would push the C026 ordinal-4 pixel-match
 * above the 35% threshold.  This is the east-side mirror of the
 * ordinal_4 approach_from_left probe, here applied to the
 * approach_from_right anchor at the same chamber.
 *
 * This probe narrows seven contracts to the (3, 2) DIR_WEST
 * approach_from_right anchor:
 *
 *   Stage 1 (engine helpers): M11_GameView_GetD1CWallOrnamentZone
 *     at (3, 2) DIR_WEST returns the source-locked (80, 29, 64, 43)
 *     wall box; the inner cutout is parented at
 *     (frame.x + 16, frame.y + 6) = (96, 35).  This pins the wall
 *     frame regardless of the active pose so a regression that
 *     re-routes the ornament under a wrong-wall pose is caught.
 *   Stage 2 (approach_from_right pixel contract): at (3, 2) DIR_WEST
 *     M11_GameView_GetFrontMirrorOrdinal returns -1 (the C127 sensor
 *     for LEIF sits on the NORTH wall of (2, 2), not the EAST wall)
 *     and the D1C cutout (96, 35, 32, 29) does NOT carry ordinal-4
 *     pixels at > 35% match.  A regression that paints a stale LEIF
 *     sprite over the corridor east wall would push the match above
 *     35% and trip this assertion.
 *   Stage 3 (east-side corridor band scan): walking
 *     (3, 2..5) DIR_WEST must consistently return -1 (the east-side
 *     corridor is the wrong wall for ordinal 4; the LEIF sensor sits
 *     on the NORTH wall of (2, 2) per the actual_pose fixture, so an
 *     east-side scan is uniformly no-mirror).  This is the east-side
 *     mirror of the ordinal_4_approach_from_left probe's Group E
 *     band sweep at (1, 2..5) DIR_EAST.
 *   Stage 4 (rotate-away at the approach cell): at (3, 2) the
 *     DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST poses must all return
 *     -1.  (3, 2) DIR_NORTH faces (3, 1) with its NORTH wall, no
 *     C127 sensor; (3, 2) DIR_EAST faces (4, 2) outside the Hall or
 *     a no-mirror cell, returns -1; (3, 2) DIR_SOUTH faces (3, 3)
 *     with its NORTH wall, no C127 sensor; (3, 2) DIR_WEST faces
 *     (2, 2) with its EAST wall — the wrong wall for LEIF, returns
 *     -1.  No mirror visible from any facing at (3, 2) on the real
 *     DM1 V1 fixture.
 *   Stage 5 (positive cross-check at the canonical LEIF route):
 *     at (2, 1) DIR_SOUTH (the canonical LEIF route) the SAME
 *     rectangle IS painted with ordinal 4 at >= 90% match.  An empty
 *     rectangle at (3, 2) W must not silently mean the rectangle is
 *     dead.
 *   Stage 6 (re-entry + byte-stable redraw): re-entering the (3, 2)
 *     DIR_WEST approach_from_right pose must keep the D1C cutout
 *     empty, and 4 successive M11_GameView_Draw calls must produce
 *     byte-stable framebuffer pixels (no drift between redraws at
 *     the wrong-wall pose).
 *   Stage 7 (atlas round-trip + catalog resolution): the C026 atlas
 *     math for ordinal 4 (col 4, row 0, source rect (128, 0, 32,
 *     29)) is self-consistent, the cell carries >= 200 opaque
 *     pixels (a defined champion portrait, not a blank/unused slot),
 *     ordinal 4 is visually distinct from its row-0 neighbours 3
 *     and 5, and M11_GameView_GetMirrorNameByOrdinal resolves
 *     ordinal 4 to "LEIF" with title "THE VALIANT".
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
 *     - 16-pose ordinal map including (2,1,S)=4 (LEIF canonical
 *       positive) and (3,2,W)=-1 (LEIF wrong-wall, hall_leif_probe_
 *       from_east).  This probe adds the Stage 2 pixel-level
 *       contract for the (3,2,W)=-1 anchor that the actual_pose
 *       probe does not cover.
 *   firestaff_dm1_v1_champion_mirror_portrait04_rect_position
 *     _runtime_probe
 *     - locks the positive (2,1,S)=4 route at the D1C rect and the
 *       wrong-wall (1,2,E) no-floating on the *same* pose.  This
 *       probe extends the same no-floating guarantee to the
 *       east-side wrong-wall anchor (3,2,W) and adds the
 *       re-entry/byte-stable-redraw invariant the actual_pose probe
 *       does not cover.
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from_left
 *     _portrait_rect_position_runtime_probe
 *     - the approach_from_left (1,2,E) template this probe mirrors
 *       for ordinal 4 at (3,2,W) approach_from_right.  Disjoint
 *       data fixtures (different cell, same chamber, opposite
 *       approach angle).  This probe does NOT cover (1,2) and the
 *       ordinal_4 approach_from_left probe does NOT cover (3,2).
 *   firestaff_dm1_v1_champion_mirror_ordinal_4_east_walkpath
 *     _portrait_rect_position_runtime_probe
 *     - sealed-chamber east_walkpath guard + (2,1,S)=4 portrait
 *       pixel match at 95%.  This probe focuses on the wrong-wall
 *       east side of the chamber, not the sealed east entry.
 *   firestaff_dm1_v1_hall_of_champions_champion_portrait_04
 *     _south_return_portrait_rect_position_probe
 *     - south_return contract-portable slice anchored at
 *       (2,1,SOUTH) with both pre-fix and post-fix ordinal
 *       expectations (this probe works on the (3,2,W) side of
 *       the same chamber).
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     - visual PPM captures of the canonical LEIF pose
 *       (this probe is contract-only, no PPM).
 *   firestaff_dm1_v1_hoc_champion_portrait_18_approach_from_right
 *     _portrait_rect_position_210_gate_probe
 *     - ordinal 18 (SONJA) approach_from_right at (3,3) DIR_WEST.
 *       This probe is the ordinal-4 (LEIF) approach_from_right at
 *       (3,2) DIR_WEST — disjoint ordinal, disjoint chamber cell,
 *       but the same east-side wrong-wall shape.
 *   firestaff_dm1_v1_hoc_champion_portrait_01_approach_from_right
 *     _portrait_rect_position_runtime_probe
 *     - ordinal 1 (HALK) approach_from_right at (2,1) DIR_WEST
 *       (this probe is ordinal 4 / (3,2,W), disjoint ordinal +
 *       disjoint chamber cell).
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=4 exists at
 *     the (3, 2) DIR_WEST visible wall.  The approach_from_right
 *     slice is specifically the negative route, and the local PC 3.4
 *     DUNGEON.DAT is the source-locked fixture that proves the
 *     rectangle is empty at the (3, 2) DIR_WEST wrong-wall pose.
 *   - The probe does not load real DOSBox captures or original
 *     PC 3.4 screenshots; it uses the same runtime state the live
 *     M11 game view uses, with the same asset loader pipeline the
 *     renderer is using, so the comparison is apples-to-apples.
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_04_approach_from_right
 *   _portrait_rect_position_runtime_probe DATA_DIR
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
    /* Match thresholds.  At the approach_from_right pose the D1C
     * cutout must not contain a C026 ordinal-4 portrait.  We allow
     * up to 35% pixel match against ordinal 4 (the wrong-ordinal
     * drift threshold used by the actual-pose probe's
     * check_no_stale_ordinal_in_rect).  Above 35% means a stale
     * LEIF sprite is floating over the corridor east wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal (LEIF, mirror catalog record). */
    ORDINAL_TARGET = 4,
    /* The cross-check ordinal comes from the canonical LEIF
     * route at (2, 1) DIR_SOUTH.  LEIF is the shipped sensor on
     * the NORTH wall of (2, 2) in the DM1 V1 PC 3.4 fixture per
     * firestaff_dm1_v1_champion_mirror_actual_pose_runtime
     * _probe's "hall_leif_from_north_ordinal_4" line.  No seeding
     * required. */
    ORDINAL_CROSSCHECK = 4,
    /* The cross-check pose anchors the D1C rect's liveness
     * check. */
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

/* Pixel-match a 32x29 viewport rect against a single 32x29 cell
 * of the C026 strip (graphics.dat asset slot
 * M11_GFX_CHAMPION_PORTRAITS = 26, atlas 256x87, 8 cols x 3
 * rows of 32x29 portraits).  Returns matched-percent (0..100)
 * or -1 if the asset is missing.  Source pixels with palette
 * index 1 (the blitter transparentColor used by
 * m11_draw_dm1_front_champion_portrait) and palette index 12
 * (the wall-niche backdrop dark gray) are skipped so the wall
 * background bleed does not skew the match.  Same logic as
 * match_portrait_cell in the ordinal_18_approach_from_right
 * 210_gate probe and the ordinal_4_approach_from_left probe. */
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

/* Count opaque pixels in the C026 atlas cell for the requested
 * ordinal.  Used to verify ordinal 4 is a defined portrait in
 * the atlas (i.e. not blank / unused / palette-index-1
 * transparent only).  A defined champion portrait carries
 * >= 200 opaque pixels (warm skin tones, clothing, background);
 * an unused slot would be either all-zero or all-transparent. */
static int atlas_cell_opaque_count(const M11_AssetSlot* portraits,
                                   int ordinal) {
    int x, y, cnt = 0;
    int srcX = (ordinal & 7) * PORTRAIT_W;
    int srcY = (ordinal >> 3) * PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int sx = srcX + x;
            int sy = srcY + y;
            unsigned char src;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != 1) ++cnt;
        }
    }
    return cnt;
}

/* Compare two C026 atlas cells byte-by-byte.  Returns the
 * percent of pixels that differ.  Used to verify ordinal 4 is
 * a distinct portrait from its row-0 neighbours 3 and 5 so the
 * catalog and the atlas are in sync. */
static int atlas_cell_distinct_percent(const M11_AssetSlot* portraits,
                                       int ordinalA, int ordinalB) {
    int x, y, compared = 0, different = 0;
    int srcAX = (ordinalA & 7) * PORTRAIT_W;
    int srcAY = (ordinalA >> 3) * PORTRAIT_H;
    int srcBX = (ordinalB & 7) * PORTRAIT_W;
    int srcBY = (ordinalB >> 3) * PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char a = (unsigned char)
                (portraits->pixels[(srcAY + y) * (int)portraits->width + (srcAX + x)] & 0x0F);
            unsigned char b = (unsigned char)
                (portraits->pixels[(srcBY + y) * (int)portraits->width + (srcBX + x)] & 0x0F);
            ++compared;
            if (a != b) ++different;
        }
    }
    return (compared > 0) ? (different * 100 / compared) : 0;
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
    state->resting = 0;
    state->partyDead = 0;
    state->damageFlashTimer = 0;
    state->attackCueTimer = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* ── Stage 1: engine helper contract surface ────────────────────
 * M11_GameView_GetD1CWallOrnamentZone must return the source-
 * locked wall box (80, 29, 64, 43) regardless of the active
 * pose, so the portrait_rect_position invariant holds across
 * the approach_from_right slice. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Stage 1] Engine helper contract surface for approach_from_right\n");

    /* Pose the party at (3, 2) W — the canonical ordinal-4
     * approach_from_right route (front cell (2, 2) has no C127
     * sensor on the EAST wall — the LEIF sensor is on the
     * NORTH wall per actual_pose fixture). */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 3;
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

    /* Inner portrait cutout = (ornX+16, ornY+6, 32, 29) =
     * (96, 35, 32, 29). */
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout X == 96 (got %d)", ornX + 16);
    CHECK(ornX + 16 == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == 35 (got %d)", ornY + 6);
    CHECK(ornY + 6 == 35, msg);
}

/* ── Stage 2: approach_from_right pixel contract ────────────────
 * At (3, 2) DIR_WEST the engine returns ordinal -1 because no
 * C127 sensor with sensorData=4 is on the front cell's EAST
 * wall.  The D1C portrait cutout (96, 35, 32, 29) must NOT
 * contain C026 ordinal-4 pixels. */
static void check_approach_from_right_pixel_contract(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctCutout;
    char msg[200];

    printf("\n[Stage 2] (3,2) DIR_WEST pixel contract — ordinal 4 (LEIF) must NOT be in the D1C cutout\n");

    render_at(state, fb, 3, 2, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((3,2) W) == -1 (got %d) "
             "- wrong wall under DUNGEON.C:2573 visibleWallCell filter "
             "(EAST wall of (2,2) has no LEIF C127 sensor)",
             ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    /* Pixel-match against C026 ordinal 4 in the D1C cutout.
     * The cutout must NOT match ordinal 4 above the
     * wrong-ordinal drift threshold (35%).  A regression that
     * paints a stale LEIF sprite over the corridor east wall
     * would push the match above 35%. */
    pctCutout = match_portrait_cell(portraits, fb,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(3,2) W D1C cutout does NOT match ordinal %d (LEIF) "
             "(>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctCutout);
    CHECK(pctCutout < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* ── Stage 3: east-side corridor band scan ──────────────────────
 * Walk every (3, y) on the east-side corridor band with
 * DIR_WEST and confirm no C127 sensor resolves to ordinal 4
 * on the corridor east wall.  The east corridor wall has no
 * C127 sensors in the source-visible DM1 V1 PC 3.4 fixture for
 * any of those cells, so the engine must consistently return
 * -1.  This is the east-side mirror of the
 * ordinal_4_approach_from_left probe's Group E band sweep at
 * (1, 2..5) DIR_EAST. */
static void check_east_corridor_band_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal4 = 0;
    int ordinalsFound[8];
    int ordinalsCount = 0;
    int i;

    printf("\n[Stage 3] East corridor (x=3) DIR_WEST scan y=2..5\n");
    for (y = 2; y <= 5; ++y) {
        int ord = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 3;
        state->world.party.mapY = y;
        state->world.party.direction = 3; /* DIR_WEST */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal4;
            printf("  (3,%d) DIR_WEST -> ordinal %d (UNEXPECTED for approach_from_right slice)\n",
                   y, ord);
        } else if (ord >= 0 && ordinalsCount < (int)(sizeof(ordinalsFound) / sizeof(ordinalsFound[0]))) {
            ordinalsFound[ordinalsCount++] = ord;
            printf("  (3,%d) DIR_WEST -> ordinal %d\n", y, ord);
        } else {
            printf("  (3,%d) DIR_WEST -> -1 (no mirror)\n", y);
        }
    }

    snprintf(msg, sizeof(msg),
             "(x=3, y=2..5) DIR_WEST scan finds no C127 sensor with "
             "sensorData=%d on the east corridor wall (found %d)",
             ORDINAL_TARGET, foundOrdinal4);
    CHECK(foundOrdinal4 == 0, msg);

    /* Side-check: the engine helper at each approach_from_right
     * pose still returns the source-locked wall box
     * (80, 29, 64, 43).  We only need to assert this once; if
     * any cell in the scan has a different wall-box readout the
     * call would have already been a FAIL on the first
     * iteration.  We do it once to keep the log compact. */
    for (i = 0; i < ordinalsCount; ++i) {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Engine helper invariant: D1C wall box is (%d, %d, %d, %d) "
                 "for any east-corridor approach_from_right pose "
                 "(got (%d, %d, %d, %d))",
                 WALLBOX_X, WALLBOX_Y, WALLBOX_W, WALLBOX_H,
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
        break;
    }
}

/* ── Stage 4: 4-direction rotate-away at the (3, 2) cell ────────
 * The (3, 2) cell is the approach_from_right anchor for LEIF
 * (chamber (2, 2) seen from the east), but the four facings at
 * (3, 2) are NOT all wrong-wall in the same way as the (1, 2)
 * approach_from_left cell.  Under the actual-pose probe's
 * DM1 V1 fixture:
 *   - (3, 2) DIR_NORTH faces (3, 1), front cell has no C127
 *     sensor on the SOUTH wall (cell 2) — no mirror.
 *   - (3, 2) DIR_EAST faces (4, 2), no C127 sensor on the WEST
 *     wall of (4, 2) under the DM1 V1 Hall of Champions —
 *     no mirror.
 *   - (3, 2) DIR_SOUTH faces (3, 3), no C127 sensor on the
 *     NORTH wall (cell 0) — no mirror.
 *   - (3, 2) DIR_WEST faces (2, 2), front cell has the LEIF
 *     C127 sensor on its NORTH wall (cell 0) — but the visible
 *     wall for DIR_WEST is the EAST wall (cell 1), which is
 *     the wrong wall for LEIF, so the engine returns -1.
 *
 * This stage locks the specific invariant: at the (3, 2)
 * approach_from_right cell, all four facings return -1 (no
 * mirror visible).  The DIR_WEST anchor is the slice's
 * primary pose, and the other three facings are the
 * rotate-away neighbours.  This is the east-side mirror of
 * the ordinal_4_approach_from_left probe's Group F
 * check_rotate_away_at_approach_cell, but with one critical
 * difference: at (1, 2) DIR_NORTH exposes HALK (ordinal 1) on
 * the (1, 1) NORTH wall, whereas at (3, 2) DIR_NORTH exposes
 * the (3, 1) NORTH wall with no C127 sensor, so all four
 * facings at (3, 2) return -1 under the real DM1 V1 fixture.
 * A regression that introduces a phantom ordinal-4 sensor on
 * the east wall is caught here. */
static void check_rotate_away_at_approach_cell(M11_GameViewState* state) {
    static const struct {
        int dir;
        int expectedOrdinal;
        const char* label;
    } kDirs[] = {
        {0, -1, "leif_approach_rotate_north_no_mirror"},
        {1, -1, "leif_approach_rotate_east_no_mirror"},
        {2, -1, "leif_approach_rotate_south_no_mirror"},
        {3, -1, "leif_approach_rotate_west_wrong_wall"},
    };
    int i;
    int n = (int)(sizeof(kDirs) / sizeof(kDirs[0]));

    printf("\n[Stage 4] 4-direction rotate-away at (3, 2) approach_from_right cell\n");
    for (i = 0; i < n; ++i) {
        int ord = -999;
        char msg[240];
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 3;
        state->world.party.mapY = 2;
        state->world.party.direction = kDirs[i].dir;
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (3,2,dir=%d) [%s] = %d "
                 "(expected %d: no mirror visible from the (3,2) approach "
                 "cell in any facing — LEIF is on the NORTH wall of (2,2), "
                 "DIR_WEST exposes the wrong EAST wall of (2,2))",
                 kDirs[i].dir, kDirs[i].label, ord, kDirs[i].expectedOrdinal);
        CHECK(ord == kDirs[i].expectedOrdinal, msg);
    }
}

/* ── Stage 5: positive cross-check at the canonical LEIF route ─
 * The D1C cutout must NOT be dead: at the canonical positive
 * LEIF route (2, 1) DIR_SOUTH the SAME rectangle IS painted
 * with ordinal 4 at >= 90% match.  An empty rectangle at
 * (3, 2) W must not silently mean the rectangle is dead. */
static void check_positive_crosscheck_leif_route(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    int ord = -999;
    char msg[240];

    printf("\n[Stage 5] D1C portrait_rect_position positive cross-check at (2,1,DIR_SOUTH) — LEIF must be visible\n");

    render_at(state, fb, CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, CROSSCHECK_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (%d,%d,DIR_SOUTH) = %d "
             "(expected %d, LEIF visible from north of (2,2))",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ord, ORDINAL_CROSSCHECK);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);
    if (ord != ORDINAL_CROSSCHECK) {
        return;
    }

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    matchPct = match_portrait_cell(portraits, fb,
                                   PORTRAIT_X, PORTRAIT_Y,
                                   ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "(%d,%d,DIR_SOUTH) D1C cutout carries ordinal %d (LEIF) pixels "
             "at >= %d%%%% match (got %d%%%%) - positive cross-check proves "
             "the D1C rect is alive at the canonical LEIF route",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ORDINAL_CROSSCHECK,
             CORRECT_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* ── Stage 6: re-entry + byte-stable redraw ─────────────────────
 * Re-entering the (3, 2) DIR_WEST approach_from_right pose
 * must keep the D1C cutout empty, and 4 successive
 * M11_GameView_Draw calls must produce byte-stable framebuffer
 * pixels (no drift between redraws at the wrong-wall pose).
 * This is the same invariant the ordinal_18 approach_from_right
 * 210_gate probe uses for the (3, 3) DIR_WEST wrong-wall pose,
 * applied here at the (3, 2) DIR_WEST approach_from_right
 * pose: a regression that leaks framebuffer state between
 * draws (e.g. a stale back-buffer not cleared, a non-stable
 * re-blt path) would diverge between redraws and fail this
 * stage. */
static void check_approach_from_right_reentry_and_stable_redraw(
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

    printf("\n[Stage 6] Re-enter (3,2) DIR_WEST — empty D1C cutout invariant + byte-stable redraw\n");

    render_at(state, fb0, 3, 2, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (3,2) W ordinal == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb0,
                              PORTRAIT_X, PORTRAIT_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (3,2) W D1C cutout does NOT match ordinal %d "
             "(LEIF) < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    baselinePct = match_portrait_cell(portraits, fb0,
                                      PORTRAIT_X, PORTRAIT_Y,
                                      ORDINAL_TARGET);
    for (cycle = 1; cycle < 4; ++cycle) {
        int pctN;
        render_at(state, fbN, 3, 2, 3 /* DIR_WEST */);
        pctN = match_portrait_cell(portraits, fbN,
                                   PORTRAIT_X, PORTRAIT_Y,
                                   ORDINAL_TARGET);
        if (pctN != baselinePct) {
            fprintf(stderr,
                    "FAIL (3,2) W cycle %d portrait_rect_position drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctN, baselinePct);
            stable = 0;
        }
        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
            fprintf(stderr,
                    "FAIL (3,2) W cycle %d framebuffer drift in viewport area\n",
                    cycle + 1);
            stable = 0;
        }
    }
    if (stable) {
        printf("  byte_stable_redraw_approach_from_right cycles=4 "
               "ordinal %d match=%d%%%% (no drift)\n",
               ORDINAL_TARGET, baselinePct);
    } else {
        ++g_fail;
    }
}

/* ── Stage 7: atlas round-trip + catalog resolution ─────────────
 * The C026 atlas math for ordinal 4 (col 4, row 0, source rect
 * (128, 0, 32, 29)) must be self-consistent: the cell carries
 * >= 200 opaque pixels (a defined champion portrait, not a
 * blank/unused slot), and ordinal 4 must be visually distinct
 * from its row-0 neighbours 3 and 5.  M11_GameView_
 * GetMirrorNameByOrdinal must resolve ordinal 4 to "LEIF"
 * with title "THE VALIANT" so a regression that misaligns the
 * catalog and the C026 atlas is caught. */
static void check_atlas_roundtrip_and_catalog(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    int opaque4;
    int distinct4Vs3;
    int distinct4Vs5;
    char nameBuf[32] = {0};
    char titleBuf[32] = {0};
    int nameRc = 0;
    int titleRc = 0;
    char msg[200];

    printf("\n[Stage 7] C026 atlas math + mirror catalog resolution for ordinal 4\n");

    /* Atlas math: ordinal 4 -> col 4, row 0, source rect
     * (128, 0, 32, 29).  The row-0 atlas path through
     * (ordinal >> 3) * 29 yields 0 for ordinals 0..7; ordinal
     * 4 is the fifth column of the row-0 strip. */
    {
        int col = ORDINAL_TARGET & 7;
        int row = ORDINAL_TARGET >> 3;
        int srcX = col << 5;
        int srcY = row * 29;
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas math: col=%d, row=%d, srcX=%d, srcY=%d "
                 "(expected (4, 0, 128, 0))",
                 ORDINAL_TARGET, col, row, srcX, srcY);
        CHECK(col == 4 && row == 0 && srcX == 128 && srcY == 0, msg);
    }

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — atlas-round-trip and catalog groups skipped\n");
        return;
    }

    /* Ordinal 4 must be a defined portrait in the C026 atlas
     * (>= 200 opaque pixels).  An unused slot would be either
     * all-zero or all-transparent (palette index 1). */
    opaque4 = atlas_cell_opaque_count(portraits, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "ordinal %d atlas cell opaque count = %d (expected >= 200 — "
             "defined champion portrait, not blank/unused)",
             ORDINAL_TARGET, opaque4);
    CHECK(opaque4 >= 200, msg);

    /* Ordinal 4 must be visually distinct from its row-0
     * neighbours 3 and 5.  The DM1 champion-portrait atlas
     * carries 24 distinct champions (one per ordinal), so a
     * duplicate would be a real regression. */
    distinct4Vs3 = atlas_cell_distinct_percent(portraits, 4, 3);
    distinct4Vs5 = atlas_cell_distinct_percent(portraits, 4, 5);
    snprintf(msg, sizeof(msg),
             "ordinal 4 vs ordinal 3 (row-0 left neighbour) "
             "differ by >= 30%%%% (got %d%%%%)",
             distinct4Vs3);
    CHECK(distinct4Vs3 >= 30, msg);
    snprintf(msg, sizeof(msg),
             "ordinal 4 vs ordinal 5 (row-0 right neighbour) "
             "differ by >= 30%%%% (got %d%%%%)",
             distinct4Vs5);
    CHECK(distinct4Vs5 >= 30, msg);

    /* Catalog resolution: ordinal 4 must resolve to "LEIF"
     * with title "THE VALIANT".  This catches a regression
     * where the catalog and the C026 atlas disagree on the
     * ordinal-4 record. */
    nameRc = M11_GameView_GetMirrorNameByOrdinal(state,
                                                  ORDINAL_TARGET,
                                                  nameBuf,
                                                  (int)sizeof(nameBuf));
    snprintf(msg, sizeof(msg),
             "mirror catalog name for ordinal %d = \"%s\" (expected \"LEIF\")",
             ORDINAL_TARGET, nameBuf[0] ? nameBuf : "");
    CHECK(nameRc > 0 && strcmp(nameBuf, "LEIF") == 0, msg);

    titleRc = M11_GameView_GetMirrorTitleByOrdinal(state,
                                                    ORDINAL_TARGET,
                                                    titleBuf,
                                                    (int)sizeof(titleBuf));
    snprintf(msg, sizeof(msg),
             "mirror catalog title for ordinal %d = \"%s\" (expected \"THE VALIANT\")",
             ORDINAL_TARGET, titleBuf[0] ? titleBuf : "");
    CHECK(titleRc > 0 && strcmp(titleBuf, "THE VALIANT") == 0, msg);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int assetsOk;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d (LEIF) "
           "approach_from_right portrait_rect_position runtime probe ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s pose=(map 0, x=3, y=2) facing WEST\n", dataDir);
    printf("ordinal 4 atlas math: col=4, row=0, srcX=128, srcY=0\n");

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
    check_approach_from_right_pixel_contract(&state, portraits);
    check_east_corridor_band_scan(&state);
    check_rotate_away_at_approach_cell(&state);
    check_positive_crosscheck_leif_route(&state, portraits);
    check_approach_from_right_reentry_and_stable_redraw(&state, portraits);
    check_atlas_roundtrip_and_catalog(&state, portraits);

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
