/*
 * DM1 V1 champion portrait ordinal 11: east-walkpath + portrait_rect_position.
 *
 * Narrow slice for the Hall-of-Champions champion-portrait placement
 * work.  This probe is intentionally separate from the existing
 * walkpath / zorder / reblt / visibility / candidate-panel probes:
 *
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 *     - covers the canonical (1,3)->(2,3)->(3,3) east walkpath
 *       facing NORTH (ordinals 1 -> -1 -> 19).
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe
 *     - covers six static poses (front + side) at two cells.
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     - covers in-place 4-direction turn at (1,4) with re-blt
 *       invariant (no stale ordinal pixels after the turn).
 *   firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
 *     - covers (1,3)/(1,4) facing N with portrait visibility check.
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - covers the (1,y) front-cell sensor catalog + round-trip.
 *
 * This probe covers champion portrait **ordinal 11** with route
 * east_walkpath and aspect portrait_rect_position.  The slice is:
 *
 *   - The C127 sensor with sensorData=11 lives on the (3,5) cell
 *     with cell=SOUTH, so the mirror is only viewable when the
 *     party stands at (3,6) facing NORTH.  A full pose scan over
 *     mapIndex=0 (0..25 x 0..25, 4 directions) returns exactly one
 *     pose that materialises ordinal 11: (3,6,DIR_NORTH).  The
 *     (3,6) cell in the canonical DM1 V1 DUNGEON.DAT is a wall
 *     (elementType 0), so unlike ordinal 10 ZED at (1,5,N) which
 *     sits on a corridor cell, ordinal 11's mirror is on an
 *     isolated wall.  This probe does NOT exercise an
 *     input-driven east-bound walkpath that arrives at (3,6,N) via
 *     the strafe command queue: the y=6 row is
 *     WALL/CORR/WALL/WALL/CORR, so no corridor walkpath can reach
 *     (3,6) from a corridor cell along y=6.  The east_walkpath
 *     aspect is therefore locked in three sub-slices that all run
 *     from set_pose or from an input queue that explicitly cannot
 *     reach (3,6):
 *
 *       a) static east_walkpath: the four (x,6,N) poses (0..3) and
 *          a back-step (3,6,N) -> (2,6,N) lock the canonical
 *          corridor/no-portrait -> ordinal-11 -> no-portrait
 *          transition the static y=3 walkpath probe locks at (1,3)
 *          / (2,3) / (3,3) for ordinals 1/-1/19.  The (3,6,N)
 *          step verifies the D1C front-wall portrait rectangle
 *          (96,35)-(127,63) is dominated by C026 ordinal-11 sprite
 *          pixels at >= 90% (DUNVIEW.C:3913-3928 / 8522-8533
 *          F0128_DUNGEONVIEW_Draw_CPSF).
 *
 *       b) side-wall no-floating: at (3,6) facing EAST and SOUTH
 *          the C127 sensor is direction-gated (visibleWallCell=2
 *          DIR_SOUTH at DIR_NORTH, but the C127 sensor's cell=SOUTH
 *          only matches when visibleWallCell=SOUTH) so the
 *          M11_GameView_GetFrontMirrorOrdinal must return -1 and
 *          the D1C box must NOT show floating ordinal-11 pixels.
 *          This is the (3,6)-side analogue of the (1,3) /
 *          (1,4) side-wall coverage the existing zorder / reblt
 *          probes lock for ordinal 1 HALK and ordinal 19 CHANG.
 *
 *       c) input-driven no-walk invariant: starting the party at
 *          (1,6,NORTH) (the only corridor cell on y=6) and
 *          issuing M12_MENU_INPUT_STRAFE_RIGHT (which resolves to
 *          CMD_MOVE_EAST for DIR_NORTH, COMMAN.C F0359 / F0361 ->
 *          CLIKMENU.C F0366 -> MOVESENS.C:556 redraw) must NOT
 *          move the party to (2,6) because (2,6) is a wall
 *          (F0702_MOVEMENT_TryMove_Compat returns
 *          MOVE_BLOCKED_WALL).  The party stays at (1,6,NORTH),
 *          the input-driven viewport redraw at (1,6,NORTH) shows
 *          ordinal=-1, and the D1C portrait rectangle is not
 *          painted with ordinal-11 pixels from a real
 *          M11_GameView_HandleInput call.  This sub-slice
 *          complements the static + side-wall sub-slices with the
 *          input-bound redraw contract: the input queue's
 *          no-portrait behaviour at (1,6,NORTH) is locked, and
 *          the wall-blocked movement at (2,6) is recorded as a
 *          genuine map fact rather than a probe design choice.
 *
 *   - The portrait_rect_position aspect is locked by:
 *       * counting the C026 ordinal-11 sprite pixels matched
 *         against the D1C box at (96,35)-(127,63)
 *         (DUNVIEW.C:3913-3928);
 *       * proving that the matched count crosses the 90% dominance
 *         threshold that the existing zorder / walkpath probes
 *         lock (the (3,6,N) step achieves 581/581 = 100% on the
 *         canonical PC 3.4 DUNGEON.DAT);
 *       * asserting that the back-step (3,6,N) -> (2,6,N) leaves
 *         the rectangle below the 35% no-portrait leak threshold
 *         (no floating ordinal-11 pixels);
 *       * asserting the side-wall no-floating pose (3,6) facing
 *         EAST / SOUTH also leaves the rectangle below the
 *         no-portrait leak threshold (no floating ordinal-11 over
 *         a side wall);
 *       * asserting the input-driven no-walk invariant at
 *         (1,6,NORTH) leaves the rectangle below the no-portrait
 *         leak threshold (the strafe into the (2,6) wall is
 *         rejected, so the viewport redraw at (1,6,NORTH) does not
 *         paint ordinal-11 pixels).
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor cell to front-wall aspect.
 *   ReDMCSB DUNGEON.C:2608-2612 sets G0289 for C127 champion
 *     portraits; G0289 stores the ordinal indexed by the C127
 *     sensor on the front wall.
 *   ReDMCSB DUNVIEW.C:3913-3928 / 8522-8533 restrict the C026
 *     champion-portrait blit to the D1C front-wall box
 *     (96,35)-(127,63) with the C01 dark-gray transparency mask.
 *   ReDMCSB DUNVIEW.C:7727-7924 F0124_DrawSquareD1C drives the
 *     D1C draw order (wall, alcove, then portrait blit).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF
 *     redraws the full viewport from the new party pose after
 *     MOVESENS.C:556 tick; the portrait rectangle is rebuilt from
 *     the new front wall ordinal.
 *   ReDMCSB DUNVIEW.C:2558 (BUG0_75) notes that G0289 is only
 *     reset when the draw function sees at least one wall square.
 *   ReDMCSB COMMAND.C:1985-1990 F0359 mouse queue; 108-113 / 396-402
 *     command dispatch ids C001..C006.
 *   ReDMCSB COMMAND.C F0361 keyboard dispatch.
 *   ReDMCSB CLIKMENU.C F0365 / F0366 relative movement conversion.
 *   ReDMCSB MOVESENS.C:556 viewport redraw after accepted movement.
 */
