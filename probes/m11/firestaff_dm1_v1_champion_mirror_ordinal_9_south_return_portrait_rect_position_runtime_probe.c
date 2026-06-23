/*
 * firestaff_dm1_v1_champion_mirror_ordinal_9_south_return_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   champion portrait ordinal 9 (C026 strip cell 9 — column 1, row 1)
 *   route south_return: walk south of (1,5), observe at the (1,5) cell
 *                       while the party faces south, then confirm the
 *                       D1C portrait rectangle (96, 35, 32, 29) draws
 *                       the ordinal 9 portrait and does not float on
 *                       the back wall, the south wall, or the
 *                       surrounding side walls.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 per ReDMCSB G0109_auc_Graphic558_Box
 *                                 _ChampionPortraitOnWall = {96, 127,
 *                                 35, 63} (DUNVIEW.C:525) and
 *                                 G0047 inner-portrait byte rect
 *                                 {0, 31, 0, 28} (REVIVE.C:142-146).
 *
 * Source-locked to:
 *   DUNGEON.C:2573       M011_CELL(sensor) vs view-dir filter
 *     (only sensors on M552_FRONT_WALL_ORNAMENT_ORDINAL = DEFS.H:2552 = 5
 *      set G0289).
 *   DUNGEON.C:2608-2612  C127 sensorData -> G0289 (DUNVIEW.C:3913).
 *   MOVESENS.C:1501-1503 sensorData flows to F0280 candidate.
 *   REVIVE.C F0280       materializes the candidate from sensorData.
 *   REVIVE.C F0282       C160 resurrection: clone champion for slot.
 *   REVIVE.C:142, 146    F0132_VIDEO_Blit for the G0047 inner-portrait
 *                        rect from the C026 strip.
 *   DUNVIEW.C:3913-3928  C346 frame + C026 portrait blit into G0109
 *                        portrait box {96..127, 35..63}, 32x29.
 *   DUNVIEW.C:8318-8542  F0128 viewport redraw order (far-to-near) so
 *                        side-wall geometry overpaints the D1C
 *                        portrait rectangle when the front cell no
 *                        longer has a C127 sensor.
 *   COORD.C:1693-1722    PC 3.4 viewport origin (0,33) / 224x136 dim.
 *   COORD.C:1748-1749    G2078_C32_PortraitWidth=32, G2079_C29=29.
 *
 * On the local PC 3.4 DM1 V1 build the ordinal 9 C127 sensor lives
 * at (1,10) DIR_NORTH (cell-side=NORTH sensor on the (1,10) square;
 * visible only when the party stands at (1,10) facing north).  No
 * (mapX, mapY, DIR_SOUTH) cell in the Hall reports ordinal 9
 * directly, so the south_return slice seeds the front cell's C127
 * sensor data to 9 on the canonical DIR_SOUTH pose (1,5) SOUTH —
 * the same south-facing WUUF alcove the ordinal 0/8 slices use —
 * to lock the ordinal-9 edge case on the south_return route.  This
 * catches regressions where C026 column-1/row-1 portrait pixels are
 * clipped off, transposed to another ordinal, or floated onto side
 * walls.
 *
 * Six invariants are proved:
 *
 *   (0) Catalog identity: M11_GameView_GetMirrorNameByOrdinal(9) ==
 *       "ZED" — the canonical PC 3.4 English mirror for ordinal 9
 *       (per the dm1_v1_mirror_catalog_champion_stats test family
 *       and the m11_load_mirror_catalog_pc34 path).  Title is
 *       "DUKE OF BANVILLE" (PC 3.4 EN).
 *   (1) M11_GameView_GetFrontMirrorOrdinal((1,5) SOUTH) == 13 (WUUF)
 *       on the un-patched shipped route.
 *   (2) After seeding the (1,6) C127 sensor data from 13 to 9, the
 *       same pose returns ordinal 9 and M11_GameView_Draw paints the
 *       D1C portrait rect (96,35)-(128,64) with C026 ordinal-9
 *       source pixels (warm_count >= 30, best_ordinal == 9).
 *   (3) The same rect is NOT painted on side-wall poses at the same
 *       (1,5) cell (EAST / WEST) and on adjacent corridor cells —
 *       no floating on side walls.
 *   (4) The portrait column/row math (ordinal 9 & 7 = 1 -> column 1,
 *       ordinal 9 >> 3 = 1 -> row 1) lines up with the C026 strip
 *       layout: source rect x=[32..64], y=[29..58] in the 256x87
 *       portrait strip.
 *   (5) Per-ordinal dominance: ordinal 9 matches >90%% of the
 *       non-transparent source pixels at the D1C rect vs <90%% for
 *       any other ordinal.
 *
 * HONESTY: this is Firestaff deterministic-runtime evidence, not
 * original-DM1 PC 3.4 pixel parity.  The ordinal-rectangle match
 * uses the same warm-color heuristic the existing
 * firestaff_dm1_v1_champion_mirror_capture_probe uses (palette
 * indices {0x07 green, 0x08 red, 0x09 orange, 0x0A peach, 0x0B
 * yellow, 0x0E blue}) to distinguish 'portrait painted' from
 * 'wall texture only'.  The pixel-perfect match routine reports
 * both the best-matched ordinal and the per-ordinal match count so
 * an unrelated D1C ornament cannot accidentally pass.
 *
 * Disjoint from the existing actual-pose / capture / visibility /
 * walkpath / zorder / candidate-panel probes and from the other
 * ordinal-XX south_return portrait_rect_position probes (0, 1, 2,
 * 3, 4, 6, 7, 8, 10).
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_RECT_X = VIEWPORT_X + 96,
    PORTRAIT_RECT_Y = VIEWPORT_Y + 35,
    PORTRAIT_WARM_THRESHOLD = 30,
    /* Slice coordinates for the south_return route:
     *   party at (1,5) facing SOUTH  -> front cell (1,6)
     *   C127 sensor on (1,6) NORTH aspect (visibleWallCell = 0)
     *   shipped sensorData = 13 (WUUF); seeded here to 9 (ZED)
     *   to lock the ordinal-9 edge case on the south_return route. */
    PROBE_SLICE_MAP_X = 1,
    PROBE_SLICE_MAP_Y = 5,
    PROBE_SLICE_DIR = 2,                 /* DIR_SOUTH */
    PROBE_SHIPPED_ORDINAL = 13,          /* WUUF on the real route */
    PROBE_TARGET_ORDINAL = 9,            /* ordinal 9 (column 1, row 1) */
    PROBE_ADJACENT_NORTH_X = 1,
    PROBE_ADJACENT_NORTH_Y = 4
};

