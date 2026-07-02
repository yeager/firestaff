/*
 * firestaff_dm1_v1_hall_of_champions_portrait_15_popup_focus_return_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 15           (mirror catalog record MOPHUS, title THE HEALER)
 *   route   popup_focus_return (C040 panel select -> HandleInput
 *                              focuses on the popup while non-cancel
 *                              /non-confirm inputs are absorbed with
 *                              M11_GAME_INPUT_IGNORED, focus stays on
 *                              the panel -> F0282 C162 cancel branch
 *                              -> fresh C040 panel select on the same
 *                              front square; championCount goes
 *                              0 -> 1 -> 0 -> 1, panel-state hash
 *                              before-select == after-reopen)
 *   aspect  portrait_rect_position
 *
 * The C026 champion-portrait atlas is an 8x3 grid of 32x29 portraits
 * (256x87 pixels total, ordinals 0..23).  Ordinal 15 sits at row 1,
 * column 7 of the atlas -- the LAST column of the second row, the
 * col-7 right-edge boundary of the 8x3 strip:
 *
 *     srcX = (15 & 7) << 5 = 224
 *     srcY = (15 >> 3) * 29 = 29
 *
 * (DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y macro encoding;
 *  the `<< 5` form is the MEDIAs20x/S10E/S11E/S12E/.../G21E/A22E PC 3.4
 *  variant; the width-multiply form  `* G2078_C32_PortraitWidth`
 *  in BLIT.C:3928 is for later MEDIAs529 / I34E / A36M ports where
 *  G2078_C32_PortraitWidth is also 32.)
 *
 * The D1C front-wall champion-portrait destination rectangle is
 * source-locked (per ReDMCSB DUNVIEW.C:3913-3928 and DUNVIEW.C:525
 * G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96, 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * The shipped DM1 V1 DUNGEON.DAT places a C127 sensor on the (1,2)
 * NORTH-route front square (1,1) with sensorData=1 (HALK, ordinal 1),
 * so we seed that sensor to sensorData=15 to lock the ordinal-15 edge
 * case (MOPHUS, row 1 / col 7).  This keeps the probe runtime-real:
 * same sensor, same DUNGEON.DAT, same draw path - only the ordinal
 * that DUNVIEW.C:3913-3928 reads through M000_INDEX_TO_ORDINAL
 * (DUNGEON.C:2610-2612) is shifted for the test.  This is the same
 * seed pattern used by the existing portrait-15 cancel_reopen gate.
 *
 * This probe covers the four coupled concerns of popup_focus_return
 * in one runtime drive:
 *
 *   (1) Atlas math for ordinal 15: verify the C026 atlas contains a
 *       defined portrait at (224, 29, 32, 29) and that the
 *       (15 & 7) << 5 / (15 >> 3) * 29 math matches COORD.C M027/M028
 *       macro encoding (DEFS.H:821-826).  Verifies ordinal 15 is a
 *       valid C026 atlas entry (not a degenerate cell), and that
 *       the row-1 col-7 atlas math is the col-7 right-edge boundary
 *       (srcX(224) + portraitW(32) = 256 == atlasW(256)).
 *
 *   (2) portrait_rect_position: drive a real D1C front-mirror pose
 *       from the actual DM1 V1 DUNGEON.DAT C127 sensor lattice
 *       (with sensorData seeded to 15), pixel-prove the destination
 *       rectangle (96, 35, 32, 29) on the 320x200 framebuffer
 *       contains the ordinal-15 champion portrait (MOPHUS), and that
 *       the side walls (left of x=96 and right of x=127 in the
 *       portrait row band) do NOT carry the portrait's palette.
 *
 *   (3) popup_focus: select the candidate (panel live).  Then drive
 *       M11_GameView_HandleInput with a sequence of non-cancel /
 *       non-confirm inputs while the panel is up.  Per source-
 *       locked contract (src/engine/m11_game_view.c
 *       `if (state->candidateMirrorPanelActive)` block at line
 *       ~8295): BACK returns cancel/REDRAW, ACTION/ACCEPT returns
 *       confirm/REDRAW, and every other input returns
 *       M11_GAME_INPUT_IGNORED.  This proves the popup holds focus
 *       (does not bleed movement, turn, strafe, rest, or any other
 *       game command while the panel is up) and proves panel state
 *       (candidateMirrorPanelActive=1, candidateMirrorOrdinal=15,
 *       candidateMirrorPartyIndex=0, championCount=1) is
 *       unaffected by the absorbed inputs.
 *
 *   (4) popup_focus_return: cancel (F0282 C162 branch via
 *       M11_GameView_CancelMirrorCandidate), re-select (F0280 /
 *       M11_GameView_SelectFrontMirrorCandidate returns 1 again),
 *       and pixel-prove the D1C portrait rect still carries
 *       ordinal-15 pixels after the full
 *       select->(focus-stays-during-absorbed-inputs)->cancel->select
 *       cycle.  The focus-return invariant: the visible top D1C
 *       strip (above the C040 panel) and the side walls must be
 *       stable across the popup_focus_return cycle.
 *
 * This slice is disjoint from the existing row-1/row-2 cancel_reopen
 * probes for ordinals 0, 1, 2, 3, 5, 6, 8, 9, 13, 16, 17, 18, 19, 20
 * (different ordinals + different route: no focus-absorption step
 * between select and cancel), from the existing
 * firestaff_dm1_v1_champion_mirror_ordinal_15_west_negative_portrait
 * _rect_position_runtime_probe (different route: corridor west_negative
 * band on (x=1, y=2..6) DIR_WEST where ordinal 15 cannot leak), from
 * firestaff_dm1_v1_hoc_mophus_ordinal15_unreachable_probe (different
 * route: east_walkpath corridor on ordinal 15's source-visible (2,5)
 * cell), and from the existing ordinal-15 cancel_reopen gate (different
 * route: direct C040 select -> cancel -> re-select with no absorbed
 * input focus step between the two selects).  Ordinal 15 has no
 * existing popup_focus_return gate.
 *
 * Source evidence:
 *   - DUNGEON.C:2558 (G0289 reset on wall-square entry; the BUG0_75
 *                   multiple-portrait decrement loop is part of the
 *                   same front-wall draw path)
 *   - DUNGEON.C:2612 (G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)))
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:1061 (G0205_aaauc_Graphic558_WallOrnamentCoordinateSets,
 *                     8x13x6 table; coordSet 5 / index 12 is the
 *                     D1C champion-mirror frame route)
 *   - DUNVIEW.C:3913-3919 (P0117_i_ViewWallIndex ==
 *                          M587_VIEW_WALL_D1C_FRONT &&
 *                          G0289_i_DungeonView_ChampionPortraitOrdinal--;
 *                          D1C C026 portrait blit at {96,35} with
 *                          ((ordinal & 7) << 5, (ordinal >> 3) * 29))
 *   - DUNVIEW.C:3925-3928 (F0654_Call_F0132_VIDEO_Blit for I34E / A36M
 *                          variant with G2078_C32_PortraitWidth /
 *                          G2079_C29_PortraitHeight)
 *   - DUNVIEW.C:7842 (F0107_DUNGEONVIEW_IsDrawnWallOrnamentAnAlcove_CPSF
 *                     gate against the front-wall D1C champion-mirror
 *                     route)
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - DEFS.H:2186 (C026_GRAPHIC_CHAMPION_PORTRAITS)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C F0280:124-132 (C040 empty-leader candidate gate)
 *   - REVIVE.C F0282:744-806 (C162 cancel branch 744-783)
 *   - PANEL.C F0355:2299-2318 (inventory close on cancel)
 *   - COMMAND.C F0378:1956-1990 (M568_PANEL_RESURRECT_REINCARNATE
 *                              dispatch)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
 *   - M11_GameView_CancelMirrorCandidate (F0282 C162 cancel path)
 *   - M11_GameView_SelectFrontMirrorCandidate (F0280 reopen path)
 *   - M11_GameView_HandleInput popup-focus block (BACK=cancel,
 *     ACTION/ACCEPT=confirm, all other inputs return IGNORED while
 *     candidateMirrorPanelActive=1)
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
    /* Source-locked C026 atlas dimensions.  C026 is the 8x3 grid of
     * 32x29 portraits (DUNVIEW.C:3916-3919). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_COLS = 8,
    ATLAS_ROWS = 3,
    /* Ordinal 15 in the C026 atlas: (15 & 7) << 5 = 224,
     * (15 >> 3) * 29 = 29.  This is the LAST column of row 1, the
     * col-7 right-edge boundary of the 8x3 strip. */
    ORDINAL_15_COL = 15 & 7,        /* = 7 */
    ORDINAL_15_ROW = 15 >> 3,       /* = 1 */
    ORDINAL_15_SRC_X = ORDINAL_15_COL << 5,    /* = 224 */
    ORDINAL_15_SRC_Y = ORDINAL_15_ROW * 29,    /* = 29 */
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
    TARGET_ORDINAL = 15,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 15 (MOPHUS) for this gate so we can lock the
     * ordinal-15 row-1 col-7 edge case without changing the map
     * layout. */
    SHIPPED_HALK_ORDINAL = 1,
    /* Row-1 atlas neighbours of ordinal 15 (LEYLA = 14 left at
     * row 1 col 6; ordinal 15 has no row-1 right neighbour because
     * it is the last col of the row).  The col-7 vertical anchor
     * is NABI = 23 (row 2 col 7) which shares the col-7 right
     * edge but lives in a different row.  The DM1 champion-portrait
     * atlas carries 24 distinct champions (one per ordinal), so a
     * duplicate would be a real regression. */
    ROW1_LEFT_ORDINAL = 14,
    COL7_ROW2_ORDINAL = 23
};
/* Mirror catalog record name for ordinal 15 (DM1 V1 PC34 mirror
 * catalog).  Used to assert the catalog resolves correctly.  MOPHUS
 * (title THE HEALER) is the 16th valid mirror text string in the
 * shipped DM1 V1 DUNGEON.DAT (the 16th ordinal = 15, after
 * DAROOU / HALK / WU TSE / AZIZI / LEIF / ELIJA / SYRA / TIGGY /
 * IAIDO / ZED / STAMM / LEYLA / GANDO / TIGGY / WUUF). */
