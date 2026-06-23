/*
 * firestaff_dm1_v1_hoc_champion_portrait_05_approach_from_right_portrait_rect_position_runtime_probe.c
 *
 * Source-locked DM1 V1 Hall of Champions slice:
 *
 *   ordinal 5              (mirror catalog record ELIJA, atlas col 5 row 0)
 *   route  approach_from_right: approach the ordinal-5 C127 sensor cell
 *                              (1,1) from the right (east) side.  The
 *                              sensor is anchored to the NORTH wall of
 *                              (1,1) (M011_CELL=0), so:
 *
 *                                - (2,1) DIR_WEST sees the EAST wall of
 *                                  (1,1); the C127 sensor does NOT match
 *                                  and the engine must return -1.
 *                                - (1,1) DIR_EAST sees the WEST wall of
 *                                  (1,1); the C127 sensor does NOT match
 *                                  and the engine must return -1.
 *                                - (1,2) DIR_EAST sees the SOUTH wall of
 *                                  (1,1); the C127 sensor does NOT match
 *                                  and the engine must return -1.
 *                                - (1,1) DIR_SOUTH sees the NORTH wall of
 *                                  (1,1) from a wrong-side pose; the
 *                                  engine must also return -1 here
 *                                  because ReDMCSB DUNGEON.C:2573 compares
 *                                  the sensor's M011_CELL against
 *                                  (direction+2)&3 of the party, not the
 *                                  cell's M011_CELL directly.
 *
 *                              The D1C portrait_rect_position
 *                              (96, 35, 32, 29) in viewport coords
 *                              (DUNVIEW.C:525 G0109 portrait box
 *                              {96, 127, 35, 63}, source-locked inner
 *                              cutout) must therefore contain no ordinal-5
 *                              pixels at any of the right-side approach
 *                              poses.  A regression that re-paints
 *                              ordinal 5 over the side walls would push
 *                              the C026 ordinal-5 pixel match above the
 *                              35% wrong-ordinal drift threshold the
 *                              existing per-ordinal west_negative probes
 *                              lock.
 *   aspect portrait_rect_position: viewport rectangle (96, 35, 32, 29)
 *                                 + portrait atlas math (col 5 row 0,
 *                                 source rect (160, 0, 32, 29)) and the
 *                                 +1 right-side cross-check (1,2,N) and
 *                                 +1 atlas round-trip.
 *
 * The probe is the source-visible right-side counterpart of
 * firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe
 * (which covers the +1 north pose (1,2,N) only) and
 * firestaff_dm1_v1_hall_of_champions_portrait_05_cancel_reopen
 * _portrait_rect_position_runtime_probe (which covers the select ->
 * cancel -> reopen state-machine slice on the same (1,2,N) pose).
 * This probe narrows the geographic coverage to the right-side
 * (east) approach band and locks the no-floating invariant.
 *
 * The DM1 V1 DUNGEON.DAT shipped with the public PC 3.4 English
 * release places the (1,2) NORTH-route C127 sensor at sensorData=1
 * (HALK), so we discover the actual ordinal-5 sensor cell by
 * scanning the dungeon and (when the build has an ordinal-5 sensor
 * on (1,1) NORTH, which the local PC 3.4 fixture does) we use that
 * cell.  When the local DUNGEON.DAT does not expose ordinal 5
 * directly we seed the (1,2) NORTH C127 sensor from HALK to ordinal
 * 5 (same pattern the cancel_reopen probe uses) and then drive the
 * right-side approach band against the same sensorData=5 cell.
 *
 * Source evidence:
 *   - DUNGEON.C:2573 maps M011_CELL(sensor) against view direction
 *     (PC 3.4 I34E builds, M552=5; visibleWallCell = (dir+2)&3)
 *   - DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   - DUNVIEW.C:3913-3928 / 8522-8533 C026 portrait blit into
 *     G0109 portrait box (D1C only — M587_VIEW_WALL_D1C_FRONT)
 *   - DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *     = {96, 127, 35, 63}
 *   - DUNVIEW.C:3916-3919 C026_GRAPHIC_CHAMPION_PORTRAITS,
 *     "A portrait is 32x29 pixels"
 *   - COORD.C:1748-1749 G2078_C32_PortraitWidth=32,
 *     G2079_C29_PortraitHeight=29
 *   - COORD.C:1693-1749 PC34 viewport origin
 *   - DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y macro math
 *   - MOVESENS.C:1501-1503 sensorData -> F0280 candidate ordinal
 *   - REVIVE.C F0280:124-132 C040 empty-leader candidate gate
 *   - DEFS.H:1284 C127_SENSOR_WALL_CHAMPION_PORTRAIT=127
 *   - DEFS.H:2552 M552_FRONT_WALL_ORNAMENT_ORDINAL=5
 *   - m11_front_cell_mirror_ordinal (src/engine/m11_game_view.c:11652)
 *   - m11_draw_dm1_front_champion_portrait
 *     (src/engine/m11_game_view.c:13952 — D1C only)
 *   - m11_get_front_cell (src/engine/m11_game_view.c:11708)
 *   - m11_disable_front_mirror_route
 *     (src/engine/m11_game_view.c:7898 — F0282 sensor disable)
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe
 *     - locks the +1 (1,2,N) positive route and the same-cell
 *       non-NORTH negative band.  Does NOT cover the (2,1)W /
 *       (1,1)E right-side approach band.
 *   firestaff_dm1_v1_hall_of_champions_portrait_05_cancel_reopen
 *     _portrait_rect_position_runtime_probe
 *     - locks the (1,2,N) select -> cancel -> reopen state-machine
 *       slice (Group A-D).  Does NOT cover the right-side approach
 *       band and does NOT cover the no-floating pixel contract for
 *       ordinal 5 from any non-(1,2,N) pose.
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     - lists the (1,y) corridor sensor layout.  Does NOT cover
 *       ordinal 5 (the corridor cells it lists are 1, 3, 10, 4,
 *       15, 13).
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 *     - locks the (1,3)->(3,3) NORTH forward-walk route through
 *       ordinals 1, -1, 19.  Does NOT cover ordinal 5.
 *
 * Honesty:
 *   This is Firestaff deterministic runtime evidence.  It does NOT
 *   claim DOS pixel parity because no paired original DM1 PC 3.4
 *   screenshot covers the ordinal-5 (1,1) wall from the (2,1)
 *   west or (1,1) east poses (those are non-source-visible wrong-
 *   side approaches).  The probe drives real Firestaff game-view
 *   state through the same M11 input pipeline the live game uses,
 *   and the no-floating pixel contract is computed against the
 *   local C026 strip pulled from the same GRAPHICS.DAT the runtime
 *   is drawing from.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

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
    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109).  Inner cutout is the (96, 35, 32, 29)
     * portion of the (80, 29, 64, 43) wall ornament box. */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* The C026 champion portrait atlas is 256x87 pixels (8 cols x
     * 3 rows of 32x29 portraits, DUNVIEW.C:3916-3919).  Ordinal 5
     * sits at (col=5, row=0) which the C027 macro math from
     * DEFS.H:821-826 computes as ((5 & 7) << 5, (5 >> 3) * 29) =
     * (160, 0). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    ORDINAL_5_COL = 5 & 7,
    ORDINAL_5_ROW = 5 >> 3,
    ORDINAL_5_SRC_X = ORDINAL_5_COL << 5,
    ORDINAL_5_SRC_Y = ORDINAL_5_ROW * 29,
    /* The DM1 V1 DUNGEON.DAT ships the (1,2) NORTH-route front
     * square C127 sensor with sensorData=1 (HALK).  When the local
     * DUNGEON.DAT has no ordinal-5 sensor at all, we seed the
     * (1,2)N C127 sensor from HALK to ordinal 5 (the same pattern
     * the cancel_reopen probe uses) so the probe still drives a
     * real ordinal-5 route on the right-side approach band. */
    SHIPPED_HALK_ORDINAL = 1,
    TARGET_ORDINAL = 5,
    /* The wrong-ordinal drift threshold the existing per-ordinal
     * west_negative / south_return / east_walkpath probes lock.
     * Above 35% means a stale portrait sprite is floating on a
     * wall that should not carry the ordinal.  This is the same
     * threshold firestaff_dm1_v1_champion_mirror_ordinal_2_west
     * _negative_portrait_rect_position_runtime_probe locks. */
    WRONG_ORDINAL_MATCH_PCT = 35,
    /* Cross-check threshold for the +1 (1,2,N) positive route:
     * the D1C cutout must carry the ordinal-5 portrait at
     * >= 90% pixel match. */
    POSITIVE_ORDINAL_MATCH_PCT = 90,
    /* Match floor for the corridor / surround to prove the
     * framebuffer is alive at the right-side approach poses. */
    RECT_ALIVE_DISTINCT = 3,
    /* Warm-color palette indices (skin/clothing/face tones) for
     * the no-floating proof.  Same set the cancel_reopen probe
     * uses. */
    PORTRAIT_TRANSPARENT = 1
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Mirror catalog record name for ordinal 5. */
static const char kExpectedCatalogName[] = "ELIJA";