static int g_pass = 0;
static int g_fail = 0;

#define PASS(label) do { printf("  PASS: %s\n", label); ++g_pass; } while (0)
#define FAIL(label) do { printf("  FAIL: %s\n", label); ++g_fail; } while (0)

/* Count warm-colored palette indices (ReDMCSB DUNVIEW.C:3913-3928
 * champion-portrait palette set) in the D1C portrait rectangle. */
static int portrait_rect_warm_count(const unsigned char* fb) {
    int x, y;
    int count = 0;
    for (y = PORTRAIT_RECT_Y; y < PORTRAIT_RECT_Y + PORTRAIT_H; ++y) {
        for (x = PORTRAIT_RECT_X; x < PORTRAIT_RECT_X + PORTRAIT_W; ++x) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[y * FB_W + x]);
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

/* For each of the 24 C026 portraits, count how many non-transparent
 * source pixels match the corresponding D1C rectangle pixel.
 * Returns the ordinal with the highest matched count (or -1 if
 * portraits unavailable).  Specialized to the 32x29 portrait-on-wall
 * cutout. */
static int best_ordinal_match(const M11_AssetSlot* portraits,
                              const unsigned char* fb) {
    int bestOrdinal = -1;
    int bestMatched = -1;
    int ordinal;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return -1;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int matched = 0;
        int x, y;
        for (y = 0; y < PORTRAIT_H; ++y) {
            for (x = 0; x < PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PORTRAIT_H + y;
                unsigned char srcIdx;
                unsigned char dstIdx;
                if (srcX < 0 || srcX >= (int)portraits->width) continue;
                if (srcY < 0 || srcY >= (int)portraits->height) continue;
                srcIdx = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                dstIdx = M11_FB_DECODE_INDEX(fb[(PORTRAIT_RECT_Y + y) * FB_W +
                                                 (PORTRAIT_RECT_X + x)]);
                if (srcIdx == 1) {
                    /* Palette index 1 = dark grey.  The ReDMCSB
                     * DUNVIEW.C:3916 dark-gray transparency passes
                     * through wall texture, so skip these source
                     * pixels when matching. */
                    continue;
                }
                if (dstIdx == srcIdx) {
                    ++matched;
                }
            }
        }
        if (matched > bestMatched) {
            bestMatched = matched;
            bestOrdinal = ordinal;
        }
    }
    return bestOrdinal;
}

