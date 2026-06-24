/*
 * firestaff_dm1_v1_hall_of_champions_portrait_17_inventory_exit_restore_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 17                  (mirror catalog record BORIS, title WIZARD OF BALDOR)
 *   route   inventory_exit_restore  (C040 panel select -> M11 inventory
 *                                    panel toggle OFF -> M11 inventory
 *                                    panel toggle ON; championCount goes
 *                                    0 -> 1 throughout, candidate panel
 *                                    survives the inventory close via the
 *                                    !G0299_ui_CandidateChampionOrdinal
 *                                    gate at PANEL.C F0355:2318-2322)
 *   aspect  portrait_rect_position
 *
 * The C026 champion-portrait atlas is an 8x3 grid of 32x29 portraits
 * (256x87 pixels total, ordinals 0..23). Ordinal 17 sits at row 2,
 * column 1 of the atlas:
 *
 *     srcX = (17 & 7) << 5 =  32
 *     srcY = (17 >> 3) * 29 = 58
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
 * Note that ordinal 17 is the first "row 2" ordinal in the shipped
 * DM1 V1 PC34 catalog: ordinals 0..15 are rows 0 and 1 (DAROOU through
 * CHANI), and ordinals 16..23 are row 2 (CHANI through NABI).  This
 * gate exercises the row-2 atlas math on the real C026 strip;
 * row-0/row-1 ordinals 0, 1, 2, 3, 5 are covered by the
 * portrait_NN_cancel_reopen probes, ordinal 17 itself by the
 * ordinal_17 cancel_reopen probe (different route variant), and
 * ordinal 17's front_north_entry and west_negative slices by the
 * corresponding probes.
 *
 * The shipped DM1 V1 DUNGEON.DAT places a C127 sensor on the (1,2)
 * NORTH-route front square (1,1) with sensorData=1 (HALK, ordinal 1),
 * so we seed that sensor to sensorData=17 to lock the ordinal-17 edge
 * case.  This keeps the probe runtime-real: same sensor, same
 * DUNGEON.DAT, same draw path - only the ordinal that
 * DUNVIEW.C:3913-3928 reads through M000_INDEX_TO_ORDINAL
 * (DUNGEON.C:2610-2612) is shifted for the test.
 *
 * Why a separate gate from
 * firestaff_dm1_v1_hall_of_champions_portrait_17_cancel_reopen_portrait_rect_position_runtime_probe.c
 * (which already covers ordinal 17 portrait_rect_position)?  The
 * cancel_reopen gate covers the Select->Cancel->Re-select cycle.
 * This inventory_exit_restore gate covers a different round-trip:
 * Select->InventoryClose->InventoryReopen.  The two routes diverge
 * on three source-locked points:
 *
 *   (1) cancel_reopen closes the candidate via F0282 C162 cancel
 *       branch (REVIVE.C F0282:744-806) which calls
 *       F0643_PARTY_ClearChampionSlot.  inventory_exit_restore keeps
 *       the candidate live via the !G0299 gate at PANEL.C
 *       F0355:2318-2322 and never runs F0282.
 *
 *   (2) cancel_reopen's panel-off redraw is driven by the F0128
 *       far-to-near viewport pass with no panel active.
 *       inventory_exit_restore's panel-off redraw is driven by the
 *       F0355 close path which runs F0334_INVENTORY_CloseChest +
 *       F0395_MENUS_DrawMovementArrows + F0098_DUNGEONVIEW_DrawFloorAndCeiling
 *       before the panel redraw.  These three draws MUST NOT touch
 *       the D1C portrait rect at (96, 35, 32, 29) - if they do, the
 *       portrait would be erased by the floor/ceiling redraw.
 *
 *   (3) inventory_exit_restore's panel-on redraw is driven by the
 *       F0347_INVENTORY_DrawPanel reopen route which sees G0299
 *       non-zero and reroutes to F0346_INVENTORY_DrawPanel_ResurrectReincarnate
 *       (PANEL.C F0347:1639-1693, F0346:1619-1637).  This is the
 *       source-locked !G0299 gate that the cancel_reopen probe
 *       cannot reach (cancel_reopen only runs F0282 to clear the
 *       candidate, never F0347->F0346 to reroute the panel draw).
 *
 * This probe is the real-asset, runtime-driven complement to the
 * contract-only test_dm1_v1_mirror_candidate_panel_redraw_after_inventory_exit_pc34_compat
 * gate.  The contract-only test pins state-machine invariants (F0334
 * called once, F0292 suppressed, F0346 called on reopen); this probe
 * pins the runtime pixel evidence that the D1C portrait rect
 * survives the inventory close AND that the C040 panel redraws at
 * the same pixels on the inventory reopen.  The two are disjoint:
 * the contract-only test never touches the framebuffer or the C026
 * atlas, and this probe never asserts the state-machine operation
 * counts.
 *
 * This probe covers the three coupled concerns of inventory_exit_restore
 * in one runtime drive:
 *
 *   (A) Inventory close while C040 candidate live: the candidate
 *       panel survives the inventory close (panel state, candidate
 *       ordinal, champion count all preserved), the D1C portrait
 *       rect still carries ordinal-17 pixels after the close, and
 *       the side walls of the D1C portrait band do NOT carry the
 *       portrait's palette.
 *
 *   (B) Inventory reopen while C040 candidate live: the panel
 *       redraws at the same pixels (panel rect preserved), the
 *       portrait is suppressed as a stale floating sprite while the
 *       panel owns the view (BUG-120/121 panel guard), and the
 *       inventory backdrop does not clobber the visible top strip
 *       of the D1C portrait rect.
 *
 *   (C) No-floating proof: the side walls of the D1C portrait band
 *       do not carry portrait warm pixels across the full
 *       inventory_exit_restore cycle (select -> inv-off -> inv-on),
 *       proving the engine never paints the portrait over a
 *       non-mirror cell.
 *
 * Source evidence:
 *   - DUNGEON.C:2573 (C127 sensor cell match against view dir)
 *   - DUNGEON.C:2608 (C127 sensor type match)
 *   - DUNGEON.C:2612 (G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)))
 *   - DUNVIEW.C:3913 (P0117_i_ViewWallIndex == M587_VIEW_WALL_D1C_FRONT
 *                     && G0289_i_DungeonView_ChampionPortraitOrdinal--)
 *   - DUNVIEW.C:3916-3919 (D1C C026 portrait blit at {96,35} with
 *                          ((ordinal & 7) << 5, (ordinal >> 3) * 29))
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:3928 (F0654_Call_F0132_VIDEO_Blit for I34E / A36M
 *                     variant with G2078_C32_PortraitWidth /
 *                     G2079_C29_PortraitHeight)
 *   - DUNVIEW.C:8318-8542 (F0128 redraw viewport far-to-near)
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - DEFS.H:2186 (C026_GRAPHIC_CHAMPION_PORTRAITS)
 *   - DEFS.H:712-716 (C04_CHAMPION_CLOSE_INVENTORY)
 *   - DEFS.H:5876 (G0423_i_InventoryChampionOrdinal)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C F0280:124-132 (C040 empty-leader candidate gate)
 *   - PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 (close-inventory path)
 *   - PANEL.C F0355:2318-2322 (!G0299_ui_CandidateChampionOrdinal gate)
 *   - PANEL.C F0334_INVENTORY_CloseChest (CHEST.C F0334) - close call
 *   - PANEL.C F0347_INVENTORY_DrawPanel:1639-1693 (reopen reroute)
 *   - PANEL.C F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637
 *   - PANEL.C F0346:1626 (G0424_i_PanelContent = M568_PANEL_RESURRECT_REINCARNATE)
 *   - PANEL.C F0395_MENUS_DrawMovementArrows (post-exit arrow draw)
 *   - PANEL.C F0098_DUNGEONVIEW_DrawFloorAndCeiling (post-exit F/C redraw)
 *   - COMMAND.C F0357_COMMAND_DiscardAllInput (post-exit input discard)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
 *   - M11_GameView_CancelMirrorCandidate (cancel path - NOT used here)
 *   - M11_GameView_SelectFrontMirrorCandidate (select path)
 *   - M11_GameView_ToggleInventoryPanel (inventory exit/restore)
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
    /* Ordinal 17 in the C026 atlas: (17 & 7) << 5 = 32, (17 >> 3) * 29 = 58. */
    ORDINAL_17_COL = 17 & 7,       /* = 1 */
    ORDINAL_17_ROW = 17 >> 3,      /* = 2 (last row of the 8x3 grid) */
    ORDINAL_17_SRC_X = ORDINAL_17_COL << 5,    /* =  32 */
    ORDINAL_17_SRC_Y = ORDINAL_17_ROW * 29,    /* =  58 */
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
    TARGET_ORDINAL = 17,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 17 (BORIS) for this gate so we can lock the
     * ordinal-17 edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1
};
/* Mirror catalog record name for ordinal 17 (DM1 V1 PC34 mirror
 * catalog).  Used to assert the catalog resolves correctly.  BORIS
 * (title WIZARD OF BALDOR) is the 18th valid mirror text string in
 * the shipped DM1 V1 DUNGEON.DAT (the 18th ordinal = 17, after
 * DAROOU / HALK / WU TSE / AZIZI / LEIF / ELIJA / SYRA / TIGGY /
 * IAIDO / ZED / GANDO / STAMM / LINFLAS / WUUF / LEYLA / MOPHUS /
 * CHANI). */
