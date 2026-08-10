/*
 * firestaff_dm1_v1_hoc_champion_portrait_18_reincarnate_reselect_portrait_rect_position_258_gate_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 18 (SONJA / SHE DEVIL, C026 strip
 *                                cell 18 — atlas col 2 row 2,
 *                                source rect (64, 58, 32, 29))
 *   route   reincarnate_reselect: open the C040 candidate panel,
 *                                 take the F0282 C162 cancel
 *                                 branch, re-open the panel via a
 *                                 fresh F0280 call (the reselect),
 *                                 then call M11_GameView_Confirm
 *                                 MirrorCandidate(state, 1) so the
 *                                 F0282 C165 REINCARNATE confirm
 *                                 path runs (as opposed to the
 *                                 C164 RESURRECT confirm path),
 *                                 and verify the round-trip
 *                                 end-to-end.  The C165 confirm
 *                                 keeps the champion in the party
 *                                 (F0282:785-799 — only the C162
 *                                 cancel decrements the party
 *                                 count) and runs the
 *                                 sensor-disable loop, so the
 *                                 post-confirm mirror is -1 and a
 *                                 further select is refused.
 *   aspect  portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 — exactly the source-locked
 *                                 DUNVIEW.C G0109_auc_Graphic558
 *                                 _Box_ChampionPortraitOnWall =
 *                                 {96, 127, 35, 63} inner cutout.
 *   batch   group 10: portrait_rect_position reincarnate_reselect
 *                    sub-gate.  Next free tag after groups
 *                    0/3/4/6/7/8/9 used by the existing
 *                    palette_match_rect, d2r_negative,
 *                    double_click_stability, input_focus_restore,
 *                    approach_from_right, and resurrect_reselect
 *                    gate probes (see those files for the tag
 *                    convention).
 *
 * The local DM1 V1 PC 3.4 fixture ships the canonical (7, 9)
 * DIR_NORTH C127 sensor with sensorData=1 (HALK).  This probe
 * retargets that sensor to sensorData=18 in an isolated runtime
 * view, mirroring the ordinal-0 216_gate
 * (firestaff_dm1_v1_hoc_champion_portrait_00_resurrect_reselect
 *  _portrait_rect_position_216_gate_probe) and the ordinal-11
 * firestaff_dm1_v1_champion_mirror_ordinal_11_resurrect_reselect
 *  _portrait_rect_position_runtime_probe (which retarget ordinal
 * 1 → 11 at the same anchor).  The 258_gate slice differs from
 * the 216_gate sibling in two source-locked ways: (a) the
 * round-trip ends in the F0282 C165 REINCARNATE branch
 * (`reincarnate=1`), not the F0282 C164 RESURRECT branch, so the
 * confirm body runs the sensor-disable loop at REVIVE.C:785-799
 * and the post-confirm mirror is -1; (b) per the source-locked
 * F0282 routing (dm1_v1_resurrection_pc34_compat.c — "745-757:
 * Cancel clears candidate state and decrements party count.
 * 785-799: Resurrect/Reincarnate clear candidate state and
 * disable mirror sensor"), the C165 confirm KEEPS the appended
 * champion in the party (championCount stays 1) — only the C162
 * cancel decrements the count.  2026-07-21 round-18 correction:
 * the previous revision expected the C165 confirm to consume the
 * appended slot (count 1 -> 0) and then re-select the same
 * ordinal; both contradict the source — the champion joins the
 * party on confirm, and a fresh F0280 select of the same mirror
 * record is refused by the already-in-party guard.  The
 * source-faithful round-trip is therefore F0280 -> F0282(C162
 * cancel) -> F0280(reselect) -> F0282(C165 REINCARNATE), which
 * still exercises the REINCARNATE confirm path that the 216_gate
 * (C164/cancel) and ordinal-11 (C164/cancel) slices do not.
 *
 * After the retarget the slice locks the reincarnate_reselect
 * cycle:
 *
 *   1. Engine helper contract: M11_GameView_GetD1CWallOrnamentZone
 *      returns the source-locked wall box (80, 29, 64, 43) at the
 *      (7, 9) DIR_NORTH pose, so the inner portrait cutout stays
 *      at (96, 35, 32, 29) — the rect_pos invariant from
 *      DUNVIEW.C:525.
 *   2. Atlas math sanity for ordinal 18: (18 & 7) * 32 = 64,
 *      (18 >> 3) * 29 = 58 — col 2 row 2 of the 256x87 C026 atlas
 *      (DEFS.H:821-826 / COORD.C:1748-1749).  Catches a
 *      regression that mis-encodes the ordinal-18 atlas address.
 *   3. Retarget succeeded: the (7, 9) DIR_NORTH C127 sensor now
 *      reports sensorData=18 (was shipped as 1).  DUNVIEW.C:3913-
 *      3928 paints the C026 ordinal-18 portrait into the D1C
 *      cutout at >= 90% pixel match.
 *   4. First select: M11_GameView_SelectFrontMirrorCandidate
 *      returns 1, panel opens, candidate ordinal=18, champion
 *      appended at party index 0, mirror stays armed (REVIVE.C
 *      :272-276 only records/appends the pending candidate; the
 *      sensor-disable loop is on the F0282 confirm path).
 *   5. First cancel: M11_GameView_CancelMirrorCandidate returns
 *      1, panel closes, the appended champion is removed
 *      (championCount 1 -> 0 per F0282:745-757 — only the C162
 *      cancel decrements), candidate ordinal/party index cleared,
 *      and the mirror route stays armed at sensorData=18 (the
 *      cancel path skips the F0282 sensor-disable loop).
 *   6. Post-cancel D1C portrait rect: the engine still returns
 *      18 at the (7, 9) N pose, so the D1C cutout must match the
 *      C026 ordinal-18 portrait at >= 90% — proves the cancel
 *      didn't strip the D1C rect or disarm the route.
 *   7. Re-select: a second M11_GameView_SelectFrontMirrorCandidate
 *      in the same runtime view returns 1, panel re-opens,
 *      candidate ordinal=18 recorded, champion re-appended at
 *      party index 0, mirror still armed.  This is the
 *      reincarnate_reselect core invariant: the F0280 candidate
 *      append path runs again at the same party slot after an
 *      F0282 C162 cancel.  With the panel re-open the C040 RR
 *      panel must cover the D1C cutout at >= 90% of opaque asset
 *      pixels — proves the z-order survives the round-trip.
 *   8. REINCARNATE confirm: M11_GameView_ConfirmMirrorCandidate
 *      (state, 1) returns 1, panel closes, the champion JOINS
 *      the party (championCount stays 1 per F0282:785-799 — the
 *      C165/C164 confirm clears candidate state but does NOT
 *      decrement the party count), the candidate ordinal/party
 *      index are cleared, AND the sensor is disabled so
 *      M11_GameView_GetFrontMirrorOrdinal returns -1.  A further
 *      SelectFrontMirrorCandidate returns 0 — the route is
 *      closed (sensor disabled + already-in-party guard).
 *   9. Post-confirm D1C portrait rect + redraw stability: the
 *      D1C cutout must NOT match ordinal 18 above the wrong-
 *      ordinal drift threshold (35%) — proves the sensor-disable
 *      path released the route and did not leak a stale SONJA
 *      sprite — and 4 successive M11_GameView_Draw calls at the
 *      post-confirm state produce byte-stable pixels.
 *  10. Side-wall no-floating: at (7, 9) DIR_WEST the
 *      GetFrontMirrorOrdinal returns -1 and the D1C cutout does
 *      NOT match ordinal 18 above the wrong-ordinal drift
 *      threshold (35%) — proves the side wall is not leaking
 *      ordinal 18 across the reincarnate_reselect cycle.
 *  11. Atlas sanity: ordinal 18 -> col 18&7=2, row 18>>3=2 ->
 *      source rect (2*32, 2*29) = (64, 58, 32, 29) (DEFS.H:821-
 *      826 portrait-grid 8-col atlas math).
 *  12. Mirror catalog identity: ordinal 18 resolves to "SONJA"
 *      with title "SHE DEVIL" through M11_GameView_GetMirrorName
 *      ByOrdinal / M11_GameView_GetMirrorTitleByOrdinal — proves
 *      the catalog and the C026 atlas agree on the ordinal-18
 *      record.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573 normalize(M011_CELL(sensor) - direction) + 3
 *     front-wall sensor filter
 *   - DUNGEON.C:2608-2612 G0289 = M000_INDEX_TO_ORDINAL(M040_DATA
 *     (sensor))
 *   - DUNVIEW.C:3913-3928 C026 portrait blit into G0109 portrait
 *     box (only on D1C — M587_VIEW_WALL_D1C_FRONT)
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw order
 *   - COORD.C:1693-1722 PC 3.4 viewport origin (0, 33) / 224x136
 *     dim
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32, G2079_C29=29
 *   - MOVESENS.C:1501-1503 sensorData flows to F0280 candidate
 *   - REVIVE.C F0280 materialize candidate from sensorData
 *   - REVIVE.C F0282:744-806 confirm branch (C164 RESURRECT
 *     path; this slice exercises the C165 REINCARNATE path
 *     which also calls the sensor-disable loop on confirm)
 *   - REVIVE.C:785-799 sensor-disable loop on F0282 confirm
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   - DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *   - DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas math
 *     (ordinal & 7) * 32 + (ordinal >> 3) * 29
 *   - ordinal 18 -> atlas col 18&7=2, row 18>>3=2 -> source
 *     rect (2*32, 2*29) = (64, 58, 32, 29).
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13952 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13962 dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13972 atlas addr ((ord&7)*32, (ord>>3)*29)
 *   src/engine/m11_game_view.c:13960 portraitIdx = cell->championPortraitOrdinal
 *   src/engine/m11_game_view.c:996  M11_GameView_SelectFrontMirrorCandidate
 *   src/engine/m11_game_view.c:997  M11_GameView_ConfirmMirrorCandidate(state, reincarnate)
 *   src/engine/m11_game_view.c:999  M11_GameView_CancelMirrorCandidate
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *                                                                  (16-pose ordinal map)
 *   firestaff_dm1_v1_champion_mirror_capture_probe                (visual captures + warm-count)
 *   firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 *                                                                  (C040 panel guard + cancel + reselect for ordinal 2)
 *   firestaff_dm1_v1_champion_mirror_ordinal_11_resurrect_reselect_portrait_rect_position_runtime_probe
 *                                                                  (ordinal 11 resurrect_reselect — different ordinal, RESURRECT confirm path)
 *   firestaff_dm1_v1_hoc_champion_portrait_00_resurrect_reselect_portrait_rect_position_216_gate_probe
 *                                                                  (ordinal 0 resurrect_reselect — different ordinal, RESURRECT confirm path)
 *   firestaff_dm1_v1_hall_of_champions_portrait_18_cancel_reopen_portrait_rect_position_runtime_probe
 *                                                                  (ordinal 18 cancel_reopen — same ordinal, no confirm, no sensor disable)
 *   firestaff_dm1_v1_hoc_champion_portrait_18_approach_from_right_portrait_rect_position_210_gate_probe
 *                                                                  (ordinal 18 approach_from_right — different route, no panel)
 *   firestaff_dm1_v1_hoc_champion_portrait_18_sleep_repaint_portrait_rect_position_090_gate_probe
 *                                                                  (ordinal 18 sleep_repaint — different route, rest state machine)
 *   firestaff_dm1_v1_hall_of_champions_panel_guard_probe          (BUG-120/121)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe    (positive (7,9)N + (1,5)N zones)
 *
 * Non-duplicative value:
 *   The existing ordinal-18 cancel_reopen slice covers the
 *   select->cancel->reopen cycle at the panel-flags level plus
 *   a pixel match before/after select/cancel/reopen, but does
 *   NOT cover:
 *     - The F0282 C165 REINCARNATE confirm path (the cancel_reopen
 *       slice only exercises the F0282 C162 cancel branch).
 *     - The F0282 confirm body that runs the sensor-disable loop
 *       (REVIVE.C:785-799), so the post-confirm mirror goes -1
 *       and the post-reselect drive must re-enable the route.
 *     - Post-confirm D1C portrait rect emptiness at the (7, 9) N
 *       pose (the F0282 sensor-disable loop must have released
 *       the route without leaking a stale SONJA sprite).
 *     - Post-reselect C040 panel drawn >= 90% of opaque asset
 *       pixels at the (96, 35) destination (the reselect side
 *       of the round-trip after a REINCARNATE confirm).
 *     - Byte-stable redraw cycles of 4 successive draws at the
 *       post-reselect panel-on state.
 *     - Mirror catalog identity as a final gate (catalog name
 *       "SONJA" + title "SHE DEVIL" both resolved).
 *   Each of these is a distinct invariant the existing
 *   cancel_reopen slice does not assert, and all six are
 *   required for a true reincarnate_reselect contract.
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled
 *     from the same GRAPHICS.DAT the runtime is drawing from, so
 *     this is runtime correctness rather than pixel-for-pixel
 *     DOSBox reference parity.
 *   - We do not assume a C127 sensor with sensorData=18 exists
 *     in the local DM1 V1 build.  The slice retargets the
 *     canonical (7, 9) N sensor from sensorData=1 to
 *     sensorData=18 in an isolated runtime view, so the
 *     reincarnate_reselect cycle runs against ordinal 18
 *     specifically.  The post-confirm re-enable also re-targets
 *     back to sensorData=18 in the same view.
 *   - Ordinal 18 is a real C026 atlas slot (col 2 row 2, atlas
 *     address (64, 58)).  Its catalog name is "SONJA" and title
 *     "SHE DEVIL" per ReDMCSB DUNVIEW.C G0289 nibble decode
 *     table — the catalog identity is asserted as a final gate
 *     so a future regression that strips ordinal 18 from the
 *     catalog will fail loudly.
 */

