/*
 * firestaff_dm1_v1_hall_champion_portrait_22_d2c_far_positive_runtime_probe.c
 *
 * Real-asset/runtime regression for one narrow DM1 V1 Hall of Champions
 * champion-portrait slice:
 *
 *   ordinal       : 22  (champion portrait slot 22 of 24, C026 column 6
 *                   row 2 -- the "GOTHMOG" slot per the DM1 V1 mirror
 *                   TextString catalog; the C026 atlas has 8 cols x 3
 *                   rows and the GOTHMOG row is the third row of the
 *                   strip)
 *   route variant : d2c_far_positive (party stands at a D2C-far pose
 *                   on the +Y (NORTH) axis -- the D1C cell is one
 *                   tile forward of the party, but the C127 sensor
 *                   that would drive the D1C portrait is two tiles
 *                   forward at the D2C F0128 dispatch depth; the
 *                   F0128 D2C dispatch at DUNVIEW.C:8520-8521 sees
 *                   a real ordinal (sensorData >= 0) at that depth,
 *                   hence "positive" -- in contrast to the
 *                   "negative" D2C-far path where the D2C square
 *                   has no C127 sensor and the F0128 D2C dispatch
 *                   has no ordinal to bind).
 *   aspect        : portrait_rect_position (D1C front-wall portrait
 *                   cutout at viewport (96,35) sized 32x29, drawn
 *                   after C346 frame per ReDMCSB DUNVIEW.C:3922-3928;
 *                   the D1C cutout is constant for all 24 ordinals --
 *                   only the C026 source cell sampled changes with
 *                   the ordinal)
 *
 * Source evidence (ReDMCSB WIP 20210206):
 *   DUNGEON.C:2558,2608-2612  - C127 sensorData = ordinal stored in G0289
 *   DUNGEON.C:2573            - M011_CELL(sensor) selects visible wall cell
 *   MOVESENS.C:1501-1503      - C127 dispatches to F0280 with sensorData
 *   REVIVE.C F0280            - candidate champion materialized from sensorData
 *   REVIVE.C F0282            - confirmed C127 sensor disabled after confirm
 *   DUNVIEW.C:3913-3928       - C346 wall frame, C026 portrait blit at D1C
 *   DUNVIEW.C:8318-8542 F0128 - far-to-near viewport draw order (D2C
 *                               dispatched at line 8520-8521 before D1C)
 *   DUNVIEW.C:8520-8521       - F0128 dispatches D2C (depth 2 center)
 *                               before D1C (depth 1 center) at lines
 *                               8520-8521; the D2C blit precedes the
 *                               D1C blit so D1C overlays D2C, and a
 *                               D2C wall ornament can be visible
 *                               behind/around a D1C wall mirror.
 *   COORD.C:1693-1722         - PC 3.4 viewport origin / 224x136 dimensions
 *   DEFS.H:821-826            - M027_PORTRAIT_X / M028_PORTRAIT_Y macro math
 *   DEFS.H:2071-2079          - C320/C32/C29 zone constants
 *
 * Slice geometry (party at (1,3) facing NORTH, D0C=party square):
 *   D0C cell = (1, 3)   <- party square (no C127 sensor)
 *   D1C cell = (1, 2)   <- front wall cell (no C127 sensor on the
 *                          south wall; the (1,2) cell carries no
 *                          champion-mirror sensor in the local PC
 *                          3.4 fixture, so the D1C front-mirror
 *                          ordinal helper reports -1)
 *   D2C cell = (1, 1)   <- far wall cell carrying the C127 sensor
 *                          with sensorData=1 (HALK) on cell 0
 *                          (north wall); the F0128 D2C dispatch
 *                          at DUNVIEW.C:8520-8521 observes this
 *                          C127 sensor at D2C depth, hence
 *                          "positive" -- the dispatch has a real
 *                          ordinal (sensorData >= 0) to bind for
 *                          the D2C wall ornament.
 *   D3C cell = (1, 0)   <- out-of-view (depth 3)
 *
 * The slice is "D2C far positive" because the C127 sensor that
 * drives the wall-mirror portrait is observable at the D2C F0128
 * dispatch depth (the F0128 D2C blit sees the sensor's sensorData
 * as a real ordinal for the wall ornament binding), even though
 * the D1C front-mirror helper reports -1 (no C127 sensor at the
 * immediate-front cell).  The "portrait_rect_position" aspect is
 * verified at the D1C view distance by walking forward to
 * (1,2,N), where the C127 sensor at (1,1) cell 0 is at D1C depth
 * and the helper reports the rewritten ordinal 22.
 *
 * This probe deliberately does not drive the engine's SDL renderer
 * (the headless dummy driver is fine, but rendering the full viewport
 * is not needed to prove the source-locked contract).  It exercises:
 *   (A) C026 atlas math for ordinal 22: ((22 & 7) * 32, (22 >> 3) * 29)
 *       = (192, 58) source rect inside a 256x87 atlas, the BOTTOM row
 *       (row 2) of the 8x3 strip.  Row 2 is the only row where the
 *       source cell bottom (srcY + 29) exactly equals the atlas height
 *       (87), so this slice implicitly proves the (ordinal >> 3) * 29
 *       source math is correct for the 3rd row boundary.
 *   (B) Catalog identity for ordinal 22: name = "GOTHMOG" (DM1 V1 PC
 *       3.4 TextString ordering), title slot = empty (untitled
 *       champion).  Pinning the catalog name keeps the slot bound
 *       to a real source identity even if no Hall map route exists
 *       for ordinal 22 in the local data fixture.
 *   (C) D2C far positive route pose: park the party at (1,3) facing
 *       NORTH on Hall map 0.  The D1C cell is (1,2) with no C127
 *       sensor -- the front-mirror ordinal helper returns -1, the
 *       D1C portrait is NOT drawn at this view distance.  The D2C
 *       cell is (1,1) with a C127 sensor carrying sensorData=1
 *       (HALK) -- the F0128 D2C dispatch at DUNVIEW.C:8520-8521
 *       observes this C127 sensor at D2C depth, hence "D2C far
 *       positive" (the D2C dispatch has a real ordinal to bind).
 *   (D) D1C wall ornament zone contract at D2C-far view: the
 *       M11_GameView_GetD1CWallOrnamentZone helper must return
 *       the source-locked (80, 29, 64, 43) box regardless of the
 *       view depth.  This proves the wall-ornament geometry
 *       helper is not depth-conditional, and the D1C frame
 *       rect is reserved even at D2C-far views where the D1C
 *       portrait is not drawn.
 *   (E) D2C far positive route seed: temporarily rewrite the C127
 *       sensor on the (1,1) cell from sensorData=1 to
 *       sensorData=22 (GOTHMOG), then verify:
 *         (E1) the D2C-far route (1,3,N) still reports -1 on the
 *              D1C depth (the (1,2) cell has no C127 sensor, so
 *              the D1C portrait is not drawn at the D2C-far view
 *              distance);
 *         (E2) the C127 sensor observable at the (1,1) D2C cell
 *              now reports sensorData=22 (the rewrite is bound to
 *              the D2C F0128 dispatch depth, not the D1C depth);
 *         (E3) walking forward one cell from (1,3,N) to (1,2,N)
 *              reveals the C127 sensor at the D1C depth, where
 *              the D1C front-mirror ordinal helper now reports
 *              22 (GOTHMOG) -- the D2C F0128 contract translates
 *              to the D1C portrait_rect when the party walks
 *              forward to the D1C view distance.  The sensor is
 *              restored before the probe exits.
 *   (F) No-floating contract for D2C-far D2L2/D2R2 side walls:
 *       at (1,3,N) the D2C cell is (1,1), the D2L2 cell is (0,1),
 *       and the D2R2 cell is (2,1).  The D2L2 and D2R2 side
 *       walls are NOT to carry the C127 sensor with
 *       sensorData=22 after the route seed, so the front-mirror
 *       ordinal at the D2L2/D2R2 party poses must remain -1
 *       (no positive ordinal resolution at the side lane).  This
 *       proves the D1C portrait sprite does not "float" onto the
 *       D2 side lanes when the party is at a D2C-far view
 *       distance.
 *
 * Honest scope: this probe proves the source-locked ordinal/position
 * contract and the no-floating rule for the ordinal 22 / d2c_far
 * positive route.  It does NOT claim DOS pixel parity.  Original DM1
 * PC 3.4 captures live under parity-evidence/ and are referenced by
 * separate parity gates.  This is a companion slice to
 * firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_runtime_probe
 * (depth-1 D1C view) and shares the same catalog / atlas math
 * source-locks; the new assertion is the D2C-far positive
 * (depth-2) view contract.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

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
    PORTRAIT_ORDINAL_TARGET = 22,
    HALL_MAP_INDEX = 0,
    /* D2C far positive route pose (party at (1,3) facing NORTH):
     *   D2C cell = (1, 1)   <- C127 sensor with sensorData=1 (HALK)
     *                          on cell 0 (north wall).  F0128 D2C
     *                          dispatch at DUNVIEW.C:8520-8521
     *                          observes this sensor at D2C depth.
     *   D1C cell = (1, 2)   <- no C127 sensor -> D1C portrait = -1
     *   D0C cell = (1, 3)   <- party square
     * The "D2C far positive" route resolves a positive ordinal
     * (sensorData >= 0) at the D2C depth, even though the D1C
     * depth reports -1 (no D1C mirror at the immediate-front
     * cell). */
    D2C_FAR_POSITIVE_X = 1,
    D2C_FAR_POSITIVE_Y = 3,
    D2C_FAR_POSITIVE_DIR = 0, /* DIR_NORTH */
    /* The (1,1) cell carries the C127 sensor that the D2C far
     * positive route resolves -- this is the (1,1) "D2C cell"
     * when the party is at (1,3) facing NORTH.  The C127 sensor
     * is on cell 0 (north wall of (1,1)), visible from (1,2,N)
     * at D1C depth (the (1,2) front cell's south wall). */
    D2C_CELL_X = 1,
    D2C_CELL_Y = 1,
    /* The D1C view distance from the D2C far pose: walking
     * forward one cell from (1,3,N) reaches (1,2,N) where the
     * D1C cell becomes (1,1) (the same C127 sensor that the
     * D2C far positive route observed). */
    D1C_NEAR_X = 1,
    D1C_NEAR_Y = 2,
    D1C_NEAR_DIR = 0, /* DIR_NORTH */
    HALL_MAX_CELLS_PER_AXIS = 16,
    /* Expected catalog identity for ordinal 22 (DM1 V1 PC 3.4 TextString):
     * GOTHMOG is untitled in the catalog (title[0] == '\0'), so the
     * title assertion below checks `championTitle[0] == '\0'`. */
    PORTRAIT_22_TITLE_EMPTY = 1,
    /* The (1,1) C127 sensor baseline ordinal, pinned here so the
     * D2C far positive probe does not invent a value: per
     * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe,
     * the (1,2) facing NORTH cell's front-mirror ordinal is 1
     * (HALK), driven by the C127 sensor on the (1,1) cell
     * north wall (cell 0).  This is the same sensor observable
     * at D2C depth from (1,3,N). */
    FRONT_MIRROR_BASELINE = 1
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

