/*
 * firestaff_dm1_v1_hall_of_champions_portrait_06_backtrack_same_tick_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 6                (mirror catalog record SYRA, title CHILD OF NATURE)
 *   route   backtrack_same_tick  (party parks at the (1,2) NORTH-route
 *                                 C127 sensor seeded to ordinal 6,
 *                                 opens the C040 candidate panel via
 *                                 M11_GameView_SelectFrontMirrorCandidate,
 *                                 then drives a SINGLE call to
 *                                 M11_GameView_HandleInput with
 *                                 M12_MENU_INPUT_BACK and verifies the
 *                                 input dispatch atomically (a) routes
 *                                 BACK into M11_GameView_CancelMirrorCandidate
 *                                 (F0282 C162 branch), (b) clears the
 *                                 panel state, (c) decrements
 *                                 championCount, (d) sets
 *                                 lastAction="MIRROR" /
 *                                 lastOutcome="CANCELLED", (e) sets
 *                                 inspectTitle="CHAMPION MIRROR" /
 *                                 inspectDetail="SELECTION CANCELLED",
 *                                 and (f) returns
 *                                 M11_GAME_INPUT_REDRAW - all in a
 *                                 single same-tick input dispatch.
 *                                 After the backtrack the probe
 *                                 pixel-proves the D1C portrait rect
 *                                 still carries ordinal-6 pixels
 *                                 (same-tick redraw uses the
 *                                 wall-ornament + portrait draw path,
 *                                 not the panel-guard path), and
 *                                 that the C127 sensor is still
 *                                 alive (a fresh select returns 1 and
 *                                 reopens the panel).
 *   aspect  portrait_rect_position
 *
 * This probe is a *companion* to the cancel_reopen and
 * leave_and_reenter slices for ordinal 6:
 *
 *   - cancel_reopen    (firestaff_dm1_v1_hall_of_champions_portrait_06_cancel_reopen_...)
 *                       drives select -> CancelMirrorCandidate
 *                       (direct API) -> reopen via SelectFrontMirrorCandidate.
 *   - leave_and_reenter (firestaff_dm1_v1_hall_of_champions_portrait_06_leave_and_reenter_...)
 *                       drives select -> CancelMirrorCandidate (direct)
 *                       -> walk-away via M11_GameView_HandleInput
 *                       -> teleport back -> re-select.
 *   - backtrack_same_tick  (this probe)
 *                       drives select -> SINGLE HandleInput(BACK)
 *                       call.  The single-tick constraint is the
 *                       differentiator: cancel_reopen and
 *                       leave_and_reenter never call HandleInput
 *                       for the cancel leg, they call
 *                       M11_GameView_CancelMirrorCandidate
 *                       directly.  The backtrack_same_tick slice
 *                       verifies the production input dispatch path
 *                       (the BACK key on the panel) does the cancel
 *                       atomically and returns REDRAW, not IGNORED.
 *
 * Why the input-dispatch path matters: the source-locked contract
 * for the candidate panel is that BACK closes the panel and
 * ACKNOWLEDGES the cancel with a redraw (m11_game_view.c:8303-8314
 * "M11 maps ACTION/ACCEPT to the default resurrect choice for now
 * and BACK to cancel; the public confirm API keeps probes
 * explicit").  If a future refactor of M11_GameView_HandleInput
 * drops the BACK path (e.g. treats it as IGNORED while the panel
 * is live), the panel would be unclosable for the user.  This
 * probe locks that BACK -> CancelMirrorCandidate routing in a
 * real runtime drive.
 *
 * The probe covers three coupled concerns of backtrack_same_tick
 * in one runtime drive:
 *
 *   (1) Atlas math for ordinal 6 (Group A).  Same as
 *       cancel_reopen: the C026 atlas contains a defined portrait
 *       at (192, 0, 32, 29), the (6 & 7) << 5 / (6 >> 3) * 29
 *       math matches DEFS.H:821-826, ordinal 6 vs 5 (ELIJA) and
 *       vs 7 (SONJA) are distinct portraits, and the mirror
 *       catalog resolves ordinal 6 to "SYRA".
 *
 *   (2) portrait_rect_position baseline (Group B).  Same pose as
 *       cancel_reopen: park the party at (1,2) NORTH, seed the
 *       C127 sensor from HALK (1) to ordinal 6, and pixel-prove
 *       the destination rectangle (96, 35, 32, 29) on the 320x200
 *       framebuffer contains the ordinal-6 champion portrait
 *       (>= 90% match) with no floating wall ornament.
 *
 *   (3) backtrack_same_tick (Group C): select the candidate, then
 *       drive a SINGLE call to M11_GameView_HandleInput with
 *       M12_MENU_INPUT_BACK.  Verify the return value is
 *       M11_GAME_INPUT_REDRAW (not IGNORED - the BACK key
 *       must produce a redraw on the same tick), the panel
 *       state is fully cleared (active=0, ordinal=-1,
 *       partyIndex=-1, championCount decremented), the
 *       lastAction/lastOutcome and inspectTitle/inspectDetail
 *       readouts are set to the source-locked "CANCELLED"
 *       strings, and a pixel-prove of the D1C portrait rect
 *       on the SAME framebuffer shows ordinal-6 pixels at
 *       >= 90% match (proving the same-tick redraw used the
 *       wall-ornament + portrait draw path, not the panel
 *       draw path).  Then verify the C127 sensor is still
 *       alive by re-selecting via the direct API - if the
 *       cancel had disabled the sensor, the re-select would
 *       return 0 instead of 1.
 *
 * Source evidence:
 *   - DUNGEON.C:2558 (BUG0_75 portrait ordinal counter reset)
 *   - DUNGEON.C:2608 (C127 sensor type match)
 *   - DUNGEON.C:2612 (G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)))
 *   - DUNVIEW.C:3913 (P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT
 *                     && G0289_i_DungeonView_ChampionPortraitOrdinal--)
 *   - DUNVIEW.C:3916-3919 (D1C C026 portrait blit at {96,35} with
 *                          ((ordinal & 7) << 5, (ordinal >> 3) * 29))
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:8318-8542 (F0128 redraw viewport far-to-near)
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C F0280:124-132 (C040 empty-leader candidate gate)
 *   - REVIVE.C F0282:744-806 (C162 cancel branch 744-783)
 *   - PANEL.C F0355:2299-2318 (inventory close on cancel)
 *   - COMMAND.C F0378:1956-1990 (M568_PANEL_RESURRECT_REINCARNATE
 *                              dispatch)
 *   - m11_game_view.c:8303-8314 (M11_GameView_HandleInput panel-active
 *                                BACK -> M11_GameView_CancelMirrorCandidate
 *                                dispatch, ACTION/ACCEPT -> confirm dispatch,
 *                                else IGNORED).
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard:
 *                                     candidateMirrorPanelActive == 1
 *                                     skips the wall ornament and draws
 *                                     the portrait only; the cancel path
 *                                     clears the flag so the same-tick
 *                                     redraw draws the wall ornament
 *                                     + portrait.)
 *   - M11_GameView_CancelMirrorCandidate (F0282 C162 cancel path:
 *                                         clears panel state, decrements
 *                                         championCount, sets
 *                                         lastAction="MIRROR" /
 *                                         lastOutcome="CANCELLED", sets
 *                                         inspectTitle="CHAMPION MIRROR" /
 *                                         inspectDetail="SELECTION CANCELLED",
 *                                         refreshes the hash, returns 1).
 *   - M11_GameView_SelectFrontMirrorCandidate (F0280 reopen path:
 *                                              live sensor must return
 *                                              1 after a backtrack).
 *   - M11_GameView_HandleInput (F0359 input dispatch; the BACK
 *                               routing above is the production
 *                               code path for the cancel).
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
    C040_PANEL_X = VIEWPORT_X + 80,
    C040_PANEL_Y = VIEWPORT_Y + 52,
    D1C_PORTRAIT_TOP_VISIBLE_H = C040_PANEL_Y - D1C_PORTRAIT_Y,
    /* Source-locked C026 atlas dimensions. */
    ATLAS_W = 256,
    ATLAS_H = 87,
    /* Ordinal 6 in the C026 atlas: (6 & 7) << 5 = 192, (6 >> 3) * 29 = 0. */
    ORDINAL_6_COL = 6 & 7,         /* = 6 */
    ORDINAL_6_ROW = 6 >> 3,        /* = 0 */
    ORDINAL_6_SRC_X = ORDINAL_6_COL << 5,   /* = 192 */
    ORDINAL_6_SRC_Y = ORDINAL_6_ROW * 29,   /* =  0 */
    /* Side wall sample zones for the no-floating proof. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_WARM_THRESHOLD = 30,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    TARGET_ORDINAL = 6,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 6 (SYRA) for this gate so we can lock the
     * ordinal-6 edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1
};
/* Mirror catalog record name for ordinal 6 (DM1 V1 PC34 mirror
 * catalog).  Used to assert the catalog resolves correctly.
 * SYRA (title CHILD OF NATURE) is the 7th valid mirror text string
 * in the shipped DM1 V1 DUNGEON.DAT. */
