/*
 * firestaff_dm1_v1_hoc_champion_portrait_22_input_focus_restore_portrait_rect_position_022_gate_probe.c
 *
 * Source-locked verification gate for one narrow DM1 V1 Hall of
 * Champions slice:
 *
 *   ordinal 22  (C026 col 6, row 2 -> source (192, 58, 32, 29) inside
 *                the 256x87 atlas. Catalog name "GOTHMOG", untitled.
 *                The real DM1 V1 DUNGEON.DAT carries a C127 sensor on
 *                Hall map 0 at cell (3, 6) on the WEST wall with
 *                sensorData=22, so the front_mirror route is reached
 *                by parking at (3, 6) facing WEST (dir=3).)
 *   route   input_focus_restore (C040 candidate panel open -> C162
 *                                cancel via HandleInput(BACK) OR
 *                                C160 resurrect confirm via
 *                                HandleInput(ACTION) -> C127 route
 *                                still active -> UP should now
 *                                produce a redraw and the gameplay
 *                                tick should advance, proving the
 *                                engine returned input focus to
 *                                the world after the modal panel
 *                                closed.)
 *   aspect  portrait_rect_position (D1C cutout at viewport (96, 35)
 *                                   sized 32x29 inside the C346
 *                                   wall-mirror frame (80, 29, 64, 43),
 *                                   source-locked to DUNVIEW.C:3913-3928
 *                                   and DUNVIEW.C:525 G0109_Graphic558
 *                                   _Box_ChampionPortraitOnWall.)
 *
 * The slice is intentionally disjoint from the existing portrait-22
 * gates:
 *
 *   * firestaff_dm1_v1_hall_champion_portrait_22_front_north_entry_
 *     runtime_probe.c
 *     - entry-pose + atlas math + C127 sensor rewrite proof.
 *       Does NOT drive the candidate panel or any input focus cycle.
 *
 *   * firestaff_dm1_v1_hall_of_champions_portrait_22_redraw_after_
 *     candidate_runtime_probe.c
 *     - framebuffer state with the C040 panel live (BUG-120/121).
 *       Does NOT exercise HandleInput across the panel open/close
 *       transition, so it cannot prove the input focus was actually
 *       returned to the world after the modal panel closed.
 *
 *   * firestaff_dm1_v1_hall_of_champions_portrait_XX_cancel_reopen_*
 *     - select/cancel/select with the public CancelMirrorCandidate /
 *       SelectFrontMirrorCandidate helpers, but the public helpers
 *       do not route through HandleInput, so a "BACK fires cancel"
 *       keyboard regression in the input dispatcher would be
 *       invisible to those probes. The input_focus_restore route
 *       covered here goes through HandleInput, which is the only
 *       path that exercises M11_GAME_INPUT_IGNORED vs REDRAW for
 *       the candidate panel guard at m11_game_view.c:8303-8314 and
 *       the world re-entry path at m11_game_view.c:8371-8554.
 *
 * Source evidence (ReDMCSB WIP 20210206):
 *   DUNGEON.C:2558,2608-2612  - C127 sensorData = ordinal stored in G0289
 *   DUNGEON.C:2573            - M011_CELL(sensor) selects visible wall cell
 *   MOVESENS.C:1501-1503      - C127 dispatches to F0280 with sensorData
 *   REVIVE.C F0280:124-132    - C040 empty-leader candidate gate
 *   REVIVE.C F0282:744-806    - C162 cancel branch 744-783
 *   COMMAND.C F0378:1956-1990 - M568_PANEL_RESURRECT_REINCARNATE dispatch
 *   PANEL.C F0355:2299-2318   - inventory close on cancel
 *   DUNVIEW.C:3913-3928       - C346 wall frame, C026 portrait blit at D1C
 *   DUNVIEW.C:525             - G0109_Graphic558_Box_ChampionPortraitOnWall
 *   DUNVIEW.C:8318-8542 F0128 - far-to-near viewport draw order
 *   COORD.C:1693-1722         - PC 3.4 viewport origin / 224x136 dims
 *   DEFS.H:821-826            - M027_PORTRAIT_X / M028_PORTRAIT_Y macros
 *   m11_game_view.c:8303-8314 - candidate panel HandleInput guard
 *   m11_game_view.c:8371-8554 - world input dispatch (UP/DOWN/TURN/STRAFE)
 *
 * This probe deliberately does NOT claim DOS pixel parity. Original
 * DM1 PC 3.4 captures live under parity-evidence/ and are referenced
 * by separate parity gates. The pixel proof here is a "portrait
 * pixels are present in the C026 cutout and never leak into the
 * side walls" check, which is independent of the original VGA palette
 * or capture timing.
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
    /* Source-locked D1C portrait rectangle (DUNVIEW.C:3913-3928). */
    D1C_PORTRAIT_X = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* C040 candidate panel rectangle (DUNVIEW.C:1341 G0109_Graphic558
     *       _Box_ChampionPortraitOnWall and the C040 modal panel
     *       DUNVIEW.C:13018-13045). */
    C040_PANEL_X = VIEWPORT_X + 80,
    C040_PANEL_Y = VIEWPORT_Y + 52,
    D1C_PORTRAIT_TOP_VISIBLE_H = C040_PANEL_Y - D1C_PORTRAIT_Y,
    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    /* Ordinal 22 in the C026 atlas: (22 & 7) * 32 = 192,
     *                                 (22 >> 3) * 29 =  58.
     * This is the BOTTOM row of the 8x3 atlas (row 2). */
    ORDINAL_22_COL = 22 & 7,        /* = 6 */
    ORDINAL_22_ROW = 22 >> 3,       /* = 2 */
    ORDINAL_22_SRC_X = ORDINAL_22_COL * 32,   /* = 192 */
    ORDINAL_22_SRC_Y = ORDINAL_22_ROW * 29,   /* = 58 */
    TARGET_ORDINAL = 22,
    /* The real DM1 V1 DUNGEON.DAT C127 sensor for ordinal 22 sits on
     * cell (3, 6) at the WEST wall: when the party is at (3, 6) facing
     * WEST (dir=3), the front cell is (2, 6) and the C127 sensor on
     * the west wall of (2, 6) reports sensorData=22 (GOTHMOG).  This
     * matches the discovery output of the existing portrait-22
     * front_north_entry probe (HIT: ordinal 22 at pose=(map=0, x=3,
     * y=6, dir=3)). */
    HALL_MAP_INDEX = 0,
    HALL_22_POSE_X = 3,
    HALL_22_POSE_Y = 6,
    HALL_22_POSE_DIR = 3, /* DIR_WEST per DEFS.H:1768 (#define DIR_WEST 3) */
    /* Side wall sample zones - the no-floating proof checks that
     * the portrait sprite pixels do not bleed into the left/right
     * side walls of the D1C cell band. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_WARM_THRESHOLD = 30,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    PORTRAIT_BAND_H = PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0,
    /* Sample the engine's first-movement tick as a stable evidence
     * anchor: HandleInput(UP) in a non-blocked cell advances the tick
     * by exactly 1 (m11_game_view.c:8421-8448).  If the engine ticks
     * the world extra times between input dispatches (e.g. via the
     * idle-tick branch on M11_GameView_AdvanceIdleTick), this delta
     * will be greater than 1 and the slice will still pass as long
     * as the tick actually advances. */
    TICK_DELTA_MIN = 1,
    TICK_DELTA_MAX = 16
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count non-zero pixels in a framebuffer rectangle. */
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

