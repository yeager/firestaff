/*
 * firestaff_dm1_v1_hall_of_champions_portrait_09_candidate_panel_cancel_portrait_rect_position_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 9              (mirror catalog record ZED,
 *                           title "DUKE OF BANVILLE")
 *   route   candidate_panel_cancel (C040 panel select -> F0282 C162 cancel
 *                                   branch, no follow-up re-select; the
 *                                   2-step terminal slice that proves the
 *                                   cancel arm cleans the candidate-panel
 *                                   state machine back to idle)
 *   aspect  portrait_rect_position
 *
 * The C026 champion-portrait atlas is an 8x3 grid of 32x29 portraits
 * (256x87 pixels total, ordinals 0..23).  Ordinal 9 sits at row 1,
 * column 1 of the atlas - the SECOND cell of the second row, directly
 * to the right of ordinal 8 (IAIDO):
 *
 *     srcX = (9 & 7) << 5 =  32
 *     srcY = (9 >> 3) * 29 =  29
 *
 * The D1C front-wall champion-portrait destination rectangle is
 * source-locked (per ReDMCSB DUNGEON.C:2573 + MOVESENS.C:1501-1503 +
 * DUNVIEW.C:3913-3928 + COORD.C:1693-1749 + DUNVIEW.C:525
 * G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96, 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * The shipped DM1 V1 DUNGEON.DAT places a C127 sensor on the (1,2)
 * NORTH-route front square (1,1) with sensorData=1 (HALK), so we
 * seed that sensor to sensorData=9 to lock the ordinal-9 edge case.
 * This keeps the probe runtime-real: same sensor, same DUNGEON.DAT,
 * same draw path - only the ordinal is shifted for the test.
 *
 * Why a separate slice from
 * firestaff_dm1_v1_champion_mirror_ordinal_9_south_return_portrait_rect_position_runtime_probe.c
 * and from the candidate_panel_runtime_probe ordinal-20 redraw pose:
 *
 *   - The south_return ordinal-9 sibling proves the
 *     portrait_rect_position when the (1,5) DIR_SOUTH front cell is
 *     seeded to ordinal 9 (ZED), but it never opens the C040
 *     candidate panel.  Candidate-panel open + cancel is a separate
 *     state-machine path: REVIVE.C F0280 appends the candidate
 *     (panel-on), PANEL.C F0346 / F0347 draws the modal C040 panel,
 *     REVIVE.C F0282 C162 cancel clears the panel and the appended
 *     champion.  This slice drives that 2-step terminal sequence
 *     for ordinal 9.
 *
 *   - The candidate_panel_runtime_probe ordinal-20 redraw pose
 *     (Pose A0 in main) does the same 2-step terminal cancel for
 *     ordinal 20.  This slice is the same shape with the same
 *     invariants, specialized to ordinal 9, so the
 *     portrait_rect_position contract for the 2-step terminal
 *     cancel arm is locked for one more ordinal (and one more
 *     distinct atlas cell - row 1, col 1, between ordinal 8 in the
 *     same row and ordinal 1 directly above).
 *
 *   - The candidate_panel_cancel invariants that this slice locks:
 *     (a) tear down inventoryPanelActive (BUG-120/121 panel guard)
 *     (b) reset activeChampionIndex when the canceled slot was the
 *         only champion (F0643_PARTY_ClearChampionSlot_Compat side
 *         effect, ReDMCSB REVIVE.C:744-783)
 *     (c) clear the candidate champion slot (F0643 + F0600 zero-init,
 *         including .present=0 and the packed name bytes)
 *     (d) re-paint the D1C portrait rect at full pixel match after
 *         the panel is gone (BUG-120 panel guard releases on
 *         candidateMirrorPanelActive=0)
 *     (e) re-paint the side walls cleanly (no panel-frame warm pixels
 *         left behind from the C040 panel chrome)
 *     (f) write the F0282 C162 cancel sentinel into
 *         lastAction/lastOutcome/inspectTitle/inspectDetail AND NOT
 *         leak the candidate's name ("ZED") into the inspect
 *         readout (the select arm sets inspectTitle to "MIRROR: ZED";
 *         cancel must overwrite that with "CHAMPION MIRROR" and
 *         inspectDetail with "SELECTION CANCELLED")
 *     (g) leave the C127 sensorData unchanged (cancel does NOT
 *         mutate the route - m11_disable_front_mirror_route is only
 *         called from ConfirmMirrorCandidate, ReDMCSB REVIVE.C F0282
 *         C160 resurrect path)
 *
 *   cancel_reopen covers (a)+(d) implicitly and (c) partially, but
 *   the third select arm masks (b)+(c) by re-creating the candidate,
 *   and the cancel_reopen probe does not sample side-wall warm
 *   pixels after cancel.  This slice closes those gaps for the
 *   ordinal-9 + (1,2) NORTH-route pose.  cancel_reopen for ordinal
 *   9 is NOT in the existing series, so this slice is also the
 *   first ordinal-9 candidate-panel cancel gate.
 *
 * Sibling contracts (do not duplicate):
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe       (16-pose ordinal map)
 *   firestaff_dm1_v1_champion_mirror_ordinal_9_south_return_portrait_rect_position_runtime_probe
 *                                                                       (south_return route, no panel)
 *   firestaff_dm1_v1_champion_mirror_ordinal_9_west_negative_portrait_rect_position_runtime_probe
 *                                                                       (west_negative no-floating)
 *   firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe   (panel select/cancel/confirm coverage for
 *                                                                       the ordinal-2 reference route)
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
 *   - REVIVE.C F0282:744-806 (C162 cancel branch 744-783)
 *   - PANEL.C F0346:1619-1635 (C040 panel blit to C101)
 *   - PANEL.C F0347:1654-1656 (route to C040 while panel live)
 *   - PANEL.C F0355:2299-2318 (inventory close on cancel)
 *   - COMMAND.C F0378:1956-1990 (M568_PANEL_RESURRECT_REINCARNATE
 *                              dispatch)
 *   - F0643_PARTY_ClearChampionSlot_Compat (m11_game_view.c cancel path)
 *   - F0600_CHAMPION_InitEmpty_Compat (zero-init including .present=0)
 *   - m11_draw_dm1_front_mirror_route (BUG-120/121 panel guard)
 *   - M11_GameView_CancelMirrorCandidate (F0282 C162 cancel path)
 *   - M11_GameView_SelectFrontMirrorCandidate (F0280 select path)
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
    /* Ordinal 9 in the C026 atlas: (9 & 7) << 5 = 32, (9 >> 3) * 29 = 29.
     * Row 1, col 1 - between ordinal 8 (left, IAIDO) and ordinal 10
     * (right, GANDO) in the same row, and between ordinal 1 (above,
     * HALK) and ordinal 17 (below, BORIS) in the same column. */
    ORDINAL_9_COL = 9 & 7,         /* = 1 */
    ORDINAL_9_ROW = 9 >> 3,        /* = 1 */
    ORDINAL_9_SRC_X = ORDINAL_9_COL << 5,   /* =  32 */
    ORDINAL_9_SRC_Y = ORDINAL_9_ROW * 29,   /* =  29 */
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
    /* The C040 panel rectangle (centered chrome frame that overlays
     * the candidate portrait on the D1C wall).  This is the panel
     * "frame" band that must be empty after a terminal cancel. */
    C040_FRAME_X = VIEWPORT_X + 80,
    C040_FRAME_Y = VIEWPORT_Y + 52,
    C040_FRAME_W = 160,
    C040_FRAME_H = 28,
    /* Tight inner C040 panel-core sample (excluding the panel
     * border) used by the post-cancel residue check.  The cancel
     * path clears inventoryPanelActive, so the panel chrome must
     * be gone and the underlying wall texture (D1C wall ornament)
     * must be visible through this rectangle. */
    C040_INNER_X = C040_FRAME_X + 4,
    C040_INNER_Y = C040_FRAME_Y + 4,
    C040_INNER_W = C040_FRAME_W - 8,
    C040_INNER_H = C040_FRAME_H - 8,
    TARGET_ORDINAL = 9,
    /* The HALK ordinal (1) is what DM1 V1 DUNGEON.DAT ships on the
     * (1,2) NORTH-route front square (1,1).  We seed that sensor
     * to ordinal 9 for this gate so we can lock the ordinal-9
     * edge case without changing the map layout. */
    SHIPPED_HALK_ORDINAL = 1
};
/* Mirror catalog record name and title for ordinal 9 (DM1 V1 PC34
 * mirror catalog).  Used to assert the catalog resolves correctly
 * and to verify the cancel arm strips the candidate name from the
 * inspect readout. */
