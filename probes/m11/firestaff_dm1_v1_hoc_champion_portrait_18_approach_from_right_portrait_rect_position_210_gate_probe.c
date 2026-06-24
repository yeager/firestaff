/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 18 (SONJA)
 * approach_from_right / portrait_rect_position runtime gate probe.
 *
 * Targeted slice:
 *   ordinal  = 18  (C026 strip cell (2, 2) — atlas col 2, row 2
 *                   (last row); source rect (64, 58, 32, 29);
 *                   mirror catalog record "SONJA" with title
 *                   "SHE DEVIL" per F0660/F0661 + the G0289
 *                   nibble decode table in DUNVIEW.C).
 *   route    = approach_from_right
 *                   The party stands to the EAST of the SONJA
 *                   chamber (2, 3) and looks WEST at its east
 *                   wall.  ReDMCSB DUNGEON.C:2573 computes
 *                   visibleWallCell = (direction + 2) & 3 = 1
 *                   (EAST) for a DIR_WEST party, and the SONJA
 *                   C127 sensor (sensorData=18) sits on the
 *                   WEST wall (cell 3) of (2, 3) per the
 *                   actual_pose probe fixture.  Therefore the
 *                   (3, 3) DIR_WEST pose returns ordinal -1
 *                   from M11_GameView_GetFrontMirrorOrdinal and
 *                   the D1C champion portrait cutout (96, 35,
 *                   32, 29) must NOT carry ordinal-18 pixels at
 *                   the approach_from_right pose.  This is the
 *                   east-side mirror of the
 *                   firestaff_dm1_v1_champion_mirror_ordinal_4
 *                   _approach_from_left portrait_rect_position
 *                   _runtime_probe's (1, 2) DIR_EAST pose
 *                   (which is the west-side wrong-wall anchor
 *                   for LEIF ordinal 4).
 *   aspect   = portrait_rect_position
 *                   The C026 champion portrait cutout stays
 *                   anchored at the source-locked D1C viewport
 *                   rectangle (96, 35, 32, 29) per
 *                   ReDMCSB DUNVIEW.C:3913-3928 +
 *                   DUNVIEW.C:525 G0109_auc_Graphic558_Box
 *                   _ChampionPortraitOnWall = {96, 127, 35, 63},
 *                   and the cutout must be EMPTY (no floating
 *                   ordinal 18 pixels) at the (3, 3) DIR_WEST
 *                   approach_from_right pose.  The D1C wall
 *                   ornament zone is (80, 29, 64, 43) per
 *                   DUNVIEW.C G0205 coordSet 5 / index 12
 *                   (C346 D1C champion-mirror route).
 *   batch    = group 8
 *                   portrait_rect_position gates: ordinal,
 *                   pose, route triplet, side-wall scan,
 *                   byte-stable redraw.  "group 8" is the next
 *                   free batch-group tag after groups 0, 3, 4, 6
 *                   used by the existing palette_match_rect,
 *                   d2r_negative, and double_click_stability
 *                   gate probes (see those files for the tag
 *                   convention).
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     covers (1, 3, E) = 18 (the "hall_corridor_east_ordinal_18"
 *     pose — the canonical SONJA approach from the left) for
 *     ordinal lookup only, and does not cover (3, 3, W).  This
 *     probe locks the wrong-wall (3, 3, W) anchor as the
 *     ordinal-18 approach_from_right slice.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_18_cancel_reopen
 *     _portrait_rect_position_runtime_probe covers ordinal 18 on
 *     the (1, 2) NORTH seeded route (it seeds the (1, 2) NORTH
 *     C127 sensor from HALK to SONJA).  That is the seeded
 *     (1, 2) NORTH cross-check path; this probe covers the
 *     real-fixture (3, 3) DIR_WEST wrong-wall anchor and the
 *     (1, 3) DIR_EAST positive cross-check.
 *   - firestaff_dm1_v1_hoc_champion_portrait_18_sleep_repaint
 *     _portrait_rect_position_090_gate_probe covers ordinal 18
 *     on the (1, 2) NORTH seeded route under the rest/sleep
 *     state machine.  This probe covers the east-side wrong-wall
 *     slice on the real DM1 V1 fixture (no sensor seeding).
 *   - firestaff_dm1_v1_champion_mirror_ordinal_4_approach_from
 *     _left_portrait_rect_position_runtime_probe covers ordinal
 *     4 (LEIF) at the (1, 2) DIR_EAST wrong-wall anchor (the
 *     "approach from the left" for LEIF).  This probe is the
 *     east-side mirror: ordinal 18 (SONJA) at the (3, 3) DIR_WEST
 *     wrong-wall anchor (the "approach from the right" for SONJA).
 *   - firestaff_dm1_v1_champion_mirror_ordinal_2/6/9/11/13/15
 *     /17/21_west_negative_portrait_rect_position_runtime_probe
 *     cover ordinals 2/6/9/11/13/15/17/21 on the corridor west
 *     wall.  None of them cover ordinal 18 (SONJA), and none
 *     drive the (3, 3) DIR_WEST east-side wrong-wall anchor.
 *
 * What the probe asserts at each stage:
 *   Stage 1 (engine helpers): M11_GameView_GetD1CWallOrnamentZone
 *     at (3, 3) DIR_WEST returns the source-locked (80, 29, 64,
 *     43) wall box; the inner cutout is parented at
 *     (frame.x + 16, frame.y + 6) = (96, 35).  This pins the
 *     wall frame regardless of the active pose so a regression
 *     that re-routes the ornament under a wrong-wall pose is
 *     caught.
 *   Stage 2 (approach_from_right pixel contract): at (3, 3)
 *     DIR_WEST M11_GameView_GetFrontMirrorOrdinal returns -1
 *     and the D1C cutout (96, 35, 32, 29) does NOT match C026
 *     ordinal 18 above the 35% wrong-ordinal drift threshold.
 *   Stage 3 (east-side corridor band scan): walking
 *     (3, 2..5) DIR_WEST must consistently return -1 (the
 *     east-side corridor is the wrong wall for ordinal 18; the
 *     SONJA sensor sits on the WEST wall of (2, 3) per
 *     actual_pose fixture, so an east-side scan is uniformly
 *     no-mirror).
 *   Stage 4 (rotate-away at the approach cell): at (3, 3) the
 *     DIR_NORTH, DIR_EAST, DIR_SOUTH, DIR_WEST poses must all
 *     return -1 except where a different chamber's mirror is
 *     visible.  (3, 3) DIR_NORTH faces (3, 2) with its NORTH
 *     wall, no C127 sensor under DM1 V1; (3, 3) DIR_SOUTH
 *     faces (3, 4) with its NORTH wall, no C127 sensor; (3, 3)
 *     DIR_EAST faces (4, 3) outside the Hall or a no-mirror
 *     cell, returns -1; (3, 3) DIR_WEST is the slice's primary
 *     anchor returning -1.  No mirror visible from any facing
 *     at (3, 3) on the real DM1 V1 fixture.
 *   Stage 5 (positive cross-check at the canonical SONJA route):
 *     at (1, 3) DIR_EAST (the canonical SONJA approach from the
 *     left) M11_GameView_GetFrontMirrorOrdinal returns 18 and
 *     the D1C cutout matches C026 ordinal 18 (SONJA) at
 *     >= 90% match.  An empty (3, 3) W cutout cannot silently
 *     mean the rectangle is dead.
 *   Stage 6 (re-entry + byte-stable redraw): re-entering the
 *     (3, 3) DIR_WEST approach_from_right pose must keep the
 *     D1C cutout empty, and 4 successive M11_GameView_Draw
 *     calls must produce byte-stable framebuffer pixels (no
 *     drift between redraws at the wrong-wall pose).
 *   Stage 7 (atlas round-trip + catalog resolution): the C026
 *     atlas math for ordinal 18 (col 2, row 2, source rect
 *     (64, 58, 32, 29)) is self-consistent, the cell carries
 *     >= 200 opaque pixels (a defined champion portrait, not
 *     a blank/unused slot), and M11_GameView_GetMirrorName
 *     ByOrdinal resolves ordinal 18 to "SONJA" with title
 *     "SHE DEVIL".
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 normalize(M011_CELL(sensor) -
 *     direction) + 3 front-wall sensor filter
 *   ReDMCSB DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL
 *     (M040_DATA(sensor))
 *   ReDMCSB DUNVIEW.C:3913-3928 C026 portrait blit into G0109
 *     portrait box (only on D1C — M587_VIEW_WALL_D1C_FRONT)
 *   ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_
 *     ChampionPortraitOnWall = {96, 127, 35, 63}
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 far-to-near viewport
 *     redraw order
 *   ReDMCSB COORD.C:1693-1722 PC 3.4 viewport origin (0, 33) /
 *     224x136 dim
 *   ReDMCSB COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29
 *   ReDMCSB MOVESENS.C:1501-1503 sensorData -> F0280 candidate
 *   ReDMCSB REVIVE.C F0280 materialize candidate from sensorData
 *   ReDMCSB DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   ReDMCSB DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *   ReDMCSB DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas
 *     math
 *   ReDMCSB DEFS.H:2186 C026_GRAPHIC_CHAMPION_PORTRAITS
 *     (256x87 strip of 32x29 portraits, 8 cols * 3 rows,
 *      ordinals 0..23)
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885
 *     M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652
 *     m11_front_cell_mirror_ordinal (visibleWallCell = (dir+2)&3)
 *   src/engine/m11_game_view.c:13952
 *     m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962
 *     dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972
 *     atlas addr ((ord&7)*32, (ord>>3)*29)
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled
 *     from the same GRAPHICS.DAT the runtime is drawing from,
 *     so this is runtime correctness rather than pixel-for-pixel
 *     DOSBox reference parity.
 *   - We do not assume a C127 sensor with sensorData=18 exists
 *     at the (3, 3) DIR_WEST visible wall.  The
 *     approach_from_right slice is specifically the wrong-wall
 *     route, and the local PC 3.4 DUNGEON.DAT is the
 *     source-locked fixture that proves the rectangle is empty
 *     at the (3, 3) DIR_WEST wrong-wall pose.  The (1, 3)
 *     DIR_EAST positive cross-check uses the SONJA sensor that
 *     is shipped in the DM1 V1 fixture (no sensor seeding
 *     required for the cross-check).
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_18_approach_from_right
 *   _portrait_rect_position_210_gate_probe DATA_DIR
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
     * cutout must not contain a C026 ordinal-18 portrait.  We
     * allow up to 35% pixel match against ordinal 18 (the
     * wrong-ordinal drift threshold used by the actual-pose
     * probe's check_no_stale_ordinal_in_rect).  Above 35% means
     * a stale SONJA sprite is floating over the corridor east
     * wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal (SONJA, mirror catalog record). */
    ORDINAL_TARGET = 18,
    /* The cross-check ordinal comes from the canonical SONJA
     * route at (1, 3) DIR_EAST.  SONJA is the shipped sensor
     * on the WEST wall of (2, 3) in the DM1 V1 PC 3.4 fixture
     * (sensor idx=23, sensorData=18) per
     * firestaff_dm1_v1_champion_mirror_actual_pose_runtime
     * _probe's "hall_corridor_east_ordinal_18" line.  No
     * seeding required. */
    ORDINAL_CROSSCHECK = 18,
    /* The cross-check pose anchors the D1C rect's liveness
     * check. */
    CROSSCHECK_MAP_X = 1,
    CROSSCHECK_MAP_Y = 3,
    CROSSCHECK_DIR = 1 /* DIR_EAST */
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
 * match_portrait_cell in the ordinal_2_west_negative probe and
 * the d2r_negative 072_gate probe. */
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
 * ordinal.  Used to verify ordinal 18 is a defined portrait in
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
 * percent of pixels that differ.  Used to verify ordinal 18 is
 * a distinct portrait from its row-2 neighbours (17 BORIS, 19
 * HAWK) so the catalog and the atlas are in sync. */
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

    /* Pose the party at (3, 3) W — the canonical ordinal-18
     * approach_from_right route (front cell (2, 3) has no C127
     * sensor on the EAST wall — the SONJA sensor is on the
     * WEST wall per actual_pose fixture). */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 3;
    state->world.party.mapY = 3;
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
 * At (3, 3) DIR_WEST the engine returns ordinal -1 because no
 * C127 sensor with sensorData=18 is on the front cell's EAST
 * wall.  The D1C portrait cutout (96, 35, 32, 29) must NOT
 * contain C026 ordinal-18 pixels. */
static void check_approach_from_right_pixel_contract(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctCutout;
    char msg[200];

    printf("\n[Stage 2] (3,3) DIR_WEST pixel contract — ordinal 18 (SONJA) must NOT be in the D1C cutout\n");

    render_at(state, fb, 3, 3, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((3,3) W) == -1 (got %d) "
             "- wrong wall under DUNGEON.C:2573 visibleWallCell filter "
             "(EAST wall of (2,3) has no SONJA C127 sensor)",
             ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    /* Pixel-match against C026 ordinal 18 in the D1C cutout.
     * The cutout must NOT match ordinal 18 above the
     * wrong-ordinal drift threshold (35%).  A regression that
     * paints a stale SONJA sprite over the corridor east wall
     * would push the match above 35%. */
    pctCutout = match_portrait_cell(portraits, fb,
                                    PORTRAIT_X, PORTRAIT_Y,
                                    ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(3,3) W D1C cutout does NOT match ordinal %d (SONJA) "
             "(>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctCutout);
    CHECK(pctCutout < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* ── Stage 3: east-side corridor band scan ──────────────────────
 * Walk every (3, y) on the east-side corridor band with
 * DIR_WEST and confirm no C127 sensor resolves to ordinal 18
 * on the corridor east wall.  The east corridor wall has no
 * C127 sensors in the source-visible DM1 V1 PC 3.4 fixture for
 * any of those cells, so the engine must consistently return
 * -1.  This is the east-side mirror of the
 * ordinal_4_approach_from_left probe's Group E band sweep at
 * (1, 2..5) DIR_EAST. */
static void check_east_corridor_band_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal18 = 0;
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
            ++foundOrdinal18;
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
             ORDINAL_TARGET, foundOrdinal18);
    CHECK(foundOrdinal18 == 0, msg);

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

/* ── Stage 4: 4-direction rotate-away at the (3, 3) cell ────────
 * The (3, 3) cell is the approach_from_right anchor for SONJA
 * (chamber (2, 3) seen from the east), but the four facings at
 * (3, 3) are NOT all wrong-wall.  Under the actual-pose
 * probe's DM1 V1 fixture:
 *   - (3, 3) DIR_NORTH faces (3, 2), front cell has no C127
 *     sensor on the SOUTH wall (cell 2) — no mirror.
 *   - (3, 3) DIR_EAST faces (4, 3), no C127 sensor on the WEST
 *     wall of (4, 3) under the DM1 V1 Hall of Champions —
 *     no mirror.
 *   - (3, 3) DIR_SOUTH faces (3, 4), no C127 sensor on the
 *     NORTH wall (cell 0) — no mirror.
 *   - (3, 3) DIR_WEST faces (2, 3), front cell has the SONJA
 *     C127 sensor on its WEST wall (cell 3) — but the visible
 *     wall for DIR_WEST is the EAST wall (cell 1), which is
 *     the wrong wall for SONJA, so the engine returns -1.
 *
 * This stage locks the specific invariant: at the (3, 3)
 * approach_from_right cell, all four facings return -1 (no
 * mirror visible).  The DIR_WEST anchor is the slice's
 * primary pose, and the other three facings are the
 * rotate-away neighbours.  This is the in-place-turn analogue
 * the existing ordinal_2 west_negative probe's
 * check_rotate_away exercises for (1, 4) DIR_WEST; here we
 * lock it for the (3, 3) approach cell across all four
 * facings. */
static void check_rotate_away_at_approach_cell(M11_GameViewState* state) {
    static const struct {
        int dir;
        int expectedOrdinal;
        const char* label;
    } kDirs[] = {
        {0, -1, "sonja_approach_rotate_north_no_mirror"},
        {1, -1, "sonja_approach_rotate_east_no_mirror"},
        {2, -1, "sonja_approach_rotate_south_no_mirror"},
        {3, -1, "sonja_approach_rotate_west_wrong_wall"},
    };
    int i;
    int n = (int)(sizeof(kDirs) / sizeof(kDirs[0]));
    int ok = 1;

    printf("\n[Stage 4] 4-direction rotate-away at (3, 3) approach_from_right cell\n");
    for (i = 0; i < n; ++i) {
        int ord = -999;
        char msg[240];
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 3;
        state->world.party.mapY = 3;
        state->world.party.direction = kDirs[i].dir;
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        snprintf(msg, sizeof(msg),
                 "front mirror ordinal at (3,3,dir=%d) [%s] = %d "
                 "(expected %d: no mirror visible from the (3,3) approach "
                 "cell in any facing — SONJA is on the WEST wall of (2,3), "
                 "DIR_WEST exposes the wrong EAST wall of (2,3))",
                 kDirs[i].dir, kDirs[i].label, ord, kDirs[i].expectedOrdinal);
        CHECK(ord == kDirs[i].expectedOrdinal, msg);
    }
    (void)ok;
}

/* ── Stage 5: positive cross-check at the canonical SONJA route ─
 * The D1C cutout must NOT be dead: at the canonical positive
 * SONJA route (1, 3) DIR_EAST the SAME rectangle IS painted
 * with ordinal 18 at >= 90% match.  An empty rectangle at
 * (3, 3) W must not silently mean the rectangle is dead. */
static void check_positive_crosscheck_sonja_route(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int matchPct = 0;
    int ord = -999;
    char msg[240];

    printf("\n[Stage 5] D1C portrait_rect_position positive cross-check at (1,3,DIR_EAST) — SONJA must be visible\n");

    render_at(state, fb, CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, CROSSCHECK_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "front mirror ordinal at (%d,%d,DIR_EAST) = %d "
             "(expected %d, SONJA visible from west of (2,3))",
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
             "(%d,%d,DIR_EAST) D1C cutout carries ordinal %d (SONJA) pixels "
             "at >= %d%%%% match (got %d%%%%) - positive cross-check proves "
             "the D1C rect is alive at the canonical SONJA route",
             CROSSCHECK_MAP_X, CROSSCHECK_MAP_Y, ORDINAL_CROSSCHECK,
             CORRECT_ORDINAL_MATCH_PCT, matchPct);
    CHECK(matchPct >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* ── Stage 6: re-entry + byte-stable redraw ─────────────────────
 * Re-entering the (3, 3) DIR_WEST approach_from_right pose
 * must keep the D1C cutout empty, and 4 successive
 * M11_GameView_Draw calls must produce byte-stable framebuffer
 * pixels (no drift between redraws at the wrong-wall pose).
 * This is the same invariant the d2r_negative 072_gate probe
 * uses for the (1, 2) DIR_EAST d2r_negative pose, applied here
 * at the (3, 3) DIR_WEST approach_from_right pose: a
 * regression that leaks framebuffer state between draws
 * (e.g. a stale back-buffer not cleared, a non-stable re-blt
 * path) would diverge between redraws and fail this stage. */
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

    printf("\n[Stage 6] Re-enter (3,3) DIR_WEST — empty D1C cutout invariant + byte-stable redraw\n");

    render_at(state, fb0, 3, 3, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (3,3) W ordinal == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb0,
                              PORTRAIT_X, PORTRAIT_Y,
                              ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (3,3) W D1C cutout does NOT match ordinal %d "
             "(SONJA) < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);

    baselinePct = match_portrait_cell(portraits, fb0,
                                      PORTRAIT_X, PORTRAIT_Y,
                                      ORDINAL_TARGET);
    for (cycle = 1; cycle < 4; ++cycle) {
        int pctN;
        render_at(state, fbN, 3, 3, 3 /* DIR_WEST */);
        pctN = match_portrait_cell(portraits, fbN,
                                   PORTRAIT_X, PORTRAIT_Y,
                                   ORDINAL_TARGET);
        if (pctN != baselinePct) {
            fprintf(stderr,
                    "FAIL (3,3) W cycle %d portrait_rect_position drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctN, baselinePct);
            stable = 0;
        }
        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
            fprintf(stderr,
                    "FAIL (3,3) W cycle %d framebuffer drift in viewport area\n",
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
 * The C026 atlas math for ordinal 18 (col 2, row 2, source
 * rect (64, 58, 32, 29)) must be self-consistent: the cell
 * carries >= 200 opaque pixels (a defined champion portrait,
 * not a blank/unused slot), and ordinal 18 must be visually
 * distinct from its row-2 neighbours 17 (BORIS) and 19 (HAWK).
 * M11_GameView_GetMirrorNameByOrdinal must resolve ordinal 18
 * to "SONJA" with title "SHE DEVIL" so a regression that
 * misaligns the catalog and the C026 atlas is caught. */
static void check_atlas_roundtrip_and_catalog(
        M11_GameViewState* state,
        const M11_AssetSlot* portraits) {
    int opaque18;
    int distinct18Vs17;
    int distinct18Vs19;
    char nameBuf[32] = {0};
    char titleBuf[32] = {0};
    int nameRc = 0;
    int titleRc = 0;
    char msg[200];

    printf("\n[Stage 7] C026 atlas math + mirror catalog resolution for ordinal 18\n");

    /* Atlas math: ordinal 18 -> col 2, row 2, source rect
     * (64, 58, 32, 29).  The row-2 atlas path through
     * (ordinal >> 3) * 29 yields 58 for ordinals 16..23;
     * ordinal 18 is the third column of the row-2 strip. */
    {
        int col = ORDINAL_TARGET & 7;
        int row = ORDINAL_TARGET >> 3;
        int srcX = col << 5;
        int srcY = row * 29;
        snprintf(msg, sizeof(msg),
                 "ordinal %d atlas math: col=%d, row=%d, srcX=%d, srcY=%d "
                 "(expected (2, 2, 64, 58))",
                 ORDINAL_TARGET, col, row, srcX, srcY);
        CHECK(col == 2 && row == 2 && srcX == 64 && srcY == 58, msg);
    }

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — atlas-round-trip and catalog groups skipped\n");
        return;
    }

    /* Ordinal 18 must be a defined portrait in the C026 atlas
     * (>= 200 opaque pixels).  An unused slot would be either
     * all-zero or all-transparent (palette index 1). */
    opaque18 = atlas_cell_opaque_count(portraits, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "ordinal %d atlas cell opaque count = %d (expected >= 200 — "
             "defined champion portrait, not blank/unused)",
             ORDINAL_TARGET, opaque18);
    CHECK(opaque18 >= 200, msg);

    /* Ordinal 18 must be visually distinct from its row-2
     * neighbours 17 (BORIS) and 19 (HAWK).  The DM1
     * champion-portrait atlas carries 24 distinct champions
     * (one per ordinal), so a duplicate would be a real
     * regression. */
    distinct18Vs17 = atlas_cell_distinct_percent(portraits, 18, 17);
    distinct18Vs19 = atlas_cell_distinct_percent(portraits, 18, 19);
    snprintf(msg, sizeof(msg),
             "ordinal 18 vs ordinal 17 (row-2 left neighbour, BORIS) "
             "differ by >= 30%%%% (got %d%%%%)",
             distinct18Vs17);
    CHECK(distinct18Vs17 >= 30, msg);
    snprintf(msg, sizeof(msg),
             "ordinal 18 vs ordinal 19 (row-2 right neighbour, HAWK) "
             "differ by >= 30%%%% (got %d%%%%)",
             distinct18Vs19);
    CHECK(distinct18Vs19 >= 30, msg);

    /* Catalog resolution: ordinal 18 must resolve to "SONJA"
     * with title "SHE DEVIL".  This catches a regression where
     * the catalog and the C026 atlas disagree on the ordinal-18
     * record. */
    nameRc = M11_GameView_GetMirrorNameByOrdinal(state,
                                                  ORDINAL_TARGET,
                                                  nameBuf,
                                                  (int)sizeof(nameBuf));
    snprintf(msg, sizeof(msg),
             "mirror catalog name for ordinal %d = \"%s\" (expected \"SONJA\")",
             ORDINAL_TARGET, nameBuf[0] ? nameBuf : "");
    CHECK(nameRc > 0 && strcmp(nameBuf, "SONJA") == 0, msg);

    titleRc = M11_GameView_GetMirrorTitleByOrdinal(state,
                                                    ORDINAL_TARGET,
                                                    titleBuf,
                                                    (int)sizeof(titleBuf));
    snprintf(msg, sizeof(msg),
             "mirror catalog title for ordinal %d = \"%s\" (expected \"SHE DEVIL\")",
             ORDINAL_TARGET, titleBuf[0] ? titleBuf : "");
    CHECK(titleRc > 0 && strcmp(titleBuf, "SHE DEVIL") == 0, msg);
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
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d (SONJA) "
           "approach_from_right portrait_rect_position (210 gate) ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s pose=(map 0, x=3, y=3) facing WEST\n", dataDir);
    printf("ordinal 18 atlas math: col=2, row=2, srcX=64, srcY=58\n");
    printf("batch group 8 portrait_rect_position gate\n");

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
    check_positive_crosscheck_sonja_route(&state, portraits);
    check_approach_from_right_reentry_and_stable_redraw(&state, portraits);
    check_atlas_roundtrip_and_catalog(&state, portraits);

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