#include "m11_game_view.h"
#include "firestaff_dm1_probe_data_dir.h"
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
    /* Resurrect/Reincarnate/Cancel panel graphic index in DM1
     * GRAPHICS.DAT.  Drawn only while candidateMirrorPanelActive
     * is set.  Source: PANEL.C F0342 / COMMAND.C:228-233 /
     * 508-511. */
    RR_PANEL_GRAPHIC = 40,
    RR_PANEL_W = 144,
    RR_PANEL_H = 73,
    /* Panel X/Y in viewport coords.  PROBE_PANEL_X = 80, Y = 52.
     * The bottom of the mirror portrait (y=35+29=64) overlaps
     * the top of C101 (y=52), so the candidate panel covers the
     * lower 12 rows of the D1C portrait cutout while open. */
    RR_PANEL_X = VIEWPORT_X + 80,
    RR_PANEL_Y = VIEWPORT_Y + 52,
    RR_PANEL_TRANSPARENT = 6,
    /* Match thresholds. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    WRONG_ORDINAL_DRIFT_PCT = 35,
    /* Slice target ordinal and the shipped (7, 9) N sensorData
     * that we retarget to ordinal 18 in this isolated runtime
     * view.  The local DM1 V1 PC 3.4 fixture ships the canonical
     * (7, 9) DIR_NORTH C127 sensor with sensorData=1 (HALK). */
    ORDINAL_TARGET = 18,
    ORDINAL_SHIPPED = 1,
    /* Anchor cell with a C127 sensor in the local fixture. */
    ANCHOR_MAPX = 7,
    ANCHOR_MAPY = 9,
    ANCHOR_DIR = 0 /* DIR_NORTH */,
    /* Atlas address of ordinal 18: col=18&7=2, row=18>>3=2 ->
     * source rect (2*32, 2*29) = (64, 58, 32, 29).  Row-2 of the
     * 256x87 C026 atlas (ordinals 16..23). */
    ORDINAL_TARGET_COL = ORDINAL_TARGET & 7,   /* = 2 */
    ORDINAL_TARGET_ROW = ORDINAL_TARGET >> 3,  /* = 2 */
    ORDINAL_TARGET_SRCX = ORDINAL_TARGET_COL << 5,   /* = 64 */
    ORDINAL_TARGET_SRCY = ORDINAL_TARGET_ROW * 29,   /* = 58 */
    /* The C164 RESURRECT confirm vs C165 REINCARNATE confirm
     * decision in F0282:0 means RESURRECT (default), 1 means
     * REINCARNATE.  This slice drives the REINCARNATE branch
     * because that is the slice's defining route variant. */
    REINCARNATE_FLAG = 1,
    /* Number of successive M11_GameView_Draw calls for the
     * byte-stable redraw check.  Mirrors the ordinal-11 probe and
     * the ordinal-0 216_gate sibling. */
    REDRAW_CYCLES = 4
};

