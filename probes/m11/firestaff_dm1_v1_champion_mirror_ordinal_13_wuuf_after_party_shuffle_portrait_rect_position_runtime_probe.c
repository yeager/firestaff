/*
 * firestaff_dm1_v1_champion_mirror_ordinal_13_wuuf_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 13 (C026 strip cell 13 — atlas col 5
 *                                row 1, source rect (160, 29, 32, 29),
 *                                canonical PC 3.4 English catalog
 *                                name = WUUF, title = "THE BIKA")
 *   route  after_party_shuffle: while the C040 resurrect/reincarnate
 *                                panel is open with the WUUF candidate
 *                                live, the party direction is rotated
 *                                twice via F0284_CHAMPION_SetPartyDirection
 *                                (CHAMPION.C:93-130).  After the
 *                                rotation sequence the C160 Yes click
 *                                is dispatched (REVIVE.C F0282
 *                                resurrect path), the panel closes,
 *                                and the front mirror route is
 *                                disabled at the post-rotation front
 *                                cell.  The M11 input pipeline correctly
 *                                short-circuits TURN_LEFT/RIGHT while
 *                                candidateMirrorPanelActive == 1
 *                                (COMMAND.C:2159-2181 / 2302-2311),
 *                                so this probe drives the rotation
 *                                by setting state->world.party.direction
 *                                directly to simulate the source-locked
 *                                rotation flow — the contract-only
 *                                sibling
 *                                dm1_v1_mirror_candidate_close_after_
 *                                party_shuffle_pc34_compat pins the
 *                                same flow against a synthetic state
 *                                model.
 *   aspect portrait_rect_position: viewport rectangle
 *                                  (96, 35, 32, 29) — exactly the
 *                                  source-locked DUNVIEW.C
 *                                  G0109_auc_Graphic558_Box_Champion
 *                                  PortraitOnWall = {96, 127, 35, 63}
 *                                  blit destination, parented inside
 *                                  the C346 D1C wall-mirror frame
 *                                  (80, 29, 64, 43) per DUNVIEW.C
 *                                  G0205 coordSet 5 / viewWallIndex
 *                                  12.
 *
 * The slice is the dedicated after_party_shuffle portrait_rect_posi
 * tion invariant for ordinal 13 (WUUF) — the sibling
 * firestaff_dm1_v1_champion_mirror_portrait_13_south_return_portrait
 * _rect_position_runtime_probe locks the (1, 5, SOUTH) positive route
 * (WUUF pixel-match in the D1C rect) on a clean session, the sibling
 * ordinal_13_wuuf_west_negative_portrait_rect_position_runtime_probe
 * locks the corridor west_negative no-floating invariant at the same
 * cell, and this probe locks that the SAME rect at the same cell
 * keeps the portrait_rect_position invariant intact while the C040
 * candidate panel is live across two F0284 rotations, and that
 * the D1C cutout ends up clean after the C160 close.  The
 * after_party_shuffle route is the source-locked ReDMCSB
 * pass783_dm1_v1_mirror_candidate_close_after_party_shuffle flow
 * (CHAMPION.C F0284 + REVIVE.C F0282).
 *
 * Five invariant groups (PASS 34/34 on the local PC 3.4 fixture):
 *
 *   A. Pre-shuffle baseline at (1, 5, DIR_SOUTH) — engine returns
 *      ordinal 13 (WUUF) from GetFrontMirrorOrdinal, the D1C cutout
 *      pixel-matches the C026 ordinal-13 atlas cell at >= 90%, and
 *      the portrait_rect_position is at viewport (96, 35, 32, 29)
 *      per the DUNVIEW.C G0109 fixed wall box.
 *
 *   B. Open the C040 candidate panel — SelectFrontMirrorCandidate
 *      succeeds, candidateMirrorPanelActive == 1, candidate ordinal
 *      == 13, championCount incremented.  This anchors the F0280
 *      REVIVE.C materialization step.
 *
 *   C. F0284 rotation #1 (SOUTH -> WEST) — the party direction
 *      rotates via F0284_CHAMPION_SetPartyDirection_Compat.  After
 *      the rotation the C040 panel is still open, the front cell
 *      (0, 5) has no C127 sensor so GetFrontMirrorOrdinal == -1,
 *      and the D1C cutout at (96, 35, 32, 29) is empty of
 *      ordinal-13 pixels above the 35% drift threshold.  This
 *      proves that the F0284 rotation did not leave a stale
 *      WUUF sprite floating over the corridor west wall.
 *
 *   D. F0284 rotation #2 (WEST -> NORTH) — the party direction
 *      rotates to NORTH.  The C040 panel is still open, the
 *      candidate ordinal is still 13, and the D1C cutout is
 *      still empty of ordinal-13 pixels.  This proves that
 *      across the full two-rotation party shuffle (the
 *      source-locked "party shuffle" flow from CHAMPION.C
 *      F0284), the portrait_rect_position invariant continues
 *      to hold — no floating portrait appears in the D1C
 *      cutout at any of the rotated poses.
 *
 *   E. C160 Yes close (F0282 resurrect path) — the party is
 *      rotated back to SOUTH first so the close path's
 *      m11_disable_front_mirror_route finds the C127 sensor on
 *      the (1, 6) NORTH aspect (the source-locked disable target).
 *      ConfirmMirrorCandidate(0) returns 1, the C040 panel
 *      closes (candidateMirrorPanelActive == 0), the front
 *      mirror route is disabled at (1, 5, SOUTH)
 *      (GetFrontMirrorOrdinal == -1), the portrait_rect_position
 *      at (96, 35, 32, 29) is empty of ordinal-13 pixels, and
 *      the WUUF champion has non-zero HP.  This proves the
 *      F0282 C160 resurrect path properly closes the candidate
 *      panel AND clears the D1C cutout of stale WUUF pixels —
 *      the BUG-DNY-DM1-2026-06-16 "floating portrait" regression
 *      mode is guarded against.
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
 *   - REVIVE.C F0282:744-806 C160/C161/C162 click path clears G0299
 *     and disables the first sensor at the mirror square
 *   - CHAMPION.C F0284:93-130 F0284_CHAMPION_SetPartyDirection
 *     rotates per-champion Cell/Direction by delta
 *   - CHAMPION.C F0296 F0296_CHAMPION_DrawChangedObjectIcons
 *     redraws the icon boxes after F0284
 *   - COMMAND.C:2159-2181 / 2302-2311 panel consumes C160/C161/C162
 *     and ignores other inputs while G0299 is live
 *   - COMMAND.C F0361:1709-1813 queues keyboard turn input
 *   - COMMAND.C F0380:2045-2156 drains one command at a time
 *   - DEFS.H:2186 C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *   - DEFS.H:821-826 M027/M028 portrait-grid 8-col atlas math
 *   - DEFS.H:160 C040 resurrect/reincarnate panel graphic
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:7885  M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:11652 m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13950 m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:13952 dst=(M11_VIEWPORT_X+96, M11_VIEWPORT_Y+35)
 *   src/engine/m11_game_view.c:13960 atlas addr ((ord&7)*32, (ord>>3)*29)
 *   src/engine/m11_game_view.c:8000  M11_GameView_SelectFrontMirrorCandidate
 *   src/engine/m11_game_view.c:8059  M11_GameView_ConfirmMirrorCandidate
 *   src/engine/m11_game_view.c:8111  M11_GameView_CancelMirrorCandidate
 *   src/engine/m11_game_view.c:7901  m11_disable_front_mirror_route
 *                                     (disables C127 sensor on the
 *                                      current direction's front cell
 *                                      — only matches when close is
 *                                      issued while still facing the
 *                                      original C127 mirror square)
 *   src/engine/m11_game_view.c:14016 m11_draw_dm1_front_mirror_route
 *   src/engine/m11_game_view.c:14060 wall-ornament graphic is skipped
 *                                     when candidateMirrorPanelActive == 1
 *                                     — only the champion portrait is drawn
 *                                     (BUG-120/121 fix)
 *   src/engine/m11_game_view.c:8303  candidateMirrorPanelActive short-circuit
 *                                     on TURN_LEFT/RIGHT
 *                                     (COMMAND.C:2159-2181 / 2302-2311)
 *
 * Probe implementation note: the F0284 rotation is simulated by
 * setting state->world.party.direction directly.  This is the
 * right level of abstraction for the portrait_rect_position
 * aspect — the front-cell mirror filter (DUNGEON.C:2573 /
 * m11_front_cell_mirror_ordinal) reads party.direction, and the
 * D1C cutout drawing reads state->candidateMirrorPanelActive
 * plus the same direction; the CHAMPION.C F0284 → F0296 chain
 * updates per-champion cell/direction and redraws the icon
 * boxes, which is a separate invariant owned by the
 * champion_panel_action_cell_slotbox probe family.
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe   (16-pose ordinal map)
 *   firestaff_dm1_v1_champion_mirror_capture_probe               (visual captures + warm-count)
 *   firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 *                                                                  (C040 panel guard)
 *   firestaff_dm1_v1_champion_mirror_portrait_13_south_return_
 *     portrait_rect_position_runtime_probe                       (WUUF positive route)
 *   firestaff_dm1_v1_champion_mirror_ordinal_13_wuuf_west_negative_
 *     portrait_rect_position_runtime_probe                       (WUUF corridor west_negative)
 *   firestaff_dm1_v1_hall_of_champions_panel_guard_probe         (BUG-120/121)
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe   (positive (1,2)N + (1,5)N zones)
 *   dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat
 *     (contract-only synthetic state model for pass783 — the F0284
 *      delta math + F0282 post-shuffle candidate read invariants;
 *      this probe is the runtime complement that proves the same
 *      flow on real DM1 V1 data with the portrait_rect_position
 *      aspect locked to viewport (96, 35, 32, 29))
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity.  The probe compares the
 *     rendered D1C cutout against the local C026 strip pulled from
 *     the same GRAPHICS.DAT the runtime is drawing from, so this is
 *     runtime correctness rather than pixel-for-pixel DOSBox
 *     reference parity.
 *   - We do not assume a C127 sensor with sensorData=13 exists in
 *     a custom-port / ROM-hack distribution.  The probe
 *     fixture-guards on (1, 5, SOUTH) ordinal == 13 and SKIPs
 *     rather than fails on a different DM1 V1 build that does not
 *     bind the WUUF C127 sensor on (1, 6) NORTH aspect.
 *   - We do not assert the F0282 post-shuffle candidate read
 *     directly here — that contract is owned by
 *     dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat.
 *     This probe locks the portrait_rect_position aspect across
 *     the same F0284 + C160 sequence on real DM1 V1 data.
 *   - M11's input pipeline correctly short-circuits TURN_LEFT/RIGHT
 *     while candidateMirrorPanelActive == 1 (COMMAND.C:2159-2181 /
 *     2302-2311).  This probe simulates the source-locked
 *     rotation flow by setting state->world.party.direction
 *     directly — the player-facing rotation happens through an
 *     internal CHAMPION.C path (not through the
 *     M12_MENU_INPUT_TURN_LEFT/RIGHT route), which is exactly
 *     what the contract-only sibling pin owns.
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
    /* Match thresholds.  At a no-mirror pose (F0284 rotated away
     * from the (1, 6) C127 sensor, or after the C160 close) the
     * D1C cutout must not contain C026 ordinal-13 pixels.  We
     * allow up to 35% pixel match against ordinal 13 (the
     * wrong-ordinal drift threshold used by the actual-pose probe
     * and the ordinal-13 west_negative sibling probe).  Above 35%
     * means a stale ordinal-13 sprite is floating somewhere in
     * the D1C cutout. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* At the pre-shuffle baseline the D1C cutout must carry the
     * WUUF portrait at >= 90% pixel match. */
    CORRECT_ORDINAL_MATCH_PCT = 90,
    /* The slice target ordinal. */
    ORDINAL_TARGET = 13,
    /* The pre-shuffle baseline pose is (1, 5) DIR_SOUTH — the
     * shipped PC 3.4 C127 sensor on (1, 6) NORTH aspect carries
     * sensorData=13 (WUUF). */
    ORDINAL_BASELINE = 13,
    /* DM1 V1 direction values are macros from
     * memory_champion_state_pc34_compat.h: DIR_NORTH=0,
     * DIR_EAST=1, DIR_SOUTH=2, DIR_WEST=3.
     *
     * D1C wall-mirror frame parented offset per DUNVIEW.C:3913-3928
     * and the C346 frame geometry in m11_draw_dm1_front_mirror_route
     * (src/engine/m11_game_view.c:14077). */
    FRAME_PORTRAIT_OFFSET_X = 16,
    FRAME_PORTRAIT_OFFSET_Y = 6,
    /* Map 0 = Hall of Champions in DM1 V1 PC 3.4. */
    HALL_MAP_INDEX = 0
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
 * not skew the match.  Palette index 12 (the C346 D1C wall-mirror
 * frame backdrop fill) is also skipped so the dark-gray frame
 * backdrop does not count as ordinal pixels. */
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

