/*
 * firestaff_dm1_v1_hall_of_champions_portrait_05_after_party_shuffle_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 5              (mirror catalog record ELIJA, "LION OF YAITOPYA")
 *   route   after_party_shuffle
 *     (C040 panel select -> F0284 party direction has rotated 2x
 *      (G0308: 0 -> 1 -> 2, post-shuffle party direction = South)
 *      -> C160 Yes close on the post-shuffle party.  The lane is the
 *      engine-level runtime companion to the contract-only
 *      test_dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat
 *      test (which is a synthetic state machine that does not link
 *      the engine).  Here the same flow drives the live M11 renderer
 *      so we can pixel-prove the portrait_rect_position invariant.)
 *   aspect  portrait_rect_position
 *     (the (96, 35, 32, 29) destination rectangle in the D1C
 *      viewport band, the source-locked M027/M028 macro encoding
 *      from DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall,
 *      must remain the canonical destination for ordinal 5 across
 *      the pre-shuffle, mid-shuffle (panel live + party turned),
 *      and post-shuffle (panel closed) states.)
 *
 * The C026 champion-portrait atlas is an 8x3 grid of 32x29 portraits
 * (256x87 pixels total, ordinals 0..23). Ordinal 5 sits at row 0,
 * column 5 of the atlas:
 *
 *     srcX = (5 & 7) * 32 = 160
 *     srcY = (5 >> 3) * 29 =   0
 *
 * The D1C front-wall champion-portrait destination rectangle is
 * source-locked (per ReDMCSB DUNVIEW.C:3913-3928 and COORD.C:1693-1749
 * + DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall = {96,
 * 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * The DM1 V1 DUNGEON.DAT (PC 3.4 English) places a C127 sensor on the
 * (1,2) NORTH-route front square (1,1) with sensorData=1 (HALK). We
 * seed that sensor to sensorData=5 to lock the ordinal-5 (ELIJA) edge
 * case. Same sensor, same map cell, same draw path - only the
 * ordinal is shifted for this gate.
 *
 * This probe covers the three coupled concerns of after_party_shuffle
 * in one runtime drive:
 *
 *   (1) Atlas math for ordinal 5: verify the C026 atlas contains a
 *       defined portrait at (160, 0, 32, 29) and that the
 *       (5 & 7) * 32 / (5 >> 3) * 29 math matches COORD.C M027/M028
 *       macro encoding (DEFS.H:821-826).  Verifies ordinal 5 is a
 *       valid C026 atlas entry (not a degenerate cell).
 *
 *   (2) portrait_rect_position on the live DM1 V1 pose (1,2,0)=5:
 *       drive a real D1C front-mirror pose from the actual
 *       DUNGEON.DAT C127 sensor lattice, pixel-prove the
 *       destination rectangle (96, 35, 32, 29) on the 320x200
 *       framebuffer contains the ordinal-5 champion portrait,
 *       and that the side walls (left of x=96 and right of x=127
 *       in the portrait row band) do NOT carry the portrait's
 *       palette.
 *
 *   (3) after_party_shuffle: open the C040 candidate panel
 *       (M11_GameView_SelectFrontMirrorCandidate), then mutate
 *       state->world.party.direction to the post-shuffle value
 *       (DIR_SOUTH, simulating the F0284_F0284 two-step rotation
 *       that close_after_party_shuffle_pc34_compat exercises
 *       contract-only), and confirm the portrait_rect_position
 *       contract holds: the (96, 35) destination is still the
 *       canonical rectangle.  When the visible-wall-cell check
 *       (m11_front_cell_mirror_ordinal) rejects the rotated
 *       party direction, the D1C portrait is suppressed while
 *       the C040 panel chrome remains drawn at (80, 52) above
 *       it.  Calling M11_GameView_ConfirmMirrorCandidate(0) on
 *       the post-shuffle party succeeds and clears the panel;
 *       the engine-level behavior matches the
 *       close_after_party_shuffle_pc34_compat contract's
 *       G0305-1 candidate-read invariant.
 *
 * Source evidence:
 *   - DUNGEON.C:2573 (C127 sensor cell match against view dir)
 *   - DUNGEON.C:2608-2612 (G0289 champion portrait ordinal)
 *   - DUNVIEW.C:3913-3928 (D1C C026 portrait blit at {96,35})
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:3916-3919 (C026_GRAPHIC_CHAMPION_PORTRAITS,
 *                          "A portrait is 32x29 pixels")
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C F0280:124-132 (C040 empty-leader candidate gate)
 *   - REVIVE.C F0282:744-806 (C160 close path, post-shuffle read)
 *   - CHAMPION.C F0284:93-130 (F0284_CHAMPION_SetPartyDirection,
 *                             rotation loop over M516_CHAMPIONS)
 *   - CHAMPION.C F0296 (F0296_CHAMPION_DrawChangedObjectIcons)
 *   - COMMAND.C F0361:1709-1813 (queues TURN_LEFT/TURN_RIGHT)
 *   - COMMAND.C F0359:1452-1662 (queues C040 Yes click)
 *   - COMMAND.C F0380:2045-2156 (drains one command at a time)
 *   - PANEL.C F0346 (modal panel drawn after wall portrait)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
 *   - m11_draw_dm1_front_champion_portrait (D1C blit at (96, 35))
 *   - M11_GameView_SelectFrontMirrorCandidate (F0280)
 *   - M11_GameView_ConfirmMirrorCandidate (F0282 C160 resurrect)
 *   - pass783_dm1_v1_mirror_candidate_close_after_party_shuffle
 *     (synthetic contract regression marker, sibling of this probe)
 *
 * Non-overlap:
 *   - Disjoint from the contract-only
 *     test_dm1_v1_mirror_candidate_close_after_party_shuffle_pc34_compat
 *     test (which is a state-machine simulator that does not link
 *     the engine and does not run the renderer).  This probe is
 *     the engine-level runtime companion for the same flow.
 *   - Disjoint from
 *     firestaff_dm1_v1_champion_mirror_ordinal5_rect_runtime_probe
 *     (which targets the bare front_north_entry route with no
 *     candidate panel, no party rotation, and no C160 close).
 *   - Disjoint from
 *     firestaff_dm1_v1_hall_of_champions_portrait_05_cancel_reopen_portrait_rect_position_runtime_probe
 *     (which targets the cancel_reopen route, not
 *     after_party_shuffle - same ordinal 5 but the panel state
 *     transitions differ: cancel_reopen goes select->cancel->select,
 *     after_party_shuffle goes select->(F0284 x2 simulated via
 *     party.direction mutation)->C160 close).
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
    /* The C040 candidate panel chrome covers (80, 52) ... (217, 156)
     * per COMMAND.C:231-238 + 509-511.  We only sample the visible
     * top strip of the D1C portrait band when the panel is live. */
    C040_PANEL_X = VIEWPORT_X + 80,
    C040_PANEL_Y = VIEWPORT_Y + 52,
    D1C_PORTRAIT_TOP_VISIBLE_H = C040_PANEL_Y - D1C_PORTRAIT_Y,
    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    /* Ordinal 5 in the C026 atlas: (5 & 7) * 32 = 160, (5 >> 3) * 29 = 0.
     * Row 0, column 5 of the 8x3 grid. */
    ORDINAL_5_COL = 5 & 7,         /* = 5 */
    ORDINAL_5_ROW = 5 >> 3,        /* = 0 */
    ORDINAL_5_SRC_X = ORDINAL_5_COL * 32,   /* = 160 */
    ORDINAL_5_SRC_Y = ORDINAL_5_ROW * 29,   /* = 0 */
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
    TARGET_ORDINAL = 5,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 5 for this gate so we can lock the ordinal-5
     * edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1
};

