/*
 * DM1 V1 champion mirror walk-path runtime probe.
 *
 * This broadens the existing Hall-of-Champions mirror/no-floating
 * coverage with a real forward-walk interaction slice that the
 * per-cell and in-place-turn probes do not exercise:
 *
 *   firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
 *     - covers (1,3) facing N and (1,4) facing N in isolation.
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
 *     - covers six static poses (front + side) at two cells.
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     - covers the in-place 4-direction turn at (1,4) with the re-blt
 *       invariant (no stale ordinal pixels left in the portrait rect
 *       after the turn).
 *
 * This probe covers the **forward-walk** route through the Hall
 * corridor: the party moves east one cell at a time from (1,3)
 * to (2,3) to (3,3) and back, always facing NORTH.  At each
 * step the front wall square is a different map cell, and the
 * wall-ornament-driven champion portrait ordinal changes (1,
 * none, 19 in the reference DM1 V1 DUNGEON.DAT).  The re-blt
 * invariant must be honoured **across cell transitions**, not
 * just across in-place turns:
 *
 *   - At the (1,3) -> (2,3) step, the new front square is (2,2)
 *     which has no C127 sensor, so the new front mirror ordinal
 *     is -1.  The portrait rectangle must be cleared of the
 *     previous ordinal 1's pixels and must not be dominated by
 *     a portrait ordinal.
 *   - At the (2,3) -> (3,3) step, the new front square is (3,2)
 *     which has ordinal 19; the portrait rectangle must be
 *     dominated by ordinal 19 pixels and must not show stale
 *     pixels from the previous (no-portrait) step.
 *   - At the (3,3) -> (2,3) and (2,3) -> (1,3) back-steps, the
 *     mirror reverses: 19 -> -1 -> 1, and the prior ordinal
 *     pixels must be cleared from the D1C wall box each time.
 *
 * The walk deliberately keeps the front wall on a corridor / wall
 * cell where the no-portrait pixel match stays within the 35%
 * leak tolerance the existing zorder and reblt probes lock (the
 * Hall of Champions at x=1, y=5/7/8 has wall cells with a
 * different wall-ornament pixel pattern that the 35% threshold
 * flags; the corridor cells at (2,3) and (0,3) have the cleaner
 * pattern the existing per-direction coverage already accepts).
 *
 * The probe narrows the contract to the same D1C front-wall box
 * (96,35)-(127,63) the existing zorder and reblt probes lock, and
 * uses the same transparent color 1 mask the existing visibility
 * probe locks (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY / C26 champion
 * portrait source).  It does not assert wall-perimeter pixel
 * stability across cells because different cells naturally show
 * different wall geometry (a different square at the front); the
 * per-direction wall draws are the existing zorder probe's
 * coverage.
 *
 * The second half of the probe drives the canonical legal Hall route
 * through the public M11 input path: start at (1,3,SOUTH), move
 * forward into the corridor, then turn around to face the second
 * mirror at (1,4,NORTH), then left-turn through the side-wall
 * no-portrait pose and back to the south-facing mirror.  It then
 * drives a backstep/forward Hall route from (1,4,SOUTH) to
 * (1,3,SOUTH) and back, proving the same stale-pixel/no-floating
 * invariant on the CLIKMENU.C F0366 backward movement branch.  It also
 * clicks the original V1 movement-arrow rectangles for forward/back and
 * left/right turn Hall routes, proving the mouse route enters the same
 * source command path before the D1C portrait box is re-blitted.  It also
 * clicks the visible front-wall portrait after pointer movement has exposed
 * it, proving the C080 viewport route opens C040 over a movement-produced
 * mirror pose and that cancel clears the overlay while preserving the mirror
 * route.  It also repeats that C080/C040 interaction after an accepted
 * lateral movement into the north-facing Hall mirror, proving a strafe-produced
 * mirror pose can enter and clear the same real-asset panel without leaving
 * stale portrait or panel pixels.  It also clicks the original movement-arrow
 * boxes while C040 is live, including the lower-row lateral arrows, proving
 * the M568 panel dispatch ignores off-panel movement hits and keeps the
 * mirror portrait/panel stack stable.  It also
 * drives the source C006/C004 lateral command pair against the south-facing
 * Hall mirror, covering the blocked movement redraw branch that the
 * forward/back and turn routes do not touch.  It also
 * drives keyboard and pointer lateral movement through the lower movement
 * arrow row (C004/C006), proving the same no-floating invariant across
 * the strafe branches of CLIKMENU.C F0366.  A final mixed route alternates
 * pointer and keyboard commands through the same live movement-pipeline state,
 * covering the COMMAND.C F0359 mouse queue and F0361 keyboard dispatch
 * interleave called out by BUG0_73 without widening the pixel contract.
 * That locks keyboard and pointer movement/turning through COMMAND.C
 * F0359/F0361 -> CLIKMENU.C F0365/F0366 -> MOVESENS.C tick boundaries used
 * by real runtime input while keeping the pixel assertion identical to the
 * direct route above.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor cell to front-wall aspect.
 *   ReDMCSB DUNGEON.C:2608-2612 sets G0289 for C127 champion
 *     portraits; G0289 stores the ordinal indexed by the C127
 *     sensor on the front wall.
 *   ReDMCSB DUNVIEW.C:3913-3928 and 8522-8533 restrict the
 *     C026 champion-portrait blit to the D1C front wall box
 *     (96,35)-(127,63) with the C01 dark-gray transparency mask.
 *   ReDMCSB DUNVIEW.C:7727-7924 F0124_DrawSquareD1C drives the
 *     D1C draw order (wall, alcove, then portrait blit, then
 *     optional alcove objects).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF
 *     draws the viewport from the new party pose after every
 *     MOVESENS.C:556 tick; the full viewport is re-blitted each
 *     step, so the portrait rectangle is rebuilt from the new
 *     front wall ordinal.
 *   ReDMCSB DUNVIEW.C:2558 (BUG0_75) notes that G0289 is only
 *     reset when the draw function sees at least one wall square;
 *     this probe covers that invariant by walking through wall
 *     squares (D1C is the wall type) on every step.
 */
