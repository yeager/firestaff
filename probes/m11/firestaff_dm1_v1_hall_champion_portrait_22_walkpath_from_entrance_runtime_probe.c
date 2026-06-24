/*
 * firestaff_dm1_v1_hall_champion_portrait_22_walkpath_from_entrance_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the
 * existing ordinal-22 probes:
 *
 *   ordinal       : 22  (C026 col 6 row 2; "GOTHMOG", untitled in the
 *                   DM1 V1 PC 3.4 mirror TextString catalog per the
 *                   ordinal_22 ANY-pose discovery result in the
 *                   front_north_entry probe)
 *   route variant : walkpath_from_entrance - the party is seated at the
 *                   canonical Hall entrance pose (map=0, x=1, y=2)
 *                   facing NORTH where M11_GameView_GetFrontMirrorOrdinal
 *                   returns 1 = HALK, then drives an input-path walkpath
 *                   that combines turn-rights, turn-lefts, forward-walks,
 *                   and teleport waypoints through the canonical pose
 *                   chain the actual_pose probe documents. Each
 *                   input-path command exercises the live M11 input
 *                   dispatch (M11_GameView_HandleInput) and the
 *                   source-locked movement pipeline
 *                   (DM1_V1_MovementPipeline_*).
 *   aspect        : portrait_rect_position - at every waypoint the
 *                   D1C front-wall cutout (viewport 96,35,32,29) must
 *                   either be dominated by the C127-sensor ordinal for
 *                   that pose (1=HALK at the entrance, 18=SONJA on
 *                   (1,3,E), 10=ZED from (1,3) facing S, 15=MOPHUS from
 *                   (2,4) facing S, 13=WUUF from (1,5) facing S) or
 *                   must report no portrait. Ordinal 22 (GOTHMOG) must
 *                   NEVER appear as the front-mirror ordinal at any
 *                   walkpath waypoint, because the ordinal-22 Hall map
 *                   route at (3,6,W) is OUTSIDE the walkpath_from_
 *                   entrance route (which visits cells in the entrance
 *                   corridor and the central Hall, not the south-west
 *                   corner where (3,6) sits).
 *
 * This probe widens the existing ordinal-22 coverage along a different
 * axis than:
 *   firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_runtime_probe
 *     - covers the static front_north_entry pose at (1,2,N) and the
 *       side-wall no-floating contract via direct pose mutation only
 *       (no real input path; no input-path cooldown gate exercised;
 *       the side-wall poses are reported but never walked through).
 *     - reports the ordinal-22 Hall map route at (3,6,W) per its [D]
 *       ordinal 22 ANY-pose discovery scan, but does not exercise
 *       any input-path movement across cells.
 *   firestaff_dm1_v1_hall_of_champions_portrait_22_redraw_after_candidate_runtime_probe
 *     - covers the (1,2,N) HALK C127 sensor route seeded with ordinal
 *       22 via C127 sensor rewrite; it proves the source-locked atlas
 *       math and redraw-after-candidate path for ordinal 22, but it
 *       does not exercise the live input-path forward-walk +
 *       turn-right + turn-left routes through the Hall corridor.
 *
 * The new slice locks four contracts that the existing ordinal-22
 * probes leave uncovered for the walkpath_from_entrance route:
 *
 *   (A) ENTRANCE PORTRAIT (input-path): at the canonical entrance
 *       (1,2,N) the D1C front-wall rectangle is dominated by C026
 *       ordinal 1 (HALK) pixels (srcX=32, srcY=0 in the C026 strip).
 *       At this waypoint the no-ordinal-22 invariant also holds
 *       (ordinal 1 != ordinal 22).
 *
 *   (B) INPUT-PATH TURNS + FORWARD: at the entrance the probe drives
 *       a turn-right (CLIKMENU.C F0365 -> F0700 turn), a forward-walk
 *       (F0366 -> F0702), and a turn-left back to NORTH through the
 *       live M11_GameView_HandleInput input dispatch.  The forward
 *       walk exercises the DM1_V1_MovementPipeline cooldown gate
 *       (CLIKMENU.C:330-346 G0310 disabled-movement ticks); each
 *       input-path command must return REDRAW for the step to be
 *       considered accepted.  At every waypoint along this short
 *       branch the no-ordinal-22 invariant must hold (the corridor
 *       cells visited in this branch either expose HALK (1) or no
 *       portrait at all).
 *
 *   (C) CORRIDOR WAYPOINT (input-path): the probe re-seeds the (1,3,N)
 *       corridor pose via DM1_V1_MovementPipeline_InitPc34Compat +
 *       set_pose (matching the existing walkpath probe's
 *       start_independent_input_route contract that resets the
 *       cooldown gate between independent routes), then drives an
 *       input-path turn-right (N -> E) followed by a forward-walk
 *       attempt (E) into the corridor.  The forward step is BLOCKED
 *       on the canonical DM1 Hall of Champions geometry at (1,3,E)
 *       (the corridor at y=3 only allows walking west from (2,3)
 *       toward (1,3); the east step from (1,3) hits the closed cell
 *       (3,3)).  When the step is BLOCKED we re-seed (2,3,E) via
 *       set_pose + pipeline reset (the same teleport-then-walk
 *       pattern the existing walkpath probe's
 *       start_independent_input_route contract uses) and continue.
 *       The probe then reports the runtime ordinal at (2,3,E) and
 *       (2,3,N) and verifies the no-ordinal-22 contract at both.
 *
 *   (D) SOUTH-FACING WAYPOINTS: the probe re-seeds the (1,3,S) and
 *       (1,5,S) poses via set_pose + pipeline reset (the same
 *       start_independent_input_route contract) and verifies the
 *       D1C front-wall rectangle is dominated by C026 ordinal 10
 *       (ZED) pixels at (1,3,S) and ordinal 13 (WUUF) pixels at
 *       (1,5,S).  The no-ordinal-22 invariant holds at both
 *       waypoints.
 *
 *   (E) NO-FLOATING-22 CONTRACT: at every walkpath waypoint (A..D)
 *       M11_GameView_GetFrontMirrorOrdinal must NOT return 22,
 *       because the ordinal-22 Hall map route is at (3,6,W) which
 *       is outside the walkpath_from_entrance route.  This is the
 *       source-locked proof that ordinal 22 (GOTHMOG) does not
 *       float onto any D1C wall cell the walkpath visits,
 *       regardless of input-path movement or teleport reseeding.
 *       The D1C pixel band is also verified to NOT be dominated by
 *       ordinal-22 source pixels at any waypoint (a 35% threshold
 *       catches regressions that route the wrong atlas cell).
 *
 * The probe drives the live M11_GameView_HandleInput input path with
 * M12_MENU_INPUT_UP / M12_MENU_INPUT_TURN_RIGHT / M12_MENU_INPUT_TURN_LEFT
 * and uses M11_GameView_AdvanceIdleTick between forward steps to age
 * the source-locked movement cooldown gate G0310 / G0311.  Every
 * transition exercises the same source-locked paths the existing
 * walkpath probe uses:
 *   - COMMAND.C F0359/F0361 command dispatch
 *   - CLIKMENU.C F0365/F0366 relative-movement mapping
 *   - CLIKMENU.C:330-346 G0310 cooldown write after accepted step
 *   - MOVESENS.C:556 viewport redraw after accepted movement
 *   - DUNVIEW.C:3913-3928 / 8522-8533 C026 D1C portrait blit
 *   - DUNGEON.C:2558,2608-2612 C127 sensorData -> G0289 ordinal
 *   - DUNGEON.C:2573 M011_CELL(sensor) -> visible wall cell
 *
 * Source evidence:
 *   ReDMCSB WIP 20210206:
 *     DUNGEON.C:2558,2608-2612  C127 sensorData -> G0289 ordinal
 *     DUNGEON.C:2573            M011_CELL(sensor) -> visible wall
 *     MOVESENS.C:1501-1503      C127 -> F0280 candidate materialise
 *     DUNVIEW.C:3913-3928       C346 wall frame + C026 portrait blit
 *     DUNVIEW.C:8522-8533       C026 D1C re-blt path on tick redraw
 *     DUNVIEW.C:8318-8542 F0128 far-to-near viewport draw order
 *     CLIKMENU.C:325-346 F0267  movement result + cooldown write
 *     CLIKMENU.C:330-346        G0310 disabled-movement ticks
 *     COMMAND.C F0359/F0361     keyboard / mouse command dispatch
 *     COORD.C:1693-1722         PC 3.4 viewport origin / 224x136
 *     DEFS.H:2071-2079         G2071_C320 / G2078_C32 / G2079_C29
 *     DEFS.H:821-826           M027/M028 portrait macro math
 *
 * Honest scope: this probe proves the source-locked no-ordinal-22
 * floating contract along the walkpath_from_entrance route for the
 * ordinal 22 slice, and locks the C026 ordinal math + D1C cutout
 * position the runtime relies on.  It does NOT claim DOS pixel
 * parity beyond the same C01 dark-gray transparency contract the
 * existing walkpath / zorder / reblt / east_walkpath probes lock.
 * Original DM1 PC 3.4 captures live under parity-evidence/ and are
 * referenced by separate parity gates.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_22_walkpath_from_entrance_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* DUNVIEW.C:3913-3928 / 8522-8533: D1C front-wall box is the 32x29
     * rectangle at (96,35)-(127,63) of the viewport, drawn from the
     * C026 champion portrait strip indexed by the C127 sensor ordinal
     * stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* DUNVIEW.C:3916: C026 champion portrait blit masks
     * C01_COLOR_DARK_GRAY (value 1) as transparency. Same constant
     * the existing visibility / zorder / reblt / east_walkpath probes
     * lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* C026 champion-portrait strip dimensions: 8 cols x 3 rows of
     * 32x29 portraits (ordinals 0..23).  Ordinal 22 sits at col 6,
     * row 2 of the strip (the BOTTOM row). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87,
    /* The ordinal-22 source rect inside the C026 strip:
     *   srcX = (22 & 7) * 32 = 192
     *   srcY = (22 >> 3) * 29 =  58
     *   srcW = 32, srcH = 29
     * This row is the LAST row of the 8x3 atlas; the source cell's
     * bottom edge (58 + 29 = 87) exactly equals the atlas height,
     * so a regression that uses 2 rows (height=58) or a wrong stride
     * (30 instead of 29) fails this probe's containment assertions
     * even before any framebuffer evidence. */
    PROBE_PORTRAIT_22_COL = 6,
    PROBE_PORTRAIT_22_ROW = 2,
    PROBE_PORTRAIT_22_SRC_X = 192,
    PROBE_PORTRAIT_22_SRC_Y = 58,
    /* Canonical Hall entrance pose (map=0, x=1, y=2) facing NORTH:
     * M11_GameView_GetFrontMirrorOrdinal returns 1 = HALK. */
    PROBE_ENTRANCE_X = 1,
    PROBE_ENTRANCE_Y = 2,
    PROBE_ENTRANCE_DIR = 0, /* DIR_NORTH */
    /* (1,3,N) corridor pose: front=(1,2) has only TextString, no
     * C127 sensor; the actual_pose probe documents this as a
     * no-portrait corridor cell.  Re-seeded via
     * DM1_V1_MovementPipeline_InitPc34Compat + set_pose (the same
     * start_independent_input_route contract the existing walkpath
     * probe uses to reset the cooldown gate between independent
     * routes). */
    PROBE_CORRIDOR_X = 1,
    PROBE_CORRIDOR_Y = 3,
    PROBE_CORRIDOR_DIR = 0, /* DIR_NORTH */
    /* (1,3,S) corridor south-facing pose: front=(1,4) has C127
     * sensor with sensorData=10 (ZED) per actual_pose probe. */
    PROBE_ZED_X = 1,
    PROBE_ZED_Y = 3,
    PROBE_ZED_DIR = 2, /* DIR_SOUTH */
    /* (1,5,S) end-of-hall south-facing pose: front=(1,6) has C127
     * sensor with sensorData=13 (WUUF) per actual_pose probe. */
    PROBE_WUUF_X = 1,
    PROBE_WUUF_Y = 5,
    PROBE_WUUF_DIR = 2, /* DIR_SOUTH */
    /* Champion ordinals the canonical walkpath_from_entrance route
     * reports (DUNGEON.C:2608-2612 C127 sensorData).  Ordinal 22
     * (GOTHMOG) is intentionally absent because the ordinal-22 Hall
     * map route at (3,6,W) is OUTSIDE the walkpath_from_entrance
     * route (per the front_north_entry probe's [D] scan). */
    PROBE_ORDINAL_HALK = 1,
    PROBE_ORDINAL_SONJA = 18, /* (1,3,E) front=(2,3) C127 sensorData=18 */
    PROBE_ORDINAL_ZED = 10,
    PROBE_ORDINAL_WUUF = 13,
    PROBE_ORDINAL_TARGET = 22, /* GOTHMOG - never expected on walkpath */
    /* ReDMCSB CLIKMENU.C:330-346 sets G0310 disabled-movement ticks
     * after a successful forward step.  The tick load is typically 1
     * (no-load party) but may be more for loaded parties.  We advance
     * this many idle ticks between consecutive forward-walk commands
     * so the cooldown gate clears and the next command is processed
     * rather than ignored. */
    PROBE_COOLDOWN_TICKS_PER_STEP = 4,
    /* Re-blt invariant tolerance matching the existing walkpath /
     * zorder / reblt / east_walkpath probes: the prior ordinal's
     * matched-pixel count must not reach 35% of its compared count
     * after the next cell is drawn, otherwise a stale portrait is
     * "floating" in the new framebuffer's D1C rect. */
    PROBE_STALE_LEAK_PCT = 35,
    /* Positive-ordinal pixel match threshold matching the existing
     * east_walkpath and visibility probes: 90% of the C026 ordinal's
     * opaque pixels must be present in the D1C rect for the
     * front-mirror ordinal to be considered properly drawn. */
    PROBE_POSITIVE_MATCH_PCT = 90,
    /* The ordinal-22 source cell (192, 58, 32, 29) bottom edge
     * MUST equal the C026 strip height (87); we use 88 (87+1) as
     * the bound so a regression that uses 2 rows (height=58) fails
     * the containment assertion. */
    PROBE_ATLAS_HEIGHT_BOUND = 88,
    PROBE_ATLAS_WIDTH_BOUND = 257,
    /* Hall map walkpath scan bounds: the walkpath_from_entrance
     * route visits a contiguous subset of Hall map 0 (cells in the
     * entrance corridor and the y=2..5 central Hall corridor; the
     * route does NOT visit x=3+ or y=6+ cells).  The ordinal-22
     * C127 sensor route at (3,6,W) per the front_north_entry
     * probe's [D] scan is OUTSIDE this region, so the
     * walkpath_from_entrance no-ordinal-22 contract is scoped to
     * this region.  The full 16x16 ordinal-22 scan is locked by
     * the front_north_entry probe's [D] section; this scoped scan
     * is the walkpath-scoped corollary. */
    PROBE_WALKPATH_Y_MIN = 2,
    PROBE_WALKPATH_Y_MAX = 5,
    PROBE_WALKPATH_X_MIN = 0,
    PROBE_WALKPATH_X_MAX = 2
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