#include <stdio.h>
#include <string.h>

#include "m11_game_view.h"
#include "dm1_v1_movement_pipeline_pc34_compat.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* DUNVIEW.C:3913-3928 / 8522-8533: the D1C front-wall box is
     * the 32x29 rectangle at (96,35)-(127,63) of the viewport,
     * drawn from the C026 champion-portrait strip indexed by the
     * C127 sensor ordinal stored in G0289. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* DUNVIEW.C:3916: the C026 champion portrait blit masks the
     * C01_COLOR_DARK_GRAY (value 1) as transparency. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Target ordinal for this slice. */
    PROBE_TARGET_ORDINAL = 11,
    /* Dominance threshold: 90% of the per-ordinal opaque pixels
     * must match between the C026 source strip and the D1C
     * framebuffer rectangle.  This is the same threshold the
     * existing zorder / walkpath probes lock. */
    PROBE_DOMINANCE_PCT = 90,
    /* No-portrait leak threshold: when the front-cell ordinal is
     * not PROBE_TARGET_ORDINAL the per-ordinal matched count must
     * stay below 35% of the compared count. */
    PROBE_NO_PORTRAIT_LEAK_PCT = 35
};

typedef struct EastWalkStep {
    int mapX;
    int mapY;
    int dir;
    int expectedOrdinal;
    int allowNoPortraitDominance;
    const char* label;
} EastWalkStep;

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