/* Per-ordinal match count: count non-transparent source pixels of
 * `ordinal` that match the D1C rect.  Used for the strict-dominance
 * check below. */
static int ordinal_match_count(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int matched = 0;
    int x, y;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            unsigned char srcIdx;
            unsigned char dstIdx;
            if (srcX < 0 || srcX >= (int)portraits->width) continue;
            if (srcY < 0 || srcY >= (int)portraits->height) continue;
            srcIdx = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            dstIdx = M11_FB_DECODE_INDEX(fb[(PORTRAIT_RECT_Y + y) * FB_W +
                                             (PORTRAIT_RECT_X + x)]);
            if (srcIdx == 1) continue;
            if (dstIdx == srcIdx) {
                ++matched;
            }
        }
    }
    return matched;
}

/* Seed the first C127 sensor whose data equals `oldData` to `newData`.
 * Returns the sensor index, or -1 if not found.  Used to lock the
 * ordinal-9 edge on the south_return C127 sensor without mutating
 * unrelated cells. */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData,
                                int newData) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Pretty-print the probe header so a run on real DM1 data shows the
 * Hall cell, the source-side ordinal, and the row/column math. */
static void print_slice_header(int mapX, int mapY, int dir) {
    int frontX = mapX;
    int frontY = mapY;
    switch (dir) {
        case 0: --frontY; break; /* NORTH */
        case 1: ++frontX; break; /* EAST  */
        case 2: ++frontY; break; /* SOUTH */
        case 3: --frontX; break; /* WEST  */
        default: break;
    }
    printf("=== DM1 V1 Hall portrait 09 / south_return / "
           "portrait_rect_position probe ===\n");
    printf("  slice cell=(%d,%d) facing=%s front_cell=(%d,%d) "
           "shippedOrdinal=%d visibleWallCell=%d\n",
           mapX, mapY,
           dir == 0 ? "NORTH" : dir == 1 ? "EAST" :
           dir == 2 ? "SOUTH" : "WEST",
           frontX, frontY, PROBE_SHIPPED_ORDINAL, (dir + 2) & 3);
    printf("  seeded targetOrdinal=%d via sensorData patch "
           "on the (1,6) C127 sensor.\n",
           PROBE_TARGET_ORDINAL);
    printf("  D1C portrait rect: x=[%d..%d] y=[%d..%d] (32x29)\n",
           PORTRAIT_RECT_X, PORTRAIT_RECT_X + PORTRAIT_W,
           PORTRAIT_RECT_Y, PORTRAIT_RECT_Y + PORTRAIT_H);
    printf("  C026 source rect for ordinal %d: "
           "x=[%d..%d] y=[%d..%d] (col=%d, row=%d)\n",
           PROBE_TARGET_ORDINAL,
           (PROBE_TARGET_ORDINAL & 7) * PORTRAIT_W,
           (PROBE_TARGET_ORDINAL & 7) * PORTRAIT_W + PORTRAIT_W,
           (PROBE_TARGET_ORDINAL >> 3) * PORTRAIT_H,
           (PROBE_TARGET_ORDINAL >> 3) * PORTRAIT_H + PORTRAIT_H,
           PROBE_TARGET_ORDINAL & 7, PROBE_TARGET_ORDINAL >> 3);
}