/* Pixel-match a single 32x29 C026 cell against the D1C portrait
 * cutout.  Returns matched-percent (0..100) or -1 if the asset is
 * missing.  Source pixels with palette index 1 (C01_COLOR_DARK_GRAY
 * blitter transparent) are skipped so the wall-niche background
 * bleed does not skew the match.  Same algorithm the per-ordinal
 * west_negative probes use. */
static int match_portrait_cell(const M11_AssetSlot* portraits,
                               const unsigned char* fb,
                               int ordinal) {
    int matched = 0, compared = 0, x, y;
    int srcX0, srcY0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return -1;
    if ((int)portraits->width < ATLAS_W || (int)portraits->height < ATLAS_H) {
        return -1;
    }
    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)(
                portraits->pixels[(srcY0 + y) * (int)portraits->width + (srcX0 + x)] & 0x0F);
            unsigned char dst = M11_FB_DECODE_INDEX(
                fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            if (src == PORTRAIT_TRANSPARENT) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count opaque (non-zero, non-transparent) pixels in the C026
 * atlas cell for the requested ordinal.  Used to lock the ordinal-5
 * atlas cell is a defined portrait and not an unused slot. */
static int atlas_cell_opaque_count(const M11_AssetSlot* portraits,
                                   int ordinal) {
    int x, y, cnt = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcX = (ordinal & 7) * PORTRAIT_W;
    srcY = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int sx = srcX + x, sy = srcY + y;
            unsigned char src;
            if (sx >= (int)portraits->width || sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != PORTRAIT_TRANSPARENT) ++cnt;
        }
    }
    return cnt;
}

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent
 * of pixels that differ.  Used to lock ordinal 5 is a distinct
 * portrait from its row-0 neighbours (4 LEIF, 6 SYRA).  The DM1
 * champion-portrait atlas carries 24 distinct champions, so a
 * duplicate would be a real regression. */