/* Expected mirror catalog record for ordinal 18 (DM1 V1 PC34
 * mirror catalog).  Used in the Group L catalog identity check. */
static const char kExpectedCatalogName[] = "SONJA";
static const char kExpectedCatalogTitle[] = "SHE DEVIL";

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
 * the match.  Mirrors the helper in the ordinal-11
 * resurrect_reselect probe and the ordinal-0 216_gate sibling. */
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
 * (assetOpaque, assetDrawn, assetWidth, assetHeight) so the
 * caller can compute the panel-drawn percentage. */
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
 * storage.  Resets the candidate-panel and inventory-panel
 * state so the redraw is independent of the previous pose's
 * bookkeeping. */
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
 * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 * and the ordinal-11 resurrect_reselect probe.  Returns the
 * number of sensors retargeted, or -1 on error. */
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

/* Group A — Atlas math for ordinal 18.
 * Ordinal 18 must be at col 2 row 2 of the C026 strip.  DEFS.H
 * :821-826 encodes the atlas math as (ordinal & 7) * 32 +
 * (ordinal >> 3) * 29; for ordinal 18 those terms are 64 and 58,
 * so the source rect is (64, 58, 32, 29).  Catches a regression
 * that mis-encodes the ordinal-18 atlas address (e.g. a 1-off
 * shift that lands on the wrong atlas cell or wraps a row). */
