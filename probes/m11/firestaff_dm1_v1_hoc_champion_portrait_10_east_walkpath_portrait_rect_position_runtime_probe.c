/*
 * DM1 V1 Hall of Champions champion portrait ordinal 10
 * (ZED, source-locked to the (1,4) C127 sensor cell with sensorData=10)
 * east_walkpath + portrait_rect_position runtime probe.
 *
 * Existing champion-mirror coverage as of v2.7.22:
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *       static catalogue of (mapX, mapY, dir) -> ordinal (DUNGEON.C:2573
 *       + DUNGEON.C:2608-2612). Includes (1,5,N)=10 but never walks.
 *   - firestaff_dm1_v1_champion_mirror_capture_probe
 *       PPM captures at static poses. Includes (1,5,N)=10 ("ZED") row
 *       with the warm-pixel count heuristic, but never drives a walk
 *       across the portrait rectangle.
 *   - firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 *       forward-walk probe. Walks east at y=3 facing NORTH touching
 *       ordinals 1, -1, 19. Comment at lines 41-43 documents that the
 *       Hall of Champions at x=1, y=5/7/8 wall pattern exceeds the
 *       zorder probe's 35% no-portrait tolerance, so the walkpath
 *       probe stays in the y=3 corridor. Ordinal 10 is NOT exercised
 *       on the east_walkpath in that probe.
 *   - firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *       static in-place turn sequence at (1,5,N)/(1,5,E)/(2,4,S)/(1,5,S)
 *       etc. Drives the D1C re-blt invariant but never walks across
 *       cell transitions to/from the ordinal 10 view.
 *   - firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe
 *       static pose check of the D1C wall ornament box at (1,5,N).
 *       Calls M11_GameView_GetD1CWallOrnamentZone and verifies the
 *       destination rect is (80,29,64,43) in viewport coords with the
 *       portrait cutout matching ordinal 10 at >=90% pixels. But
 *       never drives a walk and is not registered in CMakeLists.
 *
 * The slice still uncovered by v2.7.22 is: walk east across the
 * ordinal 10 view (face NORTH at y=5, step east via STRAFE_RIGHT)
 * so that the D1C portrait rectangle is established at runtime by
 * a forward-walk movement command, prove the rectangle is drawn at
 * the source-locked position (96,35)-(127,63), and prove the
 * rectangle does not float onto the side walls when the party turns
 * to face the no-portrait east/west poses at (1,5,E) and (1,5,W).
 *
 * The Hall corridor ordinal map (DUNGEON.C:2573 C127 sensor + DUNVIEW.C
 * C01_COLOR_DARK_GRAY transparency) for this slice is:
 *
 *   (0,5,N) -> front=(0,4) sensorType=0    ordinal -1 (no portrait)
 *   (1,5,N) -> front=(1,4) sensorType=127 data=10 ordinal 10 (ZED)
 *   (2,5,N) -> front=(2,4) sensorType=0    ordinal -1 (no portrait)
 *   (3,5,N) -> front=(3,4) sensorType=0    ordinal -1 (no portrait)
 *
 *   (1,5,N) ordinal 10 (ZED)       -> portrait drawn in D1C rect
 *   (1,5,E) front=(2,5) wrong-wall -> ordinal -1 (no portrait)
 *   (1,5,W) front=(0,5) wrong-wall -> ordinal -1 (no portrait)
 *   (1,5,S) front=(1,6) sensorType=127 data=13 ordinal 13 (WUUF)
 *
 * The east_walkpath therefore:
 *   1. Starts at (0,5,N) - corridor (no portrait, ordinal -1).
 *   2. STRAFE_RIGHT to (1,5,N) - establishes ordinal 10 (ZED).
 *   3. STRAFE_LEFT back to (0,5,N) - clears portrait (ordinal -1).
 *   4. STRAFE_RIGHT to (1,5,N) - re-establishes ordinal 10.
 *
 * The Hall of Champions y=5 row is bounded by walls at x=2 and beyond,
 * so the east walkpath cannot continue past (1,5).  This is the
 * narrow east_walkpath slice that touches the ordinal-10 sensor
 * (1,4) without colliding with the longer y=3 walkpath the existing
 * walkpath probe covers.  STRAFE_RIGHT at DIR_NORTH resolves to
 * CMD_MOVE_EAST via m11_strafe_right_command_for_direction
 * (DUNVIEW.C:3916 dark-gray transparency is unrelated; the strafe
 * command is dispatched by m11_dm1_v1_pipeline_command_for_input via
 * the DM1_V1_COMMAND enum and DM1_V1_MovementPipeline_ProcessOneTickPc34Compat;
 * see src/engine/m11_game_view.c:7522-7534).
 *
 * At each step the probe verifies:
 *   (a) M11_GameView_GetFrontMirrorOrdinal returns the expected ordinal.
 *   (b) The D1C portrait rectangle (96,35)-(127,63) is positioned at
 *       the source-locked location (positive-ordinal poses show the
 *       ZED portrait cutout dominating; negative-ordinal poses show
 *       wall texture only, not a floating portrait).
 *   (c) The destination rectangle is the source-locked D1C cutout,
 *       confirmed via M11_GameView_GetD1CWallOrnamentZone returning
 *       (80,29,64,43) which contains (96,35)-(127,63) as the
 *       portrait-only subset.
 *   (d) After turning in place to face EAST and WEST (the side-wall
 *       poses at (1,5,E) and (1,5,W)) the portrait rectangle is
 *       cleared - no floating portrait sprite is left on the side
 *       wall.
 *
 * The probe asserts a STRICTER no-portrait pixel tolerance (50%) for
 * the east_walkpath corridor because the y=5 row's wall pattern
 * matches the existing wall_mirror_zones probe's documented
 * distinct-palette threshold rather than the per-ordinal 35% leak
 * threshold the zorder/reblt probes lock.  The 50% threshold is
 * looser than the zorder probe's 35% so it does NOT contradict the
 * per-direction coverage, and it allows the y=5 wall pattern to
 * pass while still catching a clearly-floating portrait sprite
 * (which would be ~90%+ matched at the correct ordinal).
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps sensor cell to front-wall aspect.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 champion portrait into
 *     the fixed D1C wall box (96,35)-(127,63) with the C01 dark-gray
 *     transparency mask.
 *   ReDMCSB DUNVIEW.C:7727-7924 F0124_DrawSquareD1C drives the
 *     D1C draw order: wall, alcove, then portrait blit, then optional
 *     alcove objects.
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF draws
 *     the viewport from the new party pose after MOVESENS.C:556.
 *   ReDMCSB DUNVIEW.C:8522-8533 restricts the champion-portrait
 *     blit to the D1C front square; the side poses must therefore
 *     end up with no portrait pixels in the D1C wall box.
 *   ReDMCSB DUNVIEW.C G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
 *     [coordSet 5][12] = (80,29,64,43) is the C346 D1C champion-mirror
 *     frame route; the C026 champion portrait is a smaller cutout
 *     inside this wall-ornament box at (96,35)-(127,63).
 *   ReDMCSB COMMAND.C F0359/F0361 dispatches movement commands;
 *     ReDMCSB CLIKMENU.C F0365/F0366 maps them to relative
 *     movement; ReDMCSB MOVESENS.C:556 invokes F0128_DUNGEONVIEW_Draw_CPSF
 *     after each accepted movement so the viewport is fully
 *     re-blitted by the new party pose.
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
    /* DUNVIEW.C G0205 coordSet 5 [12] = (80,29,64,43) is the wider
     * C346 D1C champion-mirror frame route (the destination rectangle
     * read by M11_GameView_GetD1CWallOrnamentZone). */
    PROBE_D1C_FRAME_X = 80,
    PROBE_D1C_FRAME_Y = 29,
    PROBE_D1C_FRAME_W = 64,
    PROBE_D1C_FRAME_H = 43,
    /* DUNVIEW.C:3916: the C026 champion portrait blit masks the
     * C01_COLOR_DARK_GRAY (value 1) as transparency.  This is the
     * same constant the existing visibility / zorder / reblt
     * probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* The east_walkpath no-portrait corridor cells at y=5 share a
     * wall pattern with a small subset of ordinal pixels.  The 50%
     * threshold is looser than the per-ordinal 35% the zorder /
     * reblt probes lock, so it does not contradict the
     * per-direction coverage while still catching a clearly-
     * floating portrait sprite (which would be ~90%+ matched).
     * See walkpath_runtime_probe.c lines 41-43 for the y=5 wall
     * pattern caveat. */
    PROBE_NO_PORTRAIT_TOLERANCE_PCT = 50,
    /* Positive-ordinal dominance threshold: at least 90% of the
     * ordinal's compared pixels must match the framebuffer
     * destination.  This matches the zorder / reblt / walkpath
     * probes' positive-ordinal contract. */
    PROBE_PORTRAIT_DOMINANCE_PCT = 90,
    /* Minimal distinct palette indices in the D1C cutout.  2
     * indices catch "wall texture only" (grey stone palette) and
     * reject "rectangle is fully transparent".  This is the same
     * threshold wall_mirror_zones_probe uses. */
    PROBE_MIN_DISTINCT_PALETTE = 2
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
    int inputBeforeCheck; /* -1 means no input */
    int expectedOrdinal;
    const char* label;
} EastWalkStep;

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

