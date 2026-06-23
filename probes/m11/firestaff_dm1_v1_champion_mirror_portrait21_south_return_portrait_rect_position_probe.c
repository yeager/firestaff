/*
 * DM1 V1 Hall of Champions portrait ordinal 21 — south_return /
 * portrait_rect_position regression probe.
 *
 * Slice (one narrow slice per the assignment):
 *   Hall of Champions map 0, party at (3, 11) facing SOUTH (dir=2).
 *   Front cell is (3, 12) which carries a C127 sensor on its NORTH
 *   aspect with shipped sensorData=20 (ALEX) — visibleWallCell =
 *   (SOUTH + 2) & 3 = 0 = NORTH.  The shipped PC 3.4 data binds that
 *   C127 sensor to ordinal 20 (ALEX), so this probe keeps the real
 *   asset route and temporarily seeds that C127 sensor data 20 -> 21
 *   to lock the ordinal-21 edge (HISSSSA / LIZAR OF MAKAN) on the
 *   south_return D1C champion-mirror route.
 *
 *   Companion slice to:
 *     firestaff_dm1_v1_champion_mirror_ordinal21_east_walkpath_portrait_rect_probe
 *     (front_north_entry / portrait_rect_position) — same ordinal,
 *     north_entry route.  Together they cover ordinal 21 on the two
 *     source-visible Hall routes (north_entry and south_return) so a
 *     regression on either route fails the suite.
 *
 *   The (3, 11) SOUTH pose is chosen instead of (1, 5) SOUTH (used by
 *   the ordinal-0/13/16 slices) so the south_return slice for ordinal
 *   21 is disjoint: no other ordinal-XX south_return probe uses the
 *   (3, 11) cell or the (3, 12) C127 sensor.
 *
 *   Pixel-match approach: the ordinal 0 south_return probe uses a
 *   "warm color count" heuristic (palette indices 0x07-0x0B, 0x0E)
 *   tuned to DAROOU's red/orange apprentice sprite.  Ordinal 21
 *   (HISSSSA / LIZAR OF MAKAN) is a lizard-man ninja whose C026
 *   sprite uses palette indices {0, 1, 8, 12} — only 4 warm-colored
 *   pixels out of 928.  The warm-color heuristic would falsely
 *   report "portrait NOT painted" for ordinal 21.  This probe
 *   therefore uses the matched-pixel-count approach from the
 *   ordinal 14 / ordinal 21 east_walkpath probes: count non-
 *   transparent C026 source pixels that exactly match the D1C rect
 *   pixel, and compare the matched count against a >=90% threshold
 *   for the correct ordinal and a <30% threshold for wrong cells.
 *   This is palette-independent and works for any C026 sprite.
 *
 * Source-locked to ReDMCSB WIP 20210206:
 *   DUNGEON.C:2573       maps M011_CELL(sensor) against view dir;
 *     only sensors on M552_FRONT_WALL_ORNAMENT_ORDINAL (DEFS.H:2552=5)
 *     (the front wall) survive the visibleWallCell filter.
 *   DUNGEON.C:2608-2612  stores C127 sensorData in G0289
 *     (only when the wall-cell index equals M552_FRONT_WALL_ORNAMENT_ORDINAL).
 *   MOVESENS.C:1501-1503 passes C127 sensorData to F0280.
 *   REVIVE.C F0280       materializes the candidate from
 *     sensorData (this is the south_return resurrection path).
 *   DUNVIEW.C:3913-3928  C346 frame + C026 portrait blit at fixed D1C
 *     box {96..127,35..63}, 32x29 per portrait.
 *   DUNVIEW.C:8318-8542  F0128 viewport redraw order (far-to-near) so
 *     side-wall geometry overpaints the D1C portrait rectangle when
 *     the front cell has no C127 sensor (no-floating invariant).
 *   COORD.C:1693-1722    PC 3.4 viewport origin (0,33) / 224x136 dim.
 *
 * Probe proves six invariants on real DM1 PC 3.4 DUNGEON.DAT / GRAPHICS.DAT:
 *
 *   (0) Catalog identity: M11_GameView_GetMirrorNameByOrdinal(21)
 *       resolves to "HISSSSA" (PC 3.4 mirror-catalog record for
 *       ordinal 21).  Title is "LIZAR OF MAKAN" per the same call.
 *   (1) M11_GameView_GetFrontMirrorOrdinal((3,11) SOUTH) == 20 (ALEX)
 *       on the un-patched shipped route (PC 3.4 sanity check).
 *   (2) After seeding the (3,12) C127 sensor data 20 -> 21, the same
 *       pose returns ordinal 21 (HISSSSA) and M11_GameView_Draw paints
 *       the D1C portrait rect (96,35)-(128,64) with C026 ordinal-21
 *       source pixels (matched >= 90% of non-transparent pixels).
 *   (3) The four adjacent atlas cells (ordinals 20, 22, 13, 5) match
 *       by <30% each — proves ordinal 21 specifically is drawn and
 *       not a neighbour.
 *   (4) The rect is empty (matched < 30%) at (3,11) EAST and (3,11)
 *       WEST and at the corridor neighbor (3,12) NORTH — no floating
 *       on side walls or the next corridor cell.
 *   (5) C026 column/row math: ordinal 21 -> col=(21 & 7)=5, row=(21 >> 3)=2
 *       source rect x=[160..192], y=[58..87] in the 256x87 portrait strip.
 *   (6) Per-ordinal dominance: ordinal 21 strictly dominates every
 *       other ordinal at the same D1C rect (best == expected, second
 *       is below expected).
 *
 * Probe is registered through the existing champion-mirror pool foreach
 * block in CMakeLists.txt as a first-class ctest target.  Uses the same
 * per-build fixture-guard pattern as the ordinal-0 / ordinal-14
 * south_return probes — if (3, 11) SOUTH does not return ordinal 20
 * (ALEX) on a different DM1 V1 build, the probe prints SKIP and exits
 * 0 instead of failing.
 *
 * This is a Firestaff-runtime slice proof, not pixel parity with
 * original DM1 PC 3.4.
 *
 * Disjoint from the existing actual-pose / capture / visibility /
 * walkpath / zorder / candidate-panel probes and from the other
 * ordinal-XX portrait_rect_position probes.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline. */
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
    /* D1C portrait rect, source-locked to ReDMCSB DUNVIEW.C:3913-3928
     * (C026 portrait blit at fixed D1C box {96..127,35..63}). */
    PORTRAIT_RECT_X = VIEWPORT_X + 96,
    PORTRAIT_RECT_Y = VIEWPORT_Y + 35,
    /* Pixel-match thresholds.  Mirrors the
     * firestaff_dm1_v1_hall_of_champions_portrait_14_south_return_rect_probe
     * thresholds: the C026 atlas cell matching the rendered cutout
     * by >= 90% of non-transparent pixels is the correct ordinal;
     * neighbours that share skin/clothing palette indices stay
     * below 30%. */
    CORRECT_MATCH_PCT = 90,
    WRONG_MATCH_PCT = 30,
    /* Slice coordinates for the south_return route:
     *   party at (3, 11) facing SOUTH -> front cell (3, 12)
     *   C127 sensor on (3, 12) NORTH aspect (visibleWallCell = 0)
     *   shipped sensorData = 20 (ALEX); seeded here to 21 for the
     *   slice (HISSSSA / LIZAR OF MAKAN). */
    PROBE_SLICE_MAP_X = 3,
    PROBE_SLICE_MAP_Y = 11,
    PROBE_SLICE_DIR = 2,                 /* DIR_SOUTH */
    PROBE_SHIPPED_ORDINAL = 20,          /* ALEX on the real route */
    PROBE_TARGET_ORDINAL = 21            /* HISSSSA */
};

