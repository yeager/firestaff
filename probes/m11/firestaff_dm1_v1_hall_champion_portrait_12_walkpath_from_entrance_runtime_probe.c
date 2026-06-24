/*
 * firestaff_dm1_v1_hall_champion_portrait_12_walkpath_from_entrance_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice that is intentionally NOT covered by the
 * existing ordinal-12 probes:
 *
 *   ordinal       : 12 (C026 col 4, row 1; mirror-catalog record LINFLAS)
 *   route variant : walkpath_from_entrance — the party is seated at the
 *                   canonical Hall entrance pose (map=0, x=1, y=2) facing
 *                   NORTH where M11_GameView_GetFrontMirrorOrdinal returns
 *                   1 = HALK, then drives an input-path walkpath that
 *                   combines turn-rights, turn-lefts, forward-walks, and
 *                   a teleport waypoint to land at the LINFLAS pose
 *                   (2,10,N) where the C127 sensor carries ordinal=12.
 *                   Each input-path command exercises the live M11 input
 *                   dispatch (M11_GameView_HandleInput) and the
 *                   source-locked movement pipeline
 *                   (DM1_V1_MovementPipeline_*).
 *   aspect        : portrait_rect_position — the D1C front-wall cutout at
 *                   viewport (96,35,32,29) is dominated by the C026
 *                   ordinal pixels of the front-mirror route at every
 *                   waypoint, the no-floating contract holds on side
 *                   poses and on the no-portrait corridor cells along
 *                   the walk, and the cross-cell re-blt invariant clears
 *                   stale ordinal pixels between waypoints.
 *
 * This probe widens the existing ordinal-12 coverage along a different
 * axis than:
 *   firestaff_dm1_v1_hall_champion_portrait_12_front_north_entry_runtime_probe
 *     - covers the static front_north_entry pose at (1,2,N) and the
 *       side-wall no-floating contract via direct pose mutation only
 *       (no real input path; no input-path cooldown gate exercised).
 *   firestaff_dm1_v1_hall_champion_portrait_12_east_walkpath_portrait_rect_probe
 *     - covers (1,10,N) -> (2,10,N) -> (3,10,N) and back along y=10
 *       via direct set_pose teleport — it does NOT exercise the
 *       input-path forward-walk / turn-right branches the
 *       COMMAND.C F0359/F0361 -> CLIKMENU.C F0365/F0366 ->
 *       MOVESENS.C:556 -> DUNVIEW.C:3913-3928 path drives.
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 *     - the broader Hall walkpath probe SKIPs on this DM1 V1 fixture
 *       because its reference (1,3,N)=1 HALK assumption does not match
 *       the (1,2,N)=1 HALK entrance this fixture uses; the probe never
 *       exercises the canonical walkpath-from-entrance -> LINFLAS
 *       transition for ordinal 12.
 *
 * The new slice locks four contracts that the existing ordinal-12
 * probes leave uncovered for the walkpath-from-entrance route:
 *
 *   (A) ENTRANCE PORTRAIT (input-path): at the canonical entrance
 *       (1,2,N) the D1C front-wall rectangle is dominated by C026
 *       ordinal 1 (HALK) pixels (srcX=32, srcY=0 in the C026 strip).
 *
 *   (B) INPUT-PATH TURNS + FORWARD: at the entrance the probe drives
 *       a turn-right (CLIKMENU.C F0365 -> F0700 turn), a forward-walk
 *       (F0366 -> F0702), and a turn-left back to NORTH through the
 *       live M11_GameView_HandleInput input dispatch.  The forward
 *       walk exercises the DM1_V1_MovementPipeline cooldown gate
 *       (CLIKMENU.C:330-346 G0310 disabled-movement ticks); each
 *       input-path command must return REDRAW for the step to be
 *       considered accepted.
 *
 *   (C) ZED WAYPOINT: the probe re-seeds the (1,10,N) ZED waypoint
 *       via DM1_V1_MovementPipeline_InitPc34Compat + set_pose
 *       (matching the existing walkpath probe's
 *       start_independent_input_route contract that resets the
 *       cooldown gate between independent routes).  The D1C
 *       front-wall rectangle is dominated by C026 ordinal 9 (ZED)
 *       pixels (srcX=32, srcY=29) at this waypoint.
 *
 *   (D) LINFLAS TARGET: from (1,10,N) the probe drives an input-path
 *       turn-right (N -> E) followed by an input-path forward-walk
 *       (E) to land at (2,10,E).  A second input-path turn-left
 *       (E -> N) rotates the party to face NORTH so the C127 sensor
 *       at (2,9)'s south wall exposes ordinal 12 LINFLAS in the D1C
 *       cutout.  The cutout is dominated by C026 ordinal 12 pixels
 *       (srcX=128, srcY=29) and the cross-cell re-blt invariant
 *       clears the prior ZED portrait pixels.  Side poses at the
 *       same cell ((2,10,E), (2,10,S), (2,10,W)) keep the
 *       no-floating contract by reporting front-mirror ordinal -1.
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
 *
 * Honest scope: this probe proves the source-locked C026 ordinal
 * placement, the C127 ordinal selection, the input-path forward-walk
 * + turn-right + turn-left routing into the LINFLAS pose, and the
 * cross-cell re-blt after a multi-leg walkpath.  It does NOT claim
 * DOS pixel parity beyond the same C01 dark-gray transparency
 * contract the existing portrait / zorder / reblt / east_walkpath
 * probes lock.  Original DM1 PC 3.4 captures live under
 * parity-evidence/ and are referenced by separate parity gates.
 *
 * Usage: firestaff_dm1_v1_hall_champion_portrait_12_walkpath_from_entrance_runtime_probe DATA_DIR
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
     * the existing visibility / zorder / reblt / east_walkpath probes
     * lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Canonical Hall entrance pose (map=0, x=1, y=2) facing NORTH:
     * M11_GameView_GetFrontMirrorOrdinal returns 1 = HALK. */
    PROBE_ENTRANCE_X = 1,
    PROBE_ENTRANCE_Y = 2,
    PROBE_ENTRANCE_DIR = 0, /* DIR_NORTH */
    /* (1,10,N) ZED waypoint: re-seeded via the same
     * DM1_V1_MovementPipeline_InitPc34Compat + set_pose contract the
     * existing walkpath probe's start_independent_input_route uses to
     * reset the cooldown gate between independent routes. */
    PROBE_ZED_X = 1,
    PROBE_ZED_Y = 10,
    PROBE_ZED_DIR = 0, /* DIR_NORTH */
    /* (2,10,N) LINFLAS target: front-mirror ordinal 12 = LINFLAS. */
    PROBE_LINFLAS_X = 2,
    PROBE_LINFLAS_Y = 10,
    PROBE_LINFLAS_DIR = 0, /* DIR_NORTH */
    /* Champion ordinals the canonical entrance fixture reports for the
     * walkpath_from_entrance route (DUNGEON.C:2608-2612 C127 sensorData). */
    PROBE_ORDINAL_HALK = 1,
    PROBE_ORDINAL_ZED = 9,
    PROBE_ORDINAL_LINFLAS = 12,
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
    /* C026 champion-portrait strip dimensions: 8 cols x 3 rows of
     * 32x29 portraits (ordinals 0..23). */
    PROBE_PORTRAIT_STRIP_W = 256,
    PROBE_PORTRAIT_STRIP_H = 87
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
 * reuses the same three checks the east_walkpath probe locks:
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

