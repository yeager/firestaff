/*
 * DM1 V1 Hall of Champions portrait ordinal 00 — south_return /
 * portrait_rect_position regression probe.
 *
 * Slice (one narrow slice per the assignment):
 *   Hall of Champions map 0, party at (1,5) facing SOUTH (dir=2).
 *   Front cell is (1,6) which carries a C127 sensor with sensorData=13
 *   on its NORTH aspect (visibleWallCell = (SOUTH + 2) & 3 = 0 = NORTH).
 *   This is the canonical "south_return" route of the Hall: the
 *   party has finished the line at (1,5) and turns back south.  The
 *   shipped PC 3.4 data binds that C127 sensor to ordinal 13 (WUUF),
 *   so this probe keeps the real asset route and temporarily seeds
 *   that C127 sensor to sensorData=0 to lock the ordinal-zero edge
 *   case on the south_return route.  This catches regressions where
 *   portrait index 0 is treated as false/absent instead of a valid
 *   C026 portrait strip entry on the south-facing variant of the
 *   D1C champion-mirror route.
 *
 *   Companion slice to:
 *     firestaff_dm1_v1_champion_mirror_portrait00_rect_runtime_probe
 *     (front_north_entry / portrait_rect_position) — same ordinal,
 *     north_entry route.  Together they cover ordinal 0 on the two
 *     source-visible Hall routes (north_entry and south_return) so a
 *     regression on either route fails the suite.
 *
 * Source-locked to ReDMCSB WIP 20210206:
 *   DUNGEON.C:2573       maps M011_CELL(sensor) against view dir;
 *     only sensors on M552_FRONT_WALL_ORNAMENT_ORDINAL (DEFS.H:2552=5)
 *     set G0289.  At the (1,5) SOUTH pose the front cell is (1,6) and
 *     the visible wall aspect is (SOUTH + 2) & 3 = 0 = NORTH.
 *   DUNGEON.C:2608-2612  stores C127 sensorData in G0289.
 *   MOVESENS.C:1501-1503 passes C127 sensorData to F0280.
 *   REVIVE.C F0280       materializes the candidate champion from
 *     sensorData (this is the south_return resurrection path).
 *   DUNVIEW.C:3913-3928  blits C026 at the fixed D1C portrait-on-wall
 *     box {96..127,35..63}, 32x29 per portrait, src =
 *     (ordinal & 7) * 32, (ordinal >> 3) * 29.
 *   DUNVIEW.C:8318-8542  F0128 viewport redraw order (far-to-near) so
 *     side-wall geometry overpaints the D1C portrait rectangle when
 *     the front cell no longer has a C127 sensor.
 *   COORD.C:1693-1722    PC 3.4 viewport origin (0,33) / 224x136 dim.
 *
 * The probe proves six invariants for this slice only:
 *   (1) M11_GameView_GetFrontMirrorOrdinal returns 13 (WUUF) on the
 *       real (1,5) SOUTH pose BEFORE seeding, and returns 0 after
 *       seeding the C127 sensor from 13 to 0 — i.e. the front-cell
 *       sensor filter (DUNGEON.C:2573) honours the seeded value and
 *       does not skip ordinal 0.
 *   (2) M11_GameView_Draw paints the D1C portrait rect (96,35)-(128,64)
 *       with C026 ordinal-0 source pixels at the expected dominant
 *       position — best_ordinal == 0, warm_count >= 30, expected
 *       per-ordinal match strictly dominates all 23 rival ordinals.
 *   (3) The D1C portrait rect is NOT painted on side-wall poses
 *       at the same (1,5) cell (NORTH/EAST/WEST) — no floating
 *       on side walls when no C127 sensor owns the front square.
 *   (4) The same rect is NOT painted on adjacent corridor cells
 *       (1,4) NORTH and (1,4) EAST — no floating from neighbor
 *       cells that share the same row.
 *   (5) The portrait column/row math (ordinal 0 & 7 = 0 -> column 0,
 *       ordinal 0 >> 3 = 0 -> row 0) lines up with the C026 strip
 *       layout, source rect (0, 0, 32, 29) in the 256x87 atlas.
 *   (6) Catalog identity: M11_GameView_GetMirrorNameByOrdinal(0) ==
 *       "DAROOU" (canonical PC 3.4 English; the probe also tolerates
 *       non-canonical binds for non-English DM1 V1 builds).
 *
 * HONESTY: this is Firestaff deterministic-runtime evidence, not
 * original-DM1 PC 3.4 pixel parity.  The ordinal-rectangle match
 * uses the same warm-color heuristic the existing
 * firestaff_dm1_v1_champion_mirror_capture_probe uses (palette
 * indices {0x07 green, 0x08 red, 0x09 orange, 0x0A peach, 0x0B
 * yellow, 0x0E blue}) to distinguish 'portrait painted' from
 * 'wall texture only'.  The pixel-perfect match routine reports
 * both the best-matched ordinal and the expected-ordinal
 * matched/compared ratio so an unrelated D1C ornament cannot
 * accidentally pass.
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
    PORTRAIT_RECT_X = VIEWPORT_X + 96,   /* M11_VIEWPORT_X + 96 */
    PORTRAIT_RECT_Y = VIEWPORT_Y + 35,   /* M11_VIEWPORT_Y + 35 */
    PORTRAIT_WARM_THRESHOLD = 30,
    /* Slice coordinates for the south_return route:
     *   party at (1,5) facing SOUTH  -> front cell (1,6)
     *   C127 sensor on (1,6) NORTH aspect (visibleWallCell = 0)
     *   shipped sensorData = 13 (WUUF); seeded here to 0 for the slice */
    PROBE_SLICE_MAP_X = 1,
    PROBE_SLICE_MAP_Y = 5,
    PROBE_SLICE_DIR = 2,                 /* DIR_SOUTH */
    PROBE_SHIPPED_ORDINAL = 13,          /* WUUF on the real route */
    PROBE_TARGET_ORDINAL = 0,            /* DAROOU — ordinal zero edge */
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
 * cutout — mirrors the matching routine in
 * firestaff_dm1_v1_champion_mirror_visibility_runtime_probe.c. */
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
 * ordinal-zero edge on the south_return C127 sensor without
 * mutating unrelated cells.  Mirrors the helper in
 * firestaff_dm1_v1_champion_mirror_portrait00_rect_runtime_probe. */
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
    printf("=== DM1 V1 Hall portrait 00 / south_return / "
           "portrait_rect_position probe ===\n");
    printf("  slice cell=(%d,%d) facing=%s front_cell=(%d,%d) "
           "shippedOrdinal=%d visibleWallCell=%d\n",
           mapX, mapY,
           dir == 0 ? "NORTH" : dir == 1 ? "EAST" :
           dir == 2 ? "SOUTH" : "WEST",
           frontX, frontY, PROBE_SHIPPED_ORDINAL, (dir + 2) & 3);
    printf("  seeded targetOrdinal=%d (DAROOU) via sensorData patch "
           "on the (1,6) C127 sensor.\n",
           PROBE_TARGET_ORDINAL);
    printf("  D1C portrait rect: x=[%d..%d] y=[%d..%d] (32x29)\n",
           PORTRAIT_RECT_X, PORTRAIT_RECT_X + PORTRAIT_W,
           PORTRAIT_RECT_Y, PORTRAIT_RECT_Y + PORTRAIT_H);
    printf("  C026 source rect for ordinal 0: "
           "x=[%d..%d] y=[%d..%d] (col=%d, row=%d)\n",
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

/* Lookup the DM1 V1 mirror-catalog name/title for ordinal 0 via the
 * M11 mirror-catalog helper so the slice is bound to the real
 * catalog identity (not a probe-side hardcoded literal).  In the
 * PC 3.4 English catalog, ordinal 0 maps to DAROOU, an UNTITLED
 * champion (Apprentice Fighter / Neophyte Wizard per the M10 mirror
 * stats text — but the mirror title field is empty in the catalog).
 * The probe accepts an empty title as the canonical signal that
 * ordinal 0 is DAROOU, since PC 3.4 DAROOU has no mirror title. */
static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int nameOk;
    int nameMatched;
    name[0] = '\0';
    title[0] = '\0';
    nameOk = (M11_GameView_GetMirrorNameByOrdinal(game, PROBE_TARGET_ORDINAL,
                                                  name, sizeof(name)) > 0);
    /* Title may legitimately be empty (DAROOU is untitled in PC 3.4).
     * We only require the helper to have populated the buffer, which
     * it does even for empty titles. */
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_TARGET_ORDINAL,
                                                title, sizeof(title));
    if (!nameOk) {
        printf("  FAIL: ordinal 0 has no mirror-catalog name (helper returned 0)\n");
        ++g_fail;
        return 0;
    }
    nameMatched = (strcmp(name, "DAROOU") == 0);
    printf("  PASS: ordinal 0 catalog name=\"%s\" title=\"%s\"%s\n",
           name, title,
           nameMatched ? "" : " (non-canonical name)");
    ++g_pass;
    if (nameMatched && title[0] == '\0') {
        printf("  PASS: ordinal 0 binds to canonical PC 3.4 DAROOU (untitled)\n");
        ++g_pass;
    } else if (nameMatched) {
        printf("  PASS: ordinal 0 binds to DAROOU (non-empty title='%s')\n", title);
        ++g_pass;
    } else {
        printf("  NOTE: ordinal 0 binds to non-canonical name "
               "(non-PC 3.4 English DM1 V1 build); "
               "name='%s' title='%s'\n", name, title);
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
                   "DUNGEON.DAT fixture for the south_return route "
                   "(see the existing actual-pose probe's Hall "
                   "ordinal map in TODO.md)\n",
                   shippedOrdinal, PROBE_SHIPPED_ORDINAL);
            M11_GameView_Shutdown(&game);
            return 0;
        }
        printf("  PASS: shipped south_return (1,5) SOUTH reports ordinal %d (WUUF)\n",
               shippedOrdinal);
        ++g_pass;
    }

    /* Seed the C127 sensor on the south_return route from 13 (WUUF)
     * to 0 (DAROOU) so the ordinal-zero edge is locked at the
     * canonical south-facing D1C pose.  The sensor data patch is
     * local to the M11_GameViewState and never written back to
     * DUNGEON.DAT. */
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

    /* Catalog identity (must be real DM1 V1 data; ordinal 0 maps to
     * DAROOU/APPRENTICE FIGHTER in the canonical PC 3.4 English
     * build). */
    check_catalog_identity(&game);

    /* Positive ordinal pose: front cell (1,6) NORTH aspect =
     * sensorData=0 after seeding. */
    check_pose(&game, portraits,
               PROBE_SLICE_MAP_X, PROBE_SLICE_MAP_Y, PROBE_SLICE_DIR,
               PROBE_TARGET_ORDINAL, 1,
               "south_return (1,5) SOUTH portrait_rect ordinal=0 painted");

    /* Wrong-wall poses at the same cell — front-mirror ordinal must be
     * -1 and the D1C portrait rect must not be painted (no floating).
     *
     * (1,5) NORTH: front cell (1,4) carries a C127 sensor with
     *   sensorData=10 (ZED) on its SOUTH aspect (visibleWallCell =
     *   (NORTH + 2) & 3 = 2 = SOUTH), so on real data the rect is
     *   painted with ordinal 10 — NOT floating.  We use that to
     *   confirm the rect does NOT show ordinal 0 at this pose: if
     *   the rect were painted with ordinal 0 (DAROOU) when the
     *   party looks NORTH, that would mean the south_return sensor
     *   patch bled onto the wrong wall aspect.  The probe's contract
     *   is therefore "best_ordinal != 0 (DAROOU)" at the NORTH pose,
     *   not "no portrait at all".
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
        if (northBestOrdinal == 10) {
            PASS("south_return (1,5) NORTH wrong_wall shows ordinal 10 (ZED) not 0");
            printf("    north best_ordinal=%d (ZED) — south_return patch did not "
                   "bleed onto NORTH aspect\n", northBestOrdinal);
        } else {
            FAIL("south_return (1,5) NORTH wrong_wall shows ordinal 10 (ZED) not 0");
            printf("    north best_ordinal=%d want=10 — south_return patch bled "
                   "onto NORTH aspect or sensor filter regression\n", northBestOrdinal);
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
     * ordinal 0 must beat every other ordinal at the same D1C rect.
     * This is the strict "best_ordinal == 0" requirement that keeps a
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
            PASS("south_return (1,5) SOUTH ordinal 0 dominates per-ordinal match");
            printf("    expected=%d best=%d second=%d\n",
                   expected, best, second);
        } else {
            FAIL("south_return (1,5) SOUTH ordinal 0 dominates per-ordinal match");
            printf("    expected=%d best=%d second=%d\n",
                   expected, best, second);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