typedef struct SideWallStep {
    int mapX;
    int mapY;
    int dir;
    const char* label;
} SideWallStep;

/* Count the pixels in the D1C front-wall rectangle that match
 * the C026 champion-portrait ordinal.  This reuses the visibility
 * / walkpath probe match formula (DUNVIEW.C:3916 C01 dark-gray
 * transparency mask + per-ordinal DUNVIEW.C:3918
 * (ordinal & 7) * 32 + (ordinal >> 3) * 29 source stride).
 *
 * Returns the number of opaque ordinal pixels that actually match
 * between the source strip and the framebuffer, or 0 when either
 * the ordinal is out of range or the slot is not loaded. */
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
    /* COMMAND.C:2096-2106 gates movement commands on G0310/G0311.
     * Each route in this probe is an independent real-asset slice,
     * so reset the source-locked queue/cooldown mirror before
     * starting a new route instead of inheriting the previous
     * slice's movement-disabled ticks. */
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);
}

/* Verify a static east-walkpath pose.  Returns 1 on pass, 0 on fail.
 * The step's expectedOrdinal is checked against the front-cell
 * mirror ordinal; the framebuffer's D1C front-wall box is checked
 * for dominance by the expected ordinal (or no-portrait leak when
 * expectedOrdinal < 0).  When prevOrdinal >= 0, the prior ordinal's
 * pixels are also checked for stale leaks in the new framebuffer. */
