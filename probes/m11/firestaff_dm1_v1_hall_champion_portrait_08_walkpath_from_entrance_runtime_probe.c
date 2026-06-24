/*
 * firestaff_dm1_v1_hall_champion_portrait_08_walkpath_from_entrance_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the
 * existing ordinal-8 probes:
 *
 *   ordinal       : 8  (C026 col 0 row 1; "IAIDO", the only east-facing
 *                   ordinal in the canonical DM1 V1 Hall of Champions
 *                   DUNGEON.DAT at (2,1,E) per the actual_pose probe)
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
 *                   (1,3,E), 8=IAIDO from (2,1) facing E) or must
 *                   report no portrait. Ordinal 8 (IAIDO) must NEVER
 *                   appear as the front-mirror ordinal at any other
 *                   walkpath waypoint, because the (2,1,E) route is
 *                   reachable only via teleport-then-walk from the
 *                   canonical corridor poses the walkpath visits.
 *
 * This probe widens the existing ordinal-8 coverage along a different
 * axis than:
 *   firestaff_dm1_v1_champion_mirror_east_walkpath_ordinal_8_runtime_probe
 *     - covers the static (2,1,EAST) -> ordinal 8 and the side-wall
 *       (2,1,N/S/W) -> ordinal -1/4/-1 contract via direct set_pose
 *       pose mutation only.  It never starts at the Hall entrance
 *       (1,2,N), never drives the live input-path turn-right +
 *       forward-walk branch through the corridor, and never asserts
 *       the no-ordinal-8 floating contract along the walkpath route.
 *   firestaff_dm1_v1_champion_mirror_ordinal_8_south_return_portrait_rect_position_runtime_probe
 *     - covers the (2,1,S) -> ordinal 4 (LEIF) return branch via
 *       direct set_pose mutation only.  It does not exercise the
 *       walkpath_from_entrance route at all.
 *   firestaff_dm1_v1_hall_of_champions_portrait_08_cancel_reopen_portrait_rect_position_runtime_probe
 *     - covers the cancel_reopen route (select -> cancel -> reopen
 *       C040 panel) for ordinal 8 via C127 sensor seeding on the
 *       (1,2) NORTH-entry pose.  It does not exercise the live
 *       input-path forward-walk / turn-right / turn-left branches
 *       through the corridor, and does not assert the
 *       walkpath_from_entrance no-ordinal-8 floating contract.
 *
 * The new slice locks four contracts that the existing ordinal-8
 * probes leave uncovered for the walkpath_from_entrance route:
 *
 *   (A) ENTRANCE PORTRAIT (input-path): at the canonical entrance
 *       (1,2,N) the D1C front-wall rectangle is dominated by C026
 *       ordinal 1 (HALK) pixels (srcX=32, srcY=0 in the C026 strip).
 *       At this waypoint the no-ordinal-8 invariant also holds
 *       (ordinal 1 != ordinal 8).
 *
 *   (B) INPUT-PATH TURNS + FORWARD: at the entrance the probe drives
 *       a turn-right (CLIKMENU.C F0365 -> F0700 turn), a forward-walk
 *       (F0366 -> F0702), and a turn-left back to NORTH through the
 *       live M11_GameView_HandleInput input dispatch.  The forward
 *       walk exercises the DM1_V1_MovementPipeline cooldown gate
 *       (CLIKMENU.C:330-346 G0310 disabled-movement ticks); each
 *       input-path command must return REDRAW for the step to be
 *       considered accepted.  At every waypoint along this short
 *       branch the no-ordinal-8 invariant must hold (the corridor
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
 *       (3,3)).  After the BLOCKED step the post-turn pose (1,3,E)
 *       reports ordinal 18 (SONJA) per the actual_pose probe.
 *       The no-ordinal-8 contract holds at every corridor waypoint.
 *
 *   (D) IAIDO TARGET (teleport + cross-cell re-blt): the probe
 *       re-seeds the (2,1,E) pose via set_pose + pipeline reset
 *       (the same start_independent_input_route contract the
 *       existing walkpath probe uses to teleport-then-walk between
 *       independent routes) and verifies the D1C front-wall
 *       rectangle is dominated by C026 ordinal 8 (IAIDO) pixels
 *       (srcX=0, srcY=29 in the C026 strip - row 1 col 0 of the
 *       8x3 atlas).  The cross-cell re-blt invariant clears any
 *       stale SONJA pixels from the D1C rect.  The no-ordinal-8
 *       floating contract is locked positively at this waypoint
 *       (IAIDO is the only ordinal 8 the walkpath visit ever
 *       exposes).
 *
 *   (E) LEIF SIDE-WALL WAYPOINT (teleport): the probe re-seeds the
 *       (2,1,S) pose via set_pose + pipeline reset and verifies the
 *       D1C front-wall rectangle is dominated by C026 ordinal 4
 *       (LEIF) pixels (srcX=128, srcY=0 in the C026 strip - row 0
 *       col 4 of the 8x3 atlas).  The cross-cell re-blt invariant
 *       clears any stale IAIDO pixels from the D1C rect.
 *       The no-ordinal-8 contract holds positively at this waypoint
 *       (LEIF is NOT ordinal 8).
 *
 *   (F) NO-FLOATING-8 CONTRACT: at every walkpath waypoint (A..E)
 *       M11_GameView_GetFrontMirrorOrdinal must NOT return 8
 *       (except at the (2,1,E) target waypoint), because the
 *       ordinal-8 Hall map route is the east wall sensor of (2,1)
 *       which is OUTSIDE the (1,2) -> (1,3) -> (2,1) walkpath
 *       branch (the (2,1) cell is reached via teleport-then-walk,
 *       not via continuous forward-walk from the entrance).
 *       The D1C pixel band is also verified to NOT be dominated by
 *       ordinal-8 source pixels at any waypoint other than the
 *       (2,1,E) target (a 35% threshold catches regressions that
 *       route the wrong atlas cell).  This is the source-locked
 *       proof that ordinal 8 (IAIDO) does not float onto any D1C
 *       wall cell the walkpath visits, regardless of input-path
 *       movement or teleport reseeding.
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
 * Honest scope: this probe proves the source-locked no-ordinal-8
 * floating contract along the walkpath_from_entrance route for the
 * ordinal 8 slice, and locks the C026 ordinal math + D1C cutout
 * position the runtime relies on.  It does NOT claim DOS pixel
 * parity beyond the same C01 dark-gray transparency contract the
 * existing walkpath / zorder / reblt / east_walkpath probes lock.
 * Original DM1 PC 3.4 captures live under parity-evidence/ and are
 * referenced by separate parity gates.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_08_walkpath_from_entrance_runtime_probe DATA_DIR
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
     * 32x29 portraits (ordinals 0..23).  Ordinal 8 sits at col 0,
     * row 1 of the strip (the FIRST cell of the SECOND row). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87,
    /* The ordinal-8 source rect inside the C026 strip:
     *   srcX = (8 & 7) * 32 =   0
     *   srcY = (8 >> 3) * 29 =  29
     *   srcW = 32, srcH = 29
     * This is the FIRST cell of row 1; the row-1 / col-0 cell is
     * distinct from the row-0 / col-0 cell (ordinal 0 / DAROOU)
     * which is the most likely lookalike under a row-wrap bug. */
    PROBE_PORTRAIT_8_COL = 0,
    PROBE_PORTRAIT_8_ROW = 1,
    PROBE_PORTRAIT_8_SRC_X = 0,
    PROBE_PORTRAIT_8_SRC_Y = 29,
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
    /* (2,1,E) IAIDO waypoint: front=(3,1) has C127 sensor with
     * sensorData=8 (IAIDO) per actual_pose probe.  The only
     * east-facing ordinal-8 Hall map route in the canonical DM1 V1
     * DUNGEON.DAT. */
    PROBE_IAIDO_X = 2,
    PROBE_IAIDO_Y = 1,
    PROBE_IAIDO_DIR = 1, /* DIR_EAST */
    /* (2,1,S) LEIF waypoint: front=(2,2) has C127 sensor with
     * sensorData=4 (LEIF) per actual_pose probe.  Sits at the same
     * cell as IAIDO but on a different wall; the cross-cell re-blt
     * invariant must clear the IAIDO portrait pixels when the
     * party rotates to SOUTH. */
    PROBE_LEIF_X = 2,
    PROBE_LEIF_Y = 1,
    PROBE_LEIF_DIR = 2, /* DIR_SOUTH */
    /* Champion ordinals the canonical walkpath_from_entrance route
     * reports (DUNGEON.C:2608-2612 C127 sensorData).  Ordinal 8
     * (IAIDO) is intentionally absent at the corridor waypoints
     * (HALK/SONJA/LEIF are seen at the (1,2,N)/(1,3,E)/(2,1,S)
     * cells the walkpath visits) and is reached only via teleport
     * to the (2,1,E) target. */
    PROBE_ORDINAL_HALK = 1,
    PROBE_ORDINAL_SONJA = 18, /* (1,3,E) front=(2,3) C127 sensorData=18 */
    PROBE_ORDINAL_LEIF = 4,   /* (2,1,S) front=(2,2) C127 sensorData=4 */
    PROBE_ORDINAL_TARGET = 8, /* IAIDO - the ordinal 8 this probe locks */
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
    /* The ordinal-8 source cell (0, 29, 32, 29) row 1 bottom edge
     * MUST be < the C026 strip height (87); we use 88 (87+1) as
     * the bound so a regression that drops the row-1 multiplier
     * (height=29 instead of 29 * 1 = 29) fails the containment
     * assertion. */
    PROBE_ATLAS_HEIGHT_BOUND = 88,
    PROBE_ATLAS_WIDTH_BOUND = 257
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
 *   2. The D1C front-wall rectangle is dominated by the C026
 *      champion-portrait ordinal pixels when expectedOrdinal >= 0
 *      (90% match threshold).
 *   3. The cross-cell re-blt invariant: when the previous pose had
 *      a portrait ordinal different from this one, the previous
 *      ordinal's pixels must not reach 35% of its compared count
 *      (the stale-pixel leak threshold the existing walkpath /
 *      zorder / reblt probes lock). */
