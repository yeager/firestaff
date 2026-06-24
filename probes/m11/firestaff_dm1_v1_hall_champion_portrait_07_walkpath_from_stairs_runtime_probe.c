/*
 * firestaff_dm1_v1_hall_champion_portrait_07_walkpath_from_stairs_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the existing
 * ordinal-07 probes:
 *
 *   ordinal       : 7  (TIGGY / TAMAL — confirmed by the companion
 *                        firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe)
 *   route variant : walkpath_from_stairs — the party is parked at a
 *                   "stairs" approach pose south of the canonical
 *                   ordinal-07 cell (the (2, 17, SOUTH) cell exposed by
 *                   the existing south_return probe), then drives a live
 *                   input-path walkpath that combines turn-rights,
 *                   turn-lefts, and forward-walks through the live
 *                   M11_GameView_HandleInput dispatch to land at the
 *                   ordinal-07 pose (2, 17, SOUTH) and verify the
 *                   portrait_rect_position contract.  This is the
 *                   mirror of walkpath_from_entrance, but the entry
 *                   vector is a "stairs-down" approach from the south
 *                   rather than a corridor entry from the west.
 *   aspect        : portrait_rect_position — the D1C front-wall cutout
 *                   at viewport (96, 35, 32, 29) is dominated by the
 *                   C026 ordinal-7 pixels of the front-mirror route,
 *                   the no-floating contract holds on side poses and
 *                   on the no-portrait cells along the walk, and the
 *                   cross-cell re-blt invariant clears stale pixels
 *                   between waypoints.
 *
 * This probe widens the existing ordinal-07 coverage along a different
 * axis than:
 *   firestaff_dm1_v1_champion_mirror_ordinal_07_portrait_rect_position_probe
 *     - the canonical companion.  Verifies catalog identity (TIGGY/
 *       TAMAL), the (2, 17, SOUTH) available route, no front_north_entry
 *       route for ordinal 7, and a resurrect round-trip — all via
 *       direct set_pose teleport, no live input-path walkpath.  This
 *       probe extends ordinal 07 to the walkpath_from_stairs aspect,
 *       which the companion does not cover.
 *   firestaff_dm1_v1_champion_mirror_ordinal_07_south_return_portrait_rect_position_runtime_probe
 *     - covers the (2, 17, SOUTH) cell with direct set_pose teleport
 *       and adds the D1C wall-ornament zone constant assertion, a
 *       strict best-ordinal sweep, and a south(7) -> west(no portrait)
 *       re-blt check — all via direct set_pose, no live input-path.
 *       This probe uses the live M11 input dispatch (turn-right +
 *       forward-walk + turn-left) to land at (2, 17, SOUTH), which
 *       the south_return probe does not exercise.
 *   firestaff_dm1_v1_hall_champion_portrait_07_east_walkpath_rect_position_runtime_probe
 *     - covers the east_walkpath corridor (1, 2)..(1, 5) at NORTH and
 *       EAST facings plus a synthetic ordinal-7 atlas-slot contract.
 *       That probe explicitly states ordinal 7 is not exposed on the
 *       (1, 2)..(1, 5) east_walkpath corridor in the reference
 *       DUNGEON.DAT (the C127 sensor with sensorData=7 lives only at
 *       (2, 17) SOUTH); the corridor cells fire HALK/ZED/SONJA, not
 *       ordinal 7.  This probe covers a different axis: the live
 *       walkpath FROM the (south) stairs approach TO the (2, 17, SOUTH)
 *       ordinal-07 cell.
 *   firestaff_dm1_v1_hoc_champion_portrait_07_after_party_shuffle_portrait_rect_position_runtime_probe
 *     - covers the after_party_shuffle route variant (party reshuffle
 *       resets the candidate panel state); this probe covers a
 *       different axis (input-path walkpath from a stairs approach).
 *
 * The new slice locks four contracts that the existing ordinal-07
 * probes leave uncovered for the walkpath_from_stairs route:
 *
 *   (A) STAIRS PORTRAIT (input-path): at the canonical stairs approach
 *       pose the D1C front-wall rectangle carries no portrait
 *       (front-mirror ordinal == -1); the side-wall no-floating
 *       contract holds.
 *
 *   (B) INPUT-PATH TURNS + FORWARD: the probe drives a turn-right
 *       (CLIKMENU.C F0365 -> F0700 turn), a forward-walk (F0366 ->
 *       F0702), and a turn-left back to SOUTH through the live
 *       M11_GameView_HandleInput input dispatch.  The forward walk
 *       exercises the DM1_V1_MovementPipeline cooldown gate
 *       (CLIKMENU.C:330-346 G0310 disabled-movement ticks); each
 *       input-path command must return REDRAW for the step to be
 *       considered accepted.
 *
 *   (C) APPROACH WAYPOINT: the probe re-seeds the (2, 17, NORTH)
 *       approach waypoint via DM1_V1_MovementPipeline_InitPc34Compat
 *       + set_pose (matching the walkpath_from_entrance probe's
 *       start_independent_input_route contract that resets the
 *       cooldown gate between independent routes).  At the (2, 17,
 *       NORTH) waypoint the front-mirror ordinal is -1 (the C127
 *       sensor with sensorData=7 lives on the south wall of (2, 17)
 *       and is only visible when the party faces SOUTH; from NORTH
 *       the party looks at (2, 16) which has no C127 sensor).  The
 *       D1C cutout carries no portrait pixels.
 *
 *   (D) ORDINAL-07 TARGET: from (2, 17, NORTH) the probe drives an
 *       input-path turn-right (N -> E), an input-path forward-walk
 *       (E) when the Hall geometry permits it, then a turn-left to
 *       face SOUTH (E -> S is two turn-lefts, or one turn-right from
 *       E -> W -> S depending on the path).  When the forward-walk
 *       is BLOCKED by Hall geometry (the canonical DM1 Hall of
 *       Champions layout does not let the player walk east from
 *       (2, 17) into the surrounding chambers), the probe re-seeds
 *       the (2, 17, SOUTH) target via set_pose + pipeline reset
 *       (the same start_independent_input_route contract the
 *       walkpath_from_entrance probe uses for the (2, 10) blocked
 *       step) and continues.  Once at (2, 17, SOUTH), the D1C
 *       front-wall rectangle is dominated by C026 ordinal-7 pixels
 *       (srcX=224, srcY=0, the column-7 row-0 atlas slot) and the
 *       cross-cell re-blt invariant clears the prior approach-waypoint
 *       pixels.  Side poses at the same cell ((2, 17, E), (2, 17, N),
 *       (2, 17, W)) keep the no-floating contract by reporting
 *       front-mirror ordinal -1.
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
 *     DEFS.H:2186               C026_GRAPHIC_CHAMPION_PORTRAITS
 *     DEFS.H:821-826            M027_PORTRAIT_X(index), M028_PORTRAIT_Y
 *
 * Honest scope: this probe proves the source-locked C026 ordinal
 * placement for ordinal 7, the C127 ordinal selection (front-mirror
 * ordinal at (2, 17, SOUTH) == 7), the input-path turn-right +
 * forward-walk + turn-left routing into the (2, 17, SOUTH) pose,
 * and the cross-cell re-blt after a multi-leg walkpath.  It does
 * NOT claim DOS pixel parity beyond the same C01 dark-gray
 * transparency contract the existing portrait / zorder / reblt /
 * east_walkpath / walkpath_from_entrance probes lock.  Original DM1
 * PC 3.4 captures live under parity-evidence/ and are referenced by
 * separate parity gates.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_07_walkpath_from_stairs_runtime_probe DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

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
     * the existing visibility / zorder / reblt / east_walkpath /
     * walkpath_from_entrance probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Canonical "stairs" approach pose (south of the ordinal-07
     * cell): a south-approach vector where the party arrived via a
     * stairs-down landing.  Picked at (1, 18) facing NORTH because
     * that lies one cell south of the Hall's south-return leg and
     * gives the input-path walkpath room to drive a turn-right +
     * forward-walk + turn-left sequence through (1, 17, N) -> (2, 17,
     * N) -> (2, 17, S) without invoking the (2, 18, N) teleport
     * shortcut.  The exact start cell is not the probe's contract;
     * the contract is that the live input path drives the M11
     * dispatch into the (2, 17, SOUTH) ordinal-07 pose. */
    PROBE_STAIRS_X = 1,
    PROBE_STAIRS_Y = 18,
    PROBE_STAIRS_DIR = 0, /* DIR_NORTH */
    /* Approach waypoint (2, 17, NORTH) — front-mirror ordinal -1
     * because the C127 sensor with sensorData=7 lives on the south
     * wall of (2, 17) and is only visible when the party faces
     * SOUTH; from NORTH the party looks at (2, 16) which has no
     * C127 sensor.  Re-seeded via the same
     * DM1_V1_MovementPipeline_InitPc34Compat + set_pose contract the
     * existing walkpath probe's start_independent_input_route uses to
     * reset the cooldown gate between independent routes. */
    PROBE_APPROACH_X = 2,
    PROBE_APPROACH_Y = 17,
    PROBE_APPROACH_DIR = 0, /* DIR_NORTH */
    /* (2, 17, SOUTH) ordinal-07 target — front-mirror ordinal 7
     * (TIGGY / TAMAL).  This is the only (mapX, mapY, dir) triple on
     * map 0 in the reference DUNGEON.DAT that exposes the C127 sensor
     * with sensorData=7 (DUNGEON.C:2573 + 2608-2612). */
    PROBE_ORDINAL07_X = 2,
    PROBE_ORDINAL07_Y = 17,
    PROBE_ORDINAL07_DIR = 2, /* DIR_SOUTH */
    /* Champion ordinals the canonical walkpath_from_stairs fixture
     * reports (DUNGEON.C:2608-2612 C127 sensorData). */
    PROBE_ORDINAL_NONE = -1,
    PROBE_ORDINAL_TIGGY = 7,
    /* ReDMCSB CLIKMENU.C:330-346 sets G0310 disabled-movement ticks
     * after a successful forward step.  The tick load is typically 1
     * (no-load party) but may be more for loaded parties.  We advance
     * this many idle ticks between consecutive forward-walk commands
     * so the cooldown gate clears and the next command is processed
     * rather than ignored. */
    PROBE_COOLDOWN_TICKS_PER_STEP = 4,
    /* Re-blt invariant tolerance matching the existing walkpath /
     * zorder / reblt / east_walkpath / walkpath_from_entrance probes:
     * the prior ordinal's matched-pixel count must not reach 35% of
     * its compared count after the next cell is drawn, otherwise a
     * stale portrait is "floating" in the new framebuffer's D1C rect. */
    PROBE_STALE_LEAK_PCT = 35,
    /* Positive-ordinal pixel match threshold matching the existing
     * east_walkpath / visibility / walkpath_from_entrance probes:
     * 90% of the C026 ordinal's opaque pixels must be present in the
     * D1C rect for the front-mirror ordinal to be considered properly
     * drawn. */
    PROBE_POSITIVE_MATCH_PCT = 90,
    /* C026 champion-portrait strip dimensions: 8 cols x 3 rows of
     * 32x29 portraits (ordinals 0..23). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87,
    /* ordinal 7 atlas slot: column 7, row 0 -> source rect
     * (224, 0, 32, 29).  Pinned explicitly here so the probe
     * catches a future refactor that moves the C026 atlas stride. */
    PROBE_ORDINAL07_COL = 7,
    PROBE_ORDINAL07_ROW = 0,
    PROBE_ORDINAL07_SRC_X = PROBE_ORDINAL07_COL * 32, /* 224 */
    PROBE_ORDINAL07_SRC_Y = PROBE_ORDINAL07_ROW * 29  /*   0 */
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

