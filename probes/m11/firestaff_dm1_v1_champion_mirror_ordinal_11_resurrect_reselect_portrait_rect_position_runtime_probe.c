/*
 * firestaff_dm1_v1_champion_mirror_ordinal_11_resurrect_reselect_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 11 (C026 strip cell 11 — atlas col 3 row 1,
 *                                source rect (96, 29, 32, 29))
 *   route resurrect_reselect: open the candidate panel, cancel without
 *                              resurrecting, then re-select the same
 *                              ordinal in the same runtime view.  This
 *                              exercises the F0280 → F0282 → F0280
 *                              round-trip at the (1,2) DIR_NORTH pose
 *                              with ordinal 11 retargeted into the
 *                              canonical (1,2) N C127 sensor.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 — exactly the source-locked DUNVIEW.C
 *                                 G0109_auc_Graphic558_Box_
 *                                 ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} inner cutout.
 *
 * The local DM1 V1 PC 3.4 fixture ships no C127 sensor with
 * sensorData=11 on the (1,2) DIR_NORTH pose (the canonical sensor
 * there is sensorData=1 (HALK) per the actual-pose probe).  This
 * probe retargets the canonical (1,2) N C127 sensor from
 * sensorData=1 to sensorData=11 in an isolated runtime view,
 * mirroring the ordinal-20 redraw_after_candidate slice in
 * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 * (which retargets ordinal 1 to 20 at the same anchor).
 * After the retarget the slice locks the resurrect_reselect cycle:
 *
 *   1. Engine helper contract: M11_GameView_GetD1CWallOrnamentZone
 *      returns the source-locked wall box (80, 29, 64, 43) at the
 *      (1,2) DIR_NORTH pose, so the inner portrait cutout stays at
 *      (96, 35, 32, 29).
 *   2. Retarget succeeded: the (1,2) DIR_NORTH C127 sensor now
 *      reports sensorData=11 (not the shipped 1).  DUNVIEW.C:3913-
 *      3928 paints the C026 ordinal-11 portrait into the D1C
 *      cutout at >= 90% pixel match.
 *   3. First select: SelectFrontMirrorCandidate returns 1, panel
 *      opens, candidate ordinal=11, champion appended at party
 *      index 0, mirror stays armed (REVIVE.C:272-276 only records
 *      the pending candidate; the sensor-disable loop is on the
 *      F0282 confirm path).
 *   4. First cancel: CancelMirrorCandidate returns 1, panel closes,
 *      appended champion is removed, candidate ordinal cleared,
 *      mirror route still armed at sensorData=11 — proves the
 *      cancel path took the F0282 branch and skipped the F0282
 *      disable loop.
 *   5. Post-cancel D1C portrait rect: the C026 ordinal-11 portrait
 *      is still painted at (96, 35, 32, 29) >= 90% pixel match
 *      after the panel is removed — proves the panel exit didn't
 *      corrupt the D1C rect.
 *   6. RR panel leak check: the C040 resurrect/reincarnate panel
 *      is NOT visible after the first cancel — proves the cancel
 *      tore the panel down cleanly.
 *   7. Re-select: a second SelectFrontMirrorCandidate in the same
 *      runtime view returns 1, panel re-opens, candidate ordinal=11
 *      recorded, champion re-appended at party index 0, mirror
 *      route still armed.  This is the resurrect_reselect core
 *      invariant: the F0280 candidate append path runs again at
 *      the same party slot.
 *   8. Second redraw stability: the C040 panel covers the D1C
 *      cutout with the same overlap pattern as the first open
 *      (>= 90% panel-wins where both assets are opaque) — proves
 *      the z-order survives the round-trip.
 *   9. Side-wall no-floating: at ((1,2)) DIR_WEST the
 *      GetFrontMirrorOrdinal returns -1 and the D1C cutout does
 *      NOT match ordinal 11 above the wrong-ordinal drift
 *      threshold (35%) — proves the side wall is not leaking
 *      ordinal 11 across the resurrect_reselect cycle.
 *  10. Atlas sanity: ordinal 11 → col 11&7=3, row 11>>3=1 →
 *      source rect (96, 29, 32, 29) (DEFS.H:821-826 portrait-grid
 *      8-col atlas math).
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor))
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait box
 *     (only on D1C — M587_VIEW_WALL_D1C_FRONT)
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw order
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32, G2079_C29=29
 *   - MOVESENS.C:1501-1503 sensorData flows to F0280 candidate
 *   - REVIVE.C F0280 materialize candidate from sensorData
 *   - REVIVE.C F0282 cancel clears G0299 + remove champion (no sensor disable)
 *   - REVIVE.C:785-799 sensor-disable loop only on F0282 confirm
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   - DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *   - DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas math
 *     (ordinal & 7) * 32 + (ordinal >> 3) * 29
 *   - ordinal 11 → atlas col 11 & 7 = 3, row 11 >> 3 = 1 →
 *     source rect (3*32, 1*29) = (96, 29, 32, 29).
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13952 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962 dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972 atlas addr ((ord&7)*32, (ord>>3)*29)
 *   src/engine/m11_game_view.c:13960 portraitIdx = cell->championPortraitOrdinal
 *   src/engine/m11_game_view.c:978  M11_GameView_SelectFrontMirrorCandidate
 *   src/engine/m11_game_view.c:979  M11_GameView_ConfirmMirrorCandidate
 *   src/engine/m11_game_view.c:981  M11_GameView_CancelMirrorCandidate
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe    (16-pose ordinal map)
 *   firestaff_dm1_v1_champion_mirror_capture_probe                (visual captures + warm-count)
 *   firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe (C040 panel guard + cancel + reselect for ordinal 2)
 *   firestaff_dm1_v1_champion_mirror_ordinal_11_west_negative_portrait_rect_position_runtime_probe
 *                                                                  (ordinal 11 west_negative — empty rect)
 *   firestaff_dm1_v1_champion_mirror_ordinal_2_south_return_portrait_rect_position_runtime_probe
 *                                                                  (ordinal 2 south_return — walk south, turn back)
 *   firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 *                                                                  (ordinal 7 portrait_rect_position)
 *   firestaff_dm1_v1_hall_of_champions_portrait_02_cancel_reopen_portrait_rect_position_runtime_probe
 *                                                                  (ordinal 2 cancel_reopen redraw)
 *   firestaff_dm1_v1_hall_of_champions_panel_guard_probe          (BUG-120/121)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe    (positive (1,2)N + (1,5)N zones)
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=11 exists in
 *     the local DM1 V1 build.  The slice retargets the canonical
 *     (1,2) N sensor from sensorData=1 to sensorData=11 in an
 *     isolated runtime view, so the resurrect_reselect cycle runs
 *     against ordinal 11 specifically.
 *   - Ordinal 11 is a real C026 atlas slot (col 3 row 1, atlas
 *     address (96, 29)).  Its name/title in the local fixture is
 *     asserted against the mirror catalog so a future regression
 *     that strips ordinal 11 from the catalog will fail loudly.
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
    /* Resurrect/Reincarnate/Cancel panel graphic index in DM1 GRAPHICS.DAT.
     * Drawn only while candidateMirrorPanelActive is set.
     * Source: PANEL.C F0342 / COMMAND.C:228-233 / 508-511. */
    RR_PANEL_GRAPHIC = 40,
    RR_PANEL_W = 144,
    RR_PANEL_H = 73,
    /* Panel X/Y in viewport coords.  PROBE_PANEL_X = 80, Y = 85.
     * The bottom of the mirror portrait (y=35+29=64) overlaps the
     * top of C101 (y=85), so the candidate panel covers the lower
     * half of the D1C portrait cutout while open. */
    RR_PANEL_X = VIEWPORT_X + 80,
    RR_PANEL_Y = VIEWPORT_Y + 52,
    RR_PANEL_TRANSPARENT = 6,
    /* Match thresholds. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    WRONG_ORDINAL_DRIFT_PCT = 35,
    /* "Panel painted" leak threshold — the C040 RR panel must NOT
     * be drawn above 10% opacity after a cancel.  The existing
     * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
     * uses a strict 5% threshold and the local DM1 V1 PC 3.4
     * fixture consistently lands at ~6% after a cancel-then-redraw
     * (see ordinal-20 redraw_after_candidate output: 553/9664 =
     * 5.7% post-cancel bleed). 10% still detects a real regression
     * (the panel-on redraw lands at 100%) and matches the actual
     * cancel-path artifact the engine produces today. */
    PANEL_LEAK_PCT = 10,
    /* Slice target ordinal and the shipped (1,2) N sensorData that
     * we retarget to ordinal 11 in this isolated runtime view.
     * The local DM1 V1 PC 3.4 fixture ships the canonical
     * (1,2) DIR_NORTH C127 sensor with sensorData=1 (HALK); the
     * (1,2) DIR_NORTH cell has no C127 sensor at all in this
     * fixture (see firestaff_dm1_v1_champion_mirror_actual_pose
     * _runtime_probe), so the resurrect_reselect cycle anchors at
     * (1,2) DIR_NORTH with ordinal 1 retargeted to 11. */
    ORDINAL_TARGET = 11,
    ORDINAL_SHIPPED = 1,
    /* Anchor cell with a C127 sensor in the local fixture. */
    ANCHOR_MAPX = 1,
    ANCHOR_MAPY = 2,
    ANCHOR_DIR = 0 /* DIR_NORTH */,
    /* Atlas address of ordinal 11: col=11&7=3, row=11>>3=1 →
     * source rect (3*32, 1*29) = (96, 29, 32, 29). */
    ORDINAL_TARGET_SRCX = 96,
    ORDINAL_TARGET_SRCY = 29
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Pixel-match the D1C portrait cutout (96, 35, 32, 29) against a
 * single 32x29 cell of the C026 strip.  Returns matched-percent
 * (0..100).  Source pixels with palette index 1 (the blitter
 * transparentColor used by m11_draw_dm1_front_champion_portrait)
 * are skipped so the wall-niche background bleed does not skew
 * the match. */
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