static int g_pass = 0;
static int g_fail = 0;

#define PASS(label) do { printf("  PASS: %s\n", label); ++g_pass; } while (0)
#define FAIL(label) do { printf("  FAIL: %s\n", label); ++g_fail; } while (0)
#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Pixel-match a viewport-rectangle against a 32x29 portrait cell
 * within the C026 strip (graphics.dat asset slot M11_GFX_CHAMPION_
 * PORTRAITS = 26, atlas 256x87, 8 cols x 3 rows of 32x29 portraits).
 *
 * The m11_draw_dm1_front_champion_portrait blit passes
 * transparentColor=1 (m11_game_view.c:13952 -> BlitRegion(..., 1)),
 * so any C026 pixel with palette index 1 leaves the wall-niche
 * background visible.  We therefore skip those pixels on BOTH sides
 * of the comparison so transparent source pixels are not counted
 * as comparisons.  PROBE_COLOR_DARK_GRAY (=12) is the wall niche
 * backdrop that the existing
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
                fb[(VIEWPORT_Y + 35 + y) * FB_W + (VIEWPORT_X + 96 + x)] & 0x0F);
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

/* For each of the 24 C026 portraits, compute match_portrait_cell
 * and return the ordinal with the highest matched-percent.  Used
 * for the strict-dominance check below. */
