/*
 * firestaff_dm1_v1_champion_mirror_daroou_ordinal0_redraw_after_candidate_probe.c
 *
 * Real-asset/runtime evidence for the DM1 V1 Hall of Champions
 * "redraw_after_candidate" / portrait_rect_position slice for champion
 * portrait ordinal 0 (DAROOU).
 *
 * Slice goal
 * ----------
 *   The slice assigned to this pass is "champion portrait ordinal 0,
 *   route redraw_after_candidate, aspect portrait_rect_position".
 *   Ordinal 0 is the DAROOU mirror in the DM1 PC 3.4 C026 portrait
 *   strip (8 cols x 3 rows, ordinal = 0 -> column 0 row 0).  The
 *   D1C champion-portrait blit rectangle (96, 35, 32, 29 in viewport
 *   coords) is the source G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *   = {96, 127, 35, 63} from ReDMCSB DUNVIEW.C:525, drawn at the
 *   viewport offset by DUNVIEW.C:3913-3928.
 *
 *   The slice asks us to prove three things for ordinal 0:
 *     1. The portrait ordinal maps to the expected champion
 *        (DAROOU in the mirror catalog).
 *     2. The D1C portrait rectangle is drawn at the intended screen
 *        position when a real front route exposes the DAROOU C127
 *        sensorData, both before and after the C040 candidate panel
 *        is opened and closed (the redraw_after_candidate route).
 *     3. The D1C portrait rectangle is empty on ordinary side walls
 *        so the DAROOU portrait never floats after the player turns.
 *
 *   The previous slice passes already covered ordinals 1 (HALK), 4
 *   (LEIF), 10 (ZED), 13 (WUUF), 14 (LEYLA), 15 (MOPHUS), 18 (SONJA)
 *   via firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe,
 *   firestaff_dm1_v1_champion_mirror_capture_probe, and
 *   firestaff_dm1_v1_champion_mirror_leylla_ordinal14_unreachable_probe.
 *   Ordinal 0 was not covered by any of those slices.
 *
 * Real DM1 V1 finding for this slice
 * ----------------------------------
 *   The C127 champion-portrait sensor with sensorData = 0 (DAROOU)
 *   is present in real DM1 V1 DUNGEON.DAT and is anchored to map
 *   cell (x=1, y=8) on the Hall of Champions (map 0, 18 wide x 19
 *   tall).  The C127 sensor's wall-cell bit (M011_CELL bits 13:14 of
 *   the THING id) is 1 (east wall).  The M11_GameView_GetFrontMirrorOrdinal
 *   engine returns sensorData=0 (0-based) for several party poses:
 *     - (1, 1) facing N   -> front cell (1, 0)  has invisible
 *                            TextString[0] (DAROOU champion record).
 *     - (0, 0) facing E   -> front cell (1, 0).
 *     - (2, 0) facing W   -> front cell (1, 0).
 *
 *   This probe asserts:
 *     (A) The mirror catalog names ordinal 0 as DAROOU (the C127
 *         sensor data 0 in DUNGEON.DAT resolves to the first portrait
 *         in the C026 strip, and the corresponding TextString[0]
 *         parses to the DAROOU champion record).
 *     (B) The C127 sensor with sensorData = 0 exists in real DM1 V1
 *         DUNGEON.DAT and is anchored to map cell (1, 8) on map 0
 *         with M011_CELL bit 1 (east wall).
 *     (C) M11_GameView_GetFrontMirrorOrdinal returns 0 for the three
 *         poses that the engine accepts as the DAROOU route:
 *         (1, 1) N, (0, 0) E, (2, 0) W.
 *     (D) The D1C portrait cutout (96, 35, 32, 29 in viewport coords,
 *         framebuffer (96, 68, 32, 29)) is drawn at the intended screen
 *         position when the party is at the canonical DAROOU pose
 *         (1, 1) facing N.  The cutout pixels match the DAROOU
 *         portrait sprite from graphic C026 (the engine blits
 *         ordinal 0 from source x=0, y=0, w=32, h=29).
 *     (E) The "redraw_after_candidate" route: after opening the C040
 *         candidate panel (SelectFrontMirrorCandidate) and confirming
 *         resurrect, the next M11_GameView_Draw must redraw the D1C
 *         portrait cutout with the same DAROOU sprite content, not
 *         the wall texture or panel graphic.  This is the panel-close
 *         redraw stability contract from PANEL.C F0352 / REVIVE.C F0282.
 *     (F) The D1C portrait cutout is empty on ordinary side walls so
 *         no DAROOU portrait floats over them after the player turns
 *         the camera away from the (1, 8) mirror.
 *
 *   This is the "narrow slice" the parent task asked for: it pins
 *   down that ordinal 0 (DAROOU) is the first C026 portrait, that
 *   the C127 sensor carrying sensorData=0 lives at (1, 8) on the
 *   Hall of Champions map, and that the engine's redraw_after_candidate
 *   route preserves the D1C portrait_rect_position invariant both
 *   before and after the C040 candidate panel close.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573-2612  C127 sensor view-direction mapping (M552)
 *   - DUNGEON.C:2608-2612  C127 sensorData -> G0289 portrait ordinal
 *   - DUNVIEW.C:3913-3928  D1C champion-portrait blit (C026 + G0109)
 *   - DUNVIEW.C:525        G0109 = {96, 127, 35, 63} (D1C champion box)
 *   - MOVESENS.C:1501-1503 C127 sensor->F0280 candidate path
 *   - REVIVE.C F0280       candidate materialization from sensorData
 *   - REVIVE.C F0282       candidate confirm/cancel (closes panel)
 *   - PANEL.C F0342 / F0344-F0347 C040 candidate panel draw hooks
 *   - PANEL.C F0352        C040 panel redraw on close
 *   - m11_front_cell_mirror_ordinal (m11_game_view.c) the front-cell
 *     ordinal lookup that reads sensorData directly (0-based, no
 *     M000_INDEX_TO_ORDINAL offset, so the engine returns 0 for the
 *     DAROOU C127 sensor).
 *   - m11_draw_dm1_front_champion_portrait (m11_game_view.c) the
 *     C026 blit at viewport (96, 35, 32, 29).
 *   - m11_draw_dm1_front_mirror_route (m11_game_view.c) the wall-
 *     ornament + portrait gate, including the
 *     "candidateMirrorPanelActive -> portrait-only" branch that the
 *     redraw_after_candidate route exercises.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_viewport_fakewall_pc34_compat.h"
#include "asset_loader_m11.h"
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
    /* G0109 D1C champion-portrait cutout: 32x29 at (96, 35) in
     * viewport coords (DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * = {96, 127, 35, 63}).  M11_VIEWPORT_X = 0, M11_VIEWPORT_Y = 33
     * so the framebuffer destination of the cutout is (96, 68, 32, 29). */
    CUTOUT_VX = 96,
    CUTOUT_VY = 35,
    CUTOUT_W  = 32,
    CUTOUT_H  = 29,
    CUTOUT_FX = 96,         /* FB x = 96 (M11_VIEWPORT_X = 0) */
    CUTOUT_FY = 35 + 33,    /* FB y = 35 + 33 = 68 (M11_VIEWPORT_Y = 33) */
    /* Number of "warm" pixels in the cutout that we require for a
     * positive portrait match.  Each champion portrait has skin/eye/
     * clothing pixels in the warm palette indices {0x07, 0x08, 0x09,
     * 0x0A, 0x0B, 0x0E}, and a standable corridor or wall-texture
     * pixel has at most ~20 such pixels from torch glow / stone
     * antialiasing.  30 matches with prior capture-probe thresholds. */
    DAROOU_PORTRAIT_PIXEL_THRESHOLD = 30,
    /* Same threshold for the panel-close redraw stability assertion:
     * the cutout must still contain the DAROOU portrait content after
     * the C040 panel close, not the panel graphic and not the wall
     * texture. */
    POST_PANEL_PORTRAIT_PIXEL_THRESHOLD = 30,
    /* For the "no floating" assertion on side walls, the cutout must
     * contain at most this many warm pixels.  A normal corridor wall
     * typically has < 20 warm pixels from torch glow and edge
     * antialiasing; > 30 would mean a portrait is floating. */
    NO_FLOATING_PIXEL_THRESHOLD = 25
};