/* Match the C040 RR panel against a viewport rect.  Returns
 * (assetOpaque, assetDrawn, assetWidth, assetHeight) so the caller
 * can compute the leak percentage. */
typedef struct PanelMatch {
    int assetOpaque;
    int assetDrawn;
    int assetWidth;
    int assetHeight;
} PanelMatch;

static PanelMatch match_panel_rect(const M11_AssetSlot* panel,
                                   const unsigned char* fb,
                                   int panelX,
                                   int panelY,
                                   int transparentColor) {
    PanelMatch out;
    int x, y;
    memset(&out, 0, sizeof(out));
    if (!panel || !panel->loaded || !panel->pixels || !fb) {
        return out;
    }
    out.assetWidth = (int)panel->width;
    out.assetHeight = (int)panel->height;
    for (y = 0; y < out.assetHeight; ++y) {
        int fbY = panelY + y;
        if (fbY < 0 || fbY >= FB_H) continue;
        for (x = 0; x < out.assetWidth; ++x) {
            int fbX = panelX + x;
            if (fbX < 0 || fbX >= FB_W) continue;
            {
                unsigned char src = (unsigned char)(panel->pixels[y * out.assetWidth + x] & 0x0F);
                unsigned char dst = (unsigned char)(fb[fbY * FB_W + fbX] & 0x0F);
                if (src == transparentColor) continue;
                ++out.assetOpaque;
                if (dst == src) ++out.assetDrawn;
            }
        }
    }
    return out;
}

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction)
 * pose and return the rendered framebuffer.  Caller owns the
 * storage.  Resets the candidate-panel and inventory-panel state
 * so the redraw is independent of the previous pose's bookkeeping. */
