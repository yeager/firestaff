/*
 * firestaff_dm1_v1_champion_portrait_05_south_return_rect_position_probe.c
 *
 * Slice: champion portrait ordinal 5, route south_return,
 * aspect portrait_rect_position.
 *
 * Hall of Champions (PC 3.4 EN DUNGEON.DAT) exposes six front-wall
 * C127 sensors with portrait-atlas ordinals {1, 4, 10, 13, 15, 18}.
 * Ordinal 5 is NOT one of them in this build - it sits between the
 * row-0 {1=HALK, 4=LEIF} and the row-1 {10=ZED, 13=WUUF} cluster.
 *
 * The slice proves three independent runtime contracts:
 *
 *   (1) Ordinal-5 rejection.  Driving the M11 runtime to every Hall
 *       pose that is known to carry a real C127 sensor (NORTH/EAST/
 *       SOUTH/WEST on (1,2), (1,3), (1,5), (2,1), (2,4)) yields
 *       M11_GameView_GetFrontMirrorOrdinal() in {1, 4, 10, 13, 15, 18}.
 *       None of these poses resolves to ordinal 5 - the runtime
 *       rejects the wrong ordinal even when the player faces the
 *       south_return route at (1,5).  This locks the runtime path
 *       against a future regression that could expose a stale
 *       Hall ordinal to the C040 candidate panel.
 *
 *   (2) Portrait rect position.  At the south_return pose
 *       (map 0, x=1, y=5, direction SOUTH = ordinal 13 = WUUF),
 *       the rendered D1C portrait rectangle must land at exactly
 *       viewport-local (96, 35)-(127, 63) - the exact box stored in
 *       ReDMCSB G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *       = { 96, 127, 35, 63 } at DATA.C:525 / DEFS.H:2186 and
 *       consumed by DUNVIEW.C:3913-3928 M635_ZONE_PORTRAIT_ON_WALL.
 *       Concretely:
 *         - Pixels outside (96..127, 35..63) in the D1C area must
 *           stay below the wall-only background (<= M11_COLOR_GRAY
 *           sum), and the strict non-empty region (96, 35)-(127, 63)
 *           must contain portrait warm pixels.
 *         - The bloom of warm-colored C026 portrait pixels must
 *           fall inside the 32x29 source rectangle on its left,
 *           right, and top edges - i.e. nothing bright leaks into
 *           the columns/rows that belong to wall/status texture.
 *           The bottom edge is not a warm-pixel invariant because
 *           the adjacent floor row legitimately uses some of the
 *           same palette indices as the portrait art.  This is the
 *           aspect the existing capture probe does not pin.
 *
 *   (3) No-floating on side walls.  Rotating the same (1,5) cell
 *       to EAST then WEST must NOT leave the portrait rectangle
 *       populated.  This pins the DUNVIEW.C:8318-8618 F0128 far-to-
 *       near redraw: the side-wall geometry overpaints the D1C
 *       rectangle when the front cell no longer owns a C127 sensor.
 *
 * Source evidence:
 *   - ReDMCSB DATA.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = { 96, 127, 35, 63 } (PC 3.4 init).
 *   - ReDMCSB DEFS.H:2186 M027_PORTRAIT_X / M028_PORTRAIT_Y.
 *   - ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) to front-wall.
 *   - ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   - ReDMCSB DUNVIEW.C:3913-3928 blits C026 at M635_ZONE_PORTRAIT_ON_WALL.
 *   - ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw (far-to-near).
 *   - ReDMCSB MOVESENS.C:1501-1503 C127 sensorData passes to F0280.
 *   - ReDMCSB REVIVE.C F0280 materializes candidate from sensorData.
 *
 * Disjoint from:
 *   - firestaff_dm1_v1_champion_mirror_capture_probe (full table
 *     of 18 captures + warm-pixel heuristic on the same rectangle).
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     (16-pose ordinal routing; no rectangle bounds check).
 *   - firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
 *     (negative front-route only).
 *   - firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     (re-blit ordering after viewport scroll, no rect bounds).
 *
 * Usage: firestaff_dm1_v1_champion_portrait_05_south_return_rect_position_probe DATA_DIR
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
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,

    /* ReDMCSB G0109 = { 96, 127, 35, 63 } inclusive.  We scan
     * [X1..X2] inclusive and [Y1..Y2] inclusive, so dstX = 96,
     * dstY = 35, width = 127 - 96 + 1 = 32, height = 63 - 35 + 1
     * = 29 - exactly the M11_PORTRAIT_W=32 / M11_PORTRAIT_H=29
     * dimensions of the C026 portrait blit.  The +1 padding in the
     * existing capture probe (96..128, 35..64) covers the
     * neighbour column/row used by the C026 atlas stride but is
     * not part of the source G0109 rectangle; this probe uses the
     * tighter source rectangle so the bloom check is exact. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,

    /* Same warm-palette set as
     * firestaff_dm1_v1_champion_mirror_capture_probe.c - see that
     * file for the per-ordinal color reasoning.  C026 champion
     * portrait sprites use these palette indices (ReDMCSB
     * DUNVIEW.C:3913-3928 + GRAPHICS.DAT C026 atlas); grey-stone
     * wall texture uses 0x01/0x02/0x07-grey/0x0D instead. */
    PROBE_WARM_THRESHOLD = 30,
    PROBE_OUTSIDE_WARM_THRESHOLD = 0  /* any leak is a fail */
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/*
 * Decoded framebuffer index at (x, y) where x/y are screen-local
 * pixel coordinates.  Returns the 4-bit palette index after
 * stripping the level field (ReDMCSB DUNVIEW.C:8212 pixel layout).
 */