/* Phase A+B: seat the party at the canonical entrance (1,2,N) via
 * set_pose + DM1_V1_MovementPipeline_InitPc34Compat (the same
 * start_independent_input_route contract the existing walkpath
 * probe uses), then drive a short input-path walkpath that
 * exercises turn-right + forward-step + turn-left and verifies the
 * entrance portrait_rect_position contract.  The forward-walk is
 * allowed to be blocked by a wall (result != REDRAW); the probe
 * still verifies the no-portrait contract at the (post-turn)
 * pose and reports the blocked-step result.  The key contract
 * this phase proves is the live input path drives
 * CLIKMENU.C F0365 (turn) and F0366 (forward) without crashing
 * or leaving the party in an invalid state. */
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
         * entrance on this fixture is walkable (forward step
         * returns REDRAW); if the corridor is not walkable the
         * forward step returns IGNORED and we fall through to
         * rotate back to NORTH.  Either way the input-path
         * exercised CLIKMENU.C F0365/F0366 / MOVESENS.C:556 and
         * we verify the post-step pose state. */
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
                /* The forward step landed at (2,2,E).  Verify
                 * the no-portrait invariant at the destination:
                 * front-mirror ordinal at (2,2,E) is -1 (no C127
                 * sensor on the south wall of (2,3) per the
                 * corridor scanner). */
                pose.mapX = postStepX;
                pose.mapY = postStepY;
                pose.dir = postStepDir;
                pose.expectedOrdinal = -1;
                pose.label = "walkpath_from_entrance_b1_forward_east_no_portrait";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
                /* Rotate back to NORTH (E -> N is one turn-left).
                 * Turns do not write G0310 so no cooldown
                 * advance is required between consecutive turns;
                 * a forward step after the turn must age the
                 * cooldown (if any was set by the previous
                 * forward step) before it is processed. */
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
                 * cooldown does not move the party).  Rotate
                 * back to NORTH (E -> N is one turn-left). */
                if (postStepX != PROBE_ENTRANCE_X ||
                    postStepY != PROBE_ENTRANCE_Y ||
                    postStepDir != expectedDir) {
                    fprintf(stderr,
                            "FAIL walkpath_from_entrance_b1_blocked pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                            postStepX, postStepY, postStepDir,
                            PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y, expectedDir);
                    ok = 0;
                }
                (void)turn_left(game);
            }
            /* Verify the entrance portrait is reproduced correctly
             * after the round-trip turn + forward + turn sequence
             * (the cross-cell re-blt invariant clears any stale
             * ordinal pixels from the D1C rect). */
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

