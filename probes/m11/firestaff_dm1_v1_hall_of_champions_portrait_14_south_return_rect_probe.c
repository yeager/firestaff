/*
 * firestaff_dm1_v1_hall_of_champions_portrait_14_south_return_rect_probe.c
 *
 * Source-locked, real-asset runtime probe for DM1 V1 Hall of Champions
 * portrait ordinal 14 on the south_return route.
 *
 * Background:
 *   The local DM1 V1 DUNGEON.DAT carries a C127 champion-portrait
 *   sensor (ReDMCSB DUNGEON.C:2608-2612 / DEFS.H:2186) on the south
 *   wall of map 0 cell (1, 18) with sensorData=14.  The C026 graphic
 *   is the 256x87 atlas (ReDMCSB DEFS.H: M11_GFX_CHAMPION_PORTRAITS)
 *   of 24 portraits, 8 cols x 3 rows of 32x29 cells, so ordinal 14
 *   = atlas (col 6, row 1) = source rect (192, 29, 32, 29).
 *
 *   To see this sensor the party must stand at map 0 (1, 19)
 *   facing DIR_NORTH so the view-cone front cell lands on (1, 18)
 *   and the visible-wall side (DIR_NORTH + 2 == SOUTH = 2) matches
 *   the sensor's wall side (cell = 2).  Map 0 is 18x19 so the
 *   party at y=19 is one cell south of the map edge; this is the
 *   only party pose that resolves frontMirrorOrdinal to 14.  The
 *   in-bounds (1, 17) facing DIR_SOUTH and (0, 18) facing DIR_EAST
 *   poses both fail the visible-wall-side filter, and (1, 18)
 *   facing DIR_NORTH puts the front cell on the corridor square
 *   (1, 17) instead of (1, 18).  No normal in-game party step
 *   can reach this route — the engine reaches (1, 18)'s south
 *   wall only when sampling the world past the south map edge.
 *   The probe therefore parks the party at the canonical
 *   south_return pose directly, the same way the existing
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *   parks at (1, 2) and (1, 5) to reach the front-mirror routes.
 *
 * What this proves:
 *   A. The C127 sensor on (1, 18) south wall carries
 *      sensorData=14, so the south_return route is owned by
 *      portrait ordinal 14 (ReDMCSB DUNGEON.C:2608-2612,
 *      MOVESENS.C:1501-1503, REVIVE.C F0280).
 *   B. The canonical front-route pose (1, 19) facing DIR_NORTH
 *      resolves the front-mirror ordinal to 14 via
 *      m11_front_cell_mirror_ordinal (m11_game_view.c:11652).
 *   C. The public D1C wall-ornament zone helper
 *      M11_GameView_GetD1CWallOrnamentZone returns the source-locked
 *      destination box (80, 29, 64, 43) which contains the
 *      portrait cutout (96, 35, 32, 29) from
 *      G0109_auc_Graphic558_Box_ChampionPortraitOnWall =
 *      { 96, 127, 35, 63 } (ReDMCSB DUNVIEW.C:525 / 3913-3928).
 *   D. The C026 champion portrait cell at atlas (col 6, row 1)
 *      matches the (96, 35, 32, 29) cutout in the rendered frame
 *      by at least 90% of non-transparent pixels, while the four
 *      adjacent atlas cells (5,1), (7,1), (6,0), (6,2) all stay
 *      below 30% — proving ordinal 14 specifically is drawn and
 *      not a neighbor.
 *   E. Stepping the party to any non-front-mirror pose
 *      ((0, 19) DIR_EAST, (2, 19) DIR_WEST, (1, 18) DIR_NORTH
 *      with front=(1, 17) corridor) yields frontMirrorOrdinal=-1
 *      AND the (96, 35, 32, 29) cutout does NOT match ordinal 14
 *      (match < 30%).  The D1C frame is not left "floating" over
 *      side walls or corridor floors.
 *   F. Re-entering (1, 19) DIR_NORTH restores ordinal 14 and the
 *      portrait cutout re-matches ordinal 14 above the 90%
 *      threshold.
 *   G. Ordinal 14 stays inside the mirror catalog range so
 *      m11_front_cell_mirror_ordinal's catalog-bound clamp
 *      (m11_game_view.c:11688-11692) accepts it.
 *
 * Non-claims:
 *   - We do not claim DOS pixel parity. The probe compares against
 *     the local C026 strip pulled from the same GRAPHICS.DAT the
 *     runtime is drawing from; that is a runtime-correctness check,
 *     not a pixel-for-pixel match against an external DOSBox
 *     reference.
 *   - We do not assume the south_return route is reachable in
 *     normal gameplay. The probe parks the party directly, the
 *     same way existing Hall-of-Champions probes reach the
 *     (1, 2)/(1, 5) front-mirror routes. Documented above.
 *
 * Source-lock map:
 *   ReDMCSB DUNGEON.C:2608-2612  C127 sensorData -> G0289
 *   ReDMCSB MOVESENS.C:1501-1503 C127 sensorData -> F0280
 *   ReDMCSB REVIVE.C F0280       materialize candidate from sensorData
 *   ReDMCSB DUNVIEW.C:525        G0109_box = { 96, 127, 35, 63 }
 *   ReDMCSB DUNVIEW.C:3913-3928  D1C champion portrait blit
 *   ReDMCSB DUNVIEW.C:8318-8618  F0128 far-to-near viewport redraw
 *   ReDMCSB DEFS.H:2552          M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   ReDMCSB DEFS.H:2186          C127_SENSOR_WALL_CHAMPION_PORTRAIT
 *
 * Firestaff anchors:
 *   src/engine/m11_game_view.c:11652  m11_front_cell_mirror_ordinal
 *   src/engine/m11_game_view.c:13952  m11_draw_dm1_front_champion_portrait
 *   src/engine/m11_game_view.c:14054  m11_draw_dm1_front_mirror_route
 *   src/engine/m11_game_view.c:7881   M11_GameView_GetFrontMirrorOrdinal
 *   src/engine/m11_game_view.c:7885   M11_GameView_GetD1CWallOrnamentZone
 *   src/engine/m11_game_view.c:13962  blit dst = (M11_VIEWPORT_X + 96, M11_VIEWPORT_Y + 35)
 *   src/engine/m11_game_view.c:13972-13975 atlas addressing = (ordinal & 7) * 32, (ordinal >> 3) * 29
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    D1C_X = 96,
    D1C_Y = 35,
    D1C_W = 32,
    D1C_H = 29,
    /* Source-locked against ReDMCSB DEFS.H:2088 (G2078_C32_PortraitWidth=32,
     * G2079_C29_PortraitHeight=29). */
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* Match thresholds.  After skipping transparent source pixels
     * (palette index 1 == blitter transparentColor from
     * m11_game_view.c:13975) and dark gray (index 12, the
     * mirror_zones_probe's transparency convention) the C026
     * portrait cell matches the rendered cutout by >= 90% for the
     * correct ordinal and stays below 30% for adjacent atlas
     * cells.  The wide gap is intentional: the C026 strip shares
     * skin/clothing palette indices across nearby cells, so a
     * single 30% cut-point reliably catches "wrong atlas cell". */
    CORRECT_MATCH_PCT = 90,
    WRONG_MATCH_PCT = 30
};

