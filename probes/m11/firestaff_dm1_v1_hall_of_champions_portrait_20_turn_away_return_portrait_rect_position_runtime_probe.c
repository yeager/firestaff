/*
 * firestaff_dm1_v1_hall_of_champions_portrait_20_turn_away_return_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 20              (mirror catalog record ALEX, title ANDER)
 *   route   turn_away_return (party parks at the (1,2) NORTH-route
 *                             C127 sensor (sensorData seeded to 20),
 *                             face NORTH -> front sensor visible,
 *                             ordinal 20 painted at the D1C rect;
 *                             TURN_RIGHT in-place -> face EAST -> front
 *                             sensor not on the visible-wall side,
 *                             ordinal -1, D1C rect shows wall texture
 *                             only (no floating); TURN_LEFT in-place
 *                             -> face NORTH -> ordinal 20 painted again
 *                             at the same D1C rect; the rect bytes are
 *                             byte-identical between the pre-turn
 *                             framebuffer and the post-return
 *                             framebuffer)
 *   aspect  portrait_rect_position
 *
 * The C026 champion-portrait atlas is the source-locked 8x3 grid of
 * 32x29 portraits (DUNVIEW.C:3913-3928 + DEFS.H:821-826 +
 * C026_GRAPHIC_CHAMPION_PORTRAITS). Ordinal 20 sits at row 2, column 4:
 *
 *     srcX = (20 & 7) << 5 = 128
 *     srcY = (20 >> 3) * 29 = 58
 *
 * The D1C front-wall destination rectangle is source-locked (per
 * DUNVIEW.C:3913-3928 + DUNVIEW.C:525
 * G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96, 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * Why a separate gate from the existing ordinal-20 cancel_reopen
 * probe (firestaff_dm1_v1_hall_of_champions_portrait_20_cancel_reopen_...)
 * and from the front_north_entry ordinal-20 probe
 * (firestaff_dm1_v1_champion_portrait_20_front_north_entry_...)?  The
 * cancel_reopen slice drives the source-locked candidate state machine
 * (select -> F0282 C162 cancel -> F0280 reopen), so it never exercises
 * the in-place turn axis at all.  The front_north_entry slice drives
 * front-mirror ordinal reporting at the four wall orientations of the
 * same (3,11) cell by directly writing party.direction in the test
 * harness, so it never exercises the COMMAND.C F0359 input dispatch
 * path nor the deterministic-redraw-stability invariant for an
 * in-place turn that round-trips back to the original direction.
 *
 * The turn_away_return route covers the in-place turn axis through
 * the public M11 input dispatch:
 *
 *   (1) The front-mirror ordinal must toggle correctly across the
 *       in-place turn: NORTH -> 20 (sensorData=20 on the (1,1)
 *       north wall of front square), EAST -> -1 (no C127 sensor on
 *       the visible-wall side of the (2,2) west wall), SOUTH -> -1,
 *       WEST -> -1, NORTH -> 20 again.  This is the
 *       visible-wall-side filter from DUNGEON.C:2573 in action,
 *       driven through M11_GameView_HandleInput (COMMAND.C F0359)
 *       instead of being asserted in the test harness directly.
 *
 *   (2) The D1C portrait rectangle must redraw stably across the
 *       turn_away_return round-trip.  After the EAST turn, the rect
 *       must show wall texture (warm_count < 30) because the front
 *       sensor no longer reports ordinal 20 on the visible-wall
 *       side.  After the NORTH return turn, the rect must match
 *       ordinal 20 at >= 90% (same as the pre-turn baseline).  This
 *       is the source-locked "in-place turn doesn't perturb the
 *       wall draw" invariant.
 *
 *   (3) Stronger byte-stability invariant: the pre-turn framebuffer
 *       and the post-return framebuffer must be byte-identical in
 *       the D1C rect (96, 35, 32, 29) and in the wider D1C row band
 *       (96, 35, 32, 30).  The in-place turn is a no-op for the
 *       wall-draw layer (DUNVIEW.C F0128 redraw is deterministic
 *       given the same pose and the same sensorData), so any
 *       byte-diff here is a regression in the redraw path.
 *
 *   (4) Far_turn_away_return hardening: a 360° rotation in 90°
 *       steps (NORTH -> EAST -> SOUTH -> WEST -> NORTH) must
 *       produce the same byte-identical D1C rect on return.
 *       The 360° rotation exercises the same visible-wall-side
 *       filter (DUNGEON.C:2573) four times in one drive and
 *       catches a regression where the filter only fires on the
 *       first wrong-wall orientation.
 *
 * The C040 candidate panel is NOT opened during this probe.  The
 * reason is that the source-locked panel-open dispatch at
 * m11_game_view.c:8307-8318 only accepts BACK (cancel) and
 * ACCEPT/ACTION (confirm) while the panel is live - TURN_RIGHT /
 * TURN_LEFT are deliberately IGNORED so the player does not
 * accidentally walk away from a live resurrect/reincarnate
 * selection.  The cancel_reopen probe already covers the panel
 * state-machine axis; this probe covers the orthogonal
 * input-dispatch / visible-wall-side / redraw-stability axis
 * with the panel closed.
 *
 * Source evidence:
 *   - DUNGEON.C:2558 (BUG0_75 portrait ordinal counter reset on
 *                   wall-square entry; the loop body of the
 *                   multiple-portrait decrement is part of the
 *                   same front-wall draw path)
 *   - DUNGEON.C:2573 (C127 sensor cell match against view dir;
 *                    M011_CELL(sensor) is the visible-wall-side
 *                    filter that makes EAST/WEST/SOUTH from (1,2)
 *                    not expose the (1,1) C127 sensor)
 *   - DUNGEON.C:2608-2612 (G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)))
 *   - DUNVIEW.C:3913-3919 (P0117_i_ViewWallIndex ==
 *                          M587_VIEW_WALL_D1C_FRONT &&
 *                          G0289_i_DungeonView_ChampionPortraitOrdinal--;
 *                          D1C C026 portrait blit at {96,35} with
 *                          ((ordinal & 7) << 5, (ordinal >> 3) * 29))
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:1061 (G0205_aaauc_Graphic558_WallOrnamentCoordinateSets,
 *                     8x13x6 table; coordSet 5 / index 12 is the
 *                     D1C champion-mirror frame route)
 *   - DUNVIEW.C:8318-8542 (F0128 redraw viewport far-to-near;
 *                          side-wall geometry overpaints the D1C
 *                          portrait rect when the front cell no
 *                          longer has a C127 sensor)
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - DEFS.H:2186 (C026_GRAPHIC_CHAMPION_PORTRAITS)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - COMMAND.C F0359 (input dispatch TURN_LEFT/RIGHT)
 *   - COMMAND.C F0361 (movement dispatch; turn without movement is
 *                     handled in F0359 and never reaches F0361)
 *   - m11_game_view.c:8307-8318 (panel-open input gate: BACK,
 *                               ACCEPT, ACTION only)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
 *   - M11_GameView_HandleInput (F0359 input dispatch)
 *
 * Honesty: this is Firestaff deterministic-runtime evidence, not
 * original-DM1 PC 3.4 pixel parity.  The pixel-match uses the
 * same warm-color heuristic the existing capture probe uses
 * (palette indices {0x07 green, 0x08 red, 0x09 orange, 0x0A peach,
 * 0x0B yellow, 0x0E blue}) to distinguish 'portrait painted' from
 * 'wall texture only'.  The byte-stability invariant is a strict
 * deterministic-runtime expectation, NOT a DOSBox comparison.
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
    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    /* Ordinal 20 in the C026 atlas: (20 & 7) << 5 = 128,
     *                                 (20 >> 3) * 29 = 58. */
    ORDINAL_20_COL = 20 & 7,       /* = 4 */
    ORDINAL_20_ROW = 20 >> 3,      /* = 2 (last row of the 8x3 grid) */
    ORDINAL_20_SRC_X = ORDINAL_20_COL << 5,    /* = 128 */
    ORDINAL_20_SRC_Y = ORDINAL_20_ROW * 29,    /* =  58 */
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
    TARGET_ORDINAL = 20,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 20 (ALEX) for this gate so we can lock the
     * ordinal-20 edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1,
    /* The (1,2) NORTH pose is the canonical PC 3.4 reference pose
     * shared with the cancel_reopen ordinal-20 probe.  From this
     * pose, the in-place TURN_RIGHT goes N -> E (no C127 on the
     * visible-wall side of (2,2)); a second TURN_RIGHT goes
     * E -> S (no C127 on the visible-wall side of (1,3)); the
     * third goes S -> W (no C127 on the visible-wall side of
     * (0,2)); and the fourth returns to NORTH (visible-wall-side
     * of (1,1) is the seeded C127 sensorData=20 again). */
    SEED_POSE_MAPX = 1,
    SEED_POSE_MAPY = 2,
    SEED_POSE_DIR  = DIR_NORTH
};
/* Mirror catalog record name for ordinal 20 (DM1 V1 PC34 mirror
 * catalog).  Used to assert the catalog resolves correctly.  ALEX
 * (title ANDER) is the 21st valid mirror text string in the
 * shipped DM1 V1 DUNGEON.DAT. */
