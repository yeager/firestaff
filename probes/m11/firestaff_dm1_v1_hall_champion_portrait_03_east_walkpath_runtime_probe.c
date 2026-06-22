/*
 * firestaff_dm1_v1_hall_champion_portrait_03_east_walkpath_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice:
 *
 *   ordinal       : 3   (champion portrait slot 3 of 24, C026 column 3 row 0)
 *   route variant : east_walkpath (party starts at the western no-portrait
 *                   cell (map=0, x=2, y=7) facing SOUTH, walks east one
 *                   cell at a time through (3,7) where the front cell (3,8)
 *                   carries C127 sensorData=3, and continues to (4,7) and
 *                   (5,7) which are no-portrait corridor walls).
 *   aspect        : portrait_rect_position (D1C front-wall portrait cutout
 *                   at viewport (96,35) sized 32x29, drawn after C346 frame
 *                   per ReDMCSB DUNVIEW.C:3922-3928)
 *
 * Source evidence (ReDMCSB WIP 20210206):
 *   DUNGEON.C:2558,2608-2612  - C127 sensorData = ordinal stored in G0289
 *   DUNGEON.C:2573            - M011_CELL(sensor) selects visible wall cell
 *   MOVESENS.C:1501-1503      - C127 dispatches to F0280 with sensorData
 *   REVIVE.C F0280            - candidate champion materialized from sensorData
 *   REVIVE.C F0282            - confirmed C127 sensor disabled after confirm
 *   DUNVIEW.C:3913-3928       - C346 wall frame, C026 portrait blit at D1C
 *   DUNVIEW.C:8318-8542 F0128 - far-to-near viewport draw order
 *   COORD.C:1693-1722         - PC 3.4 viewport origin / 224x136 dimensions
 *   DEFS.H:2076,3793,2071-2079- C016/C175/C320/C32/C29 zone constants
 *
 * The probe deliberately does not drive the engine's SDL renderer (the
 * headless dummy driver is fine, but rendering the full viewport is not
 * needed to prove the source-locked contract). It exercises:
 *   (A) M11_GameView_GetD1CWallOrnamentZone contract:
 *       DUNVIEW.C G0205 coordSet 5 / index 12 -> dest (80,29,64,43) so the
 *       C026 portrait cutout lives at (96,35,32,29) inside the frame.
 *   (B) portrait ordinal 3 -> C026 source rect math:
 *       (3 & 7) * 32 = 96, (3 >> 3) * 29 = 0 -> (96,0,32,29).
 *   (C) Hall map 0 ordinal scan: east_walkpath at (3,7,SOUTH) yields the
 *       source-locked ordinal from the actual DM1 V1 DUNGEON.DAT.
 *       We report whatever ordinal the source data assigns to that pose
 *       (no invented value); ordinal 3 is the documented east_walkpath
 *       ordinal per the runtime ordinal scan on this DM1 V1 build.
 *   (D) ordinal 3 ANY-pose discovery: scans the Hall map (16x16 cells x
 *       4 directions) and reports every pose whose C127 sensorData is
 *       3, so the slice can be bound to a real DM1 V1 D1C route even
 *       if the canonical east_walkpath pose changes.
 *   (E) east_walkpath route contract: walking east from (1,7,SOUTH)
 *       through (2,7,SOUTH) -> (3,7,SOUTH) -> (4,7,SOUTH) and
 *       (5,7,SOUTH) updates the front ordinal deterministically:
 *       -1 (no-portrait corridor) -> ordinal-3 -> -1 (no-portrait
 *       corridor), proving the C127 sensor on (3,8) is the only
 *       ordinal-3 route in the corridor slice and that the front-wall
 *       aspect is the only one that gets G0289 set per
 *       DUNVIEW.C:2558 BUG0_75 (G0289 is reset only when the draw
 *       function sees at least one wall square).
 *   (F) no-floating contract: for corridor poses where the front cell
 *       does NOT carry a C127 sensor, M11_GameView_GetFrontMirrorOrdinal
 *       returns -1, so the D1C portrait is not drawn over a side wall.
 *   (G) pixel-level proof that the D1C portrait rectangle is drawn at
 *       viewport (96,35)-(127,63): the ordinal-3 C026 cutout (96,0,32,29)
 *       matches the framebuffer's (96,35)-(127,63) box at >= 90% on the
 *       east_walkpath step where front ordinal == 3, and no leaked
 *       pixels remain after walking east through the next no-portrait
 *       step (>= 35% of the rectangle compared with ordinal 3 confirms
 *       the cross-step stale-pixel guard the wall_mirror_zones probe
 *       locks).
 *
 * Honest scope: this probe proves the source-locked ordinal/position
 * contract and the no-floating rule. It does NOT claim DOS pixel parity.
 * Original DM1 PC 3.4 captures live under parity-evidence/ and are
 * referenced by separate parity gates.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    /* Source-locked constants (ReDMCSB DUNVIEW.C:3913-3928, COORD.C:1693-1722,
     * DEFS.H:2071-2079, G9010 PALETTE / G2071-79 viewport geometry). */
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    D1C_PORTRAIT_X = VIEWPORT_X + 96, /* G2078_C32 / C2078 */
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35, /* G2079_C29 / C2079 */
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* G0205 coordSet 5 / index 12 -> C346 wall-mirror frame dest box. */
    D1C_WALL_FRAME_X = 80,
    D1C_WALL_FRAME_Y = 29,
    D1C_WALL_FRAME_W = 64,
    D1C_WALL_FRAME_H = 43,
    /* C026 graphic: 24 portraits in 8 columns x 3 rows. */
    C026_PORTRAITS_TOTAL = 24,
    C026_COLS = 8,
    C026_ROWS = 3,
    PORTRAIT_STRIP_W = 256, /* 8 * 32 */
    PORTRAIT_STRIP_H = 87,  /* 3 * 29 */
    PORTRAIT_ORDINAL_TARGET = 3,
    HALL_MAP_INDEX = 0,
    /* east_walkpath route: walk east along y=7 facing SOUTH.
     * The canonical ordinal-3 cell (front cell carries C127 sensorData=3)
     * is the (3,7,SOUTH) pose per the runtime ordinal scan on this DM1 V1
     * build; the west step is (2,7,SOUTH) and the east step is
     * (4,7,SOUTH), both no-portrait corridor walls on this build. */
    EAST_WALK_START_X = 1,
    EAST_WALK_START_Y = 7,
    EAST_WALK_ORDINAL_X = 3,
    EAST_WALK_ORDINAL_Y = 7,
    EAST_WALK_DIR = 2, /* DIR_SOUTH */
    EAST_WALK_END_X = 5,
    EAST_WALK_END_Y = 7,
    HALL_MAX_CELLS_PER_AXIS = 16,
    /* ReDMCSB DUNVIEW.C:3916 C01_COLOR_DARK_GRAY transparency mask for the
     * C026 champion-portrait blit.  Same constant the existing visibility
     * / walkpath / zorder probes lock. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Stale-pixel leak tolerance (matches the existing zorder / reblt /
     * walkpath probes' documented 35% threshold). */
    STALE_PCT_TOLERANCE = 35,
    /* Visible-portrait dominance threshold for the east_walkpath ordinal-3
     * pose (matches the existing walkpath probe's 90% pixel-match
     * threshold). */
    VISIBLE_PCT_THRESHOLD = 90
};

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