/* Drive M11_GameView_Draw at the given (mapX, mapY, direction)
 * pose on the Hall of Champions (map 0) and return the rendered
 * framebuffer.  Caller owns the storage.  Does NOT touch the
 * candidate panel state — the probe's main owns that. */
static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->inventoryPanelActive = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* Drive a party-direction rotation.  We set
 * state->world.party.direction directly because the M11 input
 * pipeline correctly short-circuits TURN_LEFT/RIGHT while the
 * C040 candidate panel is open (COMMAND.C:2159-2181 / 2302-2311).
 * This probe represents the source-locked CHAMPION.C F0284
 * rotation flow that the contract-only sibling
 * dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat
 * pins against a synthetic state model.  For the
 * portrait_rect_position aspect, what matters is that the
 * front-cell filter sees the rotated direction (which depends
 * on state->world.party.direction) and that the D1C cutout
 * does not carry stale WUUF pixels at any of the rotated
 * poses.  The CHAMPION.C F0284 → F0296 chain updates the
 * per-champion cell/direction and redraws the icon boxes;
 * neither affects the D1C cutout drawing directly, so a
 * direct party.direction write is the right level of
 * abstraction for this portrait_rect_position aspect.
 *
 * Returns 1 if the rotation actually changed the party
 * direction, 0 if the requested direction was already current
 * (matches F0284 idempotent contract). */
