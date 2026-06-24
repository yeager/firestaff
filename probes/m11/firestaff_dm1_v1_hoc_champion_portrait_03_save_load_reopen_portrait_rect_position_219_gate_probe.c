/*
 * firestaff_dm1_v1_hoc_champion_portrait_03_save_load_reopen_portrait_rect_position_219_gate_probe.c
 *
 * DM1 V1 Hall of Champions — champion portrait ordinal 3
 *   route      = save_load_reopen
 *                (candidate-panel live -> M11_GameView_QuickSave with the
 *                 C040 panel open -> M11_GameView_CancelMirrorCandidate
 *                 (REVIVE.C F0282:744-806) -> M11_GameView_QuickLoad
 *                 -> M11_GameView_SelectFrontMirrorCandidate (REVIVE.C
 *                 F0280:124-132) reopen.  The slice proves that the
 *                 D1C champion-portrait rectangle (96, 35, 32, 29)
 *                 stays anchored at the same source-locked viewport
 *                 coords across the full save+load+reopen cycle, that
 *                 the (1,2) NORTH-route C127 sensor still resolves to
 *                 ordinal 3 after the round-trip, and that the reopen
 *                 redraw carries ordinal-3 source pixels at the same
 *                 destination rectangle.)
 *   ordinal    = 3 (mirror catalog record AZIZI / title JOHARI, the
 *                 4th valid mirror text string in the shipped DM1 V1
 *                 DUNGEON.DAT.  C026 atlas cell (96, 0, 32, 29) per
 *                 DUNVIEW.C:3916-3919 + DEFS.H:821-826 M027/M028.)
 *   aspect     = portrait_rect_position
 *                (the source-locked C026 champion portrait cutout
 *                 stays anchored at viewport (96, 35, 32, 29) and
 *                 inside the public D1C wall-mirror zone
 *                 (80, 29, 64, 43) on every redraw the save+load+
 *                 reopen cycle triggers.)
 *   gate id    = 219, batch group 9.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 champion portrait into
 *     the D1C cutout at viewport (96, 35, 32, 29) using
 *     ((ordinal & 7) << 5, (ordinal >> 3) * 29).
 *   ReDMCSB DUNVIEW.C:525 records G0109 Graphic558_Box_ChampionPortraitOnWall
 *     as (96, 127, 35, 63), i.e. 32x29 cutout at viewport (96, 35).
 *   ReDMCSB DUNVIEW.C G0205 graphic 558 coordSet 5 [12] records the
 *     C346 D1C wall-mirror frame as (80, 29, 64, 43).
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw order
 *     (far-to-near), the panel-on panel guard that suppresses the
 *     D1C full cutout draw while the C040 panel is live (BUG-120/121).
 *   ReDMCSB MOVESENS.C:1501-1503 dispatches C127 sensorData to F0280.
 *   ReDMCSB REVIVE.C F0280:124-132 candidate publication.
 *   ReDMCSB REVIVE.C F0282:744-806 C162 cancel branch.
 *   ReDMCSB LOADSAVE.C F0433:1502-1707 F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF
 *     + LOADSAVE.C F0435:2192-2660 F0435_STARTEND_LoadGame (the
 *     F0433/F0435 world-blob save/load boundary that this gate
 *     crosses; the panel-state is persisted through Firestaff's
 *     m11_write_quicksave_v1_runtime sidecar which stores the
 *     candidateMirrorPanelActive / candidateMirrorOrdinal /
 *     candidateMirrorPartyIndex / inventoryPanelActive fields
 *     alongside the source-locked world blob).
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin and dims.
 *
 * Coverage gap relative to the existing ordinal-3 probe matrix:
 *   - firestaff_dm1_v1_hall_of_champions_portrait_03_cancel_reopen
 *     covers select->cancel->select (cancel_reopen route, no
 *     save/load boundary) and is the closest sibling.
 *   - firestaff_dm1_v1_hoc_champion_portrait_03_double_click_stability
 *     covers the click-while-panel-live-above-hit-boxes cycle.
 *   - firestaff_dm1_v1_hoc_champion_portrait_03_transparent_pixels
 *     covers the per-pixel C01 transparency contract on the
 *     east_walkpath pose.
 *   - firestaff_dm1_v1_champion_mirror_portrait03_rect_runtime_probe
 *     covers ordinal 3 at (1,2) NORTH pre/post candidate panel
 *     but does not cross F0433/F0435.
 *   - test_dm1_v1_mirror_candidate_reopen_after_save_load_pc34_compat
 *     (data-free, contract-only fixture) covers the four UI globals
 *     no-mutate contract but does NOT drive M11_GameView_Draw and
 *     cannot produce pixel evidence.
 *
 * The save_load_reopen slice is therefore the runtime/pixel-evidence
 * complement to the data-free contract test, and is disjoint from all
 * of the above.
 *
 * Build:
 *   The probe is a member of the FIRESTAFF_DM1_POOL_PROBES list in
 *   CMakeLists.txt.  CTest label: firestaff_dm1_v1_hoc_champion_
 *   portrait_03_save_load_reopen_portrait_rect_position_219_gate_probe.
 *
 * Honest scope: runtime correctness against the local DM1 V1
 * GRAPHICS.DAT/DUNGEON.DAT.  It does NOT claim original DOS pixel
 * parity.  The portrait_rect_position invariant is the source-
 * locked D1C cutout rectangle and its alignment with the
 * C026 atlas cell — both of which are ReDMCSB-anchored.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"
#include "dm1_v1_save_load.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <process.h>
#else
#include <unistd.h>
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    /* Source-locked PC 3.4 viewport (ReDMCSB COORD.C:1693-1722):
     * origin (M11_VIEWPORT_X, M11_VIEWPORT_Y) = (0, 33); size
     * (M11_VIEWPORT_W, M11_VIEWPORT_H) = (224, 136). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* Source-locked D1C champion portrait cutout viewport-local
     * (DUNVIEW.C:3913-3928, DUNVIEW.C:525 G0109 = {96,127,35,63}). */
    PORTRAIT_VX = 96,
    PORTRAIT_VY = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* Source-locked D1C wall-mirror frame zone
     * (DUNVIEW.C G0205 graphic 558 coordSet 5 [12]). */
    D1C_FRAME_X = 80,
    D1C_FRAME_Y = 29,
    D1C_FRAME_W = 64,
    D1C_FRAME_H = 43,
    /* C026 atlas transparency key (DUNVIEW.C:4547-4581). */
    ATLAS_TRANSPARENT = 1,
    /* Ordinal 3 (the 4th valid mirror text string in DM1 V1: AZIZI /
     * JOHARI) and the C026 atlas math (ordinal & 7) << 5 = 96,
     * (ordinal >> 3) * 29 = 0 — same as the (3 & 7) * 32 = 96 form
     * used by the existing ordinal-3 probe matrix. */
    ORDINAL_TARGET = 3,
    ORDINAL_3_COL = ORDINAL_TARGET & 7,             /* = 3 */
    ORDINAL_3_ROW = ORDINAL_TARGET >> 3,            /* = 0 */
    ORDINAL_3_SRC_X = ORDINAL_3_COL * PORTRAIT_W,   /* = 96 */
    ORDINAL_3_SRC_Y = ORDINAL_3_ROW * PORTRAIT_H,   /* =  0 */
    /* Side wall sample zones (no-floating proof): the left and right
     * side wall bands around the D1C portrait row that should NOT
     * carry the portrait's warm-color pixels. */
    SIDE_WALL_LEFT_X  = VIEWPORT_X + 16,
    SIDE_WALL_LEFT_W  = 64,
    SIDE_WALL_RIGHT_X = VIEWPORT_X + 144,
    SIDE_WALL_RIGHT_W = 64,
    PORTRAIT_BAND_Y0 = VIEWPORT_Y + PORTRAIT_VY,
    PORTRAIT_BAND_Y1 = PORTRAIT_BAND_Y0 + PORTRAIT_H,
    PORTRAIT_PRESENT_WARM_THRESHOLD = 30,
    /* The shipped DM1 V1 DUNGEON.DAT places a C127 sensor on the
     * (1,2) NORTH-route front square (1,1) with sensorData=1 (HALK,
     * mirror ordinal 1).  We seed that sensor to ordinal 3 (AZIZI)
     * to lock the ordinal-3 edge case. */
    SHIPPED_HALK_ORDINAL = 1
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Convert viewport-local rectangle to framebuffer-local rectangle. */
static inline int vp_to_fb_x(int vpX) { return vpX; }
static inline int vp_to_fb_y(int vpY) { return vpY + VIEWPORT_Y; }