static const char kExpectedCatalogName[] = "ZED";
static const char kExpectedCatalogTitle[] = "DUKE OF BANVILLE";
/* F0282 C162 cancel sentinel strings written by
 * M11_GameView_CancelMirrorCandidate into lastAction/lastOutcome
 * and inspectTitle/inspectDetail (m11_game_view.c:8127-8129). */
static const char kCancelAction[] = "MIRROR";
static const char kCancelOutcome[] = "CANCELLED";
static const char kCancelInspectTitle[] = "CHAMPION MIRROR";
static const char kCancelInspectDetail[] = "SELECTION CANCELLED";

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
 * Used to verify ordinal 9 is a defined portrait in the atlas
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

/* Compute the byte-difference percent between two C026 atlas cells.
 * Used to verify ordinal 9 is a visually distinct portrait from its
 * atlas neighbours.  The DM1 champion-portrait atlas carries 24
 * distinct champions, so a duplicate would be a real regression. */
static int atlas_pair_distinct_pct(const M11_AssetSlot* portraits,
                                   int ordA, int ordB) {
    int srcAX, srcAY, srcBX, srcBY, x, y, compared = 0, different = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcAX = (ordA & 7) * D1C_PORTRAIT_W;
    srcAY = (ordA >> 3) * D1C_PORTRAIT_H;
    srcBX = (ordB & 7) * D1C_PORTRAIT_W;
    srcBY = (ordB >> 3) * D1C_PORTRAIT_H;
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
 * on success, or -1 if no such sensor was found.  We use this to
 * lock the ordinal-9 edge case on the real DM1 V1 DUNGEON.DAT
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
 * seed_first_c127_data the same square reports ordinal 9. */
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

/* Sum the packed-name bytes of the canceled champion slot.  The
 * F0643_PARTY_ClearChampionSlot_Compat path (m11_game_view.c:8117)
 * runs F0600_CHAMPION_InitEmpty_Compat which zeros every byte of
 * the slot including the 8-byte packed name.  This helper returns
 * the sum so the probe can lock the post-cancel slot is genuinely
 * zero-initialised rather than carrying residual mirror text. */
static int slot_packed_name_byte_sum(const M11_GameViewState* state,
                                     int slot) {
    int i, sum = 0;
    if (!state || slot < 0 || slot >= 4) return -1;
    for (i = 0; i < 8; ++i) {
        sum += state->world.party.champions[slot].name[i];
    }
    return sum;
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
    unsigned char fbAfterSelect[FB_W * FB_H];
    unsigned char fbAfterCancel[FB_W * FB_H];
    int matchBefore, matchAfterSelect, matchAfterCancel;
    int nonzeroBefore, nonzeroAfterCancel;
    int distinctBefore, distinctAfterCancel;
    int warmBefore, warmAfterCancel;
    int leftSideBefore, leftSideAfterCancel;
    int rightSideBefore, rightSideAfterCancel;
    int c040FrameNonzeroBefore;
    int c040InnerDistinctBefore;
    int initialCount, countAfterSelect, countAfterCancel;
    int selectRc, cancelRc;
    int sensorDataAfterCancel;
    int slotNameSumBefore;
    char nameBuf[32];
    char titleBuf[32];
    int nameLookupRc;
    int titleLookupRc;
    int ordinal9Opaque;
    int ordinal9Vs8;    /* left in same row (IAIDO) */
    int ordinal9Vs10;   /* right in same row (GANDO) */
    int ordinal9Vs1;    /* above in same column (HALK) */
    int seededSensorIdx;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-09 / candidate_panel_cancel / portrait_rect_position (v2.7.27) ===\n");
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
     * Group A - Atlas math for ordinal 9
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 1 /
     * column 1 and that the math matches COORD.C / DEFS.H:821-826.
     * The atlas dimensions and the 8x3 cell layout come from
     * DUNVIEW.C:3916-3919 (C026 is "256x87 strip of 32x29
     * portraits: 8 columns by 3 rows"). */
    printf("\n[Group A] C026 atlas math for ordinal 9\n");

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
                 "ordinal 9 col = 9 & 7 = %d (expected 1)",
                 ORDINAL_9_COL);
        CHECK(ORDINAL_9_COL == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 row = 9 >> 3 = %d (expected 1)",
                 ORDINAL_9_ROW);
        CHECK(ORDINAL_9_ROW == 1, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_9_SRC_X, ORDINAL_9_SRC_Y,
                 ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_9_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_9_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }

    /* Ordinal 9 must be a defined portrait: opaque count > 50% of the
     * 32*29 = 928 cell.  An unused slot would be either all-zero or
     * all-transparent (palette index 1 = transparent, per
     * M11_AssetLoader_BlitRegion).  This catches a regression where
     * ordinal 9 is treated as "no portrait" or the wrong atlas cell
     * is blitted at (32, 29, 32, 29). */
    ordinal9Opaque = atlas_cell_opaque_count(portraits, 9);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 atlas cell has >= 100 opaque pixels (got %d) "
                 "- defined portrait, not blank/unused",
                 ordinal9Opaque);
        CHECK(ordinal9Opaque >= 100, msg);
    }

    /* Ordinal 9 must be visually distinct from its atlas neighbours:
     *   - ordinal 8  (row 1, col 0 - IAIDO, immediately left)
     *   - ordinal 10 (row 1, col 2 - GANDO, immediately right)
     *   - ordinal 1  (row 0, col 1 - HALK, directly above)
     * The DM1 champion-portrait atlas carries 24 distinct champions
     * (one per ordinal), so a duplicate would be a real regression.
     * The col-stride check (9 vs 8 / 9 vs 10) catches a column
     * stride bug (e.g. 33-pixel columns would scramble neighbour
     * pixels); the cross-row check (9 vs 1) catches a row-stride
     * bug (e.g. 30-pixel rows). */
    ordinal9Vs8 = atlas_pair_distinct_pct(portraits, 9, 8);
    ordinal9Vs10 = atlas_pair_distinct_pct(portraits, 9, 10);
    ordinal9Vs1 = atlas_pair_distinct_pct(portraits, 9, 1);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 vs ordinal 8 (left, IAIDO) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal9Vs8);
        CHECK(ordinal9Vs8 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 vs ordinal 10 (right, GANDO) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal9Vs10);
        CHECK(ordinal9Vs10 >= 30, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 9 vs ordinal 1 (above, HALK) differ "
                 "by >= 30%% (got %d%%)",
                 ordinal9Vs1);
        CHECK(ordinal9Vs1 >= 30, msg);
    }

    /* Ordinal 9 must resolve to ZED / "DUKE OF BANVILLE" through the
     * mirror catalog.  This catches a regression where the catalog and
     * the C026 atlas disagree on the ordinal-9 record. */
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
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 9 to \"%s\" (expected \"%s\")",
                 nameBuf[0] ? nameBuf : "", kExpectedCatalogName);
        CHECK(nameLookupRc > 0 &&
              strcmp(nameBuf, kExpectedCatalogName) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 9 title to \"%s\" (expected \"%s\")",
                 titleBuf[0] ? titleBuf : "", kExpectedCatalogTitle);
        CHECK(titleLookupRc >= 0 &&
              strcmp(titleBuf, kExpectedCatalogTitle) == 0, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to ordinal
     * 9 (ZED).  Same sensor, same map cell, same draw path - only
     * G0289 shifts.  This keeps the probe runtime-real. */
    park_d1c_front_route(&state);
    seededSensor = seed_first_c127_data(&state,
                                         SHIPPED_HALK_ORDINAL,
                                         TARGET_ORDINAL);
    seededSensorIdx = seededSensor;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,2) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }
    if (seededSensor < 0) {
        fprintf(stderr,
                "FATAL: could not find a C127 sensor with sensorData=%d; "
                "cannot verify portrait_rect_position or candidate_panel_cancel\n",
                SHIPPED_HALK_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* The same front route now reports ordinal 9.  After seeding
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
                "cannot verify portrait_rect_position or candidate_panel_cancel\n",
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
     * Group B - portrait_rect_position panel-off baseline
     * ----------------------------------------------------------------
     * Render the framebuffer before any selection, and verify the
     * D1C destination rectangle (96, 35, 32, 29) holds ordinal-9
     * pixels.  This baseline is the reference for the post-cancel
     * re-paint invariants (Group C2). */
    printf("\n[Group B] portrait_rect_position baseline (panel-off, pre-select)\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;
    slotNameSumBefore = slot_packed_name_byte_sum(&state, 0);

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

    /* No-floating proof (panel-off): side walls of the D1C portrait
     * band must NOT carry the portrait's warm pixels. */
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

    /* C040 panel-frame baseline: with the panel CLOSED (panel-off
     * baseline), the rectangle where the panel chrome would render
     * (80, 52, 160, 28) is filled with whatever the wall-ornament
     * / dungeon view shows through.  After cancel the same rectangle
     * must restore to the same look (no chrome residue). */
    c040FrameNonzeroBefore = rect_nonzero(fbBefore,
                                          C040_FRAME_X, C040_FRAME_Y,
                                          C040_FRAME_W, C040_FRAME_H);
    c040InnerDistinctBefore = rect_distinct(fbBefore,
                                            C040_INNER_X, C040_INNER_Y,
                                            C040_INNER_W, C040_INNER_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C040 frame region baseline (panel-off) has %d non-zero "
                 "pixels and %d distinct palette indices (reference for "
                 "post-cancel residue check)",
                 c040FrameNonzeroBefore, c040InnerDistinctBefore);
        CHECK(c040FrameNonzeroBefore > 0 && c040InnerDistinctBefore > 0, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - candidate_panel_cancel: select -> cancel (terminal)
     * ----------------------------------------------------------------
     * Drive the source-locked candidate selection, then the C162
     * cancel branch via M11_GameView_CancelMirrorCandidate
     * (REVIVE.C F0282:744-783).  The slice is the 2-step terminal
     * variant: there is NO follow-up re-select.  The framebuffer's
     * D1C portrait rect must be re-painted at full match after the
     * cancel arm tears the panel down, and the C127 sensorData must
     * be unchanged (the route remains hot for any future select,
     * which is covered by a future cancel_reopen slice, not here). */
    printf("\n[Group C] candidate_panel_cancel: select, cancel (terminal, no follow-up re-select)\n");

    /* Step 1: SelectFrontMirrorCandidate (F0280).
     * After this step the panel is live, the candidate champion is
     * in slot 0, and the BUG-120/121 panel guard suppresses the
     * D1C portrait sprite.  The select-arm check locks the
     * pre-cancel state machine entry condition. */
    park_d1c_front_route(&state);
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
                 "championCount=%d, inventoryPanelActive=%d, "
                 "activeChampionIndex=%d, champions[0].present=%d "
                 "(panel-on entry state for cancel arm)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex,
                 countAfterSelect,
                 state.inventoryPanelActive,
                 state.world.party.activeChampionIndex,
                 state.world.party.champions[0].present);
        CHECK(state.candidateMirrorPanelActive == 1 &&
              state.candidateMirrorOrdinal == TARGET_ORDINAL &&
              state.candidateMirrorPartyIndex == 0 &&
              countAfterSelect == initialCount + 1 &&
              state.inventoryPanelActive == 1 &&
              state.world.party.activeChampionIndex == 0 &&
              state.world.party.champions[0].present == 1, msg);
    }

    /* Render with C040 panel live.  The portrait must NOT be drawn
     * as a stale floating sprite while the panel owns the view
     * (BUG-120/121 panel guard).  Match against ordinal 9 should
     * be low.  This is the panel-on reference frame for the
     * post-cancel residue check. */
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

    /* Capture the inspect readout BEFORE cancel, so Group C2 can
     * verify the cancel arm overwrites them with the F0282 C162
     * sentinel AND does NOT leak the candidate name. */
    {
        const char* preTitle = state.inspectTitle;
        const char* preDetail = state.inspectDetail;
        char preTitleBuf[sizeof(state.inspectTitle)];
        char preDetailBuf[sizeof(state.inspectDetail)];
        snprintf(preTitleBuf, sizeof(preTitleBuf), "%s",
                 preTitle ? preTitle : "");
        snprintf(preDetailBuf, sizeof(preDetailBuf), "%s",
                 preDetail ? preDetail : "");
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "before cancel: inspectTitle=\"%s\", "
                     "lastAction=\"%s\"",
                     preTitleBuf,
                     state.lastAction[0] ? state.lastAction : "");
            CHECK(strstr(preTitleBuf, "MIRROR") != NULL ||
                  strstr(preTitleBuf, kExpectedCatalogName) != NULL,
                  msg);
        }
        {
            char msg[240];
            snprintf(msg, sizeof(msg),
                     "before cancel: inspectDetail contains candidate "
                     "name \"%s\" (got \"%s\")",
                     kExpectedCatalogName, preDetailBuf);
            CHECK(strstr(preDetailBuf, kExpectedCatalogName) != NULL, msg);
        }
    }

    /* Step 2: CancelMirrorCandidate (F0282 C162 cancel branch).
     * This is the source-locked terminal arm.  After this call the
     * candidate-panel state machine MUST be fully idle: panel
     * closed, inventory closed, candidate cleared, slot zero-initialised,
     * leader cleared (when the canceled slot was the only champion),
     * inspect readout reset to the F0282 C162 sentinel, and the
     * C127 sensorData UNCHANGED.  The slice is terminal here - we
     * do NOT call a third select. */
    cancelRc = M11_GameView_CancelMirrorCandidate(&state);
    countAfterCancel = state.world.party.championCount;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "CancelMirrorCandidate on (1,2,0) returns 1 (got %d)",
                 cancelRc);
        CHECK(cancelRc == 1, msg);
    }

    /* C2a: candidate-panel state machine cleared. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: candidateMirrorPanelActive=%d, "
                 "candidateMirrorOrdinal=%d, candidateMirrorPartyIndex=%d "
                 "(all must be idle sentinel)",
                 state.candidateMirrorPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1, msg);
    }

    /* C2b: inventory panel torn down (PANEL.C F0355 inventory close,
     * m11_game_view.c:8122 sets inventoryPanelActive=0 in the cancel
     * arm).  This is a unique invariant of the candidate_panel_cancel
     * slice; cancel_reopen's third select re-opens inventory, so it
     * cannot verify the cancel arm's inventory-close. */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: inventoryPanelActive=%d (must be 0; "
                 "PANEL.C F0355 inventory close on C162 cancel)",
                 state.inventoryPanelActive);
        CHECK(state.inventoryPanelActive == 0, msg);
    }

    /* C2c: party state restored.  The canceled champion is removed
     * (count decremented), the canceled slot is zero-initialised
     * (.present=0, packed name bytes sum to 0), and the active
     * champion index is -1 because the canceled slot was the only
     * champion. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: championCount=%d (was %d before select), "
                 "activeChampionIndex=%d, champions[0].present=%d "
                 "(F0643_PARTY_ClearChampionSlot_Compat + F0600 zero-init)",
                 countAfterCancel, initialCount,
                 state.world.party.activeChampionIndex,
                 state.world.party.champions[0].present);
        CHECK(countAfterCancel == initialCount &&
              state.world.party.activeChampionIndex == -1 &&
              state.world.party.champions[0].present == 0, msg);
    }

    /* C2c-extra: the canceled slot's packed-name bytes must be zero
     * (F0600_CHAMPION_InitEmpty_Compat zeroes the entire
     * ChampionState_Compat struct).  A regression that left residual
     * mirror text in the slot would mean the cancel arm did not
     * route through F0643 + F0600. */
    {
        int slotNameSumAfter = slot_packed_name_byte_sum(&state, 0);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: champions[0].name packed-byte sum = %d "
                 "(must be 0 after F0600 zero-init; pre-cancel sum was %d)",
                 slotNameSumAfter, slotNameSumBefore);
        CHECK(slotNameSumAfter == 0, msg);
    }

    /* C2d: status text carries the F0282 C162 cancel sentinel AND
     * does NOT leak the candidate name.  The select arm sets
     * lastAction="MIRROR", lastOutcome="RESURRECT OR REINCARNATE",
     * inspectTitle="MIRROR: ZED", inspectDetail with the candidate
     * name embedded.  The cancel arm must overwrite all of these
     * with the cancel sentinel (m11_game_view.c:8127-8129) and must
     * NOT carry "ZED" anywhere in the inspect readout. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: lastAction=\"%s\" (expected \"%s\")",
                 state.lastAction[0] ? state.lastAction : "",
                 kCancelAction);
        CHECK(strcmp(state.lastAction, kCancelAction) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: lastOutcome=\"%s\" (expected \"%s\")",
                 state.lastOutcome[0] ? state.lastOutcome : "",
                 kCancelOutcome);
        CHECK(strcmp(state.lastOutcome, kCancelOutcome) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: inspectTitle=\"%s\" (expected \"%s\")",
                 state.inspectTitle[0] ? state.inspectTitle : "",
                 kCancelInspectTitle);
        CHECK(strcmp(state.inspectTitle, kCancelInspectTitle) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: inspectDetail=\"%s\" (expected \"%s\")",
                 state.inspectDetail[0] ? state.inspectDetail : "",
                 kCancelInspectDetail);
        CHECK(strcmp(state.inspectDetail, kCancelInspectDetail) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: inspectDetail does NOT contain candidate "
                 "name \"%s\" (cancel arm must overwrite the select-arm "
                 "inspect detail)",
                 kExpectedCatalogName);
        CHECK(strstr(state.inspectDetail, kExpectedCatalogName) == NULL, msg);
    }

    /* C2e: C127 sensorData is unchanged.  m11_disable_front_mirror_route
     * is ONLY called from M11_GameView_ConfirmMirrorCandidate
     * (m11_game_view.c:8094); the cancel arm does NOT disable the
     * route.  This invariant is what makes the route variant a true
     * "cancel" rather than a "cancel + disable": the sensor still
     * reports ordinal 9 on the same front square, so any future
     * select call would succeed (covered by cancel_reopen, not here). */
    sensorDataAfterCancel =
        (int)state.world.things->sensors[seededSensorIdx].sensorData;
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: C127 sensor[%d].sensorData = %d (must "
                 "stay %d - cancel does not mutate route)",
                 seededSensorIdx, sensorDataAfterCancel, TARGET_ORDINAL);
        CHECK(sensorDataAfterCancel == TARGET_ORDINAL, msg);
    }
    /* Cross-check via the public front-mirror helper: the route must
     * still resolve to ordinal 9 after cancel. */
    {
        int frontOrdinalAfterCancel = M11_GameView_GetFrontMirrorOrdinal(&state);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: GetFrontMirrorOrdinal = %d (must stay %d; "
                 "route still hot for future select)",
                 frontOrdinalAfterCancel, TARGET_ORDINAL);
        CHECK(frontOrdinalAfterCancel == TARGET_ORDINAL, msg);
    }

    /* Step 3: Render after cancel.  The panel is gone, so the D1C
     * portrait rect must be painted at full match again (BUG-120
     * panel guard releases on candidateMirrorPanelActive=0).  This
     * is the post-cancel re-paint proof. */
    memset(fbAfterCancel, 0, sizeof(fbAfterCancel));
    M11_GameView_Draw(&state, fbAfterCancel, FB_W, FB_H);
    matchAfterCancel = match_portrait_at_rect(portraits,
                                               fbAfterCancel,
                                               TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect carries ordinal %d "
                 "pixels at >= 90%% match (got %d%%) - panel guard "
                 "released, portrait re-painted",
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
                 "palette indices (got %d)",
                 distinctAfterCancel);
        CHECK(distinctAfterCancel >= 4, msg);
    }
    warmAfterCancel = rect_warm_count(fbAfterCancel,
                                      D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                      D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: D1C portrait rect has >= %d warm-color "
                 "pixels (got %d) - portrait sprite restored",
                 PORTRAIT_WARM_THRESHOLD, warmAfterCancel);
        CHECK(warmAfterCancel >= PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-floating proof (panel-off after cancel): side walls must NOT
     * carry portrait warm pixels.  cancel_reopen does not sample
     * this; this is a unique invariant of candidate_panel_cancel. */
    leftSideAfterCancel = rect_warm_count(fbAfterCancel,
                                          SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                          SIDE_WALL_LEFT_W,
                                          PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: left side wall of D1C portrait band has "
                 "< %d warm pixels (got %d) - portrait not floating on "
                 "left wall",
                 PORTRAIT_WARM_THRESHOLD, leftSideAfterCancel);
        CHECK(leftSideAfterCancel < PORTRAIT_WARM_THRESHOLD, msg);
    }
    rightSideAfterCancel = rect_warm_count(fbAfterCancel,
                                           SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                           SIDE_WALL_RIGHT_W,
                                           PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "after cancel: right side wall of D1C portrait band has "
                 "< %d warm pixels (got %d) - portrait not floating on "
                 "right wall",
                 PORTRAIT_WARM_THRESHOLD, rightSideAfterCancel);
        CHECK(rightSideAfterCancel < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* No-panel-chrome-residue proof: the C040 panel frame rectangle
     * must look identical to the panel-off baseline.  After the
     * cancel arm clears inventoryPanelActive, the panel chrome is
     * gone and the underlying wall-ornament frame is visible.  We
     * assert the post-cancel frame-region distinct-palette-count is
     * within +/- 1 of the panel-off baseline distinct-palette-count
     * (wall ornament + portrait sprite palette differs by at most 1
     * from wall ornament + wall behind portrait).  This proves the
     * cancel arm tore the panel down without leaving chrome residue. */
    {
        int c040FrameNonzeroAfterCancel = rect_nonzero(fbAfterCancel,
                                                       C040_FRAME_X, C040_FRAME_Y,
                                                       C040_FRAME_W, C040_FRAME_H);
        int c040InnerDistinctAfterCancel = rect_distinct(fbAfterCancel,
                                                         C040_INNER_X, C040_INNER_Y,
                                                         C040_INNER_W, C040_INNER_H);
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after cancel: C040 frame region has %d non-zero pixels "
                 "(baseline was %d; post-cancel must stay in wall-ornament "
                 "density band [200, 4000])",
                 c040FrameNonzeroAfterCancel, c040FrameNonzeroBefore);
        CHECK(c040FrameNonzeroAfterCancel >= 200 &&
              c040FrameNonzeroAfterCancel <= 4000, msg);
        {
            char msg2[240];
            snprintf(msg2, sizeof(msg2),
                     "after cancel: C040 inner distinct palette count = %d "
                     "(baseline was %d; cancel must restore the panel-off "
                     "look - delta must be <= 1)",
                     c040InnerDistinctAfterCancel, c040InnerDistinctBefore);
            int delta = c040InnerDistinctAfterCancel - c040InnerDistinctBefore;
            if (delta < 0) delta = -delta;
            CHECK(delta <= 1, msg2);
        }
    }

    /* The portrait_rect_position contract: across the select -> cancel
     * cycle the D1C destination rectangle does NOT change screen
     * position.  The (96, 35, 32, 29) destination is source-locked to
     * DUNVIEW.C:3913-3928 + DUNVIEW.C:525 G0109_Graphic558_Box_
     * ChampionPortraitOnWall, so we verify the same rect lines up with
     * ordinal-9 pixels when the panel is closed (before select, after
     * cancel) and is suppressed as a stale sprite while the panel is
     * live (after select).  This is the 2-step candidate_panel_cancel
     * contract: panel-off == panel-off, panel-on != panel-off. */
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position (candidate_panel_cancel): "
                 "before=%d%%, after-select=%d%%, after-cancel=%d%% "
                 "(panel-off >=90, panel-on <=20)",
                 matchBefore, matchAfterSelect, matchAfterCancel);
        CHECK(matchBefore >= 90 &&
              matchAfterSelect <= 20 &&
              matchAfterCancel >= 90, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - terminal-state proof: the slice ends here, no third select
     * ----------------------------------------------------------------
     * The candidate_panel_cancel route variant is the 2-step terminal
     * slice.  We prove the post-cancel state is fully terminal by
     * locking that the route is NOT in some "waiting" sub-state: the
     * front-mirror helper still resolves to ordinal 9, but no
     * candidate-panel state remains active.  This is the
     * difference between candidate_panel_cancel and the cancel_reopen
     * route: this slice does NOT depend on any follow-up select. */
    printf("\n[Group D] terminal state: post-cancel state machine is fully idle\n");
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "terminal: candidateMirrorPanelActive=%d, "
                 "inventoryPanelActive=%d, candidateMirrorOrdinal=%d, "
                 "candidateMirrorPartyIndex=%d (all must be idle)",
                 state.candidateMirrorPanelActive,
                 state.inventoryPanelActive,
                 state.candidateMirrorOrdinal,
                 state.candidateMirrorPartyIndex);
        CHECK(state.candidateMirrorPanelActive == 0 &&
              state.inventoryPanelActive == 0 &&
              state.candidateMirrorOrdinal == -1 &&
              state.candidateMirrorPartyIndex == -1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "terminal: championCount=%d (must equal initial %d), "
                 "activeChampionIndex=%d (must be -1)",
                 state.world.party.championCount, initialCount,
                 state.world.party.activeChampionIndex);
        CHECK(state.world.party.championCount == initialCount &&
              state.world.party.activeChampionIndex == -1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "terminal: champions[0].present=%d, name sum=%d "
                 "(slot must be empty with zeroed packed name)",
                 state.world.party.champions[0].present,
                 slot_packed_name_byte_sum(&state, 0));
        CHECK(state.world.party.champions[0].present == 0 &&
              slot_packed_name_byte_sum(&state, 0) == 0, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "terminal: route still hot for future select "
                 "(GetFrontMirrorOrdinal = %d, must stay %d) - "
                 "candidate_panel_cancel is a true cancel, not a "
                 "cancel+disable",
                 M11_GameView_GetFrontMirrorOrdinal(&state),
                 TARGET_ORDINAL);
        CHECK(M11_GameView_GetFrontMirrorOrdinal(&state) == TARGET_ORDINAL,
              msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