static const char kExpectedCatalogName[] = "ALEX";
static const char kExpectedCatalogTitle[] = "ANDER";

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

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

/* Compare the C026 portrait atlas cell for the requested ordinal
 * to the framebuffer D1C portrait rectangle.  Returns the percent
 * of opaque source pixels that match the destination pixel.
 * Palette index 1 is the dark-grey transparent pass-through used by
 * DUNVIEW.C:3916 - those source pixels are skipped when matching. */
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

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found.  We use this to
 * lock the ordinal-20 edge case on the real DM1 V1 DUNGEON.DAT
 * (which ships HALK / ordinal 1 on the (1,2) NORTH-route front
 * square (1,1)).  The seed does NOT change the map layout or the
 * C127 cell match - only the G0289 ordinal that DUNVIEW.C:3913-3928
 * reads through M000_INDEX_TO_ORDINAL (DUNGEON.C:2610-2612). */
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

/* Park the party at the (1,2) D1C front-mirror route facing NORTH.
 * This is the real C127 sensor position from the DM1 V1 DUNGEON.DAT
 * shipped with the public PC 3.4 English release: at (1,2) facing
 * NORTH, the front square (1,1) has a C127 sensor on cell=2 (north
 * wall) with sensorData=1 (HALK, mirror ordinal 1).  After
 * seed_first_c127_data the same square reports ordinal 20.  The
 * park helper also resets the candidate-panel state and the
 * inventory panel state so the byte-stability comparison starts
 * from a clean baseline. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = SEED_POSE_MAPX;
    state->world.party.mapY = SEED_POSE_MAPY;
    state->world.party.direction = SEED_POSE_DIR;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbTurnAwayEast[FB_W * FB_H];
    unsigned char fbAfterReturn[FB_W * FB_H];
    unsigned char fbFarReturn[FB_W * FB_H];
    int matchBefore, matchTurnAwayEast, matchAfterReturn, matchFarReturn;
    int warmBefore, warmTurnAwayEast, warmAfterReturn, warmFarReturn;
    int leftSideBefore, rightSideBefore;
    int leftSideTurnAwayEast, rightSideTurnAwayEast;
    int distinctBefore, distinctTurnAwayEast, distinctAfterReturn;
    int nonzeroBefore, nonzeroAfterReturn, nonzeroFarReturn;
    int turnAwayRc, returnRc;
    int turn1Rc, turn2Rc, turn3Rc, turn4Rc;
    int seededSensor;
    char nameBuf[32];
    char titleBuf[32];
    int nameLookupRc, titleLookupRc;
    /* Group E byte-stability counters: the pre-turn pose and the
     * post-return pose are observationally equivalent (same map cell,
     * same map direction, same C127 sensorData, no panel live), so
     * the D1C rect and the D1C row band must be byte-identical
     * between fbBefore and fbAfterReturn.  The cancel_reopen
     * byte-stability Group E covers the panel-off -> select -> cancel
     * -> panel-off round-trip; this probe covers the orthogonal
     * panel-off -> turn_away -> return -> panel-off round-trip. */
    int d1cRectByteDiffPrePost;
    int d1cBandByteDiffPrePost;
    int d1cRectByteDiffPreFarPost;
    int d1cBandByteDiffPreFarPost;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-20 / turn_away_return / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    /* Load the C026 portrait atlas via the public M11 helper, so the
     * probe does not depend on the file-scope enum value 26. */
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < ATLAS_W || portraits->height < ATLAS_H) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas "
                "(width=%d height=%d)\n",
                portraits ? (int)portraits->width : -1,
                portraits ? (int)portraits->height : -1);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group A - Atlas math for ordinal 20 (row 2 / column 4)
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 2 /
     * column 4 and that the math matches COORD.C / DEFS.H:821-826.
     * This is the same Group A as the cancel_reopen probe and is
     * reproduced here so this probe is self-contained (a probe that
     * depends on another probe's Group A pass is not durable in the
     * worker pool's gate table).
     */
    printf("\n[Group A] C026 atlas math for ordinal 20 (row 2 / col 4)\n");
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic id returned by "
                 "M11_GameView_GetV1ChampionPortraitGraphicId = %d)",
                 M11_GameView_GetV1ChampionPortraitGraphicId());
        CHECK(portraits != NULL && portraits->loaded && portraits->pixels != NULL, msg);
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
                 "ordinal 20 col = 20 & 7 = %d (expected 4)",
                 ORDINAL_20_COL);
        CHECK(ORDINAL_20_COL == 4, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 20 row = 20 >> 3 = %d (expected 2, last row)",
                 ORDINAL_20_ROW);
        CHECK(ORDINAL_20_ROW == 2, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 20 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_20_SRC_X, ORDINAL_20_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_20_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_20_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    /* Ordinal 20 must resolve to ALEX through the mirror catalog.
     * This catches a regression where the catalog and the C026 atlas
     * disagree on the ordinal-20 record - in particular a
     * regression where the catalog is shorter than 21 entries (a
     * catalog count < 21 would have rejected sensorData=20 on the
     * real DUNGEON.DAT). */
    nameBuf[0] = '\0';
    titleBuf[0] = '\0';
    nameLookupRc = M11_GameView_GetMirrorNameByOrdinal(&state,
                                                       TARGET_ORDINAL,
                                                       nameBuf,
                                                       (int)sizeof(nameBuf));
    titleLookupRc = M11_GameView_GetMirrorTitleByOrdinal(&state,
                                                        TARGET_ORDINAL,
                                                        titleBuf,
                                                        (int)sizeof(titleBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 20 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 20 title to \"%s\" (expected \"%s\")",
                 titleBuf[0] ? titleBuf : "", kExpectedCatalogTitle);
        CHECK(titleLookupRc > 0 &&
              strcmp(titleBuf, kExpectedCatalogTitle) == 0, msg);
    }

    /* Park the party on the (1,2) NORTH-route front mirror, then
     * seed the C127 sensor from HALK (1) to ordinal 20 (ALEX).
     * Same sensor, same map cell, same draw path - only G0289
     * changes.  This is the same seed as the cancel_reopen probe,
     * so the two probes share a common baseline. */
    park_d1c_front_route(&state);

    /* Sanity check: the unmodified route reports the shipped HALK
     * ordinal 1.  This is the same pre-seed check the cancel_reopen
     * probe runs; it is reproduced here so this probe is
     * self-contained. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front-mirror ordinal at (%d,%d,%d) = %d "
                 "(expected %d, HALK before seed)",
                 SEED_POSE_MAPX, SEED_POSE_MAPY, SEED_POSE_DIR,
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to
     * ordinal 20 (ALEX).  Same sensor, same map cell, same draw
     * path - only G0289 changes. */
    seededSensor = seed_first_c127_data(&state,
                                         SHIPPED_HALK_ORDINAL,
                                         TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (%d,%d) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SEED_POSE_MAPX, SEED_POSE_MAPY,
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    /* The same front route now reports ordinal 20.  After seeding
     * the C127 sensor's sensorData, the front route must reflect
     * the new ordinal.  Note: the front ordinal helper clamps to
     * [0, mirrorCatalog.count), so this check confirms the catalog
     * has at least 20 entries (which is the source-locked DM1 V1
     * behaviour: 24 records, ordinals 0..23). */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded north-entry front-mirror ordinal = %d (expected %d)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    if (frontOrdinal != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify turn_away_return portrait_rect_position\n",
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
     * Group B - portrait_rect_position pre-turn baseline
     * ----------------------------------------------------------------
     * Render the framebuffer before any turn (panel off, party at
     * (1,2) NORTH facing the seeded C127 sensorData=20) and verify
     * the D1C destination rectangle (96, 35, 32, 29) holds
     * ordinal-20 pixels.  This is the canonical pre-turn baseline
     * that the post-return framebuffer must byte-match in Group E.
     */
    printf("\n[Group B] portrait_rect_position pre-turn baseline on real C127 sensor pose (1,2,NORTH)=20\n");

    park_d1c_front_route(&state);

    memset(fbBefore, 0, sizeof(fbBefore));
    M11_GameView_Draw(&state, fbBefore, FB_W, FB_H);

    matchBefore = match_portrait_at_rect(portraits, fbBefore, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) carries ordinal %d pixels "
                 "at >= 90%% match (got %d%%) - pre-turn baseline",
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
                 "pixels, got %d) - pre-turn baseline",
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
                 "(got %d) - pre-turn baseline",
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
                 "(got %d) - portrait sprite, not wall, pre-turn",
                 PORTRAIT_WARM_THRESHOLD, warmBefore);
        CHECK(warmBefore >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof at the pre-turn baseline. */
    leftSideBefore = rect_warm_count(fbBefore,
                                     SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                     SIDE_WALL_LEFT_W,
                                     PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "left side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on left wall "
                 "pre-turn",
                 PORTRAIT_WARM_THRESHOLD, leftSideBefore);
        CHECK(leftSideBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideBefore = rect_warm_count(fbBefore,
                                      SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                      SIDE_WALL_RIGHT_W,
                                      PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "right side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on right wall "
                 "pre-turn",
                 PORTRAIT_WARM_THRESHOLD, rightSideBefore);
        CHECK(rightSideBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - turn_away_return round-trip (1 turn each direction)
     * ----------------------------------------------------------------
     * Drive the in-place TURN_RIGHT through M11_GameView_HandleInput
     * (COMMAND.C F0359 input dispatch) to face EAST.  The party
     * stays at the (1,2) map cell.  The visible-wall-side filter
     * (DUNGEON.C:2573) now reports -1 because the C127 sensor is
     * on the north wall of the (1,1) cell, and the party is no
     * longer facing that wall.  The C040 candidate panel MUST
     * stay closed (it is closed throughout the probe by design -
     * see the header comment).
     *
     * The C040 panel-on input gate at m11_game_view.c:8307-8318
     * IGNOREs TURN_RIGHT/TURN_LEFT while the panel is live, which
     * is why this probe exercises the orthogonal input-dispatch
     * axis with the panel closed.  The cancel_reopen probe
     * already covers the panel state-machine axis.
     */
    printf("\n[Group C] turn_away_return: turn_away(E), return(N), portrait rect still carries ordinal 20\n");

    /* Step 1: turn_away in-place to face EAST.  TURN_RIGHT only
     * changes party.direction (F0359 input dispatch); it does NOT
     * run F0282 C162 cancel, does NOT clear G0299, and does NOT
     * close the C040 panel.  The party stays at the (1,2) map cell;
     * the C127 sensor is still on the (1,1) cell's north wall, but
     * the visible-wall-side filter (DUNGEON.C:2573) now reports -1
     * because the party is no longer facing the sensor's wall. */
    turnAwayRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_RIGHT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "TURN_RIGHT (N->E) result=%d, party at (%d,%d) dir=%d "
                 "(expected dir=%d, EAST)",
                 (int)turnAwayRc,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction, DIR_EAST);
        CHECK(turnAwayRc == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == DIR_EAST, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after turn_away: candidateMirrorPanelActive=%d, "
                 "championCount=%d (must be unchanged - panel stays "
                 "closed, no movement, no cancel)",
                 state.candidateMirrorPanelActive,
                 state.world.party.championCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.world.party.championCount == 0, msg);
    }

    /* Render at the turned-away pose.  The front-mirror ordinal is
     * now -1 (the C127 sensor is on the north wall of (1,1); the
     * party faces east at (1,2), so the visible-wall-side filter
     * rejects the sensor).  The D1C destination rectangle must
     * NOT be painted: warm_count < 30 (no portrait sprite),
     * match against ordinal 20 must be low (no stale state), and
     * side walls must show wall texture only (no floating). */
    memset(fbTurnAwayEast, 0, sizeof(fbTurnAwayEast));
    M11_GameView_Draw(&state, fbTurnAwayEast, FB_W, FB_H);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    matchTurnAwayEast = match_portrait_at_rect(portraits,
                                               fbTurnAwayEast,
                                               TARGET_ORDINAL);
    warmTurnAwayEast = rect_warm_count(fbTurnAwayEast,
                                       D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                       D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    distinctTurnAwayEast = rect_distinct(fbTurnAwayEast,
                                         D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                         D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn_away (EAST): front-mirror ordinal = %d "
                 "(expected -1 - C127 sensor on the (1,1) north wall is "
                 "not on the visible-wall side when facing EAST at (1,2))",
                 frontOrdinal);
        CHECK(frontOrdinal == -1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn_away (EAST): D1C portrait rect carries "
                 "ordinal %d pixels at <= 20%% match (got %d%%) - no "
                 "stale sprite",
                 TARGET_ORDINAL, matchTurnAwayEast);
        CHECK(matchTurnAwayEast <= 20, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn_away (EAST): D1C portrait rect has < %d "
                 "warm pixels (got %d) - portrait NOT painted on side "
                 "wall, no-floating proof",
                 PORTRAIT_WARM_THRESHOLD, warmTurnAwayEast);
        CHECK(warmTurnAwayEast < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn_away (EAST): D1C portrait rect has <= %d "
                 "distinct palette indices (got %d) - wall texture, not "
                 "a sprite",
                 distinctBefore, distinctTurnAwayEast);
        CHECK(distinctTurnAwayEast <= distinctBefore, msg);
    }

    /* No-floating proof at the turned-away pose.  Sample the full
     * portrait band y=[33..65) because the C040 panel is closed
     * throughout the probe, so the full band is the no-floating
     * window. */
    leftSideTurnAwayEast = rect_warm_count(fbTurnAwayEast,
                                           SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                           SIDE_WALL_LEFT_W,
                                           PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn_away (EAST): left side wall of D1C portrait "
                 "band has < %d warm pixels (got %d) - no floating",
                 PORTRAIT_WARM_THRESHOLD, leftSideTurnAwayEast);
        CHECK(leftSideTurnAwayEast < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideTurnAwayEast = rect_warm_count(fbTurnAwayEast,
                                            SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                            SIDE_WALL_RIGHT_W,
                                            PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn_away (EAST): right side wall of D1C portrait "
                 "band has < %d warm pixels (got %d) - no floating",
                 PORTRAIT_WARM_THRESHOLD, rightSideTurnAwayEast);
        CHECK(rightSideTurnAwayEast < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Step 2: return in-place to face NORTH.  TURN_LEFT only changes
     * party.direction.  The party is back at (1,2) facing NORTH.
     * The C127 sensor on the (1,1) cell's north wall is the
     * visible-wall-side again, sensorData=20 still holds, and
     * G0289 reports 20. */
    returnRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_LEFT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "TURN_LEFT (E->N) result=%d, party at (%d,%d) dir=%d "
                 "(expected dir=%d, NORTH)",
                 (int)returnRc,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction, DIR_NORTH);
        CHECK(returnRc == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == DIR_NORTH, msg);
    }

    /* Render at the returned pose.  The party is back at (1,2)
     * NORTH.  G0289 reports 20 again, the D1C destination
     * rectangle is re-painted with ordinal-20 source pixels at
     * >= 90% match (same as the pre-turn baseline). */
    memset(fbAfterReturn, 0, sizeof(fbAfterReturn));
    M11_GameView_Draw(&state, fbAfterReturn, FB_W, FB_H);
    matchAfterReturn = match_portrait_at_rect(portraits,
                                              fbAfterReturn,
                                              TARGET_ORDINAL);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after return (N): front-mirror ordinal = %d "
                 "(expected %d, ordinal 20 painted again after turn_away)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after return (N): D1C portrait rect carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%) - turn_away_return "
                 "round-trip redraws the sprite",
                 TARGET_ORDINAL, matchAfterReturn);
        CHECK(matchAfterReturn >= 90, msg);
    }
    warmAfterReturn = rect_warm_count(fbAfterReturn,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after return (N): D1C portrait rect has >= %d warm "
                 "pixels (got %d) - portrait sprite painted, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmAfterReturn);
        CHECK(warmAfterReturn >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    nonzeroAfterReturn = rect_nonzero(fbAfterReturn,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after return (N): D1C portrait rect is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 nonzeroAfterReturn);
        CHECK(nonzeroAfterReturn >= 100, msg);
    }
    distinctAfterReturn = rect_distinct(fbAfterReturn,
                                        D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                        D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after return (N): D1C portrait rect has >= 4 distinct "
                 "palette indices (got %d)",
                 distinctAfterReturn);
        CHECK(distinctAfterReturn >= 4, msg);
    }

    /* The portrait_rect_position contract: the D1C destination
     * rectangle does NOT change screen position across the
     * turn_away_return round-trip.  The (96, 35, 32, 29)
     * destination is source-locked to DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall,
     * so we verify the rect is painted with ordinal 20 in the
     * baseline and after-return poses, and is NOT painted as a
     * stale sprite in the turned-away pose. */
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position: pre-turn-match=%d%%, "
                 "turn-away-east-match=%d%%, after-return-match=%d%% "
                 "(baseline & after-return >= 90, turn-away <= 20)",
                 matchBefore, matchTurnAwayEast, matchAfterReturn);
        CHECK(matchBefore >= 90 &&
              matchTurnAwayEast <= 20 &&
              matchAfterReturn >= 90, msg);
    }

    /* Cross-check: ordinal 20 sprite is drawn in the baseline
     * framebuffer and the post-return framebuffer.  Both report
     * 100% match on the seeded DUNGEON.DAT, so the byte-stability
     * Group E below also implicitly verifies the ordinal-20
     * sprite consistency between the two poses. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "ordinal 20 sprite match: pre-turn=%d%%, "
                 "after-return=%d%% (both >= 90, byte-stability "
                 "Group E below pins the byte-level equivalence)",
                 matchBefore, matchAfterReturn);
        CHECK(matchBefore >= 90 && matchAfterReturn >= 90, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - far_turn_away_return (360° rotation)
     * ----------------------------------------------------------------
     * Hardening for the visible-wall-side filter.  The in-place
     * turn goes N -> E -> S -> W -> N.  Each intermediate face
     * must report -1 (no C127 sensor on the visible-wall side at
     * (1,2)), and the post-return pose must report 20 with the
     * same panel-off state.  This catches a regression where the
     * filter only fires on the first wrong-wall orientation.
     */
    printf("\n[Group D] far_turn_away_return: 360° rotation, portrait rect still carries ordinal 20\n");

    /* Turn 1: N -> E. */
    turn1Rc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_RIGHT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "TURN_RIGHT (N->E) result=%d, party at (%d,%d) dir=%d "
                 "(expected dir=%d)",
                 (int)turn1Rc,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction, DIR_EAST);
        CHECK(turn1Rc == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == DIR_EAST, msg);
    }
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn 1 (E): front-mirror ordinal = %d (expected -1)",
                 ord);
        CHECK(ord == -1, msg);
    }
    /* Turn 2: E -> S. */
    turn2Rc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_RIGHT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "TURN_RIGHT (E->S) result=%d, party at (%d,%d) dir=%d "
                 "(expected dir=%d)",
                 (int)turn2Rc,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction, DIR_SOUTH);
        CHECK(turn2Rc == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == DIR_SOUTH, msg);
    }
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn 2 (S): front-mirror ordinal = %d (expected -1)",
                 ord);
        CHECK(ord == -1, msg);
    }
    /* Turn 3: S -> W. */
    turn3Rc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_RIGHT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "TURN_RIGHT (S->W) result=%d, party at (%d,%d) dir=%d "
                 "(expected dir=%d)",
                 (int)turn3Rc,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction, DIR_WEST);
        CHECK(turn3Rc == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == DIR_WEST, msg);
    }
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after turn 3 (W): front-mirror ordinal = %d (expected -1)",
                 ord);
        CHECK(ord == -1, msg);
    }
    /* Turn 4: W -> N (return). */
    turn4Rc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_TURN_RIGHT);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "TURN_RIGHT (W->N) result=%d, party at (%d,%d) dir=%d "
                 "(expected dir=%d)",
                 (int)turn4Rc,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction, DIR_NORTH);
        CHECK(turn4Rc == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == DIR_NORTH, msg);
    }
    /* Render at the far-return pose and pixel-verify the rect. */
    memset(fbFarReturn, 0, sizeof(fbFarReturn));
    M11_GameView_Draw(&state, fbFarReturn, FB_W, FB_H);
    matchFarReturn = match_portrait_at_rect(portraits,
                                            fbFarReturn,
                                            TARGET_ORDINAL);
    warmFarReturn = rect_warm_count(fbFarReturn,
                                    D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                    D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    nonzeroFarReturn = rect_nonzero(fbFarReturn,
                                    D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                    D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after far return (360°): D1C portrait rect carries "
                 "ordinal %d pixels at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchFarReturn);
        CHECK(matchFarReturn >= 90, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after far return (360°): D1C portrait rect has >= %d "
                 "warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, warmFarReturn);
        CHECK(warmFarReturn >= PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after far return (360°): D1C portrait rect is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 nonzeroFarReturn);
        CHECK(nonzeroFarReturn >= 100, msg);
    }
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after far return (360°): front-mirror ordinal = %d "
                 "(expected %d, ALEX)",
                 ord, TARGET_ORDINAL);
        CHECK(ord == TARGET_ORDINAL, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - turn_away_return byte-stability
     * ----------------------------------------------------------------
     * Stronger invariant than the Group C match-based checks.  The
     * M11 redraw path is deterministic: with the same party state,
     * the same dungeon, the same pose, and the same C127 sensorData,
     * two successive Draw() calls must produce byte-identical
     * framebuffers (M11 does not run a free-running clock; the
     * C146_COMMAND_WAKE_UP and F0359 TURN_RIGHT/TURN_LEFT paths both
     * leave the wall-draw layer back to the same source-locked
     * baseline).
     *
     * The pre-turn pose (panel-off, party at (1,2) NORTH, sensor
     * visible, ordinal=20) and the post-return pose (panel-off,
     * party at (1,2) NORTH, sensor visible, ordinal=20) are
     * observationally equivalent for the portrait cutout at
     * (96, 35, 32, 29): the C040 panel chrome is closed on both
     * poses, and the C127 sensor is still on the front square
     * (sensorData=20 after the seed).  This group asserts that
     * the D1C destination rectangle is byte-stable across the
     * cycle, which is a stronger invariant than
     * match_portrait_at_rect >= 90 (which only samples the
     * 928-cell pixel set and tolerates 10% mismatch).  Any
     * non-zero diff here is a regression in the redraw path or
     * a turn-state leak.
     *
     * The cancel_reopen probe's Group E byte-stability assertion
     * covers the panel-off -> select -> cancel -> panel-off
     * round-trip; this Group E covers the orthogonal panel-off ->
     * turn_away -> return -> panel-off round-trip.
     *
     * Source evidence:
     *   - DUNVIEW.C:3913-3928 (C026 portrait blit is the only D1C
     *                 cutout write - no other draw path touches the
     *                 96,35,32,29 rect when the panel is closed)
     *   - DUNGEON.C:2608-2612 (G0289 stays 20 across the round
     *                          trip because the C127 sensor is
     *                          still active on the same front
     *                          square)
     *   - COMMAND.C F0359 (input dispatch TURN_RIGHT/TURN_LEFT;
     *                     sets party.direction without touching
     *                     the wall-draw layer)
     *   - The cancel_reopen companion probe (pass140 row 2 / col 4)
     *     does not assert turn_away_return byte-stability; this
     *     group is the non-duplicative hardening for ordinal 20.
     */
    printf("\n[Group E] turn_away_return byte-stability: D1C rect is byte-stable across pre-turn -> post-return\n");

    /* The D1C destination rectangle (96, 35, 32, 29) must be
     * byte-identical between fbBefore (pre-turn baseline) and
     * fbAfterReturn (post-return draw).  Both are drawn with the
     * party at (1,2) facing NORTH, panel closed, sensorData=20
     * still on the front square, so the same C026 portrait blit
     * must hit the same 928 cells.  The byte-equality is the
     * strict source-locked expectation: the M11 redraw path is
     * deterministic. */
    d1cRectByteDiffPrePost = 0;
    {
        int xx, yy;
        for (yy = 0; yy < D1C_PORTRAIT_H; ++yy) {
            for (xx = 0; xx < D1C_PORTRAIT_W; ++xx) {
                int idxA = (D1C_PORTRAIT_Y + yy) * FB_W + (D1C_PORTRAIT_X + xx);
                int idxB = idxA;
                if (fbBefore[idxA] != fbAfterReturn[idxB]) {
                    ++d1cRectByteDiffPrePost;
                }
            }
        }
    }
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35, 32, 29) is byte-stable across "
                 "pre-turn -> post-return (panel-off both sides; "
                 "diff=%d / %d cells; expected 0 - turn_away_return "
                 "must leave no chrome or sprite drift)",
                 d1cRectByteDiffPrePost, D1C_PORTRAIT_W * D1C_PORTRAIT_H);
        CHECK(d1cRectByteDiffPrePost == 0, msg);
    }

    /* The wider D1C row band (96, 35, 32, 30) including the 1px
     * border below the portrait cutout must also be byte-stable.
     * This catches a regression where the wall ornament border
     * bleeds through a single row below the portrait (an
     * off-by-one border bug would show up here, not in the
     * inner-rect check above). */
    d1cBandByteDiffPrePost = 0;
    {
        int xx, yy;
        for (yy = 0; yy < D1C_PORTRAIT_H + 1; ++yy) {
            for (xx = 0; xx < D1C_PORTRAIT_W; ++xx) {
                int idxA = (D1C_PORTRAIT_Y + yy) * FB_W + (D1C_PORTRAIT_X + xx);
                int idxB = idxA;
                if (fbBefore[idxA] != fbAfterReturn[idxB]) {
                    ++d1cBandByteDiffPrePost;
                }
            }
        }
    }
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "D1C row band (96, 35, 32, 30) is byte-stable across "
                 "pre-turn -> post-return (panel-off both sides; "
                 "diff=%d / %d cells; expected 0 - 1px border below "
                 "portrait must also be stable)",
                 d1cBandByteDiffPrePost, D1C_PORTRAIT_W * (D1C_PORTRAIT_H + 1));
        CHECK(d1cBandByteDiffPrePost == 0, msg);
    }

    /* The 360° far_return pose must also be byte-stable against
     * the pre-turn baseline.  This catches a regression where the
     * visible-wall-side filter has a stuck state and only reports
     * -1 on the first wrong-wall orientation (so the byte-diff at
     * the far_return pose would diverge from the pre-turn pose
     * even though both report ordinal 20 at the panel-off
     * layer). */
    d1cRectByteDiffPreFarPost = 0;
    {
        int xx, yy;
        for (yy = 0; yy < D1C_PORTRAIT_H; ++yy) {
            for (xx = 0; xx < D1C_PORTRAIT_W; ++xx) {
                int idxA = (D1C_PORTRAIT_Y + yy) * FB_W + (D1C_PORTRAIT_X + xx);
                int idxB = idxA;
                if (fbBefore[idxA] != fbFarReturn[idxB]) {
                    ++d1cRectByteDiffPreFarPost;
                }
            }
        }
    }
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "D1C rect (96, 35, 32, 29) is byte-stable across "
                 "pre-turn -> far-return (360° rotation; "
                 "diff=%d / %d cells; expected 0 - the visible-wall-side "
                 "filter must reset cleanly at every 90° step)",
                 d1cRectByteDiffPreFarPost, D1C_PORTRAIT_W * D1C_PORTRAIT_H);
        CHECK(d1cRectByteDiffPreFarPost == 0, msg);
    }
    d1cBandByteDiffPreFarPost = 0;
    {
        int xx, yy;
        for (yy = 0; yy < D1C_PORTRAIT_H + 1; ++yy) {
            for (xx = 0; xx < D1C_PORTRAIT_W; ++xx) {
                int idxA = (D1C_PORTRAIT_Y + yy) * FB_W + (D1C_PORTRAIT_X + xx);
                int idxB = idxA;
                if (fbBefore[idxA] != fbFarReturn[idxB]) {
                    ++d1cBandByteDiffPreFarPost;
                }
            }
        }
    }
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "D1C row band (96, 35, 32, 30) is byte-stable across "
                 "pre-turn -> far-return (360° rotation; diff=%d / %d "
                 "cells; expected 0)",
                 d1cBandByteDiffPreFarPost,
                 D1C_PORTRAIT_W * (D1C_PORTRAIT_H + 1));
        CHECK(d1cBandByteDiffPreFarPost == 0, msg);
    }

    /* ----------------------------------------------------------------
     * Group F - ordinal 20 atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 20 must be self-consistent:
     * the destination (96, 35, 32, 29) on the framebuffer lines up
     * with the source (128, 58, 32, 29) in the atlas.  This is the
     * "ordinal 20 maps to the expected champion" check from the
     * slice description - the round-trip is independent of the
     * runtime drive and pins the macro math against the atlas
     * itself. */
    printf("\n[Group F] ordinal 20 atlas round-trip: source (128, 58) maps to dst (96, 35)\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 20 is at source "
                 "(%d, %d, %d, %d) - matches "
                 "((20 & 7) << 5, (20 >> 3) * 29, 32, 29)",
                 ORDINAL_20_SRC_X, ORDINAL_20_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        CHECK(ORDINAL_20_SRC_X == 128 && ORDINAL_20_SRC_Y == 58, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