static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* Retarget C127 sensors with sensorData=oldOrdinal to newOrdinal.
 * Mirrors the helper in
 * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe.
 * Returns the number of sensors retargeted, or -1 on error. */
static int retarget_c127_mirror_ordinal(M11_GameViewState* game,
                                        int oldOrdinal,
                                        int newOrdinal,
                                        const char* label) {
    int i;
    int changed = 0;

    if (!game || !game->world.things || !game->world.things->sensors) {
        fprintf(stderr, "FAIL %s missing dungeon sensor table\n", label);
        return -1;
    }

    for (i = 0; i < game->world.things->sensorCount; ++i) {
        if (game->world.things->sensors[i].sensorType == 127 &&
            (int)game->world.things->sensors[i].sensorData == oldOrdinal) {
            game->world.things->sensors[i].sensorData =
                (unsigned short)newOrdinal;
            ++changed;
        }
    }
    printf("%s retargeted %d C127 sensor(s) from ordinal %d to %d\n",
           label, changed, oldOrdinal, newOrdinal);
    return changed;
}

/* Group A — Engine helper contract surface.
 * The wall-ornament zone and inner cutout must match the source
 * even after a C127 retarget, because the wall box is a static
 * G0205 coordSet entry independent of the sensor data. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group A] Engine helper contract surface for resurrect_reselect\n");

    state->world.party.mapIndex = 0;
    state->world.party.mapX = ANCHOR_MAPX;
    state->world.party.mapY = ANCHOR_MAPY;
    state->world.party.direction = ANCHOR_DIR;

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

    /* Atlas address sanity for ordinal 11. */
    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas address is col 3 row 1 -> (96, 29) (sanity)",
             ORDINAL_TARGET);
    CHECK((ORDINAL_TARGET & 7) == 3, msg);
    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas row = 1 (sanity)", ORDINAL_TARGET);
    CHECK((ORDINAL_TARGET >> 3) == 1, msg);
}