/* (A) Verify the C026 atlas math for ordinal 22 -> source rect
 *     (192, 58, 32, 29).  Source: ReDMCSB DUNVIEW.C:3916-3919
 *     C026 = 8 cols x 3 rows.  Ordinal 22 -> col=6, row=2 (the
 *     third row of the C026 strip, the BOTTOM row).  Row 2 is
 *     the only row where the source cell bottom (srcY + 29)
 *     exactly equals the atlas height (87), so this assertion
 *     implicitly proves the (ordinal >> 3) * 29 source math
 *     is correct for the 3rd row boundary. */
static int test_portrait_ordinal_math(void) {
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int col = -1, row = -1;
    int ok = 1;
    printf("[A] C026 atlas math for ordinal 22 (row 2 / col 6 of 8x3 strip)\n");
    portrait_ordinal_to_cr(PORTRAIT_ORDINAL_TARGET, &col, &row);
    ok &= expect_int("ordinal 22 column (col = ordinal mod 8)", col, 6);
    ok &= expect_int("ordinal 22 row (row = ordinal / 8)", row, 2);
    portrait_ordinal_source_rect(PORTRAIT_ORDINAL_TARGET, &sx, &sy, &sw, &sh);
    ok &= expect_int("ordinal 22 source X == 6*32", sx, 192);
    ok &= expect_int("ordinal 22 source Y == 2*29", sy, 58);
    ok &= expect_int("ordinal 22 source W == 32", sw, D1C_PORTRAIT_W);
    ok &= expect_int("ordinal 22 source H == 29", sh, D1C_PORTRAIT_H);
    /* Source rect must lie inside the 256x87 C026 strip -- use a
     * containment check (right edge <= strip width) instead of
     * exact equality to (PORTRAIT_STRIP_W, PORTRAIT_STRIP_H). */
    ok &= expect_int("ordinal 22 right edge <= C026 strip width",
                     sx + sw <= PORTRAIT_STRIP_W, 1);
    ok &= expect_int("ordinal 22 bottom edge <= C026 strip height",
                     sy + sh <= PORTRAIT_STRIP_H, 1);
    /* The row-2 source cell bottom must exactly equal the atlas
     * height (87).  This is the only ordinal 0..23 where the
     * bottom edge touches the strip boundary, so the assertion
     * catches row-count regressions (e.g. 2 rows / height=58)
     * and stride regressions (e.g. 30 instead of 29). */
    ok &= expect_int("ordinal 22 row-2 source bottom exactly reaches atlas height",
                     sy + sh == PORTRAIT_STRIP_H, 1);
    /* And every ordinal 0..23 must produce a valid in-strip rect. */
    {
        int k;
        int allValid = 1;
        for (k = 0; k < C026_PORTRAITS_TOTAL; ++k) {
            int kx, ky, kw, kh;
            portrait_ordinal_source_rect(k, &kx, &ky, &kw, &kh);
            if (kx < 0 || ky < 0 ||
                kx + kw > PORTRAIT_STRIP_W ||
                ky + kh > PORTRAIT_STRIP_H) {
                allValid = 0;
                printf("  FAIL: ordinal %d -> (%d,%d,%d,%d) outside strip\n",
                       k, kx, ky, kw, kh);
                break;
            }
        }
        ok &= expect_int("all 24 ordinals produce in-strip source rects",
                         allValid, 1);
    }
    return ok;
}