/* Champion ordinal assigned to the south_return route.  Source-locked
 * to the C127 sensor at (map 0, x=1, y=18) south wall (sensor idx=21)
 * per the firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 * sensor scan. */
enum {
    SOUTH_RETURN_ORDINAL = 14
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count distinct non-zero palette indices in a viewport-rectangle. */
static int count_distinct(const unsigned char* fb,
                          int vx, int vy, int w, int h) {
    unsigned char seen[16] = {0};
    int n = 0;
    int xx, yy;
    int x0 = VIEWPORT_X + vx;
    int y0 = VIEWPORT_Y + vy;
    for (yy = 0; yy < h; ++yy) {
        int y = y0 + yy;
        if (y < 0 || y >= FB_H) continue;
        for (xx = 0; xx < w; ++xx) {
            int x = x0 + xx;
            if (x < 0 || x >= FB_W) continue;
            unsigned char idx = (unsigned char)(fb[y * FB_W + x] & 0x0F);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

/* Pixel-match a viewport-rectangle against a 32x29 portrait cell
 * within the C026 strip (graphics.dat asset slot M11_GFX_CHAMPION_
 * PORTRAITS = 26, atlas 256x87, 8 cols x 3 rows of 32x29 portraits).
 *
 * The m11_draw_dm1_front_champion_portrait blit passes
 * transparentColor=1 (m11_game_view.c:13952 -> BlitRegion(..., 1)),
 * so any C026 pixel with palette index 1 leaves the wall-niche
 * background visible.  We therefore skip those pixels on BOTH
 * sides of the comparison so transparent source pixels are not
 * counted as comparisons.  PROBE_COLOR_DARK_GRAY (=12) is the
 * wall niche backdrop that the existing
 * firestaff_dm1_v1_hall_of_champions_mirror_zones_probe treats
 * as transparent; we skip that on the source side too.
 *
 * Returns matched-percent (0..100) or -1 if the asset is missing. */
static int match_portrait_cell(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y;
    int srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if ((int)portraits->width < 8 * PORTRAIT_W) return -1;
    if ((int)portraits->height < 3 * PORTRAIT_H) return -1;
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)(
                portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            unsigned char dst = (unsigned char)(
                fb[(VIEWPORT_Y + D1C_Y + y) * FB_W + (VIEWPORT_X + D1C_X + x)] & 0x0F);
            /* Skip blitter-transparent source pixels — the wall-niche
             * background shows through those, so source/dst cannot
             * agree and counting them would dilute the match. */
            if (src == 1) continue;
            /* Skip wall-niche dark-gray on the source side; the
             * mirror_zones probe also treats 12 as transparent. */
            if (src == 12) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Render one frame at the given party pose. */
static void render(M11_GameViewState* view,
                   unsigned char* fb,
                   int mapX, int mapY, int direction) {
    view->world.party.mapIndex = 0;
    view->world.party.mapX = mapX;
    view->world.party.mapY = mapY;
    view->world.party.direction = direction;
    view->showDebugHUD = 0;
    view->candidateMirrorPanelActive = 0;
    view->candidateMirrorOrdinal = -1;
    view->candidateMirrorPartyIndex = -1;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(view, fb, FB_W, FB_H);
}

/* Park the party at (1, 19) DIR_NORTH — the canonical south_return
 * route pose.  See the file header for why this position is OOB and
 * how the engine still resolves frontMirrorOrdinal to 14 there. */
static void park_south_return_route(M11_GameViewState* view) {
    view->world.party.mapIndex = 0;
    view->world.party.mapX = 1;
    view->world.party.mapY = 19;
    view->world.party.direction = 0; /* DIR_NORTH */
    view->world.party.championCount = 0;
    view->showDebugHUD = 0;
    view->candidateMirrorPanelActive = 0;
    view->candidateMirrorOrdinal = -1;
    view->candidateMirrorPartyIndex = -1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    unsigned char fb[FB_W * FB_H];
    const M11_AssetSlot* portraits;
    int assetsOk = 0;
    int ord;
    char mirrorName[16];
    char mirrorTitle[32];

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions: portrait ordinal %d south_return route ===\n",
           SOUTH_RETURN_ORDINAL);
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    assetsOk = (portraits && portraits->loaded && portraits->pixels &&
                portraits->width >= 8 * PORTRAIT_W &&
                portraits->height >= 3 * PORTRAIT_H);
    if (!assetsOk) {
        printf("  WARN: C026 portrait strip missing or too small; "
               "pixel-match groups will be skipped.\n");
    }

    /* ── A) Route identification ──────────────────────────────── */
    printf("\n[Group A] south_return route identification\n");
    park_south_return_route(&game);
    ord = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(1,19) DIR_NORTH front-mirror ordinal = %d (want %d)",
                 ord, SOUTH_RETURN_ORDINAL);
        CHECK(ord == SOUTH_RETURN_ORDINAL, msg);
    }

    mirrorName[0] = '\0';
    mirrorTitle[0] = '\0';
    if (ord == SOUTH_RETURN_ORDINAL) {
        M11_GameView_GetMirrorNameByOrdinal(&game, ord,
                                            mirrorName, sizeof(mirrorName));
        M11_GameView_GetMirrorTitleByOrdinal(&game, ord,
                                             mirrorTitle, sizeof(mirrorTitle));
        printf("  ordinal=%d name='%s' title='%s'\n",
               ord, mirrorName[0] ? mirrorName : "(none)",
               mirrorTitle[0] ? mirrorTitle : "(none)");
    }

    /* Confirm the public D1C wall-ornament zone helper returns the
     * source-locked destination box.  ReDMCSB DUNVIEW.C:3913-3928 +
     * G0109_box = { 96, 127, 35, 63 } ⇒ viewport rect
     * (80, 29, 64, 43).  See m11_game_view.c:7885. */
    {
        int ornX = 0, ornY = 0, ornW = 0, ornH = 0;
        M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
        printf("  D1C wall-ornament zone = (%d, %d, %d, %d) viewport\n",
               ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43,
              "D1C wall-ornament zone matches source-locked (80,29,64,43)");
        CHECK(D1C_X >= ornX && D1C_X + D1C_W <= ornX + ornW &&
              D1C_Y >= ornY && D1C_Y + D1C_H <= ornY + ornH,
              "portrait cutout (96,35,32,29) is inside the D1C wall box");
    }

    /* ── B) portrait_rect_position: D1C cutout at (96, 35) ────── */
    printf("\n[Group B] portrait_rect_position: D1C cutout at (96, 35, 32, 29)\n");
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);

    {
        int distinct = count_distinct(fb, D1C_X, D1C_Y, D1C_W, D1C_H);
        char msg[200];
        printf("  cutout (96,35,32,29): distinct=%d palette indices\n", distinct);
        snprintf(msg, sizeof(msg),
                 "cutout has >= 5 distinct palette indices "
                 "(wall-only would be 1-4; portrait needs skin+cloth+hair)");
        CHECK(distinct >= 5, msg);
    }

    /* ── C) The drawn portrait matches atlas (col=6, row=1) ───── */
    printf("\n[Group C] portrait_rect_position: pixel-match against ordinal %d atlas cell\n",
           SOUTH_RETURN_ORDINAL);
    if (assetsOk) {
        int pctWant = match_portrait_cell(portraits, fb, SOUTH_RETURN_ORDINAL);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout matches ordinal %d atlas cell >= %d%% (got %d%%)",
                     SOUTH_RETURN_ORDINAL, CORRECT_MATCH_PCT, pctWant);
            CHECK(pctWant >= CORRECT_MATCH_PCT, msg);
        }