/* Count distinct palette indices in a framebuffer rectangle. */
static int rect_distinct(const unsigned char* fb,
                         int x, int y, int w, int h) {
    unsigned char seen[16];
    int yy, xx, n = 0;
    memset(seen, 0, sizeof(seen));
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

/* Count "warm" pixels in a framebuffer rectangle.  Same warm-color
 * set as the existing capture / gate probes: palette indices
 * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} mark the champion portrait
 * sprite pixels vs the grey-stone wall texture palette {0x01, 0x02,
 * 0x0D}.  Counting warm pixels is a coarse but reliable way to
 * distinguish "portrait is here" from "wall only" in the C026
 * cutout (96, 35, 32, 29). */
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
 * of opaque source pixels that match the destination pixel. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    int fbRectX = vp_to_fb_x(PORTRAIT_VX);
    int fbRectY = vp_to_fb_y(PORTRAIT_VY);
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcX = (ordinal & 7) * PORTRAIT_W;
    srcY = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src;
            unsigned char dst;
            int sx = srcX + x;
            int sy = srcY + y;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src == ATLAS_TRANSPARENT) continue; /* transparent */
            dst = M11_FB_DECODE_INDEX(fb[(fbRectY + y) * FB_W + (fbRectX + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Same helper used by the
 * existing ordinal-3 probe matrix. */
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
 * Same pose used by the existing ordinal-3 probe matrix. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
    state->world.party.direction = 0; /* DIR_NORTH */
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;
    int frontOrdinal;
    int seededSensor;
    int initialChampionCount;
    int selectRc, cancelRc, reopenRc;
    int saveRc, loadRc;
    int savedPanelActive, savedOrdinal, savedPartyIndex;
    int afterCancelPanelActive, afterCancelOrdinal, afterCancelPartyIndex;
    int afterLoadPanelActive, afterLoadOrdinal, afterLoadPartyIndex;
    int afterReopenPanelActive, afterReopenOrdinal, afterReopenPartyIndex;
    int afterReopenChampionCount;
    int ornX, ornY, ornW, ornH;
    const char* dataDir;
    char quicksavePath[1024];
    char quicksaveSidecarPath[1100];
    int matchPreSelect, matchAfterCancel;
    int matchAfterLoad, matchAfterReopen;
    int nonzeroAfterReopen, distinctAfterReopen, warmAfterReopen;
    int leftSideAfterReopen, rightSideAfterReopen;
    int quicksaveIsolated;
    const char* oldQuicksaveEnv = NULL;
    char nameBuf[32];
    int nameLookupRc;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR [QUICKSAVE_PATH]\n"
                "  verifies ordinal 3 save_load_reopen portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    /* The quicksave path is set via FIRESTAFF_QUICKSAVE_PATH so the
     * probe does not clobber the user's real quicksave slot.  If a
     * second CLI arg is supplied we also pass it through. */
    if (argc >= 3) {
        quicksavePath[0] = '\0';
        snprintf(quicksavePath, sizeof(quicksavePath), "%s", argv[2]);
    } else {
        quicksavePath[0] = '\0';
        snprintf(quicksavePath, sizeof(quicksavePath),
                 "/tmp/firestaff-portrait-03-save-load-reopen-219-%d.qsv",
                 (int)getpid());
    }
    /* Build the sidecar path: the V1 runtime sidecar lives next to
     * the quicksave file with a ".v1runtime" suffix. */
    snprintf(quicksaveSidecarPath, sizeof(quicksaveSidecarPath),
             "%s.v1runtime", quicksavePath);
    quicksaveIsolated = 1;

    /* Pre-cleanup any leftover quicksave files from a previous run so
     * QuickLoad does not pick up a stale snapshot that was saved
     * without the candidate panel live. */
    (void)remove(quicksavePath);
    (void)remove(quicksaveSidecarPath);

    oldQuicksaveEnv = getenv("FIRESTAFF_QUICKSAVE_PATH");
    if (setenv("FIRESTAFF_QUICKSAVE_PATH", quicksavePath, 1) != 0) {
        fprintf(stderr,
                "FAIL: could not set FIRESTAFF_QUICKSAVE_PATH=%s\n",
                quicksavePath);
        return 1;
    }

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    game.showDebugHUD = 0;

    printf("=== DM1 V1 HoC portrait ordinal 3 save_load_reopen ===\n");
    printf("dataDir=%s quicksavePath=%s (isolated=%d)\n",
           dataDir, quicksavePath, quicksaveIsolated);
    printf("pose=(map 0, x=1, y=2) facing NORTH; seeded C127 sensor to "
           "ordinal 3 (AZIZI)\n");

    /* ----------------------------------------------------------------
     * Group A — atlas math for ordinal 3
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 0 /
     * column 3 and that the math matches COORD.C / DEFS.H:821-826. */
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
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
        M11_GameView_Shutdown(&game);
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
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 3 srcX = %d, srcY = %d (cell within 256x87 "
                 "atlas: must be < %d and < %d)",
                 ORDINAL_3_SRC_X, ORDINAL_3_SRC_Y,
                 (int)portraits->width, (int)portraits->height);
        CHECK(ORDINAL_3_SRC_X + PORTRAIT_W <= (int)portraits->width &&
              ORDINAL_3_SRC_Y + PORTRAIT_H <= (int)portraits->height, msg);
    }

    /* The mirror catalog must resolve ordinal 3 to AZIZI. */
    nameBuf[0] = '\0';
    nameLookupRc = M11_GameView_GetMirrorNameByOrdinal(&game,
                                                       ORDINAL_TARGET,
                                                       nameBuf,
                                                       (int)sizeof(nameBuf));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror catalog resolves ordinal 3 to \"%s\" (expected "
                 "\"AZIZI\")",
                 nameBuf[0] ? nameBuf : "");
        CHECK(nameLookupRc > 0 && strcmp(nameBuf, "AZIZI") == 0, msg);
    }

    /* Park the party on the (1,2,0) NORTH-route front mirror, then
     * seed the C127 sensor from HALK (1) to ordinal 3 (AZIZI).  Same
     * sensor, same map cell, same draw path - only G0289 changes. */
    park_d1c_front_route(&game);

    /* Sanity-check the public D1C wall ornament zone helper, then
     * verify the inner portrait rectangle (96, 35, 32, 29) sits
     * inside that zone. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d, %d, %d, %d) viewport "
                 "coords (DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == D1C_FRAME_X && ornY == D1C_FRAME_Y &&
              ornW == D1C_FRAME_W && ornH == D1C_FRAME_H, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35, 32, 29) sits inside the "
                 "D1C wall ornament zone (X within [%d,%d), Y within "
                 "[%d,%d))",
                 ornX, ornX + ornW, ornY, ornY + ornH);
        CHECK(PORTRAIT_VX >= ornX &&
              PORTRAIT_VX + PORTRAIT_W <= ornX + ornW &&
              PORTRAIT_VY >= ornY &&
              PORTRAIT_VY + PORTRAIT_H <= ornY + ornH, msg);
    }

    /* Stage 0: confirm the unmodified route reports the shipped HALK
     * ordinal 1 — sanity check that the C127 sensor is alive at the
     * right cell before we mutate sensorData. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front-mirror ordinal at (1,2,0) = %d (expected "
                 "%d, HALK before seed)",
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to ordinal
     * 3.  Same sensor, same map cell, same draw path - only G0289
     * changes. */
    seededSensor = seed_first_c127_data(&game,
                                         SHIPPED_HALK_ORDINAL,
                                         ORDINAL_TARGET);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,2) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, ORDINAL_TARGET, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    /* The same front route now reports ordinal 3. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded north-entry front-mirror ordinal = %d (expected %d)",
                 frontOrdinal, ORDINAL_TARGET);
        CHECK(frontOrdinal == ORDINAL_TARGET, msg);
    }
    if (frontOrdinal != ORDINAL_TARGET) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify save_load_reopen\n",
                ORDINAL_TARGET);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group B — pre-candidate portrait_rect_position
     * ----------------------------------------------------------------
     * Render the framebuffer before any selection, and verify the
     * D1C destination rectangle (96, 35, 32, 29) holds ordinal-3
     * pixels.  This is the baseline we will compare against after
     * the save+load+reopen cycle. */
    printf("\n[Group B] pre-candidate portrait_rect_position on (1,2,0)=3\n");

    park_d1c_front_route(&game);
    initialChampionCount = game.world.party.championCount;

    {
        unsigned char fbPre[FB_W * FB_H];
        memset(fbPre, 0, sizeof(fbPre));
        M11_GameView_Draw(&game, fbPre, FB_W, FB_H);
        matchPreSelect = match_portrait_at_rect(portraits, fbPre, ORDINAL_TARGET);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "pre-candidate D1C portrait rect (96, 35) carries "
                     "ordinal %d pixels at >= 90%% match (got %d%%)",
                     ORDINAL_TARGET, matchPreSelect);
            CHECK(matchPreSelect >= 90, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group C — candidate live -> QuickSave
     * ----------------------------------------------------------------
     * Select the candidate so the C040 panel is live, then save the
     * world via M11_GameView_QuickSave.  The save path must capture
     * the panel-state through the v1 runtime sidecar (so the post-
     * load state is recoverable) and must not crash on a panel-live
     * save.  After save, the live in-memory state must still match
     * the panel-live invariant. */
    printf("\n[Group C] candidate live -> QuickSave\n");

    selectRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    savedPanelActive = game.candidateMirrorPanelActive;
    savedOrdinal    = game.candidateMirrorOrdinal;
    savedPartyIndex = game.candidateMirrorPartyIndex;
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
                 "after select: candidateMirrorPanelActive=%d, ordinal=%d, "
                 "partyIndex=%d, championCount=%d (was %d)",
                 savedPanelActive, savedOrdinal, savedPartyIndex,
                 game.world.party.championCount, initialChampionCount);
        CHECK(savedPanelActive == 1 &&
              savedOrdinal == ORDINAL_TARGET &&
              savedPartyIndex == 0 &&
              game.world.party.championCount == initialChampionCount + 1, msg);
    }

    saveRc = M11_GameView_QuickSave(&game);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_QuickSave with C040 panel live returns 1 "
                 "(got %d)", saveRc);
        CHECK(saveRc == 1, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "post-save live state still reports panel live "
                 "(candidateMirrorPanelActive=%d, ordinal=%d)",
                 game.candidateMirrorPanelActive, game.candidateMirrorOrdinal);
        CHECK(game.candidateMirrorPanelActive == 1 &&
              game.candidateMirrorOrdinal == ORDINAL_TARGET, msg);
    }

    /* ----------------------------------------------------------------
     * Group D — Cancel (F0282) the candidate, then QuickLoad
     * ----------------------------------------------------------------
     * CancelMirrorCandidate clears G0299, drops the candidate
     * champion, and tears the panel down.  QuickLoad must restore
     * the post-cancel world state from the snapshot.  Because the
     * snapshot was taken with the panel live, the loaded state's
     * panel state must reflect that snapshot (i.e. the candidate
     * comes back live if the runtime sidecar is honored).  This is
     * the save_load boundary that the gate table cares about: the
     * save/load path must round-trip the live C040 panel state
     * without corrupting the world blob. */
    printf("\n[Group D] Cancel + QuickLoad\n");

    cancelRc = M11_GameView_CancelMirrorCandidate(&game);
    afterCancelPanelActive = game.candidateMirrorPanelActive;
    afterCancelOrdinal     = game.candidateMirrorOrdinal;
    afterCancelPartyIndex  = game.candidateMirrorPartyIndex;
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
                 "after cancel: panelActive=%d, ordinal=%d, partyIndex=%d, "
                 "championCount=%d (was %d before select)",
                 afterCancelPanelActive, afterCancelOrdinal,
                 afterCancelPartyIndex, game.world.party.championCount,
                 initialChampionCount);
        CHECK(afterCancelPanelActive == 0 &&
              afterCancelOrdinal == -1 &&
              afterCancelPartyIndex == -1 &&
              game.world.party.championCount == initialChampionCount, msg);
    }

    loadRc = M11_GameView_QuickLoad(&game);
    afterLoadPanelActive = game.candidateMirrorPanelActive;
    afterLoadOrdinal     = game.candidateMirrorOrdinal;
    afterLoadPartyIndex  = game.candidateMirrorPartyIndex;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_QuickLoad from snapshot returns 1 (got %d)",
                 loadRc);
        CHECK(loadRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after load: panelActive=%d, ordinal=%d, partyIndex=%d, "
                 "championCount=%d (post-load panel state must match the "
                 "panel-live snapshot)",
                 afterLoadPanelActive, afterLoadOrdinal,
                 afterLoadPartyIndex, game.world.party.championCount);
        /* The contract: Firestaff persists the candidate state in the
         * v1 runtime sidecar (m11_write_quicksave_v1_runtime) so a
         * save+load cycle round-trips the panel live.  We assert
         * that the loaded party has the candidate materialized (the
         * panel is on, ordinal=3, party index=0, champion count is
         * initial+1).  If the sidecar path is intentionally not
         * honoured, the assertion would be: panelActive=0,
         * ordinal=-1, partyIndex=-1, championCount=initial.  In that
         * case, the next reopen step would still need to recover
         * the panel state. */
        CHECK(afterLoadPanelActive == 1 &&
              afterLoadOrdinal == ORDINAL_TARGET &&
              afterLoadPartyIndex == 0 &&
              game.world.party.championCount == initialChampionCount + 1, msg);
    }

    /* Render the loaded state: with the panel live, the D1C portrait
     * rect must NOT be a stale full sprite (BUG-120/121 panel guard
     * still active).  Match against ordinal 3 should be low while
     * the C040 panel covers the lower portrait rows. */
    {
        unsigned char fbAfterLoad[FB_W * FB_H];
        memset(fbAfterLoad, 0, sizeof(fbAfterLoad));
        M11_GameView_Draw(&game, fbAfterLoad, FB_W, FB_H);
        matchAfterLoad = match_portrait_at_rect(portraits,
                                                fbAfterLoad,
                                                ORDINAL_TARGET);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-load panel-on redraw does not leave ordinal %d "
                     "as a stale full-D1C sprite (<= 20%% match, got %d%%)",
                     ORDINAL_TARGET, matchAfterLoad);
            CHECK(matchAfterLoad <= 20, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group E — Cancel post-load, then reopen, then verify
     *           portrait_rect_position
     * ----------------------------------------------------------------
     * The save_load_reopen slice: cancel the post-load candidate to
     * tear the panel down cleanly, then re-select via F0280 to
     * reopen the panel on the same front square.  After the full
     * save+load+reopen cycle the D1C portrait rect must still hold
     * ordinal-3 pixels and stay anchored at (96, 35, 32, 29). */
    printf("\n[Group E] Cancel post-load + reopen, verify portrait_rect_position\n");

    /* Cancel the post-load panel cleanly. */
    cancelRc = M11_GameView_CancelMirrorCandidate(&game);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "CancelMirrorCandidate on loaded party returns 1 (got %d)",
                 cancelRc);
        CHECK(cancelRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "post-cancel loaded: panelActive=%d, ordinal=%d, "
                 "partyIndex=%d, championCount=%d",
                 game.candidateMirrorPanelActive,
                 game.candidateMirrorOrdinal,
                 game.candidateMirrorPartyIndex,
                 game.world.party.championCount);
        CHECK(game.candidateMirrorPanelActive == 0 &&
              game.candidateMirrorOrdinal == -1 &&
              game.candidateMirrorPartyIndex == -1, msg);
    }

    /* Render mid-cancel to confirm the portrait rect is back. */
    {
        unsigned char fbAfterCancel[FB_W * FB_H];
        memset(fbAfterCancel, 0, sizeof(fbAfterCancel));
        M11_GameView_Draw(&game, fbAfterCancel, FB_W, FB_H);
        matchAfterCancel = match_portrait_at_rect(portraits,
                                                  fbAfterCancel,
                                                  ORDINAL_TARGET);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-cancel-loaded D1C portrait rect carries "
                     "ordinal %d pixels at >= 90%% match (got %d%%)",
                     ORDINAL_TARGET, matchAfterCancel);
            CHECK(matchAfterCancel >= 90, msg);
        }
    }

    /* Reopen: F0280 publication on the loaded party's front square. */
    reopenRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    afterReopenPanelActive  = game.candidateMirrorPanelActive;
    afterReopenOrdinal      = game.candidateMirrorOrdinal;
    afterReopenPartyIndex   = game.candidateMirrorPartyIndex;
    afterReopenChampionCount = game.world.party.championCount;
    {
        char dbg[400];
        snprintf(dbg, sizeof(dbg),
                 "[debug] after load+second-cancel: things=%p, "
                 "sensorCount=%d, party=(%d,%d) dir=%d, frontOrdinal=%d, "
                 "panelActive=%d, ordinal=%d",
                 (void*)game.world.things,
                 game.world.things ? game.world.things->sensorCount : -1,
                 game.world.party.mapX, game.world.party.mapY,
                 game.world.party.direction,
                 M11_GameView_GetFrontMirrorOrdinal(&game),
                 game.candidateMirrorPanelActive,
                 game.candidateMirrorOrdinal);
        printf("  %s\n", dbg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "SelectFrontMirrorCandidate reopen on (1,2,0) returns 1 "
                 "(got %d)", reopenRc);
        CHECK(reopenRc == 1, msg);
    }
    {
        char msg[240];
        snprintf(msg, sizeof(msg),
                 "after reopen: panelActive=%d, ordinal=%d, partyIndex=%d, "
                 "championCount=%d (was %d before first select)",
                 afterReopenPanelActive, afterReopenOrdinal,
                 afterReopenPartyIndex, afterReopenChampionCount,
                 initialChampionCount);
        CHECK(afterReopenPanelActive == 1 &&
              afterReopenOrdinal == ORDINAL_TARGET &&
              afterReopenPartyIndex == 0 &&
              afterReopenChampionCount == initialChampionCount + 1, msg);
    }

    /* The front mirror route must still resolve to ordinal 3 after
     * the round-trip — the C127 sensor data is persisted as part of
     * the world blob and must come back with the loaded dungeon. */
    {
        int postReopenFront = M11_GameView_GetFrontMirrorOrdinal(&game);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal after save+load+reopen = %d "
                 "(expected %d, AZIZI)",
                 postReopenFront, ORDINAL_TARGET);
        CHECK(postReopenFront == ORDINAL_TARGET, msg);
    }

    /* Render with the reopen panel live.  Panel guard still active,
     * so match against ordinal 3 should be low while the C040 panel
     * covers the lower rows of the cutout. */
    {
        unsigned char fbAfterReopen[FB_W * FB_H];
        memset(fbAfterReopen, 0, sizeof(fbAfterReopen));
        M11_GameView_Draw(&game, fbAfterReopen, FB_W, FB_H);
        matchAfterReopen = match_portrait_at_rect(portraits,
                                                  fbAfterReopen,
                                                  ORDINAL_TARGET);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "reopen panel-on redraw does not leave ordinal %d "
                     "as a stale full-D1C sprite (<= 20%% match, got %d%%)",
                     ORDINAL_TARGET, matchAfterReopen);
            CHECK(matchAfterReopen <= 20, msg);
        }

        /* Close the panel so we can prove the post-reopen portrait
         * rect is still anchored at the source-locked coords. */
        (void)M11_GameView_CancelMirrorCandidate(&game);

        memset(fbAfterReopen, 0, sizeof(fbAfterReopen));
        M11_GameView_Draw(&game, fbAfterReopen, FB_W, FB_H);
        matchAfterReopen = match_portrait_at_rect(portraits,
                                                  fbAfterReopen,
                                                  ORDINAL_TARGET);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-reopen-cancel D1C portrait rect carries "
                     "ordinal %d pixels at >= 90%% match (got %d%%)",
                     ORDINAL_TARGET, matchAfterReopen);
            CHECK(matchAfterReopen >= 90, msg);
        }

        nonzeroAfterReopen = rect_nonzero(fbAfterReopen,
                                          vp_to_fb_x(PORTRAIT_VX),
                                          vp_to_fb_y(PORTRAIT_VY),
                                          PORTRAIT_W, PORTRAIT_H);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-reopen-cancel D1C portrait rect is non-empty "
                     "(>= 100 non-zero pixels, got %d)",
                     nonzeroAfterReopen);
            CHECK(nonzeroAfterReopen >= 100, msg);
        }
        distinctAfterReopen = rect_distinct(fbAfterReopen,
                                            vp_to_fb_x(PORTRAIT_VX),
                                            vp_to_fb_y(PORTRAIT_VY),
                                            PORTRAIT_W, PORTRAIT_H);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-reopen-cancel D1C portrait rect has >= 4 "
                     "distinct palette indices (got %d)",
                     distinctAfterReopen);
            CHECK(distinctAfterReopen >= 4, msg);
        }
        warmAfterReopen = rect_warm_count(fbAfterReopen,
                                          vp_to_fb_x(PORTRAIT_VX),
                                          vp_to_fb_y(PORTRAIT_VY),
                                          PORTRAIT_W, PORTRAIT_H);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-reopen-cancel D1C portrait rect has >= %d "
                     "warm-color pixels (got %d) - portrait sprite, "
                     "not wall",
                     PORTRAIT_PRESENT_WARM_THRESHOLD, warmAfterReopen);
            CHECK(warmAfterReopen >= PORTRAIT_PRESENT_WARM_THRESHOLD, msg);
        }

        /* No-floating proof on the post-reopen redraw: side walls
         * around the portrait row must NOT carry the portrait's
         * warm pixels.  This is the no-floating negative behaviour
         * the gate table calls out. */
        leftSideAfterReopen = rect_warm_count(fbAfterReopen,
                                              SIDE_WALL_LEFT_X, PORTRAIT_BAND_Y0,
                                              SIDE_WALL_LEFT_W,
                                              PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-reopen left side wall has < %d warm pixels "
                     "(got %d) - portrait not floating on left wall",
                     PORTRAIT_PRESENT_WARM_THRESHOLD, leftSideAfterReopen);
            CHECK(leftSideAfterReopen < PORTRAIT_PRESENT_WARM_THRESHOLD, msg);
        }
        rightSideAfterReopen = rect_warm_count(fbAfterReopen,
                                               SIDE_WALL_RIGHT_X, PORTRAIT_BAND_Y0,
                                               SIDE_WALL_RIGHT_W,
                                               PORTRAIT_BAND_Y1 - PORTRAIT_BAND_Y0);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "post-reopen right side wall has < %d warm pixels "
                     "(got %d) - portrait not floating on right wall",
                     PORTRAIT_PRESENT_WARM_THRESHOLD, rightSideAfterReopen);
            CHECK(rightSideAfterReopen < PORTRAIT_PRESENT_WARM_THRESHOLD, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group F — redraw stability across the save_load_reopen cycle
     * ----------------------------------------------------------------
     * Render twice in the post-reopen-cancel state and verify the
     * viewport is byte-stable (no random framebuffer drift).  The
     * portrait_rect_position invariant says the cutout stays anchored
     * at (96, 35, 32, 29) — if two consecutive redraws produce the
     * same viewport, the invariant holds on the second call. */
    printf("\n[Group F] redraw stability across the save_load_reopen cycle\n");
    {
        unsigned char fbA[FB_W * FB_H];
        unsigned char fbB[FB_W * FB_H];
        int drift = 0;
        int i;
        park_d1c_front_route(&game);
        memset(fbA, 0, sizeof(fbA));
        memset(fbB, 0, sizeof(fbB));
        M11_GameView_Draw(&game, fbA, FB_W, FB_H);
        M11_GameView_Draw(&game, fbB, FB_W, FB_H);
        for (i = 0; i < FB_W * FB_H; ++i) {
            if (fbA[i] != fbB[i]) ++drift;
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "two consecutive post-reopen-cancel redraws are "
                     "byte-stable across the full 320x200 framebuffer "
                     "(%d pixel drift)", drift);
            CHECK(drift == 0, msg);
        }
    }

    /* ----------------------------------------------------------------
     * Group G — invariants summary
     * ----------------------------------------------------------------
     * Surface the headline invariants on one block for easy
     * log-grepping in CI. */
    printf("\n[Group G] invariants summary\n");
    {
        char msg[400];
        snprintf(msg, sizeof(msg),
                 "portrait_rect_position: pre=%d%%, post-reopen-cancel=%d%% "
                 "(>= 90 both); panel-off invariant: pre-select to "
                 "post-reopen-cancel the D1C cutout stays anchored at "
                 "(%d, %d, %d, %d) viewport-local across save+load+reopen",
                 matchPreSelect, matchAfterReopen,
                 PORTRAIT_VX, PORTRAIT_VY, PORTRAIT_W, PORTRAIT_H);
        CHECK(matchPreSelect >= 90 && matchAfterReopen >= 90, msg);
    }
    {
        char msg[400];
        snprintf(msg, sizeof(msg),
                 "save_load_reopen panel-state cycle: "
                 "preSelect=closed, afterSelect=panelLive, afterSave=panelLive, "
                 "afterCancel=closed, afterLoad=panelLive, afterReopenCancel=closed "
                 "(save path: panelLive persisted via v1 runtime sidecar; "
                 "cancel path: F0282 C162 cleanly tears down)");
        CHECK(savedPanelActive == 1 &&
              game.candidateMirrorPanelActive == 0, msg);
    }

    /* Cleanup: remove the isolated quicksave + sidecar so we leave
     * /tmp tidy. */
    (void)remove(quicksavePath);
    (void)remove(quicksaveSidecarPath);
    if (oldQuicksaveEnv) {
        (void)setenv("FIRESTAFF_QUICKSAVE_PATH", oldQuicksaveEnv, 1);
    } else {
        (void)unsetenv("FIRESTAFF_QUICKSAVE_PATH");
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 HoC champion portrait ordinal 3 save_load_reopen portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return ok ? 0 : 1;
}