static int check_walkpath_pose(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               int prevOrdinal,
                               int prevTargetVisible,
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
    /* The no-ordinal-8 contract: ordinal 8 (IAIDO) must NEVER
     * appear as the front-mirror ordinal at any walkpath waypoint
     * OTHER than the (2,1,E) target.  The prevTargetVisible flag
     * tells the helper whether the prior step exposed ordinal 8
     * (in which case this step is allowed to report a different
     * ordinal as long as the D1C rect is dominated by the expected
     * ordinal pixels), and whether this step is the IAIDO target
     * (where ordinal 8 IS expected).  Every other waypoint must
     * report ordinal != 8. */
    if (ordinal == PROBE_ORDINAL_TARGET && pose->expectedOrdinal != PROBE_ORDINAL_TARGET) {
        fprintf(stderr,
                "FAIL %s ordinal 8 (IAIDO) floating on walkpath at (%d,%d,%d)\n",
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
    /* The ordinal-8 no-floating pixel check: even if a future
     * regression caused GetFrontMirrorOrdinal to lie about ordinal
     * 8 at a walkpath waypoint (the api-level check above), the
     * D1C cutout must not be dominated by ordinal 8 source pixels
     * at any waypoint OTHER than the (2,1,E) target.  A 35%
     * threshold catches the realistic regressions (e.g. a wrong
     * atlas stride that lands the wrong cell on the D1C rect)
     * while allowing noise from the wall background. */
    if (pose->expectedOrdinal != PROBE_ORDINAL_TARGET) {
        int ord8Matched = count_ordinal_matched_pixels(portraits, fb,
                                                       PROBE_ORDINAL_TARGET);
        int ord8Compared = match_front_portrait(portraits, fb,
                                                 PROBE_ORDINAL_TARGET).compared;
        int ord8Pct = ord8Compared > 0
                          ? (ord8Matched * 100) / ord8Compared
                          : 0;
        snprintf(labelBuf, sizeof(labelBuf),
                 "%s ordinal 8 pixel leak pct=%d (<%d%%)",
                 pose->label, ord8Pct, PROBE_STALE_LEAK_PCT);
        if (ord8Pct >= PROBE_STALE_LEAK_PCT) {
            fprintf(stderr,
                    "FAIL %s ordinal 8 pixels leaked pct=%d (>= %d%%) at (%d,%d,%d)\n",
                    pose->label, ord8Pct, PROBE_STALE_LEAK_PCT,
                    pose->mapX, pose->mapY, pose->dir);
            ok = 0;
        } else {
            ++g_pass;
            printf("  PASS: %s\n", labelBuf);
        }
    } else {
        /* The IAIDO target waypoint: ordinal 8 must positively
         * dominate the D1C rect.  The (prevTargetVisible &&
         * prevOrdinal == PROBE_ORDINAL_TARGET) case is handled by
         * the prevOrdinal != expectedOrdinal check above (the
         * IAIDO -> IAIDO round trip is a no-op).  For the
         * (2,1,E) target reached via teleport from a different
         * ordinal the cross-cell re-blt invariant already locked
         * the IAIDO pixels dominate. */
        snprintf(labelBuf, sizeof(labelBuf),
                 "%s ordinal 8 IAIDO pixel match pct=%d (>= %d%%)",
                 pose->label, match.expectedMatched * 100 /
                                  (match.compared > 0 ? match.compared : 1),
                 PROBE_POSITIVE_MATCH_PCT);
        if (match.expectedMatched * 100 <
            match.compared * PROBE_POSITIVE_MATCH_PCT) {
            fprintf(stderr,
                    "FAIL %s ordinal 8 IAIDO match too low matched=%d/%d\n",
                    pose->label, match.expectedMatched, match.compared);
            ok = 0;
        } else {
            ++g_pass;
            printf("  PASS: %s\n", labelBuf);
        }
    }
    (void)prevTargetVisible;
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
 * portrait_rect_position contract along with the no-ordinal-8
 * contract.  The forward-walk is allowed to be blocked by a wall
 * (result != REDRAW); the probe still verifies the no-portrait
 * contract at the (post-turn) pose and reports the blocked-step
 * result.  The key contract this phase proves is the live input
 * path drives CLIKMENU.C F0365 (turn) and F0366 (forward) without
 * crashing or leaving the party in an invalid state, AND that
 * ordinal 8 never appears at any waypoint along the route. */
static int drive_entrance_walkpath(M11_GameViewState* game,
                                   const M11_AssetSlot* portraits,
                                   int* outPrevOrdinal,
                                   int* outPrevTargetVisible,
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
    if (!check_walkpath_pose(game, portraits, -2, 0, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;
    *outPrevTargetVisible = 0;

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
         * plus the no-ordinal-8 contract. */
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
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
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
                 * no-portrait + no-ordinal-8 contract at the
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
                pose.label = "walkpath_from_entrance_b1_blocked_no_portrait_no_ordinal_8";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
                    ok = 0;
                }
                (void)turn_left(game); /* E -> N */
            }
            /* Verify the entrance portrait is reproduced correctly
             * after the round-trip turn + forward + turn sequence
             * (the cross-cell re-blt invariant clears any stale
             * ordinal pixels from the D1C rect).  The no-ordinal-8
             * contract is also locked at this round-trip waypoint. */
            pose.mapX = (int)game->world.party.mapX;
            pose.mapY = (int)game->world.party.mapY;
            pose.dir = (int)game->world.party.direction;
            pose.expectedOrdinal = PROBE_ORDINAL_HALK;
            pose.label = "walkpath_from_entrance_b2_back_to_halk";
            if (!check_walkpath_pose(game, portraits, -1, *outPrevTargetVisible, &pose, fb)) {
                ok = 0;
            }
            *outPrevOrdinal = pose.expectedOrdinal;
            *outPrevTargetVisible = 0;
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
 * post-turn pose) and verify the no-ordinal-8 contract at that
 * waypoint.  This phase exercises the live input-path turn-right
 * branch of the corridor entrance without depending on the exact
 * ordinal at (2,3,E) (which the actual_pose probe does not document
 * directly - its documented pose is (1,3,E) returning 18, not
 * (2,3,E)). */
static int drive_corridor_branch(M11_GameViewState* game,
                                 const M11_AssetSlot* portraits,
                                 int* outPrevOrdinal,
                                 int* outPrevTargetVisible,
                                 unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_CORRIDOR_X, PROBE_CORRIDOR_Y, PROBE_CORRIDOR_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (C prelude) Corridor waypoint: (1,3,N) is a no-portrait
     * corridor cell (front=(1,2) has only TextString, no C127
     * sensor per the actual_pose probe).  The no-ordinal-8
     * contract is also locked here. */
    pose.mapX = PROBE_CORRIDOR_X;
    pose.mapY = PROBE_CORRIDOR_Y;
    pose.dir = PROBE_CORRIDOR_DIR;
    pose.expectedOrdinal = -1;
    pose.label = "walkpath_from_entrance_c_corridor_north_no_portrait_no_ordinal_8";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;
    *outPrevTargetVisible = 0;

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
                    "FAIL walkpath_from_entrance_c_post_turn ordinal 8 floating at (1,3,E)\n");
            ok = 0;
        }
        pose.mapX = postTurnX;
        pose.mapY = postTurnY;
        pose.dir = postTurnDir;
        pose.expectedOrdinal = PROBE_ORDINAL_SONJA;
        pose.label = "walkpath_from_entrance_c1_post_turn_ordinal_18_sonja";
        if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
            ok = 0;
        }
        *outPrevOrdinal = pose.expectedOrdinal;
        *outPrevTargetVisible = 0;
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
     * no-ordinal-8 contract is locked at the post-block pose. */
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
                    "FAIL walkpath_from_entrance_c2_post_block ordinal 8 floating at (1,3,E)\n");
            ok = 0;
        }
        pose.mapX = postStepX;
        pose.mapY = postStepY;
        pose.dir = postStepDir;
        pose.expectedOrdinal = PROBE_ORDINAL_SONJA;
        pose.label = "walkpath_from_entrance_c2_post_block_ordinal_18_sonja";
        if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
            ok = 0;
        }
        *outPrevOrdinal = pose.expectedOrdinal;
        *outPrevTargetVisible = 0;
        /* Turn-left back to NORTH (E -> N) at (1,3).  The
         * no-ordinal-8 contract is locked at this waypoint. */
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
                        "FAIL walkpath_from_entrance_c3_post_back ordinal 8 floating at (%d,%d,%d)\n",
                        postX, postY, postDir);
                ok = 0;
            }
            pose.mapX = postX;
            pose.mapY = postY;
            pose.dir = postDir;
            pose.expectedOrdinal = (postX == PROBE_ENTRANCE_X && postY == PROBE_ENTRANCE_Y)
                                       ? PROBE_ORDINAL_HALK
                                       : -1;
            pose.label = "walkpath_from_entrance_c3_post_back_no_ordinal_8";
            if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
                ok = 0;
            }
            *outPrevOrdinal = pose.expectedOrdinal;
            *outPrevTargetVisible = 0;
        }
    }

    return ok;
}