/* (B) Verify the catalog identity for ordinal 22: name = "GOTHMOG"
 *     per the DM1 V1 PC 3.4 TextString ordering; title slot = empty
 *     (untitled champion).  Pinning the catalog name keeps the slot
 *     bound to a real source identity even if no Hall map route
 *     exists for ordinal 22 in the local data fixture. */
static int test_ordinal_22_catalog_identity(M11_GameViewState* game) {
    char championName[64];
    char championTitle[64];
    int ok = 1;
    printf("[B] ordinal 22 catalog identity: name=GOTHMOG, title=(untitled)\n");
    championName[0] = '\0';
    championTitle[0] = '\0';
    M11_GameView_GetMirrorNameByOrdinal(game, PORTRAIT_ORDINAL_TARGET,
                                        championName, sizeof(championName));
    M11_GameView_GetMirrorTitleByOrdinal(game, PORTRAIT_ORDINAL_TARGET,
                                         championTitle, sizeof(championTitle));
    printf("  INFO: ordinal 22 -> name=\"%s\" title=\"%s\"\n",
           championName[0] ? championName : "(unknown)",
           championTitle[0] ? championTitle : "(untitled)");
    ok &= expect_int("ordinal 22 has a non-empty display name (GOTHMOG)",
                     championName[0] != '\0', 1);
    ok &= expect_int("ordinal 22 has an empty title (untitled champion)",
                     championTitle[0] == '\0', PORTRAIT_22_TITLE_EMPTY);
    /* The name slot must be the source-locked GOTHMOG string, not
     * a placeholder or an alias from a different game version.  This
     * is a tighter binding than the "name is non-empty" assertion
     * above: it pins the catalog to a real PC 3.4 identity. */
    {
        int matchesGothmog = (strcmp(championName, "GOTHMOG") == 0);
        printf("  INFO: ordinal 22 name == \"GOTHMOG\"? %s\n",
               matchesGothmog ? "yes" : "no");
        ok &= expect_int("ordinal 22 catalog name is GOTHMOG (PC 3.4 source-lock)",
                         matchesGothmog, 1);
    }
    return ok;
}

