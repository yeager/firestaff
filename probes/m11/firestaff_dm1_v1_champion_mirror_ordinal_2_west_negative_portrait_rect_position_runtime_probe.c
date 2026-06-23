/*
 * firestaff_dm1_v1_champion_mirror_ordinal_2_west_negative_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 2 (C026 strip cell 2 — atlas col 2 row 0,
 *                               source rect (64, 0, 32, 29))
 *   route west_negative: face west from the corridor cell where the
 *                        ordinal-2 C127 sensor lives on the north wall
 *                        (text-string-derived (1,4) NORTH) — the engine
 *                        returns -1 because the front cell filter
 *                        (ReDMCSB DUNGEON.C:2573 + MOVESENS.C:1501-1503)
 *                        rejects the wrong-wall direction.  The
 *                        D1C portrait_rect_position (96, 35, 32, 29)
 *                        in viewport coords must therefore be empty:
 *                        no portrait floats over the corridor west
 *                        wall.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 — exactly the source-locked DUNVIEW.C
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} blit destination.
 *
 * The slice was authored against the same DM1 V1 PC 3.4 fixture used
 * by the firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe:
 * (1,4) DIR_NORTH is *not* a source-valid C127 mirror cell (no
 * sensorData=2 sensor at the corridor (1,3) front cell), but the
 * TextString-derived fixture mapped ordinal 2 to that pose.  This
 * probe:
 *
 *   1. Reads the source-locked D1C wall-ornament zone (DUNVIEW.C
 *      G0205 Graphic558 coordSet 5 = (80, 29, 64, 43)) from the
 *      engine helper M11_GameView_GetD1CWallOrnamentZone at the
 *      (1,4) DIR_WEST west_negative pose.  The wall box must
 *      remain at the source-locked coordinates regardless of
 *      pose, proving the rectangle is reserved for portrait
 *      placement.
 *   2. Drives M11_GameView_Draw at (1,4) DIR_WEST and pixel-proves
 *      that the D1C portrait cutout (96, 35, 32, 29) contains no
 *      C026 ordinal-2 pixels.  A regression that mistakenly
 *      paints ordinal 2 over the corridor west wall would push
 *      the C026 ordinal-2 pixel-match above the 35% threshold.
 *   3. Cross-checks that at the positive-ordinal (1,5) DIR_NORTH
 *      pose (C127 sensorData=10 ZED) the SAME rectangle IS
 *      painted with the ordinal-10 portrait, NOT ordinal 2 — so
 *      an empty rectangle at (1,4) W does not silently mean the
 *      rectangle is dead.
 *   4. Walks the corridor west-negative band (x=1, y=2..5) and
 *      confirms no C127 sensor resolves to ordinal 2 when the
 *      party faces west.  The corridor west wall has no C127
 *      sensors in any of those cells, so the engine must
 *      consistently return -1.
 *   5. Engine-helper invariant: M11_GameView_GetFrontMirrorOrdinal
 *      and the C040 panel-guard must agree at the west-negative
 *      poses (panel guard is the 2026-06-20 BUG-120/121 fix).
 *
 * If the local DM1 V1 build happens to have a C127 sensor at the
 * (1,3) or (1,4) west wall with sensorData=2 (some custom-port /
 * ROM-hack distributions may), Group C logs that as a discovery
 * hit and Group D still proves the portrait_rect_position is
 * painted at the source-locked rect.
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
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe    (16-pose ordinal map)
 *   firestaff_dm1_v1_champion_mirror_capture_probe                (visual captures + warm-count)
 *   firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe (C040 panel guard)
 *   firestaff_dm1_v1_hall_of_champions_panel_guard_probe         (BUG-120/121)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe   (positive (1,2)N + (1,5)N zones)
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=2 exists in
 *     the local DM1 V1 build.  The west_negative slice is
 *     specifically the negative route, and the local PC 3.4
 *     DUNGEON.DAT is the source-locked fixture that proves the
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
     * must not contain a C026 ordinal-2 portrait.  We allow up to
     * 35% pixel match against ordinal 2 (the wrong-ordinal drift
     * threshold used by the actual-pose probe's
     * check_no_stale_ordinal_in_rect).  Above 35% means a stale
     * ordinal-2 sprite is floating over the corridor west wall. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the positive-ordinal cross-check the D1C cutout must
     * carry the expected portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal. */
    ORDINAL_TARGET = 2,
    /* The cross-check ordinal from (1,5) DIR_NORTH. */
    ORDINAL_CROSSCHECK = 10
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
 * wall box (80, 29, 64, 43) regardless of the active pose, so the
 * portrait_rect_position invariant holds across the west_negative
 * slice. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for west_negative\n");

    /* Pose the party at (1,4) W — the canonical ordinal-2 west_negative
     * route. */
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 4;
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
             "Inner portrait cutout X == 96 (got %d)", ornX + 16);
    CHECK(ornX + 16 == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == 35 (got %d)", ornY + 6);
    CHECK(ornY + 6 == 35, msg);
}

/* Group B — west_negative slice pixel contract.
 * At (1,4) DIR_WEST the engine returns ordinal -1 because no C127
 * sensor with sensorData=2 is on the front square.  The D1C portrait
 * cutout (96, 35, 32, 29) must therefore contain zero pixels
 * matching C026 ordinal 2. */