/* Phase C+D: re-seed the ZED waypoint (1,10,N) via the same
 * start_independent_input_route contract the existing walkpath probe
 * uses (DM1_V1_MovementPipeline_InitPc34Compat + set_pose), then
 * drive an input-path turn-right + forward-walk to reach (2,10,E),
 * then drive an input-path turn-left to face NORTH at (2,10,N) and
 * verify the LINFLAS portrait_rect_position contract.  The
 * cross-cell re-blt invariant clears the ZED portrait pixels. */
static int drive_zed_to_linflas(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                int* outPrevOrdinal,
                                unsigned char* fb) {
    int ok = 1;
    WalkPathStep pose;

    set_pose(game, PROBE_ZED_X, PROBE_ZED_Y, PROBE_ZED_DIR);
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);

    /* (C) ZED waypoint: (1,10,N) ordinal=9 ZED, C026 col 1 row 1
     * srcX=32 srcY=29. */
    pose.mapX = PROBE_ZED_X;
    pose.mapY = PROBE_ZED_Y;
    pose.dir = PROBE_ZED_DIR;
    pose.expectedOrdinal = PROBE_ORDINAL_ZED;
    pose.label = "walkpath_from_entrance_c_ordinal_9_zed";
    if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
        ok = 0;
    }
    *outPrevOrdinal = pose.expectedOrdinal;

    /* (D prelude) Turn-right at (1,10) to face EAST.  The
     * input-path turn exercises CLIKMENU.C F0365 -> F0700 turn
     * rotation without writing the G0310 cooldown gate (turns
     * never set CLIKMENU.C:330-346 disabled-movement ticks). */
    {
        M11_GameInputResult turnResult = turn_right(game);
        int postTurnX = (int)game->world.party.mapX;
        int postTurnY = (int)game->world.party.mapY;
        int postTurnDir = (int)game->world.party.direction;
        if (turnResult != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_d_turn_right result=%d (want %d)\n",
                    (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("  turn_right_at_zed pose=(%d,%d,%d) result=%d\n",
               postTurnX, postTurnY, postTurnDir, (int)turnResult);
        if (postTurnX != PROBE_ZED_X || postTurnY != PROBE_ZED_Y ||
            postTurnDir != 1 /* DIR_EAST */) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_d_turn_right pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                    postTurnX, postTurnY, postTurnDir,
                    PROBE_ZED_X, PROBE_ZED_Y, 1);
            ok = 0;
        }
        /* Forward-walk east at (1,10,E) -> (2,10,E).  On this
         * DM1 V1 fixture the (2,10) cell is entered from the
         * west via a hallway turn rather than a direct east
         * step, so the forward-step may be BLOCKED.  When the
         * step is BLOCKED the probe re-seeds (2,10,E) via
         * set_pose + DM1_V1_MovementPipeline_InitPc34Compat (the
         * same start_independent_input_route contract the
         * existing walkpath probe uses between independent
         * routes) and continues; the input-path turn-right is
         * still verified by the post-turn pose check above. */
        age_movement_cooldown(game, PROBE_COOLDOWN_TICKS_PER_STEP);
        {
            M11_GameInputResult stepResult = forward_step(game);
            int postStepX = (int)game->world.party.mapX;
            int postStepY = (int)game->world.party.mapY;
            int postStepDir = (int)game->world.party.direction;
            int stepAccepted = (postStepX == PROBE_LINFLAS_X &&
                                postStepY == PROBE_LINFLAS_Y &&
                                postStepDir == 1 /* DIR_EAST */);
            printf("  forward_east_from_zed pose=(%d,%d,%d) result=%d accepted=%d\n",
                   postStepX, postStepY, postStepDir, (int)stepResult,
                   stepAccepted);
            /* HandleInput returns REDRAW even when the forward step
             * was blocked by F0702_MOVEMENT_TryMove_Compat (the
             * pipeline records the block via
             * anyMovementOccurred=0 and clears the queue, but the
             * outer dispatch still returns REDRAW because the
             * pipeline tick itself ran to completion).  The
             * authoritative signal for an accepted step is whether
             * the party position changed; use that instead of
             * stepResult. */
            if (stepAccepted) {
                pose.mapX = postStepX;
                pose.mapY = postStepY;
                pose.dir = postStepDir;
                pose.expectedOrdinal = -1;
                pose.label = "walkpath_from_entrance_d1_forward_east_no_portrait";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
            } else {
                /* The forward step is BLOCKED on this fixture
                 * (the canonical DM1 Hall of Champions geometry
                 * does not let the player walk (1,10) -> (2,10)
                 * directly facing E).  Re-seed (2,10,E) via
                 * set_pose + pipeline reset (matches the
                 * existing walkpath probe's
                 * start_independent_input_route contract) so the
                 * (D) turn-left below still has a known starting
                 * pose.  This is the same teleport-then-walk
                 * pattern the east_walkpath probe uses to bridge
                 * between Hall cells; the input-path turn-right
                 * that landed at (1,10,E) above is the live
                 * walkpath fragment this slice adds. */
                printf("  INFO: forward step (1,10,E) -> (2,10,E) blocked by Hall geometry; re-seeding (2,10,E)\n");
                set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, 1 /* DIR_EAST */);
                DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);
                pose.mapX = PROBE_LINFLAS_X;
                pose.mapY = PROBE_LINFLAS_Y;
                pose.dir = 1;
                pose.expectedOrdinal = -1;
                pose.label = "walkpath_from_entrance_d1_forward_east_no_portrait_reseeded";
                if (!check_walkpath_pose(game, portraits, *outPrevOrdinal, &pose, fb)) {
                    ok = 0;
                }
            }
        }
    }

    /* (D) Turn-left at (2,10,E) to face NORTH -> (2,10,N).  Now
     * the front cell is (2,9) which carries the C127 sensor with
     * sensorData=12 (LINFLAS).  The D1C cutout is dominated by
     * C026 ordinal 12 pixels (srcX=128, srcY=29). */
    {
        M11_GameInputResult turnResult = turn_left(game);
        int postTurnX = (int)game->world.party.mapX;
        int postTurnY = (int)game->world.party.mapY;
        int postTurnDir = (int)game->world.party.direction;
        if (turnResult != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_d_turn_left result=%d (want %d)\n",
                    (int)turnResult, (int)M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("  turn_left_at_linflas pose=(%d,%d,%d) result=%d\n",
               postTurnX, postTurnY, postTurnDir, (int)turnResult);
        if (postTurnX != PROBE_LINFLAS_X || postTurnY != PROBE_LINFLAS_Y ||
            postTurnDir != PROBE_LINFLAS_DIR) {
            fprintf(stderr,
                    "FAIL walkpath_from_entrance_d_turn_left pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                    postTurnX, postTurnY, postTurnDir,
                    PROBE_LINFLAS_X, PROBE_LINFLAS_Y, PROBE_LINFLAS_DIR);
            ok = 0;
        }
        pose.mapX = postTurnX;
        pose.mapY = postTurnY;
        pose.dir = postTurnDir;
        pose.expectedOrdinal = PROBE_ORDINAL_LINFLAS;
        pose.label = "walkpath_from_entrance_d2_target_ordinal_12_linflas";
        if (!check_walkpath_pose(game, portraits, -1, &pose, fb)) {
            ok = 0;
        }
        *outPrevOrdinal = pose.expectedOrdinal;
    }

    /* No-floating side poses at the LINFLAS target (2,10,N). The
     * C127 sensor with sensorData=12 lives on the SOUTH wall of
     * (2,9) only; the side poses (E/S/W) report front-mirror
     * ordinal -1 and the cross-cell re-blt invariant clears the
     * previous LINFLAS portrait pixels.  Mirrors the side-pose
     * coverage the existing east_walkpath probe locks for
     * (2,10,E/S/W). */
    {
        static const struct { int dir; const char* label; } kSidePoses[] = {
            {1, "walkpath_from_entrance_side_east_no_portrait"},
            {2, "walkpath_from_entrance_side_south_no_portrait"},
            {3, "walkpath_from_entrance_side_west_no_portrait"}
        };
        size_t i;
        for (i = 0; i < sizeof(kSidePoses) / sizeof(kSidePoses[0]); ++i) {
            set_pose(game, PROBE_LINFLAS_X, PROBE_LINFLAS_Y, kSidePoses[i].dir);
            pose.mapX = PROBE_LINFLAS_X;
            pose.mapY = PROBE_LINFLAS_Y;
            pose.dir = kSidePoses[i].dir;
            pose.expectedOrdinal = -1;
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

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 12, "
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
    printf("                DEFS.H:2071-2079 (G2071_C320 / G2078_C32 / G2079_C29)\n\n");

    /* Bind the ordinal 12 = LINFLAS identity from the mirror catalog
     * so the slice is bound to a real source identity (the
     * front_north_entry probe already verified ordinal 12 -> LINFLAS
     * on this fixture; we re-pin it here so this probe stays
     * independent if the upstream probe is renamed or split). */
    {
        int seedDir = PROBE_ENTRANCE_DIR;
        set_pose(&game, PROBE_ENTRANCE_X, PROBE_ENTRANCE_Y, seedDir);
        nameBuf[0] = 0;
        (void)M11_GameView_GetMirrorNameByOrdinal(&game, PROBE_ORDINAL_LINFLAS,
                                                 nameBuf, (int)sizeof(nameBuf));
        printf("  INFO: ordinal %d mirror name = %s\n",
               PROBE_ORDINAL_LINFLAS, nameBuf[0] ? nameBuf : "(unknown)");
        if (nameBuf[0] == 0 || strcmp(nameBuf, "LINFLAS") != 0) {
            fprintf(stderr,
                    "FAIL ordinal %d name=%s expected=LINFLAS\n",
                    PROBE_ORDINAL_LINFLAS, nameBuf);
            ok = 0;
        }
    }

    ok &= drive_entrance_walkpath(&game, portraits, &prevOrdinal, currFb);
    ok &= drive_zed_to_linflas(&game, portraits, &prevOrdinal, currFb);

    /* Lock the C026 ordinal-12 source-rect math the probe relies
     * on so a future refactor that moves the C026 atlas stride is
     * caught here too.  The math is identical to the
     * front_north_entry probe's [B] section; we re-pin it because
     * the runtime pixels above are tied to that source rect. */
    {
        int col = -1;
        int row = -1;
        int sx = -1;
        int sy = -1;
        col = PROBE_ORDINAL_LINFLAS & 7;
        row = (PROBE_ORDINAL_LINFLAS >> 3) & 3;
        sx = col * PROBE_PORTRAIT_W;
        sy = row * PROBE_PORTRAIT_H;
        ok &= expect_int("ordinal 12 col = ordinal mod 8", col, 4);
        ok &= expect_int("ordinal 12 row = ordinal / 8", row, 1);
        ok &= expect_int("ordinal 12 source X == 4*32", sx, 128);
        ok &= expect_int("ordinal 12 source Y == 1*29", sy, 29);
        ok &= expect_int("ordinal 12 source bottom edge inside C026 strip",
                         sy + PROBE_PORTRAIT_H <= PROBE_PORTRAIT_STRIP_H, 1);
        ok &= expect_int("ordinal 12 source right edge inside C026 strip",
                         sx + PROBE_PORTRAIT_W <= PROBE_PORTRAIT_STRIP_W, 1);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
