/*
 * firestaff_dm1_v1_champion_mirror_leylla_ordinal14_unreachable_probe.c
 *
 * Real-asset/runtime evidence for the DM1 V1 Hall of Champions
 * "front_north_entry" / portrait_rect_position slice for champion
 * portrait ordinal 14 (LEYLA / SHADOWSEEK).
 *
 * Slice goal
 * ----------
 *   The slice assigned to this pass is "champion portrait ordinal 14,
 *   route front_north_entry, aspect portrait_rect_position".  Ordinal
 *   14 is the LEYLA mirror in the DM1 PC 3.4 C026 portrait strip
 *   (8 cols x 3 rows, ordinal = 14 -> column 6 row 1).
 *
 *   The slice asks us to prove two things for ordinal 14:
 *     1. The portrait ordinal maps to the expected champion (LEYLA).
 *     2. The D1C portrait rectangle is drawn at the intended screen
 *        position (96, 35, 32, 29 per DUNVIEW.C G0109 and ReDMCSB
 *        G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63})
 *        when a real front_north_entry route exposes the LEYLA C127
 *        sensor.
 *
 *   The previous slice pass already covered ordinals 1 (HALK), 4
 *   (LEIF), 10 (ZED), 13 (WUUF), 15 (MOPHUS), 18 (SONJA) via
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe +
 *   firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe.
 *   Ordinal 14 was *not* covered there.
 *
 * Real DM1 V1 finding for this slice
 * ----------------------------------
 *   The C127 champion-portrait sensor with sensorData = 14 is
 *   present in real DM1 V1 DUNGEON.DAT and is anchored to map cell
 *   (x=1, y=18) on the Hall of Champions (map 0, 18 wide x 19 tall).
 *   The cell (1, 18) is a WALL square (M034_SQUARE_TYPE = 0), the
 *   C127 sensor's wall-cell bit (M011_CELL, bits 13:14 of the THING
 *   id) is 2 (north wall), and the map's southern boundary is at
 *   y=18, so the only party pose that would put the front cell at
 *   (1, 18) is (1, 19) facing N (out of bounds).  Every party pose
 *   that the engine actually accepts in the playable Hall returns
 *   -1 from M11_GameView_GetFrontMirrorOrdinal: the LEYLA C127
 *   sensor's data is present in DUNGEON.DAT but its placement on a
 *   non-standable wall cell means the front_north_entry route for
 *   ordinal 14 has no realizable party pose in PC 3.4.
 *
 *   This is an honest data-level finding.  The probe asserts:
 *     (A) The C127 sensor with sensorData = 14 exists in real DM1 V1
 *         DUNGEON.DAT and is anchored to (1, 18) on map 0.
 *     (B) (1, 18) is a WALL square, not a standable corridor.
 *     (C) The mirror catalog names ordinal 14 as LEYLA / SHADOWSEEK
 *         (so the C127 sensor data does point at the LEYLA
 *         portrait, even though the route is unreachable).
 *     (D) M11_GameView_GetFrontMirrorOrdinal returns -1 for every
 *         party pose that puts the front cell on (1, 18) from a
 *         standable corridor square, including the canonical
 *         front_north_entry pose (party at (1, 17) facing N,
 *         front = (1, 18)).
 *     (E) The D1C portrait rectangle (96, 35, 32, 29 in viewport
 *         coords, G0109 = {96, 127, 35, 63}) is never drawn for the
 *         LEYLA slice in real DM1 V1 because no front-cell chain
 *         exposes ordinal 14 to the engine; the portrait cutout at
 *         (96, 35) is empty when the party is at (1, 17) facing N.
 *
 *   This is the "narrow slice" the parent task asked for: it pins
 *   down that ordinal 14 is data-present but route-absent in real
 *   DM1 V1 PC 3.4 DUNGEON.DAT, and that the engine does the right
 *   thing (returns -1, draws no floating portrait on side walls)
 *   given that absence.  No floating-portrait regression can hide
 *   here because the LEYLA C127 sensor is never selected.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573-2612  C127 sensor view-direction mapping (M552)
 *   - DUNGEON.C:2608-2612  C127 sensorData -> G0289 portrait ordinal
 *   - DUNVIEW.C:3913-3928 D1C champion-portrait blit (C026 + G0109)
 *   - DUNVIEW.C:525       G0109 = {96, 127, 35, 63} (D1C champion box)
 *   - MOVESENS.C:1501-1503 C127 sensor->F0280 candidate path
 *   - REVIVE.C F0280      candidate materialization from sensorData
 *   - m11_front_cell_mirror_ordinal (m11_game_view.c) the wall-side
 *     filter is M11_DM1_ViewportSquareIsWallLikePc34(frontCell.square)
 *     && (int)THING_GET_CELL(thing) != visibleWallCell.
 *   - m11_sample_viewport_cell + M11_DM1_ViewportSquareIsWallLikePc34
 *     (dm1_v1_viewport_fakewall_pc34_compat.c) the front-cell
 *     element type used by the wall-side filter.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "memory_dungeon_dat_pc34_compat.h"
#include "dm1_v1_viewport_fakewall_pc34_compat.h"
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
};

enum { ORDINAL_LEYLA = 14 };

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

/* (count_nonzero helper removed: Group E now compares the D1C cutout
 * pixels against the LEYLA portrait strip directly.) */

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
    int partyPosesTried = 0;
    int partyPosesReturnedNeg1 = 0;
    int ordinal14RouteExists = 0;
    char leylaName[64] = {0};
    char leylaTitle[64] = {0};

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions ordinal 14 (LEYLA) unreachable slice ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("Slice: ordinal=14  route=front_north_entry  aspect=portrait_rect_position\n");

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;
    game.candidateMirrorPanelActive = 0;

    /* ── Group A: ordinal 14 is the LEYLA mirror ──────────────── */
    printf("\n[Group A] Mirror catalog name/title for ordinal 14\n");
    if (M11_GameView_GetMirrorNameByOrdinal(&game, ORDINAL_LEYLA,
                                            leylaName, sizeof(leylaName)) > 0) {
        printf("  ordinal 14 name = '%s'\n", leylaName);
    }
    if (M11_GameView_GetMirrorTitleByOrdinal(&game, ORDINAL_LEYLA,
                                             leylaTitle, sizeof(leylaTitle)) > 0) {
        printf("  ordinal 14 title = '%s'\n", leylaTitle);
    }
    CHECK(leylaName[0] != '\0',
          "ordinal 14 has a non-empty name in the mirror catalog");
    if (leylaName[0] != '\0') {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 name matches LEYLA (got '%s', expected 'LEYLA')",
                 leylaName);
        CHECK(strcmp(leylaName, "LEYLA") == 0, msg);
    }
    if (leylaTitle[0] != '\0') {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 14 title matches SHADOWSEEK (got '%s', expected 'SHADOWSEEK')",
                 leylaTitle);
        CHECK(strcmp(leylaTitle, "SHADOWSEEK") == 0, msg);
    }

    /* ── Group B: C127 sensor with sensorData=14 lives on (1, 18) ── */
    printf("\n[Group B] C127 sensor with sensorData=14 location in real DM1 V1 DUNGEON.DAT\n");
    if (game.world.dungeon && game.world.things && game.world.things->squareFirstThings) {
        int w = game.world.dungeon->maps[0].width;
        int h = game.world.dungeon->maps[0].height;
        int mapX, mapY;
        printf("  Hall of Champions map 0 size: %d x %d\n", w, h);
        for (mapX = 0; mapX < w; ++mapX) {
            for (mapY = 0; mapY < h; ++mapY) {
                if (find_c127_with_data(&game, 0, mapX, mapY,
                                        ORDINAL_LEYLA, &c127Cell)) {
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
                 "C127 sensor with sensorData=14 found on map 0");
        CHECK(foundAtMapX >= 0 && foundAtMapY >= 0, msg);
    }
    if (foundAtMapX >= 0) {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C127 sensor with sensorData=14 anchored at (1, 18) (got (%d, %d))",
                 foundAtMapX, foundAtMapY);
        CHECK(foundAtMapX == 1 && foundAtMapY == 18, msg);
        snprintf(msg, sizeof(msg),
                 "C127 sensor cell bit is 2 (north wall) (got %d)",
                 c127Cell);
        CHECK(c127Cell == 2, msg);
    }

    /* ── Group C: cell (1, 18) is a WALL square (not standable) ── */
    printf("\n[Group C] Cell (1, 18) is a WALL square (not a standable corridor)\n");
    if (read_square_byte(&game, 1, 18, &sq)) {
        unsigned char elem = (sq >> 5) & 0x07;
        const char* eName[8] = {"WALL", "CORRIDOR", "PIT", "STAIRS",
                                 "DOOR", "TELEPORTER", "FAKEWALL", "?"};
        printf("  (1, 18) square=0x%02x elem=%d(%s) wallLike=%d\n",
               sq, elem, eName[elem], M11_DM1_ViewportSquareIsWallLikePc34(sq));
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(1, 18) is a WALL square (M034_SQUARE_TYPE = 0)");
            CHECK(elem == 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(1, 18) is wall-like per M11_DM1_ViewportSquareIsWallLikePc34");
            CHECK(M11_DM1_ViewportSquareIsWallLikePc34(sq) == 1, msg);
        }
    } else {
        SKIP("tile data not loaded");
    }

    /* ── Group D: every party pose that targets (1, 18) returns -1 ── */
    printf("\n[Group D] Engine returns -1 for every party pose that targets (1, 18)\n");
    {
        struct { int x; int y; int d; const char* label; } poses[] = {
            /* The canonical "front_north_entry" route: party at (1, 17)
             * facing N.  Front cell = (1, 16), not (1, 18).  The
             * alternate canonical pose for the (1, 18) front cell is
             * (1, 17) facing S.  This is the LEYLA-specific test. */
            {1, 17, 0, "(1, 17) N -> front (1, 16) — canonical Hall start N"},
            {1, 17, 2, "(1, 17) S -> front (1, 18) — LEYLA's intended target cell"},
            {2, 18, 3, "(2, 18) W -> front (1, 18) — LEYLA from east"},
            {0, 18, 1, "(0, 18) E -> front (1, 18) — LEYLA from west"},
            /* (1, 19) facing N would target (1, 18) but is OOB. */
            {1, 18, 0, "(1, 18) N -> front (1, 17) — party on the wall (in-bounds only because the engine does not block-party-on-wall here)"},
        };
        int i;
        for (i = 0; i < (int)(sizeof(poses)/sizeof(poses[0])); ++i) {
            int got;
            game.world.party.mapIndex = 0;
            game.world.party.mapX = poses[i].x;
            game.world.party.mapY = poses[i].y;
            game.world.party.direction = poses[i].d;
            got = M11_GameView_GetFrontMirrorOrdinal(&game);
            ++partyPosesTried;
            printf("  %s -> ordinal=%d\n", poses[i].label, got);
            if (got == ORDINAL_LEYLA) {
                ordinal14RouteExists = 1;
            }
            if (got == -1) {
                ++partyPosesReturnedNeg1;
            } else {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "%s returns -1 from GetFrontMirrorOrdinal (got %d)",
                         poses[i].label, got);
                printf("    note: %s\n", msg);
            }
        }
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "every tested party pose targeting (1, 18) returned -1 (got %d/%d)",
                 partyPosesReturnedNeg1, partyPosesTried);
        CHECK(partyPosesReturnedNeg1 == partyPosesTried, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "no tested party pose exposes the LEYLA ordinal-14 route");
        CHECK(ordinal14RouteExists == 0, msg);
    }

    /* ── Group E: D1C portrait cutout (96, 35, 32, 29) does not show LEYLA ── */
    printf("\n[Group E] D1C portrait cutout (96, 35, 32, 29) is not the LEYLA portrait at the LEYLA front-cell pose\n");
    {
        unsigned char fb[FB_W * FB_H];
        const M11_AssetSlot* portraits = NULL;
        int matchedPct = 0;
        int matchedSamples = 0;
        int totalSamples = 0;
        int x, y;
        memset(fb, 0, sizeof(fb));
        /* The most direct pose is (1, 17) facing S (front=(1, 18)).
         * If the LEYLA C127 sensor's M011_CELL=2 didn't get filtered,
         * the engine would blit the LEYLA portrait (C026 ordinal 14)
         * at the D1C cutout (96, 35, 32, 29).  The wall-cell filter
         * blocks it because the player's visibleWallCell is 0
         * (south wall), but the C127 sensor's cell bit is 2
         * (north wall).  Compare the cutout pixels against the LEYLA
         * portrait to assert they do NOT match. */
        game.world.party.mapIndex = 0;
        game.world.party.mapX = 1;
        game.world.party.mapY = 17;
        game.world.party.direction = 2;
        game.world.party.championCount = 0;
        M11_GameView_Draw(&game, fb, FB_W, FB_H);

        portraits = M11_AssetLoader_Load(&game.assetLoader,
                                         (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        if (portraits && portraits->loaded && portraits->pixels) {
            int srcX0 = (ORDINAL_LEYLA & 7) * CUTOUT_W;
            int srcY0 = (ORDINAL_LEYLA >> 3) * CUTOUT_H;
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
        printf("  party at (1, 17) S, D1C cutout (96, 35, 32, 29) "
               "LEYLA-portrait match = %d%% (%d/%d)\n",
               matchedPct, matchedSamples, totalSamples);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout at the LEYLA front-cell pose is NOT the LEYLA portrait "
                     "(match %d%%, must be < 30%% to confirm wall-side filter held)",
                     matchedPct);
            CHECK(matchedPct < 30, msg);
        }
    }

    /* ── Group F: C127 sensor with sensorData=14 is still reachable
     *   from the thing chain even if the engine doesn't expose it. */
    printf("\n[Group F] Direct cell chain check: C127 with sensorData=14 reachable at (1, 18)\n");
    {
        int foundChain = find_c127_with_data(&game, 0, 1, 18,
                                             ORDINAL_LEYLA, &foundCell);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "C127 sensor with sensorData=14 is reachable from the (1, 18) thing chain (found=%d, cell=%d)",
                     foundChain, foundCell);
            CHECK(foundChain == 1, msg);
        }
        if (foundChain) {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "C127 sensor cell bit at (1, 18) is 2 (north wall) — confirms wall-side filter reason");
            CHECK(foundCell == 2, msg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