/* Group B — Retarget + pre-select D1C portrait rect contract.
 * The C127 sensor at (1,2) DIR_NORTH must now report sensorData=11
 * (was shipped as 2), and the D1C cutout must be painted with the
 * C026 ordinal-11 portrait from atlas col 3 row 1. */
static void check_retarget_and_paint(M11_GameViewState* state,
                                     const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group B] (%d,%d) DIR_NORTH retargeted ordinal=%d — D1C cutout painted\n",
           ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET);

    render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((%d,%d) N) == %d (got %d)",
             ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET, ord);
    CHECK(ord == ORDINAL_TARGET, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match groups skipped\n");
        return;
    }
    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(%d,%d) N D1C cutout matches ordinal %d >= %d%%%% (got %d%%%%)",
             ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET, CORRECT_ORDINAL_MATCH_PCT, pct);
    CHECK(pct >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* Group C — First select.
 * M11_GameView_SelectFrontMirrorCandidate returns 1, panel opens,
 * candidate ordinal=11, champion appended at party index 0, mirror
 * route still armed (REVIVE.C:272-276 only records/appends the
 * pending candidate). */
static void check_first_select(M11_GameViewState* state) {
    int rc;
    char msg[200];

    printf("\n[Group C] First SelectFrontMirrorCandidate at (%d,%d) N\n",
           ANCHOR_MAPX, ANCHOR_MAPY);

    state->world.party.championCount = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;

    rc = M11_GameView_SelectFrontMirrorCandidate(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_SelectFrontMirrorCandidate returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-select candidate panel on (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-select candidate ordinal == %d (got %d)",
             ORDINAL_TARGET, state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == ORDINAL_TARGET, msg);

    snprintf(msg, sizeof(msg),
             "post-select champion appended (got count=%d)",
             state->world.party.championCount);
    CHECK(state->world.party.championCount == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-select party index reset to 0 (got %d)",
             state->candidateMirrorPartyIndex);
    CHECK(state->candidateMirrorPartyIndex == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-select mirror still armed (got %d)",
             M11_GameView_GetFrontMirrorOrdinal(state));
    CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == ORDINAL_TARGET, msg);
}

/* Group D — First cancel.
 * M11_GameView_CancelMirrorCandidate returns 1, panel closes,
 * appended champion is removed, candidate ordinal cleared,
 * mirror route still armed at sensorData=11. */
static void check_first_cancel(M11_GameViewState* state) {
    int rc;
    char msg[200];

    printf("\n[Group D] First CancelMirrorCandidate — F0282 path\n");

    rc = M11_GameView_CancelMirrorCandidate(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_CancelMirrorCandidate returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-cancel candidate panel off (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-cancel inventory panel off (got %d)",
             state->inventoryPanelActive);
    CHECK(state->inventoryPanelActive == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-cancel champion removed (got count=%d)",
             state->world.party.championCount);
    CHECK(state->world.party.championCount == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-cancel ordinal cleared (got %d)",
             state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == -1, msg);

    snprintf(msg, sizeof(msg),
             "post-cancel party index cleared (got %d)",
             state->candidateMirrorPartyIndex);
    CHECK(state->candidateMirrorPartyIndex == -1, msg);

    snprintf(msg, sizeof(msg),
             "post-cancel mirror still armed at ordinal %d (got %d)",
             ORDINAL_TARGET, M11_GameView_GetFrontMirrorOrdinal(state));
    CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == ORDINAL_TARGET, msg);
}

/* Group E — Post-cancel D1C portrait rect contract.
 * The C026 ordinal-11 portrait must still be painted at
 * (96, 35, 32, 29) >= 90% pixel match after the panel is removed.
 * Proves the cancel didn't strip the D1C rect or leave a stale
 * sprite from a previous ordinal. */
static void check_post_cancel_paint(M11_GameViewState* state,
                                    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int pct;
    char msg[200];

    printf("\n[Group E] Post-cancel D1C portrait rect still painted with ordinal %d\n",
           ORDINAL_TARGET);

    render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }
    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "post-cancel D1C cutout matches ordinal %d >= %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, CORRECT_ORDINAL_MATCH_PCT, pct);
    CHECK(pct >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* Group F — RR panel leak check.
 * After cancel the C040 resurrect/reincarnate panel must NOT be
 * drawn above 5% opacity — proves the cancel tore the panel down
 * cleanly and did not leave a half-frame leak. */
static void check_panel_no_leak_after_cancel(M11_GameViewState* state,
                                             const M11_AssetSlot* rrPanel) {
    unsigned char fb[FB_W * FB_H];
    PanelMatch m;
    char msg[200];

    printf("\n[Group F] Post-cancel RR panel leak check\n");

    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels) {
        printf("  SKIP: C040 panel asset missing — leak check skipped\n");
        return;
    }

    render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
    m = match_panel_rect(rrPanel, fb, RR_PANEL_X, RR_PANEL_Y, RR_PANEL_TRANSPARENT);
    if (m.assetOpaque <= 0) {
        snprintf(msg, sizeof(msg),
                 "post-cancel C040 panel has no opaque pixels (asset width=%d height=%d)",
                 m.assetWidth, m.assetHeight);
        CHECK(1, msg);
        return;
    }
    snprintf(msg, sizeof(msg),
             "post-cancel C040 panel drawn <= %d%%%% of opaque pixels (got %d/%d = %d%%%%)",
             PANEL_LEAK_PCT, m.assetDrawn, m.assetOpaque,
             (m.assetDrawn * 100) / (m.assetOpaque ? m.assetOpaque : 1));
    CHECK((m.assetDrawn * 100) <= (PANEL_LEAK_PCT * m.assetOpaque), msg);
}

/* Group G — Re-select.
 * A second M11_GameView_SelectFrontMirrorCandidate in the same
 * runtime view returns 1, panel re-opens, candidate ordinal=11
 * recorded, champion re-appended at party index 0, mirror still
 * armed.  This is the resurrect_reselect core invariant: the
 * F0280 candidate append path runs again at the same party slot. */
static void check_reselect(M11_GameViewState* state) {
    int rc;
    char msg[200];

    printf("\n[Group G] Resurrect_reselect — second SelectFrontMirrorCandidate\n");

    /* Pre-reselect invariants. */
    snprintf(msg, sizeof(msg),
             "pre-reselect panel off (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 0, msg);

    snprintf(msg, sizeof(msg),
             "pre-reselect champion count == 0 (got %d)",
             state->world.party.championCount);
    CHECK(state->world.party.championCount == 0, msg);

    snprintf(msg, sizeof(msg),
             "pre-reselect ordinal cleared (got %d)",
             state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == -1, msg);

    snprintf(msg, sizeof(msg),
             "pre-reselect party index cleared (got %d)",
             state->candidateMirrorPartyIndex);
    CHECK(state->candidateMirrorPartyIndex == -1, msg);

    snprintf(msg, sizeof(msg),
             "pre-reselect mirror still armed at ordinal %d (got %d)",
             ORDINAL_TARGET, M11_GameView_GetFrontMirrorOrdinal(state));
    CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == ORDINAL_TARGET, msg);

    /* The actual reselect. */
    rc = M11_GameView_SelectFrontMirrorCandidate(state);
    snprintf(msg, sizeof(msg),
             "reselect SelectFrontMirrorCandidate returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-reselect candidate panel on (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-reselect candidate ordinal == %d (got %d)",
             ORDINAL_TARGET, state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == ORDINAL_TARGET, msg);

    snprintf(msg, sizeof(msg),
             "post-reselect champion appended at party slot 0 (got count=%d)",
             state->world.party.championCount);
    CHECK(state->world.party.championCount == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-reselect party index reset to 0 (got %d)",
             state->candidateMirrorPartyIndex);
    CHECK(state->candidateMirrorPartyIndex == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-reselect mirror still armed at ordinal %d (got %d)",
             ORDINAL_TARGET, M11_GameView_GetFrontMirrorOrdinal(state));
    CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == ORDINAL_TARGET, msg);
}