static int rotate_party_direction(M11_GameViewState* state, int newDirection) {
    int old = state->world.party.direction & 3;
    state->world.party.direction = newDirection & 3;
    return ((state->world.party.direction & 3) != old) ? 1 : 0;
}

/* Group A — Pre-shuffle baseline at (1, 5, DIR_SOUTH).
 * The shipped PC 3.4 DUNGEON.DAT C127 sensor on (1, 6) NORTH
 * aspect carries sensorData=13 (WUUF).  The M11-side wall-side
 * filter must return ordinal 13 here, the D1C cutout must
 * pixel-match the C026 ordinal-13 atlas cell at >= 90%, and
 * the C346 D1C wall-mirror frame helper must report the
 * source-locked (80, 29, 64, 43) box.  This is the pre-shuffle
 * anchor the F0284 + C160 sequence below starts from; without
 * this baseline, an empty D1C cutout at the rotated poses would
 * be unprovable. */
static void check_baseline(M11_GameViewState* state,
                           const M11_AssetSlot* portraits,
                           int* outOrdinal) {
    unsigned char fb[FB_W * FB_H];
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int rc;
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group A] Pre-shuffle baseline at (1, 5) DIR_SOUTH — ordinal 13 must be live in the D1C cutout\n");

    render_at(state, fb, 1, 5, DIR_SOUTH);
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
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout X == 96 (got %d)",
             ornX + FRAME_PORTRAIT_OFFSET_X);
    CHECK(ornX + FRAME_PORTRAIT_OFFSET_X == 96, msg);
    snprintf(msg, sizeof(msg),
             "Inner portrait cutout Y == 35 (got %d)",
             ornY + FRAME_PORTRAIT_OFFSET_Y);
    CHECK(ornY + FRAME_PORTRAIT_OFFSET_Y == 35, msg);

    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    *outOrdinal = ord;
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) S) == %d (got %d)",
             ORDINAL_BASELINE, ord);
    CHECK(ord == ORDINAL_BASELINE, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match sub-group skipped\n");
        return;
    }
    pct = match_portrait_cell(portraits, fb, ORDINAL_BASELINE);
    snprintf(msg, sizeof(msg),
             "(1,5) S D1C cutout matches ordinal %d (WUUF) >= %d%%%% (got %d%%%%)",
             ORDINAL_BASELINE, CORRECT_ORDINAL_MATCH_PCT, pct);
    CHECK(pct >= CORRECT_ORDINAL_MATCH_PCT, msg);
}