static int atlas_cell_distinct_percent(const M11_AssetSlot* portraits,
                                       int ordinalA, int ordinalB) {
    int x, y, compared = 0, different = 0;
    int srcAX, srcAY, srcBX, srcBY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcAX = (ordinalA & 7) * PORTRAIT_W;
    srcAY = (ordinalA >> 3) * PORTRAIT_H;
    srcBX = (ordinalB & 7) * PORTRAIT_W;
    srcBY = (ordinalB >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char a = (unsigned char)(
                portraits->pixels[(srcAY + y) * (int)portraits->width + (srcAX + x)] & 0x0F);
            unsigned char b = (unsigned char)(
                portraits->pixels[(srcBY + y) * (int)portraits->width + (srcBX + x)] & 0x0F);
            ++compared;
            if (a != b) ++different;
        }
    }
    return (compared > 0) ? (different * 100 / compared) : 0;
}

/* Count distinct non-zero palette indices in a viewport rect.
 * Used to prove the right-side approach pose still has rendered
 * content (floor / wall / door frame) so the empty D1C cutout
 * cannot be explained away by "the framebuffer was never
 * painted".  Same helper the west_negative probes use. */
static int rect_distinct_nonzero(const unsigned char* fb,
                                 int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int n = 0;
    int xx, yy;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (idx != 0 && !seen[idx]) { seen[idx] = 1; ++n; }
        }
    }
    return n;
}

/* Drive M11_GameView_Draw at the given pose and return the rendered
 * framebuffer.  The candidate / inventory state is reset so the
 * no-floating contract is clean. */