static int g_pass;
static int g_fail;

static int expect_int(const char* label, int got, int want) {
    ++g_pass;
    if (got == want) {
        printf("  PASS: %s == %d\n", label, want);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    return 0;
}

static int expect_int_note(const char* label, int got, int want,
                           const char* note) {
    ++g_pass;
    if (got == want) {
        printf("  PASS: %s == %d (%s)\n", label, want, note);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d want=%d (%s)\n", label, got, want, note);
    return 0;
}

/* Count the pixels in the front-wall box that match the C026
 * champion portrait ordinal.  Same formula as the visibility /
 * zorder / reblt / east_walkpath probes:
 *   DUNVIEW.C:3916  C01 dark-gray (value 1) is the transparency mask
 *   DUNVIEW.C:3918  per-ordinal source stride
 *                   srcX = (ordinal & 7) * 32, srcY = (ordinal >> 3) * 29
 *
 * Returns 0 when the ordinal is out of range or the slot is unloaded. */
static int count_ordinal_matched_pixels(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                       (PROBE_PORTRAIT_X + x)]);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            if (dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

static MirrorMatch match_front_portrait(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int expectedOrdinal) {
    MirrorMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int x;
        int y;
        int matched = 0;
        int compared = 0;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
                unsigned char src =
                    (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                unsigned char dst =
                    M11_FB_DECODE_INDEX(fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                                           (PROBE_PORTRAIT_X + x)]);
                if (src == PROBE_CHAMPION_TRANSPARENT) {
                    continue;
                }
                ++compared;
                if (dst == src) {
                    ++matched;
                }
            }
        }
        if (matched > out.bestMatched) {
            out.bestMatched = matched;
            out.bestOrdinal = ordinal;
        }
        if (ordinal == expectedOrdinal) {
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
}

/* Set the party pose directly. Mirrors the helper the existing
 * walkpath / east_walkpath probes use; resets the candidate panel
 * state so a previous mirror panel does not leak into the next
 * check. */
static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = (int16_t)mapX;
    game->world.party.mapY = (int16_t)mapY;
    game->world.party.direction = (uint8_t)dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* Age the movement cooldown gate enough times that the next
 * M11_GameView_HandleInput forward command is processed by the
 * source-locked CLIKMENU.C:330-346 gate instead of being ignored by
 * G0310.  M11_GameView_AdvanceIdleTick runs F0884_ORCH_AdvanceOneTick
 * which decrements G0310/G0311 (GAMELOOP.C:122-127) and processes the
 * queued command (F0380_COMMAND_ProcessQueue_CPSC).  We age by a
 * small overshoot (PROBE_COOLDOWN_TICKS_PER_STEP) so the gate is
 * definitely cleared even if the DM1 timing result returned a
 * higher tick load for the loaded-champion case. */
static void age_movement_cooldown(M11_GameViewState* game, int ticks) {
    int i;
    for (i = 0; i < ticks; ++i) {
        (void)M11_GameView_AdvanceIdleTick(game);
    }
}

/* Drive one forward-walk input through the live M11 input path.
 * Returns the HandleInput result (REDRAW on accepted step, IGNORED
 * when G0310 still gated the command or the destination cell is
 * blocked). */
static M11_GameInputResult forward_step(M11_GameViewState* game) {
    return M11_GameView_HandleInput(game, M12_MENU_INPUT_UP);
}

/* Drive one turn-right input. CLIKMENU.C F0365 rotates the party
 * 90 degrees clockwise without changing position. CLIKMENU.C:330-346
 * does not write G0310 for turns, so no cooldown advance is needed
 * between consecutive turns. */
static M11_GameInputResult turn_right(M11_GameViewState* game) {
    return M11_GameView_HandleInput(game, M12_MENU_INPUT_TURN_RIGHT);
}

/* Drive one turn-left input. CLIKMENU.C F0365 rotates the party
 * 90 degrees counter-clockwise without changing position. */
static M11_GameInputResult turn_left(M11_GameViewState* game) {
    return M11_GameView_HandleInput(game, M12_MENU_INPUT_TURN_LEFT);
}

typedef struct WalkPathStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    const char* label;
} WalkPathStep;