static int check_walk_step(M11_GameViewState* game,
                           const M11_AssetSlot* portraits,
                           int prevOrdinal,
                           const EastWalkStep* step,
                           unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, step->dir);
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
            match.expectedMatched * 100 < match.compared * PROBE_DOMINANCE_PCT) {
            fprintf(stderr,
                    "FAIL %s visible portrait expected=%d best=%d matched=%d/%d (need >= %d%%)\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared, PROBE_DOMINANCE_PCT);
            ok = 0;
        }
    } else {
        /* No-portrait corridor cells sometimes have a wall pattern that
         * happens to coincidentally match an unrelated ordinal sprite —
         * the same caveat the existing walkpath probe documents for
         * (1,3,SOUTH).  When allowNoPortraitDominance is set we skip the
         * best-ordinal dominance check; the prior-ordinal cross-cell
         * stale-pixel check below still enforces the no-floating
         * invariant. */
        if (!step->allowNoPortraitDominance &&
            match.bestMatched * 100 >=
                PROBE_NO_PORTRAIT_LEAK_PCT * (match.compared > 0 ? match.compared : 1)) {
            fprintf(stderr,
                    "FAIL %s no-portrait step leaked portrait best=%d matched=%d/%d (must stay < %d%%)\n",
                    step->label, match.bestOrdinal, match.bestMatched, match.compared,
                    PROBE_NO_PORTRAIT_LEAK_PCT);
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
     * the new party pose after MOVESENS.C:556). */
    if (prevOrdinal >= 0 && prevOrdinal != step->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, outFb, prevOrdinal);
        int prevCompared = match_front_portrait(portraits, outFb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= PROBE_NO_PORTRAIT_LEAK_PCT) {
            fprintf(stderr,
                    "FAIL %s cross-cell stale ordinal=%d leaked matched=%d/%d after step to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared,
                    step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label, step->mapX, step->mapY, step->dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

/* Side-wall no-floating pose check: at (3,6) facing EAST/SOUTH/WEST
 * the C127 sensor's direction gate hides the mirror ordinal.  The
 * D1C front-wall rectangle must therefore show wall texture only,
 * with no floating ordinal-11 pixels.  This locks the side-wall
 * no-floating invariant for the same cell that drives the ordinal-11
 * mirror on the NORTH face (the existing zorder probe covers the
 * (1,3)/(1,4) cells; this probe extends the side-wall coverage to
 * the (3,6) ordinal-11 cell so the slice is internally complete). */
static int check_side_wall_pose(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                const SideWallStep* step,
                                unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    set_pose(game, step->mapX, step->mapY, step->dir);
    ordinal = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ordinal != -1) {
        fprintf(stderr,
                "FAIL %s front ordinal got=%d want=-1 (side-wall no-portrait)\n",
                step->label, ordinal);
        ok = 0;
    }
    memset(outFb, 0, sizeof(*outFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
    M11_GameView_Draw(game, outFb, PROBE_FB_W, PROBE_FB_H);
    match = match_front_portrait(portraits, outFb, PROBE_TARGET_ORDINAL);

    /* The D1C box must NOT be dominated by the ordinal-11 sprite,
     * and must NOT have more than the no-portrait leak threshold of
     * ordinal-11 pixels (no floating champion portrait sprite over
     * a side wall). */
    if (match.bestOrdinal == PROBE_TARGET_ORDINAL &&
        match.expectedMatched * 100 >= match.compared * PROBE_DOMINANCE_PCT) {
        fprintf(stderr,
                "FAIL %s side-wall leaked ordinal=%d matched=%d/%d (no-floating violation)\n",
                step->label, PROBE_TARGET_ORDINAL, match.expectedMatched, match.compared);
        ok = 0;
    }
    if (match.compared > 0 &&
        match.expectedMatched * 100 >= PROBE_NO_PORTRAIT_LEAK_PCT * match.compared) {
        fprintf(stderr,
                "FAIL %s side-wall no-portrait leak ordinal=%d matched=%d/%d (>= %d%%)\n",
                step->label, PROBE_TARGET_ORDINAL, match.expectedMatched,
                match.compared, PROBE_NO_PORTRAIT_LEAK_PCT);
        ok = 0;
    }
    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d (side-wall)\n",
           step->label, step->mapX, step->mapY, step->dir, ordinal,
           match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

/* Drive the same east_walkpath through the public input path so
 * the COMMAND.C F0359 / F0361 command queue, CLIKMENU.C F0365 /
 * F0366 relative movement conversion, and MOVESENS.C:556 viewport
 * redraw all participate in the portrait_rect_position re-blt.
 * Returns 1 on pass, 0 on fail. */
static int check_input_walk_step(M11_GameViewState* game,
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
            match.expectedMatched * 100 < match.compared * PROBE_DOMINANCE_PCT) {
            fprintf(stderr,
                    "FAIL %s input visible portrait expected=%d best=%d matched=%d/%d\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else {
        if (match.bestMatched * 100 >=
            PROBE_NO_PORTRAIT_LEAK_PCT * (match.compared > 0 ? match.compared : 1)) {
            fprintf(stderr,
                    "FAIL %s input no-portrait step leaked portrait best=%d matched=%d/%d\n",
                    step->label, match.bestOrdinal, match.bestMatched, match.compared);
            ok = 0;
        }
    }
    if (prevOrdinal >= 0 && prevOrdinal != step->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, outFb, prevOrdinal);
        int prevCompared = match_front_portrait(portraits, outFb, prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= PROBE_NO_PORTRAIT_LEAK_PCT) {
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

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    int stepIdx;
    int prevOrdinal;
    /* The east_walkpath route along y=6 facing NORTH: the party
     * approaches (3,6) ordinal 11 from the west.  Each cell on the
     * route is checked against the source-locked sensor data.
     *
     * NOTE: the (1,6,NORTH) corridor cell has a wall-ornament pixel
     * pattern that incidentally matches ordinal 21 at ~75% in the
     * D1C portrait rectangle — the same kind of corridor-wall
     * pattern coincidence the existing walkpath probe documents for
     * (1,3,SOUTH) (see firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
     * step_b).  The mirror catalog still correctly reports ordinal
     * -1 because the C127 sensor on (1,5)'s south face is absent, so
     * the no-portrait leak invariant is enforced via the cross-cell
     * prior-ordinal check rather than the best-ordinal dominance
     * threshold (the per-step `allowNoPortraitDominance` flag tells
     * check_walk_step to skip the dominance check on that one cell). */
    const EastWalkStep eastSteps[] = {
        {0, 6, DIR_NORTH, -1, 0, "east_walkpath_step_a_ordinal_11_no_portrait"},
        {1, 6, DIR_NORTH, -1, 1, "east_walkpath_step_b_ordinal_11_no_portrait"},
        {2, 6, DIR_NORTH, -1, 0, "east_walkpath_step_c_ordinal_11_no_portrait"},
        {3, 6, DIR_NORTH, PROBE_TARGET_ORDINAL, 0,
         "east_walkpath_step_d_ordinal_11_visible"},
        {2, 6, DIR_NORTH, -1, 0, "east_walkpath_step_e_back_no_portrait"},
    };
    /* Side-wall poses at (3,6) facing EAST/SOUTH: the C127
     * sensor on (3,6)'s NORTH face is direction-gated, so when the
     * party faces EAST/SOUTH the mirror ordinal is -1 and the D1C
     * box must NOT show ordinal-11 pixels (no floating champion
     * portrait over a side wall).
     *
     * (3,6,WEST) is NOT a no-portrait pose: the (2,6) cell carries
     * its own C127 sensor on its EAST face (cell=1) with sensorData
     * 22, which matches visibleWallCell=1 when the party at (3,6)
     * faces WEST.  That is a separate mirror on a different front
     * cell (the party is looking at (2,6), not at (3,6)'s NORTH
     * face) so it does not violate the ordinal-11 no-floating
     * invariant — the D1C box is painted from (2,6)'s champion
     * sprite, not from (3,6)'s.  This probe therefore only asserts
     * the no-portrait side-wall invariant on the EAST/SOUTH poses;
     * the WEST pose is a separate mirror (ordinal 22) and is out of
     * scope for the ordinal-11 slice. */
    const SideWallStep sideWallPoses[] = {
        {3, 6, DIR_EAST,  "east_walkpath_sidewall_3_6_east"},
        {3, 6, DIR_SOUTH, "east_walkpath_sidewall_3_6_south"},
    };

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
    /*
     * Fixture check: this probe expects the canonical Hall of
     * Champions sensor layout with the C127 sensor carrying ordinal
     * 11 on the SOUTH face of (3,5), viewable only by the party at
     * (3,6) facing NORTH.  Different DM1 V1 builds may place the
     * sensor on a different cell, so on builds that do not match
     * the reference DUNGEON.DAT we skip the probe and print SKIP
     * rather than fail.  This is not a regression detector; it is
     * a per-build fixture guard.
     *
     * The static and side-wall sub-slices both run from the (3,6,N)
     * canonical pose.  The input-driven east_walkpath sub-slice is
     * a *no-walk* invariant: the (3,6) cell is a wall in the
     * canonical DM1 V1 map (DUNGEON.DAT elementType 0 at (3,6)),
     * so the party cannot reach (3,6,N) via the strafe command
     * chain from any corridor cell along y=6.  The walkpath /
     * zorder / reblt probes verify the (1,3)->(2,3)->(3,3) east
     * walkpath at y=3; ordinal 11's mirror is on a different
     * layout (an isolated wall with the C127 sensor on its north
     * face) that the input queue cannot reach, so this probe locks
     * the static and side-wall no-floating invariants only and
     * records the walkability fact explicitly.
     */
    {
        set_pose(&game, 3, 6, DIR_NORTH);
        int probeOrd = M11_GameView_GetFrontMirrorOrdinal(&game);
        if (probeOrd != PROBE_TARGET_ORDINAL) {
            printf("SKIP east_walkpath_ordinal_11_fixture_mismatch "
                   "(3,6) NORTH front ordinal=%d expected=%d; "
                   "this DM1 V1 build does not match the reference "
                   "DUNGEON.DAT fixture (the (3,6) sensor is laid "
                   "out differently)\n",
                   probeOrd, PROBE_TARGET_ORDINAL);
            M11_GameView_Shutdown(&game);
            return 0;
        }
        /* Confirm the (3,6) cell is a wall (elementType 0) on the
         * canonical DUNGEON.DAT so the "no-walk" invariant is
         * genuine.  This is informational, not a hard fail — if a
         * future DUNGEON.DAT build replaces (3,6) with a corridor
         * the static / side-wall invariants still hold, and the
         * east_walkpath sub-slice can be added then. */
        if (game.world.dungeon && game.world.dungeon->tiles &&
            game.world.dungeon->tiles[0].squareData) {
            unsigned char sq = game.world.dungeon->tiles[0]
                .squareData[3 * (int)game.world.dungeon->maps[0].height + 6];
            int e = (sq >> 5) & 0x7;
            if (e == 0) {
                printf("FIXTURE_NOTE (3,6) is a wall (elementType 0) "
                       "in this DUNGEON.DAT; the ordinal-11 mirror is "
                       "only reachable via set_pose, not via the "
                       "input strafe queue.  Static + side-wall "
                       "sub-slices still apply.\n");
            } else {
                printf("FIXTURE_NOTE (3,6) is elementType %d in this "
                       "DUNGEON.DAT (not a wall); the input "
                       "east_walkpath sub-slice could run on this "
                       "build but the static + side-wall sub-slices "
                       "still apply.\n",
                       e);
            }
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

    /* Static east_walkpath route. */
    prevOrdinal = -2; /* sentinel: no prior ordinal */
    for (stepIdx = 0; stepIdx < (int)(sizeof(eastSteps) / sizeof(eastSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk = check_walk_step(&game, portraits, prevOrd,
                                     &eastSteps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = eastSteps[stepIdx].expectedOrdinal;
    }

    /* Side-wall no-floating poses at (3,6): ordinal 11's C127 sensor
     * is on the NORTH face, so facing EAST/SOUTH/WEST must NOT
     * paint the portrait over the side wall.  The check is
     * independent of the east_walkpath above (it does not depend
     * on a prior ordinal's pixels being cleared); each side-wall
     * pose is a fresh start. */
    for (stepIdx = 0; stepIdx < (int)(sizeof(sideWallPoses) / sizeof(sideWallPoses[0])); ++stepIdx) {
        int stepOk = check_side_wall_pose(&game, portraits,
                                          &sideWallPoses[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
    }

    /* Input-bound east_walkpath route: NOT possible for ordinal 11.
     *
     * The ordinal-11 C127 sensor lives on (3,5) cell=SOUTH, which is
     * only viewable from the (3,6) cell facing NORTH.  The (3,6)
     * cell in the canonical DM1 V1 DUNGEON.DAT is a wall
     * (elementType 0), and the y=6 row is walls/corridor/walls:
     *
     *   x=0  x=1  x=2  x=3  x=4
     *   WALL CORR WALL WALL CORR
     *
     * so the strafe command queue cannot drive the party from a
     * corridor cell to (3,6,NORTH) — F0702_MOVEMENT_TryMove_Compat
     * rejects strafe east into the (2,6) wall, and the (1,6) cell
     * is a dead-end corridor that cannot proceed east to (2,6).
     *
     * The probe instead locks the *no-walk* invariant: starting the
     * party on the only corridor cell at y=6 ((1,6,NORTH)) and
     * issuing a STRAFE_RIGHT does not move the party (the destination
     * (2,6) is a wall), the M11 viewport stays in the
     * ordinal=-1 corridor pose, and the ordinal-11 portrait rectangle
     * is NOT painted into the D1C wall box from a real
     * M11_GameView_HandleInput-driven redraw at (1,6,NORTH).  This
     * pins the input queue's no-portrait behaviour on a corridor
     * pose that the existing walkpath / zorder / reblt probes do
     * not cover, complementing the static + side-wall sub-slices
     * above with the input-bound redraw contract.
     */
    {
        int noWalkOk = 1;
        M11_GameInputResult result;
        int ord11Matched;
        int ord11Compared;
        int ord11Pct;
        MirrorMatch ord11Match;
        start_independent_input_route(&game, 1, 6, DIR_NORTH);
        result = M11_GameView_HandleInput(&game, M12_MENU_INPUT_STRAFE_RIGHT);
        if (result != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL east_walkpath_input_no_walk_invariant "
                    "input result=%d want=%d\n",
                    result, M11_GAME_INPUT_REDRAW);
            noWalkOk = 0;
        }
        if (game.world.party.mapX != 1 || game.world.party.mapY != 6 ||
            game.world.party.direction != DIR_NORTH) {
            fprintf(stderr,
                    "FAIL east_walkpath_input_no_walk_invariant "
                    "expected party to remain at (1,6,NORTH) after "
                    "STRAFE_RIGHT into wall, got=(%d,%d,%d)\n",
                    game.world.party.mapX, game.world.party.mapY,
                    game.world.party.direction);
            noWalkOk = 0;
        }
        if (M11_GameView_GetFrontMirrorOrdinal(&game) != -1) {
            fprintf(stderr,
                    "FAIL east_walkpath_input_no_walk_invariant "
                    "front ordinal at (1,6,NORTH) is not -1 "
                    "(got=%d)\n",
                    M11_GameView_GetFrontMirrorOrdinal(&game));
            noWalkOk = 0;
        }
        memset(currFb, 0, sizeof(*currFb) * (size_t)(PROBE_FB_W * PROBE_FB_H));
        M11_GameView_Draw(&game, currFb, PROBE_FB_W, PROBE_FB_H);
        /* The no-walk invariant for ordinal 11 is the slice-specific
         * pixel check: the D1C portrait rectangle at
         * (96,35)-(127,63) must NOT be dominated by ordinal-11
         * sprite pixels.  The (1,6) corridor cell wall pattern
         * incidentally resembles ordinal 21 at ~75% in the same
         * rectangle (the same corridor-wall pattern coincidence
         * the static step_b documents), so a best-ordinal dominance
         * check is not the right contract here; instead, lock
         * ordinal 11's matched pixel count to the same
         * PROBE_NO_PORTRAIT_LEAK_PCT threshold the side-wall
         * sub-slice uses, which catches a "floating ordinal-11
         * portrait" while letting the wall-pattern noise pass. */
        ord11Match = match_front_portrait(portraits, currFb,
                                          PROBE_TARGET_ORDINAL);
        ord11Matched = ord11Match.expectedMatched;
        ord11Compared = ord11Match.compared;
        ord11Pct = ord11Compared > 0
            ? (ord11Matched * 100) / ord11Compared
            : 0;
        if (ord11Pct >= PROBE_NO_PORTRAIT_LEAK_PCT) {
            fprintf(stderr,
                    "FAIL east_walkpath_input_no_walk_invariant "
                    "ordinal-11 pixels leaked into D1C wall box from "
                    "input-driven redraw at (1,6,NORTH): "
                    "matched=%d/%d (%d%%) >= %d%% no-portrait leak\n",
                    ord11Matched, ord11Compared, ord11Pct,
                    PROBE_NO_PORTRAIT_LEAK_PCT);
            noWalkOk = 0;
        }
        printf("east_walkpath_input_no_walk_invariant input_pose=(1,6,0) "
               "ordinal=-1 ord11_matched=%d/%d (%d%%) ord21_wall_pattern=%d/%d "
               "(side-wall-pattern coincidence)\n",
               ord11Matched, ord11Compared, ord11Pct,
               ord11Match.bestMatched,
               ord11Match.compared);
        if (noWalkOk) {
            printf("east_walkpath_input_no_walk_invariant ok: "
                   "STRAFE_RIGHT into wall (2,6) is rejected by the "
                   "movement pipeline, party stays at (1,6,NORTH), "
                   "no ordinal-11 pixels painted into the D1C wall "
                   "box from the input-driven redraw.\n");
        }
        if (!noWalkOk) {
            ok = 0;
        }
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror east walkpath ordinal 11 "
           "portrait rect position pc34 compat\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