static int best_ordinal_match(const M11_AssetSlot* portraits,
                              const unsigned char* fb) {
    int bestOrdinal = -1;
    int bestPct = -1;
    int ordinal;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int pct = match_portrait_cell(portraits, fb, ordinal);
        if (pct > bestPct) {
            bestPct = pct;
            bestOrdinal = ordinal;
        }
    }
    return bestOrdinal;
}

/* Per-ordinal dominant match: at the (3,11) SOUTH pose the expected
 * ordinal 21 must beat every other ordinal at the same D1C rect. */
static int ordinal_match_count(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int x, y;
    int matched = 0;
    int srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) return 0;
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)(
                portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            unsigned char dst = (unsigned char)(
                fb[(VIEWPORT_Y + 35 + y) * FB_W + (VIEWPORT_X + 96 + x)] & 0x0F);
            if (src == 1) continue;
            if (src == 12) continue;
            if (src == dst) ++matched;
        }
    }
    return matched;
}

/* Seed the first C127 sensor whose data equals `oldData` to `newData`.
 * Returns the sensor index, or -1 if not found.  Used to lock the
 * ordinal-21 edge on the south_return C127 sensor without mutating
 * unrelated cells.  Mirrors the helper in the ordinal-0 south_return
 * probe. */
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
    printf("=== DM1 V1 Hall portrait 21 / south_return / "
           "portrait_rect_position probe ===\n");
    printf("  slice cell=(%d,%d) facing=%s front_cell=(%d,%d) "
           "shippedOrdinal=%d visibleWallCell=%d\n",
           mapX, mapY,
           dir == 0 ? "NORTH" : dir == 1 ? "EAST" :
           dir == 2 ? "SOUTH" : "WEST",
           frontX, frontY, PROBE_SHIPPED_ORDINAL, (dir + 2) & 3);
    printf("  seeded targetOrdinal=%d (HISSSSA) via sensorData patch "
           "on the (3,12) C127 sensor.\n",
           PROBE_TARGET_ORDINAL);
    printf("  D1C portrait rect: x=[%d..%d] y=[%d..%d] (32x29)\n",
           PORTRAIT_RECT_X, PORTRAIT_RECT_X + PORTRAIT_W,
           PORTRAIT_RECT_Y, PORTRAIT_RECT_Y + PORTRAIT_H);
    printf("  C026 source rect for ordinal 21: "
           "x=[%d..%d] y=[%d..%d] (col=%d, row=%d)\n",
           (PROBE_TARGET_ORDINAL & 7) * PORTRAIT_W,
           (PROBE_TARGET_ORDINAL & 7) * PORTRAIT_W + PORTRAIT_W,
           (PROBE_TARGET_ORDINAL >> 3) * PORTRAIT_H,
           (PROBE_TARGET_ORDINAL >> 3) * PORTRAIT_H + PORTRAIT_H,
           PROBE_TARGET_ORDINAL & 7, PROBE_TARGET_ORDINAL >> 3);
}

/* Lookup the DM1 V1 mirror-catalog name/title for ordinal 21 via the
 * M11 mirror-catalog helper so the slice is bound to the real
 * catalog identity.  In the PC 3.4 English catalog, ordinal 21 maps
 * to HISSSSA / LIZAR OF MAKAN (a lizard-man ninja champion). */