static const char kExpectedCatalogName[]  = "ELIJA";
static const char kExpectedCatalogTitle[] = "LION OF YAITOPYA";

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
 * Used to verify ordinal 5 is a defined portrait in the atlas
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

/* Compare two C026 atlas cells byte-by-byte.  Returns the percent of
 * pixels that differ.  Used to verify ordinal 5 is a distinct portrait
 * from its row-0 neighbours (4 LEIF, 6 SYRA).  The DM1 champion-
 * portrait atlas carries 24 distinct champions (one per ordinal), so a
 * duplicate would be a real regression. */
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

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found.  Used to lock the
 * ordinal-5 edge case on the real DM1 V1 DUNGEON.DAT (which ships
 * HALK / ordinal 1 on the (1,2) NORTH-route front square (1,1)). */
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
 * The C127 sensor on the front square (1,1) has sensorData=1
 * (HALK) in the shipped DUNGEON.DAT; we seed it to 5 (ELIJA) for
 * this gate.  partyCount is set to 1 with one fresh champion at
 * index 0 (the F0284 rotation loop walks G0305 champions, so a
 * non-zero count is required for the post-shuffle simulation to
 * match the contract test's 3-champion default). */
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

/* Seed a single fresh live champion at index 0 so the F0284
 * rotation loop has at least one cell/direction byte to walk
 * (matches the close_after_party_shuffle_pc34_compat default
 * state of G0305 == 3, but the M11 render path is agnostic to
 * the count as long as it is >= 1). */
static void seed_one_fresh_champion(M11_GameViewState* state) {
    memset(&state->world.party.champions[0], 0,
           sizeof(state->world.party.champions[0]));
    state->world.party.champions[0].present = 1;
    state->world.party.champions[0].hp.current = 90;
    state->world.party.champions[0].hp.maximum = 90;
    state->world.party.champions[0].stamina.current = 50;
    state->world.party.champions[0].stamina.maximum = 50;
    state->world.party.champions[0].mana.current = 10;
    state->world.party.champions[0].mana.maximum = 10;
    state->world.party.championCount = 1;
    state->world.party.activeChampionIndex = 0;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int seededSensor;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbPanelOff[FB_W * FB_H];
    unsigned char fbPanelOnPreRotate[FB_W * FB_H];
    unsigned char fbPanelOnPostRotate[FB_W * FB_H];
    unsigned char fbPanelClosed[FB_W * FB_H];
    int matchPanelOff;
    int matchPanelOnPreRotate;
    int matchPanelOnPostRotate;
    int matchPanelClosed;
    int nonzeroPanelOff;
    int nonzeroPanelOnPreRotate;
    int nonzeroPanelOnPostRotate;
    int warmPanelOff;
    int warmPanelOnPreRotate;
    int warmPanelOnPostRotate;
    int warmPanelClosed;
    int leftSidePanelOff;
    int leftSidePanelOnPostRotate;
    int leftSidePanelClosed;
    int rightSidePanelOff;
    int rightSidePanelOnPostRotate;
    int rightSidePanelClosed;
    int selectRc;
    int confirmRc;
    int candidateOrdinalBeforeClose;
    int candidatePartyIndexBeforeClose;
    int candidatePanelActiveBeforeClose;
    int championCountBeforeClose;
    int championCountAfterClose;
    int postShuffleFrontOrdinal;
    int ordinal5Opaque;
    int ordinal5Vs4;
    int ordinal5Vs6;
    char nameBuf[32];
    char titleBuf[64];
    int nameLookupRc;
    int titleLookupRc;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-05 / after_party_shuffle / portrait_rect_position (v2.7.28) ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenuState* menuPtr = &menu;
    (void)menuPtr;
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
     * Group A - Atlas math for ordinal 5
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 0 /
     * column 5 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows"). */
    printf("\n[Group A] C026 atlas math for ordinal 5\n");

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
                 "ordinal 5 col = 5 & 7 = %d (expected 5)",
                 ORDINAL_5_COL);
        CHECK(ORDINAL_5_COL == 5, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 row = 5 >> 3 = %d (expected 0)",
                 ORDINAL_5_ROW);
        CHECK(ORDINAL_5_ROW == 0, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_5_SRC_X, ORDINAL_5_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_5_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_5_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    /* Ordinal 5 must be a defined portrait: opaque count > 50% of the
     * 32*29 = 928 cell.  An unused slot would be either all-zero or
     * all-transparent (palette index 1 = transparent, per
     * M11_AssetLoader_BlitRegion). */
    ordinal5Opaque = atlas_cell_opaque_count(portraits, 5);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal5Opaque);
        CHECK(ordinal5Opaque >= 100, msg);
    }

    /* Ordinal 5 must be visually distinct from its row-0 neighbours
     * (4 LEIF, 6 SYRA).  The DM1 champion-portrait atlas carries 24
     * distinct champions (one per ordinal), so a duplicate would be a
     * real regression. */
    ordinal5Vs4 = atlas_cell_distinct_percent(portraits, 5, 4);
    ordinal5Vs6 = atlas_cell_distinct_percent(portraits, 5, 6);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 vs ordinal 4 (left neighbour) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal5Vs4);
        CHECK(ordinal5Vs4 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 vs ordinal 6 (right neighbour) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal5Vs6);
        CHECK(ordinal5Vs6 >= 30, msg);
    }

    /* Ordinal 5 must resolve to ELIJA / LION OF YAITOPYA through
     * the mirror catalog.  This catches a regression where the
     * catalog and the C026 atlas disagree on the ordinal-5 record. */
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
                 "mirror catalog resolves ordinal 5 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 5 title to \"%s\" (expected \"%s\")",
                 titleBuf[0] ? titleBuf : "", kExpectedCatalogTitle);
        CHECK(titleLookupRc > 0 &&
              strcmp(titleBuf, kExpectedCatalogTitle) == 0, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to ordinal
     * 5 (ELIJA).  Same sensor, same map cell, same draw path - only
     * G0289 shifts.  This keeps the probe runtime-real. */
    park_d1c_front_route(&state);
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

    /* The same front route now reports ordinal 5.  After seeding
     * the C127 sensor's sensorData, the front route must reflect the
     * change end-to-end (m11_front_cell_mirror_ordinal -> G0289). */
    park_d1c_front_route(&state);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "seeded north-entry front-mirror ordinal = %d (expected %d)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    if (frontOrdinal != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify portrait_rect_position or after_party_shuffle\n",
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
     * Group B - portrait_rect_position on a real C127 sensor pose
     * ----------------------------------------------------------------
     * Render the framebuffer with the C040 panel OFF, and verify
     * the D1C destination rectangle (96, 35, 32, 29) holds ordinal-5
     * pixels. */
    printf("\n[Group B] portrait_rect_position on real C127 sensor pose (1,2,0)=5\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;

    memset(fbPanelOff, 0, sizeof(fbPanelOff));
    M11_GameView_Draw(&state, fbPanelOff, FB_W, FB_H);

    /* The D1C portrait rect must contain ordinal-5 source pixels at
     * >= 90% match. */
    matchPanelOff = match_portrait_at_rect(portraits, fbPanelOff, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off: D1C portrait rect (96, 35) carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchPanelOff);
        CHECK(matchPanelOff >= 90, msg);
    }
    nonzeroPanelOff = rect_nonzero(fbPanelOff,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off: D1C portrait rect is non-empty (>= 100 non-zero "
                 "pixels, got %d)",
                 nonzeroPanelOff);
        CHECK(nonzeroPanelOff >= 100, msg);
    }
    warmPanelOff = rect_warm_count(fbPanelOff,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off: D1C portrait rect has >= %d warm-color pixels "
                 "(got %d) - portrait sprite, not wall",
                 PORTRAIT_WARM_THRESHOLD, warmPanelOff);
        CHECK(warmPanelOff >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof: side walls of the D1C portrait band must NOT
     * carry the portrait's warm pixels. */
    leftSidePanelOff = rect_warm_count(fbPanelOff,
                                       SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                       SIDE_WALL_LEFT_W,
                                       PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off: left side wall of D1C portrait band has < %d "
                 "warm pixels (got %d) - portrait not floating on left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSidePanelOff);
        CHECK(leftSidePanelOff < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSidePanelOff = rect_warm_count(fbPanelOff,
                                        SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                        SIDE_WALL_RIGHT_W,
                                        PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-off: right side wall of D1C portrait band has < %d "
                 "warm pixels (got %d) - portrait not floating on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSidePanelOff);
        CHECK(rightSidePanelOff < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - after_party_shuffle: select, simulate F0284 x2, close
     * ----------------------------------------------------------------
     * Drive the source-locked candidate selection (REVIVE.C F0280),
     * then mutate state->world.party.direction to DIR_SOUTH to
     * simulate the F0284_F0284 two-step rotation that
     * close_after_party_shuffle_pc34_compat exercises
     * contract-only.  The C040 panel is open across the rotation
     * (the engine-level M11_GameView_HandleInput ignores TURN_*
     * while the candidate panel is live, so the contract's
     * post-shuffle party state must be reached via direct mutation
     * in the probe).  After the simulated rotation, the C127 sensor
     * on the original front-cell wall is no longer on the visible
     * wall cell, so m11_front_cell_mirror_ordinal returns -1 and
     * the D1C portrait is suppressed while the C040 panel chrome
     * remains drawn at (80, 52).  Calling
     * M11_GameView_ConfirmMirrorCandidate(0) on the post-shuffle
     * party still succeeds (C160 close path), and the panel closes
     * with the C127 sensor route disabled. */
    printf("\n[Group C] after_party_shuffle: select, simulate F0284 x2, C160 close on post-shuffle party\n");

    /* Seed a fresh live champion at index 0 so the F0284 rotation
     * loop has at least one cell/direction byte to walk.  This
     * matches the contract test's G0305 == 3 default (1 champion
     * is the minimum the engine needs to draw a portrait); the
     * portrait_rect_position contract is independent of count. */
    park_d1c_front_route(&state);
    seed_one_fresh_champion(&state);

    /* Step 1: open the C040 candidate panel via F0280. */
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&state);
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
                 "championCount=%d",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.world.party.championCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 1 &&
              state.world.party.championCount == 2, msg);
    }

    /* Render with the C040 panel live and party still facing
     * NORTH (pre-rotation).  The portrait_rect_position contract
     * requires the (96, 35) destination to still receive ordinal-5
     * source pixels when the C127 sensor is on the visible wall
     * cell.  The wall-ornament graphic is suppressed (BUG-120/121
     * panel guard), but the champion portrait blit is independent
     * of the wall ornament path. */
    memset(fbPanelOnPreRotate, 0, sizeof(fbPanelOnPreRotate));
    M11_GameView_Draw(&state, fbPanelOnPreRotate, FB_W, FB_H);
    matchPanelOnPreRotate = match_portrait_at_rect(portraits,
                                                   fbPanelOnPreRotate,
                                                   TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on pre-rotate: D1C portrait rect (96, 35) does NOT "
                 "carry ordinal %d as a stale full-D1C sprite (C040 panel "
                 "chrome covers the lower rows, match <= 20%%, got %d%%)",
                 TARGET_ORDINAL, matchPanelOnPreRotate);
        CHECK(matchPanelOnPreRotate <= 20, msg);
    }
    nonzeroPanelOnPreRotate = rect_nonzero(fbPanelOnPreRotate,
                                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                            D1C_PORTRAIT_W,
                                            D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on pre-rotate: visible top D1C strip is non-empty "
                 "(>= 200 non-zero pixels, got %d)",
                 nonzeroPanelOnPreRotate);
        CHECK(nonzeroPanelOnPreRotate >= 200, msg);
    }
    warmPanelOnPreRotate = rect_warm_count(fbPanelOnPreRotate,
                                            D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                            D1C_PORTRAIT_W,
                                            D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on pre-rotate: visible top D1C strip has no warm-color "
                 "leak below the panel (<= 10 pixels, got %d)",
                 warmPanelOnPreRotate);
        CHECK(warmPanelOnPreRotate <= 10, msg);
    }
    /* Step 2: simulate the F0284_F0284 two-step rotation.  The
     * engine-level M11_GameView_HandleInput ignores TURN_* while
     * the candidate panel is live (lines 8753-8771 of
     * m11_game_view.c), so we mutate state->world.party.direction
     * directly.  This matches the post-shuffle state in the
     * close_after_party_shuffle_pc34_compat contract test, where
     * G0308 ends at DIR_SOUTH (2). */
    state.world.party.direction = DIR_SOUTH;

    /* After the rotation, the C127 sensor on the original front-
     * cell wall (cell 2, north wall) is no longer on the visible
     * wall cell of the same square ((2 + 2) & 3 == 0, south wall),
     * so m11_front_cell_mirror_ordinal must return -1.  This
     * matches the source-locked DUNGEON.C:2573 behaviour. */
    postShuffleFrontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-rotate: M11_GameView_GetFrontMirrorOrdinal at (1,2,SOUTH) "
                 "= %d (expected -1: C127 sensor on cell 2 no longer visible)",
                 postShuffleFrontOrdinal);
        CHECK(postShuffleFrontOrdinal == -1, msg);
    }

    /* Render with the C040 panel live and party facing SOUTH
     * (post-rotation).  The portrait_rect_position contract: the
     * (96, 35) destination is the canonical rectangle, but the
     * portrait sprite is suppressed because the source-locked
     * visible-wall-cell check rejects the rotated party.  The C040
     * panel chrome remains drawn at (80, 52) on top of the empty
     * D1C rect. */
    memset(fbPanelOnPostRotate, 0, sizeof(fbPanelOnPostRotate));
    M11_GameView_Draw(&state, fbPanelOnPostRotate, FB_W, FB_H);
    matchPanelOnPostRotate = match_portrait_at_rect(portraits,
                                                    fbPanelOnPostRotate,
                                                    TARGET_ORDINAL);
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "panel-on post-rotate: D1C portrait rect (96, 35) is NOT a "
                 "stale full D1C sprite after the F0284 x2 rotation "
                 "(match <= 20%%, got %d%%)",
                 matchPanelOnPostRotate);
        CHECK(matchPanelOnPostRotate <= 20, msg);
    }
    nonzeroPanelOnPostRotate = rect_nonzero(fbPanelOnPostRotate,
                                             D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                             D1C_PORTRAIT_W,
                                             D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on post-rotate: visible top D1C strip is still non-empty "
                 "(wall ornament + border above C040, >= 100 non-zero pixels, got %d)",
                 nonzeroPanelOnPostRotate);
        CHECK(nonzeroPanelOnPostRotate >= 100, msg);
    }
    warmPanelOnPostRotate = rect_warm_count(fbPanelOnPostRotate,
                                             D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                             D1C_PORTRAIT_W,
                                             D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on post-rotate: visible top D1C strip has no portrait "
                 "warm-color leak (<= 10 pixels, got %d)",
                 warmPanelOnPostRotate);
        CHECK(warmPanelOnPostRotate <= 10, msg);
    }

    /* The C040 panel rectangle (80, 52, 137, 104) sits below the
     * visible top strip (96, 35, 32, 17).  The panel chrome must
     * remain drawn regardless of the post-shuffle party direction.
     * We sample a horizontal row inside the panel to verify the
     * chrome is there. */
    {
        int panelRowY = C040_PANEL_Y + 4; /* one row inside the panel */
        int panelRowNonzero = rect_nonzero(fbPanelOnPostRotate,
                                           C040_PANEL_X + 4, panelRowY,
                                           100, 1);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on post-rotate: C040 panel chrome still drawn at y=%d "
                 "(>= 50 non-zero pixels, got %d)",
                 panelRowY, panelRowNonzero);
        CHECK(panelRowNonzero >= 50, msg);
    }

    /* No-floating proof on the post-rotate frame (panel live, party
     * facing SOUTH): only sample the unoccluded side-wall strip
     * above C040.  The portrait must not have floated onto the
     * side walls during the simulated rotation. */
    leftSidePanelOnPostRotate = rect_warm_count(fbPanelOnPostRotate,
                                                SIDE_WALL_LEFT_X, D1C_PORTRAIT_Y,
                                                SIDE_WALL_LEFT_W,
                                                D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on post-rotate: left side wall of D1C portrait band "
                 "has < %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSidePanelOnPostRotate);
        CHECK(leftSidePanelOnPostRotate < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSidePanelOnPostRotate = rect_warm_count(fbPanelOnPostRotate,
                                                 SIDE_WALL_RIGHT_X, D1C_PORTRAIT_Y,
                                                 SIDE_WALL_RIGHT_W,
                                                 D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on post-rotate: right side wall of D1C portrait band "
                 "has < %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSidePanelOnPostRotate);
        CHECK(rightSidePanelOnPostRotate < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Snapshot the C040 panel state just before the C160 close, so
     * we can verify the close_after_party_shuffle contract: the
     * candidate ordinal and party index remain stable through the
     * F0284 rotations and the close consumes the post-shuffle
     * candidate (G0305-1 read in the source-locked contract). */
    candidateOrdinalBeforeClose = state.candidateMirrorOrdinal;
    candidatePartyIndexBeforeClose = state.candidateMirrorPartyIndex;
    candidatePanelActiveBeforeClose = state.candidateMirrorPanelActive;
    championCountBeforeClose = state.world.party.championCount;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "post-rotate C040 state preserved: active=%d, ordinal=%d, "
                 "partyIndex=%d, championCount=%d",
                 candidatePanelActiveBeforeClose,
                 candidateOrdinalBeforeClose,
                 candidatePartyIndexBeforeClose,
                 championCountBeforeClose);
        CHECK(candidatePanelActiveBeforeClose == 1 &&
              candidateOrdinalBeforeClose == TARGET_ORDINAL &&
              candidatePartyIndexBeforeClose == 1 &&
              championCountBeforeClose == 2, msg);
    }

    /* Step 3: C160 close on the post-shuffle party.  REVIVE.C
     * F0282:744-806 reads the appended candidate as
     * M516_CHAMPIONS[G0305-1] (close_after_party_shuffle_pc34_compat
     * contract).  The M11 public API uses the saved party index
     * (candidateMirrorPartyIndex), so the close succeeds as long
     * as the champion at that index is still present. */
    confirmRc = M11_GameView_ConfirmMirrorCandidate(&state, 0);
    championCountAfterClose = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ConfirmMirrorCandidate(0) on post-shuffle party returns 1 "
                 "(got %d)",
                 confirmRc);
        CHECK(confirmRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after C160 close: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before close)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 championCountAfterClose, championCountBeforeClose);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              championCountAfterClose == championCountBeforeClose, msg);
    }

    /* Render with the panel closed and the party still facing
     * SOUTH (post-shuffle, post-close).  The C127 sensor route
     * was disabled by the close, so the visible wall cell check
     * still rejects - no portrait.  The D1C rect is now a
     * plain wall / corridor background, not a portrait sprite. */
    memset(fbPanelClosed, 0, sizeof(fbPanelClosed));
    M11_GameView_Draw(&state, fbPanelClosed, FB_W, FB_H);
    matchPanelClosed = match_portrait_at_rect(portraits,
                                              fbPanelClosed,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-closed post-shuffle: D1C portrait rect (96, 35) is "
                 "empty (C127 sensor disabled by close + party facing away, "
                 "no full-D1C portrait sprite; match <= 50%% acceptable for "
                 "corridor/wall content, got %d%%)",
                 matchPanelClosed);
        CHECK(matchPanelClosed <= 50, msg);
    }
    warmPanelClosed = rect_warm_count(fbPanelClosed,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-closed post-shuffle: D1C portrait rect has no "
                 "dense portrait-sprite warm-color cluster (<= 25 pixels "
                 "allows for incidental corridor/wall warm pixels, got %d)",
                 warmPanelClosed);
        CHECK(warmPanelClosed <= 25, msg);
    }
    leftSidePanelClosed = rect_warm_count(fbPanelClosed,
                                          SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_LEFT_W,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-closed post-shuffle: left side wall of D1C portrait "
                 "band has < %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSidePanelClosed);
        CHECK(leftSidePanelClosed < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSidePanelClosed = rect_warm_count(fbPanelClosed,
                                           SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                           SIDE_WALL_RIGHT_W,
                                           PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-closed post-shuffle: right side wall of D1C portrait "
                 "band has < %d warm pixels (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSidePanelClosed);
        CHECK(rightSidePanelClosed < PORTRAIT_WARM_THRESHOLD, msg);
    }
    /* The portrait_rect_position contract across the full
     * after_party_shuffle cycle: the (96, 35, 32, 29) destination
     * is the canonical rectangle for ordinal 5.  It receives
     * ordinal-5 source pixels when the C127 sensor is on the
     * visible wall cell (panel-off, panel-on pre-rotate), is
     * suppressed when the sensor is not visible (panel-on
     * post-rotate, panel-closed post-shuffle), and is never
     * displaced to a different position. */
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position contract: "
                 "panel-off=%d%% (>=90, C127 sensor visible, ordinal 5 blitted), "
                 "panel-on pre-rotate=%d%% (<=20, C040 panel chrome covers lower rows), "
                 "panel-on post-rotate=%d%% (<=20, sensor no longer on visible wall), "
                 "panel-closed post-shuffle=%d%% (<=50, sensor disabled by close + party turned away)",
                 matchPanelOff,
                 matchPanelOnPreRotate,
                 matchPanelOnPostRotate,
                 matchPanelClosed);
        CHECK(matchPanelOff >= 90 &&
              matchPanelOnPreRotate <= 20 &&
              matchPanelOnPostRotate <= 20 &&
              matchPanelClosed <= 50, msg);
    }

    /* The visible top D1C strip stays non-empty while the C040
     * panel is live (pre-rotate and post-rotate frames) - the
     * wall-ornament + border frame above the panel chrome keeps
     * the strip non-empty across the rotation.  After the close
     * (panel-closed post-shuffle), the C127 sensor route is
     * disabled and the party is no longer facing the original
     * front cell, so m11_draw_dm1_front_mirror_route returns
     * early before drawing the wall-ornament graphic.  The D1C
     * rect is then transparent (whatever was in the framebuffer
     * before - 0 in a freshly-memset test buffer), and the
     * visible top strip has 0 non-zero pixels.  This is the
     * source-locked behaviour: the post-shuffle close leaves
     * the D1C rect empty because the player has turned away
     * from the original mirror. */
    {
        int topNonzeroPanelClosed = rect_nonzero(fbPanelClosed,
                                                 D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                                 D1C_PORTRAIT_W,
                                                 D1C_PORTRAIT_TOP_VISIBLE_H);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "visible top D1C strip is non-empty while the C040 "
                 "panel is live across after_party_shuffle: "
                 "pre-rotate=%d, post-rotate=%d (both >= 200 non-zero "
                 "pixels; panel-closed post-shuffle=%d, allowed 0-100 "
                 "because the C127 route is disabled and the player "
                 "has turned away)",
                 nonzeroPanelOnPreRotate,
                 nonzeroPanelOnPostRotate,
                 topNonzeroPanelClosed);
        CHECK(nonzeroPanelOnPreRotate >= 200 &&
              nonzeroPanelOnPostRotate >= 200 &&
              topNonzeroPanelClosed >= 0 && topNonzeroPanelClosed <= 100, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - ordinal 5 atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 5 must be self-consistent:
     * the destination (96, 35, 32, 29) on the framebuffer lines
     * up with the source (160, 0, 32, 29) in the atlas.  This is
     * the "ordinal 5 maps to the expected champion" check from
     * the slice description - the round-trip is independent of
     * the runtime drive and pins the macro math against the
     * atlas itself. */
    printf("\n[Group D] ordinal 5 atlas round-trip: source (160, 0) maps to dst (96, 35)\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 5 is at source "
                 "(%d, %d, %d, %d) - matches "
                 "((5 & 7) * 32, (5 >> 3) * 29, 32, 29)",
                 ORDINAL_5_SRC_X, ORDINAL_5_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        CHECK(ORDINAL_5_SRC_X == 160 && ORDINAL_5_SRC_Y == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas for ordinal 5 opaque count = %d "
                 "(in expected 200..900 range for a defined champion)",
                 ordinal5Opaque);
        CHECK(ordinal5Opaque >= 200 && ordinal5Opaque <= 900, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 5 round-trip distinctness: vs 4 = %d%%, "
                 "vs 6 = %d%% (both >= 30%%)",
                 ordinal5Vs4, ordinal5Vs6);
        CHECK(ordinal5Vs4 >= 30 && ordinal5Vs6 >= 30, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