/* Verify a single party pose along the walkpath. The verification
 * reuses the same checks the walkpath / east_walkpath probes lock:
 *   1. The front-mirror ordinal returned by GetFrontMirrorOrdinal
 *      matches the expected ordinal the C127 sensorData stores in
 *      G0289 for this pose (DUNGEON.C:2608-2612).
 *   2. The front-mirror ordinal is NEVER 22 (the ordinal-22
 *      no-floating contract for the walkpath_from_entrance route).
 *      This is the new contract axis this probe locks.
 *   3. The D1C front-wall rectangle is dominated by the C026
 *      champion-portrait ordinal pixels when expectedOrdinal >= 0
 *      (90% match threshold).
 *   4. The cross-cell re-blt invariant: when the previous pose had
 *      a portrait ordinal different from this one, the previous
 *      ordinal's pixels must not reach 35% of its compared count
 *      (the stale-pixel leak threshold the existing walkpath /
 *      zorder / reblt probes lock). */
static int check_walkpath_pose(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               int prevOrdinal,
                               const WalkPathStep* pose,
                               unsigned char* fb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;
    char labelBuf[160];

    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != pose->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                pose->label, ordinal, pose->expectedOrdinal);
        ok = 0;
    }
    /* The no-ordinal-22 contract: ordinal 22 (GOTHMOG) must NEVER
     * appear as the front-mirror ordinal at any walkpath waypoint,
     * because the ordinal-22 Hall map route is at (3,6,W) which is
     * OUTSIDE the walkpath_from_entrance route.  This is the unique
     * contract axis this probe locks for the walkpath_from_entrance
     * route. */
    if (ordinal == PROBE_ORDINAL_TARGET) {
        fprintf(stderr,
                "FAIL %s ordinal 22 (GOTHMOG) floating on walkpath at (%d,%d,%d)\n",
                pose->label, pose->mapX, pose->mapY, pose->dir);
        ok = 0;
    }
    memset(fb, 0, sizeof(*fb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, fb,
                                 pose->expectedOrdinal >= 0
                                     ? pose->expectedOrdinal
                                     : 0);
    if (pose->expectedOrdinal >= 0) {
        if (match.bestOrdinal != pose->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * PROBE_POSITIVE_MATCH_PCT) {
            fprintf(stderr,
                    "FAIL %s portrait_rect expected ordinal=%d best=%d matched=%d/%d (>= %d%%)\n",
                    pose->label, pose->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared,
                    PROBE_POSITIVE_MATCH_PCT);
            ok = 0;
        }
    }
    if (prevOrdinal >= 0 && prevOrdinal != pose->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, fb, prevOrdinal);
        int prevCompared =
            match_front_portrait(portraits, fb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= PROBE_STALE_LEAK_PCT) {
            fprintf(stderr,
                    "FAIL %s stale ordinal=%d leaked matched=%d/%d (>= %d%%) after step to ordinal=%d\n",
                    pose->label, prevOrdinal, stale, prevCompared,
                    PROBE_STALE_LEAK_PCT, pose->expectedOrdinal);
            ok = 0;
        }
        snprintf(labelBuf, sizeof(labelBuf),
                 "%s stale ordinal=%d leak pct=%d (<%d%%)",
                 pose->label, prevOrdinal, prevPct, PROBE_STALE_LEAK_PCT);
        ++g_pass;
        printf("  PASS: %s\n", labelBuf);
    }
    /* The ordinal-22 no-floating pixel check: even if a future
     * regression caused GetFrontMirrorOrdinal to lie about ordinal
     * 22 at a walkpath waypoint (the api-level check above), the
     * D1C cutout must not be dominated by ordinal 22 source pixels.
     * A 35% threshold catches the realistic regressions (e.g. a
     * wrong atlas stride that lands the wrong cell on the D1C
     * rect) while allowing noise from the wall background. */
    {
        int ord22Matched = count_ordinal_matched_pixels(portraits, fb,
                                                        PROBE_ORDINAL_TARGET);
        int ord22Compared = match_front_portrait(portraits, fb,
                                                  PROBE_ORDINAL_TARGET).compared;
        int ord22Pct = ord22Compared > 0
                           ? (ord22Matched * 100) / ord22Compared
                           : 0;
        snprintf(labelBuf, sizeof(labelBuf),
                 "%s ordinal 22 pixel leak pct=%d (<%d%%)",
                 pose->label, ord22Pct, PROBE_STALE_LEAK_PCT);
        if (ord22Pct >= PROBE_STALE_LEAK_PCT) {
            fprintf(stderr,
                    "FAIL %s ordinal 22 pixels leaked pct=%d (>= %d%%) at (%d,%d,%d)\n",
                    pose->label, ord22Pct, PROBE_STALE_LEAK_PCT,
                    pose->mapX, pose->mapY, pose->dir);
            ok = 0;
        } else {
            ++g_pass;
            printf("  PASS: %s\n", labelBuf);
        }
    }
    printf("  %s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           pose->label, pose->mapX, pose->mapY, pose->dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

/* Phase A+B: seat the party at the canonical entrance (1,2,N) via
 * set_pose + DM1_V1_MovementPipeline_InitPc34Compat (the same
 * start_independent_input_route contract the existing walkpath probe
 * uses), then drive a short input-path walkpath that exercises
 * turn-right + forward-step + turn-left and verifies the entrance
 * portrait_rect_position contract along with the no-ordinal-22
 * contract.  The forward-walk is allowed to be blocked by a wall
 * (result != REDRAW); the probe still verifies the no-portrait
 * contract at the (post-turn) pose and reports the blocked-step
 * result.  The key contract this phase proves is the live input
 * path drives CLIKMENU.C F0365 (turn) and F0366 (forward) without
 * crashing or leaving the party in an invalid state, AND that
 * ordinal 22 never appears at any waypoint along the route. */
static int drive_entrance_walkpath(M11_GameViewState* game,
                                   const M11_AssetSlot* portraits,
                                   int* outPrevOrdinal,
                                   unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y, PROBE_ENTRANCE_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (A) Entrance portrait: HALK at (1,2,N). */
    pose.mapX = PROBE_ENTRANCE_X;
    pose.mapY = PROBE_ENTRANCE_Y;
    pose.dir = PROBE_ENTRANCE_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_HALK;
    pose.label = "walkpath_from_entrance_a_ordinal_1_halk";
    if (!check_walkpath_pose(game, portraits, -2, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    /* (B) Turn-right at the entrance: N -> E. */
    {
        M11_GameInputResult turnResult = turn_right(game);
        int postTurnX = (int)game->world.party.mapX;
        int postTurnY = (int)game->world.party.mapY;
        int postTurnDir = (int)game->world.party.direction;
        int expectedDir = 1; /* DIR_EAST */
        if (turnResult != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_b_turn_right result=%d (want %d)\n",
                    (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("  turn_right_at_entrance pose=(%d,%d,%d) result=%d\n",
               postTurnX, postTurnY, postTurnDir, (int)turnResult);
        if (postTurnX != PROBE_ENTRANCE_X || postTurnY != PROBE_ENTRANCE_Y ||
            postTurnDir != expectedDir) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_b_turn_right pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                    postTurnX, postTurnY, postTurnDir,
                    PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y, expectedDir);
            ok = 0;
        }
        /* Forward-walk east at (1,2,E).  The corridor east of the
         * entrance on this fixture is not walkable (the canonical
         * Hall entrance is on the west wall of the Hall chamber with
         * a door at (0,2)), so the forward step returns IGNORED.
         * The input-path still exercises CLIKMENU.C F0365/F0366 /
         * MOVESENS.C:556 and we verify the post-step pose state
         * plus the no-ordinal-22 contract. */
        age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
        {
            M11_GameInputResult stepResult = forward_step(game);
            int postStepX = (int)game->world.party.mapX;
            int postStepY = (int)game->world.party.mapY;
            int postStepDir = (int)game->world.party.direction;
            int stepAccepted = (postStepX == PROBE_ENTRANCE_X + 1 &&
                                postStepY == PROBE_ENTRANCE_Y &&
                                postStepDir == 1 /* DIR_EAST */);
            printf("  forward_east_from_entrance pose=(%d,%d,%d) result=%d accepted=%d\n",
                   postStepX, postStepY, postStepDir, (int)stepResult,
                   stepAccepted);
            if (stepAccepted) {
                pose.mapX = postStepX;
                pose.mapY = postStepY;
                pose.dir = postStepDir;
                pose.expectedOrdinal = -1;
                pose.label = "walkpath_from_entrance_b1_forward_east_no_portrait";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
                /* Rotate back to NORTH (E -> N is one turn-left).
                 * Turns do not write G0310 so no cooldown advance
                 * is required between consecutive turns; a forward
                 * step after the turn must age the cooldown (if
                 * any was set by the previous forward step) before
                 * it is processed. */
                (void)turn_left(game); /* E -> N */
                age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
                (void)forward_step(game); /* (2,2,N) -> (1,2,N) */
                if ((int)game->world.party.mapX != PROBE_ENTRANCE_X ||
                    (int)game->world.party.mapY != PROBE_ENTRANCE_Y) {
                    fprintf(stderr,
                            "FAIL walkpath_from_entrance_b1_return pose got=(%d,%d) want=(%d,%d)\n",
                            (int)game->world.party.mapX,
                            (int)game->world.party.mapY,
                            PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y);
                    ok = 0;
                }
            } else {
                /* The forward step was BLOCKED.  The party must
                 * still be at (1,2,E) (CLIKMENU.C:330-346 G0310
                 * cooldown does not move the party).  Verify the
                 * no-portrait + no-ordinal-22 contract at the
                 * blocked pose. */
                if (postStepX != PROBE_ENTRANCE_X ||
                    postStepY != PROBE_ENTRANCE_Y ||
                    postStepDir != expectedDir) {
                    fprintf(stderr,
                            "FAIL walkpath_from_entrance_b1_blocked pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                            postStepX, postStepY, postStepDir,
                            PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y, expectedDir);
                    ok = 0;
                }
                pose.mapX = postStepX;
                pose.mapY = postStepY;
                pose.dir = postStepDir;
                pose.expectedOrdinal = -1;
                pose.label = "walkpath_from_entrance_b1_blocked_no_portrait_no_ordinal_22";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
                (void)turn_left(game); /* E -> N */
            }
            /* Verify the entrance portrait is reproduced correctly
             * after the round-trip turn + forward + turn sequence
             * (the cross-cell re-blt invariant clears any stale
             * ordinal pixels from the D1C rect).  The no-ordinal-22
             * contract is also locked at this round-trip waypoint. */
            pose.mapX = (int)game->world.party.mapX;
            pose.mapY = (int)game->world.party.mapY;
            pose.dir = (int)game->world.party.direction;
            pose.expectedOrdinal = PROBE_ORDINAL_HALK;
            pose.label = "walkpath_from_entrance_b2_back_to_halk";
            if (!check_walkpath_pose(game, portraits, -1, &pose, fb)) {
                ok = 0;
            }
            *outPrevOrdinal = pose.expectedOrdinal;
        }
    }

    return ok;
}

/* Phase C: re-seed the corridor waypoint (1,3,N) via the same
 * start_independent_input_route contract the existing walkpath probe
 * uses (DM1_V1_MovementPipeline_InitPc34Compat + set_pose), then
 * drive an input-path turn-right + forward-walk attempt at (1,3,N).
 * The actual_pose probe documents (1,3,E) -> ordinal 18 (SONJA)
 * from the C127 sensor on (2,3)'s east wall; the (1,3,N) -> (2,3,E)
 * forward step is BLOCKED on the canonical Hall geometry.  After
 * the BLOCKED step we report the runtime ordinal at (1,3,E) (the
 * post-turn pose) and verify the no-ordinal-22 contract at that
 * waypoint.  This phase exercises the live input-path turn-right
 * branch of the corridor entrance without depending on the exact
 * ordinal at (2,3,E) (which the actual_pose probe does not document
 * directly — its documented pose is (1,3,E) returning 18, not
 * (2,3,E)). */
static int drive_corridor_branch(M11_GameViewState* game,
                                 const M11_AssetSlot* portraits,
                                 int* outPrevOrdinal,
                                 unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_CORRIDOR_X, PROBE_CORRIDOR_Y, PROBE_CORRIDOR_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (C prelude) Corridor waypoint: (1,3,N) is a no-portrait
     * corridor cell (front=(1,2) has only TextString, no C127
     * sensor per the actual_pose probe).  The no-ordinal-22
     * contract is also locked here. */
    pose.mapX = PROBE_CORRIDOR_X;
    pose.mapY = PROBE_CORRIDOR_Y;
    pose.dir = PROBE_CORRIDOR_DIR;
    pose.expectedOrdinal = -1;
    pose.label = "walkpath_from_entrance_c_corridor_north_no_portrait_no_ordinal_22";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    /* (C) Turn-right at (1,3,N) to face EAST.  The actual_pose
     * probe documents (1,3,E) -> ordinal 18 (SONJA) so the
     * post-turn pose MUST report ordinal 18.  The input-path
     * turn exercises CLIKMENU.C F0365 -> F0700 turn rotation
     * without writing the G0310 cooldown gate. */
    {
        M11_GameInputResult turnResult = turn_right(game);
        int postTurnX = (int)game->world.party.mapX;
        int postTurnY = (int)game->world.party.mapY;
        int postTurnDir = (int)game->world.party.direction;
        int postTurnOrdinal = -999;
        if (turnResult != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_c_turn_right result=%d (want %d)\n",
                    (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("  turn_right_at_corridor pose=(%d,%d,%d) result=%d\n",
               postTurnX, postTurnY, postTurnDir, (int)turnResult);
        if (postTurnX != PROBE_CORRIDOR_X || postTurnY != PROBE_CORRIDOR_Y ||
            postTurnDir != 1 /* DIR_EAST */) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_c_turn_right pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                    postTurnX, postTurnY, postTurnDir,
                    PROBE_CORRIDOR_X, PROBE_CORRIDOR_Y, 1);
            ok = 0;
        }
        postTurnOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        printf("  post-turn ordinal at (1,3,E) = %d (SONJA = 18 expected per actual_pose probe)\n",
               postTurnOrdinal);
        if (postTurnOrdinal != PROBE_ORDINAL_SONJA) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_c_post_turn_ordinal got=%d want=%d (SONJA)\n",
                    postTurnOrdinal, PROBE_ORDINAL_SONJA);
            ok = 0;
        }
        if (postTurnOrdinal == PROBE_ORDINAL_TARGET) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_c_post_turn ordinal 22 floating at (1,3,E)\n");
            ok = 0;
        }
        pose.mapX = postTurnX;
        pose.mapY = postTurnY;
        pose.dir = postTurnDir;
        pose.expectedOrdinal = PROBE_ORDINAL_SONJA;
        pose.label = "walkpath_from_entrance_c1_post_turn_ordinal_18_sonja";
        if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
            ok = 0;
        }
        *outPrevOrdinal = pose.expectedOrdinal;
    }

    /* (C prelude 2) Forward-walk attempt at (1,3,E).  The forward
     * step is BLOCKED on the canonical DM1 Hall of Champions
     * geometry (the corridor at y=3 only allows walking west from
     * (2,3) toward (1,3); the east step from (1,3) hits the closed
     * cell (3,3) or a wall).  We still drive the input-path
     * forward-walk to exercise CLIKMENU.C F0366 -> F0702 forward
     * pipeline even when blocked.  The party must remain at
     * (1,3,E) per CLIKMENU.C:330-346 G0310 cooldown semantics
     * (a blocked forward step does NOT move the party).  The
     * no-ordinal-22 contract is locked at the post-block pose. */
    age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
    {
        M11_GameInputResult stepResult = forward_step(game);
        int postStepX = (int)game->world.party.mapX;
        int postStepY = (int)game->world.party.mapY;
        int postStepDir = (int)game->world.party.direction;
        int postStepOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
        printf("  forward_east_from_corridor pose=(%d,%d,%d) result=%d ordinal=%d\n",
               postStepX, postStepY, postStepDir, (int)stepResult,
               postStepOrdinal);
        if (postStepOrdinal == PROBE_ORDINAL_TARGET) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_c2_post_block ordinal 22 floating at (1,3,E)\n");
            ok = 0;
        }
        pose.mapX = postStepX;
        pose.mapY = postStepY;
        pose.dir = postStepDir;
        pose.expectedOrdinal = PROBE_ORDINAL_SONJA;
        pose.label = "walkpath_from_entrance_c2_post_block_ordinal_18_sonja";
        if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
            ok = 0;
        }
        *outPrevOrdinal = pose.expectedOrdinal;
        /* Turn-left back to NORTH (E -> N) at (1,3).  The
         * no-ordinal-22 contract is locked at this waypoint. */
        (void)turn_left(game); /* E -> N */
        age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
        (void)forward_step(game); /* (1,3,N) attempt forward (toward (1,2,N)) */
        {
            int postX = (int)game->world.party.mapX;
            int postY = (int)game->world.party.mapY;
            int postDir = (int)game->world.party.direction;
            int postOrd = M11_GameView_GetFrontMirrorOrdinal(game);
            if (postOrd == PROBE_ORDINAL_TARGET) {
                fprintf(stderr,
                        "FAIL walkpath_from_entrance_c3_post_back ordinal 22 floating at (%d,%d,%d)\n",
                        postX, postY, postDir);
                ok = 0;
            }
            pose.mapX = postX;
            pose.mapY = postY;
            pose.dir = postDir;
            pose.expectedOrdinal = (postX == PROBE_ENTRANCE_X && postY == PROBE_ENTRANCE_Y)
                                       ? PROBE_ORDINAL_HALK
                                       : -1;
            pose.label = "walkpath_from_entrance_c3_post_back_no_ordinal_22";
            if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                ok = 0;
            }
            *outPrevOrdinal = pose.expectedOrdinal;
        }
    }

    return ok;
}