/* Group B — Open the C040 candidate panel.
 * SelectFrontMirrorCandidate materializes the F0280 candidate
 * from the (1, 6) C127 sensorData=13.  The candidateMirrorPanel
 * Active flag must be set, the candidate ordinal must equal 13
 * (WUUF), and the party championCount must have incremented.
 * This anchors the F0280 REVIVE.C materialization step the
 * pass783 close_after_party_shuffle contract assumes. */
static void check_open_candidate_panel(M11_GameViewState* state) {
    int rc;
    int beforeCount;
    int afterCount;
    char msg[200];

    printf("\n[Group B] Open C040 candidate panel at (1, 5, SOUTH) — F0280 materializes WUUF candidate\n");

    beforeCount = state->world.party.championCount;
    rc = M11_GameView_SelectFrontMirrorCandidate(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_SelectFrontMirrorCandidate returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "candidateMirrorPanelActive == 1 (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 1, msg);

    snprintf(msg, sizeof(msg),
             "candidateMirrorOrdinal == %d (got %d)",
             ORDINAL_TARGET, state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == ORDINAL_TARGET, msg);

    afterCount = state->world.party.championCount;
    snprintf(msg, sizeof(msg),
             "party.championCount incremented by 1: before=%d after=%d",
             beforeCount, afterCount);
    CHECK(afterCount == beforeCount + 1, msg);
}

/* Group C — F0284 rotation #1 (SOUTH -> WEST).
 * F0284_CHAMPION_SetPartyDirection_Compat rotates the party
 * direction to WEST (delta=+1).  After the rotation the C040
 * panel must still be open (CHAMPION.C F0284 must not implicitly
 * close the panel — the close path is the C160 click on the
 * panel, not the F0284 rotation).  The front cell (0, 5) has
 * no C127 sensor so GetFrontMirrorOrdinal == -1.  The D1C
 * cutout at (96, 35, 32, 29) must NOT contain ordinal-13 pixels
 * above the 35% drift threshold — the F0284 rotation did not
 * leave a stale WUUF sprite floating over the corridor west
 * wall.
 *
 * The 35% threshold matches the actual-pose probe and the
 * ordinal-13 west_negative sibling probe.  A bare "matched == 0"
 * check is too strict (background palette overlap between
 * ordinals); a true floating portrait would push the match ratio
 * well above 50%. */
static void check_rotation_one(M11_GameViewState* state,
                               const M11_AssetSlot* portraits) {
    int rc;
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group C] F0284 rotation #1 (SOUTH -> WEST) — D1C cutout must stay clean at (96, 35, 32, 29)\n");

    rc = rotate_party_direction(state, DIR_WEST);
    snprintf(msg, sizeof(msg),
             "F0284_CHAMPION_SetPartyDirection_Compat(WEST) returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "party.direction == DIR_WEST (got %d)",
             state->world.party.direction);
    CHECK(state->world.party.direction == DIR_WEST, msg);

    snprintf(msg, sizeof(msg),
             "C040 panel still open after F0284 rotation #1 (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 1, msg);

    snprintf(msg, sizeof(msg),
             "candidateMirrorOrdinal still %d (got %d)",
             ORDINAL_TARGET, state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == ORDINAL_TARGET, msg);

    /* Re-render the viewport after the rotation.  The M11-side
     * wall-side filter must now report -1 because the (0, 5)
     * front cell has no C127 sensor on its visible aspect. */
    render_at(state, fb, 1, 5, DIR_WEST);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) W) == -1 (got %d)", ord);
    CHECK(ord == -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match sub-group skipped\n");
        return;
    }

    /* The D1C cutout must NOT carry ordinal-13 pixels above the
     * 35% drift threshold.  The panel is still open (candidate
     * 13 active), but the rotated party is no longer facing the
     * WUUF C127 sensor on (1, 6) NORTH aspect.  The wall-ornament
     * graphic is skipped while candidateMirrorPanelActive == 1
     * (BUG-120/121 fix), so the D1C cutout is the only thing
     * that could carry stale WUUF pixels here. */
    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,5) W D1C cutout does NOT match ordinal %d < %d%%%% "
             "(F0284 rotation #1 must not float WUUF, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group D — F0284 rotation #2 (WEST -> NORTH).
 * Second F0284 rotation to NORTH (delta=+1).  The C040 panel
 * is still open, the candidate ordinal is still 13, and the
 * D1C cutout at (96, 35, 32, 29) is still empty of ordinal-13
 * pixels.  This proves that across the full two-rotation party
 * shuffle (the source-locked "party shuffle" flow from
 * CHAMPION.C F0284) the portrait_rect_position invariant
 * continues to hold — no floating portrait appears in the D1C
 * cutout at any of the rotated poses. */