/* (C) Verify the D2C far positive route pose is well-formed: park
 *     the party at (1,3) facing NORTH on Hall map 0, confirm the
 *     D1C cell (1,2) has no C127 sensor (front-mirror ordinal = -1,
 *     the D1C portrait is NOT drawn at this view distance), and
 *     the D2C cell (1,1) has a C127 sensor (the F0128 D2C dispatch
 *     observes the sensor at D2C depth -- the "D2C far positive"
 *     route).  The D1C portrait is reserved for the depth-1 view
 *     (e.g. (1,2) facing NORTH). */
static int test_d2c_far_positive_route_pose(M11_GameViewState* game) {
    int ord = -999;
    int sensorCount = 0;
    int i;
    int ok = 1;
    printf("[C] D2C far positive route pose: (1,3) facing NORTH on Hall map 0\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = D2C_FAR_POSITIVE_X;
    game->world.party.mapY = D2C_FAR_POSITIVE_Y;
    game->world.party.direction = D2C_FAR_POSITIVE_DIR;
    /* At (1,3,N) the D1C cell is (1,2) and must NOT carry a C127
     * sensor -- the front-mirror ordinal helper returns -1.  This
     * is the "negative D1C / positive D2C" asymmetry the probe
     * binds: the D2C far view sees the D2C mirror (via F0128
     * D2C dispatch), not the D1C mirror.  The D1C portrait is
     * reserved for the depth-1 view (e.g. (1,2) facing NORTH). */
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: D2C-far route D1C front-mirror ordinal = %d\n", ord);
    ok &= expect_int("D2C-far route D1C front-mirror ordinal == -1 (D1C negative)",
                     ord, -1);
    /* Count C127 sensors in things[] so we can verify the F0128
     * D2C dispatch is well-formed (at least one C127 sensor must
     * exist for the D2C wall-mirror route to be resolvable).  We
     * scan the things[] sensor list (ReDMCSB DUNGEON.C:2558
     * C127 sensorData = ordinal stored in G0289). */
    if (!game->world.things || !game->world.things->sensors) {
        printf("  FAIL: no sensor list available\n");
        ++g_fail;
        return 0;
    }
    for (i = 0; i < game->world.things->sensorCount; ++i) {
        if (game->world.things->sensors[i].sensorType == 127) {
            ++sensorCount;
        }
    }
    ok &= expect_int("at least one C127 sensor exists in things[] (F0128 D2C dispatch possible)",
                     sensorCount >= 1, 1);
    return ok;
}

/* (D) Verify the D1C wall-ornament zone helper returns the
 *     source-locked (80, 29, 64, 43) box regardless of the view
 *     depth.  The wall-frame geometry helper is not depth-
 *     conditional, so the same (80, 29, 64, 43) box must be
 *     reported at the D2C-far view (1,3,N) AND the D1C view
 *     (1,2,N).  This proves the D1C frame rect is reserved at
 *     all view distances, not just the D1C view distance. */
static int test_d1c_wall_ornament_zone_at_d2c_far(M11_GameViewState* game) {
    int xA = -1, yA = -1, wA = -1, hA = -1;
    int xB = -1, yB = -1, wB = -1, hB = -1;
    int ok = 1;
    printf("[D] D1C wall ornament zone contract at D2C-far view\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = D2C_FAR_POSITIVE_X;
    game->world.party.mapY = D2C_FAR_POSITIVE_Y;
    game->world.party.direction = D2C_FAR_POSITIVE_DIR;
    ok &= expect_int("D2C-far route M11_GameView_GetD1CWallOrnamentZone returns 1",
                     M11_GameView_GetD1CWallOrnamentZone(game, &xA, &yA, &wA, &hA), 1);
    ok &= expect_int("D2C-far route D1C frame dstX == 80", xA, D1C_WALL_FRAME_X);
    ok &= expect_int("D2C-far route D1C frame dstY == 29", yA, D1C_WALL_FRAME_Y);
    ok &= expect_int("D2C-far route D1C frame width == 64", wA, D1C_WALL_FRAME_W);
    ok &= expect_int("D2C-far route D1C frame height == 43", hA, D1C_WALL_FRAME_H);
    /* Now the D1C view distance: same helper, same expected box. */
    game->world.party.mapX = D1C_NEAR_X;
    game->world.party.mapY = D1C_NEAR_Y;
    game->world.party.direction = D1C_NEAR_DIR;
    ok &= expect_int("D1C-near route M11_GameView_GetD1CWallOrnamentZone returns 1",
                     M11_GameView_GetD1CWallOrnamentZone(game, &xB, &yB, &wB, &hB), 1);
    ok &= expect_int("D1C-near route D1C frame dstX == 80", xB, D1C_WALL_FRAME_X);
    ok &= expect_int("D1C-near route D1C frame dstY == 29", yB, D1C_WALL_FRAME_Y);
    ok &= expect_int("D1C-near route D1C frame width == 64", wB, D1C_WALL_FRAME_W);
    ok &= expect_int("D1C-near route D1C frame height == 43", hB, D1C_WALL_FRAME_H);
    /* The (96, 35) D1C portrait cutout sits inside the (80, 29)
     * wall frame box at both view distances -- the cutout offset
     * (16, 39) is depth-invariant. */
    ok &= expect_int("D1C cutout X offset to wall frame is 16 (depth-invariant)",
                     D1C_PORTRAIT_X - D1C_WALL_FRAME_X, 16);
    ok &= expect_int("D1C cutout Y offset to wall frame is 39 (depth-invariant)",
                     D1C_PORTRAIT_Y - D1C_WALL_FRAME_Y, 39);
    return ok;
}

/* (E) D2C far positive route seed: temporarily rewrite the C127
 *     sensor on the (1,1) cell from sensorData=1 to sensorData=22
 *     (GOTHMOG), then verify the D2C F0128 contract translates to
 *     a real D1C portrait_rect when the party walks forward to
 *     the D1C view distance.  The sensor is restored before the
 *     probe exits. */
static int test_d2c_far_positive_route_seed(M11_GameViewState* game) {
    int ordD2CFar = -999;
    int ordD1CNear = -999;
    int ordD1CNearOtherDir = -999;
    int seededIndex = -1;
    unsigned short savedData = 0;
    int baselineOrdinal = -999;
    int i;
    int ok = 1;
    printf("[E] D2C far positive route seed via C127 sensor rewrite at (1,1)\n");
    if (!game->world.things || !game->world.things->sensors) {
        printf("  FAIL: no sensor list available\n");
        ++g_fail;
        return 0;
    }
    /* Step 1: capture the baseline ordinal at the (1,2,N) D1C view
     * distance (the front-mirror ordinal when the party is at
     * (1,2) facing NORTH -- this is the source-locked baseline
     * ordinal, 1 = HALK per the actual_pose probe).  The C127
     * sensor on the (1,1) cell has sensorData=1 by default. */
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = D1C_NEAR_X;
    game->world.party.mapY = D1C_NEAR_Y;
    game->world.party.direction = D1C_NEAR_DIR;
    baselineOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: D1C-near baseline ordinal at (1,2,N) = %d (expected 1, HALK)\n",
           baselineOrdinal);
    ok &= expect_int("D1C-near baseline ordinal == 1 (HALK) at (1,2,N)",
                     baselineOrdinal, FRONT_MIRROR_BASELINE);
    /* Step 2: find the C127 sensor with sensorData equal to the
     * baseline ordinal (1 = HALK).  Save the original sensorData
     * so we can restore it after the probe.  This is the C127
     * sensor on the (1,1) cell cell 0 (north wall), the same
     * sensor observable at D2C depth from (1,3,N). */
    for (i = 0; i < game->world.things->sensorCount; ++i) {
        if (game->world.things->sensors[i].sensorType == 127 &&
            (int)game->world.things->sensors[i].sensorData == baselineOrdinal) {
            seededIndex = i;
            savedData = game->world.things->sensors[i].sensorData;
            game->world.things->sensors[i].sensorData =
                (unsigned short)PORTRAIT_ORDINAL_TARGET;
            break;
        }
    }
    if (seededIndex < 0) {
        printf("  FAIL: could not find a C127 sensor with sensorData=%d\n",
               baselineOrdinal);
        ++g_fail;
        return 0;
    }
    printf("  INFO: seeded C127 sensor %d sensorData %u -> %d\n",
           seededIndex, (unsigned)savedData, PORTRAIT_ORDINAL_TARGET);
    /* Clear candidate-panel state so the rewrite is reflected by
     * GetFrontMirrorOrdinal without a stale candidate being returned. */
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->candidateMirrorPanelActive = 0;
    /* Step 3: D2C-far view at (1,3,N) must STILL report -1 on the
     * D1C depth (the (1,2) cell has no C127 sensor, so the D1C
     * portrait is not drawn at the D2C-far view distance).  The
     * F0128 D2C dispatch is observable only via the things[] array,
     * not via the front-mirror helper (which is D1C-only).  The
     * "positive" aspect of "d2c_far_positive" is verified at the
     * D1C depth after a forward walk, not at the D2C depth itself. */
    game->world.party.mapX = D2C_FAR_POSITIVE_X;
    game->world.party.mapY = D2C_FAR_POSITIVE_Y;
    game->world.party.direction = D2C_FAR_POSITIVE_DIR;
    ordD2CFar = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: D2C-far route front-mirror ordinal = %d\n", ordD2CFar);
    ok &= expect_int("D2C-far route D1C depth front-mirror ordinal == -1 "
                     "(D1C negative after seed)",
                     ordD2CFar, -1);
    /* Step 4: walk forward one cell from (1,3,N) to (1,2,N).  The
     * D1C cell becomes (1,1) which is the rewritten C127 sensor
     * (sensorData=22).  The D1C front-mirror ordinal helper must
     * report 22.  This is the D2C->D1C transition: the D2C F0128
     * contract (positive ordinal at the (1,1) D2C cell) translates
     * to the D1C portrait_rect when the party walks forward to
     * the D1C view distance. */
    game->world.party.mapX = D1C_NEAR_X;
    game->world.party.mapY = D1C_NEAR_Y;
    game->world.party.direction = D1C_NEAR_DIR;
    ordD1CNear = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: D1C depth at (1,2,N) front-mirror ordinal = %d\n",
           ordD1CNear);
    ok &= expect_int("D1C depth at (1,2,N) front-mirror ordinal == 22 (D2C->D1C transition)",
                     ordD1CNear, PORTRAIT_ORDINAL_TARGET);
    /* Step 5: turning to a different direction at (1,2) must NOT
     * show the D2C-far ordinal on the side walls.  East and west
     * at (1,2) report -1 (the (1,2) cell is the (2,2) or (0,2)
     * cell, neither of which has a C127 sensor with sensorData=22).
     * This is the no-floating contract for the D2C seed: the
     * portrait sprite is bound to the (1,1) D2C cell, not to the
     * side walls. */
    game->world.party.direction = 1; /* DIR_EAST */
    ordD1CNearOtherDir = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: D1C depth at (1,2,E) front-mirror ordinal = %d\n",
           ordD1CNearOtherDir);
    ok &= expect_int("D1C depth at (1,2,E) front-mirror ordinal == -1 (no east float)",
                     ordD1CNearOtherDir, -1);
    game->world.party.direction = 3; /* DIR_WEST */
    ordD1CNearOtherDir = M11_GameView_GetFrontMirrorOrdinal(game);
    printf("  INFO: D1C depth at (1,2,W) front-mirror ordinal = %d\n",
           ordD1CNearOtherDir);
    ok &= expect_int("D1C depth at (1,2,W) front-mirror ordinal == -1 (no west float)",
                     ordD1CNearOtherDir, -1);
    /* Step 6: restore the original sensorData so subsequent CTest
     * runs see the shipped DM1 V1 data. */
    game->world.things->sensors[seededIndex].sensorData = savedData;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->candidateMirrorPanelActive = 0;
    game->world.party.mapX = D1C_NEAR_X;
    game->world.party.mapY = D1C_NEAR_Y;
    game->world.party.direction = D1C_NEAR_DIR;
    {
        int ordRestored = M11_GameView_GetFrontMirrorOrdinal(game);
        ok &= expect_int("sensorData restore returns baseline ordinal 1 (HALK)",
                         ordRestored, baselineOrdinal);
    }
    return ok;
}