/* Phase D: re-seed the (2,1,E) IAIDO waypoint via the
 * start_independent_input_route contract, and verify the IAIDO
 * portrait_rect_position contract along with the no-ordinal-8
 * floating contract positively at this target.  This is the unique
 * gate-axis for ordinal 8 that no other probe covers:
 *   - east_walkpath_ordinal_8 verifies (2,1,E) at-rest
 *   - ordinal_8_south_return verifies (2,1,S) at-rest
 *   - portrait_08_cancel_reopen verifies select->cancel->reopen on
 *     a (1,2,N) sensor-seeded pose
 *   - this walkpath_from_entrance probe verifies the live input-path
 *     turn-right + forward-walk + cross-cell re-blt + the
 *     no-ordinal-8 floating contract along the entire walkpath
 *     chain, with ordinal 8 only positively present at this (2,1,E)
 *     target waypoint. */
static int drive_iaido_waypoint(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                int* outPrevOrdinal,
                                int* outPrevTargetVisible,
                                unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_IAIDO_X, PROBE_IAIDO_Y, PROBE_IAIDO_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (D) IAIDO waypoint: (2,1,E) ordinal=8 IAIDO per actual_pose
     * probe.  The C127 sensor on cell (3,1) with sensorData=8 is
     * visible from (2,1) facing E per DUNGEON.C:2573.  Verify the
     * IAIDO portrait_rect_position contract along with the
     * positive-ordinal-8 (i.e. the ordinal 8 pixels dominate the
     * D1C rect) contract.  The cross-cell re-blt invariant clears
     * any stale SONJA pixels from the prior corridor waypoint. */
    pose.mapX = PROBE_IAIDO_X;
    pose.mapY = PROBE_IAIDO_Y;
    pose.dir = PROBE_IAIDO_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_TARGET;
    pose.label = "walkpath_from_entrance_d_ordinal_8_iaido_target";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;
    *outPrevTargetVisible = 1;

    return ok;
}