        /* Cross-check: ensure the cutout does NOT match the four
         * adjacent atlas cells with similar pixel counts. */
        const int kAdjacents[4] = {
            SOUTH_RETURN_ORDINAL - 1,
            SOUTH_RETURN_ORDINAL + 1,
            SOUTH_RETURN_ORDINAL - 8,
            SOUTH_RETURN_ORDINAL + 8
        };
        const char* kAdjLabels[4] = {
            "ordinal-1 (col 5, row 1)",
            "ordinal+1 (col 7, row 1)",
            "ordinal-8 (col 6, row 0)",
            "ordinal+8 (col 6, row 2)"
        };
        int i;
        for (i = 0; i < 4; ++i) {
            int pctAdj = match_portrait_cell(portraits, fb, kAdjacents[i]);
            char msg[200];
            if (pctAdj < 0) continue;
            snprintf(msg, sizeof(msg),
                     "D1C cutout does NOT match %s <= %d%% (got %d%%)",
                     kAdjLabels[i], WRONG_MATCH_PCT, pctAdj);
            CHECK(pctAdj < WRONG_MATCH_PCT, msg);
        }
    } else {
        printf("  SKIP: assets unavailable\n");
    }

    /* ── D) No portrait pixels on side walls ──────────────────── */
    printf("\n[Group D] No portrait pixels on side walls or adjacent corridor\n");
    {
        unsigned char fbE[FB_W * FB_H];
        unsigned char fbW[FB_W * FB_H];
        unsigned char fbCorridor[FB_W * FB_H];
        int dE, dW, dCorridor;
        int pctE = -1, pctW = -1, pctCorridor = -1;

        /* D.1 — Step east to (0, 19) DIR_EAST (party OOB at west
         * edge of the map, front cell (1, 19) is also OOB).  The
         * visible-wall-side filter fails because DIR_EAST + 2 =
         * WEST ≠ SOUTH = sensor cell.  Prove the cutout does NOT
         * match ordinal 14. */
        render(&game, fbE, 0, 19, 1 /* DIR_EAST */);
        ord = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(0,19) DIR_EAST front-mirror ordinal = %d (want -1)",
                     ord);
            CHECK(ord == -1, msg);
        }
        dE = count_distinct(fbE, D1C_X, D1C_Y, D1C_W, D1C_H);
        printf("  (0,19) E cutout: distinct=%d\n", dE);
        if (assetsOk) {
            pctE = match_portrait_cell(portraits, fbE, SOUTH_RETURN_ORDINAL);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "(0,19) E cutout does NOT match ordinal %d <= %d%% (got %d%%)",
                         SOUTH_RETURN_ORDINAL, WRONG_MATCH_PCT, pctE);
                CHECK(pctE < WRONG_MATCH_PCT, msg);
            }
        }

        /* D.2 — Step west to (2, 19) DIR_WEST (mirrored edge). */
        render(&game, fbW, 2, 19, 3 /* DIR_WEST */);
        ord = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(2,19) DIR_WEST front-mirror ordinal = %d (want -1)",
                     ord);
            CHECK(ord == -1, msg);
        }
        dW = count_distinct(fbW, D1C_X, D1C_Y, D1C_W, D1C_H);
        printf("  (2,19) W cutout: distinct=%d\n", dW);
        if (assetsOk) {
            pctW = match_portrait_cell(portraits, fbW, SOUTH_RETURN_ORDINAL);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "(2,19) W cutout does NOT match ordinal %d <= %d%% (got %d%%)",
                         SOUTH_RETURN_ORDINAL, WRONG_MATCH_PCT, pctW);
                CHECK(pctW < WRONG_MATCH_PCT, msg);
            }
        }

        /* D.3 — In-bounds (1, 18) DIR_NORTH: front=(1, 17)
         * corridor.  The (1, 18) south wall sensor is on the wall
         * square itself, but the front cell is the corridor square
         * (1, 17), so the engine does not draw the wall.  No
         * portrait cutout at (96, 35, 32, 29). */
        render(&game, fbCorridor, 1, 18, 0 /* DIR_NORTH */);
        ord = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(1,18) DIR_NORTH front-mirror ordinal = %d (want -1)",
                     ord);
            CHECK(ord == -1, msg);
        }
        dCorridor = count_distinct(fbCorridor, D1C_X, D1C_Y, D1C_W, D1C_H);
        printf("  (1,18) N cutout: distinct=%d\n", dCorridor);
        if (assetsOk) {
            pctCorridor = match_portrait_cell(portraits, fbCorridor, SOUTH_RETURN_ORDINAL);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "(1,18) N cutout does NOT match ordinal %d <= %d%% (got %d%%)",
                         SOUTH_RETURN_ORDINAL, WRONG_MATCH_PCT, pctCorridor);
                CHECK(pctCorridor < WRONG_MATCH_PCT, msg);
            }
        }
    }

    /* ── E) Re-entry restores the portrait ────────────────────── */
    printf("\n[Group E] Re-entering (1,19) DIR_NORTH restores ordinal %d portrait\n",
           SOUTH_RETURN_ORDINAL);
    {
        unsigned char fbRe[FB_W * FB_H];
        int pctRe = -1;
        park_south_return_route(&game);
        ord = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "re-entered (1,19) DIR_NORTH ordinal = %d (want %d)",
                     ord, SOUTH_RETURN_ORDINAL);
            CHECK(ord == SOUTH_RETURN_ORDINAL, msg);
        }
        memset(fbRe, 0, sizeof(fbRe));
        M11_GameView_Draw(&game, fbRe, FB_W, FB_H);
        if (assetsOk) {
            pctRe = match_portrait_cell(portraits, fbRe, SOUTH_RETURN_ORDINAL);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "re-entry D1C cutout matches ordinal %d >= %d%% (got %d%%)",
                         SOUTH_RETURN_ORDINAL, CORRECT_MATCH_PCT, pctRe);
                CHECK(pctRe >= CORRECT_MATCH_PCT, msg);
            }
        }
    }

    /* ── F) South-return sensor stays inside the mirror catalog ── */
    printf("\n[Group F] south_return ordinal stays inside the mirror catalog\n");
    {
        int catalogCount = M11_GameView_GetMirrorCatalogCount(&game);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal %d is within [0, %d) mirror catalog range",
                 SOUTH_RETURN_ORDINAL, catalogCount);
        CHECK(SOUTH_RETURN_ORDINAL >= 0 && SOUTH_RETURN_ORDINAL < catalogCount, msg);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