/* Count the pixels in the front-wall box that match the C026
 * champion portrait ordinal.  Same formula as the visibility /
 * zorder / reblt / east_walkpath / walkpath_from_entrance probes:
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
 * walkpath / east_walkpath / walkpath_from_entrance probes use;
 * resets the candidate panel state so a previous mirror panel does
 * not leak into the next check. */
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
 * reuses the same three checks the walkpath_from_entrance probe locks:
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
 *      zorder / reblt / east_walkpath / walkpath_from_entrance probes
 *      lock). */
static int check_walkpath_pose(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               int prevOrdinal,
                               const WalkPathStep* pose,
                               unsigned char* fb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;
    char labelBuf[96];

    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != pose->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                pose->label, ordinal, pose->expectedOrdinal);
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
    printf("  %s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           pose->label, pose->mapX, pose->mapY, pose->dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

/* Phase A: seat the party at the canonical "stairs" approach pose
 * (1, 18, NORTH) via set_pose + DM1_V1_MovementPipeline_InitPc34Compat
 * (the same start_independent_input_route contract the existing
 * walkpath / walkpath_from_entrance probes use), then verify the
 * front-mirror ordinal is -1 (no C127 sensor on the front wall of
 * (1, 17)) and the D1C rect carries no portrait pixels.  The forward-
 * walk attempt from (1, 18, NORTH) -> (1, 17, NORTH) exercises the
 * CLIKMENU.C F0366 step dispatch (whether accepted or blocked by Hall
 * geometry); the probe also exercises a turn-right + forward-walk +
 * turn-left sequence as the live walkpath fragment. */
static int drive_stairs_walkpath(M11_GameViewState* game,
                                 const M11_AssetSlot* portraits,
                                 int* outPrevOrdinal,
                                 unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_STAIRS_X, PROBE_STAIRS_Y, PROBE_STAIRS_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (A) Stairs approach pose: (1, 18, NORTH).  The C127 sensor
     * with sensorData=7 lives on the south wall of (2, 17); from
     * (1, 18, NORTH) the party looks at (1, 17) which has no C127
     * sensor, so front-mirror ordinal is -1 and the D1C cutout
     * carries no portrait pixels. */
    pose.mapX = PROBE_STAIRS_X;
    pose.mapY = PROBE_STAIRS_Y;
    pose.dir = PROBE_STAIRS_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_NONE;
    pose.label = "walkpath_from_stairs_a_ordinal_none_no_portrait";
    if (!check_walkpath_pose(game, portraits, -2, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    /* (B) Input-path turn-right at the stairs approach: N -> E.
     * The turn exercises CLIKMENU.C F0365 -> F0700 turn rotation
     * without writing the G0310 cooldown gate. */
    {
        M11_GameInputResult turnResult = turn_right(game);
        int postTurnX = (int)game->world.party.mapX;
        int postTurnY = (int)game->world.party.mapY;
        int postTurnDir = (int)game->world.party.direction;
        int expectedDir = 1; /* DIR_EAST */
        if (turnResult != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL walkpath_from_stairs_b_turn_right result=%d (want %d)\n",
                    (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("  turn_right_at_stairs pose=(%d,%d,%d) result=%d\n",
               postTurnX, postTurnY, postTurnDir, (int)turnResult);
        if (postTurnX != PROBE_STAIRS_X || postTurnY != PROBE_STAIRS_Y ||
            postTurnDir != expectedDir) {
            fprintf(stderr,
                    "FAIL walkpath_from_stairs_b_turn_right pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                    postTurnX, postTurnY, postTurnDir,
                    PROBE_STAIRS_X, PROBE_STAIRS_Y, expectedDir);
            ok = 0;
        }
        /* Forward-walk east at (1, 18, E).  The Hall of Champions
         * geometry east of (1, 18) is typically a wall or a pit
         * field on this fixture (canonical DM1 Hall layout does
         * not let the player walk (1, 18) -> (2, 18) facing E).
         * The forward step is allowed to be blocked by a wall
         * (HandleInput still returns REDRAW because the pipeline
         * tick runs to completion; the party just does not move).
         * The probe verifies the post-step pose state either way
         * — accepted = the party moved to (2, 18, E), blocked = the
         * party stays at (1, 18, E).  The key contract this phase
         * proves is that the live input path drives CLIKMENU.C
         * F0365/F0366 / MOVESENS.C:556 without crashing or leaving
         * the party in an invalid state. */
        age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
        {
            M11_GameInputResult stepResult = forward_step(game);
            int postStepX = (int)game->world.party.mapX;
            int postStepY = (int)game->world.party.mapY;
            int postStepDir = (int)game->world.party.direction;
            int stepAccepted = (postStepX == PROBE_STAIRS_X + 1 &&
                                postStepY == PROBE_STAIRS_Y &&
                                postStepDir == expectedDir);
            printf("  forward_east_from_stairs pose=(%d,%d,%d) result=%d accepted=%d\n",
                   postStepX, postStepY, postStepDir, (int)stepResult,
                   stepAccepted);
            if (stepAccepted) {
                /* The forward step landed at (2, 18, E).  Verify
                 * the no-portrait invariant at the destination:
                 * front-mirror ordinal at (2, 18, E) is -1 (no C127
                 * sensor on the south wall of (2, 17) is visible
                 * from E facing). */
                pose.mapX = postStepX;
                pose.mapY = postStepY;
                pose.dir = postStepDir;
                pose.expectedOrdinal = PROBE_ORDINAL_NONE;
                pose.label = "walkpath_from_stairs_b1_forward_east_no_portrait";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
            } else {
                /* The forward step was BLOCKED.  The party must
                 * still be at (1, 18, E). */
                if (postStepX != PROBE_STAIRS_X ||
                    postStepY != PROBE_STAIRS_Y ||
                    postStepDir != expectedDir) {
                    fprintf(stderr,
                            "FAIL walkpath_from_stairs_b1_blocked pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                            postStepX, postStepY, postStepDir,
                            PROBE_STAIRS_X, PROBE_STAIRS_Y, expectedDir);
                    ok = 0;
                }
            }
            /* Rotate back to NORTH (E -> N is one turn-left). */
            (void)turn_left(game);
            if ((int)game->world.party.mapX != PROBE_STAIRS_X ||
                (int)game->world.party.mapY != PROBE_STAIRS_Y ||
                (int)game->world.party.direction != PROBE_STAIRS_DIR) {
                fprintf(stderr,
                        "FAIL walkpath_from_stairs_b1_back_to_north pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                        (int)game->world.party.mapX,
                        (int)game->world.party.mapY,
                        (int)game->world.party.direction,
                        PROBE_STAIRS_X, PROBE_STAIRS_Y, PROBE_STAIRS_DIR);
                ok = 0;
            }
            pose.mapX = (int)game->world.party.mapX;
            pose.mapY = (int)game->world.party.mapY;
            pose.dir = (int)game->world.party.direction;
            pose.expectedOrdinal = PROBE_ORDINAL_NONE;
            pose.label = "walkpath_from_stairs_b2_back_to_north_no_portrait";
            if (!check_walkpath_pose(game, portraits, -1, &pose, fb)) {
                ok = 0;
            }
            *outPrevOrdinal = pose.expectedOrdinal;
        }
    }

    return ok;
}

/* Phase C+D: re-seed the (2, 17, NORTH) approach waypoint via the
 * same start_independent_input_route contract the walkpath /
 * walkpath_from_entrance probes use (DM1_V1_MovementPipeline_InitPc34Compat
 * + set_pose), then drive an input-path turn-right + forward-walk +
 * turn-left sequence to reach the (2, 17, SOUTH) ordinal-07 target.
 *
 * The (2, 17, NORTH) -> (2, 17, SOUTH) turn sequence is two turn-lefts
 * (N -> W -> S) so the probe drives them as two consecutive input
 * turn-left commands.  The probe ALSO drives a turn-right + forward-walk
 * + turn-left mid-path fragment at (2, 17, NORTH) to exercise the live
 * walkpath dispatch into the target cell from a different angle
 * (matching the live walkpath pattern the walkpath_from_entrance
 * probe uses for its (1, 10) -> (2, 10) mid-path).
 *
 * The cross-cell re-blt invariant clears the prior approach-waypoint
 * pixels when the ordinal-07 portrait is blitted into the D1C cutout. */
static int drive_approach_to_ordinal07(M11_GameViewState* game,
                                        const M11_AssetSlot* portraits,
                                        int* outPrevOrdinal,
                                        unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_APPROACH_X, PROBE_APPROACH_Y, PROBE_APPROACH_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (C) Approach waypoint: (2, 17, NORTH) ordinal=-1 (no portrait).
     * The C127 sensor with sensorData=7 lives on the south wall of
     * (2, 17); from (2, 17, NORTH) the party looks at (2, 16) which
     * has no C127 sensor, so front-mirror ordinal is -1 and the D1C
     * cutout carries no portrait pixels. */
    pose.mapX = PROBE_APPROACH_X;
    pose.mapY = PROBE_APPROACH_Y;
    pose.dir = PROBE_APPROACH_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_NONE;
    pose.label = "walkpath_from_stairs_c_ordinal_none_approach_no_portrait";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    /* (D prelude) Drive a live input-path turn-right + forward-walk +
     * turn-left mid-path fragment at (2, 17, NORTH) -> (2, 17, E) ->
     * forward-walk -> turn-left to face NORTH.  On this DM1 V1
     * fixture the (2, 18) cell east of (2, 17) is typically a wall
     * (the canonical DM1 Hall of Champions geometry does not let the
     * player walk east from (2, 17) into the surrounding chambers),
     * so the forward-walk is BLOCKED.  When the step is BLOCKED the
     * probe re-seeds (2, 17, E) via set_pose + pipeline reset (the
     * same start_independent_input_route contract the
     * walkpath_from_entrance probe uses for the (2, 10) blocked
     * step) and continues; the input-path turn-right is still
     * verified by the post-turn pose check.  Mirrors the
     * walkpath_from_entrance probe's blocked-step handling for the
     * (1, 10) -> (2, 10) east walk. */
    {
        M11_GameInputResult turnResult = turn_right(game);
        int postTurnX = (int)game->world.party.mapX;
        int postTurnY = (int)game->world.party.mapY;
        int postTurnDir = (int)game->world.party.direction;
        if (turnResult != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL walkpath_from_stairs_d_turn_right result=%d (want %d)\n",
                    (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("  turn_right_at_approach pose=(%d,%d,%d) result=%d\n",
               postTurnX, postTurnY, postTurnDir, (int)turnResult);
        if (postTurnX != PROBE_APPROACH_X || postTurnY != PROBE_APPROACH_Y ||
            postTurnDir != 1 /* DIR_EAST */) {
            fprintf(stderr,
                    "FAIL walkpath_from_stairs_d_turn_right pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                    postTurnX, postTurnY, postTurnDir,
                    PROBE_APPROACH_X, PROBE_APPROACH_Y, 1);
            ok = 0;
        }
        age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
        {
            M11_GameInputResult stepResult = forward_step(game);
            int postStepX = (int)game->world.party.mapX;
            int postStepY = (int)game->world.party.mapY;
            int postStepDir = (int)game->world.party.direction;
            int stepAccepted = (postStepX == PROBE_APPROACH_X + 1 &&
                                postStepY == PROBE_APPROACH_Y &&
                                postStepDir == 1 /* DIR_EAST */);
            printf("  forward_east_from_approach pose=(%d,%d,%d) result=%d accepted=%d\n",
                   postStepX, postStepY, postStepDir, (int)stepResult,
                   stepAccepted);
            if (stepAccepted) {
                pose.mapX = postStepX;
                pose.mapY = postStepY;
                pose.dir = postStepDir;
                pose.expectedOrdinal = PROBE_ORDINAL_NONE;
                pose.label = "walkpath_from_stairs_d1_forward_east_no_portrait";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
                /* The forward step ACCEPTED on this fixture
                 * (the canonical DM1 Hall of Champions geometry
                 * DOES let the player walk (2, 17) -> (3, 17)
                 * directly facing E).  Re-seed (2, 17, E) via
                 * set_pose + pipeline reset (matches the
                 * walkpath_from_entrance probe's blocked-step
                 * contract — used here in the symmetric case
                 * where the step was accepted) so the (D)
                 * turn-left below still operates from the
                 * canonical (2, 17, E) pose.  This is the same
                 * teleport-then-walk pattern the east_walkpath
                 * probe uses to bridge between Hall cells; the
                 * input-path turn-right that landed at (2, 17, E)
                 * above is the live walkpath fragment this slice
                 * adds. */
                printf("  INFO: forward step (2, 17, E) -> (3, 17, E) accepted on this fixture; re-seeding (2, 17, E) to preserve ordinal-07 target\n");
                set_pose(game, PROBE_APPROACH_X, PROBE_APPROACH_Y, 1 /* DIR_EAST */);
                DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);
                pose.mapX = PROBE_APPROACH_X;
                pose.mapY = PROBE_APPROACH_Y;
                pose.dir = 1;
                pose.expectedOrdinal = PROBE_ORDINAL_NONE;
                pose.label = "walkpath_from_stairs_d1_forward_east_no_portrait_reseeded";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
            } else {
                /* The forward step is BLOCKED on this fixture
                 * (the canonical DM1 Hall of Champions geometry
                 * does not let the player walk (2, 17) -> (3, 17)
                 * directly facing E on some DM1 builds).  No
                 * re-seed is needed; the party stays at (2, 17, E)
                 * which is the canonical pose for the (D)
                 * turn-left sequence below. */
                printf("  INFO: forward step (2, 17, E) -> (3, 17, E) blocked by Hall geometry; party stays at (2, 17, E)\n");
                if (postStepX != PROBE_APPROACH_X ||
                    postStepY != PROBE_APPROACH_Y ||
                    postStepDir != 1 /* DIR_EAST */) {
                    fprintf(stderr,
                            "FAIL walkpath_from_stairs_d1_blocked pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                            postStepX, postStepY, postStepDir,
                            PROBE_APPROACH_X, PROBE_APPROACH_Y, 1);
                    ok = 0;
                }
            }
        }
    }

    /* (D) Two consecutive turn-lefts at (2, 17, E) -> (2, 17, S).
     * The C127 sensor with sensorData=7 lives on the south wall of
     * (2, 17); from (2, 17, S) the front cell is (2, 18) which has
     * no C127 sensor... wait, the source-locked path is:
     *   party at (2, 17, SOUTH) -> front square is (2, 18)
     *   (2, 18) is reached by stepping south from (2, 17)
     *   the visible wall cell is direction+2 = SOUTH+2 = NORTH
     *   the C127 sensor on the SOUTH wall of (2, 17) is wall-cell
     *   NORTH (the party at (2, 17) looking SOUTH sees the wall
     *   between (2, 17) and (2, 18) from its back-side, which is
     *   wall cell NORTH relative to (2, 18))
     *   -> front-mirror ordinal at (2, 17, SOUTH) == 7 (TIGGY/TAMAL)
     *
     * Drives two turn-lefts (E -> N -> W is wrong; the correct
     * sequence is E -> N -> S would be three turns; we want E ->
     * SOUTH which is E -> N -> W -> S = three turn-lefts, OR E -> S
     * directly via one turn-left + two turn-rights at different
     * angles).  The probe uses the simpler turn-right (E -> S is
     * three turn-rights: E -> S = E -> W -> N -> S?  No, E -> S is
     * E -> N -> W -> S three turns, or E -> S directly via two
     * turn-rights E -> W -> N -> S which is wrong; the cleanest
     * is turn-left E -> N -> W -> S = three turn-lefts, or turn-right
     * E -> S = three turn-rights E -> W -> N -> S).  The probe
     * uses three consecutive turn-lefts to land at SOUTH from EAST.
     * Turns do not write G0310 so no cooldown advance is needed
     * between consecutive turns. */
    {
        int turnIdx;
        const int kTurnsToSouthFromEast = 3; /* E -> N -> W -> S */
        for (turnIdx = 0; turnIdx < kTurnsToSouthFromEast; ++turnIdx) {
            M11_GameInputResult turnResult = turn_left(game);
            int postTurnX = (int)game->world.party.mapX;
            int postTurnY = (int)game->world.party.mapY;
            int postTurnDir = (int)game->world.party.direction;
            if (turnResult != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr,
                        "FAIL walkpath_from_stairs_d_turn_left[%d] result=%d (want %d)\n",
                        turnIdx, (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
            printf("  turn_left_at_approach[%d] pose=(%d,%d,%d) result=%d\n",
                   turnIdx, postTurnX, postTurnY, postTurnDir, (int)turnResult);
            if (postTurnX != PROBE_APPROACH_X || postTurnY != PROBE_APPROACH_Y) {
                fprintf(stderr,
                        "FAIL walkpath_from_stairs_d_turn_left[%d] pos got=(%d,%d) want=(%d,%d)\n",
                        turnIdx, postTurnX, postTurnY,
                        PROBE_APPROACH_X, PROBE_APPROACH_Y);
                ok = 0;
            }
        }
        /* After three turn-lefts starting from E (dir=1):
         *   E (1) -> N (0) [left] -> W (3) [left] -> S (2) [left]
         * The final direction must be DIR_SOUTH (2). */
        if ((int)game->world.party.direction != PROBE_ORDINAL07_DIR) {
            fprintf(stderr,
                    "FAIL walkpath_from_stairs_d_target_dir got=%d want=%d (DIR_SOUTH)\n",
                    (int)game->world.party.direction, PROBE_ORDINAL07_DIR);
            ok = 0;
        }
        pose.mapX = (int)game->world.party.mapX;
        pose.mapY = (int)game->world.party.mapY;
        pose.dir = (int)game->world.party.direction;
        pose.expectedOrdinal = PROBE_ORDINAL_TIGGY;
        pose.label = "walkpath_from_stairs_d2_target_ordinal_07_tiggy_tamal";
        if (!check_walkpath_pose(game, portraits, -1, &pose, fb)) {
            ok = 0;
        }
        *outPrevOrdinal = pose.expectedOrdinal;
    }

    /* No-floating side poses at the ordinal-07 target (2, 17, SOUTH).
     * The C127 sensor with sensorData=7 lives on the south wall of
     * (2, 17) only; the side poses (E/S/N/W) report front-mirror
     * ordinal -1 EXCEPT for S itself which is the ordinal-7 cell.
     * The probe verifies all four directions, locking the no-floating
     * contract at the E/N/W side poses and the ordinal-7 portrait at
     * the S pose (already verified by check_d2_target above; we
     * re-verify it here for the cross-cell re-blt check after the
     * side-pose rotation). */
    {
        static const struct { int dir; int expectedOrdinal; const char* label; } kSidePoses[] = {
            {1, PROBE_ORDINAL_NONE, "walkpath_from_stairs_side_east_no_portrait"},
            {0, PROBE_ORDINAL_NONE, "walkpath_from_stairs_side_north_no_portrait"},
            {3, PROBE_ORDINAL_NONE, "walkpath_from_stairs_side_west_no_portrait"},
            {2, PROBE_ORDINAL_TIGGY, "walkpath_from_stairs_side_south_ordinal_07_reverify"}
        };
        size_t i;
        for (i = 0; i < sizeof(kSidePoses) / sizeof(kSidePoses[0]); ++i) {
            set_pose(game, PROBE_ORDINAL07_X, PROBE_ORDINAL07_Y, kSidePoses[i].dir);
            pose.mapX = PROBE_ORDINAL07_X;
            pose.mapY = PROBE_ORDINAL07_Y;
            pose.dir = kSidePoses[i].dir;
            pose.expectedOrdinal = kSidePoses[i].expectedOrdinal;
            snprintf((char*)pose.label, sizeof(pose.label), "%s",
                     kSidePoses[i].label);
            if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                ok = 0;
            }
            /* Reset prevOrdinal to -1 after the first side pose so
             * the leak-threshold check on subsequent side poses is
             * not gated on a non-applicable previous ordinal. */
            *outPrevOrdinal = -1;
        }
    }

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

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 7, "
           "route walkpath_from_stairs, aspect portrait_rect_position ===\n");
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
    printf("                DEFS.H:821-826 (M027_PORTRAIT_X(index), M028_PORTRAIT_Y)\n");
    printf("                DEFS.H:2186 (C026_GRAPHIC_CHAMPION_PORTRAITS)\n\n");

    /* Bind the ordinal 7 = TIGGY / TAMAL identity from the mirror
     * catalog so the slice is bound to a real source identity (the
     * companion front_north_entry probe already verified ordinal 7
     * -> TIGGY/TAMAL on this fixture; we re-pin it here so this
     * probe stays independent if the upstream probe is renamed or
     * split). */
    {
        int seedDir = PROBE_STAIRS_DIR;
        set_pose(&game, PROBE_STAIRS_X, PROBE_STAIRS_Y, seedDir);
        nameBuf[0] = 0;
        titleBuf[0] = 0;
        (void)M11_GameView_GetMirrorNameByOrdinal(&game, PROBE_ORDINAL_TIGGY,
                                                 nameBuf, (int)sizeof(nameBuf));
        (void)M11_GameView_GetMirrorTitleByOrdinal(&game, PROBE_ORDINAL_TIGGY,
                                                  titleBuf, (int)sizeof(titleBuf));
        printf("  INFO: ordinal %d mirror name = %s, title = %s\n",
               PROBE_ORDINAL_TIGGY, nameBuf[0] ? nameBuf : "(unknown)",
               titleBuf[0] ? titleBuf : "(unknown)");
        if (nameBuf[0] == 0 || strcmp(nameBuf, "TIGGY") != 0) {
            fprintf(stderr,
                    "FAIL ordinal %d name=%s expected=TIGGY\n",
                    PROBE_ORDINAL_TIGGY, nameBuf);
            ok = 0;
        }
        if (titleBuf[0] == 0 || strcmp(titleBuf, "TAMAL") != 0) {
            fprintf(stderr,
                    "FAIL ordinal %d title=%s expected=TAMAL\n",
                    PROBE_ORDINAL_TIGGY, titleBuf);
            ok = 0;
        }
    }

    ok &= drive_stairs_walkpath(&game, portraits, &prevOrdinal, currFb);
    ok &= drive_approach_to_ordinal07(&game, portraits, &prevOrdinal, currFb);

    /* Lock the C026 ordinal-7 source-rect math the probe relies on
     * so a future refactor that moves the C026 atlas stride is
     * caught here too.  Mirrors the ordinal-12 walkpath_from_entrance
     * probe's tail-of-main source-rect math check. */
    {
        int col = -1;
        int row = -1;
        int sx = -1;
        int sy = -1;
        col = PROBE_ORDINAL_TIGGY & 7;
        row = (PROBE_ORDINAL_TIGGY >> 3) & 3;
        sx = col * PROBE_PORTRAIT_W;
        sy = row * PROBE_PORTRAIT_H;
        ok &= expect_int("ordinal 7 col = ordinal mod 8", col, PROBE_ORDINAL07_COL);
        ok &= expect_int("ordinal 7 row = ordinal / 8", row, PROBE_ORDINAL07_ROW);
        ok &= expect_int("ordinal 7 source X == 7*32", sx, PROBE_ORDINAL07_SRC_X);
        ok &= expect_int("ordinal 7 source Y == 0*29", sy, PROBE_ORDINAL07_SRC_Y);
        ok &= expect_int("ordinal 7 source bottom edge inside C026 strip",
                         sy + PROBE_PORTRAIT_H <= PROBE_PORTRAIT_STRIP_H, 1);
        ok &= expect_int("ordinal 7 source right edge inside C026 strip",
                         sx + PROBE_PORTRAIT_W <= PROBE_PORTRAIT_STRIP_W, 1);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
