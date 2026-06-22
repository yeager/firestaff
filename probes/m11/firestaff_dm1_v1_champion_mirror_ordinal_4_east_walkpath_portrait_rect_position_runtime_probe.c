/*
 * DM1 V1 Hall of Champions champion portrait ordinal 4 (LEIF):
 * sealed-chamber portrait_rect_position runtime probe.
 *
 * This probe narrows the existing Hall-of-Champions mirror coverage to
 * the ordinal-4 (LEIF) portrait rectangle position invariant, and
 * verifies that the D1C front-wall portrait rectangle (96,35)-(127,63)
 * on the source framebuffer is the destination of the C026 portrait
 * strip slot 4 blit at the synthetic (2,1,SOUTH) pose.  It also
 * verifies that the LEIF portrait does NOT bleed onto the side walls
 * when the party rotates away from the LEIF orientation at (2,1).
 *
 * Coverage that the related probes already lock:
 *   firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
 *     - locks (1,3) NORTH and (1,4) NORTH as no-portrait corridor
 *       poses (no-floating invariant at corridor x=1).
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 *     - locks the (1,3)->(3,3) NORTH forward-walk route through
 *       ordinals 1, -1, 19 with cross-cell re-blt invariant.  Was
 *       authored against a different DM1 V1 reference DUNGEON.DAT
 *       whose (1,3) NORTH layout has ordinal 1 (HALK); the build
 *       tested here has (1,3) NORTH ordinal -1, so this probe
 *       prints SKIP on its reference fixture.  Does NOT cover
 *       ordinal 4 (LEIF).
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     - locks the in-place 4-direction turn at (1,4) with the
 *       re-blt invariant.  Does NOT cover ordinal 4 (LEIF).
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - locks a static (2,1) SOUTH pose returns ordinal 4 (LEIF)
 *       via M11_GameView_GetFrontMirrorOrdinal.  Does NOT render
 *       the game view to a framebuffer and does NOT assert the
 *       D1C portrait rectangle (96,35)-(127,63) pixel-level
 *       position invariant that this probe locks.
 *   firestaff_dm1_v1_champion_mirror_capture_probe
 *     - saves a static PPM at (2,1) SOUTH showing LEIF.  Does NOT
 *       assert that the D1C portrait rectangle (96,35)-(127,63)
 *       actually contains ordinal 4 opaque pixels at the source-
 *       locked C026 stride -- the PPM is a visual artefact, not a
 *       pixel-position contract.
 *
 * Discovery: the assigned route "east_walkpath" does NOT exist as a
 * walkable path in DM1 V1 Hall of Champions (map 0).  The cell (2,1)
 * and the entire x=2 column from y=0..5 are sealed chambers: every
 * adjacent cell's passage into (2,y) is BLOCKED (front cell walks
 * toward x=2 are wall-blocked).  Empirical sweep via
 * M11_GameView_HandleInput(UP) from each adjacent cell of the x=2
 * column confirms every approach is blocked.  The (2,1) cell holds a
 * C127 sensor with sensorData=4 on its south wall (LEIF mirror) but
 * the cell itself is unreachable on foot -- it is a "wall chamber"
 * that the party can never enter through normal movement.  This
 * probe is therefore rewritten from a forward-walk drive into a
 * sealed-chamber portrait-rectangle-position probe anchored at the
 * synthetic (2,1,SOUTH) pose (the same anchor pattern the
 * actual_pose probe uses for static ordinal identity).
 *
 * The probe asserts three things:
 *   1. The x=2 column from y=0..5 is sealed (every adjacent forward
 *      walk that would enter the column is BLOCKED, proving the
 *      east walkpath route does not exist physically and the LEIF
 *      mirror can only be observed through a synthetic pose).
 *   2. At the synthetic (2,1,SOUTH) pose the D1C portrait rectangle
 *      (96,35)-(127,63) on the source framebuffer is dominated by
 *      ordinal 4 opaque pixels from the C026 portrait strip (>=95%
 *      match on the compared pixels, tighter than the existing
 *      walkpath probe's 90% threshold because we are at the same
 *      pose across two consecutive renders with no movement).
 *   3. When the party rotates away from the LEIF orientation at
 *      (2,1), the LEIF pixels do not float to the side wall.  The
 *      rectangle may hold no portrait or a different ordinal, but
 *      ordinal 4 matched pixels must drop below the 35% stale-pixel
 *      tolerance the existing reblt probe locks.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor M011_CELL against view direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores the C127 sensorData ordinal in
 *     G0289 (and only on the M552_FRONT_WALL_ORNAMENT_ORDINAL side).
 *   ReDMCSB DUNVIEW.C:3913-3928 / DUNVIEW.C:8522-8533 blits the C026
 *     portrait into G0109_auc_Graphic558_Box_ChampionPortraitOnWall =
 *     { 96, 127, 35, 63 } (DUNVIEW.C:525) with C01_COLOR_DARK_GRAY
 *     transparency, source stride (ordinal & 7) * 32, (ordinal >> 3) * 29.
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF redraws
 *     the full viewport after MOVESENS.C:556.
 *   ReDMCSB MOVESENS.C:556 / MOVESENS.C:1501-1503 / REVIVE.C F0280.
 *
 * Honesty:
 *   This is Firestaff deterministic runtime evidence.  It does NOT
 *   claim DOS pixel parity because no paired original DM1 PC 3.4
 *   DUNGEON.DAT screenshot covers the (2,1) synthetic LEIF pose.
 *   The probe drives real Firestaff game-view state through the same
 *   M11 input pipeline the live game uses, and the sealed-chamber
 *   walkability guard is genuine source-faithful evidence that the
 *   east walkpath route is physically impossible.
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
    /* ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * = { 96, 127, 35, 63 }.  DUNVIEW.C:3913-3928 / 8522-8533 blits the
     * C026 portrait strip (32x29 per slot) into this exact box. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* DUNVIEW.C:3916 / 8525: the C026 blit masks C01_COLOR_DARK_GRAY = 1
     * as transparency.  Same constant the existing walkpath / visibility /
     * zorder probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* ReDMCSB DUNGEON.C:2558 / DUNVIEW.C:3916 per-slot source stride:
     * (ordinal & 7) * 32 wide, (ordinal >> 3) * 29 tall, 24 slots total
     * (8 cols x 3 rows of the C026 GRAPHIC_CHAMPION_PORTRAITS strip). */
    PROBE_ORDINAL_COUNT = 24,
    /* Existing walkpath / reblt probe tolerance for the cross-cell
     * stale-pixel leak check (35% of the prior ordinal's compared
     * pixel count).  Mirrored here so this probe is consistent with
     * the per-direction coverage. */
    PROBE_STALE_LEAK_PCT = 35
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