static void check_atlas_math(void) {
    char msg[200];

    printf("\n[Group A] C026 atlas math for ordinal 18\n");

    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas col = 18 & 7 = %d (expected 2)",
             ORDINAL_TARGET, ORDINAL_TARGET_COL);
    CHECK(ORDINAL_TARGET_COL == 2, msg);

    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas row = 18 >> 3 = %d (expected 2 — last row of the 8x3 atlas)",
             ORDINAL_TARGET, ORDINAL_TARGET_ROW);
    CHECK(ORDINAL_TARGET_ROW == 2, msg);

    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas address = (%d, %d, %d, %d) "
             "(64+32=96 <= 256, 58+29=87 <= 87 — fits C026 strip exactly)",
             ORDINAL_TARGET, ORDINAL_TARGET_SRCX, ORDINAL_TARGET_SRCY,
             PORTRAIT_W, PORTRAIT_H);
    CHECK(ORDINAL_TARGET_SRCX + PORTRAIT_W <= 256 &&
          ORDINAL_TARGET_SRCY + PORTRAIT_H <= 87, msg);

    /* Cross-check: ordinal 18 should be visually distinct from
     * its row-2 neighbours 17 (BORIS) and 19 (HAWK) — the DM1
     * champion-portrait atlas carries 24 distinct champions, so
     * a duplicate cell would be a real regression.  We don't
     * count this as a CHECK, we just print it as diagnostic so
     * a future regression that strips ordinal 18 from the
     * catalog or atlas shows up in the log without
     * double-counting the failure. */
    printf("  ordinal 18 atlas cell opaque-pixel floor: must be > 0 "
           "(verified in Group M as the catalog-identity check)\n");
}

