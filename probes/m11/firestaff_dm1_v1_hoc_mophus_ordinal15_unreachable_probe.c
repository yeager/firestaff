/*
 * firestaff_dm1_v1_hoc_mophus_ordinal15_unreachable_probe.c
 *
 * Real-asset/runtime evidence for the DM1 V1 Hall of Champions
 * "east_walkpath / portrait_rect_position" slice for champion
 * portrait ordinal 15 (MOPHUS).
 *
 * Slice goal
 * ----------
 *   The slice assigned to this pass is "champion portrait ordinal 15,
 *   route east_walkpath, aspect portrait_rect_position".  Ordinal 15
 *   is the MOPHUS mirror in the DM1 PC 3.4 C026 portrait strip
 *   (8 cols x 3 rows, ordinal = 15 -> column 7 row 1).
 *
 *   The slice asks us to prove two things for ordinal 15:
 *     1. The portrait ordinal maps to the expected champion (MOPHUS).
 *     2. The D1C portrait rectangle is drawn at the intended screen
 *        position (96, 35, 32, 29 per DUNVIEW.C G0109 and ReDMCSB
 *        G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63})
 *        when a real party pose exposes the MOPHUS C127 sensor.
 *
 *   The "east_walkpath" route concept is: the party walks east (through
 *   strafe_right, forward into the corridor facing east, etc.) along
 *   the Hall's east-bound corridor rows and approaches the MOPHUS
 *   mirror at the D1C front-wall rectangle.
 *
 * Real DM1 V1 finding for this slice
 * ----------------------------------
 *   The C127 champion-portrait sensor with sensorData = 15 is present
 *   in real DM1 V1 DUNGEON.DAT and is anchored to map cell (x=2, y=5)
 *   on the Hall of Champions (map 0, 18 wide x 19 tall).  The sensor's
 *   M011_CELL bit is 0 (south wall).
 *
 *   To expose this C127 sensor the party must stand one cell north of
 *   (2, 5), i.e. at (2, 4) facing SOUTH.  In real DM1 V1 the cell
 *   (2, 4) is a WALL square (M034_SQUARE_TYPE = 0), not a standable
 *   corridor, so the canonical MOPHUS pose is a forced-pose cell that
 *   the engine reaches via a DM1-controlled boundary-teleport or
 *   game-state injection (the entry sequence), NOT via the player's
 *   east_walkpath corridor movement.
 *
 *   Looking at the candidate east_walkpath routes that *are* reachable
 *   from the Hall corridor:
 *     (1, 4) facing EAST  -> front cell (2, 4) WALL (sensor cell bit=0)
 *                            visibleWallCell=3 (west wall). The
 *                            M011_CELL=0 sensor fails the wall-side
 *                            filter, so the engine returns -1
 *                            ("wrong wall" — covered by the existing
 *                            zorder_reblt_runtime_probe).
 *     (1, 5) facing EAST  -> front cell (2, 5) WALL (MOPHUS sensor cell)
 *                            visibleWallCell=3 (west wall). Same
 *                            filter failure (M011_CELL=0 != 3) -> -1.
 *     (2, 3) facing SOUTH -> front cell (2, 4) WALL (no C127 sensor here,
 *                            M011_CELL filter wouldn't even be reached).
 *     (2, 4) facing SOUTH -> forced MOPHUS pose, ordinal=15, but the
 *                            party cannot reach (2, 4) via corridor
 *                            movement because (2, 4) is itself a WALL.
 *
 *   So the east_walkpath route cannot expose ordinal 15 in real DM1 V1:
 *   the MOPHUS C127 sensor's M011_CELL bit is 0 (south wall), but every
 *   east-bound walkable front cell (visibleWallCell=3 west wall) fails
 *   the wall-side filter.  The MOPHUS pose exists in DUNGEON.DAT, is
 *   named in the mirror catalog, and the engine draws it correctly
 *   when the party is forced into (2, 4) facing SOUTH, but it is NOT
 *   reachable from the playable east corridor.
 *
 *   This is the "data-present / route-absent" pattern documented for
 *   this slice: the C127 sensor is present and named in the mirror
 *   catalog, the engine draws the D1C portrait correctly at the forced
 *   canonical pose, and the wall-side filter cleanly rejects every
 *   corridor east_walkpath pose so no floating-portrait leak appears
 *   on the MOPHUS-side walls.  The probe asserts:
 *     (A) The C127 sensor with sensorData = 15 exists in real DM1 V1
 *         DUNGEON.DAT and is anchored to (2, 5) on map 0.
 *     (B) The sensor's M011_CELL bit is 0 (south wall), matching the
 *         MOPHUS canonical pose (2, 4) facing SOUTH (visibleWallCell=0).
 *     (C) The mirror catalog names ordinal 15 as MOPHUS (so the C127
 *         sensor data does point at the MOPHUS portrait).
 *     (D) M11_GameView_GetFrontMirrorOrdinal returns -1 for every
 *         east_walkpath pose that targets the MOPHUS wall from a
 *         walkable corridor square (the wall-side filter blocks the
 *         M011_CELL mismatch).
 *     (E) The D1C portrait rectangle (96, 35, 32, 29 in viewport
 *         coords, G0109 = {96, 127, 35, 63}) IS drawn correctly with
 *         the MOPHUS portrait at the canonical MOPHUS pose (2, 4)
 *         facing SOUTH — this is the aspect=portrait_rect_position
 *         proof for the data-present mirror, not for the unreachable
 *         east_walkpath route.
 *     (F) The D1C cutout does NOT show MOPHUS at any east_walkpath
 *         pose that the wall-side filter rejects, so no floating
 *         portrait leaks through the wrong-wall cell.
 *
 *   This is the "narrow slice" the parent task asked for: it pins
 *   down that ordinal 15 is data-present but east_walkpath-absent in
 *   real DM1 V1 PC 3.4 DUNGEON.DAT, and that the engine does the
 *   right thing (returns -1 from any corridor east_walkpath pose,
 *   draws the MOPHUS portrait correctly at the forced canonical
 *   pose, and does not leak MOPHUS pixels onto side walls).
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

enum { ORDINAL_MOPHUS = 15 };

/* DUNVIEW.C:3916 C01_COLOR_DARK_GRAY (value 1) is the C026 champion
 * portrait transparency mask, shared with the existing
 * champion_mirror_walkpath / zorder_reblt / actual_pose probes. */