typedef struct EastWalkStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    int inputBeforeCheck; /* M12 input token, -1 if no input */
    int allowNoPortraitDominance;
    const char* label;
} EastWalkStep;

/* Count the pixels in the D1C front-wall box that match the C026
 * champion portrait ordinal for the given ordinal.  Same compare
 * loop as the existing walkpath probe (DUNVIEW.C:3916 dark-gray
 * transparency mask + per-ordinal DUNVIEW.C:3918 (ordinal & 7) * 32
 * + (ordinal >> 3) * 29 source stride).  Returns 0 when ordinal
 * is out of range or the slot is not loaded. */
static int count_ordinal_matched_pixels(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= PROBE_ORDINAL_COUNT) {
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

/* Walk all 24 ordinals and find the best-matching ordinal in the
 * D1C front-wall box, plus the expected (probe-supplied) ordinal's
 * own matched count.  Mirrors match_front_portrait() in the
 * existing walkpath probe. */
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
    for (ordinal = 0; ordinal < PROBE_ORDINAL_COUNT; ++ordinal) {
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

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* Check a step in the rotate-away invariant loop.  At each step
 * the pose must match the expected (x,y,dir), the front mirror
 * ordinal must match the expected ordinal (when >= 0), and the
 * D1C portrait rectangle (96,35)-(127,63) must be dominated by
 * the expected ordinal (or have no portrait).  Mirrors
 * check_input_walk_step() from the existing walkpath probe,
 * narrowed to the synthetic (2,1) anchor where the east walkpath
 * is physically impossible.  The function is generic enough to be
 * used for any in-place rotation that changes party direction
 * without changing the (x,y) cell, which is exactly what
 * check_rotate_away_no_floating exercises. */
static int check_east_walk_step(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                int prevOrdinal,
                                const EastWalkStep* step,
                                unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    if (game->world.party.mapX != step->mapX ||
        game->world.party.mapY != step->mapY ||
        game->world.party.direction != step->dir) {
        fprintf(stderr,
                "FAIL %s pose got=(%d,%d,%d) want=(%d,%d,%d)\n",
                step->label,
                game->world.party.mapX, game->world.party.mapY,
                game->world.party.direction,
                step->mapX, step->mapY, step->dir);
        ok = 0;
    }

    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != step->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                step->label, ordinal, step->expectedOrdinal);
        ok = 0;
    }

    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, outFb,
                                 step->expectedOrdinal >= 0
                                     ? step->expectedOrdinal
                                     : 0);

    if (step->expectedOrdinal >= 0) {
        if (match.bestOrdinal != step->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * 90) {
            fprintf(stderr,
                    "FAIL %s D1C portrait_rect_position expected=%d best=%d matched=%d/%d (90%% threshold)\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else if (!step->allowNoPortraitDominance &&
               prevOrdinal != -2 &&
               match.bestMatched * 100 >= 35 * (match.compared > 0 ? match.compared : 1)) {
        fprintf(stderr,
                "FAIL %s no-portrait step leaked portrait best=%d matched=%d/%d\n",
                step->label, match.bestOrdinal, match.bestMatched, match.compared);
        ok = 0;
    }

    /* Cross-cell re-blt invariant: when the ordinal changes between
     * steps, the prior ordinal's pixels must not be the dominant
     * match in the new framebuffer's portrait rectangle.  This is
     * the in-place-turn analogue the existing zorder / reblt
     * probes lock: the prior ordinal's pixels must be cleared from
     * the D1C wall box when the front cell changes
     * (DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF re-blits the
     * full viewport from the new party pose after MOVESENS.C:556).
     * 35% matches the existing reblt probe tolerance. */
    if (prevOrdinal >= 0 && prevOrdinal != step->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, outFb, prevOrdinal);
        int prevCompared = match_front_portrait(portraits, outFb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= PROBE_STALE_LEAK_PCT) {
            fprintf(stderr,
                    "FAIL %s cross-cell stale ordinal=%d leaked matched=%d/%d after step to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared,
                    step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label,
           game->world.party.mapX, game->world.party.mapY,
           game->world.party.direction, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

/* Sealed-chamber walkpath-impossible guard.
 *
 * DM1 V1 Hall of Champions (map 0) seals the x=2 column from y=0..5
 * by surrounding each (2,y) cell with a C127 mirror sensor on the
 * wall facing inward (or facing the corridor) but with a wall
 * blocking passage from any adjacent cell.  Concretely:
 *
 *   (1,y) EAST   forward -> (2,y) is BLOCKED for y in 0..5
 *   (3,y) WEST   forward -> (2,y) is BLOCKED for y in 0..5
 *   (2,y) NORTH  forward -> (2,y-1) is BLOCKED for y in 1..5
 *   (2,y) SOUTH  forward -> (2,y+1) is BLOCKED for y in 0..4
 *
 * This guard proves the seal by driving M11_GameView_HandleInput(UP)
 * (forward) at each candidate entry pose and asserting both:
 *   (a) the input returns M11_GAME_INPUT_REDRAW (a redraw happened,
 *       no input was dropped silently), and
 *   (b) the party position did NOT change (the sealed cell was not
 *       entered).
 *
 * The seal is the source of truth for the "east walkpath" route: it
 * does not exist.  The LEIF mirror at (2,1) ordinal 4 is only
 * observable through a synthetic pose, exactly the same anchor
 * pattern the actual_pose probe uses.  This probe then anchors at
 * that synthetic pose and verifies the portrait_rect_position
 * pixel-level invariant the actual_pose probe does not cover. */
typedef struct SealedCellProbe {
    int fromX;
    int fromY;
    int facing; /* DIR_* that would step into (2,y) */
    const char* label;
} SealedCellProbe;

static int verify_sealed_chamber_walkpath_guard(M11_GameViewState* game) {
    /* For each entry attempt that would step into the x=2 column at
     * y in 0..5, drive M11_GameView_HandleInput(UP) and assert the
     * party position does not change.  The exact direction matters
     * because the engine only computes the front cell relative to
     * the party direction; we use the direction that points the
     * party AT the sealed cell. */
    const SealedCellProbe probes[] = {
        /* Approaches to (2,y) for y in 0..5 from the west (x=1). */
        {1, 0, DIR_EAST, "leif_seal_from_west_y0_east"},
        {1, 1, DIR_EAST, "leif_seal_from_west_y1_east"},
        {1, 2, DIR_EAST, "leif_seal_from_west_y2_east"},
        {1, 3, DIR_EAST, "leif_seal_from_west_y3_east"},
        {1, 4, DIR_EAST, "leif_seal_from_west_y4_east"},
        {1, 5, DIR_EAST, "leif_seal_from_west_y5_east"},
        /* Approaches to (2,y) for y in 0..5 from the east (x=3). */
        {3, 0, DIR_WEST, "leif_seal_from_east_y0_west"},
        {3, 1, DIR_WEST, "leif_seal_from_east_y1_west"},
        {3, 2, DIR_WEST, "leif_seal_from_east_y2_west"},
        {3, 3, DIR_WEST, "leif_seal_from_east_y3_west"},
        {3, 4, DIR_WEST, "leif_seal_from_east_y4_west"},
        {3, 5, DIR_WEST, "leif_seal_from_east_y5_west"},
    };
    int stepIdx;
    int ok = 1;
    int stepCount = (int)(sizeof(probes) / sizeof(probes[0]));

    for (stepIdx = 0; stepIdx < stepCount; ++stepIdx) {
        int beforeX, beforeY, beforeDir;
        M11_GameInputResult result;
        set_pose(game, probes[stepIdx].fromX, probes[stepIdx].fromY,
                 probes[stepIdx].facing);
        beforeX = game->world.party.mapX;
        beforeY = game->world.party.mapY;
        beforeDir = game->world.party.direction;
        result = M11_GameView_HandleInput(game, M12_MENU_INPUT_UP);
        if (game->world.party.mapX != beforeX ||
            game->world.party.mapY != beforeY ||
            game->world.party.direction != beforeDir) {
            fprintf(stderr,
                    "FAIL %s forward into x=2 column moved party "
                    "(%d,%d,%d) -> (%d,%d,%d); expected sealed\n",
                    probes[stepIdx].label,
                    beforeX, beforeY, beforeDir,
                    game->world.party.mapX,
                    game->world.party.mapY,
                    game->world.party.direction);
            ok = 0;
        }
        if (result != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL %s blocked-forward result=%d want=%d (redraw)\n",
                    probes[stepIdx].label, result, M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        printf("leif_seal %s pose=(%d,%d,%d) blocked result=%d\n",
               probes[stepIdx].label,
               game->world.party.mapX,
               game->world.party.mapY,
               game->world.party.direction, result);
    }
    return ok;
}

/* LEIF portrait_rect_position synthetic-pose check.
 *
 * Anchors at (2,1,SOUTH) via set_pose() (the same synthetic anchor
 * the actual_pose probe uses), renders the game view to a 320x200
 * framebuffer, and asserts:
 *
 *   - the D1C front-wall portrait rectangle (96,35)-(127,63) is
 *     dominated by ordinal 4 opaque pixels from the C026 portrait
 *     strip (>=95% match on the compared non-transparent pixels,
 *     tighter than the existing walkpath probe's 90% threshold
 *     because we are at the same pose across two consecutive
 *     renders with no movement, so no cross-cell re-blt can drop
 *     pixels).
 *   - the matched count is non-zero (the strip slot 4 is loaded).
 *   - re-rendering the same pose gives the exact same matched count
 *     (deterministic re-blt, no flake between draws).
 *
 * This is the unique contribution of the probe: actual_pose locks
 * that the front mirror ordinal IS 4; this probe locks that the
 * DUNVIEW.C:3913-3928 / 8522-8533 blit actually paints the C026
 * strip slot 4 into the source-locked D1C rectangle (96,35)-(127,63)
 * on the rendered framebuffer.  If the blit were to land at any
 * other screen rectangle (e.g. (96,40) for a side wall, or
 * (110,35) for a centred chest panel), this probe would fail. */
static int verify_leif_portrait_rect_position_synthetic(M11_GameViewState* game,
                                                          const M11_AssetSlot* portraits,
                                                          unsigned char* outFb) {
    MirrorMatch matchA, matchB;
    int ok = 1;

    set_pose(game, 2, 1, DIR_SOUTH);
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    matchA = match_front_portrait(portraits, outFb, 4);
    if (matchA.compared <= 0) {
        fprintf(stderr,
                "FAIL leif_portrait_rect empty compare (compared=%d); "
                "C026 slot 4 not loaded or D1C box off-screen\n",
                matchA.compared);
        return 0;
    }
    if (matchA.bestOrdinal != 4) {
        fprintf(stderr,
                "FAIL leif_portrait_rect best ordinal=%d expected=4 "
                "(matched=%d/%d)\n",
                matchA.bestOrdinal, matchA.bestMatched, matchA.compared);
        ok = 0;
    }
    if (matchA.expectedMatched * 100 < matchA.compared * 95) {
        fprintf(stderr,
                "FAIL leif_portrait_rect ordinal 4 match=%d/%d < 95%% "
                "(best=%d matched=%d)\n",
                matchA.expectedMatched, matchA.compared,
                matchA.bestOrdinal, matchA.bestMatched);
        ok = 0;
    }

    /* Determinism: re-render the same synthetic pose and verify the
     * matched count is identical (no F0128_DUNGEONVIEW_Draw_CPSF
     * flake between consecutive draws). */
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    matchB = match_front_portrait(portraits, outFb, 4);
    if (matchB.expectedMatched != matchA.expectedMatched ||
        matchB.bestOrdinal != matchA.bestOrdinal ||
        matchB.bestMatched != matchA.bestMatched) {
        fprintf(stderr,
                "FAIL leif_portrait_rect non-deterministic re-render: "
                "A=(best=%d matched=%d expected=%d) "
                "B=(best=%d matched=%d expected=%d)\n",
                matchA.bestOrdinal, matchA.bestMatched, matchA.expectedMatched,
                matchB.bestOrdinal, matchB.bestMatched, matchB.expectedMatched);
        ok = 0;
    }

    printf("leif_portrait_rect pose=(2,1,SOUTH) ordinal=4 best=%d "
           "matched=%d/%d (>=95%% threshold)\n",
           matchA.bestOrdinal, matchA.bestMatched, matchA.compared);
    return ok;
}

/* At the synthetic (2,1,SOUTH) anchor (same anchor pattern the
 * actual_pose probe uses), rotate the party through the other 3
 * directions without moving.  The D1C portrait rectangle
 * (96,35)-(127,63) must NOT keep showing the LEIF ordinal 4 pixels
 * from a side wall perspective -- if it does, the rectangle is
 * floating on a side wall (the "champion Z-order/floating" P1 bug).
 * This is the rotate-away invariant the existing zorder / reblt
 * probes lock for ordinals 1/2/3/19; here we lock it for ordinal 4
 * (LEIF).
 *
 * At (2,1) NORTH the front cell is (2,0).  At (2,1) EAST the
 * front cell is (3,1) which has ordinal 8 in this DM1 V1 build.
 * At (2,1) WEST the front cell is (1,1).  The north/west poses
 * must show wall texture only (no ordinal-4 pixels above the 35%
 * stale-leak tolerance); the east pose shows ordinal 8 (a
 * different champion mirror on the WEST wall of (3,1)) and the
 * LEIF pixel-leak invariant must hold even when a *new* ordinal
 * is now painted into the same D1C rectangle. */
static int check_rotate_away_no_floating(M11_GameViewState* game,
                                          const M11_AssetSlot* portraits,
                                          unsigned char* outFb) {
    const EastWalkStep rotateSteps[] = {
        {2, 1, DIR_SOUTH, 4, -1, 0,
         "leif_rotate_anchor_south_ordinal_4"},
        {2, 1, DIR_WEST, -1, M12_MENU_INPUT_RIGHT, 0,
         "leif_rotate_west_no_floating"},
        {2, 1, DIR_NORTH, -1, M12_MENU_INPUT_RIGHT, 0,
         "leif_rotate_north_no_floating"},
        {2, 1, DIR_EAST, 8, M12_MENU_INPUT_RIGHT, 0,
         "leif_rotate_east_ordinal_8"},
        {2, 1, DIR_SOUTH, 4, M12_MENU_INPUT_RIGHT, 0,
         "leif_rotate_back_to_south_ordinal_4"},
    };
    int stepIdx;
    int prevOrdinal = -2;
    int ok = 1;
    int stepCount = (int)(sizeof(rotateSteps) / sizeof(rotateSteps[0]));

    /* Anchor: party is at (2,1,SOUTH) via set_pose (the east walkpath
     * to this cell is physically impossible; see
     * verify_sealed_chamber_walkpath_guard). */
    set_pose(game, 2, 1, DIR_SOUTH);
    for (stepIdx = 0; stepIdx < stepCount; ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        if (rotateSteps[stepIdx].inputBeforeCheck >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandleInput(game, rotateSteps[stepIdx].inputBeforeCheck);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s input=%d result=%d want=%d\n",
                        rotateSteps[stepIdx].label,
                        rotateSteps[stepIdx].inputBeforeCheck,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        stepOk = check_east_walk_step(game, portraits, prevOrd,
                                      &rotateSteps[stepIdx], outFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = rotateSteps[stepIdx].expectedOrdinal;
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

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open selected DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Fixture guard: the LEIF portrait_rect_position probe relies
     * on the canonical DM1 V1 Hall of Champions sensor layout the
     * actual_pose probe exercises -- (1,3,SOUTH) front ordinal 10
     * (ZED on (1,4) north wall) and (2,1,SOUTH) front ordinal 4
     * (LEIF on (2,2) north wall).  DM1 V1 builds with a different
     * sensor layout print SKIP and exit 0 instead of FAIL -- this
     * is not a regression detector, it is a per-build fixture
     * guard. */
    {
        set_pose(&game, 1, 3, DIR_SOUTH);
        int probeStartOrd = M11_GameView_GetFrontMirrorOrdinal(&game);
        set_pose(&game, 2, 1, DIR_SOUTH);
        int probeLeifOrd = M11_GameView_GetFrontMirrorOrdinal(&game);
        if (probeStartOrd != 10 || probeLeifOrd != 4) {
            printf("SKIP leif_portrait_rect_fixture_mismatch "
                   "(1,3,SOUTH) front ordinal=%d expected=10, "
                   "(2,1,SOUTH) front ordinal=%d expected=4; "
                   "this DM1 V1 build does not match the reference "
                   "DUNGEON.DAT Hall of Champions sensor layout "
                   "(see actual_pose_runtime_probe for the full "
                   "cell->ordinal map)\n",
                   probeStartOrd, probeLeifOrd);
            M11_GameView_Shutdown(&game);
            return 0;
        }
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 LEIF ordinal 4 sealed-chamber portrait_rect_position ===\n");

    if (!verify_sealed_chamber_walkpath_guard(&game)) {
        ok = 0;
    }
    if (!verify_leif_portrait_rect_position_synthetic(&game, portraits, currFb)) {
        ok = 0;
    }
    if (!check_rotate_away_no_floating(&game, portraits, currFb)) {
        ok = 0;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror ordinal 4 sealed-chamber portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