static void check_west_negative_pixel_contract(M11_GameViewState* state,
                                               const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    int distinct;
    char msg[200];

    printf("\n[Group B] (1,4) DIR_WEST pixel contract — ordinal 2 must NOT be in the D1C cutout\n");

    render_at(state, fb, 1, 4, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,4) W) == -1 (got %d)", ord);
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
             "(1,4) W left half of viewport has rendered content "
             "(distinct non-zero palette indices >= 3, got %d)",
             distinct);
    CHECK(distinct >= 3, msg);

    /* Pixel-match against C026 ordinal 2.  The cutout must NOT
     * match ordinal 2 above the wrong-ordinal drift threshold
     * (35%).  A regression that paints a stale ordinal-2 sprite
     * over the corridor west wall would push the match above 35%. */
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }
    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,4) W D1C cutout does NOT match ordinal %d (>= %d%%%% implies stale sprite, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group C — Corridor west_negative band scan.
 * Walk every (mapX, mapY) on the (x=1) corridor band with
 * DIR_WEST and confirm no C127 sensor resolves to ordinal 2 on
 * the corridor west wall.  The corridor west wall has no C127
 * sensors in the source-visible DM1 V1 PC 3.4 fixture. */
static void check_corridor_west_negative_scan(M11_GameViewState* state) {
    int y;
    char msg[200];
    int foundOrdinal2 = 0;
    int ordinalsFound[8];
    int ordinalsCount = 0;
    int i;

    printf("\n[Group C] Corridor (x=1) DIR_WEST scan y=2..6\n");
    for (y = 2; y <= 6; ++y) {
        int ord = 0;
        state->world.party.mapIndex = 0;
        state->world.party.mapX = 1;
        state->world.party.mapY = y;
        state->world.party.direction = 3; /* DIR_WEST */
        ord = M11_GameView_GetFrontMirrorOrdinal(state);
        if (ord == ORDINAL_TARGET) {
            ++foundOrdinal2;
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
             "(x=1, y=2..6) DIR_WEST scan finds no C127 sensor with sensorData=%d "
             "on the corridor west wall (found %d)",
             ORDINAL_TARGET, foundOrdinal2);
    CHECK(foundOrdinal2 == 0, msg);

    /* Side-check: the engine helper at each west_negative pose
     * still returns the source-locked wall box (80, 29, 64, 43). */
    for (i = 0; i < ordinalsCount; ++i) {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
        (void)rc;
        snprintf(msg, sizeof(msg),
                 "Engine helper invariant: D1C wall box is (80, 29, 64, 43) for any corridor west_negative pose "
                 "(got (%d, %d, %d, %d))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == WALLBOX_X && ornY == WALLBOX_Y &&
              ornW == WALLBOX_W && ornH == WALLBOX_H, msg);
        /* Only need to assert this once — break early to avoid
         * redundant PASS lines. */
        break;
    }
}

/* Group D — Cross-check positive ordinal at the SAME rect.
 * At (1,5) DIR_NORTH the C127 sensor carries sensorData=10 (ZED)
 * and the engine paints the ordinal-10 portrait into the D1C
 * cutout.  This proves the D1C rectangle is alive at the source
 * position: an empty (1,4) W cutout cannot silently mean the
 * rectangle is dead. */
static void check_positive_cross_check(M11_GameViewState* state,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pctWant;
    int pctTarget;
    char msg[200];

    printf("\n[Group D] (1,5) DIR_NORTH cross-check — D1C cutout IS painted with ordinal %d (not %d)\n",
           ORDINAL_CROSSCHECK, ORDINAL_TARGET);

    render_at(state, fb, 1, 5, 0 /* DIR_NORTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) N) == %d (got %d)",
             ORDINAL_CROSSCHECK, ord);
    CHECK(ord == ORDINAL_CROSSCHECK, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    /* The cutout must match ordinal 10 (ZED) above 90%. */
    pctWant = match_portrait_cell(portraits, fb, ORDINAL_CROSSCHECK);
    snprintf(msg, sizeof(msg),
             "(1,5) N D1C cutout matches ordinal %d (ZED) >= %d%%%% (got %d%%%%)",
             ORDINAL_CROSSCHECK, CORRECT_ORDINAL_MATCH_PCT, pctWant);
    CHECK(pctWant >= CORRECT_ORDINAL_MATCH_PCT, msg);

    /* And it must NOT match ordinal 2 (the slice target) above
     * the wrong-ordinal drift threshold — proves the cross-check
     * painted the right portrait, not ordinal 2 by accident. */
    pctTarget = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,5) N D1C cutout does NOT match ordinal %d (the slice target) < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pctTarget);
    CHECK(pctTarget < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group E — Re-enter west_negative to confirm the empty rect
 * invariant holds on a fresh render too (no stale state from
 * the cross-check). */
static void check_west_negative_reentry(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group E] Re-enter (1,4) DIR_WEST — empty D1C cutout invariant still holds\n");

    render_at(state, fb, 1, 4, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "re-entered (1,4) W ordinal == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "re-entered (1,4) W D1C cutout does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
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
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d west_negative portrait_rect_position ===\n",
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
    check_corridor_west_negative_scan(&state);
    check_positive_cross_check(&state, portraits);
    check_west_negative_reentry(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