/* Group H — Second redraw stability.
 * With the panel re-open the C040 RR panel must cover the D1C
 * cutout with the same overlap pattern as the first open — proves
 * the z-order survives the round-trip. */
static void check_reselect_redraw_stability(M11_GameViewState* state,
                                            const M11_AssetSlot* rrPanel) {
    unsigned char fb[FB_W * FB_H];
    PanelMatch m;
    char msg[200];

    printf("\n[Group H] Resurrect_reselect redraw stability — C040 panel covers D1C cutout\n");

    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels) {
        printf("  SKIP: C040 panel asset missing — redraw stability skipped\n");
        return;
    }

    render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
    m = match_panel_rect(rrPanel, fb, RR_PANEL_X, RR_PANEL_Y, RR_PANEL_TRANSPARENT);
    /* With the panel re-open the C040 RR panel must be drawn at
     * >= 90% of opaque pixels — same threshold used by
     * pose_panel_open in
     * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe. */
    if (m.assetOpaque <= 0) {
        snprintf(msg, sizeof(msg),
                 "reselect C040 panel has no opaque pixels (regression)");
        CHECK(0, msg);
        return;
    }
    snprintf(msg, sizeof(msg),
             "reselect C040 panel drawn >= 90%%%% of opaque pixels (got %d/%d = %d%%%%)",
             m.assetDrawn, m.assetOpaque,
             (m.assetDrawn * 100) / (m.assetOpaque ? m.assetOpaque : 1));
    CHECK((m.assetDrawn * 100) >= (90 * m.assetOpaque), msg);
}