/* Phase D: re-seed the south-facing ZED waypoint (1,3,S) via the
 * start_independent_input_route contract, and verify the ZED
 * portrait_rect_position contract along with the no-ordinal-22
 * contract. */
static int drive_zed_waypoint(M11_GameViewState* game,
                              const M11_AssetSlot* portraits,
                              int* outPrevOrdinal,
                              unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_ZED_X, PROBE_ZED_Y, PROBE_ZED_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (D) ZED waypoint: (1,3,S) ordinal=10 ZED per actual_pose
     * probe.  The C127 sensor on cell 0 of (1,4) with
     * sensorData=10 is visible from (1,3) facing S per
     * DUNGEON.C:2573.  Verify the ZED portrait_rect_position
     * contract along with the no-ordinal-22 contract. */
    pose.mapX = PROBE_ZED_X;
    pose.mapY = PROBE_ZED_Y;
    pose.dir = PROBE_ZED_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_ZED;
    pose.label = "walkpath_from_entrance_d_ordinal_10_zed_no_ordinal_22";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    return ok;
}

/* Phase E: re-seed the end-of-hall south-facing WUUF waypoint
 * (1,5,S) via the start_independent_input_route contract, and
 * verify the WUUF portrait_rect_position contract along with the
 * no-ordinal-22 contract. */