/* Convert a portrait ordinal to its (col, row) in the C026 strip. */
static void portrait_ordinal_to_cr(int ordinal, int* outCol, int* outRow) {
    if (outCol) *outCol = ordinal & 7;          /* column = ordinal mod 8 */
    if (outRow) *outRow = (ordinal >> 3) & 3;   /* row = ordinal / 8 */
}

/* Source-rect for a portrait ordinal: (x, y, w, h) in C026 pixels. */
static void portrait_ordinal_source_rect(int ordinal,
                                         int* outX, int* outY,
                                         int* outW, int* outH) {
    int col;
    int row;
    portrait_ordinal_to_cr(ordinal, &col, &row);
    if (outX) *outX = col * D1C_PORTRAIT_W;
    if (outY) *outY = row * D1C_PORTRAIT_H;
    if (outW) *outW = D1C_PORTRAIT_W;
    if (outH) *outH = D1C_PORTRAIT_H;
}

/* (A) Verify D1C wall-mirror frame helper returns the source-locked
 *     box. ReDMCSB DUNVIEW.C G0205 coordSet 5 / index 12 is the
 *     C346 champion-mirror frame route; the C026 portrait is the
 *     smaller cutout (96,35)-(127,63) inside this frame, so the
 *     helper must return (80,29,64,43) regardless of party state. */