/* Count distinct palette indices in a framebuffer rectangle. */
static int rect_distinct(const unsigned char* fb,
                         int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (!seen[idx]) { seen[idx] = 1; ++n; }
        }
    }
    return n;
}

/* Count "warm" pixels in a framebuffer rectangle.  The C026 portrait
 * sprites use the warm palette set {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E}
 * (green / red / orange / peach / yellow / blue) for skin tones,
 * clothing, and backgrounds.  Grey-stone wall texture uses indices
 * 0x01, 0x02, 0x0D.  Counting warm pixels is a coarse but reliable
 * way to distinguish "portrait is here" from "wall only" in the
 * C026 cutout (96, 35, 32, 29). */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++cnt;
                    break;
                default:
                    break;
            }
        }
    }
    return cnt;
}

/* Count opaque pixels in the C026 atlas cell for the requested ordinal.
 * Used to verify ordinal 22 is a defined portrait in the atlas
 * (i.e. not blank / unused). */
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
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != 1) ++cnt;
        }
    }
    return cnt;
}

/* Compare the C026 portrait atlas cell for the requested ordinal
 * to the framebuffer D1C portrait rectangle.  Returns the percent
 * of opaque source pixels that match the destination pixel. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char src;
            unsigned char dst;
            int sx = srcX + x;
            int sy = srcY + y;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src == 1) continue; /* transparent */
            dst = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W + (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Find the first C127 sensor on the canonical (3, 6) WEST wall cell
 * and rewrite its sensorData from oldData to newData.  Returns the
 * sensor index on success, or -1 if no such sensor was found.  The
 * real DM1 V1 DUNGEON.DAT already carries sensorData=22 on this
 * cell, so seeding the (3, 6) WEST C127 sensor is a no-op when
 * oldData == 22 (the SHIPPED_ORDINAL for this gate).  We use this
 * helper for symmetry with the other portrait-22 gates that seed
 * the (1, 2) NORTH C127 sensor.  It also makes the probe robust
 * against a non-canonical DM1 V1 DUNGEON.DAT that re-sorts the
 * mirror catalog. */
static int seed_h22_c127_data(M11_GameViewState* state,
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

/* Park the party at the (3, 6) D1C front-mirror route facing WEST.
 * The (3, 6) WEST cell is where the real DM1 V1 DUNGEON.DAT places
 * the ordinal-22 (GOTHMOG) C127 sensor, per the existing
 * portrait-22 front_north_entry probe's any-pose discovery output:
 *     HIT: ordinal 22 at pose=(map=0, x=3, y=6, dir=3) */
static void park_h22_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = HALL_MAP_INDEX;
    state->world.party.mapX = HALL_22_POSE_X;
    state->world.party.mapY = HALL_22_POSE_Y;
    state->world.party.direction = HALL_22_POSE_DIR;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

/* Get the warm-pixel total across the left/right side walls of the
 * D1C portrait band.  The y window is the visible band when the C040
 * panel is closed (full band) or only the unoccluded top strip when
 * the panel is live. */
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

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int seededSensor;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbPanelOn[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    unsigned char fbAfterConfirm[FB_W * FB_H];
    int matchBefore, matchAfterCancel, matchAfterConfirm;
    int nonzeroBefore, nonzeroAfterCancel, nonzeroAfterConfirm;
    int distinctBefore, distinctAfterCancel;
    int warmBefore, warmAfterCancel, warmAfterConfirm;
    int sideWallBefore, sideWallAfterCancel, sideWallAfterConfirm;
    int initialCount, countAfterSelect, countAfterCancel, countAfterConfirm;
    int selectRc, cancelRc, confirmRc;
    int backHandleRc, actionHandleRc;
    int upHandleAfterCancel, upHandlePanel, upHandleAfterConfirm;
    /* The real ordinal-22 C127 sensor is at index 25 (per
     * Group A) on the (3, 6) WEST wall; we save its
     * original sensorType/sensorData here so the Group E
     * confirm arm's m11_disable_front_mirror_route
     * (sensorType <- 0) can be reverted at the end. */
    int h22SensorIndex = -1;
    unsigned short h22SavedType = 0;
    unsigned short h22SavedData = 0;
    int turnLeftHandlePanel, strafeLeftHandlePanel;
    int mapXBefore, mapYBefore, dirBefore;
    int mapXAfterCancel, mapYAfterCancel, dirAfterCancel;
    int mapXAfterConfirm, mapYAfterConfirm, dirAfterConfirm;
    uint32_t tickBefore, tickPanel, tickAfterCancel, tickAfterConfirm;
    uint32_t tickDeltaCancel, tickDeltaConfirm;
    int ordinal22Opaque;
    int ordinal22OpaqueRow2Min;
    char nameBuf[32];
    int nameLookupRc;
    static const char kExpectedCatalogName[] = "GOTHMOG";

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 HoC portrait-22 / input_focus_restore / portrait_rect_position (v2.7.27) ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    /* Load the C026 portrait atlas via the public M11 helper. */
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* ----------------------------------------------------------------
     * Group A - Atlas math for ordinal 22
     * ----------------------------------------------------------------
     * The C026 atlas is "256x87 strip of 32x29 portraits: 8 columns by
     * 3 rows" (DUNVIEW.C:3916-3919).  Ordinal 22 sits at row 2 /
     * column 6 of the 8x3 atlas; the row-2 source cell bottom
     * (srcY=58 + 29 = 87) exactly equals the atlas height, so this
     * also implicitly proves the (ordinal >> 3) * 29 source math
     * doesn't run off the bottom of the strip. */
    printf("\n[Group A] C026 atlas math for ordinal 22 (row 2 / col 6 of 8x3 atlas)\n");

    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic id = %d)",
                 M11_GameView_GetV1ChampionPortraitGraphicId());
        CHECK(portraits != NULL && portraits->loaded && portraits->pixels != NULL, msg);
    }
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == 256, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == 87, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 col = 22 & 7 = %d (expected 6)",
                 ORDINAL_22_COL);
        CHECK(ORDINAL_22_COL == 6, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 row = 22 >> 3 = %d (expected 2)",
                 ORDINAL_22_ROW);
        CHECK(ORDINAL_22_ROW == 2, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_22_SRC_X, ORDINAL_22_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_22_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_22_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 source cell bottom exactly reaches "
                 "atlas height: srcY(%d) + portraitH(%d) == atlasH(%u)",
                 ORDINAL_22_SRC_Y, D1C_PORTRAIT_H, portraits->height);
        CHECK(ORDINAL_22_SRC_Y + D1C_PORTRAIT_H == (int)portraits->height, msg);
    }

    ordinal22Opaque = atlas_cell_opaque_count(portraits, 22);
    /* The full row 2 (ordinals 16..23) must all be defined
     * portraits; we only assert a minimum opaque count on ordinal
     * 22 to prove it is a defined slot, not a degenerate cell. */
    ordinal22OpaqueRow2Min = 100;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 22 atlas cell has >= %d opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal22OpaqueRow2Min, ordinal22Opaque);
        CHECK(ordinal22Opaque >= ordinal22OpaqueRow2Min, msg);
    }

    /* Ordinal 22 must resolve to GOTHMOG through the mirror catalog.
     * This catches a regression where the catalog and the C026 atlas
     * disagree on the ordinal-22 record. */
    nameBuf[0] = '\0';
    nameLookupRc = M11_GameView_GetMirrorNameByOrdinal(&state,
                                                       TARGET_ORDINAL,
                                                       nameBuf,
                                                       (int)sizeof(nameBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 22 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }

    /* Seed the (3, 6) WEST C127 sensor to ordinal 22.  Real DM1 V1
     * DUNGEON.DAT ships sensorData=22 on this cell (per the existing
     * portrait-22 front_north_entry any-pose discovery), so the
     * seed is a no-op for the shipped data but keeps the probe
     * robust against a non-canonical variant.  Save the original
     * sensorType/sensorData here so the Group E confirm arm's
     * m11_disable_front_mirror_route (sensorType <- 0) can be
     * reverted at the end. */
    park_h22_front_route(&state);
    seededSensor = seed_h22_c127_data(&state, TARGET_ORDINAL, TARGET_ORDINAL);
    h22SensorIndex = seededSensor;
    if (h22SensorIndex >= 0 && state.world.things &&
        state.world.things->sensors) {
        h22SavedType = state.world.things->sensors[h22SensorIndex].sensorType;
        h22SavedData = state.world.things->sensors[h22SensorIndex].sensorData;
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "(3,6) WEST C127 sensor at index %d carries ordinal %d "
                 "(GOTHMOG; no-op seed for shipped data) - savedType=%u, "
                 "savedData=%u",
                 seededSensor, TARGET_ORDINAL,
                 (unsigned)h22SavedType, (unsigned)h22SavedData);
        CHECK(seededSensor >= 0, msg);
    }

    /* The (3, 6) WEST-route front mirror must report ordinal 22. */
    park_h22_front_route(&state);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (3,6) facing WEST = %d (expected %d)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    if (frontOrdinal != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify input_focus_restore or portrait_rect_position\n",
                TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Sanity-check the public D1C wall ornament zone helper, then
     * verify the inner portrait rectangle (96, 35, 32, 29) sits
     * inside that zone. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d, %d, %d, %d) viewport "
                 "coords (DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35, 32, 29) sits inside the "
                 "D1C wall ornament zone (X within [%d,%d), Y within "
                 "[%d,%d))",
                 ornX, ornX + ornW, ornY, ornY + ornH);
        CHECK(96 >= ornX &&
              96 + D1C_PORTRAIT_W <= ornX + ornW &&
              35 >= ornY &&
              35 + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - portrait_rect_position baseline (panel closed)
     * ----------------------------------------------------------------
     * Render the framebuffer with the C040 panel closed.  The D1C
     * destination rectangle (96, 35, 32, 29) must hold ordinal-22
     * pixels.  The no-floating proof checks the left/right side
     * walls of the D1C portrait band carry no warm pixels. */
    printf("\n[Group B] portrait_rect_position baseline on (3,6) facing WEST=22\n");

    park_h22_front_route(&state);
    initialCount = state.world.party.championCount;
    mapXBefore = state.world.party.mapX;
    mapYBefore = state.world.party.mapY;
    dirBefore = state.world.party.direction;
    tickBefore = state.world.gameTick;

    memset(fbBefore, 0, sizeof(fbBefore));
    M11_GameView_Draw(&state, fbBefore, FB_W, FB_H);

    matchBefore = match_portrait_at_rect(portraits, fbBefore, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) carries ordinal %d pixels "
                 "at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchBefore);
        CHECK(matchBefore >= 90, msg);
    }
    nonzeroBefore = rect_nonzero(fbBefore,
                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect is non-empty (>= 100 non-zero "
                 "pixels, got %d)",
                 nonzeroBefore);
        CHECK(nonzeroBefore >= 100, msg);
    }
    distinctBefore = rect_distinct(fbBefore,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= 4 distinct palette indices "
                 "(got %d)",
                 distinctBefore);
        CHECK(distinctBefore >= 4, msg);
    }
    warmBefore = rect_warm_count(fbBefore,
                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= %d warm-color pixels "
                 "(got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmBefore);
        CHECK(warmBefore >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    sideWallBefore = side_wall_warm_total(fbBefore,
                                          PORTRAIT_BAND_Y0, PORTRAIT_BAND_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "side walls of D1C portrait band have < %d warm "
                 "pixels total (got %d) - portrait not floating on "
                 "side walls (baseline)",
                 PORTRAIT_WARM_THRESHOLD, sideWallBefore);
        CHECK(sideWallBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - input focus suppression while panel is live
     * ----------------------------------------------------------------
     * With the C040 panel live, gameplay input tokens (UP, DOWN,
     * TURN_LEFT, STRAFE_LEFT) must all return M11_GAME_INPUT_IGNORED
     * from the input dispatcher.  Only BACK (-> cancel) and
     * ACCEPT/ACTION (-> confirm) are valid while the panel owns the
     * view.  This is the focus-suppression half of the
     * input_focus_restore contract. */
    printf("\n[Group C] input focus suppression while C040 panel is live\n");

    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterSelect = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (3,6) facing WEST returns 1 (got %d)",
                 selectRc);
        CHECK(selectRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after select: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterSelect, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              countAfterSelect == initialCount + 1, msg);
    }

    tickPanel = state.world.gameTick;
    upHandlePanel = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(UP) while panel live returns IGNORED (got %d)",
                 upHandlePanel);
        CHECK(upHandlePanel == M11_GAME_INPUT_IGNORED, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(UP) while panel live does not advance "
                 "gameTick (before=%u, after=%u)",
                 (unsigned)tickPanel, (unsigned)state.world.gameTick);
        CHECK(state.world.gameTick == tickPanel, msg);
    }
    turnLeftHandlePanel = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_LEFT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(TURN_LEFT) while panel live returns IGNORED (got %d)",
                 turnLeftHandlePanel);
        CHECK(turnLeftHandlePanel == M11_GAME_INPUT_IGNORED, msg);
    }
    strafeLeftHandlePanel = M11_GameView_HandleInput(&state, M12_MENU_INPUT_STRAFE_LEFT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(STRAFE_LEFT) while panel live returns IGNORED (got %d)",
                 strafeLeftHandlePanel);
        CHECK(strafeLeftHandlePanel == M11_GAME_INPUT_IGNORED, msg);
    }
    /* The party pose must be unchanged: panel focus suppression
     * means a stray UP/TURN/STRAFE cannot move or rotate the party. */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(UP/TURN_LEFT/STRAFE_LEFT) does not change "
                 "party pose: (x,y,dir) = (%d,%d,%d) (was %d,%d,%d)",
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction,
                 mapXBefore, mapYBefore, dirBefore);
        CHECK(state.world.party.mapX == mapXBefore &&
              state.world.party.mapY == mapYBefore &&
              state.world.party.direction == dirBefore, msg);
    }

    /* Render with C040 panel live.  The portrait must NOT be drawn as
     * a stale floating sprite while the panel owns the view (BUG-120/
     * 121 panel guard).  Match against ordinal 22 should be low. */
    memset(fbPanelOn, 0, sizeof(fbPanelOn));
    M11_GameView_Draw(&state, fbPanelOn, FB_W, FB_H);
    {
        int matchPanel = match_portrait_at_rect(portraits, fbPanelOn, TARGET_ORDINAL);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw does not leave ordinal %d as a stale "
                 "full-D1C sprite (<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchPanel);
        CHECK(matchPanel <= 20, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - input focus restored after cancel (BACK)
     * ----------------------------------------------------------------
     * Send BACK through HandleInput (REVIVE.C F0282 C162 cancel
     * branch) and verify the input dispatcher returns REDRAW for
     * the cancel dispatch, the candidate state is cleared, the
     * party size returns to baseline, the panel state is closed,
     * and a follow-up UP returns REDRAW with the world tick
     * advancing.  This is the cancel arm of input_focus_restore. */
    printf("\n[Group D] input focus restored after cancel (BACK) at (3,6) facing WEST\n");

    tickBefore = state.world.gameTick;
    backHandleRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    cancelRc = (backHandleRc != 0) ? 1 : 0;
    countAfterCancel = state.world.party.championCount;
    mapXAfterCancel = state.world.party.mapX;
    mapYAfterCancel = state.world.party.mapY;
    dirAfterCancel = state.world.party.direction;
    tickAfterCancel = state.world.gameTick;
    tickDeltaCancel = (tickAfterCancel > tickBefore)
                          ? (tickAfterCancel - tickBefore)
                          : 0u;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(BACK) while panel live returns REDRAW (got %d)",
                 backHandleRc);
        CHECK(backHandleRc == M11_GAME_INPUT_REDRAW, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(BACK) clears candidate state "
                 "(panelActive=%d, ord=%d, partyIdx=%d, count=%d, was %d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterCancel, initialCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterCancel == initialCount &&
              cancelRc == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(BACK) does not move the party on cancel: "
                 "(x,y,dir) = (%d,%d,%d) (was %d,%d,%d)",
                 mapXAfterCancel, mapYAfterCancel, dirAfterCancel,
                 mapXBefore, mapYBefore, dirBefore);
        CHECK(mapXAfterCancel == mapXBefore &&
              mapYAfterCancel == mapYBefore &&
              dirAfterCancel == dirBefore, msg);
    }

    /* Render after cancel: panel closed, portrait rect must hold
     * ordinal-22 pixels again. */
    memset(fbAfterCancel, 0, sizeof(fbAfterCancel));
    M11_GameView_Draw(&state, fbAfterCancel, FB_W, FB_H);
    matchAfterCancel = match_portrait_at_rect(portraits, fbAfterCancel,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchAfterCancel);
        CHECK(matchAfterCancel >= 90, msg);
    }
    nonzeroAfterCancel = rect_nonzero(fbAfterCancel,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 nonzeroAfterCancel);
        CHECK(nonzeroAfterCancel >= 100, msg);
    }
    distinctAfterCancel = rect_distinct(fbAfterCancel,
                                        D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                        D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect has >= 4 distinct "
                 "palette indices (got %d, baseline was %d)",
                 distinctAfterCancel, distinctBefore);
        CHECK(distinctAfterCancel >= 4, msg);
    }
    warmAfterCancel = rect_warm_count(fbAfterCancel,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect has >= %d warm-color "
                 "pixels (got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmAfterCancel);
        CHECK(warmAfterCancel >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    sideWallAfterCancel = side_wall_warm_total(fbAfterCancel,
                                               PORTRAIT_BAND_Y0, PORTRAIT_BAND_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: side walls of D1C portrait band have "
                 "< %d warm pixels total (got %d) - portrait not "
                 "floating on side walls (cancel restore)",
                 PORTRAIT_WARM_THRESHOLD, sideWallAfterCancel);
        CHECK(sideWallAfterCancel < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Send UP through HandleInput now that the panel is closed.
     * This is the input_focus_restore contract: gameplay input
     * that was IGNORED while the panel was live is now REDRAW,
     * and the world tick advances.  We snapshot the world tick
     * before UP, then after UP, and require the delta to fall in
     * [TICK_DELTA_MIN, TICK_DELTA_MAX] so the engine's idle-tick
     * branch is allowed to tick the world extra times. */
    tickBefore = state.world.gameTick;
    upHandleAfterCancel = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    tickAfterCancel = state.world.gameTick;
    tickDeltaCancel = (tickAfterCancel > tickBefore)
                          ? (tickAfterCancel - tickBefore)
                          : 0u;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: HandleInput(UP) returns REDRAW (got %d) "
                 "and world tick advances: tick %u -> %u (delta=%u, "
                 "expected %u..%u)",
                 upHandleAfterCancel,
                 (unsigned)tickBefore, (unsigned)tickAfterCancel,
                 (unsigned)tickDeltaCancel,
                 (unsigned)TICK_DELTA_MIN, (unsigned)TICK_DELTA_MAX);
        CHECK(upHandleAfterCancel == M11_GAME_INPUT_REDRAW &&
              tickDeltaCancel >= TICK_DELTA_MIN &&
              tickDeltaCancel <= TICK_DELTA_MAX, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - input focus restored after confirm (ACTION)
     * ----------------------------------------------------------------
     * Re-open the C040 panel and send ACTION through HandleInput
     * (REVIVE.C F0282 C160 resurrect path).  Verify the input
     * dispatcher returns REDRAW for the confirm dispatch, the
     * candidate champion stays in the party, the C127 sensor is
     * disabled (sensor disable, not candidate re-entry), and a
     * follow-up UP returns REDRAW with the world tick advancing.
     * This is the confirm arm of input_focus_restore. */
    printf("\n[Group E] input focus restored after confirm (ACTION) at (3,6) facing WEST\n");

    park_h22_front_route(&state);
    /* C127 sensor for ordinal 22 was re-enabled by the previous
     * cancel/select path? No: cancel does not re-enable the sensor
     * (REVIVE.C F0282:744-806 C162 cancel branch only handles
     * inventory + candidate slot + panel state, leaving the
     * sensorType=127 sensorData=22 sensor on the (3, 6) WEST wall
     * untouched).  So a re-select is a fresh F0280 materialize. */
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "re-SelectFrontMirrorCandidate on (3,6) facing WEST returns 1 (got %d)",
                 selectRc);
        CHECK(selectRc == 1, msg);
    }
    if (selectRc != 1) {
        fprintf(stderr,
                "FATAL: re-SelectFrontMirrorCandidate failed; cannot "
                "verify input focus restore on confirm arm\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    tickBefore = state.world.gameTick;
    actionHandleRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACTION);
    confirmRc = (actionHandleRc != 0) ? 1 : 0;
    countAfterConfirm = state.world.party.championCount;
    mapXAfterConfirm = state.world.party.mapX;
    mapYAfterConfirm = state.world.party.mapY;
    dirAfterConfirm = state.world.party.direction;
    tickAfterConfirm = state.world.gameTick;
    tickDeltaConfirm = (tickAfterConfirm > tickBefore)
                            ? (tickAfterConfirm - tickBefore)
                            : 0u;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(ACTION) while panel live returns REDRAW (got %d)",
                 actionHandleRc);
        CHECK(actionHandleRc == M11_GAME_INPUT_REDRAW, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(ACTION) confirms candidate: "
                 "panelActive=%d, ord=%d, partyIdx=%d, count=%d, was %d",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterConfirm, initialCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterConfirm == initialCount + 1 &&
              confirmRc == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(ACTION) does not move the party on confirm: "
                 "(x,y,dir) = (%d,%d,%d) (was %d,%d,%d)",
                 mapXAfterConfirm, mapYAfterConfirm, dirAfterConfirm,
                 mapXBefore, mapYBefore, dirBefore);
        CHECK(mapXAfterConfirm == mapXBefore &&
              mapYAfterConfirm == mapYBefore &&
              dirAfterConfirm == dirBefore, msg);
    }

    /* The front-mirror route is no longer ordinal 22 after confirm
     * (the C127 sensor is disabled by m11_disable_front_mirror_route,
     * so the next GetFrontMirrorOrdinal must return -1).  The D1C
     * portrait rect should hold no portrait pixels (no candidate on
     * this cell), but the C346 wall-mirror frame is still drawn as
     * background, so the nonzero count is the wall texture and the
     * warm-pixel count is the meaningful "portrait is gone" signal. */
    memset(fbAfterConfirm, 0, sizeof(fbAfterConfirm));
    M11_GameView_Draw(&state, fbAfterConfirm, FB_W, FB_H);
    matchAfterConfirm = match_portrait_at_rect(portraits, fbAfterConfirm,
                                               TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after confirm: D1C portrait rect does not carry "
                 "ordinal %d (route disabled; <= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterConfirm);
        CHECK(matchAfterConfirm <= 20, msg);
    }
    nonzeroAfterConfirm = rect_nonzero(fbAfterConfirm,
                                       D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                       D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    /* The D1C wall-mirror frame (C346) is still drawn as background
     * even when no champion is selected, so nonzero count is NOT a
     * good "no portrait" signal.  The right signal is: warm pixels
     * (skin / clothing / background) and palette match against the
     * ordinal 22 atlas cell both fall to baseline.  The nonzero count
     * is logged for diagnostics only. */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after confirm: D1C portrait rect nonzero=%d (C346 wall "
                 "frame background; no portrait)",
                 nonzeroAfterConfirm);
        CHECK(nonzeroAfterConfirm >= 0, msg);
    }
    warmAfterConfirm = rect_warm_count(fbAfterConfirm,
                                       D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                       D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after confirm: D1C portrait rect has < %d warm "
                 "pixels (got %d) - portrait cleared on confirm",
                 PORTRAIT_WARM_THRESHOLD, warmAfterConfirm);
        CHECK(warmAfterConfirm < PORTRAIT_WARM_THRESHOLD, msg);
    }
    sideWallAfterConfirm = side_wall_warm_total(fbAfterConfirm,
                                                PORTRAIT_BAND_Y0, PORTRAIT_BAND_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after confirm: side walls of D1C portrait band have "
                 "< %d warm pixels total (got %d) - no portrait, no "
                 "float",
                 PORTRAIT_WARM_THRESHOLD, sideWallAfterConfirm);
        CHECK(sideWallAfterConfirm < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Send UP through HandleInput now that the panel is closed.
     * This is the confirm arm of input_focus_restore: gameplay
     * input that was IGNORED while the panel was live is now
     * REDRAW, and the world tick advances. */
    tickBefore = state.world.gameTick;
    upHandleAfterConfirm = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    tickAfterConfirm = state.world.gameTick;
    tickDeltaConfirm = (tickAfterConfirm > tickBefore)
                            ? (tickAfterConfirm - tickBefore)
                            : 0u;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after confirm: HandleInput(UP) returns REDRAW (got %d) "
                 "and world tick advances: tick %u -> %u (delta=%u, "
                 "expected %u..%u)",
                 upHandleAfterConfirm,
                 (unsigned)tickBefore, (unsigned)tickAfterConfirm,
                 (unsigned)tickDeltaConfirm,
                 (unsigned)TICK_DELTA_MIN, (unsigned)TICK_DELTA_MAX);
        CHECK(upHandleAfterConfirm == M11_GAME_INPUT_REDRAW &&
              tickDeltaConfirm >= TICK_DELTA_MIN &&
              tickDeltaConfirm <= TICK_DELTA_MAX, msg);
    }

    /* ----------------------------------------------------------------
     * Group F - portrait_rect_position is stable across the focus
     * cycle.  The D1C destination rectangle (96, 35, 32, 29) is
     * source-locked; the rectangle must remain at the same screen
     * position regardless of whether the C040 panel is open, the
     * candidate is selected, the candidate is cancelled, or the
     * candidate is confirmed.  We verify the four framebuffer
     * captures all line up at the same destination rect, and that
     * the panel-off baseline (no panel live) and post-cancel
     * captures carry ordinal-22 pixels.  The post-confirm capture
     * carries no portrait because the C127 sensor is disabled on
     * confirm (m11_disable_front_mirror_route, REVIVE.C F0282:744-806
     * C162 cancel branch -> F0282:1358 disable, also F0282:1140
     * confirm); this is the source-locked expected behaviour. */
    printf("\n[Group F] portrait_rect_position contract: rect stable across focus cycle\n");
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect match profile (panel-off=%d%%, panel-on=%d%%, "
                 "after-cancel=%d%%, after-confirm=%d%%) - panel-off and "
                 "after-cancel must hold ordinal %d; panel-on and "
                 "after-confirm must NOT (BUG-120/121 panel guard)",
                 matchBefore,
                 match_portrait_at_rect(portraits, fbPanelOn, TARGET_ORDINAL),
                 matchAfterCancel, matchAfterConfirm, TARGET_ORDINAL);
        CHECK(matchBefore >= 90 &&
              matchAfterCancel >= 90 &&
              match_portrait_at_rect(portraits, fbPanelOn, TARGET_ORDINAL) <= 20 &&
              matchAfterConfirm <= 20, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect non-zero count profile (baseline=%d, "
                 "after-cancel=%d, after-confirm=%d) - all must be >= 100 "
                 "(C346 wall frame background is always drawn)",
                 nonzeroBefore, nonzeroAfterCancel, nonzeroAfterConfirm);
        CHECK(nonzeroBefore >= 100 &&
              nonzeroAfterCancel >= 100 &&
              nonzeroAfterConfirm >= 100, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C rect warm-pixel count profile (baseline=%d, "
                 "after-cancel=%d, after-confirm=%d) - baseline and "
                 "after-cancel must be >= %d (portrait sprite); "
                 "after-confirm must be < %d (no portrait)",
                 warmBefore, warmAfterCancel, warmAfterConfirm,
                 PORTRAIT_WARM_THRESHOLD, PORTRAIT_WARM_THRESHOLD);
        CHECK(warmBefore >= PORTRAIT_WARM_THRESHOLD &&
              warmAfterCancel >= PORTRAIT_WARM_THRESHOLD &&
              warmAfterConfirm < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "side-wall warm-pixel count profile (baseline=%d, "
                 "after-cancel=%d, after-confirm=%d) - all must be "
                 "< %d (no-floating proof across the focus cycle)",
                 sideWallBefore, sideWallAfterCancel, sideWallAfterConfirm,
                 PORTRAIT_WARM_THRESHOLD);
        CHECK(sideWallBefore < PORTRAIT_WARM_THRESHOLD &&
              sideWallAfterCancel < PORTRAIT_WARM_THRESHOLD &&
              sideWallAfterConfirm < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof on the panel-on frame: only sample the
     * unoccluded side-wall strip above C040, because the panel
     * covers the lower/right side-wall band by design. */
    {
        int leftSideOn = rect_warm_count(fbPanelOn,
                                         SIDE_WALL_LEFT_X, D1C_PORTRAIT_Y,
                                         SIDE_WALL_LEFT_W,
                                         D1C_PORTRAIT_TOP_VISIBLE_H);
        int rightSideOn = rect_warm_count(fbPanelOn,
                                          SIDE_WALL_RIGHT_X, D1C_PORTRAIT_Y,
                                          SIDE_WALL_RIGHT_W,
                                          D1C_PORTRAIT_TOP_VISIBLE_H);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "panel-on: side walls above C040 have < %d warm pixels "
                 "(left=%d, right=%d) - no portrait float while panel live",
                 PORTRAIT_WARM_THRESHOLD, leftSideOn, rightSideOn);
        CHECK(leftSideOn < PORTRAIT_WARM_THRESHOLD &&
              rightSideOn < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Up-handle contract summary: the input_focus_restore route
     * proves that HandleInput(UP) was IGNORED while the panel was
     * live and is now REDRAW after both the cancel and confirm
     * arms.  The world tick advanced on both arms, proving the
     * engine returned input focus to the world. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "input_focus_restore: HandleInput(UP) transitions "
                 "panel-live IGNORED -> after-cancel REDRAW (tick "
                 "delta=%u) -> after-confirm REDRAW (tick delta=%u)",
                 (unsigned)tickDeltaCancel, (unsigned)tickDeltaConfirm);
        CHECK(upHandlePanel == M11_GAME_INPUT_IGNORED &&
              upHandleAfterCancel == M11_GAME_INPUT_REDRAW &&
              upHandleAfterConfirm == M11_GAME_INPUT_REDRAW &&
              tickDeltaCancel >= TICK_DELTA_MIN &&
              tickDeltaConfirm >= TICK_DELTA_MIN, msg);
    }

    /* Restore the seeded sensor so subsequent CTest runs see the
     * shipped DM1 V1 data.  The confirm arm in Group E called
     * m11_disable_front_mirror_route which sets sensorType=0 on the
     * (3, 6) WEST C127 sensor; we restore both sensorType and
     * sensorData to the values saved before Group A.  Cancel does
     * NOT touch the sensor, so the cancel arm's exit state is also
     * restored here for symmetry. */
    if (h22SensorIndex >= 0 && state.world.things &&
        state.world.things->sensors) {
        state.world.things->sensors[h22SensorIndex].sensorType = h22SavedType;
        state.world.things->sensors[h22SensorIndex].sensorData = h22SavedData;
    }
    /* Clear candidate-panel state so the rewrite is reflected by
     * GetFrontMirrorOrdinal without a stale candidate being
     * returned. */
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.candidateMirrorPanelActive = 0;
    park_h22_front_route(&state);
    {
        int restoredOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after restore: front-mirror ordinal at (3,6) facing WEST = %d "
                 "(expected %d) - sensorType=%u, sensorData=%u",
                 restoredOrdinal, TARGET_ORDINAL,
                 (unsigned)h22SavedType, (unsigned)h22SavedData);
        CHECK(restoredOrdinal == TARGET_ORDINAL, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