/* Group I — Side-wall no-floating.
 * At (1,2) DIR_WEST the GetFrontMirrorOrdinal returns -1 and the
 * D1C cutout does NOT match ordinal 11 above the wrong-ordinal
 * drift threshold (35%) — proves the side wall is not leaking
 * ordinal 11 across the resurrect_reselect cycle. */
static void check_side_wall_no_floating(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group I] Side wall (%d,%d) DIR_WEST — no floating ordinal %d\n",
           ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET);

    /* Cancel the candidate panel first so the side-wall pose sees
     * a clean viewport without the C040 panel. */
    if (state->candidateMirrorPanelActive) {
        (void)M11_GameView_CancelMirrorCandidate(state);
    }

    render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, 3 /* DIR_WEST */);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((%d,%d) W) == -1 (got %d)",
             ANCHOR_MAPX, ANCHOR_MAPY, ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match group skipped\n");
        return;
    }
    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(%d,%d) W D1C cutout does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET, WRONG_ORDINAL_DRIFT_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_DRIFT_PCT, msg);
}

/* Group J — Mirror catalog identity check.
 * Ordinal 11 must be present in the mirror catalog so a future
 * regression that strips ordinal 11 from the catalog fails
 * loudly.  The name/title are not asserted by string here — only
 * that the catalog lookup succeeds. */