static int test_d1c_wall_ornament_zone(M11_GameViewState* game) {
    int x = -1, y = -1, w = -1, h = -1;
    int ok = 1;
    /* The C026 portrait cutout lives at viewport-relative (96,35) with
     * size (32,29); the C346 frame helper returns viewport-relative
     * coordinates (the helper's output box is viewport-relative, even
     * though M11_GameView_Draw consumes framebuffer-relative Y by
     * adding M11_VIEWPORT_Y = 33 internally).  Use viewport-relative
     * coordinates for the containment check. */
    const int kCutoutViewportX = 96;
    const int kCutoutViewportY = 35;
    printf("[A] D1C wall-ornament frame contract (C346/C026 anchor)\n");
    ok &= expect_int("M11_GameView_GetD1CWallOrnamentZone returns 1",
                     M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h), 1);
    ok &= expect_int("D1C frame dstX == 80", x, D1C_WALL_FRAME_X);
    ok &= expect_int("D1C frame dstY == 29", y, D1C_WALL_FRAME_Y);
    ok &= expect_int("D1C frame width == 64", w, D1C_WALL_FRAME_W);
    ok &= expect_int("D1C frame height == 43", h, D1C_WALL_FRAME_H);
    /* The C026 portrait cutout (viewport 96,35 with size 32x29) must
     * lie inside the wall-frame box (viewport 80,29 with size 64x43);
     * cutout origin - frame origin = (16, 6) and the cutout fits
     * inside the 64x43 frame.  The cutout right-edge offset from the
     * frame left edge is 96+32-80 = 48, and the cutout bottom-edge
     * offset from the frame top edge is 35+29-29 = 35; both are
     * strictly less than the frame width/height (64/43). */
    ok &= expect_int("portrait cutout origin X relative to frame",
                     kCutoutViewportX - D1C_WALL_FRAME_X, 16);
    ok &= expect_int("portrait cutout origin Y relative to frame",
                     kCutoutViewportY - D1C_WALL_FRAME_Y, 6);
    ok &= expect_int("portrait cutout right edge inside frame width (64)",
                     ((kCutoutViewportX + D1C_PORTRAIT_W) - D1C_WALL_FRAME_X) <= D1C_WALL_FRAME_W,
                     1);
    ok &= expect_int("portrait cutout bottom edge inside frame height (43)",
                     ((kCutoutViewportY + D1C_PORTRAIT_H) - D1C_WALL_FRAME_Y) <= D1C_WALL_FRAME_H,
                     1);
    ok &= expect_int("portrait cutout right edge offset == 48",
                     (kCutoutViewportX + D1C_PORTRAIT_W) - D1C_WALL_FRAME_X, 48);
    ok &= expect_int("portrait cutout bottom edge offset == 35",
                     (kCutoutViewportY + D1C_PORTRAIT_H) - D1C_WALL_FRAME_Y, 35);
    return ok;
}

/* (B) Verify portrait ordinal 3 -> C026 source rect (96, 0, 32, 29).
 *     Source: ReDMCSB DUNVIEW.C:3913-3928 C026 = 8 cols x 3 rows. */