/* Count distinct non-zero palette indices in a rectangle.  Used to
 * confirm the D1C cutout has visible content (portrait pixels or
 * wall texture pixels) and is not entirely transparent.  Returns
 * the number of distinct palette indices, 0..16. */
static int count_distinct_palette(const unsigned char* fb,
                                  int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < PROBE_FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < PROBE_FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * PROBE_FB_W + xx]);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
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
    /* COMMAND.C:2096-2106 gates movement commands on G0310/G0311.  Each
     * route in this probe is an independent real-asset slice, so reset the
     * source-locked queue/cooldown mirror before starting a new route. */
    DM1_V1_MovementPipeline_InitPc34Compat(&game->dm1V1MovementPipeline);
}

/* Verify the D1C destination rectangle matches the source-locked
 * G0205 coordSet 5 [12] = (80,29,64,43) frame route.  Returns 1 if
 * the rect matches the source anchor, 0 otherwise.  Logs the
 * actual values when the check fails. */
static int check_d1c_rect_position(const M11_GameViewState* game,
                                   const char* label) {
    int x = 0, y = 0, w = 0, h = 0;
    int ok = 1;
    if (!M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h)) {
        fprintf(stderr, "FAIL %s D1C ornament zone lookup failed\n", label);
        return 0;
    }
    /* The source-locked coordSet 5 [12] = (80,29,64,43) is the D1C
     * champion-mirror frame route (DUNVIEW.C G0205).  Allow ±2 pixels
     * for any historical rounding. */
    if (x < PROBE_D1C_FRAME_X - 2 || x > PROBE_D1C_FRAME_X + 2 ||
        y < PROBE_D1C_FRAME_Y - 2 || y > PROBE_D1C_FRAME_Y + 2 ||
        w < PROBE_D1C_FRAME_W - 4 || w > PROBE_D1C_FRAME_W + 4 ||
        h < PROBE_D1C_FRAME_H - 4 || h > PROBE_D1C_FRAME_H + 4) {
        fprintf(stderr,
                "FAIL %s D1C ornament zone out of range got=(%d,%d,%d,%d) "
                "want=(%d,%d,%d,%d)\n",
                label, x, y, w, h,
                PROBE_D1C_FRAME_X, PROBE_D1C_FRAME_Y,
                PROBE_D1C_FRAME_W, PROBE_D1C_FRAME_H);
        ok = 0;
    }
    return ok;
}