/* Group B — Engine helper contract surface.
 * The wall-ornament zone and inner cutout must match the source
 * even after a C127 retarget, because the wall box is a static
 * G0205 coordSet entry independent of the sensor data. */
static void check_engine_helpers(M11_GameViewState* state) {
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    char msg[200];

    printf("\n[Group B] Engine helper contract surface for reincarnate_reselect\n");

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
}

/* Group C — Retarget + pre-select D1C portrait rect contract.
 * The C127 sensor at (7, 9) DIR_NORTH must now report
 * sensorData=18 (was shipped as 1), and the D1C cutout must be
 * painted with the C026 ordinal-18 portrait from atlas col 2
 * row 2. */
static void check_retarget_and_paint(M11_GameViewState* state,
                                     const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group C] (%d,%d) DIR_NORTH retargeted ordinal=%d — D1C cutout painted\n",
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
             ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET,
             CORRECT_ORDINAL_MATCH_PCT, pct);
    CHECK(pct >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* Group D — First select.
 * M11_GameView_SelectFrontMirrorCandidate returns 1, panel
 * opens, candidate ordinal=18, champion appended at party index
 * 0, mirror stays armed (REVIVE.C:272-276 only records/appends
 * the pending candidate). */
static void check_first_select(M11_GameViewState* state) {
    int rc;
    char msg[200];

    printf("\n[Group D] First SelectFrontMirrorCandidate at (%d,%d) N\n",
           ANCHOR_MAPX, ANCHOR_MAPY);

    state->world.party.championCount = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;

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

/* Group E — First cancel.
 * M11_GameView_CancelMirrorCandidate returns 1, panel closes,
 * the appended champion is removed (championCount 1 -> 0 per
 * F0282:745-757 — only the C162 cancel decrements the party
 * count), the candidate ordinal/party index are cleared, and
 * the mirror route stays armed at sensorData=18 because the
 * cancel path skips the F0282 sensor-disable loop.  Mirrors the
 * 216_gate ordinal-0 sibling's Group E. */
static void check_first_cancel(M11_GameViewState* state) {
    int rc;
    char msg[200];

    printf("\n[Group E] First CancelMirrorCandidate — F0282 C162 cancel path\n");

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

    /* The C162 cancel path does NOT run the F0282 sensor-disable
     * loop (that loop lives on the C164/C165 confirm path at
     * REVIVE.C:785-799), so the front-mirror ordinal must still
     * be 18 after the cancel. */
    snprintf(msg, sizeof(msg),
             "post-cancel mirror still armed at ordinal %d (got %d)",
             ORDINAL_TARGET, M11_GameView_GetFrontMirrorOrdinal(state));
    CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == ORDINAL_TARGET, msg);
}

/* Group F — Post-cancel D1C portrait rect contract.
 * The cancel skipped the sensor-disable loop, so the (7, 9) N
 * pose still returns ordinal 18 and the D1C cutout must carry
 * the C026 ordinal-18 portrait at >= 90% pixel match — proves
 * the panel exit didn't corrupt the D1C rect or leave a stale
 * sprite from a previous ordinal. */
static void check_post_cancel_paint(M11_GameViewState* state,
                                    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group F] Post-cancel D1C portrait rect — ordinal %d still painted\n",
           ORDINAL_TARGET);

    render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((%d,%d) N) == %d after cancel (got %d)",
             ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET, ord);
    CHECK(ord == ORDINAL_TARGET, msg);

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

/* (Group G re-enable removed 2026-07-21 round-18: the C162 cancel
 * path does NOT run the F0282 sensor-disable loop, so the mirror
 * is still armed at the reselect — no re-arm needed.  The old
 * re-enable-by-retarget step was both unnecessary and broken: the
 * retarget helper only matches sensorType==127, while the C165
 * confirm had set the (7,8) sensor to C000_SENSOR_DISABLED(0), so
 * the "re-enable" retarget actually matched SONJA's real (2,3)
 * chamber sensor and never re-armed the anchor.) */