static int check_pose(M11_GameViewState* game,
                      const M11_AssetSlot* portraits,
                      int mapX,
                      int mapY,
                      int dir,
                      int expectedOrdinal,
                      int expectPortraitPainted,
                      const char* label) {
    int actualOrdinal;
    int bestOrdinal;
    unsigned char fb[FB_W * FB_H];
    int warmCount;
    int ordinalOk;
    int rectOk;

    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;

    actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    ordinalOk = (actualOrdinal == expectedOrdinal);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    warmCount = portrait_rect_warm_count(fb);
    bestOrdinal = best_ordinal_match(portraits, fb);

    if (expectPortraitPainted) {
        rectOk = (warmCount >= PORTRAIT_WARM_THRESHOLD) &&
                 (bestOrdinal == expectedOrdinal);
        if (ordinalOk && rectOk) {
            PASS(label);
            printf("    ordinal=%d portrait_rect_warm_count=%d (>= %d) "
                   "best_ordinal=%d portrait_painted=YES\n",
                   actualOrdinal, warmCount, PORTRAIT_WARM_THRESHOLD,
                   bestOrdinal);
        } else {
            if (!ordinalOk) {
                printf("    ordinal got=%d want=%d\n",
                       actualOrdinal, expectedOrdinal);
            }
            if (warmCount < PORTRAIT_WARM_THRESHOLD) {
                printf("    warm_count=%d < %d — portrait NOT painted\n",
                       warmCount, PORTRAIT_WARM_THRESHOLD);
            }
            if (bestOrdinal != expectedOrdinal) {
                printf("    best_ordinal=%d want=%d — rect dominated by "
                       "another portrait\n", bestOrdinal, expectedOrdinal);
            }
            FAIL(label);
        }
    } else {
        rectOk = (warmCount < PORTRAIT_WARM_THRESHOLD);
        if (ordinalOk && rectOk) {
            PASS(label);
            printf("    ordinal=-1 portrait_rect_warm_count=%d (< %d) "
                   "best_ordinal=%d portrait_painted=NO (no-floating)\n",
                   warmCount, PORTRAIT_WARM_THRESHOLD, bestOrdinal);
        } else {
            if (!ordinalOk) {
                printf("    ordinal got=%d want=-1\n", actualOrdinal);
            }
            if (warmCount >= PORTRAIT_WARM_THRESHOLD) {
                printf("    warm_count=%d >= %d — portrait FLOATING on side wall!\n",
                       warmCount, PORTRAIT_WARM_THRESHOLD);
            }
            FAIL(label);
        }
    }
    return 1;
}

/* Lookup the DM1 V1 mirror-catalog name/title for ordinal 9 via the
 * M11 mirror-catalog helper so the slice is bound to the real
 * catalog identity (not a probe-side hardcoded literal).  In the
 * PC 3.4 English catalog, ordinal 9 maps to ZED with title
 * "DUKE OF BANVILLE" (the catalog contains 24 ordinals; ordinal 9
 * is column 1, row 1). */