static const char kExpectedCatalogName[] = "SYRA";

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

/* Count "warm" pixels in a framebuffer rectangle.  The C026
 * portrait sprites use the warm palette set {0x07, 0x08, 0x09, 0x0A,
 * 0x0B, 0x0E} for skin tones / clothing / backgrounds.  Grey-stone
 * wall texture uses indices 0x01, 0x02, 0x0D.  Counting warm pixels
 * is a coarse but reliable way to distinguish "portrait is here"
 * from "wall only" in the C026 cutout (96, 35, 32, 29). */
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

/* Count opaque pixels in the C026 atlas cell for the requested
 * ordinal.  Used to verify ordinal 6 is a defined portrait in the
 * atlas (i.e. not blank / unused / palette-index-1 transparent
 * only). */
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

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent
 * of pixels that differ. */
static int atlas_cell_distinct_percent(const M11_AssetSlot* portraits,
                                       int ordinalA, int ordinalB) {
    int x, y, compared = 0, different = 0;
    int srcAX = (ordinalA & 7) * D1C_PORTRAIT_W;
    int srcAY = (ordinalA >> 3) * D1C_PORTRAIT_H;
    int srcBX = (ordinalB & 7) * D1C_PORTRAIT_W;
    int srcBY = (ordinalB >> 3) * D1C_PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char a = (unsigned char)
                (portraits->pixels[(srcAY + y) * (int)portraits->width + (srcAX + x)] & 0x0F);
            unsigned char b = (unsigned char)
                (portraits->pixels[(srcBY + y) * (int)portraits->width + (srcBX + x)] & 0x0F);
            ++compared;
            if (a != b) ++different;
        }
    }
    return (compared > 0) ? (different * 100 / compared) : 0;
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

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found. */
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