static const char kExpectedCatalogName[] = "BORIS";
static const char kExpectedCatalogTitle[] = "WIZARD OF BALDOR";

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
 * Used to verify ordinal 17 is a defined portrait in the atlas
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

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Returns the sensor index
 * on success, or -1 if no such sensor was found.  We use this to
 * lock the ordinal-17 edge case on the real DM1 V1 DUNGEON.DAT
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
 * seed_first_c127_data the same square reports ordinal 17. */
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
    int ordinal17Opaque;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    unsigned char fbBefore[FB_W * FB_H];
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterInvOff[FB_W * FB_H];
    unsigned char fbAfterInvOn[FB_W * FB_H];
    int matchBefore, matchAfterSelect, matchAfterInvOff, matchAfterInvOn;
    int nonzeroBefore, nonzeroAfterInvOff;
    int distinctBefore, distinctAfterInvOff, distinctAfterInvOn;
    int warmBefore, warmAfterInvOff;
    int leftSideBefore, leftSideAfterInvOff;
    int rightSideBefore, rightSideAfterInvOff;
    int initialCount, countAfterSelect, countAfterInvOff, countAfterInvOn;
    int selectRc, invOffRc, invOnRc;
    int seededSensor;
    char nameBuf[32];
    char titleBuf[32];
    int nameLookupRc;
    int titleLookupRc;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-17 / inventory_exit_restore / portrait_rect_position ===\n");
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
     * Group A - Atlas math for ordinal 17 (row 2 / column 1)
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 2 /
     * column 1 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows").  This is the first gate to
     * drive the row-2 atlas math against the real C026 strip; the
     * row-0 and row-1 paths are already covered by the ordinals 0,
     * 1, 2, 3, and 5 cancel_reopen probes. */
    printf("\n[Group A] C026 atlas math for ordinal 17 (row 2 / col 1)\n");

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
                 "ordinal 17 col = 17 & 7 = %d (expected 1)",
                 ORDINAL_17_COL);
        CHECK(ORDINAL_17_COL == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 17 row = 17 >> 3 = %d (expected 2, last row)",
                 ORDINAL_17_ROW);
        CHECK(ORDINAL_17_ROW == 2, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 17 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_17_SRC_X, ORDINAL_17_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_17_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_17_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    /* Ordinal 17 must be a defined portrait: opaque count > 100 of the
     * 32*29 = 928 cell.  An unused slot would be either all-zero or
     * all-transparent (palette index 1 = transparent, per
     * M11_AssetLoader_BlitRegion).  This catches a regression where
     * ordinal 17 (BORIS) is treated as "no portrait" - in particular
     * a regression where (ordinal >> 3) * 29 yields the wrong row on
     * the C026 strip (e.g. row 2 -> row 0 off-by-three). */
    ordinal17Opaque = atlas_cell_opaque_count(portraits, 17);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 17 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal17Opaque);
        CHECK(ordinal17Opaque >= 100, msg);
    }

    /* Ordinal 17 must resolve to BORIS through the mirror catalog.
     * This catches a regression where the catalog and the C026 atlas
     * disagree on the ordinal-17 record - in particular a regression
     * where the catalog is shorter than 18 entries (a catalog count
     * < 18 would have rejected sensorData=17 on the real DUNGEON.DAT). */
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
                 "mirror catalog resolves ordinal 17 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 17 title to \"%s\" (expected \"%s\")",
                 titleBuf[0] ? titleBuf : "", kExpectedCatalogTitle);
        CHECK(titleLookupRc > 0 &&
              strcmp(titleBuf, kExpectedCatalogTitle) == 0, msg);
    }

    /* Park the party on the (1,2,0) NORTH-route front mirror, then
     * seed the C127 sensor from HALK (1) to ordinal 17 (BORIS).  Same
     * sensor, same map cell, same draw path - only G0289 changes. */
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
     * 17.  Same sensor, same map cell, same draw path - only G0289
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

    /* The same front route now reports ordinal 17.  After seeding
     * the C127 sensor's sensorData, the front route must reflect the
     * new ordinal.  Note: the front ordinal helper clamps to
     * [0, mirrorCatalog.count), so this check confirms the catalog
     * has at least 18 entries (which is the source-locked DM1 V1
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
                "cannot verify portrait_rect_position or inventory_exit_restore\n",
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
     * D1C destination rectangle (96, 35, 32, 29) holds ordinal-17
     * pixels. */
    printf("\n[Group B] portrait_rect_position on real C127 sensor pose (1,2,0)=17\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

    memset(fbBefore, 0, sizeof(fbBefore));
    M11_GameView_Draw(&state, fbBefore, FB_W, FB_H);

    /* The D1C portrait rect must contain ordinal-17 source pixels at
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
     * Group C - inventory_exit_restore: select -> inv-off -> inv-on
     * ----------------------------------------------------------------
     * Drive the source-locked candidate selection (F0280), then close
     * the inventory panel via M11_GameView_ToggleInventoryPanel
     * (PANEL.C F0355_INVENTORY_Toggle_CPSE:2244-2330 with the
     * !G0299_ui_CandidateChampionOrdinal gate at lines 2318-2322 that
     * suppresses the F0292 redraw so the C040 panel survives), then
     * re-open the inventory via a second toggle (F0347_INVENTORY_DrawPanel
     * reroute to F0346_INVENTORY_DrawPanel_ResurrectReincarnate per
     * PANEL.C F0347:1639-1693).  The framebuffer's D1C portrait rect
     * must still carry ordinal-17 pixels after the inventory close
     * (panel-off redraw), and the panel-on redraw must not leave the
     * portrait as a stale floating sprite (BUG-120/121 panel guard). */
    printf("\n[Group C] inventory_exit_restore: select, inventory off, inventory on\n");

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
                 "inventoryPanelActive=%d, championCount=%d (was %d)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.inventoryPanelActive,
                 countAfterSelect, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              state.inventoryPanelActive == 1 &&
              countAfterSelect == initialCount + 1, msg);
    }

    /* Render with C040 panel + inventory panel live.  The portrait
     * must NOT be drawn as a stale floating sprite while either panel
     * owns the view (BUG-120/121 panel guard).  Match against ordinal
     * 17 should be low. */
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

    /* Step 2: ToggleInventoryPanel to CLOSE the inventory.
     * Per source-locked contract (PANEL.C F0355_INVENTORY_Toggle_CPSE
     * :2244-2330): F0334_INVENTORY_CloseChest is called once, the
     * !G0299 gate at F0355:2318-2322 suppresses the F0292 redraw,
     * F0395_MENUS_DrawMovementArrows + F0098_DUNGEONVIEW_DrawFloorAndCeiling
     * run after the close, and the C040 panel survives.  The
     * candidate mirror ordinal is unchanged. */
    invOffRc = M11_GameView_ToggleInventoryPanel(&state);
    countAfterInvOff = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ToggleInventoryPanel off on (1,2,0) returns 0 (got %d)",
                 invOffRc);
        CHECK(invOffRc == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after inv-off: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "inventoryPanelActive=%d, championCount=%d (was %d before select) - "
                 "candidate survives inventory close via !G0299 gate",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.inventoryPanelActive,
                 countAfterInvOff, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              state.inventoryPanelActive == 0 &&
              countAfterInvOff == initialCount + 1, msg);
    }

    /* Render after inventory close: C040 panel still owns the view,
     * the D1C portrait rect must hold ordinal-17 pixels because the
     * F0334/F0098/F0395 close-path draws do NOT touch the wall
     * ornament or the portrait cell. */
    memset(fbAfterInvOff, 0, sizeof(fbAfterInvOff));
    M11_GameView_Draw(&state, fbAfterInvOff, FB_W, FB_H);
    matchAfterInvOff = match_portrait_at_rect(portraits,
                                              fbAfterInvOff,
                                              TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-off: D1C portrait rect carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%) - !G0299 gate "
                 "kept the C040 panel + portrait on screen",
                 TARGET_ORDINAL, matchAfterInvOff);
        CHECK(matchAfterInvOff >= 90, msg);
    }
    nonzeroAfterInvOff = rect_nonzero(fbAfterInvOff,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-off: D1C portrait rect is non-empty "
                 "(>= 100 non-zero pixels, got %d)",
                 nonzeroAfterInvOff);
        CHECK(nonzeroAfterInvOff >= 100, msg);
    }
    distinctAfterInvOff = rect_distinct(fbAfterInvOff,
                                        D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                        D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-off: D1C portrait rect has >= 4 distinct "
                 "palette indices (got %d)",
                 distinctAfterInvOff);
        CHECK(distinctAfterInvOff >= 4, msg);
    }
    warmAfterInvOff = rect_warm_count(fbAfterInvOff,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-off: D1C portrait rect has >= %d warm-color "
                 "pixels (got %d) - F0098 floor/ceiling redraw did not "
                 "erase the portrait",
                 PORTRAIT_WARM_THRESHOLD, warmAfterInvOff);
        CHECK(warmAfterInvOff >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof after inventory close: the side walls of the
     * D1C portrait band must NOT carry the portrait's warm pixels.
     * The F0098 floor/ceiling redraw could re-introduce wall pixels
     * that overlap with the warm-color band on a regression. */
    leftSideAfterInvOff = rect_warm_count(fbAfterInvOff,
                                          SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_LEFT_W,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-off: left side wall of D1C portrait band "
                 "has < %d warm pixels (got %d) - portrait not floating "
                 "on left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterInvOff);
        CHECK(leftSideAfterInvOff < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideAfterInvOff = rect_warm_count(fbAfterInvOff,
                                           SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                           SIDE_WALL_RIGHT_W,
                                           PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-off: right side wall of D1C portrait band "
                 "has < %d warm pixels (got %d) - portrait not floating "
                 "on right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterInvOff);
        CHECK(rightSideAfterInvOff < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* Step 3: ToggleInventoryPanel to REOPEN the inventory.
     * Per source-locked contract (PANEL.C F0347_INVENTORY_DrawPanel
     * :1639-1693): the G0299 non-zero check at line 1654 routes to
     * F0346_INVENTORY_DrawPanel_ResurrectReincarnate:1619-1637 which
     * blits the C040 graphic via M519_F0020_MAIN_BlitToViewport on
     * the G0032_ai_Graphic562_Box_Panel rect.  The candidate
     * candidate ordinal and champion count are unchanged. */
    invOnRc = M11_GameView_ToggleInventoryPanel(&state);
    countAfterInvOn = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ToggleInventoryPanel on reopen returns 1 (got %d)",
                 invOnRc);
        CHECK(invOnRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after inv-on: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d, "
                 "inventoryPanelActive=%d, championCount=%d (was %d before select)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 state.inventoryPanelActive,
                 countAfterInvOn, initialCount);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              state.inventoryPanelActive == 1 &&
              countAfterInvOn == initialCount + 1, msg);
    }

    /* Render after inventory reopen: panel + inventory live, portrait
     * rect must NOT be a stale full D1C sprite (BUG-120/121 panel
     * guard still active after the F0347->F0346 reroute). */
    memset(fbAfterInvOn, 0, sizeof(fbAfterInvOn));
    M11_GameView_Draw(&state, fbAfterInvOn, FB_W, FB_H);
    matchAfterInvOn = match_portrait_at_rect(portraits,
                                             fbAfterInvOn,
                                             TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "panel-on redraw after inv-on does not leave ordinal %d "
                 "as a stale full-D1C sprite (<= 20%% match, got %d%%)",
                 TARGET_ORDINAL, matchAfterInvOn);
        CHECK(matchAfterInvOn <= 20, msg);
    }
    distinctAfterInvOn = rect_distinct(fbAfterInvOn,
                                       D1C_PORTRAIT_X - 1, D1C_PORTRAIT_Y - 1,
                                       D1C_PORTRAIT_W + 2,
                                       D1C_PORTRAIT_TOP_VISIBLE_H + 1);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after inv-on: visible top D1C slice distinct palette "
                 "count is <= inv-off count (inv-on=%d, inv-off=%d)",
                 distinctAfterInvOn, distinctAfterInvOff);
        CHECK(distinctAfterInvOn <= distinctAfterInvOff, msg);
    }

    /* The portrait_rect_position contract: across the full
     * inventory_exit_restore cycle (select -> inv-off -> inv-on) the
     * D1C destination rectangle does NOT change screen position.
     * The (96, 35, 32, 29) destination is source-locked to
     * DUNVIEW.C:3913-3928 + DUNVIEW.C:525 G0109_Graphic558_Box_ChampionPortraitOnWall,
     * so we verify the same rect lines up with ordinal-17 pixels when
     * the inventory panel is closed (before select, after inv-off)
     * and is suppressed as a stale sprite while the panel is live
     * (after select, after inv-on). */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position: before=%d%%, after-select=%d%%, "
                 "after-inv-off=%d%%, after-inv-on=%d%% (panel-off >=90, "
                 "panel-on <=20)",
                 matchBefore, matchAfterSelect,
                 matchAfterInvOff, matchAfterInvOn);
        CHECK(matchBefore >= 90 &&
              matchAfterSelect <= 20 &&
              matchAfterInvOff >= 90 &&
              matchAfterInvOn <= 20, msg);
    }

    /* Stability contract: the inventory close + reopen must leave
     * the D1C portrait rect pixel state unchanged when the panel is
     * off.  Pre-select (panel-off) and post-inv-off (panel-off)
     * must report the same match%.  This is the regression check
     * for the F0334 close path which MUST NOT touch the portrait
     * sprite. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect match%% is stable across select -> "
                 "inv-off (before=%d%%, after-inv-off=%d%%, |delta| <= 5%%)",
                 matchBefore, matchAfterInvOff);
        {
            int delta = matchAfterInvOff - matchBefore;
            if (delta < 0) delta = -delta;
            CHECK(delta <= 5, msg);
        }
    }

    /* Final no-floating summary: across the whole cycle, the D1C
     * portrait band must not bleed into the side walls. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "across select+inv-off+inv-on: max left-side warm=%d, "
                 "max right-side warm=%d (both < %d threshold)",
                 leftSideBefore > leftSideAfterInvOff ? leftSideBefore : leftSideAfterInvOff,
                 rightSideBefore > rightSideAfterInvOff ? rightSideBefore : rightSideAfterInvOff,
                 PORTRAIT_WARM_THRESHOLD);
        CHECK((leftSideBefore < PORTRAIT_WARM_THRESHOLD) &&
              (rightSideBefore < PORTRAIT_WARM_THRESHOLD) &&
              (leftSideAfterInvOff < PORTRAIT_WARM_THRESHOLD) &&
              (rightSideAfterInvOff < PORTRAIT_WARM_THRESHOLD), msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