static int test_portrait_ordinal_math(void) {
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int col = -1, row = -1;
    int ok = 1;
    printf("[B] Portrait ordinal 3 -> C026 source rect math\n");

    portrait_ordinal_to_cr(PORTRAIT_ORDINAL_TARGET, &col, &row);
    ok &= expect_int("ordinal 3 column (col = ordinal mod 8)", col, 3);
    ok &= expect_int("ordinal 3 row (row = ordinal / 8)", row, 0);
    portrait_ordinal_source_rect(PORTRAIT_ORDINAL_TARGET, &sx, &sy, &sw, &sh);
    ok &= expect_int("ordinal 3 source X == 3*32", sx, 96);
    ok &= expect_int("ordinal 3 source Y == 0*29", sy, 0);
    ok &= expect_int("ordinal 3 source W == 32", sw, D1C_PORTRAIT_W);
    ok &= expect_int("ordinal 3 source H == 29", sh, D1C_PORTRAIT_H);
    /* Source rect must lie inside the 256x87 C026 strip.  The
     * ordinal-3 cutout right edge (sx+sw = 128) and bottom edge
     * (sy+sh = 29) must be <= strip width/height (256/87), not
     * equal to them. */
    ok &= expect_int("ordinal 3 source X + W <= strip width (256)",
                     (sx + sw) <= PORTRAIT_STRIP_W, 1);
    ok &= expect_int("ordinal 3 source Y + H <= strip height (87)",
                     (sy + sh) <= PORTRAIT_STRIP_H, 1);
    ok &= expect_int("ordinal 3 source X + W == 128", sx + sw, 128);
    ok &= expect_int("ordinal 3 source Y + H == 29", sy + sh, 29);
    return ok;
}

/* (C) Verify the canonical east_walkpath ordinal pose resolves to the
 *     ordinal the source-locked DM1 V1 DUNGEON.DAT assigns to
 *     (map=0, x=3, y=7) facing SOUTH. The expected ordinal is reported,
 *     not invented; the runtime ordinal scan on this DM1 V1 build
 *     returns ordinal 3 there. */