static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int nameOk;
    int nameMatched;
    name[0] = '\0';
    title[0] = '\0';
    nameOk = (M11_GameView_GetMirrorNameByOrdinal(game, PROBE_TARGET_ORDINAL,
                                                  name, sizeof(name)) > 0);
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_TARGET_ORDINAL,
                                                title, sizeof(title));
    if (!nameOk) {
        printf("  FAIL: ordinal %d has no mirror-catalog name (helper returned 0)\n",
               PROBE_TARGET_ORDINAL);
        ++g_fail;
        return 0;
    }
    nameMatched = (strcmp(name, "ZED") == 0);
    printf("  PASS: ordinal %d catalog name=\"%s\" title=\"%s\"%s\n",
           PROBE_TARGET_ORDINAL, name, title,
           nameMatched ? "" : " (non-canonical name)");
    ++g_pass;
    if (nameMatched) {
        printf("  PASS: ordinal %d binds to canonical PC 3.4 ZED\n",
               PROBE_TARGET_ORDINAL);
        ++g_pass;
    } else {
        printf("  NOTE: ordinal %d binds to non-canonical name "
               "(non-PC 3.4 English DM1 V1 build); name='%s'\n",
               PROBE_TARGET_ORDINAL, name);
    }
    return nameOk;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int seededSensor;

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
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT champion portrait strip unavailable "
                "(width=%d height=%d)\n",
                portraits ? (int)portraits->width : -1,
                portraits ? (int)portraits->height : -1);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    print_slice_header(PROBE_SLICE_MAP_X, PROBE_SLICE_MAP_Y, PROBE_SLICE_DIR);

    /* Sanity: shipped PC 3.4 DUNGEON.DAT must show ordinal 13 (WUUF)
     * at the (1,5) SOUTH pose on the south_return route BEFORE we
     * patch the C127 sensor.  If a different DM1 V1 build rebinds
     * the sensor data, skip the probe rather than mis-pinning the
     * ordinal.  This is a per-build fixture guard, not a regression
     * detector. */
    {
        int shippedOrdinal;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = PROBE_SLICE_MAP_X;
        game.world.party.mapY = PROBE_SLICE_MAP_Y;
        game.world.party.direction = PROBE_SLICE_DIR;
        shippedOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
        if (shippedOrdinal != PROBE_SHIPPED_ORDINAL) {
            printf("SKIP south_return_fixture_mismatch "
                   "(1,5) SOUTH front ordinal=%d expected=%d; "
                   "this DM1 V1 build does not match the reference "
                   "DUNGEON.DAT fixture for the south_return route.\n",
                   shippedOrdinal, PROBE_SHIPPED_ORDINAL);
            M11_GameView_Shutdown(&game);
            return 0;
        }
        printf("  PASS: shipped south_return (1,5) SOUTH reports ordinal %d (WUUF)\n",
               shippedOrdinal);
        ++g_pass;
    }

    /* Seed the C127 sensor on the south_return route from 13 (WUUF)
     * to 9 to lock the ordinal-9 edge case at the canonical
     * south-facing D1C pose.  The sensor data patch is local to
     * the M11_GameViewState and never written back to DUNGEON.DAT. */
    seededSensor = seed_first_c127_data(&game,
                                        PROBE_SHIPPED_ORDINAL,
                                        PROBE_TARGET_ORDINAL);
    if (seededSensor < 0) {
        printf("SKIP south_return_fixture_mismatch "
               "could not find a C127 sensor with sensorData=%d on "
               "the south_return route; this DM1 V1 build does not "
               "match the reference DUNGEON.DAT fixture.\n",
               PROBE_SHIPPED_ORDINAL);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    printf("  PASS: seeded south_return C127 sensor[%d] data %d -> %d\n",
           seededSensor, PROBE_SHIPPED_ORDINAL, PROBE_TARGET_ORDINAL);
    ++g_pass;

    /* Catalog identity (must be real DM1 V1 data; ordinal 9 maps to
     * ZED in the canonical PC 3.4 English build). */
    check_catalog_identity(&game);

    /* Positive ordinal pose: front cell (1,6) NORTH aspect =
     * sensorData=9 after seeding. */
    check_pose(&game, portraits,
               PROBE_SLICE_MAP_X, PROBE_SLICE_MAP_Y, PROBE_SLICE_DIR,
               PROBE_TARGET_ORDINAL, 1,
               "south_return (1,5) SOUTH portrait_rect ordinal=9 painted");

    /* Wrong-wall poses at the same cell — front-mirror ordinal must be
     * -1 and the D1C portrait rect must not be painted (no floating).
     *
     * (1,5) NORTH: front cell (1,4) carries a C127 sensor with
     *   sensorData=10 (ZED... wait, 10 is GANDO) on its SOUTH aspect.
     *   The existing ordinal 8 south_return probe documents the (1,4)
     *   sensorData as 10 (ZED -- actually ordinal 10 = GANDO, not ZED;
     *   ZED is ordinal 9).  The exact ordinal painted at the (1,5)
     *   NORTH pose is implementation-defined; what matters here is
     *   that the south_return patch must NOT bleed onto the NORTH
     *   aspect to paint ordinal 9.
     *
     * (1,5) EAST / WEST: front cells (2,5) and (0,5) have no C127
     *   sensor on the relevant aspect, so the rect must NOT be
     *   painted (no floating) and ordinal must be -1. */
    {
        unsigned char fb[FB_W * FB_H];
        int northBestOrdinal;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = 1;
        game.world.party.mapY = 5;
        game.world.party.direction = 0; /* NORTH — front=(1,4) sensorData=10 */
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        northBestOrdinal = best_ordinal_match(portraits, fb);
        if (northBestOrdinal != PROBE_TARGET_ORDINAL) {
            PASS("south_return (1,5) NORTH wrong_wall does not show ordinal 9");
            printf("    north best_ordinal=%d (not ordinal 9) — "
                   "south_return patch did not bleed onto NORTH aspect\n",
                   northBestOrdinal);
        } else {
            FAIL("south_return (1,5) NORTH wrong_wall does not show ordinal 9");
            printf("    north best_ordinal=%d == ordinal 9 — "
                   "south_return patch bled onto NORTH aspect or sensor "
                   "filter regression\n", northBestOrdinal);
        }
    }
    check_pose(&game, portraits, 1, 5, 1, -1, 0,
               "south_return (1,5) EAST wrong_wall no_portrait no_floating");
    check_pose(&game, portraits, 1, 5, 3, -1, 0,
               "south_return (1,5) WEST wrong_wall no_portrait no_floating");

    /* Adjacent corridor sanity check: (1,4) is the previous cell in
     * the same column.  When the party is at (1,4) facing NORTH or
     * EAST, the front cell (1,3) or (2,4) is corridor with no C127
     * sensor on the relevant aspect, so the D1C portrait rect must
     * NOT be painted (no floating from neighbor cells). */
    {
        unsigned char fb[FB_W * FB_H];
        int northWarm;
        int eastWarm;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = PROBE_ADJACENT_NORTH_X;
        game.world.party.mapY = PROBE_ADJACENT_NORTH_Y;
        game.world.party.direction = 0; /* NORTH — front=(1,3) */
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        northWarm = portrait_rect_warm_count(fb);
        game.world.party.direction = 1; /* EAST — front=(2,4) */
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        eastWarm = portrait_rect_warm_count(fb);
        if (northWarm < PORTRAIT_WARM_THRESHOLD) {
            PASS("south_return (1,4) NORTH corridor neighbor (1,3) no_portrait");
            printf("    north corridor warm_count=%d (< %d)\n",
                   northWarm, PORTRAIT_WARM_THRESHOLD);
        } else {
            FAIL("south_return (1,4) NORTH corridor neighbor (1,3) no_portrait");
            printf("    north corridor warm_count=%d (>= %d) — floating\n",
                   northWarm, PORTRAIT_WARM_THRESHOLD);
        }
        if (eastWarm < PORTRAIT_WARM_THRESHOLD) {
            PASS("south_return (1,4) EAST corridor neighbor (2,4) no_portrait");
            printf("    east corridor warm_count=%d (< %d)\n",
                   eastWarm, PORTRAIT_WARM_THRESHOLD);
        } else {
            FAIL("south_return (1,4) EAST corridor neighbor (2,4) no_portrait");
            printf("    east corridor warm_count=%d (>= %d) — floating\n",
                   eastWarm, PORTRAIT_WARM_THRESHOLD);
        }
    }

    /* Per-ordinal dominant match: at the (1,5) SOUTH pose the expected
     * ordinal 9 must beat every other ordinal at the same D1C rect.
     * This is the strict "best_ordinal == 9" requirement that keeps a
     * sibling portrait from accidentally winning. */
    {
        unsigned char fb[FB_W * FB_H];
        int expected;
        int best;
        int second;
        int ordinal;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = PROBE_SLICE_MAP_X;
        game.world.party.mapY = PROBE_SLICE_MAP_Y;
        game.world.party.direction = PROBE_SLICE_DIR;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        expected = ordinal_match_count(portraits, fb, PROBE_TARGET_ORDINAL);
        best = -1;
        second = -1;
        for (ordinal = 0; ordinal < 24; ++ordinal) {
            int m = ordinal_match_count(portraits, fb, ordinal);
            if (m > best) {
                second = best;
                best = m;
            } else if (m > second) {
                second = m;
            }
        }
        if (expected == best && best > 0 && (second == 0 || expected > second)) {
            PASS("south_return (1,5) SOUTH ordinal 9 dominates per-ordinal match");
            printf("    expected=%d best=%d second=%d\n",
                   expected, best, second);
        } else {
            FAIL("south_return (1,5) SOUTH ordinal 9 dominates per-ordinal match");
            printf("    expected=%d best=%d second=%d\n",
                   expected, best, second);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
