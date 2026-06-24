/*
 * firestaff_dm1_v1_hoc_champion_portrait_06_input_focus_restore_portrait_rect_position_174_gate_probe.c
 *
 * Source-locked verification gate for one narrow DM1 V1 Hall of
 * Champions slice:
 *
 *   ordinal  6     (SYRA / "CHILD OF NATURE", C026 col 6 row 0 -> source
 *                  rect (192, 0, 32, 29) inside the 256x87 atlas. The
 *                  shipped DM1 V1 PC 3.4 DUNGEON.DAT places a C127
 *                  sensor with sensorData=1 (HALK) on cell (1, 1) at
 *                  the NORTH wall, so the front_mirror route is
 *                  reached by parking at (1, 2) facing NORTH (dir=0)
 *                  and seeding the same C127 sensor from data=1 to
 *                  data=6 to lock the ordinal-6 edge case without
 *                  changing the map layout.  Same sensor, same
 *                  DUNGEON.DAT, same draw path - only the G0289
 *                  ordinal that DUNVIEW.C:3913-3928 reads through
 *                  M000_INDEX_TO_ORDINAL (DUNGEON.C:2610-2612) is
 *                  shifted for the test.)
 *   route   input_focus_restore (open C040 candidate panel via
 *                                SelectFrontMirrorCandidate ->
 *                                HandleInput(BACK) for the C162 cancel
 *                                arm OR HandleInput(ACTION) for the
 *                                C160 confirm arm -> follow-up
 *                                HandleInput(TURN_RIGHT) is now REDRAW,
 *                                rotates the party, and advances the
 *                                gameplay tick, proving the engine
 *                                returned input focus to the world after
 *                                the modal panel closed)
 *   aspect  portrait_rect_position (D1C cutout at viewport (96, 35)
 *                                   sized 32x29 inside the C346
 *                                   wall-mirror frame (80, 29, 64, 43),
 *                                   source-locked to DUNVIEW.C:3913-3928
 *                                   and DUNVIEW.C:525 G0109_Graphic558
 *                                   _Box_ChampionPortraitOnWall.)
 *
 * This probe is intentionally disjoint from the existing ordinal-6
 * gates:
 *
 *   * firestaff_dm1_v1_hall_of_champions_portrait_06_cancel_reopen_*
 *     - Drives SelectFrontMirrorCandidate + CancelMirrorCandidate
 *       through the public helpers, framebuffer state across the
 *       select/cancel/select panel transitions. Does NOT exercise
 *       HandleInput across the panel open/close transition, so it
 *       cannot prove the input focus was actually returned to the
 *       world after the modal panel closed.  The public
 *       CancelMirrorCandidate / SelectFrontMirrorCandidate helpers
 *       do not route through HandleInput, so a "BACK fires cancel"
 *       keyboard regression in the input dispatcher would be
 *       invisible to that probe.  The input_focus_restore route
 *       covered here goes through HandleInput, which is the only
 *       path that exercises M11_GAME_INPUT_IGNORED vs REDRAW for
 *       the candidate panel guard at m11_game_view.c:8303-8314 and
 *       the world re-entry path at m11_game_view.c:8371-8554.
 *
 *   * firestaff_dm1_v1_champion_mirror_ordinal_6_d2l_negative_*
 *     - Anchored at (2, 4) EAST, no C127 seed needed (uses the
 *       ordinal-6 / MOPHUS sensor on the side wall), no candidate
 *       panel, no input dispatch.  Covers the (192, 0, 32, 29) C026
 *       source cell on a different D1C frame pose and verifies
 *       depth-2 side walls (D2L / D1L / D2R) do NOT carry the
 *       portrait sprite.  This probe targets the (1, 2) NORTH
 *       front-mirror route with the C127 sensor seeded to ordinal
 *       6, opens the C040 panel, and exercises the input-dispatcher
 *       BACK/ACTION focus-restore contract.
 *
 *   * firestaff_dm1_v1_champion_mirror_ordinal_6_south_return_*
 *     - Anchored at (2, 17) DIR_SOUTH, no C127 seed (real
 *       ordinal-6 / MOPHUS sensor on the back wall from the (2, 18)
 *       NORTH step), no candidate panel, no input dispatch.
 *
 *   * firestaff_dm1_v1_champion_mirror_ordinal_6_west_negative_*
 *     - Corridor west_negative no-floating, no C040 panel, no input
 *       dispatch.
 *
 *   * firestaff_dm1_v1_hall_champion_portrait_06_east_walkpath_*
 *     - Corridor east_walkpath (synthetic atlas-slot blit), no
 *       candidate panel, no input dispatch.
 *
 *   * firestaff_dm1_v1_hall_of_champions_portrait_06_leave_and_reenter_*
 *     - Leave-and-reenter movement slice (TURN_RIGHT + UP + TURN +
 *       UP), no C040 panel, no input dispatcher BACK/ACTION path.
 *
 *   * firestaff_dm1_v1_hoc_champion_portrait_06_door_nearby_no_float_*
 *     - Door-block INPUT_UP slice on the (1, 2) NORTH closed-mirror-
 *       door pose, no C040 panel, no BACK/ACTION focus restore.
 *
 *   * firestaff_dm1_v1_hoc_champion_portrait_01_input_focus_restore_*
 *     - Same route variant, but ordinal 1 (HALK) with the shipped
 *       sensorData=1 (no sensor seed).  Ordinal 6 is a different
 *       C026 source cell (col 6 row 0, 192,0) and a different
 *       mirror-catalog record (SYRA / CHILD OF NATURE vs HALK / THE
 *       BARBARIAN); the row-0 rightmost ordinal is the column
 *       boundary edge case of the cancel_reopen family and exposes
 *       a distinct bitmap path through DUNVIEW.C:3913-3928.
 *
 * Source evidence (ReDMCSB WIP 20210206):
 *   DUNGEON.C:2558,2608-2612  - C127 sensorData = ordinal stored in G0289
 *   DUNGEON.C:2573            - M011_CELL(sensor) selects visible wall cell
 *   MOVESENS.C:1501-1503      - C127 dispatches to F0280 with sensorData
 *   REVIVE.C F0280:124-132    - C040 empty-leader candidate gate
 *   REVIVE.C F0282:744-806    - C162 cancel branch 744-783
 *   REVIVE.C F0282:785-799    - C160 confirm disables matching C127 sensor
 *   COMMAND.C F0378:1956-1990 - M568_PANEL_RESURRECT_REINCARNATE dispatch
 *   PANEL.C F0355:2299-2318   - inventory close on cancel
 *   PANEL.C F0346,F0347:1619-1693 - C040 RR panel draw + dispatch while G0299
 *   DUNVIEW.C:3913-3928       - C346 wall frame, C026 portrait blit at D1C
 *   DUNVIEW.C:525             - G0109_Graphic558_Box_ChampionPortraitOnWall
 *   DUNVIEW.C:8318-8542 F0128 - far-to-near viewport draw order
 *   DUNVIEW.C G0205[coordSet=5][index=12] - D1C wall-ornament destination
 *   COORD.C:1693-1722         - PC 3.4 viewport origin / 224x136 dims
 *   DEFS.H:821-826            - M027_PORTRAIT_X / M028_PORTRAIT_Y macros
 *   m11_game_view.c:8303-8314 - candidate panel HandleInput guard
 *   m11_game_view.c:8371-8554 - world input dispatch (UP/DOWN/TURN/STRAFE)
 *   m11_game_view.c:7901-7944 - m11_disable_front_mirror_route (F0282)
 *
 * Run: firestaff_dm1_v1_hoc_champion_portrait_06_input_focus_restore_
 *      portrait_rect_position_174_gate_probe DATA_DIR
 *
 * SKIP path: probe exits 0 when M12_AssetStatus_GameAvailable("dm1")
 * returns 0 or when (1, 2) DIR_NORTH does not return ordinal 1 from
 * the C127 sensor lookup (a non-canonical DM1 V1 DUNGEON.DAT
 * fixture).  The probe seeds the (1, 1) NORTH C127 sensor from
 * shipped data=1 (HALK) to data=6 (SYRA) for this gate so we can
 * lock the ordinal-6 edge case without changing the map layout.
 *
 * Honesty scope:
 *   - Firestaff runtime portrait_rect_position evidence only.
 *   - Does not claim DOS pixel parity.
 *   - Requires real DM1 V1 PC 3.4 DUNGEON.DAT + the C026 portrait
 *     strip in GRAPHICS.DAT. Without that data the probe prints
 *     SKIP and exits 0.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "asset_status_m12.h"

#include <stdint.h>
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
    D1C_PORTRAIT_X = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    C040_PANEL_X = VIEWPORT_X + 80,
    C040_PANEL_Y = VIEWPORT_Y + 52,
    D1C_PORTRAIT_TOP_VISIBLE_H = C040_PANEL_Y - D1C_PORTRAIT_Y,
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    ORDINAL_6_COL = 6 & 7,
    ORDINAL_6_ROW = 6 >> 3,
    ORDINAL_6_SRC_X = ORDINAL_6_COL << 5,
    ORDINAL_6_SRC_Y = ORDINAL_6_ROW * 29,
    TARGET_ORDINAL = 6,
    /* The shipped DM1 V1 PC 3.4 DUNGEON.DAT places a C127 sensor
     * with sensorData=1 (HALK) on cell (1, 1) at the NORTH wall;
     * we seed that same sensor to data=6 (SYRA) to lock the
     * ordinal-6 edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1,
    HALL_MAP_INDEX = 0,
    HALL_6_POSE_X = 1,
    HALL_6_POSE_Y = 2,
    HALL_6_POSE_DIR = DIR_NORTH,
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_WARM_THRESHOLD = 30,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    PORTRAIT_BAND_H = PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0,
    TICK_DELTA_MIN = 1,
    TICK_DELTA_MAX = 32
};

static const char kExpectedCatalogName[] = "SYRA";

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static int rect_nonzero(const unsigned char* fb,
                        int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (idx != 0) ++cnt;
        }
    }
    return cnt;
}

static int rect_distinct(const unsigned char* fb,
                         int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (!seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            switch (idx) {
                case 0x07:
                case 0x08:
                case 0x09:
                case 0x0A:
                case 0x0B:
                case 0x0E:
                    ++cnt;
                    break;
                default:
                    break;
            }
        }
    }
    return cnt;
}

static int atlas_cell_opaque_count(const M11_AssetSlot* portraits,
                                   int ordinal) {
    int x, y, cnt = 0;
    int srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    int srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int sx = srcX + x;
            int sy = srcY + y;
            unsigned char src;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) {
                continue;
            }
            src = (unsigned char)
                (portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != 1) ++cnt;
        }
    }
    return cnt;
}

static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    int srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return 0;
    }
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int sx = srcX + x;
            int sy = srcY + y;
            unsigned char src;
            unsigned char dst;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) {
                continue;
            }
            src = (unsigned char)
                (portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src == 1) continue; /* transparent in the C026 atlas */
            dst = M11_FB_DECODE_INDEX(
                fb[(D1C_PORTRAIT_Y + y) * FB_W + (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

static int side_wall_warm_total(const unsigned char* fb,
                                int y, int h) {
    int left = rect_warm_count(fb,
                               SIDE_WALL_LEFT_X, y,
                               SIDE_WALL_LEFT_W, h);
    int right = rect_warm_count(fb,
                                SIDE_WALL_RIGHT_X, y,
                                SIDE_WALL_RIGHT_W, h);
    return left + right;
}

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index on
 * success, or -1 if no such sensor was found.  Same helper as the
 * cancel_reopen / leave_and_reenter / door_nearby ordinal-6 probes,
 * kept local so this probe is self contained. */
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

static void clear_party(M11_GameViewState* state) {
    if (!state) return;
    memset(state->world.party.champions, 0,
           sizeof(state->world.party.champions));
    state->world.party.championCount = 0;
    state->world.party.activeChampionIndex = -1;
}

static void park_h6_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = HALL_6_POSE_X;
    state->world.party.mapY = HALL_6_POSE_Y;
    state->world.party.direction = HALL_6_POSE_DIR;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

static void render_frame(M11_GameViewState* state,
                         unsigned char* fb) {
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(state, fb, FB_W, FB_H);
}

static int check_tick_delta(uint32_t before, uint32_t after) {
    uint32_t delta = after - before;
    return delta >= (uint32_t)TICK_DELTA_MIN &&
           delta <= (uint32_t)TICK_DELTA_MAX;
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbPanel[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    unsigned char fbAfterConfirm[FB_W * FB_H];
    int frontOrdinal;
    int ornX = 0, ornY = 0, ornW = 0, ornH = 0;
    int nameRc, titleRc;
    int atlasOpaque;
    int seededSensor;
    int matchBefore, matchPanel, matchAfterCancel, matchAfterConfirm;
    int nonzeroBefore, distinctBefore, warmBefore;
    int nonzeroAfterCancel, distinctAfterCancel, warmAfterCancel;
    int warmAfterConfirm;
    int sideBefore, sideAfterCancel, sideAfterConfirm;
    int initialCount, countAfterSelect, countAfterCancel, countAfterConfirm;
    int selectRc, backRc, actionRc;
    int turnWhilePanelRc, turnAfterCancelRc, turnAfterConfirmRc;
    int mapXBefore, mapYBefore, dirBefore;
    int mapXAfterCancel, mapYAfterCancel, dirAfterCancel;
    int mapXAfterConfirm, mapYAfterConfirm, dirAfterConfirm;
    uint32_t tickBefore, tickAfterCancel, tickAfterConfirm;
    char nameBuf[32];
    char titleBuf[64];

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("=== DM1 V1 HoC portrait-06 / input_focus_restore / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }

    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    portraits = M11_AssetLoader_Load(
        &state.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fprintf(stderr, "FAIL C026 champion portrait atlas unavailable\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    printf("\n[Group A] ordinal 6 atlas/catalog/front-route identity\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas is %ux%u (expected %dx%d)",
                 portraits->width, portraits->height, ATLAS_W, ATLAS_H);
        CHECK(portraits->width == ATLAS_W && portraits->height == ATLAS_H,
              msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 atlas source rect = (%d,%d,%d,%d)",
                 ORDINAL_6_SRC_X, ORDINAL_6_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        CHECK(ATLAS_COLS == 8 && ATLAS_ROWS == 3 &&
              ORDINAL_6_COL == 6 && ORDINAL_6_ROW == 0 &&
              ORDINAL_6_SRC_X == 192 && ORDINAL_6_SRC_Y == 0, msg);
    }
    atlasOpaque = atlas_cell_opaque_count(portraits, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 C026 cell has defined opaque pixels "
                 "(got %d, expected >=100)", atlasOpaque);
        CHECK(atlasOpaque >= 100, msg);
    }

    nameBuf[0] = '\0';
    titleBuf[0] = '\0';
    nameRc = M11_GameView_GetMirrorNameByOrdinal(
        &state, TARGET_ORDINAL, nameBuf, (int)sizeof(nameBuf));
    titleRc = M11_GameView_GetMirrorTitleByOrdinal(
        &state, TARGET_ORDINAL, titleBuf, (int)sizeof(titleBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 6 to %s",
                 nameBuf[0] ? nameBuf : "<empty>");
        CHECK(nameRc > 0 && strcmp(nameBuf, kExpectedCatalogName) == 0,
              msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog title for SYRA is non-empty (%s)",
                 titleBuf[0] ? titleBuf : "<empty>");
        CHECK(titleRc > 0 && titleBuf[0] != '\0', msg);
    }

    clear_party(&state);
    park_h6_front_route(&state);

    /* Sanity-check the shipped front-mirror ordinal (HALK / 1)
     * before we mutate the C127 sensor.  This is the canonical
     * "C127 sensor is alive at the right cell" gate. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front mirror at (1,2) NORTH reports ordinal %d "
                 "(expected %d, HALK before seed)",
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }
    if (frontOrdinal != SHIPPED_HALK_ORDINAL) {
        printf("SKIP non-canonical DM1 V1 data: (1,2) NORTH returned %d\n",
               frontOrdinal);
        M11_GameView_Shutdown(&state);
        return 0;
    }

    /* Seed the (1, 1) NORTH C127 sensor from data=1 (HALK) to
     * data=6 (SYRA).  Same sensor, same map cell, same draw path -
     * only the G0289 ordinal that DUNVIEW.C:3913-3928 reads
     * through M000_INDEX_TO_ORDINAL (DUNGEON.C:2610-2612) is
     * shifted for the test. */
    seededSensor = seed_first_c127_data(&state,
                                        SHIPPED_HALK_ORDINAL,
                                        TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,1) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    /* The same front route now reports ordinal 6.  After seeding
     * the C127 sensor's sensorData, the front route must reflect the
     * new ordinal. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded front-mirror ordinal = %d (expected %d)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    if (frontOrdinal != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify portrait_rect_position or input_focus_restore\n",
                TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    (void)M11_GameView_GetD1CWallOrnamentZone(
        &state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d,%d,%d,%d), expected "
                 "(80,29,64,43)", ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43,
              msg);
    }
    {
        char msg[220];
        snprintf(msg, sizeof(msg),
                 "portrait rect (96,35,32,29) sits inside the D1C zone");
        CHECK(96 >= ornX && 96 + D1C_PORTRAIT_W <= ornX + ornW &&
              35 >= ornY && 35 + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    printf("\n[Group B] baseline portrait_rect_position before candidate panel\n");
    render_frame(&state, fbBefore);
    matchBefore = match_portrait_at_rect(portraits, fbBefore,
                                         TARGET_ORDINAL);
    nonzeroBefore = rect_nonzero(fbBefore,
                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    distinctBefore = rect_distinct(fbBefore,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    warmBefore = rect_warm_count(fbBefore,
                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    sideBefore = side_wall_warm_total(fbBefore,
                                      PORTRAIT_BAND_Y0, PORTRAIT_BAND_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off D1C rect matches ordinal 6 at >=90%% "
                 "(got %d%%)", matchBefore);
        CHECK(matchBefore >= 90, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off D1C rect is non-empty/distinct/warm "
                 "(nonzero=%d distinct=%d warm=%d)",
                 nonzeroBefore, distinctBefore, warmBefore);
        CHECK(nonzeroBefore >= 100 && distinctBefore >= 4 &&
              warmBefore >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off side walls have < %d warm pixels total "
                 "(got %d)", PORTRAIT_WARM_THRESHOLD, sideBefore);
        CHECK(sideBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }

    printf("\n[Group C] panel focus suppresses world input while C040 is live\n");
    mapXBefore = state.world.party.mapX;
    mapYBefore = state.world.party.mapY;
    dirBefore = state.world.party.direction;
    initialCount = state.world.party.championCount;
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterSelect = state.world.party.championCount;
    {
        char msg[220];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate returns 1 and opens panel "
                 "(rc=%d panel=%d ordinal=%d partyIndex=%d count=%d)",
                 selectRc, state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterSelect);
        CHECK(selectRc == 1 &&
              state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == initialCount &&
              countAfterSelect == initialCount + 1, msg);
    }

    render_frame(&state, fbPanel);
    matchPanel = match_portrait_at_rect(portraits, fbPanel,
                                        TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw suppresses stale full SYRA sprite "
                 "(match=%d%% <=20%%)", matchPanel);
        CHECK(matchPanel <= 20, msg);
    }
    tickBefore = state.world.gameTick;
    turnWhilePanelRc = M11_GameView_HandleInput(
        &state, M12_MENU_INPUT_TURN_RIGHT);
    {
        char msg[220];
        snprintf(msg, sizeof(msg),
                 "HandleInput(TURN_RIGHT) while panel live returns IGNORED "
                 "(rc=%d) and pose/tick stay stable",
                 turnWhilePanelRc);
        CHECK(turnWhilePanelRc == (int)M11_GAME_INPUT_IGNORED &&
              state.world.party.mapX == mapXBefore &&
              state.world.party.mapY == mapYBefore &&
              state.world.party.direction == dirBefore &&
              state.world.gameTick == tickBefore &&
              state.candidateMirrorPanelActive == 1, msg);
    }

    printf("\n[Group D] BACK cancel restores game input focus and portrait rect\n");
    backRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    countAfterCancel = state.world.party.championCount;
    mapXAfterCancel = state.world.party.mapX;
    mapYAfterCancel = state.world.party.mapY;
    dirAfterCancel = state.world.party.direction;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "HandleInput(BACK) cancels candidate panel "
                 "(rc=%d panel=%d ordinal=%d partyIndex=%d count=%d)",
                 backRc, state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex, countAfterCancel);
        CHECK(backRc == (int)M11_GAME_INPUT_REDRAW &&
              state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterCancel == initialCount, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "cancel does not move the party "
                 "(pose=(%d,%d,%d), expected=(%d,%d,%d))",
                 mapXAfterCancel, mapYAfterCancel, dirAfterCancel,
                 mapXBefore, mapYBefore, dirBefore);
        CHECK(mapXAfterCancel == mapXBefore &&
              mapYAfterCancel == mapYBefore &&
              dirAfterCancel == dirBefore, msg);
    }

    render_frame(&state, fbAfterCancel);
    matchAfterCancel = match_portrait_at_rect(portraits, fbAfterCancel,
                                              TARGET_ORDINAL);
    nonzeroAfterCancel = rect_nonzero(fbAfterCancel,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    distinctAfterCancel = rect_distinct(fbAfterCancel,
                                        D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                        D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    warmAfterCancel = rect_warm_count(fbAfterCancel,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    sideAfterCancel = side_wall_warm_total(fbAfterCancel,
                                           PORTRAIT_BAND_Y0,
                                           PORTRAIT_BAND_H);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel D1C rect repaints SYRA at the same rect "
                 "(match=%d%% warm=%d nonzero=%d distinct=%d)",
                 matchAfterCancel, warmAfterCancel, nonzeroAfterCancel,
                 distinctAfterCancel);
        CHECK(matchAfterCancel >= 90 &&
              warmAfterCancel >= PORTRAIT_WARM_THRESHOLD &&
              nonzeroAfterCancel == nonzeroBefore &&
              distinctAfterCancel == distinctBefore, msg);
    }
    {
        char msg[220];
        snprintf(msg, sizeof(msg),
                 "after cancel side walls still have no floating SYRA "
                 "(warm total=%d)", sideAfterCancel);
        CHECK(sideAfterCancel < PORTRAIT_WARM_THRESHOLD, msg);
    }

    tickBefore = state.world.gameTick;
    turnAfterCancelRc = M11_GameView_HandleInput(
        &state, M12_MENU_INPUT_TURN_RIGHT);
    tickAfterCancel = state.world.gameTick;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel HandleInput(TURN_RIGHT) returns REDRAW, "
                 "direction turns NORTH->EAST, tick delta=%u",
                 (unsigned)(tickAfterCancel - tickBefore));
        CHECK(turnAfterCancelRc == (int)M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == mapXBefore &&
              state.world.party.mapY == mapYBefore &&
              state.world.party.direction == 1 &&
              check_tick_delta(tickBefore, tickAfterCancel), msg);
    }

    printf("\n[Group E] ACTION confirm restores game input focus and disables portrait route\n");
    park_h6_front_route(&state);
    clear_party(&state);
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "re-open panel for confirm arm succeeds (rc=%d panel=%d)",
                 selectRc, state.candidateMirrorPanelActive);
        CHECK(selectRc == 1 && state.candidateMirrorPanelActive == 1,
              msg);
    }
    actionRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACTION);
    countAfterConfirm = state.world.party.championCount;
    mapXAfterConfirm = state.world.party.mapX;
    mapYAfterConfirm = state.world.party.mapY;
    dirAfterConfirm = state.world.party.direction;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "HandleInput(ACTION) confirms candidate "
                 "(rc=%d panel=%d ordinal=%d partyIndex=%d count=%d)",
                 actionRc, state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex, countAfterConfirm);
        CHECK(actionRc == (int)M11_GAME_INPUT_REDRAW &&
              state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterConfirm == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "confirm does not move the party "
                 "(pose=(%d,%d,%d), expected=(%d,%d,%d))",
                 mapXAfterConfirm, mapYAfterConfirm, dirAfterConfirm,
                 mapXBefore, mapYBefore, dirBefore);
        CHECK(mapXAfterConfirm == mapXBefore &&
              mapYAfterConfirm == mapYBefore &&
              dirAfterConfirm == dirBefore, msg);
    }
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after confirm front-mirror route is disabled (got %d)",
                 frontOrdinal);
        CHECK(frontOrdinal == -1, msg);
    }
    render_frame(&state, fbAfterConfirm);
    matchAfterConfirm = match_portrait_at_rect(portraits, fbAfterConfirm,
                                               TARGET_ORDINAL);
    warmAfterConfirm = rect_warm_count(fbAfterConfirm,
                                       D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                       D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    sideAfterConfirm = side_wall_warm_total(fbAfterConfirm,
                                            PORTRAIT_BAND_Y0,
                                            PORTRAIT_BAND_H);
    {
        char msg[220];
        snprintf(msg, sizeof(msg),
                 "after confirm D1C rect is wall-only for ordinal 6 "
                 "(match=%d%% warm=%d)", matchAfterConfirm,
                 warmAfterConfirm);
        CHECK(matchAfterConfirm <= 5 &&
              warmAfterConfirm < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[220];
        snprintf(msg, sizeof(msg),
                 "after confirm side walls still have no floating SYRA "
                 "(warm total=%d)", sideAfterConfirm);
        CHECK(sideAfterConfirm < PORTRAIT_WARM_THRESHOLD, msg);
    }

    tickBefore = state.world.gameTick;
    turnAfterConfirmRc = M11_GameView_HandleInput(
        &state, M12_MENU_INPUT_TURN_RIGHT);
    tickAfterConfirm = state.world.gameTick;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after confirm HandleInput(TURN_RIGHT) returns REDRAW, "
                 "direction turns NORTH->EAST, tick delta=%u",
                 (unsigned)(tickAfterConfirm - tickBefore));
        CHECK(turnAfterConfirmRc == (int)M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == mapXBefore &&
              state.world.party.mapY == mapYBefore &&
              state.world.party.direction == 1 &&
              check_tick_delta(tickBefore, tickAfterConfirm), msg);
    }

    printf("\n[Group F] portrait_rect_position summary across focus restore\n");
    {
        char msg[260];
        snprintf(msg, sizeof(msg),
                 "rect contract: before=%d%% panel=%d%% after-cancel=%d%% "
                 "after-confirm=%d%% (closed >=90, panel <=20, "
                 "confirmed <=5)",
                 matchBefore, matchPanel, matchAfterCancel,
                 matchAfterConfirm);
        CHECK(matchBefore >= 90 && matchPanel <= 20 &&
              matchAfterCancel >= 90 && matchAfterConfirm <= 5, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