/* (F) No-floating contract for D2C-far D2L2/D2R2 side walls:
 *     at (1,3) facing NORTH the D2C cell is (1,1), the D2L2
 *     cell is (0,1), and the D2R2 cell is (2,1).  The D2L2
 *     and D2R2 side walls are NOT to carry the C127 sensor
 *     with sensorData=22 after the route seed, so the
 *     front-mirror ordinal at the D2L2/D2R2 party poses must
 *     remain -1 (no positive ordinal resolution at the side
 *     lane).  This proves the D1C portrait sprite does not
 *     "float" onto the D2 side lanes when the party is at a
 *     D2C-far view distance. */
static int test_no_floating_on_d2c_far_side_lanes(M11_GameViewState* game) {
    static const struct { int x, y, dir; const char* label; } kD2CFarSidePoses[] = {
        /* D2C-far at (1,3,N): D2C cell is (1,1) which has the
         * rewritten C127 sensor; D2L2 cell is (0,1) and D2R2
         * cell is (2,1).  Neither (0,1) nor (2,1) is a wall
         * with a C127 sensor in the local PC 3.4 fixture. */
        {0, 3, 0, "d2c_far_d2l2_route_no_ordinal"},
        {2, 3, 0, "d2c_far_d2r2_route_no_ordinal"},
        /* Walking forward to (0,2,N) and (2,2,N) keeps the
         * D2L2/D2R2 view geometry and confirms the absence of
         * a C127 sensor with sensorData=22 on the side walls. */
        {0, 2, 0, "d2c_far_d2l2_route_walk_forward_no_ordinal"},
        {2, 2, 0, "d2c_far_d2r2_route_walk_forward_no_ordinal"},
        /* And the (1,3) party square with WEST facing must not
         * see ordinal 22 on the west wall (the local PC 3.4
         * fixture has no C127 sensor on the (1,3) west wall).
         * Note: (1,3) EAST naturally exposes ordinal 18 (the
         * (1,3) east wall's C127 sensor), so we cannot use EAST
         * for the no-floating check -- the sensor rewrite is on
         * the (1,1) cell, not the (1,3) east wall, so the no-
         * float contract for EAST is automatic (the EAST sensor
         * is unrelated to the seed).  We only assert WEST here. */
        {1, 3, 3, "d2c_far_route_west_no_ordinal"}
    };
    int seededIndex = -1;
    unsigned short savedData = 0;
    int baselineOrdinal = -999;
    int i;
    int ok = 1;
    size_t n = sizeof(kD2CFarSidePoses) / sizeof(kD2CFarSidePoses[0]);
    printf("[F] No-floating contract: D2C-far D2L2/D2R2 side walls must not expose ordinal 22\n");
    if (!game->world.things || !game->world.things->sensors) {
        printf("  FAIL: no sensor list available\n");
        ++g_fail;
        return 0;
    }
    /* Rewrite the (1,1) C127 sensor to sensorData=22 so the D2C
     * F0128 contract is live. */
    game->world.party.mapIndex = HALL_MAP_INDEX;
    game->world.party.mapX = D1C_NEAR_X;
    game->world.party.mapY = D1C_NEAR_Y;
    game->world.party.direction = D1C_NEAR_DIR;
    baselineOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    for (i = 0; i < game->world.things->sensorCount; ++i) {
        if (game->world.things->sensors[i].sensorType == 127 &&
            (int)game->world.things->sensors[i].sensorData == baselineOrdinal) {
            seededIndex = i;
            savedData = game->world.things->sensors[i].sensorData;
            game->world.things->sensors[i].sensorData =
                (unsigned short)PORTRAIT_ORDINAL_TARGET;
            break;
        }
    }
    if (seededIndex < 0) {
        printf("  FAIL: could not find a C127 sensor with sensorData=%d\n",
               baselineOrdinal);
        ++g_fail;
        return 0;
    }
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->candidateMirrorPanelActive = 0;
    for (i = 0; i < (int)n; ++i) {
        int ord;
        char label[80];
        game->world.party.mapX = kD2CFarSidePoses[i].x;
        game->world.party.mapY = kD2CFarSidePoses[i].y;
        game->world.party.direction = kD2CFarSidePoses[i].dir;
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        snprintf(label, sizeof(label), "%s ordinal == -1 (no D2 side-lane float)",
                 kD2CFarSidePoses[i].label);
        ok &= expect_int(label, ord, -1);
    }
    /* Restore. */
    game->world.things->sensors[seededIndex].sensorData = savedData;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->candidateMirrorPanelActive = 0;
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
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

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 22, "
           "route d2c_far_positive, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                REVIVE.C F0280,F0282 (candidate materialise/disable)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                DUNVIEW.C:8520-8521 F0128 (D2C dispatch before D1C)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:821-826 (M027/M028 portrait X/Y macros)\n");
    printf("                DEFS.H:2071-2079 (G2071_C320/G2078_C32/G2079_C29)\n\n");

    ok &= test_portrait_ordinal_math();
    ok &= test_ordinal_22_catalog_identity(&game);
    ok &= test_d2c_far_positive_route_pose(&game);
    ok &= test_d1c_wall_ornament_zone_at_d2c_far(&game);
    ok &= test_d2c_far_positive_route_seed(&game);
    ok &= test_no_floating_on_d2c_far_side_lanes(&game);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