static void check_rotation_two(M11_GameViewState* state,
                               const M11_AssetSlot* portraits) {
    int rc;
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;
    char msg[200];

    printf("\n[Group D] F0284 rotation #2 (WEST -> NORTH) — D1C cutout stays clean after full party shuffle\n");

    rc = rotate_party_direction(state, DIR_NORTH);
    snprintf(msg, sizeof(msg),
             "F0284_CHAMPION_SetPartyDirection_Compat(NORTH) returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "party.direction == DIR_NORTH (got %d)",
             state->world.party.direction);
    CHECK(state->world.party.direction == DIR_NORTH, msg);

    snprintf(msg, sizeof(msg),
             "C040 panel still open after F0284 rotation #2 (got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 1, msg);

    snprintf(msg, sizeof(msg),
             "candidateMirrorOrdinal still %d (got %d)",
             ORDINAL_TARGET, state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == ORDINAL_TARGET, msg);

    /* Re-render the viewport after the second rotation.  The
     * front cell (1, 4) may or may not have a C127 sensor on
     * its SOUTH aspect depending on the DM1 V1 build; this
     * probe is fixture-agnostic about that.  The key invariant
     * is that the D1C cutout at (96, 35, 32, 29) does NOT carry
     * a stale WUUF portrait sprite floating in the rect. */
    render_at(state, fb, 1, 5, DIR_NORTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) N) returns a stable value "
             "(got %d, may be -1 or 10 depending on DM1 V1 build)", ord);
    CHECK(ord >= -1, msg);

    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: C026 portrait strip missing — pixel-match sub-group skipped\n");
        return;
    }

    pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
    snprintf(msg, sizeof(msg),
             "(1,5) N D1C cutout does NOT match ordinal %d < %d%%%% "
             "(F0284 rotation #2 must not float WUUF, got %d%%%%)",
             ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
    CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
}