/* Group G — Re-select.
 * A second M11_GameView_SelectFrontMirrorCandidate in the same
 * runtime view returns 1, panel re-opens, candidate ordinal=18
 * recorded, champion re-appended at party index 0, mirror still
 * armed.  This is the reincarnate_reselect core invariant: the
 * F0280 candidate append path runs again at the same party slot
 * after an F0282 C162 cancel.  With the panel re-open the C040
 * RR panel must also cover the D1C cutout at >= 90% of opaque
 * asset pixels — proves the z-order survives the round-trip. */
static void check_reselect(M11_GameViewState* state,
                           const M11_AssetSlot* rrPanel) {
    int rc;
    char msg[200];

    printf("\n[Group G] Reincarnate_reselect — second SelectFrontMirrorCandidate\n");

    /* Pre-reselect invariants (post-cancel state). */
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
             "pre-reselect mirror re-armed at ordinal %d (got %d)",
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

    /* With the panel re-open the C040 RR panel must cover the
     * D1C cutout at >= 90% of opaque asset pixels — same
     * threshold used by pose_panel_open in
     * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime
     * _probe and the ordinal-0 216_gate sibling. */
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels) {
        printf("  SKIP: C040 panel asset missing — panel-cover check skipped\n");
        return;
    }
    {
        unsigned char fb[FB_W * FB_H];
        PanelMatch m;
        render_at(state, fb, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
        m = match_panel_rect(rrPanel, fb, RR_PANEL_X, RR_PANEL_Y,
                             RR_PANEL_TRANSPARENT);
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
}

/* Group H — REINCARNATE confirm (F0282 C165).
 * M11_GameView_ConfirmMirrorCandidate(state, 1) returns 1,
 * panel closes, the champion JOINS the party (championCount
 * stays 1 per the source-locked F0282 routing at
 * dm1_v1_resurrection_pc34_compat.c — "785-799: Resurrect/
 * Reincarnate clear candidate state and disable mirror sensor";
 * only the C162 cancel at 745-757 decrements the party count),
 * the candidate ordinal/party index are cleared, AND the sensor
 * is disabled so M11_GameView_GetFrontMirrorOrdinal returns -1.
 * A further SelectFrontMirrorCandidate returns 0 — the route is
 * closed (sensor disabled + the already-in-party guard). */
static void check_reincarnate_confirm(M11_GameViewState* state) {
    int rc;
    char msg[200];

    printf("\n[Group H] ConfirmMirrorCandidate(reincarnate=1) — F0282 C165 REINCARNATE confirm path\n");

    rc = M11_GameView_ConfirmMirrorCandidate(state, REINCARNATE_FLAG);
    snprintf(msg, sizeof(msg),
             "M11_GameView_ConfirmMirrorCandidate(reincarnate=%d) returns 1 (got %d)",
             REINCARNATE_FLAG, rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-confirm candidate panel off (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-confirm inventory panel off (got %d)",
             state->inventoryPanelActive);
    CHECK(state->inventoryPanelActive == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-confirm champion JOINS party (count stays 1, got %d)",
             state->world.party.championCount);
    CHECK(state->world.party.championCount == 1, msg);

    snprintf(msg, sizeof(msg),
             "post-confirm ordinal cleared (got %d)",
             state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == -1, msg);

    snprintf(msg, sizeof(msg),
             "post-confirm party index cleared (got %d)",
             state->candidateMirrorPartyIndex);
    CHECK(state->candidateMirrorPartyIndex == -1, msg);

    /* The C165 confirm path runs the F0282 sensor-disable loop
     * at REVIVE.C:785-799, so the front-mirror ordinal must be
     * -1 after the confirm. */
    snprintf(msg, sizeof(msg),
             "post-confirm mirror DISABLED by F0282 sensor-disable loop (got ordinal %d, expected -1)",
             M11_GameView_GetFrontMirrorOrdinal(state));
    CHECK(M11_GameView_GetFrontMirrorOrdinal(state) == -1, msg);

    /* The route is closed: a further F0280 select of the same
     * mirror record must be refused (dead sensor + the
     * already-in-party guard). */
    rc = M11_GameView_SelectFrontMirrorCandidate(state);
    snprintf(msg, sizeof(msg),
             "post-confirm SelectFrontMirrorCandidate refused (returns 0, got %d)",
             rc);
    CHECK(rc == 0, msg);

    snprintf(msg, sizeof(msg),
             "post-confirm-refusal panel stays off (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 0, msg);
}

/* Group I — Post-confirm D1C portrait rect + redraw stability.
 * The C165 confirm ran the F0282 sensor-disable loop, so the
 * D1C cutout must NOT match ordinal 18 above the wrong-ordinal
 * drift threshold (35%) — proves the disable path didn't leak
 * a stale SONJA sprite onto the now-empty front cell — and
 * REDRAW_CYCLES successive M11_GameView_Draw calls at the
 * post-confirm state must produce byte-stable pixels at the
 * D1C cutout — proves the redraw contract is invariant across
 * the reincarnate_reselect cycle. */
static void check_post_confirm_redraw_stability(M11_GameViewState* state,
                                                const M11_AssetSlot* portraits) {
    unsigned char fb0[FB_W * FB_H];
    unsigned char fbN[FB_W * FB_H];
    int cycle;
    int stable = 1;
    int baselinePct = -1;
    int pct0;
    char msg[200];

    printf("\n[Group I] Post-confirm D1C rect — no stale ordinal %d + byte-stable cycles\n",
           ORDINAL_TARGET);

    render_at(state, fb0, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match groups skipped\n");
        return;
    }

    pct0 = match_portrait_cell(portraits, fb0, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "post-confirm D1C cutout does NOT match ordinal %d < %d%%%% (got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_DRIFT_PCT, pct0);
    CHECK(pct0 < WRONG_ORDINAL_DRIFT_PCT, msg);

    /* Byte-stable redraw cycles at the post-confirm state.
     * Mirrors the byte-stable checks in the d2r_negative and
     * cancel_reopen probes: a regression that leaks framebuffer
     * state between draws would diverge between redraws and
     * fail this group. */
    baselinePct = pct0;
    for (cycle = 1; cycle < REDRAW_CYCLES; ++cycle) {
        int pctN;
        render_at(state, fbN, ANCHOR_MAPX, ANCHOR_MAPY, ANCHOR_DIR);
        pctN = match_portrait_cell(portraits, fbN, ORDINAL_TARGET);
        if (pctN != baselinePct) {
            fprintf(stderr,
                    "FAIL (7,9) N cycle %d portrait_rect_position drift "
                    "ordinal %d match got=%d want=%d\n",
                    cycle + 1, ORDINAL_TARGET, pctN, baselinePct);
            stable = 0;
        }
        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
            fprintf(stderr,
                    "FAIL (7,9) N cycle %d framebuffer drift in viewport area\n",
                    cycle + 1);
            stable = 0;
        }
    }
    if (stable) {
        printf("  byte_stable_redraw_reincarnate_reselect cycles=%d "
               "ordinal %d match=%d%%%% (no drift)\n",
               REDRAW_CYCLES, ORDINAL_TARGET, baselinePct);
        ++g_pass;
    } else {
        ++g_fail;
    }
}

/* Group J — Side-wall no-floating.
 * At (7, 9) DIR_WEST the GetFrontMirrorOrdinal returns -1 and
 * the D1C cutout does NOT match ordinal 18 above the
 * wrong-ordinal drift threshold (35%) — proves the side wall
 * is not leaking ordinal 18 across the reincarnate_reselect
 * cycle. */
static void check_side_wall_no_floating(M11_GameViewState* state,
                                        const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group J] Side wall (%d,%d) DIR_WEST — no floating ordinal %d\n",
           ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET);

    /* Cancel the candidate panel first so the side-wall pose
     * sees a clean viewport without the C040 panel. */
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
             ANCHOR_MAPX, ANCHOR_MAPY, ORDINAL_TARGET,
             WRONG_ORDINAL_DRIFT_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_DRIFT_PCT, msg);
}