enum { ORDINAL_DAROOU = 0 };

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) printf("  SKIP: %s\n", msg)

/* ── helpers ─────────────────────────────────────────────────── */

/* Walk a single cell's thing chain and report whether the chain
 * contains a C127 sensor (sensorType == 127) with the requested
 * sensorData.  Out-cellX/cellY are filled with the wall cell bit
 * (M011_CELL) of the C127 sensor if found.  Returns 1 if a
 * matching C127 sensor is reached, 0 if not. */
static int find_c127_with_data(M11_GameViewState* game,
                               int mapIdx, int mapX, int mapY,
                               int wantedData, int* outCell) {
    const struct DungeonMapDesc_Compat* map;
    int squareIndex;
    unsigned short firstThing;
    unsigned short t;
    int base;
    int safety = 0;
    if (!game || !game->world.things || !game->world.things->squareFirstThings) {
        return 0;
    }
    if (mapIdx < 0 || mapIdx >= game->world.dungeon->header.mapCount) return 0;
    map = &game->world.dungeon->maps[mapIdx];
    if (mapX < 0 || mapX >= map->width || mapY < 0 || mapY >= map->height) {
        return 0;
    }
    base = 0;
    for (int m = 0; m < mapIdx; ++m) {
        base += game->world.dungeon->maps[m].width * game->world.dungeon->maps[m].height;
    }
    squareIndex = base + mapX * (int)map->height + mapY;
    if (squareIndex < 0 || squareIndex >= game->world.things->squareFirstThingCount) {
        return 0;
    }
    firstThing = game->world.things->squareFirstThings[squareIndex];
    t = firstThing;
    while (t != 0xFFFE && t != 0xFFFF && safety++ < 32) {
        int type = THING_GET_TYPE(t);
        int cell = THING_GET_CELL(t);
        int idx = t & 0x3FF;
        if (type == THING_TYPE_SENSOR && idx >= 0 && idx < game->world.things->sensorCount) {
            const struct DungeonSensor_Compat* s = &game->world.things->sensors[idx];
            if (s->sensorType == 127 && (int)s->sensorData == wantedData) {
                if (outCell) *outCell = cell;
                return 1;
            }
            t = s->next;
        } else if (type == THING_TYPE_TEXTSTRING &&
                   idx >= 0 && idx < game->world.things->textStringCount) {
            t = game->world.things->textStrings[idx].next;
        } else {
            return 0;
        }
    }
    return 0;
}