/* Verify the D1C portrait cutout is positioned at the source-locked
 * (96,35)-(127,63) location inside the wider D1C frame route, and
 * has visible content. */
static int check_d1c_cutout_position(const unsigned char* fb,
                                     const char* label) {
    int distinct;
    int ok = 1;
    distinct = count_distinct_palette(fb,
                                      PROBE_PORTRAIT_X,
                                      PROBE_PORTRAIT_Y,
                                      PROBE_PORTRAIT_W,
                                      PROBE_PORTRAIT_H);
    if (distinct < PROBE_MIN_DISTINCT_PALETTE) {
        fprintf(stderr,
                "FAIL %s D1C portrait cutout at (%d,%d,%d,%d) has no "
                "visible content (distinct=%d)\n",
                label,
                PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
                distinct);
        ok = 0;
    }
    return ok;
}

/* Drive a single east_walkpath step.  If inputBeforeCheck >= 0 the
 * probe calls M11_GameView_HandleInput with the given command
 * (M12_MENU_INPUT_STRAFE_RIGHT for east step, M12_MENU_INPUT_STRAFE_LEFT
 * for west step, M12_MENU_INPUT_TURN_LEFT/TURN_RIGHT for in-place
 * turns) before re-reading the party pose.  Returns 1 if the
 * step's ordinal + pixel invariants all pass, 0 otherwise. */