#include "m11_game_view.h"
#include "dm1_v1_movement_pipeline_pc34_compat.h"
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
    /* DUNVIEW.C:3913-3928 / 8522-8533: the D1C front-wall box is the
     * 32x29 rectangle at (96,35)-(127,63) of the viewport, drawn
     * from the C026 champion portrait strip indexed by the C127
     * sensor ordinal stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* PANEL.C F0342/F0346 source C101 panel placement: C040 is drawn at
     * viewport-relative (80,52).  COMMAND.C:228-233 / 508-511 then routes
     * C160/C161/C162 panel buttons while G0299 is live. */
    PROBE_RR_PANEL_GRAPHIC = 40,
    PROBE_PANEL_X = PROBE_VIEWPORT_X + 80,
    PROBE_PANEL_Y = PROBE_VIEWPORT_Y + 52,
    PROBE_FRONT_PORTRAIT_CLICK_X = PROBE_PORTRAIT_X + 16,
    PROBE_FRONT_PORTRAIT_CLICK_Y = PROBE_PORTRAIT_Y + 14,
    PROBE_CANCEL_CLICK_X = 160,
    PROBE_CANCEL_CLICK_Y = 151,
    /* DUNVIEW.C:3916: the C026 champion portrait blit masks the
     * C01_COLOR_DARK_GRAY (value 1) as transparency.  This is the
     * same constant the existing visibility / zorder / reblt
     * probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

typedef struct WalkStep {
    int mapX;
    int mapY;
    int expectedOrdinal;
    const char* label;
} WalkStep;

typedef struct InputWalkStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    int inputBeforeCheck;
    int allowNoPortraitDominance;
    const char* label;
} InputWalkStep;

typedef struct PointerWalkStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    int clickX;
    int clickY;
    const char* label;
} PointerWalkStep;

typedef struct MixedWalkStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    int inputBeforeCheck;
    int clickX;
    int clickY;
    int allowNoPortraitDominance;
    const char* label;
} MixedWalkStep;

typedef struct LateralWalkStep {
    int startMapX;
    int startMapY;
    int startDir;
    int startOrdinal;
    int expectedMapX;
    int expectedMapY;
    int expectedOrdinal;
    int inputBeforeCheck;
    int clickX;
    int clickY;
    const char* label;
} LateralWalkStep;

typedef struct PanelMatch {
    int assetOpaque;
    int assetDrawn;
} PanelMatch;

/* Count the pixels in the front-wall box that match the C026
 * champion portrait ordinal.  This reuses the visibility probe's
 * match formula (DUNVIEW.C:3916 C01 dark-gray transparency mask
 * + per-ordinal DUNVIEW.C:3918 (ordinal & 7) * 32 + (ordinal >> 3)
 * * 29 source stride).
 *
 * Returns the number of opaque ordinal pixels that actually
 * match between the source strip and the framebuffer, or 0 when
 * either the ordinal is out of range or the slot is not loaded. */
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