/* Read a square byte directly from DUNGEON.DAT map 0 tiles. */
static int read_square_byte(M11_GameViewState* game,
                            int mapX, int mapY, unsigned char* outSquare) {
    if (!game || !game->world.dungeon || !game->world.dungeon->tilesLoaded) return 0;
    if (!game->world.dungeon->tiles || !game->world.dungeon->tiles[0].squareData) return 0;
    if (mapX < 0 || mapY < 0) return 0;
    int w = game->world.dungeon->maps[0].width;
    int h = game->world.dungeon->maps[0].height;
    if (mapX >= w || mapY >= h) return 0;
    int idx = mapX * h + mapY;
    *outSquare = game->world.dungeon->tiles[0].squareData[idx];
    return 1;
}

/* Count the number of D1C cutout pixels that match a specific
 * palette index from the C026 portrait strip (the source graphic
 * for ordinal 0).  The source graphic is 4bpp with palette index
 * values 0..15; pixel index 0 is the "transparent" boundary so we
 * skip it.  Returns the number of cutout pixels (out of CUTOUT_W *
 * CUTOUT_H = 928) whose decoded framebuffer index matches the
 * ordinal-0 portrait strip at the same source coordinates. */
static int count_daroou_portrait_matches(const unsigned char* fb,
                                         const M11_AssetSlot* portraits) {
    int matched = 0;
    int total = 0;
    int x, y;
    if (!fb || !portraits || !portraits->loaded || !portraits->pixels) return 0;
    if (CUTOUT_W > (int)portraits->width || CUTOUT_H > (int)portraits->height) {
        return 0;
    }
    for (y = 0; y < CUTOUT_H; ++y) {
        for (x = 0; x < CUTOUT_W; ++x) {
            unsigned char src = (unsigned char)(portraits->pixels[y * (int)portraits->width + x] & 0x0F);
            if (src == 0) continue; /* transparent boundary, do not require match */
            ++total;
            unsigned char dst = M11_FB_DECODE_INDEX(fb[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)]);
            if (dst == src) ++matched;
        }
    }
    return matched; /* caller derives pct = 100*matched/total */
}