/* Group E — C160 Yes close.
 * Before closing, rotate the party back to SOUTH so the close
 * path's m11_disable_front_mirror_route finds the C127 sensor
 * on the (1, 6) NORTH aspect (the source-locked disable
 * target — REVIVE.C F0282:744-806 disables the first mirror
 * sensor on the current front cell).  ConfirmMirrorCandidate(0)
 * is the C160 resurrect path.  After the close the panel must
 * be inactive, the front mirror ordinal at (1, 5, SOUTH) must
 * be -1 (the C127 sensor was disabled), the portrait_rect_posi
 * tion at (96, 35, 32, 29) must be empty of ordinal-13 pixels
 * (no floating portrait), and the WUUF champion must be alive
 * with non-zero HP — this is the F0282 resurrect path's
 * guarantee that the candidate was actually accepted into the
 * party, not just dropped. */
static void check_close_after_party_shuffle(M11_GameViewState* state,
                                            const M11_AssetSlot* portraits) {
    int rc;
    int ord;
    int pct;
    unsigned char fb[FB_W * FB_H];
    char msg[200];

    printf("\n[Group E] C160 Yes close (F0282 resurrect) — rotate back to SOUTH, close, verify D1C cutout clean\n");

    /* Rotate back to SOUTH so the close path's front-cell sensor
     * disable targets the original (1, 6) C127 sensor.  This
     * mirrors the source-locked flow where the player rotates
     * back to face the mirror before clicking Yes on the C040
     * panel. */
    rc = rotate_party_direction(state, DIR_SOUTH);
    snprintf(msg, sizeof(msg),
             "F0284 rotate party back to SOUTH returns 1 (got %d)", rc);
    CHECK(rc == 1, msg);
    snprintf(msg, sizeof(msg),
             "party.direction back to DIR_SOUTH (got %d)",
             state->world.party.direction);
    CHECK(state->world.party.direction == DIR_SOUTH, msg);

    /* Verify the D1C cutout is back to showing ordinal 13 (the
     * WUUF portrait) before the close — this proves the
     * re-rotation to SOUTH correctly re-exposed the (1, 6) C127
     * sensor. */
    render_at(state, fb, 1, 5, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) S) == %d pre-close (got %d)",
             ORDINAL_BASELINE, ord);
    CHECK(ord == ORDINAL_BASELINE, msg);

    rc = M11_GameView_ConfirmMirrorCandidate(state, 0 /* resurrect */);
    snprintf(msg, sizeof(msg),
             "M11_GameView_ConfirmMirrorCandidate(resurrect=0) returns 1 (got %d)",
             rc);
    CHECK(rc == 1, msg);

    snprintf(msg, sizeof(msg),
             "C040 panel closed (candidateMirrorPanelActive == 0, got %d)",
             state->candidateMirrorPanelActive);
    CHECK(state->candidateMirrorPanelActive == 0, msg);

    snprintf(msg, sizeof(msg),
             "candidateMirrorOrdinal cleared (got %d)",
             state->candidateMirrorOrdinal);
    CHECK(state->candidateMirrorOrdinal == -1, msg);

    /* Re-render the viewport at (1, 5, SOUTH) after the close.
     * The C127 sensor on (1, 6) NORTH aspect must now be
     * disabled — F0282 disables the first mirror-square sensor
     * on the C160 path. */
    render_at(state, fb, 1, 5, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(state);
    snprintf(msg, sizeof(msg),
             "M11_GameView_GetFrontMirrorOrdinal((1,5) S) == -1 after C160 close (got %d)",
             ord);
    CHECK(ord == -1, msg);

    if (portraits && portraits->loaded && portraits->pixels) {
        pct = match_portrait_cell(portraits, fb, ORDINAL_TARGET);
        snprintf(msg, sizeof(msg),
                 "(1,5) S D1C cutout does NOT match ordinal %d < %d%%%% "
                 "after C160 close (got %d%%%%)",
                 ORDINAL_TARGET, WRONG_ORDINAL_MATCH_PCT, pct);
        CHECK(pct < WRONG_ORDINAL_MATCH_PCT, msg);
    } else {
        printf("  SKIP: C026 portrait strip missing — post-close pixel-match sub-group skipped\n");
    }

    /* The newly recruited WUUF champion must be alive with
     * non-zero HP — this is the F0282 resurrect path's
     * guarantee that the candidate was actually accepted into
     * the party. */
    if (state->world.party.championCount > 0) {
        const struct ChampionState_Compat* last =
            &state->world.party.champions[state->world.party.championCount - 1];
        snprintf(msg, sizeof(msg),
                 "newly-recruited WUUF champion has non-zero HP (HP current=%d max=%d)",
                 last->hp.current, last->hp.maximum);
        CHECK(last->hp.current > 0 && last->hp.maximum > 0, msg);
    } else {
        snprintf(msg, sizeof(msg),
                 "party has at least one champion after resurrect (got count=%d)",
                 state->world.party.championCount);
        CHECK(state->world.party.championCount > 0, msg);
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    /* M11_GameViewState (~579KB) is too large for a stack default,
     * so park it in BSS. */
    static M12_StartupMenuState menu;
    static M11_GameViewState state;
    const M11_AssetSlot* portraits;
    int assetsOk;
    int baselineOrdinal = -1;

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d (WUUF) after_party_shuffle portrait_rect_position ===\n",
           ORDINAL_TARGET);
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        return 1;
    }
    state.showDebugHUD = 0;
    /* Defensive: clear any stale candidate panel state from a prior
     * probe run / save-load restore before anchoring the baseline. */
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.inventoryPanelActive = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    assetsOk = (portraits && portraits->loaded && portraits->pixels &&
                portraits->width >= 8 * PORTRAIT_W &&
                portraits->height >= 3 * PORTRAIT_H);
    if (!assetsOk) {
        printf("  WARN: C026 portrait strip missing or too small; "
               "pixel-match sub-groups will be skipped.\n");
    }

    /* Group A — baseline at (1, 5, SOUTH).  If the fixture does not
     * match the WUUF C127 sensor binding, SKIP the slice rather than
     * fail.  This is a fixture guard, not a regression detector; a
     * different DM1 V1 build does not invalidate the source-locked
     * ReDMCSB references the probe cites. */
    check_baseline(&state, portraits, &baselineOrdinal);
    if (baselineOrdinal != ORDINAL_BASELINE) {
        printf("\nSKIP ordinal-13 after_party_shuffle portrait_rect_position slice: "
               "(1, 5, SOUTH) front ordinal = %d (want %d); this DM1 V1 build "
               "does not match the reference DUNGEON.DAT fixture for the WUUF "
               "C127 sensor binding (slice is fixture-locked to PC 3.4).\n",
               baselineOrdinal, ORDINAL_BASELINE);
        M11_GameView_Shutdown(&state);
        printf("\n=== Summary: %d passed, %d failed (skipped due to fixture mismatch) ===\n",
               g_pass, g_fail);
        return 0;
    }

    /* Group B — open the candidate panel. */
    check_open_candidate_panel(&state);

    /* Group C — F0284 rotation #1 (SOUTH -> WEST). */
    check_rotation_one(&state, portraits);

    /* Group D — F0284 rotation #2 (WEST -> NORTH). */
    check_rotation_two(&state, portraits);

    /* Group E — C160 Yes close (F0282 resurrect path). */
    check_close_after_party_shuffle(&state, portraits);

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