static void render_at(M11_GameViewState* state,
                      unsigned char* fb,
                      int mapX, int mapY, int direction) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
    memset(fb, 0, (size_t)FB_W * (size_t)FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found.  Same helper
 * the cancel_reopen probe uses. */
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

/* Discover whether the local DUNGEON.DAT already exposes ordinal 5
 * on the (1,1) NORTH wall.  Returns 1 if a (1,1) DIR_NORTH pose
 * resolves to ordinal 5 (no seed needed), 0 otherwise.  This
 * keeps the probe runtime-real on DM1 V1 builds that already
 * carry ordinal 5 on (1,1) (some custom-port / ROM-hack
 * distributions do) and falls back to the seed path on the
 * shipped PC 3.4 English fixture. */
static int has_native_ordinal5_north(M11_GameViewState* state) {
    int x, y, dir;
    if (!state || !state->active || !state->world.dungeon ||
        state->world.dungeon->header.mapCount <= 0) {
        return 0;
    }
    state->world.party.mapIndex = 0;
    for (x = 0; x < (int)state->world.dungeon->maps[0].width; ++x) {
        for (y = 0; y < (int)state->world.dungeon->maps[0].height; ++y) {
            for (dir = 0; dir < 4; ++dir) {
                state->world.party.mapX = x;
                state->world.party.mapY = y;
                state->world.party.direction = dir;
                if (M11_GameView_GetFrontMirrorOrdinal(state) == TARGET_ORDINAL) {
                    state->world.party.mapX = 1;
                    state->world.party.mapY = 1;
                    state->world.party.direction = 0; /* DIR_NORTH */
                    return (x == 1 && y == 1 && dir == 0);
                }
            }
        }
    }
    return 0;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int seededSensor = -1;
    int nativeOrdinal5 = 0;
    int ord;
    int pct, distinct, nameRc;
    char nameBuf[32];
    int ordinal5Opaque, ordinal5Vs4, ordinal5Vs6;
    unsigned char fbEastOfWallWest[FB_W * FB_H];
    unsigned char fbWallEast[FB_W * FB_H];
    unsigned char fbSouthOfWallEast[FB_W * FB_H];
    unsigned char fbOnCellSouth[FB_W * FB_H];
    unsigned char fbPositiveNorth[FB_W * FB_H];
    /* Right-side approach band.  Each entry is a pose whose front
     * square is the ordinal-5 sensor cell (1,1) but whose visible
     * wall side is NOT the sensor's M011_CELL=0 (north).  The
     * engine must return -1 and the D1C cutout must not contain
     * ordinal-5 pixels. */
    struct {
        int mapX;
        int mapY;
        int dir;
        const char* label;
    } rightApproach[] = {
        {2, 1, 3 /* DIR_WEST */,  "approach_from_right_2_1_W"},
        {1, 1, 1 /* DIR_EAST */,  "approach_from_right_1_1_E"},
        {1, 2, 1 /* DIR_EAST */,  "approach_from_right_1_2_E"},
        {1, 1, 2 /* DIR_SOUTH */,  "approach_from_right_1_1_S_wrong_side"},
    };
    int i;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-05 / approach_from_right / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.inventoryPanelActive = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < ATLAS_W || portraits->height < ATLAS_H) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group A - C026 atlas math for ordinal 5
     * ----------------------------------------------------------------
     * Lock the (col=5, row=0) source rect (160, 0, 32, 29), the
     * opaque-pixel floor, the neighbour distinctness, and the
     * catalog resolution to ELIJA.  Identical to the Group A checks
     * in the cancel_reopen probe so the right-side approach slice
     * inherits the same atlas self-consistency contract. */
    printf("\n[Group A] C026 atlas math for ordinal 5\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == 256, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == 87, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 col = 5 & 7 = %d (expected 5)",
                 ORDINAL_5_COL);
        CHECK(ORDINAL_5_COL == 5, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 row = 5 >> 3 = %d (expected 0)",
                 ORDINAL_5_ROW);
        CHECK(ORDINAL_5_ROW == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 srcX = %d, srcY = %d (within 256x87 atlas)",
                 ORDINAL_5_SRC_X, ORDINAL_5_SRC_Y);
        CHECK(ORDINAL_5_SRC_X + PORTRAIT_W <= ATLAS_W &&
              ORDINAL_5_SRC_Y + PORTRAIT_H <= ATLAS_H, msg);
    }

    ordinal5Opaque = atlas_cell_opaque_count(portraits, 5);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 atlas cell opaque count = %d (>= 100, defined portrait)",
                 ordinal5Opaque);
        CHECK(ordinal5Opaque >= 100, msg);
    }
    ordinal5Vs4 = atlas_cell_distinct_percent(portraits, 5, 4);
    ordinal5Vs6 = atlas_cell_distinct_percent(portraits, 5, 6);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 vs ordinal 4 differ by %d%% (>= 30%%)",
                 ordinal5Vs4);
        CHECK(ordinal5Vs4 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 vs ordinal 6 differ by %d%% (>= 30%%)",
                 ordinal5Vs6);
        CHECK(ordinal5Vs6 >= 30, msg);
    }

    nameBuf[0] = '\0';
    nameRc = M11_GameView_GetMirrorNameByOrdinal(&state, TARGET_ORDINAL,
                                                 nameBuf, (int)sizeof(nameBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog ordinal 5 = \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameRc > 0 && strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - Acquire the ordinal-5 sensor cell
     * ----------------------------------------------------------------
     * The shipped PC 3.4 English DUNGEON.DAT places the (1,2)N
     * C127 sensor at sensorData=1 (HALK), not 5 (ELIJA).  If the
     * local build has no ordinal-5 sensor on (1,1)N we seed the
     * (1,2)N sensor from HALK to ordinal 5 (same pattern the
     * cancel_reopen probe uses) so the right-side approach band
     * is driven against a real C127 sensor with sensorData=5. */
    printf("\n[Group B] Acquire ordinal-5 sensor on (1,1) NORTH\n");

    nativeOrdinal5 = has_native_ordinal5_north(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "local DUNGEON.DAT has ordinal 5 on (1,1) NORTH natively (got %d)",
                 nativeOrdinal5);
        printf("  INFO: %s\n", msg);
    }

    if (!nativeOrdinal5) {
        /* Seed (1,2)N front square C127 from HALK to ordinal 5.
         * Same sensor, same DUNGEON.DAT, same draw path - only
         * G0289 shifts.  The seed is reset on probe exit by
         * M11_GameView_Shutdown, so the user's save state is not
         * affected. */
        seededSensor = seed_first_c127_data(&state,
                                             SHIPPED_HALK_ORDINAL,
                                             TARGET_ORDINAL);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "seeded (1,2) NORTH C127 from HALK(%d) to ordinal %d "
                     "(sensor index %d)",
                     SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
            CHECK(seededSensor >= 0, msg);
        }
    }

    /* Confirm the +1 (1,2,N) positive route resolves to ordinal 5
     * after the seed.  This is the cross-check that proves the
     * same sensor is what the right-side approach band is being
     * measured against. */
    state.world.party.mapIndex = 0;
    state.world.party.mapX = 1;
    state.world.party.mapY = 2;
    state.world.party.direction = 0; /* DIR_NORTH */
    ord = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(1,2) DIR_NORTH front mirror ordinal = %d (expected %d)",
                 ord, TARGET_ORDINAL);
        CHECK(ord == TARGET_ORDINAL, msg);
    }
    if (ord != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d; "
                "cannot verify approach_from_right\n", TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Sanity-check the public D1C wall ornament zone helper, then
     * verify the inner portrait rectangle (96, 35, 32, 29) sits
     * inside that zone. */
    {
        int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
        int rc = M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_GetD1CWallOrnamentZone returns 1 (got %d)",
                 rc);
        CHECK(rc == 1, msg);
        snprintf(msg, sizeof(msg),
                 "D1C wall box = (%d, %d, %d, %d) (expected 80, 29, 64, 43)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) sits inside the D1C wall box");
        CHECK(96 >= ornX && 96 + PORTRAIT_W <= ornX + ornW &&
              35 >= ornY && 35 + PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - approach_from_right negative-route pixel contract
     * ----------------------------------------------------------------
     * For each right-side approach pose, M11_GameView_GetFrontMirror
     * Ordinal must return -1 (the C127 sensor's M011_CELL=0 north
     * wall does not match the visible wall side at that pose) and
     * the D1C portrait cutout (96, 35, 32, 29) must not carry
     * ordinal-5 pixels.  A regression that paints the ordinal-5
     * portrait on a side wall would push the C026 pixel match
     * above the 35% drift threshold. */
    printf("\n[Group C] approach_from_right negative-route pixel contract\n");
    for (i = 0; i < (int)(sizeof(rightApproach) / sizeof(rightApproach[0])); ++i) {
        unsigned char* fb;
        switch (i) {
            case 0: fb = fbEastOfWallWest; break;
            case 1: fb = fbWallEast;       break;
            case 2: fb = fbSouthOfWallEast; break;
            default: fb = fbOnCellSouth;   break;
        }
        render_at(&state, fb,
                  rightApproach[i].mapX, rightApproach[i].mapY,
                  rightApproach[i].dir);
        ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "%s: front mirror ordinal = %d (expected -1)",
                     rightApproach[i].label, ord);
            CHECK(ord == -1, msg);
        }
        /* The corridor / wall / floor must still have rendered
         * content at the right-side approach pose, so an empty D1C
         * cutout cannot be explained away by "the framebuffer was
         * never painted". */
        distinct = rect_distinct_nonzero(fb,
                                         VIEWPORT_X + 0,
                                         VIEWPORT_Y + 30,
                                         96, 60);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "%s: left half of viewport has >= %d distinct "
                     "non-zero palette indices (got %d) - framebuffer "
                     "is alive, empty portrait cutout is meaningful",
                     rightApproach[i].label, RECT_ALIVE_DISTINCT, distinct);
            CHECK(distinct >= RECT_ALIVE_DISTINCT, msg);
        }
        /* The D1C portrait cutout must NOT carry ordinal-5 pixels. */
        pct = match_portrait_cell(portraits, fb, TARGET_ORDINAL);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "%s: D1C cutout ordinal-5 pixel match = %d%% "
                     "(expected < %d%%, no floating portrait on right side)",
                     rightApproach[i].label, pct, WRONG_ORDINAL_MATCH_PCT);
            CHECK(pct >= 0 && pct < WRONG_ORDINAL_MATCH_PCT, msg);
        }
        printf("  INFO: %s map=(%d,%d) dir=%d ord=%d match=%d%% distinct=%d\n",
               rightApproach[i].label,
               rightApproach[i].mapX, rightApproach[i].mapY, rightApproach[i].dir,
               ord, pct, distinct);
    }

    /* ----------------------------------------------------------------
     * Group D - +1 (1,2,N) positive-route cross-check
     * ----------------------------------------------------------------
     * Render the (1,2,N) pose where the C127 sensor's M011_CELL=0
     * matches the visible wall side.  The D1C cutout must carry
     * ordinal-5 pixels at >= 90% match.  This is the cross-check
     * that proves the right-side approach band is being measured
     * against the same sensorData=5 sensor and that the empty
     * right-side rectangle is not silently dead. */
    printf("\n[Group D] +1 (1,2,N) positive-route cross-check\n");
    render_at(&state, fbPositiveNorth, 1, 2, 0 /* DIR_NORTH */);
    ord = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(1,2) DIR_NORTH front mirror ordinal = %d (expected %d)",
                 ord, TARGET_ORDINAL);
        CHECK(ord == TARGET_ORDINAL, msg);
    }
    pct = match_portrait_cell(portraits, fbPositiveNorth, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(1,2) DIR_NORTH D1C cutout ordinal-5 match = %d%% "
                 "(expected >= %d%%, positive route paints ordinal 5)",
                 pct, POSITIVE_ORDINAL_MATCH_PCT);
        CHECK(pct >= POSITIVE_ORDINAL_MATCH_PCT, msg);
    }
    distinct = rect_distinct_nonzero(fbPositiveNorth,
                                     VIEWPORT_X + 0,
                                     VIEWPORT_Y + 30,
                                     96, 60);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(1,2) DIR_NORTH left half of viewport has >= %d distinct "
                 "non-zero palette indices (got %d)",
                 RECT_ALIVE_DISTINCT, distinct);
        CHECK(distinct >= RECT_ALIVE_DISTINCT, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 5 must be self-consistent
     * and the catalog/atlas pair must agree on the (col=5, row=0)
     * source rect.  Same shape as the cancel_reopen probe's
     * Group D so the right-side approach slice inherits the same
     * round-trip contract. */
    printf("\n[Group E] ordinal 5 atlas round-trip\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 5 is at source "
                 "(%d, %d, %d, %d) (matches ((5&7)<<5, (5>>3)*29, 32, 29))",
                 ORDINAL_5_SRC_X, ORDINAL_5_SRC_Y,
                 PORTRAIT_W, PORTRAIT_H);
        CHECK(ORDINAL_5_SRC_X == 160 && ORDINAL_5_SRC_Y == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 opaque count = %d (in expected 200..900 range)",
                 ordinal5Opaque);
        CHECK(ordinal5Opaque >= 200 && ordinal5Opaque <= 900, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 distinctness: vs 4 = %d%%, vs 6 = %d%% (both >= 30%%)",
                 ordinal5Vs4, ordinal5Vs6);
        CHECK(ordinal5Vs4 >= 30 && ordinal5Vs6 >= 30, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