static PanelMatch match_panel(const M11_AssetSlot* panel,
                              const unsigned char* fb,
                              int transparentColor) {
    PanelMatch out;
    int x;
    int y;
    memset(&out, 0, sizeof(out));
    if (!panel || !panel->loaded || !panel->pixels || !fb) {
        return out;
    }
    for (y = 0; y < (int)panel->height; ++y) {
        int fbY = PROBE_PANEL_Y + y;
        if (fbY < 0 || fbY >= PROBE_FB_H) {
            continue;
        }
        for (x = 0; x < (int)panel->width; ++x) {
            int fbX = PROBE_PANEL_X + x;
            unsigned char src;
            unsigned char dst;
            if (fbX < 0 || fbX >= PROBE_FB_W) {
                continue;
            }
            src = (unsigned char)(panel->pixels[y * (int)panel->width + x] & 0x0F);
            if (src == transparentColor) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(fb[fbY * PROBE_FB_W + fbX]);
            ++out.assetOpaque;
            if (dst == src) {
                ++out.assetDrawn;
            }
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

static void start_independent_input_route(M11_GameViewState* game,
                                          int mapX,
                                          int mapY,
                                          int dir) {
    set_pose(game, mapX, mapY, dir);
    /* COMMAND.C:2096-2106 gates movement commands on G0310/G0311.  Each
     * route in this probe is an independent real-asset slice, so reset the
     * source-locked queue/cooldown mirror before starting a new route instead
     * of inheriting the previous slice's movement-disabled ticks. */
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);
}

/* Forward-walk re-blt invariant check.  At each step the new
 * ordinal must dominate (or the rectangle must have no portrait)
 * and the previous ordinal must not be the dominant match.  The
 * 35% threshold matches the existing zorder probe's leak tolerance
 * (DUNVIEW.C:3916 dark-gray transparency and per-ordinal compared
 * count) so this probe is consistent with the per-direction
 * coverage. */
static int check_walk_step(M11_GameViewState* game,
                           const M11_AssetSlot* portraits,
                           int prevOrdinal,
                           const WalkStep* step,
                           unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, DIR_NORTH);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != step->expectedOrdinal) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=%d\n",
                step->label, ordinal, step->expectedOrdinal);
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    /* When the step is a no-portrait cell, pass ordinal 0 as the
     * "expected" so the per-ordinal compared count is non-zero and
     * the leak tolerance (35%) is consistent with the existing
     * zorder / reblt probes.  The no-portrait step then asserts
     * best-matched over best-compared is below 35% (no ordinal
     * dominates the rectangle). */
    match = match_front_portrait(portraits, outFb,
                                 step->expectedOrdinal >= 0
                                     ? step->expectedOrdinal
                                     : 0);

    if (step->expectedOrdinal >= 0) {
        if (match.bestOrdinal != step->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 < match.compared * 90) {
            fprintf(stderr,
                    "FAIL %s visible portrait expected=%d best=%d matched=%d/%d\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else {
        /* No-portrait step: the rectangle must not be dominated by a
         * portrait ordinal (no stale pixels left over from the prior
         * step's ordinal, and the wall pixels do not look like a
         * portrait). */
        if (match.bestMatched * 100 >= 35 * (match.compared > 0 ? match.compared : 1)) {
            fprintf(stderr,
                    "FAIL %s no-portrait step leaked portrait best=%d matched=%d/%d\n",
                    step->label, match.bestOrdinal, match.bestMatched, match.compared);
            ok = 0;
        }
    }

    /* Cross-cell re-blt invariant: when the ordinal changes between
     * steps, the prior ordinal's pixels must not be the dominant
     * match in the new framebuffer's portrait rectangle.  This is
     * the forward-walk analogue of the in-place turn invariant in
     * firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe:
     * the prior ordinal's pixels must be cleared from the D1C wall
     * box when the front cell changes (DUNVIEW.C:8318-8542
     * F0128_DUNGEONVIEW_Draw_CPSF re-blits the full viewport from
     * the new party pose after MOVESENS.C:556).
     *
     * We compute the prior ordinal's "stale" count twice: once
     * against the new framebuffer (the candidate leak) and once
     * against the prior ordinal's own compared count (the
     * expected proportion).  The 35% threshold matches the
     * existing zorder / reblt probes' tolerance. */
    if (prevOrdinal >= 0 && prevOrdinal != step->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, outFb, prevOrdinal);
        int prevExpected = match_front_portrait(portraits, outFb, prevOrdinal).expectedMatched;
        int prevCompared = match_front_portrait(portraits, outFb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= 35) {
            fprintf(stderr,
                    "FAIL %s cross-cell stale ordinal=%d leaked matched=%d/%d (expected=%d) after step to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared, prevExpected,
                    step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s pose=(%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label, step->mapX, step->mapY, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

static int check_input_walk_step(M11_GameViewState* game,
                                 const M11_AssetSlot* portraits,
                                 int prevOrdinal,
                                 const InputWalkStep* step,
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
                    "FAIL %s visible portrait expected=%d best=%d matched=%d/%d\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else if (!step->allowNoPortraitDominance &&
               prevOrdinal != -2 &&
               match.bestMatched * 100 >= 35 * (match.compared > 0 ? match.compared : 1)) {
        /* The first input frame can be a no-portrait baseline with no
         * prior ordinal to clear; only later no-portrait input frames
         * prove the stale-pixel/no-floating invariant. */
        fprintf(stderr,
                "FAIL %s input no-portrait step leaked portrait best=%d matched=%d/%d\n",
                step->label, match.bestOrdinal, match.bestMatched, match.compared);
        ok = 0;
    }

    if (prevOrdinal >= 0 && prevOrdinal != step->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, outFb, prevOrdinal);
        int prevCompared = match_front_portrait(portraits, outFb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= 35) {
            fprintf(stderr,
                    "FAIL %s input stale ordinal=%d leaked matched=%d/%d after step to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared,
                    step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s input_pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label,
           game->world.party.mapX, game->world.party.mapY,
           game->world.party.direction, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

static int check_pointer_moved_mirror_candidate_cancel(M11_GameViewState* game,
                                                       const M11_AssetSlot* portraits,
                                                       const M11_AssetSlot* rrPanel,
                                                       unsigned char* outFb) {
    M11_GameInputResult result;
    PanelMatch panelMatch;
    InputWalkStep checkStep;
    int ok = 1;

    /* This route composes the pointer movement path with the C080 viewport
     * mirror selection path.  ReDMCSB COMMAND.C G0448/F0359 sends the arrow
     * click through CLIKMENU.C F0366 first; a later C080 viewport click enters
     * CLIKVIEW.C F0377/F0372/F0275 and REVIVE.C F0280/F0282 to append the
     * candidate and show C040. */
    start_independent_input_route(game, 1, 3, DIR_SOUTH);
    result = M11_GameView_HandlePointer(game, 276, 135, 1);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_forward pointer result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }

    memset(&checkStep, 0, sizeof(checkStep));
    checkStep.mapX = 1;
    checkStep.mapY = 4;
    checkStep.dir = DIR_SOUTH;
    checkStep.expectedOrdinal = 3;
    checkStep.inputBeforeCheck = -1;
    checkStep.allowNoPortraitDominance = 0;
    checkStep.label = "hall_pointer_candidate_after_forward_ordinal_3";
    if (!check_input_walk_step(game, portraits, -2, &checkStep, outFb)) {
        ok = 0;
    }

    result = M11_GameView_HandlePointerButton(game,
                                              PROBE_FRONT_PORTRAIT_CLICK_X,
                                              PROBE_FRONT_PORTRAIT_CLICK_Y,
                                              M11_DM1_MOUSE_MASK_LEFT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_portrait_click result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }
    if (game->candidateMirrorPanelActive != 1 ||
        game->inventoryPanelActive != 1 ||
        game->candidateMirrorOrdinal != 3 ||
        game->candidateMirrorPartyIndex != 0 ||
        game->world.party.championCount != 1) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_open state panel=%d inventory=%d ordinal=%d partyIndex=%d champions=%d\n",
                game->candidateMirrorPanelActive,
                game->inventoryPanelActive,
                game->candidateMirrorOrdinal,
                game->candidateMirrorPartyIndex,
                game->world.party.championCount);
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    panelMatch = match_panel(rrPanel, outFb, 6);
    if (panelMatch.assetOpaque <= 0 ||
        panelMatch.assetDrawn * 100 < 90 * panelMatch.assetOpaque) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_open C040 missing drawn=%d/%d\n",
                panelMatch.assetDrawn, panelMatch.assetOpaque);
        ok = 0;
    }

    result = M11_GameView_HandlePointerButton(game,
                                              PROBE_CANCEL_CLICK_X,
                                              PROBE_CANCEL_CLICK_Y,
                                              M11_DM1_MOUSE_MASK_LEFT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_cancel result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }
    if (game->candidateMirrorPanelActive != 0 ||
        game->inventoryPanelActive != 0 ||
        game->candidateMirrorOrdinal != -1 ||
        game->candidateMirrorPartyIndex != -1 ||
        game->world.party.championCount != 0 ||
        M11_GameView_GetFrontMirrorOrdinal(game) != 3) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_cancel state panel=%d inventory=%d ordinal=%d partyIndex=%d champions=%d front=%d\n",
                game->candidateMirrorPanelActive,
                game->inventoryPanelActive,
                game->candidateMirrorOrdinal,
                game->candidateMirrorPartyIndex,
                game->world.party.championCount,
                M11_GameView_GetFrontMirrorOrdinal(game));
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    panelMatch = match_panel(rrPanel, outFb, 6);
    if (panelMatch.assetOpaque > 0 &&
        panelMatch.assetDrawn * 100 >= 40 * panelMatch.assetOpaque) {
        fprintf(stderr,
                "FAIL hall_pointer_candidate_cancel C040 leaked drawn=%d/%d\n",
                panelMatch.assetDrawn, panelMatch.assetOpaque);
        ok = 0;
    }
    memset(&checkStep, 0, sizeof(checkStep));
    checkStep.mapX = 1;
    checkStep.mapY = 4;
    checkStep.dir = DIR_SOUTH;
    checkStep.expectedOrdinal = 3;
    checkStep.inputBeforeCheck = -1;
    checkStep.allowNoPortraitDominance = 0;
    checkStep.label = "hall_pointer_candidate_after_cancel_ordinal_3";
    if (!check_input_walk_step(game, portraits, -2, &checkStep, outFb)) {
        ok = 0;
    }

    printf("hall_pointer_candidate_move_select_cancel C040=%d/%d front=%d\n",
           panelMatch.assetDrawn, panelMatch.assetOpaque,
           M11_GameView_GetFrontMirrorOrdinal(game));
    return ok;
}

static int check_strafe_moved_mirror_candidate_cancel(M11_GameViewState* game,
                                                      const M11_AssetSlot* portraits,
                                                      const M11_AssetSlot* rrPanel,
                                                      unsigned char* outFb) {
    M11_GameInputResult result;
    PanelMatch panelMatch;
    InputWalkStep checkStep;
    int ok = 1;

    /* This route composes the accepted C004 lateral movement branch with
     * the C080 viewport champion-mirror click.  ReDMCSB DEFS.H:238-243
     * names C004/C006 as move-right/move-left, CLIKMENU.C F0366:256-269
     * maps the relative lateral step, MOVESENS.C:556 redraws from the new
     * pose, and CLIKVIEW.C F0377/F0372/F0275 then reaches REVIVE.C
     * F0280/F0282 for the C040 candidate panel. */
    start_independent_input_route(game, 0, 3, DIR_NORTH);
    result = M11_GameView_HandleInput(game, M12_MENU_INPUT_STRAFE_RIGHT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_right input result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }

    memset(&checkStep, 0, sizeof(checkStep));
    checkStep.mapX = 1;
    checkStep.mapY = 3;
    checkStep.dir = DIR_NORTH;
    checkStep.expectedOrdinal = 1;
    checkStep.inputBeforeCheck = -1;
    checkStep.allowNoPortraitDominance = 0;
    checkStep.label = "hall_strafe_candidate_after_right_ordinal_1";
    if (!check_input_walk_step(game, portraits, -1, &checkStep, outFb)) {
        ok = 0;
    }

    result = M11_GameView_HandlePointerButton(game,
                                              PROBE_FRONT_PORTRAIT_CLICK_X,
                                              PROBE_FRONT_PORTRAIT_CLICK_Y,
                                              M11_DM1_MOUSE_MASK_LEFT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_portrait_click result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }
    if (game->candidateMirrorPanelActive != 1 ||
        game->inventoryPanelActive != 1 ||
        game->candidateMirrorOrdinal != 1 ||
        game->candidateMirrorPartyIndex != 0 ||
        game->world.party.championCount != 1) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_open state panel=%d inventory=%d ordinal=%d partyIndex=%d champions=%d\n",
                game->candidateMirrorPanelActive,
                game->inventoryPanelActive,
                game->candidateMirrorOrdinal,
                game->candidateMirrorPartyIndex,
                game->world.party.championCount);
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    panelMatch = match_panel(rrPanel, outFb, 6);
    if (panelMatch.assetOpaque <= 0 ||
        panelMatch.assetDrawn * 100 < 90 * panelMatch.assetOpaque) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_open C040 missing drawn=%d/%d\n",
                panelMatch.assetDrawn, panelMatch.assetOpaque);
        ok = 0;
    }

    result = M11_GameView_HandlePointerButton(game,
                                              PROBE_CANCEL_CLICK_X,
                                              PROBE_CANCEL_CLICK_Y,
                                              M11_DM1_MOUSE_MASK_LEFT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_cancel result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }
    if (game->candidateMirrorPanelActive != 0 ||
        game->inventoryPanelActive != 0 ||
        game->candidateMirrorOrdinal != -1 ||
        game->candidateMirrorPartyIndex != -1 ||
        game->world.party.championCount != 0 ||
        M11_GameView_GetFrontMirrorOrdinal(game) != 1) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_cancel state panel=%d inventory=%d ordinal=%d partyIndex=%d champions=%d front=%d\n",
                game->candidateMirrorPanelActive,
                game->inventoryPanelActive,
                game->candidateMirrorOrdinal,
                game->candidateMirrorPartyIndex,
                game->world.party.championCount,
                M11_GameView_GetFrontMirrorOrdinal(game));
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    panelMatch = match_panel(rrPanel, outFb, 6);
    if (panelMatch.assetOpaque > 0 &&
        panelMatch.assetDrawn * 100 >= 40 * panelMatch.assetOpaque) {
        fprintf(stderr,
                "FAIL hall_strafe_candidate_cancel C040 leaked drawn=%d/%d\n",
                panelMatch.assetDrawn, panelMatch.assetOpaque);
        ok = 0;
    }
    memset(&checkStep, 0, sizeof(checkStep));
    checkStep.mapX = 1;
    checkStep.mapY = 3;
    checkStep.dir = DIR_NORTH;
    checkStep.expectedOrdinal = 1;
    checkStep.inputBeforeCheck = -1;
    checkStep.allowNoPortraitDominance = 0;
    checkStep.label = "hall_strafe_candidate_after_cancel_ordinal_1";
    if (!check_input_walk_step(game, portraits, -2, &checkStep, outFb)) {
        ok = 0;
    }

    printf("hall_strafe_candidate_move_select_cancel C040=%d/%d front=%d\n",
           panelMatch.assetDrawn, panelMatch.assetOpaque,
           M11_GameView_GetFrontMirrorOrdinal(game));
    return ok;
}