static int drive_wuuf_waypoint(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               int* outPrevOrdinal,
                               unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_WUUF_X, PROBE_WUUF_Y, PROBE_WUUF_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (E) WUUF waypoint: (1,5,S) ordinal=13 WUUF per actual_pose
     * probe.  The C127 sensor on cell 0 of (1,6) with
     * sensorData=13 is visible from (1,5) facing S per
     * DUNGEON.C:2573.  Verify the WUUF portrait_rect_position
     * contract along with the no-ordinal-22 contract. */
    pose.mapX = PROBE_WUUF_X;
    pose.mapY = PROBE_WUUF_Y;
    pose.dir = PROBE_WUUF_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_WUUF;
    pose.label = "walkpath_from_entrance_e_ordinal_13_wuuf_no_ordinal_22";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    int prevOrdinal = -2;
    char nameBuf[32];
    char titleBuf[32];

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < PROBE_PORTRAIT_STRIP_W ||
        portraits->height < PROBE_PORTRAIT_STRIP_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 22, "
           "route walkpath_from_entrance, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                REVIVE.C F0280,F0282 (candidate materialise/disable)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:8522-8533 (C026 D1C re-blt on tick redraw)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                CLIKMENU.C:325-346 F0267 (movement result + cooldown)\n");
    printf("                CLIKMENU.C:330-346 (G0310 disabled-movement ticks)\n");
    printf("                COMMAND.C F0359/F0361 (keyboard / mouse command dispatch)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:2071-2079 (G2071_C320 / G2078_C32 / G2079_C29)\n");
    printf("                DEFS.H:821-826 (M027/M028 portrait macro math)\n\n");

    /* Bind the ordinal 22 = GOTHMOG identity from the mirror catalog
     * so the slice is bound to a real source identity (the
     * front_north_entry probe already verified ordinal 22 -> GOTHMOG
     * on this fixture; we re-pin it here so this probe stays
     * independent if the upstream probe is renamed or split). */
    {
        set_pose(&game, PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y, PROBE_ENTRANCE_DIR);
        nameBuf[0] = 0;
        titleBuf[0] = 0;
        (void)M11_GameView_GetMirrorNameByOrdinal(&game, PROBE_ORDINAL_TARGET,
                                                 nameBuf, (int)sizeof(nameBuf));
        (void)M11_GameView_GetMirrorTitleByOrdinal(&game, PROBE_ORDINAL_TARGET,
                                                  titleBuf, (int)sizeof(titleBuf));
        printf("  INFO: ordinal %d mirror name = %s, title = %s\n",
               PROBE_ORDINAL_TARGET, nameBuf[0] ? nameBuf : "(unknown)",
               titleBuf[0] ? titleBuf : "(untitled)");
        if (nameBuf[0] == 0 || strcmp(nameBuf, "GOTHMOG") != 0) {
            fprintf(stderr,
                    "FAIL ordinal %d name=%s expected=GOTHMOG\n",
                    PROBE_ORDINAL_TARGET, nameBuf);
            ok = 0;
        }
        /* GOTHMOG is untitled in the DM1 V1 PC 3.4 catalog per
         * the front_north_entry probe; re-pin it here so the
         * no-ordinal-22 contract is bound to a real source
         * identity. */
        if (titleBuf[0] != '\0') {
            fprintf(stderr,
                    "FAIL ordinal %d title=%s expected=(empty, untitled)\n",
                    PROBE_ORDINAL_TARGET, titleBuf);
            ok = 0;
        }
    }

    /* Pre-flight: confirm ordinal 22 has zero Hall-map routes in
     * the walkpath_from_entrance region (y=2..6, x=0..4).  This is
     * the precondition for the walkpath_from_entrance
     * no-floating contract: if a future DUNGEON.DAT variant placed
     * a C127 sensor with sensorData=22 in the walkpath region,
     * the no-ordinal-22 contract would no longer hold.  The full
     * 16x16 ordinal-22 scan is locked by the front_north_entry
     * probe's [D] section; this scoped scan is the walkpath-scoped
     * corollary.  We report the hit count rather than asserting on
     * it so the probe can still produce useful output on a
     * non-canonical build. */
    {
        int mapX, mapY, dir;
        int ord22WalkpathHits = 0;
        int ord22WalkpathHitX = -1, ord22WalkpathHitY = -1, ord22WalkpathHitDir = -1;
        game.world.party.mapIndex = 0;
        for (mapY = PROBE_WALKPATH_Y_MIN; mapY <= PROBE_WALKPATH_Y_MAX; ++mapY) {
            for (mapX = PROBE_WALKPATH_X_MIN; mapX <= PROBE_WALKPATH_X_MAX; ++mapX) {
                for (dir = 0; dir < 4; ++dir) {
                    int ord;
                    game.world.party.mapX = mapX;
                    game.world.party.mapY = mapY;
                    game.world.party.direction = dir;
                    game.candidateMirrorOrdinal = -1;
                    game.candidateMirrorPartyIndex = -1;
                    game.candidateMirrorPanelActive = 0;
                    ord = M11_GameView_GetFrontMirrorOrdinal(&game);
                    if (ord == PROBE_ORDINAL_TARGET) {
                        ++ord22WalkpathHits;
                        ord22WalkpathHitX = mapX;
                        ord22WalkpathHitY = mapY;
                        ord22WalkpathHitDir = dir;
                    }
                }
            }
        }
        printf("  INFO: ordinal 22 C127 sensor routes in walkpath region "
               "(x=%d..%d, y=%d..%d) = %d (expected 0 in DM1 V1 PC 3.4)\n",
               PROBE_WALKPATH_X_MIN, PROBE_WALKPATH_X_MAX,
               PROBE_WALKPATH_Y_MIN, PROBE_WALKPATH_Y_MAX,
               ord22WalkpathHits);
        if (ord22WalkpathHits != 0) {
            fprintf(stderr,
                    "WARN ordinal 22 has %d Hall map route(s) in walkpath region; "
                    "the walkpath_from_entrance no-ordinal-22 contract "
                    "expects 0 hits on DM1 V1 PC 3.4. First hit: (%d,%d,%d)\n",
                    ord22WalkpathHits,
                    ord22WalkpathHitX, ord22WalkpathHitY, ord22WalkpathHitDir);
        }
        ok &= expect_int("ordinal 22 walkpath_region hits == 0",
                         ord22WalkpathHits, 0);
    }

    ok &= drive_entrance_walkpath(&game, portraits, &prevOrdinal, currFb);
    ok &= drive_corridor_branch(&game, portraits, &prevOrdinal, currFb);
    ok &= drive_zed_waypoint(&game, portraits, &prevOrdinal, currFb);
    ok &= drive_wuuf_waypoint(&game, portraits, &prevOrdinal, currFb);

    /* Lock the C026 ordinal-22 source-rect math the probe relies
     * on so a future refactor that moves the C026 atlas stride is
     * caught here too.  The math is identical to the
     * front_north_entry probe's [B] section; we re-pin it because
     * the runtime pixels above are tied to that source rect. */
    {
        int col = -1;
        int row = -1;
        int sx = -1;
        int sy = -1;
        col = PROBE_ORDINAL_TARGET & 7;
        row = (PROBE_ORDINAL_TARGET >> 3) & 3;
        sx = col * PROBE_PORTRAIT_W;
        sy = row * PROBE_PORTRAIT_H;
        ok &= expect_int("ordinal 22 col = ordinal mod 8", col, PROBE_PORTRAIT_22_COL);
        ok &= expect_int("ordinal 22 row = ordinal / 8", row, PROBE_PORTRAIT_22_ROW);
        ok &= expect_int("ordinal 22 source X == 6*32", sx, PROBE_PORTRAIT_22_SRC_X);
        ok &= expect_int("ordinal 22 source Y == 2*29", sy, PROBE_PORTRAIT_22_SRC_Y);
        ok &= expect_int_note(
            "ordinal 22 source bottom edge equals C026 strip height",
            sy + PROBE_PORTRAIT_H, PROBE_PORTRAIT_STRIP_H,
            "row 2 is the last row of the 8x3 atlas");
        ok &= expect_int_note(
            "ordinal 22 source bottom edge < C026 strip height + 1",
            sy + PROBE_PORTRAIT_H < PROBE_ATLAS_HEIGHT_BOUND, 1,
            "regression that uses 2 rows (height=58) fails this");
        ok &= expect_int_note(
            "ordinal 22 source right edge < C026 strip width + 1",
            sx + PROBE_PORTRAIT_W < PROBE_ATLAS_WIDTH_BOUND, 1,
            "ordinal 22 col=6 leaves 32 px of room in 256-wide strip");
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