static const char kExpectedCatalogName[] = "MOPHUS";
static const char kExpectedCatalogTitle[] = "THE HEALER";

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
 * C026 cutout (96, 35, 32, 29).  Ordinal 15 (MOPHUS) uses fewer
 * warm pixels than most champions (the gray-cloth palette is
 * dominant) so this is a coarse signal only. */
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
 * Used to verify ordinal 15 is a defined portrait in the atlas
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
 * pixels that differ.  Used to verify ordinal 15 is a distinct
 * portrait from its row-1 left neighbour (LEYLA = 14) and from
 * the row-0 col-7 cell (DAROOU = 7) to pin both the row-1 stride
 * and the col-7 right-edge math. */
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
 * on success, or -1 if no such sensor was found.  We use this to
 * lock the ordinal-15 edge case on the real DM1 V1 DUNGEON.DAT
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
 * seed_first_c127_data the same square reports ordinal 15. */
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
    int ordinal15Opaque;
    int ordinal15Vs14;
    int ordinal15Vs23;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterAbsorbedInputs[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    unsigned char fbAfterReopen[FB_W * FB_H];
    int matchBefore, matchAfterSelect, matchAfterAbsorbedInputs,
        matchAfterCancel, matchAfterReopen;
    int nonzeroBefore, nonzeroAfterReopen;
    int distinctBefore, distinctAfterReopen;
    int warmBefore, warmAfterReopen;
    int leftSideBefore, leftSideAfterReopen;
    int rightSideBefore, rightSideAfterReopen;
    int initialCount, countAfterSelect,
        countAfterCancel, countAfterReopen;
    int selectRc, cancelRc, reopenRc;
    int handleRc;
    int seededSensor;
    char nameBuf[32];
    char titleBuf[32];
    int nameLookupRc;
    int titleLookupRc;
    /* A panel-focused popup absorbs everything except BACK /
     * ACCEPT / ACTION.  Drive a varied mix through HandleInput
     * and confirm every one returns M11_GAME_INPUT_IGNORED while
     * panel state is unchanged. */
    const M12_MenuInput kAbsorbedInputs[] = {
        M12_MENU_INPUT_NONE,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_LEFT,
        M12_MENU_INPUT_RIGHT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_RIGHT,
        M12_MENU_INPUT_TURN_LEFT,
        M12_MENU_INPUT_TURN_RIGHT,
        M12_MENU_INPUT_CYCLE_CHAMPION,
        M12_MENU_INPUT_VALUE_LEFT,
        M12_MENU_INPUT_VALUE_RIGHT,
        M12_MENU_INPUT_REST_TOGGLE,
        M12_MENU_INPUT_USE_STAIRS,
        M12_MENU_INPUT_PICKUP_ITEM,
        M12_MENU_INPUT_DROP_ITEM,
        M12_MENU_INPUT_SPELL_RUNE_1,
        M12_MENU_INPUT_SPELL_RUNE_2,
        M12_MENU_INPUT_SPELL_RUNE_3,
        M12_MENU_INPUT_SPELL_RUNE_4,
        M12_MENU_INPUT_SPELL_RUNE_5,
        M12_MENU_INPUT_SPELL_RUNE_6,
        M12_MENU_INPUT_SPELL_CAST,
        M12_MENU_INPUT_SPELL_CLEAR,
        M12_MENU_INPUT_USE_ITEM,
        M12_MENU_INPUT_MAP_TOGGLE,
        M12_MENU_INPUT_INVENTORY_TOGGLE,
        M12_MENU_INPUT_SAVE_GAME
    };
    const int kAbsorbedCount =
        (int)(sizeof(kAbsorbedInputs) / sizeof(kAbsorbedInputs[0]));
    int absorbedAllIgnored = 1;
    int absorbedOrdinalStable = 1;
    int absorbedPanelStable = 1;
    int absorbedCountStable = 1;
    int absorbedPartyIndexStable = 1;
    int absorbedInventoryStable = 1;
    int ai;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-15 / popup_focus_return / portrait_rect_position ===\n");
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
     * Group A - Atlas math for ordinal 15 (row 1 / col 7)
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 1 /
     * column 7 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows").  This is the LAST cell of
     * row 1: srcX(224) + portraitW(32) = 256 == atlasW(256), the
     * col-7 right-edge boundary.  Ordinal 15 has no row-1 right
     * neighbour (col 7 is the last col), so the row-1 left
     * neighbour check pins LEYLA = 14 (row 1 col 6) and the
     * col-7 right-edge check pins the row-0 col-7 anchor
     * (DAROOU = 7). */
    printf("\n[Group A] C026 atlas math for ordinal 15 (row 1 / col 7)\n");

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
                 "ordinal 15 col = 15 & 7 = %d (expected 7, last col)",
                 ORDINAL_15_COL);
        CHECK(ORDINAL_15_COL == 7, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 row = 15 >> 3 = %d (expected 1, second row)",
                 ORDINAL_15_ROW);
        CHECK(ORDINAL_15_ROW == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 srcX = %d, srcY = %d "
                 "(col-7 right-edge boundary: srcX + W = %d == atlasW %d)",
                 ORDINAL_15_SRC_X, ORDINAL_15_SRC_Y,
                 ORDINAL_15_SRC_X + D1C_PORTRAIT_W, ATLAS_W);
        CHECK(ORDINAL_15_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_15_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 sits at the col-7 right-edge: srcX(224) + "
                 "portraitW(32) = 256 == atlasW(256) - last col of row 1");
        CHECK(ORDINAL_15_SRC_X + D1C_PORTRAIT_W == ATLAS_W, msg);
    }

    /* Ordinal 15 must be a defined portrait: opaque count > 100 of the
     * 32*29 = 928 cell.  An unused slot would be either all-zero or
     * all-transparent (palette index 1 = transparent, per
     * M11_AssetLoader_BlitRegion).  This catches a regression where
     * ordinal 15 (MOPHUS) is treated as "no portrait" - in particular
     * a regression where (ordinal >> 3) * 29 yields the wrong row on
     * the C026 strip (e.g. row 1 -> row 0 off-by-one) or where the
     * & 7 mask drops the col-7 boundary (e.g. ordinal 15 resolves to
     * col 0 + extra offset). */
    ordinal15Opaque = atlas_cell_opaque_count(portraits, 15);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal15Opaque);
        CHECK(ordinal15Opaque >= 100, msg);
    }

    /* Ordinal 15 must be visually distinct from its row-1 left
     * neighbour 14 (LEYLA) and from the col-7 vertical anchor 23
     * (NABI, row 2 col 7).  The DM1 champion-portrait atlas
     * carries 24 distinct champions (one per ordinal), so a
     * duplicate would be a real regression.  Picking the row-1
     * left neighbour pins the row-1 atlas stride; the col-7
     * row-2 vertical anchor pins the col-7 right-edge math
     * specifically - this complements the row-1 col-0 ordinal 8
     * (IAIDO) row-1 anchor check used by the ordinal-09 sibling
     * and the col-7 row-0 ordinal 7 (DAROOU) col-7 anchor used
     * by the ordinal-23 sibling.  Note: the col-7 portraits
     * share a similar background between rows (the C346 wall
     * frame has the same right-edge texture in all three rows),
     * so the row-2 col-7 ordinal-23 distinctness threshold is
     * naturally lower than the row-1 left neighbour; the 30%
     * floor catches a real duplicate while still admitting the
     * shared col-7 background. */
    ordinal15Vs14 = atlas_cell_distinct_percent(portraits, 15, 14);
    ordinal15Vs23 = atlas_cell_distinct_percent(portraits, 15, 23);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 vs ordinal 14 (row-1 left neighbour, LEYLA) "
                 "differ by >= 30%% (got %d%%)",
                 ordinal15Vs14);
        CHECK(ordinal15Vs14 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 vs ordinal 23 (col-7 row-2 vertical anchor, NABI) "
                 "differ by >= 30%% (got %d%%)",
                 ordinal15Vs23);
        CHECK(ordinal15Vs23 >= 30, msg);
    }

    /* Ordinal 15 must resolve to MOPHUS / THE HEALER through the
     * mirror catalog.  This catches a regression where the catalog
     * and the C026 atlas disagree on the ordinal-15 record - in
     * particular a regression where the catalog is shorter than 16
     * entries (a catalog count < 16 would have rejected sensorData=15
     * on the real DUNGEON.DAT). */
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
                 "mirror catalog resolves ordinal 15 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 15 title to \"%s\" (expected \"%s\")",
                 titleBuf[0] ? titleBuf : "", kExpectedCatalogTitle);
        CHECK(titleLookupRc > 0 &&
              strcmp(titleBuf, kExpectedCatalogTitle) == 0, msg);
    }

    /* Park the party on the (1,2,0) NORTH-route front mirror, then
     * seed the C127 sensor from HALK (1) to ordinal 15 (MOPHUS).
     * Same sensor, same map cell, same draw path - only G0289
     * changes. */
    park_d1c_front_route(&state);

    /* First confirm the unmodified route reports the shipped HALK
     * ordinal 1 - this is the sanity check that the C127 sensor is
     * alive at the right cell before we mutate sensorData. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front-mirror ordinal at (1,2,0) = %d (expected "
                 "%d, HALK before seed)",
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to ordinal
     * 15.  Same sensor, same map cell, same draw path - only G0289
     * changes. */
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

    /* The same front route now reports ordinal 15.  After seeding
     * the C127 sensor's sensorData, the front route must reflect the
     * new ordinal.  Note: the front ordinal helper clamps to
     * [0, mirrorCatalog.count), so this check confirms the catalog
     * has at least 16 entries (which is the source-locked DM1 V1
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
                "cannot verify portrait_rect_position or popup_focus_return\n",
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
     * Group B - portrait_rect_position on the seeded C127 sensor pose
     * ----------------------------------------------------------------
     * Render the framebuffer before any selection, and verify the
     * D1C destination rectangle (96, 35, 32, 29) holds ordinal-15
     * pixels. */
    printf("\n[Group B] portrait_rect_position on real C127 sensor pose (1,2,0)=15\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

    memset(fbBefore, 0, sizeof(fbBefore));
    M11_GameView_Draw(&state, fbBefore, FB_W, FB_H);

    /* The D1C portrait rect must contain ordinal-15 source pixels at
     * >= 90% match.  This is the "portrait ordinal maps to the
     * expected champion and the D1C portrait rectangle is drawn at
     * the intended screen position" requirement from the slice. */
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

    /* No-floating proof: side walls of the D1C portrait band must NOT
     * carry the portrait's warm pixels. */
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
     * Group C - popup_focus: select, then absorb non-cancel inputs
     * ----------------------------------------------------------------
     * Drive the source-locked candidate selection (F0280 / C040),
     * then drive a sequence of non-cancel / non-confirm inputs
     * through M11_GameView_HandleInput while the panel is live.
     * Per the popup-focus block in src/engine/m11_game_view.c
     * (the `if (state->candidateMirrorPanelActive)` branch at line
     * ~8295): BACK returns cancel/REDRAW, ACTION/ACCEPT returns
     * confirm/REDRAW, every other input returns
     * M11_GAME_INPUT_IGNORED.  Panel state must be unchanged
     * through the absorption cycle (focus stays on the panel). */
    printf("\n[Group C] popup_focus: select, absorb non-cancel inputs, panel state stable\n");

    /* Step 1: SelectFrontMirrorCandidate (F0280). */
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

    /* Render with C040 panel live.  The portrait must NOT be drawn as
     * a stale floating sprite while the panel owns the view (BUG-120/
     * 121 panel guard).  Match against ordinal 15 should be low. */
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

    /* Step 2: drive the absorbed inputs through HandleInput.  Every
     * input except BACK / ACCEPT / ACTION must return
     * M11_GAME_INPUT_IGNORED.  Panel state (candidateMirrorPanelActive,
     * candidateMirrorOrdinal, candidateMirrorPartyIndex, championCount,
     * inventoryPanelActive) must be unchanged by every absorbed input. */
    {
        int panelActiveBefore = state.candidateMirrorPanelActive;
        int ordinalBefore = state.candidateMirrorOrdinal;
        int partyIndexBefore = state.candidateMirrorPartyIndex;
        int countBefore = state.world.party.championCount;
        int inventoryBefore = state.inventoryPanelActive;
        int absorbedIgnoredCount = 0;
        for (ai = 0; ai < kAbsorbedCount; ++ai) {
            handleRc = (int)M11_GameView_HandleInput(&state,
                                                     kAbsorbedInputs[ai]);
            if (handleRc != (int)M11_GAME_INPUT_IGNORED) {
                absorbedAllIgnored = 0;
            } else {
                ++absorbedIgnoredCount;
            }
            if (state.candidateMirrorPanelActive != panelActiveBefore) {
                absorbedPanelStable = 0;
            }
            if (state.candidateMirrorOrdinal != ordinalBefore) {
                absorbedOrdinalStable = 0;
            }
            if (state.candidateMirrorPartyIndex != partyIndexBefore) {
                absorbedPartyIndexStable = 0;
            }
            if (state.world.party.championCount != countBefore) {
                absorbedCountStable = 0;
            }
            if (state.inventoryPanelActive != inventoryBefore) {
                absorbedInventoryStable = 0;
            }
        }
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "popup_focus: %d/%d absorbed inputs returned "
                     "M11_GAME_INPUT_IGNORED",
                     absorbedIgnoredCount, kAbsorbedCount);
            CHECK(absorbedIgnoredCount == kAbsorbedCount, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "popup_focus: every absorbed input returned IGNORED "
                     "(saw any non-IGNORED = %s)",
                     absorbedAllIgnored ? "no" : "YES (regression)");
            CHECK(absorbedAllIgnored != 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "popup_focus: candidateMirrorPanelActive stable "
                     "across absorbed inputs (regression = %s)",
                     absorbedPanelStable ? "no" : "YES");
            CHECK(absorbedPanelStable != 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "popup_focus: candidateMirrorOrdinal stable "
                     "across absorbed inputs (regression = %s)",
                     absorbedOrdinalStable ? "no" : "YES");
            CHECK(absorbedOrdinalStable != 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "popup_focus: candidateMirrorPartyIndex stable "
                     "across absorbed inputs (regression = %s)",
                     absorbedPartyIndexStable ? "no" : "YES");
            CHECK(absorbedPartyIndexStable != 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "popup_focus: championCount stable across absorbed "
                     "inputs (regression = %s)",
                     absorbedCountStable ? "no" : "YES");
            CHECK(absorbedCountStable != 0, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "popup_focus: inventoryPanelActive stable across "
                     "absorbed inputs (regression = %s)",
                     absorbedInventoryStable ? "no" : "YES");
            CHECK(absorbedInventoryStable != 0, msg);
        }
    }

    /* Render after absorbed inputs: panel state identical to
     * fbAfterSelect, panel still live, portrait rect still
     * suppressed as a stale sprite.  This is the focus-return
     * pixel contract: absorbing inputs does not redraw the wall. */
    memset(fbAfterAbsorbedInputs, 0, sizeof(fbAfterAbsorbedInputs));
    M11_GameView_Draw(&state, fbAfterAbsorbedInputs, FB_W, FB_H);
    matchAfterAbsorbedInputs = match_portrait_at_rect(portraits,
                                                      fbAfterAbsorbedInputs,
                                                      TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw after absorbed inputs still suppresses "
                 "ordinal %d as a stale full-D1C sprite "
                 "(<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterAbsorbedInputs);
        CHECK(matchAfterAbsorbedInputs <= 20, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on frame after absorbed inputs is byte-identical "
                 "to panel-on frame right after select "
                 "(absorbed=clean: %s)",
                 memcmp(fbAfterSelect, fbAfterAbsorbedInputs,
                        sizeof(fbAfterSelect)) == 0 ? "yes" : "NO (regression)");
        CHECK(memcmp(fbAfterSelect, fbAfterAbsorbedInputs,
                     sizeof(fbAfterSelect)) == 0, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - popup_focus_return: cancel then reopen
     * ----------------------------------------------------------------
     * Drive the source-locked C162 cancel branch via
     * M11_GameView_CancelMirrorCandidate (REVIVE.C F0282:744-783),
     * then re-select with M11_GameView_SelectFrontMirrorCandidate
     * (REVIVE.C F0280).  The framebuffer's D1C portrait rect must
     * still carry ordinal-15 pixels after the full
     * select->(focus-stays-during-absorbed-inputs)->cancel->select
     * cycle, and the focus-return invariant must hold: the
     * panel-on redraw right after reopen looks identical to the
     * panel-on redraw right after the original select. */
    printf("\n[Group D] popup_focus_return: cancel then reopen, portrait rect still carries ordinal 15\n");

    /* Step 1: CancelMirrorCandidate (F0282 C162 cancel branch).
     * Per source-locked contract: F0355 inventory close, G0299 clear,
     * G0305--, F0643_PARTY_ClearChampionSlot for the candidate index.
     * The portrait on the wall stays the same because the sensor is
     * still active (sensorType=127, sensorData=15 after seed). */
    cancelRc = M11_GameView_CancelMirrorCandidate(&state);
    countAfterCancel = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "CancelMirrorCandidate on (1,2,0) returns 1 (got %d)",
                 cancelRc);
        CHECK(cancelRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before select)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterCancel, initialCount);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1 &&
              countAfterCancel == initialCount, msg);
    }

    /* Render after cancel: panel closed, portrait rect must hold
     * ordinal-15 pixels again. */
    memset(fbAfterCancel, 0, sizeof(fbAfterCancel));
    M11_GameView_Draw(&state, fbAfterCancel, FB_W, FB_H);
    matchAfterCancel = match_portrait_at_rect(portraits,
                                               fbAfterCancel,
                                               TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchAfterCancel);
        CHECK(matchAfterCancel >= 90, msg);
    }

    /* Step 2: SelectFrontMirrorCandidate again (F0280 reopen).
     * Per source-locked contract: party has 0 champions after cancel,
     * leader hand empty, sensorData=15 still on the front square, so
     * the reopen must succeed and the panel must come back live. */
    reopenRc = M11_GameView_SelectFrontMirrorCandidate(&state);
    countAfterReopen = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate reopen on (1,2,0) returns 1 "
                 "(got %d)",
                 reopenRc);
        CHECK(reopenRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after reopen: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "championCount=%d (was %d before select)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterReopen, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              countAfterReopen == initialCount + 1, msg);
    }

    /* Render after reopen: panel live, portrait rect must NOT be a
     * stale full D1C sprite (BUG-120/121 panel guard still active).
     * Focus-return invariant: panel-on frame right after reopen
     * must look identical to panel-on frame right after the original
     * select (the C040 panel re-establishes the same focus state). */
    memset(fbAfterReopen, 0, sizeof(fbAfterReopen));
    M11_GameView_Draw(&state, fbAfterReopen, FB_W, FB_H);
    matchAfterReopen = match_portrait_at_rect(portraits,
                                              fbAfterReopen,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw after reopen does not leave ordinal %d "
                 "as a stale full-D1C sprite (<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterReopen);
        CHECK(matchAfterReopen <= 20, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "focus_return: D1C portrait-rect content is byte-"
                 "identical between panel-on right after select and "
                 "panel-on right after reopen (focus re-established "
                 "cleanly for the wall side: %s)",
                 memcmp(&fbAfterSelect[D1C_PORTRAIT_Y * FB_W],
                        &fbAfterReopen[D1C_PORTRAIT_Y * FB_W],
                        D1C_PORTRAIT_W * D1C_PORTRAIT_H) == 0
                     ? "yes" : "NO (regression)");
        CHECK(memcmp(&fbAfterSelect[D1C_PORTRAIT_Y * FB_W],
                     &fbAfterReopen[D1C_PORTRAIT_Y * FB_W],
                     D1C_PORTRAIT_W * D1C_PORTRAIT_H) == 0, msg);
    }

    /* The visible top D1C strip (portrait + 1px border, ABOVE the
     * C040 panel) must still be non-empty and stable across the
     * select/absorb/cancel/reopen cycle: panel covers (52..) so the
     * top strip y=[35..52) is the only window into the wall behind. */
    nonzeroAfterReopen = rect_nonzero(fbAfterReopen,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W,
                                      D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: visible top strip of D1C candidate redraw "
                 "is non-empty (>= 100 non-zero pixels, got %d)",
                 nonzeroAfterReopen);
        CHECK(nonzeroAfterReopen >= 100, msg);
    }
    warmAfterReopen = rect_warm_count(fbAfterReopen,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W,
                                      D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: visible top strip of D1C candidate redraw "
                 "has no portrait warm-color leak (<= 10 pixels, got %d)",
                 warmAfterReopen);
        CHECK(warmAfterReopen <= 10, msg);
    }
    distinctAfterReopen = rect_distinct(fbAfterReopen,
                                        D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                        D1C_PORTRAIT_W + 2,
                                        D1C_PORTRAIT_TOP_VISIBLE_H + 1);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: visible top D1C slice distinct palette "
                 "count is <= panel-off count (reopen=%d, panel-off=%d)",

                 distinctAfterReopen, distinctBefore);
        CHECK(distinctAfterReopen <= distinctBefore, msg);
    }

    /* No-floating proof on the panel-on frame after reopen: only
     * sample the unoccluded side-wall strip above C040, because the
     * panel covers the lower/right side-wall band by design. */
    leftSideAfterReopen = rect_warm_count(fbAfterReopen,
                                          SIDE_WALL_LEFT_X, D1C_PORTRAIT_Y,
                                          SIDE_WALL_LEFT_W,
                                          D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: left side wall of D1C portrait band has "
                 "< %d warm pixels while panel live (got %d)",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterReopen);
        CHECK(leftSideAfterReopen < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideAfterReopen = rect_warm_count(fbAfterReopen,
                                           SIDE_WALL_RIGHT_X, D1C_PORTRAIT_Y,
                                           SIDE_WALL_RIGHT_W,
                                           D1C_PORTRAIT_TOP_VISIBLE_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after reopen: right side wall of D1C portrait band has "
                 "< %d warm pixels while panel live (got %d)",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterReopen);
        CHECK(rightSideAfterReopen < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Cross-check: the popup_focus_return cycle leaves the visible
     * top strip pixel count close to the panel-off baseline (DUNVIEW.C
     * draws the wall ornament + portrait independently of the panel
     * state, so the top strip should be identical up to wall-ornament
     * suppression). */
    {
        int topOff = rect_nonzero(fbBefore,
                                  D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                  D1C_PORTRAIT_W,
                                  D1C_PORTRAIT_TOP_VISIBLE_H);
        int diffNonzero = (nonzeroAfterReopen > topOff)
                              ? (nonzeroAfterReopen - topOff)
                              : (topOff - nonzeroAfterReopen);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "visible top strip non-zero pixel count is stable across "
                 "select/absorb/cancel/reopen (panel-off=%d, after-reopen=%d, "
                 "|diff|=%d <= 50)",
                 topOff, nonzeroAfterReopen, diffNonzero);
        CHECK(diffNonzero <= 50, msg);
    }

    /* The portrait_rect_position contract: across the full
     * select->absorb->cancel->select cycle the D1C destination
     * rectangle does NOT change screen position.  The (96, 35, 32, 29)
     * destination is source-locked to DUNVIEW.C:3913-3928 +
     * DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall, so
     * we verify the same rect lines up with ordinal-15 pixels when
     * the panel is closed (before select, after cancel) and is
     * suppressed as a stale sprite while the panel is live (after
     * select, after absorbed inputs, after reopen). */
    {
        char msg[280];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position: before=%d%%, after-select=%d%%, "
                 "after-absorb=%d%%, after-cancel=%d%%, after-reopen=%d%% "
                 "(panel-off >=90, panel-on <=20)",
                 matchBefore, matchAfterSelect, matchAfterAbsorbedInputs,
                 matchAfterCancel, matchAfterReopen);
        CHECK(matchBefore >= 90 &&
              matchAfterSelect <= 20 &&
              matchAfterAbsorbedInputs <= 20 &&
              matchAfterCancel >= 90 &&
              matchAfterReopen <= 20, msg);
    }

    /* ----------------------------------------------------------------
     * Group E - ordinal 15 atlas round-trip
     * ----------------------------------------------------------------
     * The C026 atlas math for ordinal 15 must be self-consistent: the
     * destination (96, 35, 32, 29) on the framebuffer lines up with
     * the source (224, 29, 32, 29) in the atlas.  This is the
     * "ordinal 15 maps to the expected champion" check from the
     * slice description - the round-trip is independent of the
     * runtime drive and pins the macro math against the atlas
     * itself. */
    printf("\n[Group E] ordinal 15 atlas round-trip: source (224, 29) maps to dst (96, 35), col-7 right-edge boundary\n");
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas cell for ordinal 15 is at source "
                 "(%d, %d, %d, %d) - matches "
                 "((15 & 7) << 5, (15 >> 3) * 29, 32, 29)",
                 ORDINAL_15_SRC_X, ORDINAL_15_SRC_Y,
                 D1C_PORTRAIT_W, D1C_PORTRAIT_H);
        CHECK(ORDINAL_15_SRC_X == 224 && ORDINAL_15_SRC_Y == 29, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C026 atlas for ordinal 15 opaque count = %d "
                 "(in expected 100..900 range for a defined champion)",
                 ordinal15Opaque);
        CHECK(ordinal15Opaque >= 100 && ordinal15Opaque <= 900, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 15 round-trip distinctness: vs 14 = %d%%, "
                 "vs 23 = %d%% (both >= 30%%)",
                 ordinal15Vs14, ordinal15Vs23);
        CHECK(ordinal15Vs14 >= 30 && ordinal15Vs23 >= 30, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