static int check_panel_live_movement_arrow_guard(M11_GameViewState* game,
                                                 const M11_AssetSlot* portraits,
                                                 const M11_AssetSlot* rrPanel,
                                                 unsigned char* outFb) {
    M11_GameInputResult result;
    PanelMatch panelMatch;
    InputWalkStep checkStep;
    int ok = 1;

    /* ReDMCSB COMMAND.C F0359 lines 1985-1990 dispatches M568/C040 clicks
     * only through the resurrect/reincarnate/cancel panel boxes.  Movement
     * arrows outside C160/C161/C162 must therefore be ignored while the
     * panel owns input, preserving the live Hall mirror pose and C040 over
     * the DUNVIEW.C:3913-3928 / 8522-8533 D1C portrait blit.  COMMAND.C
     * lines 109-113/396-402 anchor the upper C001/C003 turn/forward boxes
     * and the lower C006/C004 lateral movement boxes exercised here. */
    start_independent_input_route(game, 1, 4, DIR_SOUTH);
    memset(&checkStep, 0, sizeof(checkStep));
    checkStep.mapX = 1;
    checkStep.mapY = 4;
    checkStep.dir = DIR_SOUTH;
    checkStep.expectedOrdinal = 3;
    checkStep.inputBeforeCheck = -1;
    checkStep.allowNoPortraitDominance = 0;
    checkStep.label = "hall_panel_arrow_guard_start_ordinal_3";
    if (!check_input_walk_step(game, portraits, -2, &checkStep, outFb)) {
        ok = 0;
    }

    result = M11_GameView_HandlePointerButton(game,
                                              PROBE_FRONT_PORTRAIT_CLICK_X,
                                              PROBE_FRONT_PORTRAIT_CLICK_Y,
                                              M11_DM1_MOUSE_MASK_LEFT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_portrait_click result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }
    if (game->candidateMirrorPanelActive != 1 ||
        game->inventoryPanelActive != 1 ||
        game->candidateMirrorOrdinal != 3 ||
        game->candidateMirrorPartyIndex != 0 ||
        game->world.party.championCount != 1) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_open state panel=%d inventory=%d ordinal=%d partyIndex=%d champions=%d\n",
                game->candidateMirrorPanelActive,
                game->inventoryPanelActive,
                game->candidateMirrorOrdinal,
                game->candidateMirrorPartyIndex,
                game->world.party.championCount);
        ok = 0;
    }

    result = M11_GameView_HandlePointer(game, 304, 135, 1);
    if (result != M11_GAME_INPUT_IGNORED) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_right_arrow result=%d want=%d\n",
                result, M11_GAME_INPUT_IGNORED);
        ok = 0;
    }
    result = M11_GameView_HandlePointer(game, 276, 135, 1);
    if (result != M11_GAME_INPUT_IGNORED) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_forward_arrow result=%d want=%d\n",
                result, M11_GAME_INPUT_IGNORED);
        ok = 0;
    }
    result = M11_GameView_HandlePointer(game, 248, 157, 1);
    if (result != M11_GAME_INPUT_IGNORED) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_left_strafe_arrow result=%d want=%d\n",
                result, M11_GAME_INPUT_IGNORED);
        ok = 0;
    }
    result = M11_GameView_HandlePointer(game, 304, 157, 1);
    if (result != M11_GAME_INPUT_IGNORED) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_right_strafe_arrow result=%d want=%d\n",
                result, M11_GAME_INPUT_IGNORED);
        ok = 0;
    }
    if (game->world.party.mapX != 1 ||
        game->world.party.mapY != 4 ||
        game->world.party.direction != DIR_SOUTH ||
        game->candidateMirrorPanelActive != 1 ||
        game->inventoryPanelActive != 1 ||
        game->candidateMirrorOrdinal != 3 ||
        game->candidateMirrorPartyIndex != 0 ||
        game->world.party.championCount != 1 ||
        M11_GameView_GetFrontMirrorOrdinal(game) != 3) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_state pose=(%d,%d,%d) panel=%d inventory=%d ordinal=%d partyIndex=%d champions=%d front=%d\n",
                game->world.party.mapX,
                game->world.party.mapY,
                game->world.party.direction,
                game->candidateMirrorPanelActive,
                game->inventoryPanelActive,
                game->candidateMirrorOrdinal,
                game->candidateMirrorPartyIndex,
                game->world.party.championCount,
                M11_GameView_GetFrontMirrorOrdinal(game));
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    panelMatch = match_panel(rrPanel, outFb, 6);
    if (panelMatch.assetOpaque <= 0 ||
        panelMatch.assetDrawn * 100 < 90 * panelMatch.assetOpaque) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard C040 missing drawn=%d/%d\n",
                panelMatch.assetDrawn, panelMatch.assetOpaque);
        ok = 0;
    }

    result = M11_GameView_HandlePointerButton(game,
                                              PROBE_CANCEL_CLICK_X,
                                              PROBE_CANCEL_CLICK_Y,
                                              M11_DM1_MOUSE_MASK_LEFT);
    if (result != M11_GAME_INPUT_REDRAW) {
        fprintf(stderr,
                "FAIL hall_panel_arrow_guard_cancel result=%d want=%d\n",
                result, M11_GAME_INPUT_REDRAW);
        ok = 0;
    }
    memset(&checkStep, 0, sizeof(checkStep));
    checkStep.mapX = 1;
    checkStep.mapY = 4;
    checkStep.dir = DIR_SOUTH;
    checkStep.expectedOrdinal = 3;
    checkStep.inputBeforeCheck = -1;
    checkStep.allowNoPortraitDominance = 0;
    checkStep.label = "hall_panel_arrow_guard_after_cancel_ordinal_3";
    if (!check_input_walk_step(game, portraits, -2, &checkStep, outFb)) {
        ok = 0;
    }

    printf("hall_panel_live_movement_arrow_guard lateral_arrows=ignored C040=%d/%d front=%d\n",
           panelMatch.assetDrawn, panelMatch.assetOpaque,
           M11_GameView_GetFrontMirrorOrdinal(game));
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    const M11_AssetSlot* rrPanel;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    /* Forward walk through the Hall of Champions corridor: the party
     * faces NORTH throughout and steps east from (1,3) to (3,3) and
     * back.  The front mirror ordinals in the reference DM1 V1
     * DUNGEON.DAT at y=3 facing NORTH are 1 -> -1 (no-portrait
     * corridor wall) -> 19, and the back-step reverses the sequence.
     * DUNGEON.C:2573 / 2608-2612 anchor the ordinal-by-cell lookup;
     * DUNVIEW.C:3913-3928 / 8522-8533 anchor the D1C portrait blit
     * (G0289 index into the C026 portrait strip);
     * DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF and
     * MOVESENS.C:556 anchor the full-viewport re-blt after each
     * step. */
    const WalkStep steps[] = {
        {1, 3, 1,  "hall_walk_step_a_north_ordinal_1"},
        {2, 3, -1, "hall_walk_step_b_north_no_portrait"},
        {3, 3, 19, "hall_walk_step_c_north_ordinal_19"},
        {2, 3, -1, "hall_walk_step_d_north_no_portrait_again"},
        {1, 3, 1,  "hall_walk_step_e_north_back_to_ordinal_1"},
    };
    const InputWalkStep inputSteps[] = {
        {1, 3, DIR_SOUTH, -1, -1, 0,
         "hall_input_start_south_no_portrait"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_UP, 0,
         "hall_input_forward_south_ordinal_3"},
        {1, 4, DIR_WEST, -1, M12_MENU_INPUT_RIGHT, 0,
         "hall_input_turn_right_west_no_portrait"},
        {1, 4, DIR_NORTH, 2, M12_MENU_INPUT_RIGHT, 0,
         "hall_input_turn_right_north_ordinal_2"},
        {1, 4, DIR_WEST, -1, M12_MENU_INPUT_LEFT, 0,
         "hall_input_turn_left_west_no_portrait"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_LEFT, 0,
         "hall_input_turn_left_south_ordinal_3"},
    };
    const InputWalkStep backstepSteps[] = {
        {1, 4, DIR_SOUTH, 3, -1, 0,
         "hall_backstep_start_south_ordinal_3"},
        {1, 3, DIR_SOUTH, -1, M12_MENU_INPUT_DOWN, 1,
         "hall_backstep_south_no_portrait"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_UP, 0,
         "hall_backstep_forward_back_to_ordinal_3"},
    };
    const InputWalkStep blockedStrafeSteps[] = {
        {1, 4, DIR_SOUTH, 3, -1, 0,
         "hall_blocked_strafe_start_south_ordinal_3"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_STRAFE_LEFT, 0,
         "hall_blocked_strafe_left_keeps_ordinal_3"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_STRAFE_RIGHT, 0,
         "hall_blocked_strafe_right_keeps_ordinal_3"},
    };
    const PointerWalkStep pointerSteps[] = {
        {1, 3, DIR_SOUTH, -1, -1, -1,
         "hall_pointer_start_south_no_portrait"},
        {1, 4, DIR_SOUTH, 3, 276, 135,
         "hall_pointer_forward_south_ordinal_3"},
        {1, 3, DIR_SOUTH, -1, 276, 157,
         "hall_pointer_back_south_no_portrait"},
        {1, 4, DIR_SOUTH, 3, 276, 135,
         "hall_pointer_forward_back_to_ordinal_3"},
    };
    const PointerWalkStep pointerTurnSteps[] = {
        {1, 4, DIR_SOUTH, 3, -1, -1,
         "hall_pointer_turn_start_south_ordinal_3"},
        {1, 4, DIR_WEST, -1, 304, 135,
         "hall_pointer_turn_right_west_no_portrait"},
        {1, 4, DIR_NORTH, 2, 304, 135,
         "hall_pointer_turn_right_north_ordinal_2"},
        {1, 4, DIR_WEST, -1, 248, 135,
         "hall_pointer_turn_left_west_no_portrait"},
        {1, 4, DIR_SOUTH, 3, 248, 135,
         "hall_pointer_turn_left_south_ordinal_3"},
    };
    const LateralWalkStep strafeSteps[] = {
        {1, 3, DIR_NORTH, 1, 1, 3, 1,
         M12_MENU_INPUT_STRAFE_LEFT, -1, -1,
         "hall_strafe_keyboard_left_ordinal_1_blocked_stable"},
        {1, 3, DIR_NORTH, 1, 1, 3, 1,
         M12_MENU_INPUT_STRAFE_RIGHT, -1, -1,
         "hall_strafe_keyboard_right_ordinal_1_blocked_stable"},
        {0, 3, DIR_NORTH, -1, 1, 3, 1,
         M12_MENU_INPUT_STRAFE_RIGHT, -1, -1,
         "hall_strafe_keyboard_right_no_portrait_to_ordinal_1"},
        {2, 3, DIR_NORTH, -1, 1, 3, 1,
         M12_MENU_INPUT_STRAFE_LEFT, -1, -1,
         "hall_strafe_keyboard_left_no_portrait_to_ordinal_1"},
    };
    const LateralWalkStep pointerStrafeSteps[] = {
        {1, 3, DIR_NORTH, 1, 1, 3, 1,
         -1, 248, 157,
         "hall_pointer_strafe_left_ordinal_1_blocked_stable"},
        {1, 3, DIR_NORTH, 1, 1, 3, 1,
         -1, 304, 157,
         "hall_pointer_strafe_right_ordinal_1_blocked_stable"},
        {0, 3, DIR_NORTH, -1, 1, 3, 1,
         -1, 304, 157,
         "hall_pointer_strafe_right_no_portrait_to_ordinal_1"},
        {2, 3, DIR_NORTH, -1, 1, 3, 1,
         -1, 248, 157,
         "hall_pointer_strafe_left_no_portrait_to_ordinal_1"},
    };
    const MixedWalkStep mixedSteps[] = {
        {1, 4, DIR_SOUTH, 3, -1, -1, -1, 0,
         "hall_mixed_start_south_ordinal_3"},
        {1, 4, DIR_WEST, -1, -1, 304, 135, 0,
         "hall_mixed_pointer_right_west_no_portrait"},
        {1, 4, DIR_NORTH, 2, M12_MENU_INPUT_RIGHT, -1, -1, 0,
         "hall_mixed_keyboard_right_north_ordinal_2"},
        {1, 4, DIR_WEST, -1, -1, 248, 135, 0,
         "hall_mixed_pointer_left_west_no_portrait"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_LEFT, -1, -1, 0,
         "hall_mixed_keyboard_left_south_ordinal_3"},
        {1, 3, DIR_SOUTH, -1, -1, 276, 157, 1,
         "hall_mixed_pointer_back_south_no_portrait"},
        {1, 4, DIR_SOUTH, 3, M12_MENU_INPUT_UP, -1, -1, 0,
         "hall_mixed_keyboard_forward_south_ordinal_3"},
    };
    int stepIdx;
    int prevOrdinal = -2; /* sentinel: no prior ordinal */

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    rrPanel = M11_AssetLoader_Load(&game.assetLoader,
                                   (unsigned int)PROBE_RR_PANEL_GRAPHIC);
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels ||
        rrPanel->width <= 0 || rrPanel->height <= 0) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C040 resurrect/reincarnate panel unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    for (stepIdx = 0; stepIdx < (int)(sizeof(steps) / sizeof(steps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk = check_walk_step(&game, portraits, prevOrd,
                                     &steps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = steps[stepIdx].expectedOrdinal;
    }

    /* Runtime input route: drive the legal Hall corridor through
     * M11_GameView_HandleInput so the movement command queue and
     * source-locked tick boundary participate in the mirror/no-floating
     * pixel check.  The added left-turn tail proves the opposite
     * CLIKMENU.C F0365/F0366 rotation branch after a live right-turn
     * route has painted the same D1C rectangle.  Source anchors:
     * COMMAND.C F0359/F0361 command dispatch, CLIKMENU.C F0365/F0366
     * relative movement conversion, MOVESENS.C:556 viewport redraw
     * after accepted movement, and DUNVIEW.C:3913-3928 / 8522-8533
     * C026 D1C portrait blit. */
    start_independent_input_route(&game, 1, 3, DIR_SOUTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(inputSteps) / sizeof(inputSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        if (inputSteps[stepIdx].inputBeforeCheck >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandleInput(&game, inputSteps[stepIdx].inputBeforeCheck);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s input=%d result=%d want=%d\n",
                        inputSteps[stepIdx].label,
                        inputSteps[stepIdx].inputBeforeCheck,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &inputSteps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = inputSteps[stepIdx].expectedOrdinal;
    }

    /* Backstep route: keep the party facing SOUTH and use the source-backed
     * backward command to move from the live ordinal 3 mirror at (1,4) to
     * the no-portrait pose at (1,3), then forward back to ordinal 3.  This
     * source-locks the real input path for C005/C003 movement commands
     * through COMMAND.C F0359/F0361, CLIKMENU.C F0366 lines 224-233
     * relative movement mapping, MOVESENS.C:556 redraw timing, and the
     * DUNVIEW.C:3913-3928 / 8522-8533 C026 D1C portrait blit.
     * The (1,3,SOUTH) no-portrait wall pattern naturally resembles
     * ordinal 10 in this narrow box, matching the existing probe's
     * documented Hall wall-pattern caveat; this route therefore asserts
     * the no-floating contract through the prior-ordinal stale-pixel check
     * instead of the unrelated best-ordinal dominance threshold. */
    start_independent_input_route(&game, 1, 4, DIR_SOUTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(backstepSteps) / sizeof(backstepSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        if (backstepSteps[stepIdx].inputBeforeCheck >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandleInput(&game, backstepSteps[stepIdx].inputBeforeCheck);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s input=%d result=%d want=%d\n",
                        backstepSteps[stepIdx].label,
                        backstepSteps[stepIdx].inputBeforeCheck,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &backstepSteps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = backstepSteps[stepIdx].expectedOrdinal;
    }

    /* Blocked lateral-command route: keep the party facing SOUTH and issue
     * the source-backed C006/C004 movement pair through keyboard strafe inputs.
     * ReDMCSB DEFS.H:238-243 names C004/C006 as move-right/move-left, and
     * CLIKMENU.C F0366:224-233 maps the relative lateral step before the
     * MOVESENS.C:556 redraw.  In this Hall pose both lateral commands are
     * blocked, so this proves the DUNVIEW.C:3913-3928 / 8522-8533 C026 D1C
     * portrait rectangle survives a rejected lateral command without dropping
     * or drifting from ordinal 3. */
    start_independent_input_route(&game, 1, 4, DIR_SOUTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(blockedStrafeSteps) / sizeof(blockedStrafeSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        if (blockedStrafeSteps[stepIdx].inputBeforeCheck >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandleInput(&game, blockedStrafeSteps[stepIdx].inputBeforeCheck);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s input=%d result=%d want=%d\n",
                        blockedStrafeSteps[stepIdx].label,
                        blockedStrafeSteps[stepIdx].inputBeforeCheck,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &blockedStrafeSteps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = blockedStrafeSteps[stepIdx].expectedOrdinal;
    }

    /* Pointer route: click the original V1 movement-arrow boxes in the
     * right-side command panel instead of calling M11_GameView_HandleInput
     * directly.  COMMAND.C:109-113 / 396-402 define the forward/back boxes
     * and command ids; M11_GameView_HandlePointerButton resolves those boxes
     * through G0448-compatible routes before feeding C003/C005 into the same
     * CLIKMENU.C F0366 / MOVESENS.C:556 redraw path asserted above. */
    start_independent_input_route(&game, 1, 3, DIR_SOUTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(pointerSteps) / sizeof(pointerSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        InputWalkStep checkStep;
        if (pointerSteps[stepIdx].clickX >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandlePointer(&game,
                                           pointerSteps[stepIdx].clickX,
                                           pointerSteps[stepIdx].clickY,
                                           1);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s pointer=(%d,%d) result=%d want=%d\n",
                        pointerSteps[stepIdx].label,
                        pointerSteps[stepIdx].clickX,
                        pointerSteps[stepIdx].clickY,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        memset(&checkStep, 0, sizeof(checkStep));
        checkStep.mapX = pointerSteps[stepIdx].mapX;
        checkStep.mapY = pointerSteps[stepIdx].mapY;
        checkStep.dir = pointerSteps[stepIdx].dir;
        checkStep.expectedOrdinal = pointerSteps[stepIdx].expectedOrdinal;
        checkStep.inputBeforeCheck = -1;
        checkStep.allowNoPortraitDominance =
            pointerSteps[stepIdx].expectedOrdinal < 0 &&
            pointerSteps[stepIdx].mapX == 1 &&
            pointerSteps[stepIdx].mapY == 3 &&
            pointerSteps[stepIdx].dir == DIR_SOUTH;
        checkStep.label = pointerSteps[stepIdx].label;
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &checkStep, currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = pointerSteps[stepIdx].expectedOrdinal;
    }

    if (!check_pointer_moved_mirror_candidate_cancel(&game, portraits, rrPanel, currFb)) {
        ok = 0;
    }

    if (!check_strafe_moved_mirror_candidate_cancel(&game, portraits, rrPanel, currFb)) {
        ok = 0;
    }

    if (!check_panel_live_movement_arrow_guard(&game, portraits, rrPanel, currFb)) {
        ok = 0;
    }

    /* Pointer turn route: use the same G0448 movement list, but click the
     * source turn-left/turn-right boxes (COMMAND.C:109-111 / 396-398:
     * C001/C002 at x=234..261 and x=291..318, y=125..145).  This is
     * intentionally separate from the pointer forward/back route above:
     * it proves mouse-driven in-place turns clear the south-facing ordinal
     * 3 portrait when rotating to the west no-portrait pose and repaint
     * ordinal 2/3 when rotating back through NORTH/SOUTH. */
    start_independent_input_route(&game, 1, 4, DIR_SOUTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(pointerTurnSteps) / sizeof(pointerTurnSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        InputWalkStep checkStep;
        if (pointerTurnSteps[stepIdx].clickX >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandlePointer(&game,
                                           pointerTurnSteps[stepIdx].clickX,
                                           pointerTurnSteps[stepIdx].clickY,
                                           1);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s pointer=(%d,%d) result=%d want=%d\n",
                        pointerTurnSteps[stepIdx].label,
                        pointerTurnSteps[stepIdx].clickX,
                        pointerTurnSteps[stepIdx].clickY,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        memset(&checkStep, 0, sizeof(checkStep));
        checkStep.mapX = pointerTurnSteps[stepIdx].mapX;
        checkStep.mapY = pointerTurnSteps[stepIdx].mapY;
        checkStep.dir = pointerTurnSteps[stepIdx].dir;
        checkStep.expectedOrdinal = pointerTurnSteps[stepIdx].expectedOrdinal;
        checkStep.inputBeforeCheck = -1;
        checkStep.allowNoPortraitDominance = 0;
        checkStep.label = pointerTurnSteps[stepIdx].label;
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &checkStep, currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = pointerTurnSteps[stepIdx].expectedOrdinal;
    }

    /* Strafe route: drive C006/C004 lateral movement through the keyboard
     * route.  COMMAND.C:109-113 / 396-402 define the lower movement-arrow
     * row, CLIKMENU.C F0366:256-269 maps commands C003..C006 to
     * forward/right/back/left relative movement, and
     * DUNGEON.C F0150:1389-1391 applies those relative deltas without
     * changing party direction.  Each strafe is an independent one-command
     * interaction so the test isolates C004/C006 rendering from G0310
     * movement-cooldown cadence.  The accepted cases cover C004/C006 from
     * neighbouring no-portrait cells into ordinal 1; the blocked cases start
     * on ordinal 1 and prove lateral wall bumps keep the portrait stable
     * instead of leaving a partially cleared/floating box. */
    for (stepIdx = 0; stepIdx < (int)(sizeof(strafeSteps) / sizeof(strafeSteps[0])); ++stepIdx) {
        int prevOrd = strafeSteps[stepIdx].startOrdinal;
        int stepOk;
        InputWalkStep checkStep;
        M11_GameInputResult result;
        start_independent_input_route(&game,
                                      strafeSteps[stepIdx].startMapX,
                                      strafeSteps[stepIdx].startMapY,
                                      strafeSteps[stepIdx].startDir);
        result = M11_GameView_HandleInput(&game,
                                          strafeSteps[stepIdx].inputBeforeCheck);
        if (result != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr, "FAIL %s input=%d result=%d want=%d\n",
                    strafeSteps[stepIdx].label,
                    strafeSteps[stepIdx].inputBeforeCheck,
                    result, M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        memset(&checkStep, 0, sizeof(checkStep));
        checkStep.mapX = strafeSteps[stepIdx].expectedMapX;
        checkStep.mapY = strafeSteps[stepIdx].expectedMapY;
        checkStep.dir = strafeSteps[stepIdx].startDir;
        checkStep.expectedOrdinal = strafeSteps[stepIdx].expectedOrdinal;
        checkStep.inputBeforeCheck = -1;
        checkStep.allowNoPortraitDominance = 0;
        checkStep.label = strafeSteps[stepIdx].label;
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &checkStep, currFb);
        if (!stepOk) {
            ok = 0;
        }
    }

    /* Pointer strafe route: click the same lower movement-arrow row through
     * M11_GameView_HandlePointer.  It proves G0448 C073/C071 mouse hits
     * route to C006/C004 lateral movement before the D1C portrait rectangle
     * is rebuilt by the source-locked runtime draw path. */
    for (stepIdx = 0; stepIdx < (int)(sizeof(pointerStrafeSteps) / sizeof(pointerStrafeSteps[0])); ++stepIdx) {
        int prevOrd = pointerStrafeSteps[stepIdx].startOrdinal;
        int stepOk;
        InputWalkStep checkStep;
        M11_GameInputResult result;
        start_independent_input_route(&game,
                                      pointerStrafeSteps[stepIdx].startMapX,
                                      pointerStrafeSteps[stepIdx].startMapY,
                                      pointerStrafeSteps[stepIdx].startDir);
        result = M11_GameView_HandlePointer(&game,
                                            pointerStrafeSteps[stepIdx].clickX,
                                            pointerStrafeSteps[stepIdx].clickY,
                                            1);
        if (result != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr, "FAIL %s pointer=(%d,%d) result=%d want=%d\n",
                    pointerStrafeSteps[stepIdx].label,
                    pointerStrafeSteps[stepIdx].clickX,
                    pointerStrafeSteps[stepIdx].clickY,
                    result, M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
        memset(&checkStep, 0, sizeof(checkStep));
        checkStep.mapX = pointerStrafeSteps[stepIdx].expectedMapX;
        checkStep.mapY = pointerStrafeSteps[stepIdx].expectedMapY;
        checkStep.dir = pointerStrafeSteps[stepIdx].startDir;
        checkStep.expectedOrdinal = pointerStrafeSteps[stepIdx].expectedOrdinal;
        checkStep.inputBeforeCheck = -1;
        checkStep.allowNoPortraitDominance = 0;
        checkStep.label = pointerStrafeSteps[stepIdx].label;
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &checkStep, currFb);
        if (!stepOk) {
            ok = 0;
        }
    }

    /* Mixed pointer/keyboard route: alternate G0448 pointer clicks and
     * keyboard movement commands through one live movement-pipeline state.
     * This narrows coverage to the COMMAND.C F0359/F0361 interleave described
     * near BUG0_73 at lines 1478/1485, then the same F0365/F0366 movement
     * handlers and MOVESENS.C:556 redraw boundary.  The pixel assertions stay
     * on the DUNVIEW.C:3913-3928 / 8522-8533 C026 D1C portrait rectangle. */
    start_independent_input_route(&game, 1, 4, DIR_SOUTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(mixedSteps) / sizeof(mixedSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk;
        InputWalkStep checkStep;
        if (mixedSteps[stepIdx].clickX >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandlePointer(&game,
                                           mixedSteps[stepIdx].clickX,
                                           mixedSteps[stepIdx].clickY,
                                           1);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s pointer=(%d,%d) result=%d want=%d\n",
                        mixedSteps[stepIdx].label,
                        mixedSteps[stepIdx].clickX,
                        mixedSteps[stepIdx].clickY,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        if (mixedSteps[stepIdx].inputBeforeCheck >= 0) {
            M11_GameInputResult result =
                M11_GameView_HandleInput(&game,
                                         mixedSteps[stepIdx].inputBeforeCheck);
            if (result != M11_GAME_INPUT_REDRAW) {
                fprintf(stderr, "FAIL %s input=%d result=%d want=%d\n",
                        mixedSteps[stepIdx].label,
                        mixedSteps[stepIdx].inputBeforeCheck,
                        result, M11_GAME_INPUT_REDRAW);
                ok = 0;
            }
        }
        memset(&checkStep, 0, sizeof(checkStep));
        checkStep.mapX = mixedSteps[stepIdx].mapX;
        checkStep.mapY = mixedSteps[stepIdx].mapY;
        checkStep.dir = mixedSteps[stepIdx].dir;
        checkStep.expectedOrdinal = mixedSteps[stepIdx].expectedOrdinal;
        checkStep.inputBeforeCheck = -1;
        checkStep.allowNoPortraitDominance =
            mixedSteps[stepIdx].allowNoPortraitDominance;
        checkStep.label = mixedSteps[stepIdx].label;
        stepOk = check_input_walk_step(&game, portraits, prevOrd,
                                       &checkStep, currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = mixedSteps[stepIdx].expectedOrdinal;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror walk-path runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