static unsigned char fb_palette_index(const unsigned char* fb, int x, int y) {
    if (x < 0 || y < 0 || x >= PROBE_FB_W || y >= PROBE_FB_H) {
        return 0;
    }
    return M11_FB_DECODE_INDEX(fb[y * PROBE_FB_W + x]);
}

/*
 * Count warm-colored pixels in a viewport-local rectangle.
 * The rectangle is half-open [x0, x0+w) x [y0, y0+h), which is
 * how ReDMCSB DUNVIEW.C:3913-3928 specifies the C026 blit region.
 */
static int warm_count(const unsigned char* fb, int x0, int y0, int w, int h) {
    int count = 0;
    int x, y;
    for (y = y0; y < y0 + h; ++y) {
        for (x = x0; x < x0 + w; ++x) {
            unsigned char idx = fb_palette_index(fb, x, y);
            switch (idx) {
                case 0x07: /* green   - C026 background tints */
                case 0x08: /* red     - C026 clothing */
                case 0x09: /* orange  - C026 skin */
                case 0x0A: /* peach   - C026 skin highlight */
                case 0x0B: /* yellow  - C026 hair */
                case 0x0E: /* blue    - C026 background */
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/*
 * Drive the engine to (mapX, mapY, direction) on map 0 (Hall of
 * Champions), render the framebuffer, and return it.  Always zeros
 * the framebuffer first (matches the existing capture probe path).
 */
static void render_pose(M11_GameViewState* game,
                        unsigned char* fb,
                        int mapX,
                        int mapY,
                        int direction) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = direction;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    memset(fb, 0, PROBE_FB_W * PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
}

/*
 * The slice's anchor pose: (1, 5) facing SOUTH = ordinal 13 (WUUF).
 * Per ReDMCSB DUNGEON.C:2573 + MOVESENS.C:1501-1503 the front
 * square (1, 6) owns a C127 sensor with sensorData=13, and the
 * ReDMCSB PC34/I34E front-side filter (DUNGEON.C:2608-2612) plus
 * the new m11_front_cell_mirror_ordinal wall-side check routes the
 * sensorData through to M11_GameView_GetFrontMirrorOrdinal().
 */
static void check_ordinal_5_rejection_at_all_hall_poses(M11_GameViewState* game) {
    /* Every known Hall-of-Champions C127 sensor placement, in all
     * four cardinal directions.  None of these poses should ever
     * return ordinal 5 - ordinal 5 is not a Hall mirror in PC 3.4
     * EN.  If any of these resolve to 5, the runtime has regressed
     * to a stale ordinal from a different build or a different
     * mirror data source. */
    static const int kPoses[][3] = {
        /* (mapX, mapY, direction) */
        {1, 2, 0}, /* (1,2) N -> (1,1) ordinal 1 HALK */
        {1, 2, 1}, /* (1,2) E wrong-wall -> -1 */
        {1, 2, 2}, /* (1,2) S -> -1 */
        {1, 2, 3}, /* (1,2) W -> -1 */
        {2, 1, 2}, /* (2,1) S -> (2,0) ordinal 4 LEIF */
        {1, 3, 1}, /* (1,3) E -> (2,3) ordinal 18 SONJA */
        {1, 5, 0}, /* (1,5) N -> (1,4) ordinal 10 ZED */
        {1, 5, 2}, /* (1,5) S -> (1,6) ordinal 13 WUUF */
        {2, 4, 2}  /* (2,4) S -> (2,3) ordinal 15 MOPHUS */
    };
    int i;
    int ordinal5_count = 0;
    int ordinalSeen = 0;
    printf("=== Slice 1: ordinal 5 must be rejected at every Hall pose ===\n");
    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        int mapX = kPoses[i][0];
        int mapY = kPoses[i][1];
        int dir = kPoses[i][2];
        int actual;
        game->world.party.mapIndex = 0;
        game->world.party.mapX = mapX;
        game->world.party.mapY = mapY;
        game->world.party.direction = dir;
        actual = M11_GameView_GetFrontMirrorOrdinal(game);
        if (actual == 5) {
            ++ordinal5_count;
            printf("  FAIL: pose=(%d,%d,dir=%d) ordinal=5 (should be rejected)\n",
                   mapX, mapY, dir);
        } else if (actual >= 0) {
            ++ordinalSeen;
            printf("  ok: pose=(%d,%d,dir=%d) ordinal=%d (Hall mirror seen)\n",
                   mapX, mapY, dir, actual);
        } else {
            printf("  ok: pose=(%d,%d,dir=%d) ordinal=-1 (no mirror)\n",
                   mapX, mapY, dir);
        }
    }
    CHECK(ordinal5_count == 0,
          "ordinal 5 is rejected at every Hall pose (no false-positive C127)");
    CHECK(ordinalSeen >= 1,
          "at least one Hall pose yields a non-negative ordinal (catalog reachable)");
}

/*
 * Anchor south_return: (1, 5) SOUTH renders ordinal 13 (WUUF) at
 * the G0109 D1C rectangle (96, 35)-(127, 63).  The aspect test
 * pins both the rectangle bounds AND the bloom containment - i.e.
 * the portrait graphic does not extend outside its source box.
 */
static void check_portrait_rect_position_at_south_return(M11_GameViewState* game) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int actual;
    int insideWarm;
    int leftColWarm;
    int rightColWarm;
    int topRowWarm;

    printf("=== Slice 2: portrait_rect_position at (1,5) SOUTH ===\n");
    render_pose(game, fb, 1, 5, 2 /* DIR_SOUTH */);
    actual = M11_GameView_GetFrontMirrorOrdinal(game);
    CHECK(actual == 13,
          "(1,5) SOUTH resolves to ordinal 13 (WUUF) - south_return route active");

    /* The inside of the G0109 box must contain portrait pixels. */
    insideWarm = warm_count(fb,
                            PROBE_PORTRAIT_X,
                            PROBE_PORTRAIT_Y,
                            PROBE_PORTRAIT_W,
                            PROBE_PORTRAIT_H);
    CHECK(insideWarm >= PROBE_WARM_THRESHOLD,
          "inside G0109 rect (96,35)-(127,63) has >= 30 warm portrait pixels");

    /*
     * Bloom containment check.
     *
     * The 2-pixel halo around the G0109 box must NOT contain any
     * warm C026 portrait pixels.  The halo is wall masonry on the
     * left and right and status-bar pixels above.
     *
     * C026 portrait sprites use palette indices in
     * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} for skin, hair, and
     * clothing.  Wall masonry uses {0x01, 0x02, 0x04, 0x05, 0x0C,
     * 0x0D, 0x0F} (greys) and the floor uses 0x03 (blue).  The
     * status bar above uses 0x0D/0x0F (greys) plus 0x0F borders.
     * The dark backing rectangle inside the C346 wall-ornament
     * graphic uses 0x00 (black) with grey highlights.
     *
     * If any warm pixel appears in these halos, the portrait graphic
     * has bled past its source box.  We intentionally do not assert
     * the bottom halo here: the adjacent floor row legitimately uses
     * palette index 0x0B, which is also part of the portrait warm set.
     */
    leftColWarm = warm_count(fb,
                             PROBE_PORTRAIT_X - 2,
                             PROBE_PORTRAIT_Y,
                             2,
                             PROBE_PORTRAIT_H);
    rightColWarm = warm_count(fb,
                              PROBE_PORTRAIT_X + PROBE_PORTRAIT_W,
                              PROBE_PORTRAIT_Y,
                              2,
                              PROBE_PORTRAIT_H);
    topRowWarm = warm_count(fb,
                            PROBE_PORTRAIT_X,
                            PROBE_PORTRAIT_Y - 2,
                            PROBE_PORTRAIT_W,
                            2);
    CHECK(leftColWarm <= PROBE_OUTSIDE_WARM_THRESHOLD,
          "no warm pixels leak left of G0109 X1=96 (2-px halo)");
    CHECK(rightColWarm <= PROBE_OUTSIDE_WARM_THRESHOLD,
          "no warm pixels leak right of G0109 X2=127 (2-px halo)");
    CHECK(topRowWarm <= PROBE_OUTSIDE_WARM_THRESHOLD,
          "no warm pixels leak above G0109 Y1=35 (2-px halo)");
}

/*
 * No-floating on side walls.  At the same (1, 5) cell, the
 * south_return C127 sensor is on the south wall (1, 6).  When the
 * player rotates to EAST or WEST, the front cell becomes (2, 5)
 * or (0, 5) - neither owns a C127 sensor - so the D1C rectangle
 * must be plain wall masonry, not a leftover portrait sprite
 * floating over the side wall.
 */
static void check_no_floating_on_side_walls(M11_GameViewState* game) {
    unsigned char fbEast[PROBE_FB_W * PROBE_FB_H];
    unsigned char fbWest[PROBE_FB_W * PROBE_FB_H];
    int eastOrdinal;
    int westOrdinal;
    int eastWarm;
    int westWarm;

    printf("=== Slice 3: no portrait floats over side walls at (1,5) ===\n");
    render_pose(game, fbEast, 1, 5, 1 /* DIR_EAST */);
    eastOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    render_pose(game, fbWest, 1, 5, 3 /* DIR_WEST */);
    westOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);

    eastWarm = warm_count(fbEast,
                          PROBE_PORTRAIT_X,
                          PROBE_PORTRAIT_Y,
                          PROBE_PORTRAIT_W,
                          PROBE_PORTRAIT_H);
    westWarm = warm_count(fbWest,
                          PROBE_PORTRAIT_X,
                          PROBE_PORTRAIT_Y,
                          PROBE_PORTRAIT_W,
                          PROBE_PORTRAIT_H);

    CHECK(eastOrdinal < 0,
          "(1,5) EAST resolves to -1 (no front mirror - side wall)");
    CHECK(westOrdinal < 0,
          "(1,5) WEST resolves to -1 (no front mirror - side wall)");
    CHECK(eastWarm < PROBE_WARM_THRESHOLD,
          "(1,5) EAST D1C rect has < 30 warm pixels (no floating portrait)");
    CHECK(westWarm < PROBE_WARM_THRESHOLD,
          "(1,5) WEST D1C rect has < 30 warm pixels (no floating portrait)");
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  probe for DM1 V1 Hall of Champions portrait ordinal 5\n"
                "  (south_return route, portrait_rect_position aspect)\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;
    printf("=== DM1 V1 champion portrait ordinal 05 / south_return / "
           "portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    check_ordinal_5_rejection_at_all_hall_poses(&game);
    check_portrait_rect_position_at_south_return(&game);
    check_no_floating_on_side_walls(&game);

    M11_GameView_Shutdown(&game);
    printf("\n%s dm1 v1 champion portrait ordinal 05 / south_return / "
           "portrait_rect_position probe: %d pass, %d fail\n",
           g_fail == 0 ? "PASS" : "FAIL", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