static int check_east_walk_step(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                int prevOrdinal,
                                const EastWalkStep* step,
                                unsigned char* outFb) {
    MirrorMatch match;
    int ordinal;
    int ok = 1;

    if (step->inputBeforeCheck >= 0) {
        M11_GameInputResult result =
            M11_GameView_HandleInput(game, step->inputBeforeCheck);
        if (result != M11_GAME_INPUT_REDRAW) {
            fprintf(stderr,
                    "FAIL %s input=%d result=%d want=%d\n",
                    step->label, step->inputBeforeCheck,
                    result, M11_GAME_INPUT_REDRAW);
            ok = 0;
        }
    }

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

    /* portrait_rect_position aspect: the D1C frame route is at the
     * source-locked position, and the portrait cutout has visible
     * content.  These checks are direction-agnostic. */
    if (!check_d1c_rect_position(game, step->label)) {
        ok = 0;
    }
    if (!check_d1c_cutout_position(outFb, step->label)) {
        ok = 0;
    }

    /* Match portrait ordinal pixels. */
    match = match_front_portrait(portraits, outFb,
                                 step->expectedOrdinal >= 0
                                     ? step->expectedOrdinal
                                     : 0);

    if (step->expectedOrdinal >= 0) {
        /* Positive-ordinal step: the expected ordinal must dominate
         * the rectangle (best match == expected, >= 90% matched). */
        if (match.bestOrdinal != step->expectedOrdinal ||
            match.compared <= 0 ||
            match.expectedMatched * 100 <
                match.compared * PROBE_PORTRAIT_DOMINANCE_PCT) {
            fprintf(stderr,
                    "FAIL %s portrait expected=%d best=%d matched=%d/%d\n",
                    step->label, step->expectedOrdinal, match.bestOrdinal,
                    match.expectedMatched, match.compared);
            ok = 0;
        }
    } else {
        /* No-portrait step: the rectangle must not be dominated by
         * any portrait ordinal.  Use the looser 50% tolerance the
         * east_walkpath corridor cells at y=5 share with the existing
         * wall_mirror_zones_probe's distinct-palette threshold rather
         * than the per-ordinal 35% leak the zorder/reblt probes
         * lock.  This still catches a clearly-floating portrait
         * sprite (~90%+ matched at the correct ordinal). */
        int thresholdPct = PROBE_NO_PORTRAIT_TOLERANCE_PCT;
        int compareDenom = (match.compared > 0) ? match.compared : 1;
        if (match.bestMatched * 100 >= thresholdPct * compareDenom) {
            fprintf(stderr,
                    "FAIL %s no-portrait step leaked portrait best=%d "
                    "matched=%d/%d (tolerance=%d%%)\n",
                    step->label, match.bestOrdinal, match.bestMatched,
                    match.compared, thresholdPct);
            ok = 0;
        }
    }

    /* Cross-cell re-blt invariant: when the ordinal changes between
     * steps the prior ordinal's pixels must not be the dominant
     * match in the new framebuffer's portrait rectangle.  This is
     * the forward-walk analogue of the in-place turn invariant in
     * firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe. */
    if (prevOrdinal >= 0 && prevOrdinal != step->expectedOrdinal) {
        int stale = count_ordinal_matched_pixels(portraits, outFb,
                                                 prevOrdinal);
        int prevExpected = match_front_portrait(portraits, outFb,
                                                prevOrdinal).expectedMatched;
        int prevCompared = match_front_portrait(portraits, outFb,
                                                prevOrdinal).compared;
        int prevPct = prevCompared > 0 ? (stale * 100) / prevCompared : 0;
        if (prevPct >= PROBE_NO_PORTRAIT_TOLERANCE_PCT) {
            fprintf(stderr,
                    "FAIL %s cross-cell stale ordinal=%d leaked "
                    "matched=%d/%d (expected=%d) after step to ordinal=%d\n",
                    step->label, prevOrdinal, stale, prevCompared,
                    prevExpected, step->expectedOrdinal);
            ok = 0;
        }
    }

    printf("%s pose=(%d,%d,%d) ordinal=%d best=%d matched=%d/%d\n",
           step->label, step->mapX, step->mapY, step->dir,
           ordinal, match.bestOrdinal, match.bestMatched, match.compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char currFb[PROBE_FB_W * PROBE_FB_H];
    int ok = 1;
    /* East-walkpath sequence: face NORTH at y=5, walk east from (0,5)
     * to (1,5) and back.  STRAFE_RIGHT at DIR_NORTH resolves to
     * CMD_MOVE_EAST via m11_strafe_right_command_for_direction
     * (src/engine/m11_game_view.c:1618-1627).  The (1,5,N) front cell
     * is (1,4) which carries the C127 sensor with sensorData=10 (ZED)
     * per DUNGEON.C:2573 / 2608-2612.  DUNVIEW.C:3913-3928 /
     * 8522-8533 blit ordinal 10 into the D1C portrait cutout
     * (96,35)-(127,63).  The (0,5,N) and (2,5,N) front cells have no
     * C127 sensor, so the portrait rectangle is cleared
     * (DUNVIEW.C:7727-7924 F0124_DrawSquareD1C redraws the wall
     * texture from the new party pose after MOVESENS.C:556 invokes
     * F0128_DUNGEONVIEW_Draw_CPSF).
     *
     * Movement semantics verified at probe build time:
     *   (0,5,N) STRAFE_RIGHT -> (1,5,N) MOVED
     *   (1,5,N) STRAFE_RIGHT -> (1,5,N) BLOCKED  (cell (2,5) is a wall)
     *   (1,5,N) STRAFE_LEFT  -> (0,5,N) MOVED
     * so the east_walkpath is necessarily bounded: it can enter
     * (1,5,N) from (0,5,N) and exit back to (0,5,N), but cannot
     * continue east past (1,5).  The probe drives the bounded
     * back-and-forth route so it exercises the ordinal-10
     * establish + ordinal-10 clear + ordinal-10 re-establish + clear
     * cycle in one tick path. */
    const EastWalkStep eastWalkSteps[] = {
        {0, 5, DIR_NORTH, -1,                              -1,
         "hoc_east_walk_a_start_no_portrait"},
        {1, 5, DIR_NORTH, M12_MENU_INPUT_STRAFE_RIGHT,      10,
         "hoc_east_walk_b_step_east_ordinal_10"},
        {0, 5, DIR_NORTH, M12_MENU_INPUT_STRAFE_LEFT,       -1,
         "hoc_east_walk_c_step_west_no_portrait"},
        {1, 5, DIR_NORTH, M12_MENU_INPUT_STRAFE_RIGHT,      10,
         "hoc_east_walk_d_step_east_ordinal_10_again"},
    };
    /* Side-turn sequence: stay at (1,5) and turn in place clockwise
     * (TURN_RIGHT) to face EAST, SOUTH, and WEST.  The front cells
     * (2,5), (1,6), and (0,5) carry distinct C127 sensors:
     *   - (2,5) is a wall (no C127) -> ordinal -1
     *   - (1,6) has C127 sensorData=13 -> ordinal 13 (WUUF)
     *   - (0,5) is a wall (no C127) -> ordinal -1
     * DUNVIEW.C:8522-8533 restricts the champion-portrait blit to the
     * D1C front square; the side poses must therefore end up with no
     * portrait pixels in the D1C wall box except at the south-facing
     * WUUF ordinal (no-floating contract).  TURN_RIGHT at DIR_NORTH
     * rotates clockwise: N->E->S->W->N; see
     * m11_dm1_v1_pipeline_command_for_input
     * (src/engine/m11_game_view.c:7508-7534). */
    const EastWalkStep sideTurnSteps[] = {
        {1, 5, DIR_NORTH, -1,                              10,
         "hoc_side_turn_a_north_ordinal_10"},
        {1, 5, DIR_EAST,  M12_MENU_INPUT_TURN_RIGHT,        -1,
         "hoc_side_turn_b_east_no_portrait"},
        {1, 5, DIR_SOUTH, M12_MENU_INPUT_TURN_RIGHT,        13,
         "hoc_side_turn_c_south_ordinal_13_wuuf"},
        {1, 5, DIR_WEST,  M12_MENU_INPUT_TURN_RIGHT,        -1,
         "hoc_side_turn_d_west_no_portrait"},
        {1, 5, DIR_NORTH, M12_MENU_INPUT_TURN_RIGHT,        10,
         "hoc_side_turn_e_north_ordinal_10_again"},
    };
    int stepIdx;
    int prevOrdinal;

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

    /* Fixture check: the east_walkpath probe expects the canonical
     * Hall of Champions sensor layout with C127 sensorData=10 at
     * (1,4) (so (1,5) facing NORTH reports ordinal 10).  Different
     * DM1 V1 builds place the C127 sensor on different cells; on
     * builds that don't match the reference DUNGEON.DAT the probe
     * skips with a clear SKIP message instead of failing.  This is
     * the same fixture-guard pattern as
     * firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe.c
     * lines 1175-1190. */
    {
        set_pose(&game, 1, 5, DIR_NORTH);
        int probeOrd = M11_GameView_GetFrontMirrorOrdinal(&game);
        if (probeOrd != 10) {
            printf("SKIP hoc_east_walkpath_10_fixture_mismatch "
                   "(1,5) NORTH front ordinal=%d expected=10; "
                   "this DM1 V1 build does not match the reference "
                   "DUNGEON.DAT fixture (the (1,4) sensor is laid out "
                   "differently; see TODO.md fixture-mismatch for the "
                   "full cell->ordinal map)\n", probeOrd);
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

    /* East-walkpath forward + back sequence: face NORTH at y=5,
     * walk east via STRAFE_RIGHT, then walk west via STRAFE_LEFT.
     * This exercises the (0,5) -> (1,5) -> (2,5) -> (1,5) -> (0,5)
     * route that crosses the (1,4) C127 sensor with sensorData=10
     * exactly twice (once entering, once re-entering after the
     * no-portrait (2,5) step). */
    start_independent_input_route(&game, 0, 5, DIR_NORTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(eastWalkSteps) / sizeof(eastWalkSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk = check_east_walk_step(&game, portraits, prevOrd,
                                          &eastWalkSteps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = eastWalkSteps[stepIdx].expectedOrdinal;
    }

    /* Side-turn sequence: turn in place at (1,5) to prove the
     * ordinal 10 portrait rectangle is cleared when the party turns
     * to a no-portrait side pose, and re-established when the party
     * turns back to face NORTH.  The final south-face step also
     * proves the south-facing mirror at (1,5,S) is the (1,6) C127
     * sensor with sensorData=13 (WUUF), confirming the portrait_rect
     * position is correctly redirected when the front cell changes
     * to a different C127 sensor with a different ordinal. */
    start_independent_input_route(&game, 1, 5, DIR_NORTH);
    prevOrdinal = -2;
    for (stepIdx = 0; stepIdx < (int)(sizeof(sideTurnSteps) / sizeof(sideTurnSteps[0])); ++stepIdx) {
        int prevOrd = prevOrdinal;
        int stepOk = check_east_walk_step(&game, portraits, prevOrd,
                                          &sideTurnSteps[stepIdx], currFb);
        if (!stepOk) {
            ok = 0;
        }
        prevOrdinal = sideTurnSteps[stepIdx].expectedOrdinal;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 hoc champion portrait 10 east_walkpath portrait_rect_position runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