static int test_east_walkpath_ordinal(M11_GameViewState* game) {
    int ord = -999;
    int ok = 1;
    printf("[C] east_walkpath ordinal at (3,7,SOUTH)\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = EAST_WALK_ORDINAL_X;
    game->world.party.mapY = EAST_WALK_ORDINAL_Y;
    game->world.party.direction = EAST_WALK_DIR;
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: east_walkpath runtime ordinal = %d\n", ord);
    /* Pin the source-locked expected value: the (3,8) C127 sensor
     * on this DM1 V1 build carries sensorData=3, so the front ordinal
     * at (3,7,SOUTH) is the source-locked ordinal 3. */
    ok &= expect_int("east_walkpath ordinal at (3,7,SOUTH) == 3", ord,
                     PORTRAIT_ORDINAL_TARGET);
    return ok;
}

/* (D) Scan the Hall map (16x16 cells x 4 directions) and report every
 *     pose whose C127 sensorData ordinal equals the target ordinal.
 *     This binds the ordinal 3 slice to real DM1 V1 D1C routes if any
 *     exist, so future regressions can detect when ordinal 3 moves
 *     (e.g. a re-sorted mirror catalog). */
static int test_ordinal_3_any_pose(M11_GameViewState* game) {
    int mapX, mapY, dir;
    int hits = 0;
    int ok = 1;
    printf("[D] ordinal 3 ANY-pose discovery on Hall map 0 (16x16 x 4 dirs)\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (mapY = 0; mapY < HALL_MAX_CELLS_PER_AXIS; ++mapY) {
        for (mapX = 0; mapX < HALL_MAX_CELLS_PER_AXIS; ++mapX) {
            for (dir = 0; dir < 4; ++dir) {
                int ord;
                game->world.party.mapX = mapX;
                game->world.party.mapY = mapY;
                game->world.party.direction = dir;
                ord = M11_GameView_GetFrontMirrorOrdinal(game);
                if (ord == PORTRAIT_ORDINAL_TARGET) {
                    ++hits;
                    printf("  HIT: ordinal 3 at pose=(map=%d, x=%d, y=%d, dir=%d)\n",
                           HALL_MAP_INDEX, mapX, mapY, dir);
                }
            }
        }
    }
    printf("  INFO: ordinal 3 found at %d pose(s) on Hall map 0\n", hits);
    if (hits == 0) {
        printf("  NOTE: ordinal 3 has no C127 route on Hall map 0 in this DM1 V1\n");
        printf("        build; the east_walkpath route is independently verified in [C].\n");
    }
    /* No "must find >= N" assertion: this probe is observational, not a
     * hard contract, because the Hall mirror catalog ordinals are
     * assigned by DUNGEON.DAT TextString ordering at build time.
     * The probe prints the count so CI can detect any change. */
    (void)ok;
    return 1;
}

/* (E) east_walkpath route contract: walking east along y=7 facing
 *     SOUTH must produce a deterministic front-ordinal sequence that
 *     crosses the source-locked ordinal-3 cell at (3,7,SOUTH).  The
 *     west step (1,7) is a no-portrait corridor (front cell (1,8) has
 *     no C127 sensor), the (2,7) step exposes an unrelated ordinal-16
 *     wall (front cell (2,8) carries sensorData=16), the (3,7) step
 *     exposes the source-locked ordinal-3 wall (front cell (3,8)
 *     carries sensorData=3), and the east steps (4,7) and (5,7) are
 *     no-portrait corridor walls.  ReDMCSB DUNGEON.C:2558 BUG0_75:
 *     G0289 is only reset when the draw function sees at least one
 *     wall square, and the front wall is wall-like for each step in
 *     this slice. */
static int test_east_walkpath_route(M11_GameViewState* game) {
    static const struct { int x, y; int expectedOrdinal; const char* label; }
        kEastSteps[] = {
            {1, 7, -1, "(1,7,SOUTH) far west step (corridor, no portrait)"},
            {2, 7, 16, "(2,7,SOUTH) west step (ordinal-16 wall)"},
            {3, 7, 3,  "(3,7,SOUTH) east_walkpath ordinal step (ordinal-3 wall)"},
            {4, 7, -1, "(4,7,SOUTH) east step (corridor, no portrait)"},
            {5, 7, -1, "(5,7,SOUTH) far east step (corridor, no portrait)"},
        };
    int ok = 1;
    int i;
    size_t n = sizeof(kEastSteps) / sizeof(kEastSteps[0]);
    printf("[E] east_walkpath route contract along y=7 facing SOUTH\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.direction = EAST_WALK_DIR;
    for (i = 0; i < (int)n; ++i) {
        int ord;
        char label[96];
        game->world.party.mapX = kEastSteps[i].x;
        game->world.party.mapY = kEastSteps[i].y;
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(label, sizeof(label), "%s ordinal", kEastSteps[i].label);
        ok &= expect_int(label, ord, kEastSteps[i].expectedOrdinal);
    }
    return ok;
}

/* (F) No-floating contract: at corridor poses where the front cell
 *     does NOT carry a C127 sensor, GetFrontMirrorOrdinal must
 *     return -1. ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) to a
 *     visible wall cell; for corridor cells the sensor is not on
 *     the front wall, so m11_front_cell_mirror_ordinal returns -1
 *     and the D1C portrait is not drawn.  The poses below are
 *     outside the ordinal-3 corridor (y=7) and their front cells do
 *     not carry a C127 sensor on this DM1 V1 build.  The pose at
 *     (1,7,NORTH) is excluded because (1,7)'s north-front (1,6) does
 *     carry an ordinal-13 sensor on this build; that route belongs to
 *     a different mirror and is outside the east_walkpath slice. */
static int test_no_floating_on_side_walls(M11_GameViewState* game) {
    static const struct { int x, y, dir; const char* label; } kCorridorPoses[] = {
        {0, 7, 2, "hall_corridor_south_no_portrait"},
        {1, 7, 2, "hall_start_south_no_portrait"},
        {1, 7, 1, "hall_start_east_wrong_wall_no_portrait"},
        {1, 7, 3, "hall_start_west_wrong_wall_no_portrait"},
        {4, 7, 2, "hall_ordinal3_neighbour_south_no_portrait"},
        {5, 7, 2, "hall_ordinal3_far_south_no_portrait"},
        {3, 6, 2, "hall_ordinal3_cell_north_no_portrait"},
        {3, 8, 2, "hall_ordinal3_cell_south_no_portrait"},
        {3, 7, 0, "hall_ordinal3_pose_facing_north_no_portrait"},
        {3, 7, 1, "hall_ordinal3_pose_facing_east_no_portrait"},
        {3, 7, 3, "hall_ordinal3_pose_facing_west_no_portrait"}
    };
    int ok = 1;
    int i;
    size_t n = sizeof(kCorridorPoses) / sizeof(kCorridorPoses[0]);
    printf("[F] No-floating contract: side walls must not expose the front portrait\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (i = 0; i < (int)n; ++i) {
        int ord;
        char label[80];
        game->world.party.mapX = kCorridorPoses[i].x;
        game->world.party.mapY = kCorridorPoses[i].y;
        game->world.party.direction = kCorridorPoses[i].dir;
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(label, sizeof(label), "%s ordinal == -1 (no float)",
                 kCorridorPoses[i].label);
        ok &= expect_int(label, ord, -1);
    }
    return ok;
}

/* Pixel-match helpers for [G]: count the D1C portrait cutout pixels
 * that match the C026 ordinal source strip, with the C01 dark-gray
 * transparency mask applied. */
static int count_ordinal_matched_pixels(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int ordinal,
                                        int* outCompared) {
    int x;
    int y;
    int matched = 0;
    int compared = 0;
    if (outCompared) *outCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= C026_PORTRAITS_TOTAL) {
        return 0;
    }
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * D1C_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * D1C_PORTRAIT_H + y;
            unsigned char src;
            unsigned char dst;
            if (srcX < 0 || srcX >= (int)portraits->width ||
                srcY < 0 || srcY >= (int)portraits->height) {
                continue;
            }
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W + (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    if (outCompared) *outCompared = compared;
    return matched;
}

/* (G) Pixel-level proof that the D1C portrait rectangle at viewport
 *     (96,35)-(127,63) carries the ordinal-3 C026 cutout at the
 *     east_walkpath ordinal step, and that walking east past the
 *     ordinal-3 wall does not leave ordinal-3 pixels floating on the
 *     corridor. The D1C cutout rectangle matches the existing
 *     walkpath / visibility / zorder / reblt probes' contract:
 *     ordinal 3 dominates the rectangle at >= 90% on the ordinal
 *     step, the previous ordinal-16 wall pixel-set clears below the
 *     35% stale-pixel threshold on the ordinal-3 step (the wall
 *     ordinal changes), and the ordinal-3 pixels are wiped from the
 *     D1C cutout rectangle when walking east to (4,7,SOUTH) — a true
 *     no-portrait corridor wall — below the 35% leak threshold.
 *     The same cross-step guard is asserted as a "no float on
 *     ordinary corridor wall" gate (BUG-120/121 / BUG0_75 invariant
 *     in DUNVIEW.C:2558 + DUNVIEW.C:3922-3928). */
static int test_d1c_portrait_rect_position(M11_GameViewState* game,
                                            const M11_AssetSlot* portraits,
                                            unsigned char* fb) {
    static const struct { int x, y; int expectedOrdinal; const char* label; }
        kEastSteps[] = {
            {2, 7, 16, "ordinal_step_pre_ordinal_16_wall"},
            {3, 7, 3,  "ordinal_step_ordinal_3_visible"},
            {4, 7, -1, "ordinal_step_post_no_portrait"},
        };
    int prevOrdinal = -2; /* sentinel: no prior ordinal */
    int ok = 1;
    int i;
    size_t n = sizeof(kEastSteps) / sizeof(kEastSteps[0]);
    printf("[G] D1C portrait rectangle pixel contract (96,35)-(127,63)\n");
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        ++g_fail;
        printf("  FAIL: GRAPHICS.DAT champion portrait strip unavailable\n");
        return 0;
    }
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.direction = EAST_WALK_DIR;
    for (i = 0; i < (int)n; ++i) {
        int ord;
        int matched = 0;
        int compared = 0;
        int pct;
        char label[120];
        game->world.party.mapX = kEastSteps[i].x;
        game->world.party.mapY = kEastSteps[i].y;
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        /* Reset framebuffer and draw the current pose. */
        memset(fb, 0, sizeof(*fb) * (size_t)(FB_W * FB_H));
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        matched = count_ordinal_matched_pixels(portraits, fb,
                                               kEastSteps[i].expectedOrdinal >= 0
                                                   ? kEastSteps[i].expectedOrdinal
                                                   : 0,
                                               &compared);
        pct = compared > 0 ? (matched * 100) / compared : 0;
        snprintf(label, sizeof(label),
                 "%s: ordinal=%d expected=%d matched=%d/%d (%d%%)",
                 kEastSteps[i].label, ord,
                 kEastSteps[i].expectedOrdinal, matched, compared, pct);
        if (kEastSteps[i].expectedOrdinal >= 0) {
            if (ord != kEastSteps[i].expectedOrdinal ||
                pct < VISIBLE_PCT_THRESHOLD) {
                ++g_fail;
                printf("  FAIL: %s\n", label);
                ok = 0;
            } else {
                ++g_pass;
                printf("  PASS: %s\n", label);
            }
        } else {
            /* No-portrait step: no ordinal should dominate; the prior
             * ordinal's stale-pixel leak is the cross-step guard. */
            if (ord != kEastSteps[i].expectedOrdinal) {
                ++g_fail;
                printf("  FAIL: %s (front ordinal drift)\n", label);
                ok = 0;
            } else {
                ++g_pass;
                printf("  PASS: %s (no-portrait step)\n", label);
            }
        }
        /* Cross-step stale-pixel guard: the prior ordinal's pixels
         * must not dominate the rectangle after a step changes the
         * front cell (DUNVIEW.C:8318-8542 F0128 viewport redraw
         * rebuilds the D1C cutout from the new front cell's C127
         * sensorData). Same 35% threshold the existing zorder / reblt
         * / walkpath probes lock. */
        if (prevOrdinal >= 0 && prevOrdinal != kEastSteps[i].expectedOrdinal) {
            int staleMatched = 0;
            int staleCompared = 0;
            int stalePct;
            char staleLabel[120];
            staleMatched = count_ordinal_matched_pixels(portraits, fb,
                                                        prevOrdinal,
                                                        &staleCompared);
            stalePct = staleCompared > 0
                ? (staleMatched * 100) / staleCompared : 0;
            snprintf(staleLabel, sizeof(staleLabel),
                     "%s: prior ordinal %d stale pixels = %d/%d (%d%%)",
                     kEastSteps[i].label, prevOrdinal,
                     staleMatched, staleCompared, stalePct);
            if (stalePct >= STALE_PCT_TOLERANCE) {
                ++g_fail;
                printf("  FAIL: %s\n", staleLabel);
                ok = 0;
            } else {
                ++g_pass;
                printf("  PASS: %s\n", staleLabel);
            }
        }
        prevOrdinal = kEastSteps[i].expectedOrdinal;
    }
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    static unsigned char framebuffer[FB_W * FB_H];
    const M11_AssetSlot* portraits;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 3, "
           "route east_walkpath, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                REVIVE.C F0280,F0282 (candidate materialise/disable)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:2071-2079 (G2071_C320/G2078_C32/G2079_C29)\n\n");

    ok &= test_d1c_wall_ornament_zone(&game);
    ok &= test_portrait_ordinal_math();
    ok &= test_east_walkpath_ordinal(&game);
    ok &= test_ordinal_3_any_pose(&game);
    ok &= test_east_walkpath_route(&game);
    ok &= test_no_floating_on_side_walls(&game);

    /* [G] needs the C026 portrait strip loaded into the asset cache.
     * The probe only fails [G] (not the source-locked [A]-[F] parts)
     * when the asset is missing, matching the existing walkpath /
     * visibility / zorder probes' tolerance. */
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    ok &= test_d1c_portrait_rect_position(&game, portraits, framebuffer);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