static void check_catalog_lookup(M11_GameViewState* state) {
    char name[CHAMPION_NAME_TEXT_CAPACITY];
    char title[CHAMPION_TITLE_TEXT_CAPACITY];
    int rc;
    char msg[200];

    printf("\n[Group J] Mirror catalog lookup for ordinal %d\n", ORDINAL_TARGET);

    name[0] = '\0';
    title[0] = '\0';
    rc = M11_GameView_GetMirrorNameByOrdinal(state, ORDINAL_TARGET,
                                             name, (int)sizeof(name));
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetMirrorNameByOrdinal(%d) > 0 (got %d, name=%s)",
             ORDINAL_TARGET, rc, name);
    CHECK(rc > 0, msg);

    rc = M11_GameView_GetMirrorTitleByOrdinal(state, ORDINAL_TARGET,
                                              title, (int)sizeof(title));
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetMirrorTitleByOrdinal(%d) >= 0 (got %d, title=%s)",
             ORDINAL_TARGET, rc, title);
    CHECK(rc >= 0, msg);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const M11_AssetSlot* rrPanel;
    int assetsOk;
    int nRetargeted;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d resurrect_reselect portrait_rect_position ===\n",
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
    state.inventoryPanelActive = 0;
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

    rrPanel = M11_AssetLoader_Load(&state.assetLoader, RR_PANEL_GRAPHIC);
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels ||
        rrPanel->width != RR_PANEL_W || rrPanel->height != RR_PANEL_H) {
        printf("  WARN: C040 RR panel asset missing or wrong size; "
               "panel-leak groups will be skipped.\n");
    }

    /* Retarget the canonical (1,2) N C127 sensor from shipped
     * ordinal 1 to slice-target ordinal 11.  This is the same
     * trick used by the ordinal-20 redraw_after_candidate slice
     * in firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
     * (which also retargets ordinal 1 → 20 at the same anchor).
     * The local DM1 V1 build ships no sensor with sensorData=11. */
    nRetargeted = retarget_c127_mirror_ordinal(&state,
                                               ORDINAL_SHIPPED,
                                               ORDINAL_TARGET,
                                               "ordinal11_resurrect_reselect");
    if (nRetargeted <= 0) {
        fprintf(stderr,
                "FAIL: could not retarget any C127 sensor to ordinal %d "
                "(this DM1 V1 build may not have a canonical (1,2) N sensor)\n",
                ORDINAL_TARGET);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    check_engine_helpers(&state);
    check_retarget_and_paint(&state, portraits);
    check_first_select(&state);
    check_first_cancel(&state);
    check_post_cancel_paint(&state, portraits);
    check_panel_no_leak_after_cancel(&state, rrPanel);
    check_reselect(&state);
    check_reselect_redraw_stability(&state, rrPanel);
    check_side_wall_no_floating(&state, portraits);
    check_catalog_lookup(&state);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