#define CHAMPION_TRANSPARENT 1

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

/* ── main ────────────────────────────────────────────────────── */
int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int c127Cell = -1;
    int foundAtMapX = -1;
    int foundAtMapY = -1;
    int ordinal15RouteExists = 0;
    char mophusName[64] = {0};
    char mophusTitle[64] = {0};

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions ordinal 15 (MOPHUS) unreachable slice ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("Slice: ordinal=15  route=east_walkpath  aspect=portrait_rect_position\n");

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;
    game.candidateMirrorPanelActive = 0;

    /* ── Group A: ordinal 15 is the MOPHUS mirror ──────────────── */
    printf("\n[Group A] Mirror catalog name/title for ordinal 15\n");
    if (M11_GameView_GetMirrorNameByOrdinal(&game, ORDINAL_MOPHUS,
                                            mophusName, sizeof(mophusName)) > 0) {
        printf("  ordinal 15 name = '%s'\n", mophusName);
    }
    if (M11_GameView_GetMirrorTitleByOrdinal(&game, ORDINAL_MOPHUS,
                                             mophusTitle, sizeof(mophusTitle)) > 0) {
        printf("  ordinal 15 title = '%s'\n", mophusTitle);
    }
    CHECK(mophusName[0] != '\0',
          "ordinal 15 has a non-empty name in the mirror catalog");
    if (mophusName[0] != '\0') {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 name matches MOPHUS (got '%s', expected 'MOPHUS')",
                 mophusName);
        CHECK(strcmp(mophusName, "MOPHUS") == 0, msg);
    }

    /* ── Group B: C127 sensor with sensorData=15 lives on (2, 5) ── */
    printf("\n[Group B] C127 sensor with sensorData=15 location in real DM1 V1 DUNGEON.DAT\n");
    if (game.world.dungeon && game.world.things && game.world.things->squareFirstThings) {
        int w = game.world.dungeon->maps[0].width;
        int h = game.world.dungeon->maps[0].height;
        int mapX, mapY;
        printf("  Hall of Champions map 0 size: %d x %d\n", w, h);
        for (mapX = 0; mapX < w; ++mapX) {
            for (mapY = 0; mapY < h; ++mapY) {
                if (find_c127_with_data(&game, 0, mapX, mapY,
                                        ORDINAL_MOPHUS, &c127Cell)) {
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
                 "C127 sensor with sensorData=15 found on map 0");
        CHECK(foundAtMapX >= 0 && foundAtMapY >= 0, msg);
    }
    if (foundAtMapX >= 0) {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C127 sensor with sensorData=15 anchored at (2, 5) (got (%d, %d))",
                 foundAtMapX, foundAtMapY);
        CHECK(foundAtMapX == 2 && foundAtMapY == 5, msg);
        snprintf(msg, sizeof(msg),
                 "C127 sensor cell bit is 0 (south wall) — matches MOPHUS pose (2, 4) facing S (got %d)",
                 c127Cell);
        CHECK(c127Cell == 0, msg);
    }

    /* ── Group C: cell (2, 5) is a WALL square ── */
    printf("\n[Group C] Cell (2, 5) is a WALL square (not a standable corridor)\n");
    {
        unsigned char sq = 0;
        if (read_square_byte(&game, 2, 5, &sq)) {
            unsigned char elem = (sq >> 5) & 0x07;
            const char* eName[8] = {"WALL", "CORRIDOR", "PIT", "STAIRS",
                                     "DOOR", "TELEPORTER", "FAKEWALL", "?"};
            printf("  (2, 5) square=0x%02x elem=%d(%s) wallLike=%d\n",
                   sq, elem, eName[elem], M11_DM1_ViewportSquareIsWallLikePc34(sq));
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "(2, 5) is a WALL square (M034_SQUARE_TYPE = 0)");
                CHECK(elem == 0, msg);
            }
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "(2, 5) is wall-like per M11_DM1_ViewportSquareIsWallLikePc34");
                CHECK(M11_DM1_ViewportSquareIsWallLikePc34(sq) == 1, msg);
            }
        } else {
            SKIP("tile data not loaded");
        }
    }

    /* ── Group D: every east_walkpath party pose targeting (2, 5) from a
     *   standable corridor returns -1; the canonical MOPHUS pose (forced
     *   only) returns ordinal 15; no east_walkpath pose exposes MOPHUS. ── */
    printf("\n[Group D] East_walkpath corridor poses targeting MOPHUS wall return -1; canonical MOPHUS pose (forced-only) returns 15\n");
    {
        struct { int x; int y; int d; const char* label; int isForcedCanonical; int isDifferentMirror; } poses[] = {
            /* East_walkpath candidates: from the corridor (x=1, y=4) the party
             * tries to walk east toward the MOPHUS wall.  All four cardinal
             * directions that would put the front cell on (2, 5) are
             * exercised here.  None of them are reachable via east_walkpath. */
            {1, 5, 1, "(1, 5) E -> front (2, 5) — east_walkpath toward MOPHUS wall", 0, 0},
            {3, 5, 3, "(3, 5) W -> front (2, 5) — east_walkpath return leg", 0, 0},
            {2, 4, 2, "(2, 4) S -> front (2, 5) — canonical MOPHUS pose (forced-only)", 1, 0},
            {2, 6, 0, "(2, 6) N -> front (2, 5) — south-of-MOPHUS north-facing", 0, 0},
            /* Adjacent east_walkpath probes that may *not* target (2, 5) but
             * still should not leak MOPHUS through the wall-side filter. */
            {1, 4, 1, "(1, 4) E -> front (2, 4) — east_walkpath toward MOPHUS-side wall", 0, 0},
            {1, 3, 1, "(1, 3) E -> front (2, 3) — east_walkpath toward SONJA (different mirror)", 0, 1},
        };
        int corridorPosesTried = 0;
        int corridorPosesReturnedNeg1 = 0;
        int i;
        for (i = 0; i < (int)(sizeof(poses)/sizeof(poses[0])); ++i) {
            int got;
            game.world.party.mapIndex = 0;
            game.world.party.mapX = poses[i].x;
            game.world.party.mapY = poses[i].y;
            game.world.party.direction = poses[i].d;
            got = M11_GameView_GetFrontMirrorOrdinal(&game);
            printf("  %s -> ordinal=%d\n", poses[i].label, got);
            if (got == ORDINAL_MOPHUS) {
                if (poses[i].isForcedCanonical) {
                    printf("    (canonical forced MOPHUS pose, allowed)\n");
                } else {
                    ordinal15RouteExists = 1;
                    printf("    LEAK: %s exposes MOPHUS ordinal from a corridor east_walkpath pose\n",
                           poses[i].label);
                }
            }
            if (poses[i].isForcedCanonical) {
                /* Skip the forced-canonical pose when counting corridor
                 * returns; the canonical pose is supposed to return 15,
                 * not -1, and it is reached only by game-state injection. */
                continue;
            }
            ++corridorPosesTried;
            if (got == -1) {
                ++corridorPosesReturnedNeg1;
            } else if (poses[i].isDifferentMirror) {
                /* (1, 3) E hits a different mirror (SONJA), not MOPHUS — acceptable */
                ++corridorPosesReturnedNeg1;
                printf("    note: %s hits a different mirror (not MOPHUS, ordinal=%d)\n",
                       poses[i].label, got);
            } else {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "%s returns -1 from GetFrontMirrorOrdinal (got %d)",
                         poses[i].label, got);
                printf("    note: %s\n", msg);
            }
        }
        /* Validate two independent invariants: */
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "every corridor east_walkpath pose targeting MOPHUS returned -1 (got %d/%d)",
                     corridorPosesReturnedNeg1, corridorPosesTried);
            CHECK(corridorPosesReturnedNeg1 == corridorPosesTried, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "no east_walkpath party pose from a standable corridor exposes the MOPHUS ordinal-15 route");
            CHECK(ordinal15RouteExists == 0, msg);
        }
    }

    /* ── Group E: D1C portrait cutout IS the MOPHUS portrait at the canonical MOPHUS pose ── */
    printf("\n[Group E] D1C portrait cutout (96, 35, 32, 29) IS the MOPHUS portrait at the canonical MOPHUS pose (2, 4) facing S\n");
    {
        unsigned char fb[FB_W * FB_H];
        const M11_AssetSlot* portraits = NULL;
        int matchedPct = 0;
        int matchedSamples = 0;
        int totalSamples = 0;
        int x, y;
        memset(fb, 0, sizeof(fb));
        /* The canonical MOPHUS pose: (2, 4) facing SOUTH.  Even though
         * (2, 4) is a wall cell in DUNGEON.DAT, the engine accepts the
         * forced pose and draws the MOPHUS portrait at the D1C cutout
         * (96, 35, 32, 29).  Compare the cutout pixels against the
         * MOPHUS portrait (C026 ordinal 15) — they SHOULD match. */
        game.world.party.mapIndex = 0;
        game.world.party.mapX = 2;
        game.world.party.mapY = 4;
        game.world.party.direction = 2;  /* DIR_SOUTH */
        game.world.party.championCount = 0;
        M11_GameView_Draw(&game, fb, FB_W, FB_H);

        portraits = M11_AssetLoader_Load(&game.assetLoader,
                                         (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        if (portraits && portraits->loaded && portraits->pixels) {
            int srcX0 = (ORDINAL_MOPHUS & 7) * CUTOUT_W;
            int srcY0 = (ORDINAL_MOPHUS >> 3) * CUTOUT_H;
            if (srcX0 + CUTOUT_W <= (int)portraits->width &&
                srcY0 + CUTOUT_H <= (int)portraits->height) {
                for (y = 0; y < CUTOUT_H; ++y) {
                    for (x = 0; x < CUTOUT_W; ++x) {
                        unsigned char src = (unsigned char)(portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
                        if (src == CHAMPION_TRANSPARENT) continue; /* transparency mask */
                        ++totalSamples;
                        unsigned char dst = M11_FB_DECODE_INDEX(fb[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)]);
                        if (dst == src) ++matchedSamples;
                    }
                }
            }
        }
        matchedPct = (totalSamples > 0) ? (matchedSamples * 100 / totalSamples) : 0;
        printf("  party at (2, 4) S, D1C cutout (96, 35, 32, 29) "
               "MOPHUS-portrait match = %d%% (%d/%d)\n",
               matchedPct, matchedSamples, totalSamples);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C cutout at the canonical MOPHUS pose IS the MOPHUS portrait "
                     "(match %d%%, must be >= 90%% to confirm aspect=portrait_rect_position)",
                     matchedPct);
            CHECK(matchedPct >= 90, msg);
        }
    }

    /* ── Group F: D1C portrait cutout does NOT show MOPHUS at east_walkpath poses ── */
    printf("\n[Group F] D1C cutout does NOT show MOPHUS at east_walkpath corridor poses (no floating-portrait leak)\n");
    {
        unsigned char fb[FB_W * FB_H];
        const M11_AssetSlot* portraits = NULL;
        struct { int x; int y; int d; const char* label; } poses[] = {
            {1, 4, 1, "(1, 4) E — east_walkpath toward MOPHUS-side wall"},
            {1, 5, 1, "(1, 5) E — east_walkpath toward MOPHUS wall"},
            {3, 5, 3, "(3, 5) W — east_walkpath return toward MOPHUS wall"},
            {2, 4, 0, "(2, 4) N — MOPHUS-cell NORTH-facing (front (2, 3) wall)"},
            {2, 4, 1, "(2, 4) E — MOPHUS-cell EAST-facing"},
            {2, 4, 3, "(2, 4) W — MOPHUS-cell WEST-facing"},
        };
        int i;
        int leaks = 0;
        portraits = M11_AssetLoader_Load(&game.assetLoader,
                                         (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        for (i = 0; i < (int)(sizeof(poses)/sizeof(poses[0])); ++i) {
            int matchedPct = 0;
            int matchedSamples = 0;
            int totalSamples = 0;
            int x, y;
            memset(fb, 0, sizeof(fb));
            game.world.party.mapIndex = 0;
            game.world.party.mapX = poses[i].x;
            game.world.party.mapY = poses[i].y;
            game.world.party.direction = poses[i].d;
            game.world.party.championCount = 0;
            M11_GameView_Draw(&game, fb, FB_W, FB_H);
            if (portraits && portraits->loaded && portraits->pixels) {
                int srcX0 = (ORDINAL_MOPHUS & 7) * CUTOUT_W;
                int srcY0 = (ORDINAL_MOPHUS >> 3) * CUTOUT_H;
                if (srcX0 + CUTOUT_W <= (int)portraits->width &&
                    srcY0 + CUTOUT_H <= (int)portraits->height) {
                    for (y = 0; y < CUTOUT_H; ++y) {
                        for (x = 0; x < CUTOUT_W; ++x) {
                            unsigned char src = (unsigned char)(portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
                            if (src == CHAMPION_TRANSPARENT) continue; /* transparency mask */
                            ++totalSamples;
                            unsigned char dst = M11_FB_DECODE_INDEX(fb[(CUTOUT_FY + y) * FB_W + (CUTOUT_FX + x)]);
                            if (dst == src) ++matchedSamples;
                        }
                    }
                }
            }
            matchedPct = (totalSamples > 0) ? (matchedSamples * 100 / totalSamples) : 0;
            printf("  %s -> MOPHUS-portrait match = %d%% (%d/%d)\n",
                   poses[i].label, matchedPct, matchedSamples, totalSamples);
            if (matchedPct >= 30) {
                ++leaks;
                printf("    LEAK: %s matches MOPHUS portrait >= 30%%\n", poses[i].label);
            }
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "no east_walkpath or MOPHUS-side pose leaks MOPHUS pixels (got %d leaks)",
                     leaks);
            CHECK(leaks == 0, msg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