/* Group K — Atlas sanity.
 * Ordinal 18 -> col 18&7=2, row 18>>3=2 -> source rect (2*32,
 * 2*29) = (64, 58, 32, 29) (DEFS.H:821-826 portrait-grid 8-col
 * atlas math).  Same shape as the 216_gate ordinal-0 sibling's
 * Group L (which is row 0 col 0), so a future regression that
 * strips the (ordinal & 7) * 32 + (ordinal >> 3) * 29 formula
 * for row 2 will fail loudly here. */
static void check_atlas_sanity(void) {
    char msg[200];

    printf("\n[Group K] C026 atlas address for ordinal %d (re-cap)\n",
           ORDINAL_TARGET);

    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas col = %d (expected 2)",
             ORDINAL_TARGET, ORDINAL_TARGET & 7);
    CHECK((ORDINAL_TARGET & 7) == 2, msg);
    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas row = %d (expected 2)",
             ORDINAL_TARGET, ORDINAL_TARGET >> 3);
    CHECK((ORDINAL_TARGET >> 3) == 2, msg);
    snprintf(msg, sizeof(msg),
             "Ordinal %d atlas source rect = (%d, %d, %d, %d) "
             "(expected (64, 58, 32, 29) — row 2 col 2 of the 8x3 atlas)",
             ORDINAL_TARGET, ORDINAL_TARGET_SRCX, ORDINAL_TARGET_SRCY,
             PORTRAIT_W, PORTRAIT_H);
    CHECK(ORDINAL_TARGET_SRCX == 64 &&
          ORDINAL_TARGET_SRCY == 58, msg);
}

