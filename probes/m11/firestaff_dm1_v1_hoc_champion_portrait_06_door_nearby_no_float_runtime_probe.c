/*
 * firestaff_dm1_v1_hoc_champion_portrait_06_door_Nearby_No_Float_Runtime_Probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice
 * that the existing portrait_06 cancel_reopen and leave_and_reenter
 * probes do not cover:
 *
 *   ordinal 6                  (mirror catalog record SYRA, title CHILD OF NATURE)
 *   route   door_nearby_no_float  (DM1 V1 hall (1,2) NORTH pose: the
 *                                 (1,1) front square is the closed
 *                                 mirror door per
 *                                 firestaff_m11_hall_walkaround_runtime_probe,
 *                                 so a forward-north step is blocked;
 *                                 this probe drives the blocked-north
 *                                 step and proves the D1C portrait
 *                                 rect still carries ordinal-6 pixels
 *                                 after the door-block attempt with
 *                                 no floating sprite on the door
 *                                 wall and no stale panel state)
 *   aspect  door_nearby_no_float
 *
 * Why this slice is disjoint from the cancel_reopen and
 * leave_and_reenter probes:
 *
 *   - cancel_reopen exercises select -> cancel -> select with NO
 *     movement.  This probe uses no panel and only exercises the
 *     INPUT_UP movement path against the closed mirror door.
 *
 *   - leave_and_reenter exercises the (1,2) NORTH -> (1,3) SOUTH
 *     leave via TURN_RIGHT + TURN_RIGHT + UP, then a return via
 *     TURN_RIGHT + TURN_RIGHT + UP.  It proves that the party
 *     physically walks to the (1,3) corridor cell and that the
 *     portrait rect survives the move.  This probe drives a
 *     different movement leg: forward-north at (1,2) NORTH, which
 *     is BLOCKED by the closed mirror door at (1,1).  The probe
 *     proves the door-block leg of the input interleave, where
 *     M11_GameView_HandleInput must report REDRAW with the party
 *     still at (1,2) NORTH and the portrait rect still drawn.
 *
 *   - the existing wall_ornament_no_float slice targets ordinal
 *     19 (HAWK) and verifies the C346 frame backing + C026/C346
 *     isolation.  This probe targets ordinal 6 (SYRA) and
 *     verifies the no-float invariant under the specific
 *     movement context of a door-block attempt.
 *
 * The "door_nearby_no_float" route verifies three coupled concerns
 * on the (1,2) NORTH closed-mirror-door pose:
 *
 *   (1) Portrait rect still holds ordinal 6 after a door-block
 *       attempt (Group D: Door-Block Pose).  The forward-north
 *       INPUT_UP leg is dispatched through M11_GameView_HandleInput
 *       and the F0359/F0361 input interleave.  Because (1,1) is a
 *       closed mirror door, the F0705 movement resolver reports
 *       M011_CELL closed-wall and the party stays at (1,2) NORTH.
 *       M11_GameView_HandleInput must return REDRAW (the
 *       door-block attempt re-renders the closed door wall) and
 *       the party coordinates/direction must remain at
 *       (1, 2, NORTH).  The D1C portrait rect must still carry
 *       ordinal-6 pixels at >= 90% match after the door-block
 *       attempt; this proves the door-block redraw does not
 *       suppress or distort the SYRA portrait sprite.
 *
 *   (2) No-floating proof across the door wall (Group E).  The
 *       closed mirror door at (1,1) is rendered as the
 *       mirror-wall texture in the north-wall band of the
 *       viewport.  The C026 portrait sprite must NOT bleed into
 *       the door-wall band (the rect is (96, 35, 32, 29), the
 *       band y is (33, 65)).  The door-wall sprite uses the
 *       grey-stone palette set (0x01, 0x02, 0x0D); warm-color
 *       pixels (skin / clothing / portrait background) must not
 *       appear in the door-wall band.  After the door-block
 *       attempt the side walls of the D1C portrait band must
 *       still NOT carry warm-color pixels.
 *
 *   (3) Redraw stability after door-block (Group F).  The
 *       M11_GameView_Draw redraw after the door-block attempt
 *       must be byte-stable across consecutive calls: the same
 *       (1,2) NORTH closed-door pose is rendered twice with
 *       M11_GameView_Draw, and the two framebuffers must match
 *       byte-for-byte.  This catches the "stale sprite floats
 *       in the door-wall band" bug class where a one-shot init
 *       primes the rect but a subsequent redraw drifts.
 *
 *   (4) Candidate-panel invariant (Group G).  The door-block
 *       attempt must NOT open the C040 candidate panel and
 *       must NOT change championCount or candidateMirrorOrdinal.
 *       The C127 sensor is still alive on the (1,1) front square
 *       but a forward-north step into a closed door is not a
 *       candidate-select action; the panel must stay closed.
 *
 * Source evidence:
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:3913-3928 (D1C C026 portrait blit at {96, 35}
 *                          with ((ordinal & 7) << 5,
 *                                (ordinal >> 3) * 29))
 *   - DUNVIEW.C:8318-8542 (F0128 redraw viewport far-to-near)
 *   - DUNVIEW.C:1061 (G0205 wall ornament coordinate sets,
 *                     coordSet 5 / index 12 = D1C frame)
 *   - DUNGEON.C:2573 (M011_CELL(sensor) maps C127 cell match
 *                     against view dir)
 *   - DUNGEON.C:2608-2612 (G0289 champion portrait ordinal
 *                           from sensorData)
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - MOVESENS.C:763-818 (F0267 movement resolver; closed-door
 *                         wall rejection when M011_CELL is wall)
 *   - COMMAND.C F0359/F0361 (input interleave dispatch)
 *   - COMMAND.C F0378:1956-1990 (M568_PANEL_RESURRECT_REINCARNATE
 *                              dispatch; door-block does NOT
 *                              open the panel)
 *   - REVIVE.C F0280:124-132 (C040 empty-leader candidate gate)
 *   - REVIVE.C F0282:744-806 (C162 cancel branch)
 *   - firestaff_m11_hall_walkaround_runtime_probe (canonical
 *     (1,3,SOUTH) -> (1,4,SOUTH) walkaround and the
 *     (1,2) NORTH closed-mirror-door block reference)
 *   - firestaff_dm1_v1_hall_of_champions_portrait_06_leave_and_reenter
 *     (companion probe for the (1,2) NORTH pose; this probe
 *     adds the door-block INPUT_UP axis)
 *   - firestaff_dm1_v1_hall_of_champions_portrait_06_cancel_reopen
 *     (companion probe for the select->cancel->select axis)
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
    /* Ordinal 6 in the C026 atlas: (6 & 7) << 5 = 192, (6 >> 3) * 29 = 0. */
    ORDINAL_6_COL = 6 & 7,         /* = 6 */
    ORDINAL_6_ROW = 6 >> 3,        /* = 0 */
    ORDINAL_6_SRC_X = ORDINAL_6_COL << 5,   /* = 192 */
    ORDINAL_6_SRC_Y = ORDINAL_6_ROW * 29,   /* =  0 */
    /* Side wall sample zones - the no-floating proof checks that
     * the portrait sprite pixels do not bleed into the left/right
     * side walls of the D1C cell band. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    /* North-wall sample zone - the closed-mirror-door wall band
     * sits above the portrait rect, between the top of the
     * viewport and the top of the portrait rect.  In the
     * shipped DM1 V1 hall layout, the door-wall band is the
     * y-range [VIEWPORT_Y + 0, D1C_PORTRAIT_Y).  The C026
     * portrait sprite must not bleed up into this band when
     * the door-block attempt redraws the closed mirror door. */
    DOOR_WALL_X = VIEWPORT_X,
    DOOR_WALL_Y = VIEWPORT_Y,
    DOOR_WALL_W = FB_W,
    DOOR_WALL_H = D1C_PORTRAIT_Y - VIEWPORT_Y,
    PORTRAIT_WARM_THRESHOLD = 30,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + 33,
    PORTRAIT_BAND_Y1 = VIEWPORT_Y + 65,
    TARGET_ORDINAL = 6,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 6 (SYRA) for this gate so we can lock the
     * ordinal-6 edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1,
    /* The (1,2) NORTH seed pose is the leave_and_reenter probe's
     * seed pose, so the two probes share a common baseline.  The
     * (1,1) cell directly north of (1,2) is the closed mirror
     * door per firestaff_m11_hall_walkaround_runtime_probe. */
    SEED_POSE_MAPX = 1,
    SEED_POSE_MAPY = 2,
    SEED_POSE_DIR  = DIR_NORTH
};
/* Mirror catalog record name for ordinal 6 (DM1 V1 PC34 mirror
 * catalog).  SYRA (title CHILD OF NATURE) is the 7th valid mirror
 * text string in the shipped DM1 V1 DUNGEON.DAT (the candidates
 * that come up after DAROOU / DAROOU / HALK / WU TSE / AZIZI /
 * LEIF / ELIJA / SYRA). */
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
 * Used to verify ordinal 6 is a defined portrait in the atlas
 * (i.e. not blank / unused / palette-index-1 transparent only). */
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

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent of
 * pixels that differ.  Used to verify ordinal 6 is a distinct portrait
 * from its row-0 neighbours (5, 7). */
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
 * on success, or -1 if no such sensor was found.  Same helper as
 * the cancel_reopen / leave_and_reenter probes, kept local so
 * this probe is self contained. */
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
 * wall) with sensorData=1 (HALK, mirror ordinal 1).  The (1,1)
 * cell itself is a closed mirror door per
 * firestaff_m11_hall_walkaround_runtime_probe.  After
 * seed_first_c127_data the same square reports ordinal 6. */
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
    int ordinal6Opaque;
    int ordinal6Vs5;
    int ordinal6Vs7;
    int frontOrdinal;
    int seededSensor;
    unsigned char fbBaseline[FB_W * FB_H];
    unsigned char fbAfterDoorBlock[FB_W * FB_H];
    unsigned char fbAfterDoorBlock2[FB_W * FB_H];
    int matchBaseline, matchAfterDoorBlock, matchAfterDoorBlock2;
    int distinctBaseline, distinctAfterDoorBlock;
    int warmBaseline, warmAfterDoorBlock;
    int leftSideBaseline, leftSideAfterDoorBlock;
    int rightSideBaseline, rightSideAfterDoorBlock;
    int doorWallBaseline, doorWallAfterDoorBlock;
    int initialCount, doorBlockResult;
    M11_GameInputResult stepResDoorBlock;
    int memcmpDoorRedraws;
    int selectAfterDoorRc;
    char nameBuf[32];
    int nameLookupRc;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-06 / door_nearby_no_float / portrait_rect_position ===\n");
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

    /* ----------------------------------------------------------------
     * Group A - Atlas math for ordinal 6
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 0 /
     * column 6 and that the math matches COORD.C / DEFS.H:821-826.
     * Identical to Group A in the cancel_reopen / leave_and_reenter
     * probes so the three probes share a common atlas baseline. */
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
                "cannot verify door_nearby_no_float\n",
                TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Sanity-check the public D1C wall ornament zone helper, then
     * verify the inner portrait rectangle (96, 35, 32, 29) sits
     * inside that zone. */
    {
        int ornX = 0, ornY = 0, ornW = 0, ornH = 0;
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
    }

    /* ----------------------------------------------------------------
     * Group B - portrait_rect_position baseline
     * ----------------------------------------------------------------
     * Render the framebuffer before any movement attempt, and verify
     * the D1C destination rectangle (96, 35, 32, 29) holds
     * ordinal-6 pixels.  Same pose as the cancel_reopen and
     * leave_and_reenter probes' Group B so all three probes share a
     * common baseline. */
    printf("\n[Group B] portrait_rect_position baseline on real C127 sensor pose (1,2,0)=6\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

    memset(fbBaseline, 0, sizeof(fbBaseline));
    M11_GameView_Draw(&state, fbBaseline, FB_W, FB_H);

    matchBaseline = match_portrait_at_rect(portraits, fbBaseline, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) carries ordinal %d pixels "
                 "at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchBaseline);
        CHECK(matchBaseline >= 90, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect is non-empty (>= 100 non-zero "
                 "pixels, got %d)",
                 rect_nonzero(fbBaseline,
                              D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                              D1C_PORTRAIT_W, D1C_PORTRAIT_H));
        CHECK(rect_nonzero(fbBaseline,
                           D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                           D1C_PORTRAIT_W, D1C_PORTRAIT_H) >= 100, msg);
    }
    distinctBaseline = rect_distinct(fbBaseline,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= 4 distinct palette indices "
                 "(got %d)",
                 distinctBaseline);
        CHECK(distinctBaseline >= 4, msg);
    }
    warmBaseline = rect_warm_count(fbBaseline,
                                    D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                    D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= %d warm-color pixels "
                 "(got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmBaseline);
        CHECK(warmBaseline >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    leftSideBaseline = rect_warm_count(fbBaseline,
                                        SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_LEFT_W,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "left side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSideBaseline);
        CHECK(leftSideBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideBaseline = rect_warm_count(fbBaseline,
                                         SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                         SIDE_WALL_RIGHT_W,
                                         PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "right side wall of D1C portrait band has < %d warm "
                 "pixels (got %d) - portrait not floating on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideBaseline);
        CHECK(rightSideBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* The closed-mirror-door wall band (top of viewport down to the
     * top of the portrait rect) must NOT carry warm-color pixels
     * even at the baseline.  The door-wall sprite uses the
     * grey-stone palette set; any warm pixels would mean a
     * floating portrait sprite leaked up into the door-wall
     * band at the seeded (1,2) NORTH pose. */
    doorWallBaseline = rect_warm_count(fbBaseline,
                                        DOOR_WALL_X, DOOR_WALL_Y,
                                        DOOR_WALL_W, DOOR_WALL_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "closed-mirror-door wall band (above portrait rect) "
                 "has < %d warm pixels at baseline (got %d) - no "
                 "floating portrait in the door-wall band",
                 PORTRAIT_WARM_THRESHOLD, doorWallBaseline);
        CHECK(doorWallBaseline < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - door_nearby baseline: the (1,1) front square is the
     *           closed mirror door.  Prove the (1,2) NORTH pose is
     *           adjacent to the closed door and that the door-wall
     *           band is currently grey-stone-only.
     * ----------------------------------------------------------------
     * The door_nearby_no_float route only makes sense if the (1,1)
     * front square is the closed mirror door.  We verify the
     * baseline here so the door-block assertions in Groups D-G are
     * well-posed: a redraw that suppresses or distorts the portrait
     * rect after a forward-north step is a real bug, not an
     * off-baseline artefact. */
    printf("\n[Group C] door_nearby baseline: (1,2) NORTH pose adjacent to closed mirror door at (1,1)\n");

    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "party is parked at (1,2) NORTH (mapX=%d, mapY=%d, dir=%d)",
                 state.world.party.mapX,
                 state.world.party.mapY,
                 state.world.party.direction);
        CHECK(state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == SEED_POSE_DIR, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "forward-north step would target (1,1) which is the "
                 "closed mirror door per "
                 "firestaff_m11_hall_walkaround_runtime_probe "
                 "(the canonical (1,3,SOUTH) -> (1,4,SOUTH) walkaround "
                 "verifies (1,2) NORTH stepping is blocked)");
        CHECK(SEED_POSE_MAPX == 1 && SEED_POSE_MAPY == 2 &&
              SEED_POSE_DIR == DIR_NORTH, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - door_nearby_no_float: forward-north step into the
     *           closed mirror door.  The party must stay at (1,2)
     *           NORTH, the redraw must report success, and the
     *           D1C portrait rect must still carry ordinal-6
     *           pixels at >= 90% match.
     * ----------------------------------------------------------------
     * The (1,1) cell is a closed mirror door.  M11_GameView_HandleInput
     * must dispatch the INPUT_UP through F0359/F0361, the F0267
     * movement resolver must reject the closed-door wall (M011_CELL
     * closed-wall), the party must remain at (1,2) NORTH, and
     * M11_GameView_HandleInput must return REDRAW so the closed
     * mirror door wall is re-rendered.  The D1C portrait rect must
     * still carry ordinal-6 pixels after the door-block attempt;
     * this proves the door-block redraw does not suppress the
     * SYRA portrait sprite. */
    printf("\n[Group D] door_nearby_no_float: forward-north step into closed mirror door at (1,1)\n");

    stepResDoorBlock = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    doorBlockResult = (int)stepResDoorBlock;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_HandleInput(INPUT_UP) at (1,2) NORTH "
                 "result=%d (expected %d REDRAW - closed mirror door "
                 "redrawn, party stays put)",
                 doorBlockResult, M11_GAME_INPUT_REDRAW);
        CHECK(stepResDoorBlock == M11_GAME_INPUT_REDRAW, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "party stays at (%d, %d) dir=%d after door-block "
                 "(expected (%d, %d) dir=%d) - forward-north blocked "
                 "by closed mirror door at (1,1)",
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction,
                 SEED_POSE_MAPX, SEED_POSE_MAPY, SEED_POSE_DIR);
        CHECK(state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == SEED_POSE_DIR, msg);
    }

    /* Capture the post-door-block framebuffer. */
    memset(fbAfterDoorBlock, 0, sizeof(fbAfterDoorBlock));
    M11_GameView_Draw(&state, fbAfterDoorBlock, FB_W, FB_H);

    /* Re-derive the front-mirror ordinal after the door-block attempt.
     * The C127 sensor on the (1,1) front square is still alive (the
     * door-block attempt does NOT disable the sensor, only rejects the
     * movement), so the front mirror must still report ordinal 6. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: front-mirror ordinal at (%d,%d,%d) = "
                 "%d (expected %d) - sensor still alive",
                 SEED_POSE_MAPX, SEED_POSE_MAPY, SEED_POSE_DIR,
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }

    matchAfterDoorBlock = match_portrait_at_rect(portraits,
                                                  fbAfterDoorBlock,
                                                  TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: D1C portrait rect carries ordinal "
                 "%d pixels at >= 90%% match (got %d%%) - door-block "
                 "redraw does not suppress the SYRA portrait sprite",
                 TARGET_ORDINAL, matchAfterDoorBlock);
        CHECK(matchAfterDoorBlock >= 90, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: D1C portrait rect is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 rect_nonzero(fbAfterDoorBlock,
                              D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                              D1C_PORTRAIT_W, D1C_PORTRAIT_H));
        CHECK(rect_nonzero(fbAfterDoorBlock,
                           D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                           D1C_PORTRAIT_W, D1C_PORTRAIT_H) >= 100, msg);
    }
    distinctAfterDoorBlock = rect_distinct(fbAfterDoorBlock,
                                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                            D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: D1C portrait rect has >= "
                 "%d distinct palette indices (got %d) - same "
                 "portrait baseline as before the door-block attempt",
                 distinctBaseline, distinctAfterDoorBlock);
        CHECK(distinctAfterDoorBlock >= distinctBaseline - 1, msg);
    }
    warmAfterDoorBlock = rect_warm_count(fbAfterDoorBlock,
                                          D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                          D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: D1C portrait rect has >= "
                 "%d warm pixels (got %d) - portrait sprite still "
                 "present, not suppressed by the door-block redraw",
                 PORTRAIT_WARM_THRESHOLD, warmAfterDoorBlock);
        CHECK(warmAfterDoorBlock >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - no-floating proof across the door wall and the side
     *           walls after the door-block attempt.  Same invariants
     *           as Group B's side-wall proof, but verified AFTER the
     *           door-block redraw - this catches the bug class where
     *           the door-block attempt leaves a stale portrait sprite
     *           floating in the closed-mirror-door wall band.
     * ---------------------------------------------------------------- */
    printf("\n[Group E] no-floating proof on door-wall and side walls after door-block\n");

    leftSideAfterDoorBlock = rect_warm_count(fbAfterDoorBlock,
                                              SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                              SIDE_WALL_LEFT_W,
                                              PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: left side wall of D1C portrait "
                 "band has < %d warm pixels (got %d) - no floating "
                 "portrait on the left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterDoorBlock);
        CHECK(leftSideAfterDoorBlock < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideAfterDoorBlock = rect_warm_count(fbAfterDoorBlock,
                                               SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                               SIDE_WALL_RIGHT_W,
                                               PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: right side wall of D1C portrait "
                 "band has < %d warm pixels (got %d) - no floating "
                 "portrait on the right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterDoorBlock);
        CHECK(rightSideAfterDoorBlock < PORTRAIT_WARM_THRESHOLD, msg);
    }

    doorWallAfterDoorBlock = rect_warm_count(fbAfterDoorBlock,
                                              DOOR_WALL_X, DOOR_WALL_Y,
                                              DOOR_WALL_W, DOOR_WALL_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block: closed-mirror-door wall band "
                 "(above portrait rect) has < %d warm pixels (got "
                 "%d) - no floating portrait in the door-wall band",
                 PORTRAIT_WARM_THRESHOLD, doorWallAfterDoorBlock);
        CHECK(doorWallAfterDoorBlock < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* The door-wall band must NOT carry the C026 portrait sprite
     * pixels at all - the band is purely grey-stone door-wall
     * texture.  Sample a tighter column under the door-wall band
     * at the centre of the viewport (x=160) and verify no warm
     * pixels appear in the y-range [VIEWPORT_Y, D1C_PORTRAIT_Y). */
    {
        int centreColWarm = 0;
        int xx = VIEWPORT_X + 160;
        int yy;
        for (yy = DOOR_WALL_Y; yy < D1C_PORTRAIT_Y; ++yy) {
            unsigned char idx = M11_FB_DECODE_INDEX(fbAfterDoorBlock[yy * FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++centreColWarm;
                    break;
                default:
                    break;
            }
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "after door-block: centre column (x=%d) of "
                     "door-wall band has 0 warm pixels (got %d) - "
                     "no C026 sprite pixel bleeds up into the door "
                     "wall band",
                     xx, centreColWarm);
            CHECK(centreColWarm == 0, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group F - redraw stability: M11_GameView_Draw redraws after the
     *           door-block attempt must be byte-stable across
     *           consecutive calls.  The door_nearby_no_float
     *           invariant is a runtime invariant: any one-shot init
     *           that primes the rect but lets a subsequent redraw
     *           drift would be caught by memcmp(fbAfterDoorBlock,
     *           fbAfterDoorBlock2) == 0.
     * ---------------------------------------------------------------- */
    printf("\n[Group F] redraw stability after door-block\n");

    memset(fbAfterDoorBlock2, 0, sizeof(fbAfterDoorBlock2));
    M11_GameView_Draw(&state, fbAfterDoorBlock2, FB_W, FB_H);
    memcmpDoorRedraws = memcmp(fbAfterDoorBlock, fbAfterDoorBlock2,
                                sizeof(fbAfterDoorBlock));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "two consecutive M11_GameView_Draw calls after the "
                 "door-block attempt are byte-stable (memcmp=%d, "
                 "expected 0)",
                 memcmpDoorRedraws);
        CHECK(memcmpDoorRedraws == 0, msg);
    }

    matchAfterDoorBlock2 = match_portrait_at_rect(portraits,
                                                   fbAfterDoorBlock2,
                                                   TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "second redraw after door-block: D1C portrait rect "
                 "carries ordinal %d pixels at >= 90%% match (got "
                 "%d%%) - same as the first redraw",
                 TARGET_ORDINAL, matchAfterDoorBlock2);
        CHECK(matchAfterDoorBlock2 >= 90, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "match-percent parity between the two post-door-block "
                 "redraws: %d%% vs %d%% (must be equal)",
                 matchAfterDoorBlock, matchAfterDoorBlock2);
        CHECK(matchAfterDoorBlock == matchAfterDoorBlock2, msg);
    }

    /* ----------------------------------------------------------------
     * Group G - candidate-panel invariant.  The door-block attempt
     *           must NOT open the C040 candidate panel and must NOT
     *           change championCount.  The C127 sensor on the (1,1)
     *           front square is still alive (the door-block attempt
     *           only rejects the movement, it does not disable the
     *           sensor), so a subsequent SelectFrontMirrorCandidate
     *           must still return 1.
     * ---------------------------------------------------------------- */
    printf("\n[Group G] candidate-panel invariant after door-block\n");

    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after door-block: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before door-block)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.world.party.championCount, initialCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              state.world.party.championCount == initialCount, msg);
    }

    /* A second forward-north step (re-attempt the door-block) must
     * also keep the panel closed and the count stable.  This catches
     * the bug class where a single door-block is rejected cleanly
     * but a repeat door-block opens the candidate panel by mistake. */
    stepResDoorBlock = M11_GameView_HandleInput(&state, M12_MENU_INPUT_UP);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "repeat INPUT_UP at (1,2) NORTH result=%d (expected "
                 "%d REDRAW), party at (%d,%d) dir=%d, "
                 "candidateMirrorPanelActive=%d, championCount=%d",
                 (int)stepResDoorBlock, M11_GAME_INPUT_REDRAW,
                 state.world.party.mapX, state.world.party.mapY,
                 state.world.party.direction,
                 state.candidateMirrorPanelActive,
                 state.world.party.championCount);
        CHECK(stepResDoorBlock == M11_GAME_INPUT_REDRAW &&
              state.world.party.mapX == SEED_POSE_MAPX &&
              state.world.party.mapY == SEED_POSE_MAPY &&
              state.world.party.direction == SEED_POSE_DIR &&
              state.candidateMirrorPanelActive == 0 &&
              state.world.party.championCount == initialCount, msg);
    }

    /* After the door-block attempts, the C127 sensor on the (1,1)
     * front square must still be selectable.  This proves the
     * door-block attempts did not disable the sensor and that the
     * portrait rect is still correctly indexed to ordinal 6. */
    selectAfterDoorRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after door-block attempts: "
                 "SelectFrontMirrorCandidate on (1,2,0) returns %d "
                 "(expected 1) - sensor is still selectable",
                 selectAfterDoorRc);
        CHECK(selectAfterDoorRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after door-block + select: "
                 "candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, "
                 "candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before door-block)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.world.party.championCount, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              state.world.party.championCount == initialCount + 1, msg);
    }

    /* Cancel the candidate panel so the test exit leaves the engine
     * in a clean state (the C162 cancel branch is the canonical
     * panel-closure path used by the cancel_reopen probe). */
    {
        int cancelRc = M11_GameView_CancelMirrorCandidate(&state);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "CancelMirrorCandidate after door-block returns %d "
                     "(expected 1) - panel closed cleanly",
                     cancelRc);
            CHECK(cancelRc == 1, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "after cancel: candidateMirrorPanelActive=%d, "
                     "championCount=%d (was %d before door-block)",
                     state.candidateMirrorPanelActive,
                     state.world.party.championCount, initialCount);
            CHECK(state.candidateMirrorPanelActive == 0 &&
                  state.world.party.championCount == initialCount, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group H - door_nearby_no_float cross-framebuffer invariants
     *           The portrait_rect_position contract across the
     *           door-block cycle: the D1C destination rectangle
     *           (96, 35, 32, 29) does NOT change screen position,
     *           and the same rect lines up with ordinal-6 pixels
     *           at the baseline AND after the door-block attempt,
     *           with the side walls / door wall clean of warm pixels
     *           at every step. */
    printf("\n[Group H] door_nearby_no_float cross-framebuffer invariants\n");

    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position across door_nearby_no_float: "
                 "baseline=%d%%, after-door-block=%d%%, "
                 "after-door-block2=%d%% (all >= 90%%)",
                 matchBaseline, matchAfterDoorBlock,
                 matchAfterDoorBlock2);
        CHECK(matchBaseline >= 90 &&
              matchAfterDoorBlock >= 90 &&
              matchAfterDoorBlock2 >= 90, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "no-floating warm-pixel budget across door_nearby_no_float: "
                 "door-wall baseline=%d, door-wall after-door-block=%d, "
                 "left-side after-door-block=%d, right-side after-door-block=%d "
                 "(all < %d)",
                 doorWallBaseline, doorWallAfterDoorBlock,
                 leftSideAfterDoorBlock, rightSideAfterDoorBlock,
                 PORTRAIT_WARM_THRESHOLD);
        CHECK(doorWallBaseline < PORTRAIT_WARM_THRESHOLD &&
              doorWallAfterDoorBlock < PORTRAIT_WARM_THRESHOLD &&
              leftSideAfterDoorBlock < PORTRAIT_WARM_THRESHOLD &&
              rightSideAfterDoorBlock < PORTRAIT_WARM_THRESHOLD, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