/* Park the party at the (1,2) D1C front-mirror route facing
 * NORTH.  Same pose as the cancel_reopen and leave_and_reenter
 * probes so the three slices share a common baseline. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = DIR_NORTH;
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
    int ordinal6Opaque;
    int ordinal6Vs5;
    int ordinal6Vs7;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterBacktrack[FB_W * FB_H];
    unsigned char fbAfterReselect[FB_W * FB_H];
    int matchBefore, matchAfterSelect, matchAfterBacktrack, matchAfterReselect;
    int distinctBefore, distinctAfterBacktrack;
    int warmBefore, warmAfterBacktrack;
    int leftSideBefore, leftSideAfterBacktrack;
    int rightSideBefore, rightSideAfterBacktrack;
    int initialCount, countAfterSelect, countAfterBacktrack, countAfterReselect;
    int selectRc, reselectRc;
    M11_GameInputResult backtrackRc;
    int seededSensor;
    char nameBuf[32];
    int nameLookupRc;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-06 / backtrack_same_tick / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    /* Load the C026 portrait atlas via the public M11 helper, so
     * the probe does not depend on the file-scope enum value 26. */
    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* ----------------------------------------------------------------
     * Group A - Atlas math for ordinal 6
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 0 /
     * column 6 and that the math matches COORD.C / DEFS.H:821-826.
     * Identical to Group A in the cancel_reopen and
     * leave_and_reenter probes so the three slices share a
     * common atlas baseline. */
    printf("\n[Group A] C026 atlas math for ordinal 6\n");

    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic id returned by "
                 "M11_GameView_GetV1ChampionPortraitGraphicId = %d)",
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
                 "ordinal 6 col = 6 & 7 = %d (expected 6)",
                 ORDINAL_6_COL);
        CHECK(ORDINAL_6_COL == 6, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 row = 6 >> 3 = %d (expected 0)",
                 ORDINAL_6_ROW);
        CHECK(ORDINAL_6_ROW == 0, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_6_SRC_X, ORDINAL_6_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_6_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_6_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    ordinal6Opaque = atlas_cell_opaque_count(portraits, 6);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal6Opaque);
        CHECK(ordinal6Opaque >= 100, msg);
    }

    ordinal6Vs5 = atlas_cell_distinct_percent(portraits, 6, 5);
    ordinal6Vs7 = atlas_cell_distinct_percent(portraits, 6, 7);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 vs ordinal 5 (left neighbour, ELIJA) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal6Vs5);
        CHECK(ordinal6Vs5 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 6 vs ordinal 7 (right neighbour, SONJA) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal6Vs7);
        CHECK(ordinal6Vs7 >= 30, msg);
    }

    nameBuf[0] = '\0';
    nameLookupRc = M11_GameView_GetMirrorNameByOrdinal(&state,
                                                       TARGET_ORDINAL,
                                                       nameBuf,
                                                       (int)sizeof(nameBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 6 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }

    /* Park the party on the (1,2,0) NORTH-route front mirror, then
     * seed the C127 sensor from HALK (1) to ordinal 6 (SYRA).  Same
     * sensor, same map cell, same draw path - only the G0289 ordinal
     * that DUNVIEW.C:3913-3928 reads through M000_INDEX_TO_ORDINAL
     * (DUNGEON.C:2610-2612) is shifted for the test. */
    park_d1c_front_route(&state);

    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front-mirror ordinal at (1,2,0) = %d (expected "
                 "%d, HALK before seed)",
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }

    seededSensor = seed_first_c127_data(&state,
                                         SHIPPED_HALK_ORDINAL,
                                         TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,2) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

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
                "cannot verify portrait_rect_position or backtrack_same_tick\n",
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
     * Group B - portrait_rect_position baseline
     * ----------------------------------------------------------------
     * Render the framebuffer before any selection, and verify the
     * D1C destination rectangle (96, 35, 32, 29) holds ordinal-6
     * pixels.  Same pose as the cancel_reopen and
     * leave_and_reenter probes' Group B so the three slices
     * share a common baseline. */
    printf("\n[Group B] portrait_rect_position baseline on real C127 sensor pose (1,2,0)=6\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

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
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect is non-empty (>= 100 non-zero "
                 "pixels, got %d)",
                 rect_nonzero(fbBefore,
                              D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                              D1C_PORTRAIT_W, D1C_PORTRAIT_H));
        CHECK(rect_nonzero(fbBefore,
                           D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                           D1C_PORTRAIT_W, D1C_PORTRAIT_H) >= 100, msg);
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

    leftSideBefore = rect_warm_count(fbBefore,
                                     SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                     SIDE_WALL_LEFT_W,
                                     PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "left side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on left wall",
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
                 "pixels (got %d) - portrait not floating on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideBefore);
        CHECK(rightSideBefore < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - backtrack_same_tick: select -> HandleInput(BACK) once
     * ----------------------------------------------------------------
     * Drive the source-locked candidate selection, then a SINGLE
     * call to M11_GameView_HandleInput with M12_MENU_INPUT_BACK
     * (the production BACK key path) and verify the same-tick
     * atomicity:
     *   (C1) HandleInput(BACK) returns M11_GAME_INPUT_REDRAW
     *        (not IGNORED - the BACK key MUST produce a redraw
     *        on the same tick, otherwise the panel would be
     *        unclosable for the user).
     *   (C2) The cancel branch ran in the same tick: panel state
     *        cleared, championCount decremented,
     *        lastAction="MIRROR" / lastOutcome="CANCELLED",
     *        inspectTitle="CHAMPION MIRROR" /
     *        inspectDetail="SELECTION CANCELLED".
     *   (C3) Same-tick redraw uses the wall-ornament + portrait
     *        draw path (not the panel draw path) - so the D1C
     *        destination rectangle (96, 35, 32, 29) carries
     *        ordinal-6 pixels at >= 90% match.  This is the
     *        portrait_rect_position contract for the
     *        backtrack_same_tick slice: the rect must not have
     *        shifted, suppressed, or floated to a different
     *        ordinal after the single-tick cancel.
     *   (C4) The C127 sensor is still alive after the
     *        backtrack: a fresh SelectFrontMirrorCandidate
     *        returns 1 and reopens the panel, proving the
     *        single-tick cancel did not disable the sensor
     *        and the backtrack did not corrupt panel state.
     */
    printf("\n[Group C] backtrack_same_tick: select, single HandleInput(BACK), portrait rect stable\n");

    /* C-prep: clear the status lozenge strings so the same-tick
     * lastAction/lastOutcome and inspectTitle/inspectDetail writes
     * are observable. */
    state.lastAction[0] = '\0';
    state.lastOutcome[0] = '\0';
    state.inspectTitle[0] = '\0';
    state.inspectDetail[0] = '\0';

    /* C0: SelectFrontMirrorCandidate (F0280).  Same precondition
     * as cancel_reopen and leave_and_reenter: panel live, one
     * champion in slot 0. */
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterSelect = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate on (1,2,0) returns 1 (got %d)",
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

    /* Render with C040 panel live.  The portrait must NOT be drawn
     * as a stale floating sprite while the panel owns the view
     * (BUG-120/121 panel guard).  Match against ordinal 6 should
     * be low. */
    memset(fbAfterSelect, 0, sizeof(fbAfterSelect));
    M11_GameView_Draw(&state, fbAfterSelect, FB_W, FB_H);
    matchAfterSelect = match_portrait_at_rect(portraits,
                                              fbAfterSelect,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw does not leave ordinal %d as a stale "
                 "full-D1C sprite (<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterSelect);
        CHECK(matchAfterSelect <= 20, msg);
    }

    /* C1: SINGLE call to M11_GameView_HandleInput with
     * M12_MENU_INPUT_BACK.  The backtrack_same_tick constraint:
     * the cancel must be driven through the input dispatch path
     * (the production BACK key), not through the direct
     * M11_GameView_CancelMirrorCandidate API.  This is the
     * differentiator from the cancel_reopen slice. */
    backtrackRc = M11_GameView_HandleInput(&state, M12_MENU_INPUT_BACK);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "HandleInput(BACK) on panel-live (1,2,0) returns "
                 "M11_GAME_INPUT_REDRAW (got %d, expected %d) - the "
                 "BACK key produces a redraw on the same tick, "
                 "not IGNORED",
                 (int)backtrackRc, (int)M11_GAME_INPUT_REDRAW);
        CHECK(backtrackRc == M11_GAME_INPUT_REDRAW, msg);
    }

    /* C2: same-tick cancel state.  All of these writes must have
     * landed in the SAME tick as the HandleInput call.  The
     * source-locked M11_GameView_CancelMirrorCandidate runs the
     * F0282 C162 branch which sets the panel state, the
     * championCount, the status strings, and the inspect
     * readout.  A regression that split the cancel into two
     * ticks (e.g. queue the cancel and run it on the next
     * HandleInput call) would fail these checks because the
     * HandleInput return would be REDRAW but the state would
     * not yet be cleared. */
    countAfterBacktrack = state.world.party.championCount;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "same-tick cancel: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before select) - panel "
                 "cleared and champion decremented on the BACK tick",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterBacktrack, initialCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterBacktrack == initialCount, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick status: lastAction=\"%s\" (expected \"MIRROR\"), "
                 "lastOutcome=\"%s\" (expected \"CANCELLED\")",
                 state.lastAction, state.lastOutcome);
        CHECK(strcmp(state.lastAction, "MIRROR") == 0 &&
              strcmp(state.lastOutcome, "CANCELLED") == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "same-tick inspect readout: inspectTitle=\"%s\" "
                 "(expected \"CHAMPION MIRROR\"), inspectDetail=\"%s\" "
                 "(expected \"SELECTION CANCELLED\")",
                 state.inspectTitle, state.inspectDetail);
        CHECK(strcmp(state.inspectTitle, "CHAMPION MIRROR") == 0 &&
              strcmp(state.inspectDetail, "SELECTION CANCELLED") == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick inventory: inventoryPanelActive=%d (expected 0) "
                 "- the C162 cancel branch closes any open inventory "
                 "panel atomically with the cancel",
                 state.inventoryPanelActive);
        CHECK(state.inventoryPanelActive == 0, msg);
    }

    /* C3: same-tick redraw.  The panel guard in
     * m11_draw_dm1_front_mirror_route keys off
     * candidateMirrorPanelActive; once the cancel has cleared
     * that flag in the same tick, the very next redraw uses the
     * wall-ornament + portrait draw path.  Pixel-prove the D1C
     * destination rectangle (96, 35, 32, 29) carries ordinal-6
     * pixels at >= 90% match - this is the
     * portrait_rect_position contract for the backtrack_same_tick
     * slice. */
    memset(fbAfterBacktrack, 0, sizeof(fbAfterBacktrack));
    M11_GameView_Draw(&state, fbAfterBacktrack, FB_W, FB_H);
    matchAfterBacktrack = match_portrait_at_rect(portraits,
                                                 fbAfterBacktrack,
                                                 TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw: D1C portrait rect carries ordinal "
                 "%d pixels at >= 90%% match (got %d%%) - the BACK "
                 "key's same-tick redraw used the wall-ornament + "
                 "portrait draw path, not the panel-guard path",
                 TARGET_ORDINAL, matchAfterBacktrack);
        CHECK(matchAfterBacktrack >= 90, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw: D1C portrait rect is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 rect_nonzero(fbAfterBacktrack,
                              D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                              D1C_PORTRAIT_W, D1C_PORTRAIT_H));
        CHECK(rect_nonzero(fbAfterBacktrack,
                           D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                           D1C_PORTRAIT_W, D1C_PORTRAIT_H) >= 100, msg);
    }
    distinctAfterBacktrack = rect_distinct(fbAfterBacktrack,
                                           D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                           D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw: D1C portrait rect has >= "
                 "%d distinct palette indices (got %d) - same "
                 "portrait baseline as before the backtrack",
                 distinctBefore, distinctAfterBacktrack);
        CHECK(distinctAfterBacktrack >= distinctBefore - 1, msg);
    }
    warmAfterBacktrack = rect_warm_count(fbAfterBacktrack,
                                         D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                         D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw: D1C portrait rect has >= "
                 "%d warm pixels (got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmAfterBacktrack);
        CHECK(warmAfterBacktrack >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof on the same-tick redraw: side walls of
     * the D1C portrait band must NOT carry the portrait's warm
     * pixels.  Same thresholds as the Group B baseline so a
     * regression where the portrait floats onto the side walls
     * during the same-tick redraw is caught. */
    leftSideAfterBacktrack = rect_warm_count(fbAfterBacktrack,
                                             SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                             SIDE_WALL_LEFT_W,
                                             PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw: left side wall of D1C portrait "
                 "band has < %d warm pixels (got %d) - no floating "
                 "portrait on the left wall after the BACK cancel",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterBacktrack);
        CHECK(leftSideAfterBacktrack < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideAfterBacktrack = rect_warm_count(fbAfterBacktrack,
                                              SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                              SIDE_WALL_RIGHT_W,
                                              PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw: right side wall of D1C portrait "
                 "band has < %d warm pixels (got %d) - no floating "
                 "portrait on the right wall after the BACK cancel",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterBacktrack);
        CHECK(rightSideAfterBacktrack < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* C4: sensor still alive.  A fresh SelectFrontMirrorCandidate
     * after the same-tick cancel must return 1 (sensor alive,
     * panel can be reopened).  This proves the BACK-routed
     * cancel did not disable the C127 sensor (which would lock
     * the player out of the mirror) and did not corrupt the
     * panel state. */
    reselectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterReselect = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-backtrack reselect: SelectFrontMirrorCandidate "
                 "on (1,2,0) returns 1 (got %d) - C127 sensor still "
                 "alive after the same-tick cancel",
                 reselectRc);
        CHECK(reselectRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "post-backtrack reselect: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before first select)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterReselect, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              countAfterReselect == initialCount + 1, msg);
    }

    /* Render with the C040 panel live again (post-reselect).  The
     * portrait must NOT be drawn as a stale floating sprite
     * (BUG-120/121 panel guard re-engaged on the re-select
     * framebuffer). */
    memset(fbAfterReselect, 0, sizeof(fbAfterReselect));
    M11_GameView_Draw(&state, fbAfterReselect, FB_W, FB_H);
    matchAfterReselect = match_portrait_at_rect(portraits,
                                                fbAfterReselect,
                                                TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-backtrack reselect: panel-on redraw does not "
                 "leave ordinal %d as a stale full-D1C sprite "
                 "(<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterReselect);
        CHECK(matchAfterReselect <= 20, msg);
    }

    /* The portrait_rect_position contract across the
     * backtrack_same_tick sequence: the D1C destination rectangle
     * (96, 35, 32, 29) does NOT change screen position, and the
     * same rect lines up with ordinal-6 pixels when the panel is
     * closed (before any select, after the same-tick BACK cancel)
     * and is suppressed as a stale sprite while the panel is live
     * (after select, after reselect).  The single HandleInput
     * call with BACK is the differentiator from cancel_reopen
     * (which used the direct API) and leave_and_reenter (which
     * used the direct API + movement). */
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position across backtrack_same_tick: "
                 "before=%d%%, after-select=%d%%, after-backtrack=%d%%, "
                 "after-reselect=%d%% (panel-off >=90, panel-on <=20) - "
                 "single HandleInput(BACK) tick restores ordinal %d "
                 "pixels at the same D1C rectangle",
                 matchBefore, matchAfterSelect,
                 matchAfterBacktrack, matchAfterReselect,
                 TARGET_ORDINAL);
        CHECK(matchBefore >= 90 &&
              matchAfterSelect <= 20 &&
              matchAfterBacktrack >= 90 &&
              matchAfterReselect <= 20, msg);
    }

    /* Cross-check: the same-tick redraw must match the panel-off
     * baseline drawn before the select.  The wall-ornament + portrait
     * draw path is the same code in both cases (BUG-120/121 panel
     * guard keying off candidateMirrorPanelActive), so the D1C
     * rectangle's non-zero count should be close to the baseline.
     * A regression that left a stale sprite or shifted the rect
     * during the same-tick redraw would inflate the count. */
    {
        int offBaseline = rect_nonzero(fbBefore,
                                       D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                       D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        int backtrackNonzero = rect_nonzero(fbAfterBacktrack,
                                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                            D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        int diff = (backtrackNonzero > offBaseline)
                       ? (backtrackNonzero - offBaseline)
                       : (offBaseline - backtrackNonzero);
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "same-tick redraw non-zero pixel count is stable vs "
                 "panel-off baseline (off=%d, after-backtrack=%d, "
                 "|diff|=%d <= 100) - portrait rect did not shift, "
                 "suppress, or float during the BACK cancel",
                 offBaseline, backtrackNonzero, diff);
        CHECK(diff <= 100, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