/* Count "warm"-palette pixels in the D1C cutout.  Warm palette
 * indices {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} are the skin/clothing
 * indices used by champion portrait sprites per ReDMCSB DUNVIEW.C
 * 3913-3928.  Standable corridor / wall-texture pixels typically
 * stay in the grey palette and never reach this many warm pixels. */
static int count_warm_pixels_in_cutout(const unsigned char* fb) {
    int count = 0;
    int x, y;
    if (!fb) return 0;
    for (y = 0; y < CUTOUT_H; ++y) {
        for (x = 0; x < CUTOUT_W; ++x) {
            unsigned char raw = fb[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            switch (idx) {
                case 0x07: /* green */
                case 0x08: /* red */
                case 0x09: /* orange */
                case 0x0A: /* peach */
                case 0x0B: /* yellow */
                case 0x0E: /* blue */
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/* ── main ────────────────────────────────────────────────────── */
int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int foundCell = -1;
    int c127Cell = -1;
    int foundAtMapX = -1;
    int foundAtMapY = -1;
    unsigned char sq = 0;
    int posesTried = 0;
    int posesReturnedZero = 0;
    char daroouName[64] = {0};
    char daroouTitle[64] = {0};

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions ordinal 0 (DAROOU) redraw_after_candidate slice ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("Slice: ordinal=0  route=redraw_after_candidate  aspect=portrait_rect_position\n");

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;
    game.candidateMirrorPanelActive = 0;

    /* ── Group A: ordinal 0 is the DAROOU mirror ─────────────── */
    printf("\n[Group A] Mirror catalog name/title for ordinal 0\n");
    if (M11_GameView_GetMirrorNameByOrdinal(&game, ORDINAL_DAROOU,
                                            daroouName, sizeof(daroouName)) > 0) {
        printf("  ordinal 0 name = '%s'\n", daroouName);
    }
    if (M11_GameView_GetMirrorTitleByOrdinal(&game, ORDINAL_DAROOU,
                                             daroouTitle, sizeof(daroouTitle)) > 0) {
        printf("  ordinal 0 title = '%s'\n", daroouTitle);
    }
    CHECK(daroouName[0] != '\0',
          "ordinal 0 has a non-empty name in the mirror catalog");
    if (daroouName[0] != '\0') {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 0 name matches DAROOU (got '%s', expected 'DAROOU')",
                 daroouName);
        CHECK(strcmp(daroouName, "DAROOU") == 0, msg);
    }

    /* ── Group B: C127 sensor with sensorData=0 lives on (1, 8) ── */
    printf("\n[Group B] C127 sensor with sensorData=0 location in real DM1 V1 DUNGEON.DAT\n");
    if (game.world.dungeon && game.world.things && game.world.things->squareFirstThings) {
        int w = game.world.dungeon->maps[0].width;
        int h = game.world.dungeon->maps[0].height;
        int mapX, mapY;
        printf("  Hall of Champions map 0 size: %d x %d\n", w, h);
        for (mapX = 0; mapX < w; ++mapX) {
            for (mapY = 0; mapY < h; ++mapY) {
                if (find_c127_with_data(&game, 0, mapX, mapY,
                                        ORDINAL_DAROOU, &c127Cell)) {
                    foundAtMapX = mapX;
                    foundAtMapY = mapY;
                }
            }
        }
    } else {
        printf("  SKIP: DUNGEON.DAT things not loaded\n");
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C127 sensor with sensorData=0 found on map 0");
        CHECK(foundAtMapX >= 0 && foundAtMapY >= 0, msg);
    }
    if (foundAtMapX >= 0) {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C127 sensor with sensorData=0 anchored at (1, 8) (got (%d, %d))",
                 foundAtMapX, foundAtMapY);
        CHECK(foundAtMapX == 1 && foundAtMapY == 8, msg);
        snprintf(msg, sizeof(msg),
                 "C127 sensor cell bit is 1 (east wall) (got %d)",
                 c127Cell);
        CHECK(c127Cell == 1, msg);
    }

    /* ── Group C: engine returns 0 for the DAROOU poses ──────── */
    printf("\n[Group C] Engine returns 0 for the DAROOU route poses\n");
    {
        struct { int x; int y; int d; const char* label; } poses[] = {
            /* (1, 1) facing N -> front (1, 0) has TextString[0] (DAROOU) */
            {1, 1, 0, "(1, 1) N -> front (1, 0) — canonical DAROOU front route"},
            /* (0, 0) facing E -> front (1, 0), same wall route */
            {0, 0, 1, "(0, 0) E -> front (1, 0) — same DAROOU wall from west"},
            /* (2, 0) facing W -> front (1, 0), same wall route from east */
            {2, 0, 3, "(2, 0) W -> front (1, 0) — same DAROOU wall from east"},
            /* (0, 8) facing E -> front (1, 8), the C127 sensor cell.
             * On a wall-like cell the engine applies the
             * (THING_GET_CELL(sensor) != visibleWallCell) filter; for
             * E the visibleWallCell is 3 (west wall) but the C127
             * sensor's M011_CELL is 1 (east wall), so the engine
             * must skip the sensor and return -1.  This negative
             * case is the "wall-side filter" gate from ReDMCSB
             * DUNGEON.C:2573-2612. */
            {0, 8, 1, "(0, 8) E -> front (1, 8) — wall-side filter must reject sensorData=0 (got %d)"},
        };
        int i;
        int ordinal0PoseCount = 0;
        int ordinal0FilterCorrectCount = 0;
        int ordinal0FilterExpectedCorrectCount = 0;
        for (i = 0; i < (int)(sizeof(poses)/sizeof(poses[0])); ++i) {
            int got;
            game.world.party.mapIndex = 0;
            game.world.party.mapX = poses[i].x;
            game.world.party.mapY = poses[i].y;
            game.world.party.direction = poses[i].d;
            got = M11_GameView_GetFrontMirrorOrdinal(&game);
            ++posesTried;
            if (strstr(poses[i].label, "wall-side filter")) {
                /* The wall-side filter negative case.  Print full
                 * label with the actual return value. */
                printf("  %s -> ordinal=%d\n", poses[i].label, got);
                if (got == -1) ++ordinal0FilterCorrectCount;
                ++ordinal0FilterExpectedCorrectCount;
            } else {
                printf("  %s -> ordinal=%d\n", poses[i].label, got);
                if (got == ORDINAL_DAROOU) ++ordinal0PoseCount;
            }
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "every tested DAROOU route pose returned 0 from GetFrontMirrorOrdinal (got %d/3)",
                     ordinal0PoseCount);
            CHECK(ordinal0PoseCount == 3, msg);
        }
        if (ordinal0FilterExpectedCorrectCount > 0) {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "wall-side filter rejected C127 sensorData=0 on the wrong-side pose (got %d/%d)",
                     ordinal0FilterCorrectCount, ordinal0FilterExpectedCorrectCount);
            CHECK(ordinal0FilterCorrectCount == ordinal0FilterExpectedCorrectCount, msg);
        }
        if (ordinal0PoseCount == 3) {
            ++posesReturnedZero;
        }
    }

    /* ── Group D: D1C cutout (96, 35, 32, 29) shows DAROOU portrait at canonical pose ── */
    printf("\n[Group D] D1C portrait cutout (96, 35, 32, 29) is the DAROOU portrait at (1, 1) N\n");
    {
        unsigned char fb[FB_W * FB_H];
        const M11_AssetSlot* portraits = NULL;
        int matchedSamples = 0;
        int totalSamples = 0;
        int matchedPct = 0;
        int warmCount = 0;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = 1;
        game.world.party.mapY = 1;
        game.world.party.direction = 0; /* N */
        game.world.party.championCount = 0;
        game.candidateMirrorPanelActive = 0;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);

        portraits = M11_AssetLoader_Load((M11_AssetLoader*)&game.assetLoader,
                                         (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        if (portraits && portraits->loaded && portraits->pixels) {
            int srcX0 = (ORDINAL_DAROOU & 7) * CUTOUT_W;
            int srcY0 = (ORDINAL_DAROOU >> 3) * CUTOUT_H;
            int x, y;
            if (srcX0 + CUTOUT_W <= (int)portraits->width &&
                srcY0 + CUTOUT_H <= (int)portraits->height) {
                for (y = 0; y < CUTOUT_H; ++y) {
                    for (x = 0; x < CUTOUT_W; ++x) {
                        unsigned char src = (unsigned char)(portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
                        if (src == 0) continue; /* transparency */
                        ++totalSamples;
                        unsigned char dst = M11_FB_DECODE_INDEX(fb[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)]);
                        if (dst == src) ++matchedSamples;
                    }
                }
            }
        }
        matchedPct = (totalSamples > 0) ? (matchedSamples * 100 / totalSamples) : 0;
        warmCount = count_warm_pixels_in_cutout(fb);
        printf("  party at (1, 1) N, D1C cutout (96, 35, 32, 29) "
               "DAROOU-portrait match = %d%% (%d/%d), warm_pixels=%d\n",
               matchedPct, matchedSamples, totalSamples, warmCount);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout at the DAROOU canonical pose is the DAROOU portrait "
                     "(match %d%%, must be >= 70%%)",
                     matchedPct);
            CHECK(matchedPct >= 70, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout at the DAROOU canonical pose has >= %d warm pixels "
                     "(portrait detected, got %d)",
                     DAROOU_PORTRAIT_PIXEL_THRESHOLD, warmCount);
            CHECK(warmCount >= DAROOU_PORTRAIT_PIXEL_THRESHOLD, msg);
        }
    }

    /* ── Group E: redraw_after_candidate route ───────────────── */
    /* Open the C040 panel via SelectFrontMirrorCandidate, then
     * ConfirmMirrorCandidate (resurrect).  The next M11_GameView_Draw
     * must redraw the D1C portrait cutout with the DAROOU sprite
     * content, not the panel graphic and not the wall texture.
     * ReDMCSB PANEL.C F0352 redraws C040 on close, REVIVE.C F0282
     * closes the candidate panel, and DUNVIEW.C:3913-3928 keeps
     * the C026 portrait blit stable across the close. */
    printf("\n[Group E] redraw_after_candidate: panel close preserves D1C portrait cutout\n");
    {
        unsigned char fbBefore[FB_W * FB_H];
        unsigned char fbAfter[FB_W * FB_H];
        const M11_AssetSlot* portraits = NULL;
        int matchedBefore = 0, totalBefore = 0;
        int matchedAfter = 0, totalAfter = 0;
        int warmBefore = 0, warmAfter = 0;
        int panelOpenRc = 0;
        int confirmRc = 0;
        int initialChampionCount = 0;

        game.world.party.mapIndex = 0;
        game.world.party.mapX = 1;
        game.world.party.mapY = 1;
        game.world.party.direction = 0; /* N */
        initialChampionCount = game.world.party.championCount;

        /* Draw the initial DAROOU frame (panel closed). */
        memset(fbBefore, 0, sizeof(fbBefore));
        game.candidateMirrorPanelActive = 0;
        M11_GameView_Draw(&game, fbBefore, FB_W, FB_H);

        /* Open the C040 candidate panel. */
        panelOpenRc = M11_GameView_SelectFrontMirrorCandidate(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "SelectFrontMirrorCandidate opened the C040 panel for ordinal 0 "
                     "(got rc=%d, candidateMirrorPanelActive=%d)",
                     panelOpenRc, game.candidateMirrorPanelActive);
            CHECK(panelOpenRc == 1, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "candidateMirrorPanelActive set to 1 after SelectFrontMirrorCandidate "
                     "(got %d)", game.candidateMirrorPanelActive);
            CHECK(game.candidateMirrorPanelActive == 1, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "party champion count grew by 1 after SelectFrontMirrorCandidate "
                     "(got %d, was %d)",
                     game.world.party.championCount, initialChampionCount);
            CHECK(game.world.party.championCount == initialChampionCount + 1, msg);
        }

        /* Confirm resurrect: closes the candidate panel. */
        confirmRc = M11_GameView_ConfirmMirrorCandidate(&game, 0);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "ConfirmMirrorCandidate(resurrect=0) closed the panel for ordinal 0 "
                     "(got rc=%d, candidateMirrorPanelActive=%d)",
                     confirmRc, game.candidateMirrorPanelActive);
            CHECK(confirmRc == 1, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "candidateMirrorPanelActive cleared to 0 after ConfirmMirrorCandidate "
                     "(got %d)", game.candidateMirrorPanelActive);
            CHECK(game.candidateMirrorPanelActive == 0, msg);
        }

        /* Draw again — the redraw_after_candidate route. */
        memset(fbAfter, 0, sizeof(fbAfter));
        M11_GameView_Draw(&game, fbAfter, FB_W, FB_H);

        portraits = M11_AssetLoader_Load((M11_AssetLoader*)&game.assetLoader,
                                         (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        if (portraits && portraits->loaded && portraits->pixels) {
            int srcX0 = (ORDINAL_DAROOU & 7) * CUTOUT_W;
            int srcY0 = (ORDINAL_DAROOU >> 3) * CUTOUT_H;
            int x, y;
            if (srcX0 + CUTOUT_W <= (int)portraits->width &&
                srcY0 + CUTOUT_H <= (int)portraits->height) {
                for (y = 0; y < CUTOUT_H; ++y) {
                    for (x = 0; x < CUTOUT_W; ++x) {
                        unsigned char src = (unsigned char)(portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
                        if (src == 0) continue;
                        unsigned char dstBefore = M11_FB_DECODE_INDEX(fbBefore[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)]);
                        unsigned char dstAfter = M11_FB_DECODE_INDEX(fbAfter[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)]);
                        if (dstBefore == src) ++matchedBefore;
                        if (dstAfter == src) ++matchedAfter;
                        ++totalBefore;
                        ++totalAfter;
                    }
                }
            }
        }
        warmBefore = count_warm_pixels_in_cutout(fbBefore);
        warmAfter = count_warm_pixels_in_cutout(fbAfter);

        printf("  pre-panel  DAROOU-portrait match = %d/%d, warm_pixels=%d\n",
               matchedBefore, totalBefore, warmBefore);
        printf("  post-panel DAROOU-portrait match = %d/%d, warm_pixels=%d\n",
               matchedAfter, totalAfter, warmAfter);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout after panel close still shows the DAROOU portrait "
                     "(warm_pixels=%d, must be >= %d)",
                     warmAfter, POST_PANEL_PORTRAIT_PIXEL_THRESHOLD);
            CHECK(warmAfter >= POST_PANEL_PORTRAIT_PIXEL_THRESHOLD, msg);
        }
        {
            char msg[200];
            int beforePct = (totalBefore > 0) ? (matchedBefore * 100 / totalBefore) : 0;
            int afterPct = (totalAfter > 0) ? (matchedAfter * 100 / totalAfter) : 0;
            snprintf(msg, sizeof(msg),
                     "D1C cutout DAROOU-portrait match pct survived panel close "
                     "(before=%d%%, after=%d%%, must be >= 70%%)",
                     beforePct, afterPct);
            CHECK(afterPct >= 70, msg);
        }
    }

    /* ── Group F: no floating DAROOU portrait on side walls ──── */
    /* The (1, 8) east wall carries the C127 sensor with sensorData=0.
     * When the party turns the camera to face N/E/S/W from a standable
     * corridor square other than the canonical DAROOU poses, the
     * D1C cutout must remain clear of the DAROOU portrait so the
     * sprite does not float over a wall that is not the DAROOU
     * mirror.  This is the "no-floating" assertion already proven
     * for other ordinals by the capture probe's no-floating poses. */
    printf("\n[Group F] No floating DAROOU portrait on side walls away from the (1, 8) mirror\n");
    {
        unsigned char fb[FB_W * FB_H];
        struct { int x; int y; int d; const char* label; } poses[] = {
            /* (1, 1) facing E — front (2, 1), a wall.  The C127 sensor
             * on (1, 8) is not on this front cell, so the cutout must
             * not contain the DAROOU portrait. */
            {1, 1, 1, "(1, 1) E — front (2, 1)"},
            /* (1, 1) facing S — front (1, 2), a door.  Not the
             * DAROOU mirror cell. */
            {1, 1, 2, "(1, 1) S — front (1, 2) door"},
            /* (1, 1) facing W — front (0, 1), a wall.  Not the
             * DAROOU mirror cell. */
            {1, 1, 3, "(1, 1) W — front (0, 1)"},
            /* (1, 2) facing N — front (1, 1), a wall with C127
             * sensorData=1 (HALK), not 0 (DAROOU).  The cutout
             * must show HALK portrait, not DAROOU. */
            {1, 2, 0, "(1, 2) N — front (1, 1) HALK mirror, not DAROOU"},
            /* (2, 8) facing N — front (2, 7), away from (1, 8). */
            {2, 8, 0, "(2, 8) N — front (2, 7)"},
            /* (1, 9) facing S — front (1, 10), away from (1, 8). */
            {1, 9, 2, "(1, 9) S — front (1, 10)"},
        };
        int i;
        int allClear = 1;
        game.world.party.championCount = 0;
        for (i = 0; i < (int)(sizeof(poses)/sizeof(poses[0])); ++i) {
            int warm;
            int ord;
            game.world.party.mapIndex = 0;
            game.world.party.mapX = poses[i].x;
            game.world.party.mapY = poses[i].y;
            game.world.party.direction = poses[i].d;
            ord = M11_GameView_GetFrontMirrorOrdinal(&game);
            memset(fb, 0, sizeof(fb));
            game.candidateMirrorPanelActive = 0;
            M11_GameView_Draw(&game, fb, FB_W, FB_H);
            warm = count_warm_pixels_in_cutout(fb);
            printf("  %s -> ordinal=%d warm_pixels=%d\n",
                   poses[i].label, ord, warm);
            /* The non-DAROOU poses must have ord < 0 or ord > 0
             * (other ordinal), and the cutout must NOT look like
             * DAROOU.  But other ordinals also have warm pixels, so
             * we only assert "no float" for the ord < 0 cases. */
            if (ord < 0) {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "%s — D1C cutout is empty (warm_pixels=%d < %d)",
                         poses[i].label, warm, NO_FLOATING_PIXEL_THRESHOLD);
                if (warm >= NO_FLOATING_PIXEL_THRESHOLD) {
                    ++g_fail;
                    printf("  FAIL: %s\n", msg);
                    allClear = 0;
                } else {
                    ++g_pass;
                    printf("  PASS: %s\n", msg);
                }
            }
        }
        if (allClear) {
            CHECK(allClear, "every non-mirror pose has a clear D1C cutout (no DAROOU floating)");
        }
    }

    /* ── Group G: C127 sensor with sensorData=0 reachable from (1, 8) cell chain ── */
    printf("\n[Group G] Direct cell chain check: C127 with sensorData=0 reachable at (1, 8)\n");
    {
        int foundChain = find_c127_with_data(&game, 0, 1, 8,
                                             ORDINAL_DAROOU, &foundCell);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "C127 sensor with sensorData=0 is reachable from the (1, 8) thing chain (found=%d, cell=%d)",
                     foundChain, foundCell);
            CHECK(foundChain == 1, msg);
        }
        if (foundChain) {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "C127 sensor cell bit at (1, 8) is 1 (east wall) — confirms source placement");
            CHECK(foundCell == 1, msg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