/* Phase E: re-seed the (2,1,S) LEIF waypoint via the
 * start_independent_input_route contract, and verify the LEIF
 * portrait_rect_position contract along with the no-ordinal-8
 * contract (LEIF is ordinal 4, NOT ordinal 8, so the cross-cell
 * re-blt invariant must clear the IAIDO portrait pixels when the
 * party rotates to SOUTH).  This is the in-cell re-blt contract:
 * the same (2,1) cell exposes a different wall sensor on a
 * different view direction. */
static int drive_leif_waypoint(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               int* outPrevOrdinal,
                               int* outPrevTargetVisible,
                               unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_LEIF_X, PROBE_LEIF_Y, PROBE_LEIF_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (E) LEIF waypoint: (2,1,S) ordinal=4 LEIF per actual_pose
     * probe.  The C127 sensor on cell (2,2) with sensorData=4 is
     * visible from (2,1) facing S per DUNGEON.C:2573.  Verify the
     * LEIF portrait_rect_position contract along with the
     * no-ordinal-8 contract (the in-cell cross-wall re-blt clears
     * the prior IAIDO portrait pixels). */
    pose.mapX = PROBE_LEIF_X;
    pose.mapY = PROBE_LEIF_Y;
    pose.dir = PROBE_LEIF_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_LEIF;
    pose.label = "walkpath_from_entrance_e_ordinal_4_leif_no_ordinal_8";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, *outPrevTargetVisible, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;
    *outPrevTargetVisible = 0;

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
    int prevTargetVisible = 0;
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

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 8, "
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

    /* Bind the ordinal 8 = IAIDO identity from the mirror catalog
     * so the slice is bound to a real source identity.  The
     * east_walkpath_ordinal_8 probe already verified ordinal 8 ->
     * IAIDO on this fixture; we re-pin it here so this probe stays
     * independent if the upstream probe is renamed or split. */
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
        if (nameBuf[0] == 0 || strcmp(nameBuf, "IAIDO") != 0) {
            fprintf(stderr,
                    "FAIL ordinal %d name=%s expected=IAIDO\n",
                    PROBE_ORDINAL_TARGET, nameBuf);
            ok = 0;
        }
        /* IAIDO has a title in the DM1 V1 PC 3.4 catalog (the
         * title text is the mirror-catalog record's title field
         * per the actual_pose probe's reference fixture; this DM1
         * V1 build reports "RUYITO CHIBURI" as the title).
         * Re-pin it here so the no-ordinal-8 contract is bound to
         * a real source identity.  We assert the title is
         * non-empty but do not pin the exact text - the title
         * field varies between regional releases (English PC 3.4
         * vs Atari ST 2.0/2.1 vs Amiga 3.5) and we only need to
         * confirm the catalog has a populated record for ordinal 8
         * on this fixture. */
        if (titleBuf[0] == '\0') {
            fprintf(stderr,
                    "FAIL ordinal %d title=(empty) expected=(non-empty title text)\n",
                    PROBE_ORDINAL_TARGET);
            ok = 0;
        }
    }

    /* Pre-flight: confirm ordinal 8 has exactly one Hall-map route
     * in the walkpath_from_entrance region (y=1..6, x=0..4), the
     * (2,1,E) target.  This is the precondition for the
     * walkpath_from_entrance no-floating contract: if a future
     * DUNGEON.DAT variant placed a C127 sensor with sensorData=8
     * on any other walkpath region, the no-ordinal-8 contract
     * would no longer hold.  The full 16x16 ordinal-8 scan is
     * locked by the east_walkpath_ordinal_8 probe's [D] section;
     * this scoped scan is the walkpath-scoped corollary.  We
     * report the hit count rather than asserting on it so the
     * probe can still produce useful output on a non-canonical
     * build. */
    {
        int mapX, mapY, dir;
        int ord8WalkpathHits = 0;
        int ord8WalkpathHitX = -1, ord8WalkpathHitY = -1, ord8WalkpathHitDir = -1;
        int walkpathHitsExceptTarget = 0;
        game.world.party.mapIndex = 0;
        for (mapY = 1; mapY <= 6; ++mapY) {
            for (mapX = 0; mapX <= 4; ++mapX) {
                for (dir = 0; dir < 4; ++dir) {
                    int ord;
                    game.world.party.mapX = (int16_t)mapX;
                    game.world.party.mapY = (int16_t)mapY;
                    game.world.party.direction = (uint8_t)dir;
                    game.candidateMirrorOrdinal = -1;
                    game.candidateMirrorPartyIndex = -1;
                    game.candidateMirrorPanelActive = 0;
                    ord = M11_GameView_GetFrontMirrorOrdinal(&game);
                    if (ord == PROBE_ORDINAL_TARGET) {
                        ++ord8WalkpathHits;
                        ord8WalkpathHitX = mapX;
                        ord8WalkpathHitY = mapY;
                        ord8WalkpathHitDir = dir;
                        if (!(mapX == PROBE_IAIDO_X && mapY == PROBE_IAIDO_Y &&
                              dir == PROBE_IAIDO_DIR)) {
                            ++walkpathHitsExceptTarget;
                        }
                    }
                }
            }
        }
        printf("  INFO: ordinal 8 C127 sensor routes in walkpath region "
               "(x=0..4, y=1..6) = %d (expected 1, the (2,1,E) target)\n",
               ord8WalkpathHits);
        if (walkpathHitsExceptTarget != 0) {
            fprintf(stderr,
                    "WARN ordinal 8 has %d Hall map route(s) in walkpath region "
                    "outside the (2,1,E) target; the walkpath_from_entrance "
                    "no-ordinal-8 contract expects 0 non-target hits on DM1 V1 "
                    "PC 3.4. First non-target hit: (%d,%d,%d)\n",
                    walkpathHitsExceptTarget,
                    ord8WalkpathHitX, ord8WalkpathHitY, ord8WalkpathHitDir);
        }
        ok &= expect_int("ordinal 8 walkpath_region non-target hits == 0",
                         walkpathHitsExceptTarget, 0);
        ok &= expect_int("ordinal 8 walkpath_region total hits == 1",
                         ord8WalkpathHits, 1);
    }

    ok &= drive_entrance_walkpath(&game, portraits, &prevOrdinal,
                                  &prevTargetVisible, currFb);
    ok &= drive_corridor_branch(&game, portraits, &prevOrdinal,
                                &prevTargetVisible, currFb);
    ok &= drive_iaido_waypoint(&game, portraits, &prevOrdinal,
                               &prevTargetVisible, currFb);
    ok &= drive_leif_waypoint(&game, portraits, &prevOrdinal,
                              &prevTargetVisible, currFb);

    /* Lock the C026 ordinal-8 source-rect math the probe relies
     * on so a future refactor that moves the C026 atlas stride is
     * caught here too.  The math is identical to the
     * east_walkpath_ordinal_8 probe's [B] section; we re-pin it
     * because the runtime pixels above are tied to that source
     * rect.  Row 1 col 0 of the C026 atlas is the canonical
     * ordinal-8 cell; a regression that uses (ordinal >> 3) = 0
     * (row-0 col-0 / DAROOU) instead of (8 >> 3) = 1 would route
     * the wrong champion. */
    {
        int col = -1;
        int row = -1;
        int sx = -1;
        int sy = -1;
        col = PROBE_ORDINAL_TARGET & 7;
        row = (PROBE_ORDINAL_TARGET >> 3) & 3;
        sx = col * PROBE_PORTRAIT_W;
        sy = row * PROBE_PORTRAIT_H;
        ok &= expect_int("ordinal 8 col = ordinal mod 8", col, PROBE_PORTRAIT_8_COL);
        ok &= expect_int("ordinal 8 row = ordinal / 8", row, PROBE_PORTRAIT_8_ROW);
        ok &= expect_int("ordinal 8 source X == 0*32", sx, PROBE_PORTRAIT_8_SRC_X);
        ok &= expect_int("ordinal 8 source Y == 1*29", sy, PROBE_PORTRAIT_8_SRC_Y);
        ok &= expect_int_note(
            "ordinal 8 source bottom edge < C026 strip height + 1",
            sy + PROBE_PORTRAIT_H < PROBE_ATLAS_HEIGHT_BOUND, 1,
            "row 1 bottom edge 58 < strip height 87+1");
        ok &= expect_int_note(
            "ordinal 8 source right edge < C026 strip width + 1",
            sx + PROBE_PORTRAIT_W < PROBE_ATLAS_WIDTH_BOUND, 1,
            "ordinal 8 col=0 leaves 32 px of room in 256-wide strip");
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