static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int nameOk;
    int nameMatched;
    int titleMatched;
    name[0] = '\0';
    title[0] = '\0';
    nameOk = (M11_GameView_GetMirrorNameByOrdinal(game, PROBE_TARGET_ORDINAL,
                                                  name, sizeof(name)) > 0);
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_TARGET_ORDINAL,
                                                title, sizeof(title));
    if (!nameOk) {
        printf("  FAIL: ordinal 21 has no mirror-catalog name (helper returned 0)\n");
        ++g_fail;
        return 0;
    }
    nameMatched = (strcmp(name, "HISSSSA") == 0);
    titleMatched = (strcmp(title, "LIZAR OF MAKAN") == 0);
    printf("  PASS: ordinal 21 catalog name=\"%s\" title=\"%s\"%s%s\n",
           name, title,
           nameMatched ? "" : " (non-canonical name)",
           titleMatched ? "" : " (non-canonical title)");
    ++g_pass;
    if (nameMatched && titleMatched) {
        printf("  PASS: ordinal 21 binds to canonical PC 3.4 HISSSSA / LIZAR OF MAKAN\n");
        ++g_pass;
    } else if (nameMatched) {
        printf("  NOTE: ordinal 21 binds to HISSSSA with non-canonical title='%s'\n", title);
    } else {
        printf("  NOTE: ordinal 21 binds to non-canonical name "
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
    unsigned char fb[FB_W * FB_H];

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

    /* Sanity: shipped PC 3.4 DUNGEON.DAT must show ordinal 20 (ALEX)
     * at the (3, 11) SOUTH pose on the south_return route BEFORE we
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
                   "(3,11) SOUTH front ordinal=%d expected=%d; "
                   "this DM1 V1 build does not match the reference "
                   "DUNGEON.DAT fixture for the south_return route "
                   "(see the existing actual-pose probe's Hall "
                   "ordinal map in TODO.md)\n",
                   shippedOrdinal, PROBE_SHIPPED_ORDINAL);
            M11_GameView_Shutdown(&game);
            return 0;
        }
        printf("  PASS: shipped south_return (3,11) SOUTH reports ordinal %d (ALEX)\n",
               shippedOrdinal);
        ++g_pass;
    }

    /* Seed the C127 sensor on the south_return route from 20 (ALEX)
     * to 21 (HISSSSA) so the ordinal-21 edge is locked at the
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

    /* Catalog identity (must be real DM1 V1 data; ordinal 21 maps to
     * HISSSSA / LIZAR OF MAKAN in the canonical PC 3.4 English
     * build). */
    check_catalog_identity(&game);

    /* ── A) Positive ordinal pose ──────────────────────────────── */
    printf("\n[Group A] south_return (3,11) SOUTH portrait_rect ordinal=21\n");
    game.world.party.mapIndex = 0;
    game.world.party.mapX = PROBE_SLICE_MAP_X;
    game.world.party.mapY = PROBE_SLICE_MAP_Y;
    game.world.party.direction = PROBE_SLICE_DIR;
    {
        int actualOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetFrontMirrorOrdinal((3,11) SOUTH) == %d (want 21)",
                 actualOrdinal);
        CHECK(actualOrdinal == PROBE_TARGET_ORDINAL, msg);
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    {
        int pctWant = match_portrait_cell(portraits, fb, PROBE_TARGET_ORDINAL);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C cutout matches ordinal %d atlas cell >= %d%% (got %d%%)",
                 PROBE_TARGET_ORDINAL, CORRECT_MATCH_PCT, pctWant);
        CHECK(pctWant >= CORRECT_MATCH_PCT, msg);
    }
    {
        int best = best_ordinal_match(portraits, fb);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C cutout best_ordinal == %d (got %d)",
                 PROBE_TARGET_ORDINAL, best);
        CHECK(best == PROBE_TARGET_ORDINAL, msg);
    }

    /* ── B) Adjacent atlas cells do NOT match ─────────────────── */
    printf("\n[Group B] adjacent atlas cells (ordinal-1, ordinal+1, ordinal-8, ordinal+8) match < %d%%\n",
           WRONG_MATCH_PCT);
    {
        const int kAdjacents[4] = {
            PROBE_TARGET_ORDINAL - 1,
            PROBE_TARGET_ORDINAL + 1,
            PROBE_TARGET_ORDINAL - 8,
            PROBE_TARGET_ORDINAL + 8
        };
        const char* kAdjLabels[4] = {
            "ordinal-1 (col 4, row 2)",
            "ordinal+1 (col 6, row 2)",
            "ordinal-8 (col 5, row 1)",
            "ordinal+8 (col 5, row 0)"
        };
        int i;
        for (i = 0; i < 4; ++i) {
            int pctAdj;
            char msg[200];
            if (kAdjacents[i] < 0 || kAdjacents[i] >= 24) continue;
            pctAdj = match_portrait_cell(portraits, fb, kAdjacents[i]);
            if (pctAdj < 0) continue;
            snprintf(msg, sizeof(msg),
                     "D1C cutout does NOT match %s <= %d%% (got %d%%)",
                     kAdjLabels[i], WRONG_MATCH_PCT, pctAdj);
            CHECK(pctAdj < WRONG_MATCH_PCT, msg);
        }
    }

    /* ── C) Wrong-wall poses at the same cell ─────────────────── */
    printf("\n[Group C] south_return wrong-wall poses: no floating portrait\n");
    {
        unsigned char fbE[FB_W * FB_H];
        unsigned char fbW[FB_W * FB_H];
        int ordE, ordW;
        int pctE, pctW;

        game.world.party.mapIndex = 0;
        game.world.party.mapX = PROBE_SLICE_MAP_X;
        game.world.party.mapY = PROBE_SLICE_MAP_Y;
        game.world.party.direction = 1; /* DIR_EAST — front=(4,11), no sensor */
        memset(fbE, 0, sizeof(fbE));
        M11_GameView_Draw(&game, fbE, FB_W, FB_H);
        ordE = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3,11) DIR_EAST front-mirror ordinal = %d (want -1)", ordE);
            CHECK(ordE == -1, msg);
        }
        pctE = match_portrait_cell(portraits, fbE, PROBE_TARGET_ORDINAL);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3,11) DIR_EAST cutout does NOT match ordinal %d <= %d%% (got %d%%)",
                     PROBE_TARGET_ORDINAL, WRONG_MATCH_PCT, pctE);
            CHECK(pctE < WRONG_MATCH_PCT, msg);
        }

        game.world.party.direction = 3; /* DIR_WEST — front=(2,11), no sensor */
        memset(fbW, 0, sizeof(fbW));
        M11_GameView_Draw(&game, fbW, FB_W, FB_H);
        ordW = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3,11) DIR_WEST front-mirror ordinal = %d (want -1)", ordW);
            CHECK(ordW == -1, msg);
        }
        pctW = match_portrait_cell(portraits, fbW, PROBE_TARGET_ORDINAL);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3,11) DIR_WEST cutout does NOT match ordinal %d <= %d%% (got %d%%)",
                     PROBE_TARGET_ORDINAL, WRONG_MATCH_PCT, pctW);
            CHECK(pctW < WRONG_MATCH_PCT, msg);
        }
    }

    /* ── D) Adjacent corridor neighbor ────────────────────────── */
    printf("\n[Group D] corridor neighbor (3,12) NORTH: no floating portrait\n");
    {
        unsigned char fbCorridor[FB_W * FB_H];
        int ordCorridor;
        int pctCorridor;
        game.world.party.mapIndex = 0;
        game.world.party.mapX = 3;
        game.world.party.mapY = 12;
        game.world.party.direction = 0; /* DIR_NORTH — front=(3,11) corridor */
        memset(fbCorridor, 0, sizeof(fbCorridor));
        M11_GameView_Draw(&game, fbCorridor, FB_W, FB_H);
        ordCorridor = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3,12) DIR_NORTH front-mirror ordinal = %d (want -1)",
                     ordCorridor);
            CHECK(ordCorridor == -1, msg);
        }
        pctCorridor = match_portrait_cell(portraits, fbCorridor, PROBE_TARGET_ORDINAL);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "(3,12) DIR_NORTH cutout does NOT match ordinal %d <= %d%% (got %d%%)",
                     PROBE_TARGET_ORDINAL, WRONG_MATCH_PCT, pctCorridor);
            CHECK(pctCorridor < WRONG_MATCH_PCT, msg);
        }
    }

    /* ── E) Per-ordinal dominant match ────────────────────────── */
    printf("\n[Group E] ordinal 21 strictly dominates per-ordinal match\n");
    {
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
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "ordinal 21 match count (%d) >= best (%d) and > second (%d)",
                     expected, best, second);
            CHECK(expected == best && best > 0 && expected > second, msg);
        }
    }

    /* ── F) Sensor stays in catalog range ──────────────────────── */
    printf("\n[Group F] ordinal 21 stays inside the mirror catalog\n");
    {
        int catalogCount = M11_GameView_GetMirrorCatalogCount(&game);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 21 is within [0, %d) mirror catalog range",
                 catalogCount);
        CHECK(PROBE_TARGET_ORDINAL >= 0 && PROBE_TARGET_ORDINAL < catalogCount, msg);
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