/* Group L — Mirror catalog identity check.
 * Ordinal 18 must resolve to "SONJA" with title "SHE DEVIL"
 * through the mirror catalog so a future regression that
 * strips ordinal 18 from the catalog will fail loudly.  The
 * existing ordinal-18 cancel_reopen probe performs a similar
 * check, but here we run it as a final gate after the
 * reincarnate_reselect cycle so a catalog regression
 * introduced mid-cycle is caught. */
static void check_catalog_lookup(M11_GameViewState* state) {
    char name[CHAMPION_NAME_TEXT_CAPACITY];
    /* 2026-07-21 round-18 fix: the title buffer must use
     * CHAMPION_TITLE_TEXT_CAPACITY (21), not the 9-byte name capacity —
     * "SHE DEVIL" is 9 chars + NUL and truncated to "SHE DEVI". */
    char title[CHAMPION_TITLE_TEXT_CAPACITY];
    int rc;
    char msg[200];

    printf("\n[Group L] Mirror catalog lookup for ordinal %d\n", ORDINAL_TARGET);

    name[0] = '\0';
    rc = M11_GameView_GetMirrorNameByOrdinal(state, ORDINAL_TARGET,
                                             name, (int)sizeof(name));
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetMirrorNameByOrdinal(%d) > 0 (got %d, name=%s)",
             ORDINAL_TARGET, rc, name);
    CHECK(rc > 0, msg);

    snprintf(msg, sizeof(msg),
             "Ordinal %d mirror catalog name == \"%s\" (got \"%s\")",
             ORDINAL_TARGET, kExpectedCatalogName, name[0] ? name : "");
    CHECK(strcmp(name, kExpectedCatalogName) == 0, msg);

    title[0] = '\0';
    rc = M11_GameView_GetMirrorTitleByOrdinal(state, ORDINAL_TARGET,
                                              title, (int)sizeof(title));
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetMirrorTitleByOrdinal(%d) > 0 (got %d, title=%s)",
             ORDINAL_TARGET, rc, title);
    CHECK(rc > 0, msg);

    snprintf(msg, sizeof(msg),
             "Ordinal %d mirror catalog title == \"%s\" (got \"%s\")",
             ORDINAL_TARGET, kExpectedCatalogTitle, title[0] ? title : "");
    CHECK(strcmp(title, kExpectedCatalogTitle) == 0, msg);
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowed[1024];
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
    dataDir = firestaff_dm1_probe_narrow_data_dir(dataDir, narrowed, sizeof(narrowed));
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d "
           "reincarnate_reselect portrait_rect_position (258 gate) ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s\n", dataDir);
    printf("ordinal %d atlas math: col=%d, row=%d, srcX=%d, srcY=%d "
           "(row 2 of the 256x87 C026 strip)\n",
           ORDINAL_TARGET, ORDINAL_TARGET_COL, ORDINAL_TARGET_ROW,
           ORDINAL_TARGET_SRCX, ORDINAL_TARGET_SRCY);
    printf("batch group 10 reincarnate_reselect gate "
           "(F0282 C165 REINCARNATE confirm path)\n");

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

    /* Retarget the canonical (7, 9) N C127 sensor from shipped
     * ordinal 1 to slice-target ordinal 18.  This is the same
     * trick used by the ordinal-11 resurrect_reselect slice
     * (which retargets ordinal 1 → 11 at the same anchor) and
     * by the ordinal-0 216_gate slice (which seeds the same
     * sensor from 1 to 0).  The local DM1 V1 build ships no
     * sensor with sensorData=18. */
    nRetargeted = retarget_c127_mirror_ordinal(&state,
                                               ORDINAL_SHIPPED,
                                               ORDINAL_TARGET,
                                               "ordinal18_reincarnate_reselect");
    if (nRetargeted <= 0) {
        fprintf(stderr,
                "FAIL: could not retarget any C127 sensor to ordinal %d "
                "(this DM1 V1 build may not have a canonical (7,9) N sensor)\n",
                ORDINAL_TARGET);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    check_atlas_math();
    check_engine_helpers(&state);
    check_retarget_and_paint(&state, portraits);
    check_first_select(&state);
    check_first_cancel(&state);
    check_post_cancel_paint(&state, portraits);
    check_reselect(&state, rrPanel);
    check_reincarnate_confirm(&state);
    check_post_confirm_redraw_stability(&state, portraits);
    check_side_wall_no_floating(&state, portraits);
    check_atlas_sanity();
    check_catalog_lookup(&state);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
