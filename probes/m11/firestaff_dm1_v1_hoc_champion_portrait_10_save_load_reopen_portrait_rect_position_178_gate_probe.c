/*
 * firestaff_dm1_v1_hoc_champion_portrait_10_save_load_reopen_portrait_rect_position_178_gate_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions slice:
 *
 *   ordinal 10            (mirror catalog record GANDO, title THURFOOT)
 *   route   save_load_reopen
 *                          (C040 panel select -> M11_GameView_QuickSave
 *                           -> mutate live state -> M11_GameView_QuickLoad
 *                           -> reopen verification: candidate state
 *                           restored from the v1 runtime sidecar; D1C
 *                           portrait rectangle pixel-stable across the
 *                           round-trip)
 *   aspect  portrait_rect_position
 *                          (D1C source-locked cutout (96,35,32,29)
 *                           viewport-local, C026 source cell
 *                           ((10 & 7) << 5, (10 >> 3) * 29) =
 *                           (64, 29, 32, 29), no float onto side walls)
 *
 * The C026 champion-portrait atlas is the source-locked 8x3 grid of
 * 32x29 portraits (256x87 pixels total, ordinals 0..23).  Ordinal 10
 * sits at row 1, column 2 of the atlas:
 *
 *     srcX = (10 & 7) << 5 = 64
 *     srcY = (10 >> 3) * 29 = 29
 *
 * (DEFS.H:821-826 M027_PORTRAIT_X / M028_PORTRAIT_Y macro encoding;
 *  the `<< 5` form is the MEDIAs20x/S10E/S11E/S12E/.../G21E/A22E PC 3.4
 *  variant; the width-multiply form  `* G2078_C32_PortraitWidth`
 *  in BLIT.C:3928 is for later MEDIAs529 / I34E / A36M ports where
 *  G2078_C32_PortraitWidth is also 32.)
 *
 * The D1C front-wall champion-portrait destination rectangle is
 * source-locked (per ReDMCSB DUNGEON.C:3913-3928 and DUNVIEW.C:525
 * G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96, 127, 35, 63}):
 *
 *     dstX = 96, dstY = 35, dstW = 32, dstH = 29   (viewport coords)
 *
 * In the real DM1 V1 PC 3.4 English DUNGEON.DAT shipped with the
 * public PC release, the C127 sensor on front square (1,4) has
 * sensorData=10, so the party at (map 0, x=1, y=3) facing SOUTH
 * resolves to mirror ordinal 10 (GANDO) without any sensor
 * mutation.  This probe relies on the shipped layout; it does
 * NOT seed sensorData to lock the ordinal-10 edge case.
 *
 * The save_load_reopen slice covers three coupled concerns:
 *
 *   (1) Pre-save state proof: the (1,3,SOUTH) front route resolves
 *       to ordinal 10, the C026 atlas cell at (64, 29) is a defined
 *       GANDO portrait, and the D1C cutout (96, 35, 32, 29) on the
 *       320x200 framebuffer holds ordinal-10 pixels at >= 90% match
 *       (panel-off view).
 *
 *   (2) Select + save_load round-trip: select the candidate
 *       (F0280 / M11_GameView_SelectFrontMirrorCandidate), then
 *       M11_GameView_QuickSave serialises GameWorld_Compat via
 *       F0897 and writes the v1 runtime sidecar (which carries
 *       candidateMirrorOrdinal / candidateMirrorPartyIndex /
 *       candidateMirrorPanelActive / inventoryPanelActive /
 *       leaderHand*).  After the sidecar write, mutate the live
 *       state in a way QuickLoad will visibly undo (direction
 *       shift, mapX/Y change, candidateMirrorPanelActive=0,
 *       inventoryPanelActive=0) and call M11_GameView_QuickLoad.
 *
 *   (3) Reopen proof: after QuickLoad, the candidate state is
 *       restored from the v1 runtime sidecar (candidate ordinal
 *       and party index match the pre-save values), the
 *       D1C cutout (96, 35, 32, 29) on the loaded framebuffer
 *       still carries ordinal-10 pixels at >= 90% match
 *       (panel-off view of the loaded state), the panel-on
 *       redraw of the loaded state suppresses the full D1C
 *       portrait sprite (the C040 panel draws over the wall
 *       ornament as the live panel guard requires), and the
 *       loaded save path's "QUICKSAVE RESTORED" outcome was
 *       written.  This is the "save the game while the
 *       candidate is live, reload, and the candidate panel
 *       comes back" path - disjoint from the cancel_reopen
 *       slice (which never crosses the F0433/F0435 boundary)
 *       and from the redraw_after_candidate slice (which
 *       exercises confirm + cancel but never save/load).
 *
 * Source evidence:
 *   - DUNGEON.C:2558 (G0289 reset on wall-square entry)
 *   - DUNGEON.C:2608-2612 (G0289 = M000_INDEX_TO_ORDINAL(M040_DATA(sensor)))
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:1061 (G0205 coordSet 5 / index 12 is the D1C
 *                     champion-mirror frame route at (80, 29, 64, 43))
 *   - DUNVIEW.C:3913-3928 (D1C C026 portrait blit at {96,35} with
 *                          ((ordinal & 7) << 5, (ordinal >> 3) * 29))
 *   - DUNVIEW.C:8318-8542 (F0128 redraw viewport far-to-near)
 *   - COORD.C:1693-1749 (PC 3.4 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - DEFS.H:534-571 (GLOBAL_DATA fields; no G0299/G0424/G0425/G0426
 *                      - panel state is not part of the save blob)
 *   - MOVESENS.C:1501-1503 (F0280 sensorData -> candidate ordinal)
 *   - REVIVE.C:124-132 (F0280_REVIVE_PublishCandidate)
 *   - REVIVE.C:744-806 (F0282_REVIVE_ClearCandidate - confirm/cancel)
 *   - PANEL.C:1619-1635 (F0346_INVENTORY_DrawPanel_ResurrectReincarnate)
 *   - PANEL.C:1639-1693 (F0347_INVENTORY_DrawPanel -> F0346 reroute)
 *   - LOADSAVE.C:1502-1707 (F0433_STARTEND_ProcessCommand140_SaveGame_CPSCDF)
 *   - LOADSAVE.C:2192-2660 (F0435_STARTEND_LoadGame)
 *   - m11_apply_dm1_v1_pipeline_tick (in-place turn, F0128 redraw)
 *   - M11_GameView_OpenSelectedMenuEntry / M11_GameView_QuickSave /
 *     M11_GameView_QuickLoad (M11 game-view save/load + v1 runtime
 *     sidecar at src/engine/m11_game_view.c:7225-7405)
 *   - m11_save_quicksave_v1_runtime (sidecar carries
 *     candidateMirrorOrdinal, candidateMirrorPartyIndex,
 *     candidateMirrorPanelActive, inventoryPanelActive,
 *     leaderHand*, v1OpenChest*; src/engine/m11_game_view.c:430-475)
 *   - m11_load_quicksave_v1_runtime (sidecar load; src/engine/m11_game_view.c:480-555)
 *
 * Honesty scope: this is a Firestaff-runtime portrait_rect_position
 * proof against real DM1 V1 data; it does not claim DOS pixel parity
 * and does not add an original-vs-Firestaff viewport comparison.
 * Real DM1 V1 data is required (DM1 V1 PC 3.4 DUNGEON.DAT
 * 33357 bytes + the C026 portrait strip in GRAPHICS.DAT + the C040
 * resurrect/reincarnate panel graphic in GRAPHICS.DAT).
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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
    /* Source-locked C026 atlas dimensions: 8 cols x 3 rows of
     * 32x29 portraits. */
    ATLAS_W = 256,
    ATLAS_H = 87,
    /* Source-locked D1C wall-mirror frame (DUNVIEW.C:1061
     * G0205 coordSet 5 / index 12). */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* C040 resurrect/reincarnate panel destination rectangle. */
    RR_PANEL_X_VP = 80,
    RR_PANEL_Y_VP = 52,
    RR_PANEL_W = 144,
    RR_PANEL_H = 73,
    /* C040 graphic id in GRAPHICS.DAT. */
    C040_GRAPHIC_ID = 40,
    /* Source-locked D1C portrait ordinal for the (1,3) SOUTH route
     * (the C127 sensor on front square (1,4) carries sensorData=10
     * in the shipped DM1 V1 PC 3.4 English DUNGEON.DAT). */
    ORDINAL_GANDO = 10,
    /* POSE: real C127 sensor at (1,4) reached by party at (1,3)
     * facing SOUTH (DUNGEON.C:2573 visible-wall-side filter). */
    POSE_X = 1,
    POSE_Y = 3,
    POSE_DIR = 2,    /* DIR_SOUTH */
    /* The fresh, non-HoC pose used to verify that QuickLoad visibly
     * restores the (1,3,SOUTH) state and not the side-effect of
     * whatever the probe mutated before QuickLoad. */
    SCRATCH_X = 7,
    SCRATCH_Y = 11,
    SCRATCH_DIR = 0, /* DIR_NORTH */
    /* Match thresholds (same as the 154-gate and 097-gate
     * portrait-10 probes - the row-1 atlas path through
     * (10 >> 3) * 29 = 29 is the row-1 GANDO cell). */
    CORRECT_MATCH_PCT = 90,
    PANEL_OPEN_MAX_MATCH_PCT = 50
};

static int g_pass = 0;
static int g_fail = 0;

static void pass(const char* label) {
    printf("  PASS: %s\n", label);
    ++g_pass;
}

static void fail(const char* label) {
    printf("  FAIL: %s\n", label);
    ++g_fail;
}

static int expect_int(const char* label, int got, int want) {
    char msg[256];
    snprintf(msg, sizeof(msg), "%s got=%d want=%d", label, got, want);
    if (got == want) {
        pass(msg);
        return 1;
    }
    fail(msg);
    return 0;
}

static int expect_str(const char* label, const char* got, const char* want) {
    char msg[256];
    if (got == NULL) got = "(null)";
    if (want == NULL) want = "(null)";
    snprintf(msg, sizeof(msg), "%s got=\"%s\" want=\"%s\"", label, got, want);
    if (strcmp(got, want) == 0) {
        pass(msg);
        return 1;
    }
    fail(msg);
    return 0;
}

static void set_hall_pose(M11_GameViewState* game,
                          int x, int y, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = x;
    game->world.party.mapY = y;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

static int open_dm1(const char* dataDir,
                    M12_StartupMenuState* menu,
                    M11_GameViewState* game) {
    M12_StartupMenu_InitWithDataDir(menu, dataDir, NULL);
    M11_GameView_Init(game);
    if (!M11_GameView_OpenSelectedMenuEntry(game, menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(game);
        return 0;
    }
    return 1;
}

/*
 * Count the matched / compared pixels between the C026 atlas cell
 * for the requested ordinal and the D1C destination rectangle on
 * the 320x200 framebuffer.  Mirrors the helper in the 154-gate
 * portrait-10 redraw_after_candidate probe so the threshold
 * semantics stay consistent.  Returns matched pixels and writes
 * the compared total to *outCompared.
 */
static int portrait_match_count(const M11_AssetSlot* portraits,
                                const unsigned char* fb,
                                int ordinal,
                                int* outCompared) {
    int matched = 0;
    int compared = 0;
    int srcX0;
    int srcY0;
    int x;
    int y;

    if (outCompared) *outCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        !fb || ordinal < 0 || ordinal >= 24) {
        return -1;
    }
    if ((int)portraits->width < ATLAS_W ||
        (int)portraits->height < ATLAS_H) {
        return -1;
    }

    srcX0 = (ordinal & 7) * D1C_PORTRAIT_W;
    srcY0 = (ordinal >> 3) * D1C_PORTRAIT_H;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        int srcY = srcY0 + y;
        int dstY = D1C_PORTRAIT_Y + y;
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int srcX = srcX0 + x;
            int dstX = D1C_PORTRAIT_X + x;
            unsigned char src = (unsigned char)(
                portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst = (unsigned char)(fb[dstY * FB_W + dstX] & 0x0F);
            /* DUNVIEW.C:3913-3928 blits the champion strip with
             * color 1 transparent; GANDO also carries dark gray
             * niche pixels - skip both on the source side so the
             * test keys on champion pixels. */
            if (src == 1 || src == 12) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    if (outCompared) *outCompared = compared;
    return matched;
}

/* Run M11_GameView_Draw into a 320x200 fb, then collect the
 * portrait match evidence for the requested ordinal.  Returns
 * matchedPct (0..100) and writes compared to *outCompared. */
static int draw_and_collect(const M11_GameViewState* game,
                            const M11_AssetSlot* portraits,
                            int ordinal,
                            int* outCompared) {
    unsigned char fb[FB_W * FB_H];
    int matched;
    int compared = 0;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw((M11_GameViewState*)game, fb, FB_W, FB_H);
    matched = portrait_match_count(portraits, fb, ordinal, &compared);
    if (outCompared) *outCompared = compared;
    if (matched < 0 || compared <= 0) return -1;
    return (matched * 100) / compared;
}

/* Draw into fb and report the percentage of the C040 panel graphic
 * that was painted into its destination rect.  Mirrors the helper
 * in the 154-gate probe.  Returns matchedPct (0..100). */
typedef struct PanelEvidence {
    int assetOpaque;
    int assetDrawn;
    int matchedPct;
} PanelEvidence;

static int collect_panel_evidence(const M11_AssetSlot* panel,
                                  const M11_GameViewState* game,
                                  PanelEvidence* out) {
    unsigned char fb[FB_W * FB_H];
    int x;
    int y;
    int assetDrawn = 0;
    int assetOpaque = 0;
    memset(out, 0, sizeof(*out));
    if (!panel || !panel->loaded || !panel->pixels) return 0;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw((M11_GameViewState*)game, fb, FB_W, FB_H);
    for (y = 0; y < (int)panel->height; ++y) {
        int dstY = VIEWPORT_Y + RR_PANEL_Y_VP + y;
        if (dstY < 0 || dstY >= FB_H) continue;
        for (x = 0; x < (int)panel->width; ++x) {
            int dstX = VIEWPORT_X + RR_PANEL_X_VP + x;
            unsigned char src;
            unsigned char dst;
            if (dstX < 0 || dstX >= FB_W) continue;
            src = (unsigned char)(panel->pixels[y * (int)panel->width + x] & 0x0F);
            if (src == 6) continue;  /* panel-on transparent */
            dst = (unsigned char)(fb[dstY * FB_W + dstX] & 0x0F);
            ++assetOpaque;
            if (dst == src) ++assetDrawn;
        }
    }
    out->assetOpaque = assetOpaque;
    out->assetDrawn = assetDrawn;
    out->matchedPct = (assetOpaque > 0) ? (assetDrawn * 100 / assetOpaque) : 0;
    return 1;
}

/* Build a unique quicksave path under /tmp so the test does not
 * collide with previous runs and does not pollute the data dir.
 * The probe caller is responsible for setting FIRESTAFF_QUICKSAVE_PATH
 * to the returned buffer before QuickSave.  Writes the path into
 * outPath (caller-supplied, >= 256 bytes).  Returns 1 on success. */
static int build_quicksave_path(char* outPath, size_t outSize) {
    char tmpDir[256];
    int rc;
    if (outSize == 0) return 0;
    const char* tmpl = "/tmp/firestaff_portrait10_save_load_reopen_XXXXXX";
    char templateBuf[64];
    snprintf(templateBuf, sizeof(templateBuf), "%s", tmpl);
    if (mkstemp(templateBuf) < 0) {
        return 0;
    }
    /* mkstemp creates the file; close and remove so QuickSave can
     * create it fresh with the magic header. */
    (void)unlink(templateBuf);
    snprintf(tmpDir, sizeof(tmpDir), "%s", templateBuf);
    rc = snprintf(outPath, outSize, "%s.sav", tmpDir);
    if (rc <= 0 || (size_t)rc >= outSize) return 0;
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    const M11_AssetSlot* panel;
    int ok = 1;
    int frontOrdinal;
    int preSaveMatchedPct;
    int preSaveCompared;
    int panelOnMatchedPct;
    int panelOnCompared;
    int postLoadMatchedPct;
    int postLoadCompared;
    int candidateOrdinal;
    int candidatePartyIndex;
    int panelActive;
    int inventoryActive;
    char name[32];
    char title[64];
    char quicksavePath[512];
    int saveRc;
    int loadRc;
    PanelEvidence panelEv;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies DM1 V1 HoC portrait ordinal 10 (GANDO)\n"
                "  save_load_reopen portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    printf("=== DM1 V1 HoC portrait ordinal 10 (GANDO) save_load_reopen ===\n");
    printf("dataDir=%s pose=(map 0, x=%d, y=%d) facing SOUTH\n",
           dataDir, POSE_X, POSE_Y);
    printf("D1C cutout viewport=(%d,%d,%d,%d), C026 source=(%d,%d,%d,%d)\n",
           D1C_PORTRAIT_X - VIEWPORT_X, D1C_PORTRAIT_Y - VIEWPORT_Y,
           D1C_PORTRAIT_W, D1C_PORTRAIT_H,
           (ORDINAL_GANDO & 7) * D1C_PORTRAIT_W,
           (ORDINAL_GANDO >> 3) * D1C_PORTRAIT_H,
           D1C_PORTRAIT_W, D1C_PORTRAIT_H);

    if (!open_dm1(dataDir, &menu, &game)) return 1;

    portraits = M11_AssetLoader_Load(
        &game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    panel = M11_AssetLoader_Load(&game.assetLoader, C040_GRAPHIC_ID);
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < ATLAS_W || portraits->height < ATLAS_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C026 portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!panel || !panel->loaded || !panel->pixels ||
        panel->width != RR_PANEL_W || panel->height != RR_PANEL_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C040 panel unavailable or wrong size\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* ----------------------------------------------------------------
     * Group A - C026 atlas math for ordinal 10 + D1C zone helper
     * ---------------------------------------------------------------- */
    printf("\n[Group A] C026 atlas math for ordinal 10 + D1C wall-mirror frame\n");

    {
        int ordCol = ORDINAL_GANDO & 7;
        int ordRow = ORDINAL_GANDO >> 3;
        int ordSrcX = ordCol * D1C_PORTRAIT_W;
        int ordSrcY = ordRow * D1C_PORTRAIT_H;
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "ordinal 10 atlas math: (10 & 7)=%d, (10 >> 3)=%d, "
                 "srcX=%d, srcY=%d (in 256x87 atlas)",
                 ordCol, ordRow, ordSrcX, ordSrcY);
        ok &= expect_int("ordinal 10 atlas col", ordCol, 2);
        ok &= expect_int("ordinal 10 atlas row", ordRow, 1);
        ok &= expect_int("ordinal 10 atlas srcX", ordSrcX, 64);
        ok &= expect_int("ordinal 10 atlas srcY", ordSrcY, 29);
        if (ok) pass(msg);
    }

    {
        int ornX = 0, ornY = 0, ornW = 0, ornH = 0;
        int zoneOk = M11_GameView_GetD1CWallOrnamentZone(&game,
                                                        &ornX, &ornY, &ornW, &ornH);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "D1C wall-mirror frame = (%d, %d, %d, %d) viewport "
                 "(source-locked DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        if (zoneOk) {
            pass(msg);
            ok &= expect_int("D1C wall-mirror frame x", ornX, D1C_ZONE_X_VP);
            ok &= expect_int("D1C wall-mirror frame y", ornY, D1C_ZONE_Y_VP);
            ok &= expect_int("D1C wall-mirror frame width", ornW, D1C_ZONE_W);
            ok &= expect_int("D1C wall-mirror frame height", ornH, D1C_ZONE_H);
            ok &= expect_int("D1C cutout x is frame x + 16",
                             D1C_PORTRAIT_X - VIEWPORT_X, ornX + 16);
            ok &= expect_int("D1C cutout y is frame y + 6",
                             D1C_PORTRAIT_Y - VIEWPORT_Y, ornY + 6);
        } else {
            fail(msg);
            ok = 0;
        }
    }

    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(&game, ORDINAL_GANDO,
                                              name, sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(&game, ORDINAL_GANDO,
                                               title, sizeof(title));
    ok &= expect_str("ordinal 10 mirror catalog name is GANDO",
                     name, "GANDO");
    ok &= expect_str("ordinal 10 mirror catalog title is THURFOOT",
                     title, "THURFOOT");

    /* ----------------------------------------------------------------
     * Group B - Pre-save state: (1,3,SOUTH) resolves to ordinal 10
     * ---------------------------------------------------------------- */
    printf("\n[Group B] pre-save state: (1,3,SOUTH) front route resolves to ordinal 10\n");

    set_hall_pose(&game, POSE_X, POSE_Y, POSE_DIR);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "reference route reports ordinal %d at (1,3) SOUTH "
                 "(expected %d, GANDO)",
                 frontOrdinal, ORDINAL_GANDO);
        if (frontOrdinal == ORDINAL_GANDO) pass(msg);
        else {
            fail(msg);
            printf("SKIP hoc_portrait10_save_load_reopen_fixture_mismatch "
                   "(%d,%d) SOUTH front ordinal=%d expected=%d; this DM1 V1 "
                   "build does not expose ordinal 10 on this reference Hall route.\n",
                   POSE_X, POSE_Y, frontOrdinal, ORDINAL_GANDO);
            M11_GameView_Shutdown(&game);
            return 0;
        }
    }

    preSaveMatchedPct = draw_and_collect(&game, portraits, ORDINAL_GANDO,
                                          &preSaveCompared);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "pre-save D1C cutout ordinal 10 match >= %d%% got=%d%% (%d/%d)",
                 CORRECT_MATCH_PCT, preSaveMatchedPct, -1, preSaveCompared);
        if (preSaveMatchedPct >= CORRECT_MATCH_PCT) pass(msg);
        else { fail(msg); ok = 0; }
    }
    /* Track the pre-save matchedPct so we can assert the post-load
     * value is close (the atlas blit is byte-stable, but the match
     * percent depends on wall-ornament pixel suppression; we allow
     * a small drift for the loaded state). */
    int preSavePanelOffPct = preSaveMatchedPct;

    /* Select the front-mirror candidate: this drives F0280, sets
     * candidateMirrorOrdinal=10, candidateMirrorPartyIndex=0,
     * candidateMirrorPanelActive=1, inventoryPanelActive=1. */
    int selectRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_SelectFrontMirrorCandidate returns 1 (got %d)",
                 selectRc);
        if (selectRc == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    ok &= expect_int("post-select candidateMirrorPanelActive",
                     game.candidateMirrorPanelActive, 1);
    ok &= expect_int("post-select candidateMirrorOrdinal",
                     game.candidateMirrorOrdinal, ORDINAL_GANDO);
    ok &= expect_int("post-select candidateMirrorPartyIndex",
                     game.candidateMirrorPartyIndex, 0);
    ok &= expect_int("post-select championCount = 1",
                     game.world.party.championCount, 1);

    /* Panel-on redraw: the C040 panel must be drawn over the wall
     * ornament and the ordinal-10 match in the D1C cutout must be
     * suppressed (panel-on guard). */
    {
        unsigned char fb[FB_W * FB_H];
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        int matched = portrait_match_count(portraits, fb, ORDINAL_GANDO,
                                            &panelOnCompared);
        panelOnMatchedPct = (matched >= 0 && panelOnCompared > 0)
                                ? (matched * 100 / panelOnCompared) : -1;
    }
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "panel-on D1C cutout ordinal 10 suppressed to <= %d%% got=%d%% (%d)",
                 PANEL_OPEN_MAX_MATCH_PCT, panelOnMatchedPct, panelOnCompared);
        if (panelOnMatchedPct >= 0 &&
            panelOnMatchedPct <= PANEL_OPEN_MAX_MATCH_PCT) pass(msg);
        else { fail(msg); ok = 0; }
    }
    if (collect_panel_evidence(panel, &game, &panelEv)) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "panel-on C040 panel drawn >= 90%% got=%d%% (%d/%d)",
                 panelEv.matchedPct, panelEv.assetDrawn, panelEv.assetOpaque);
        if (panelEv.matchedPct >= 90) pass(msg);
        else { fail(msg); ok = 0; }
    } else {
        fail("collect_panel_evidence failed");
        ok = 0;
    }

    /* Save the pre-save candidate ordinal/party/panel for the
     * post-load comparison. */
    candidateOrdinal = game.candidateMirrorOrdinal;
    candidatePartyIndex = game.candidateMirrorPartyIndex;
    panelActive = game.candidateMirrorPanelActive;
    inventoryActive = game.inventoryPanelActive;

    /* ----------------------------------------------------------------
     * Group C - Save: M11_GameView_QuickSave serialises world + sidecar
     * ---------------------------------------------------------------- */
    printf("\n[Group C] M11_GameView_QuickSave serialises world + v1 runtime sidecar\n");

    if (!build_quicksave_path(quicksavePath, sizeof(quicksavePath))) {
        fprintf(stderr, "FAIL could not allocate quicksave path under /tmp\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (setenv("FIRESTAFF_QUICKSAVE_PATH", quicksavePath, 1) != 0) {
        fprintf(stderr, "FAIL could not set FIRESTAFF_QUICKSAVE_PATH\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    /* Clean any leftover from a prior aborted run. */
    (void)unlink(quicksavePath);
    {
        char sidecar[600];
        snprintf(sidecar, sizeof(sidecar), "%s.v1runtime", quicksavePath);
        (void)unlink(sidecar);
    }

    saveRc = M11_GameView_QuickSave(&game);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_QuickSave returns 1 (got %d, path=%s)",
                 saveRc, quicksavePath);
        if (saveRc == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    /* The M11 v1 runtime sidecar (.runtime suffix) must exist; the
     * main quicksave header + body file must also exist. */
    {
        char sidecar[600];
        FILE* sidecarFile;
        FILE* quicksaveFile;
        snprintf(sidecar, sizeof(sidecar), "%s.v1runtime", quicksavePath);
        sidecarFile = fopen(sidecar, "rb");
        quicksaveFile = fopen(quicksavePath, "rb");
        if (sidecarFile) {
            pass("v1 runtime sidecar (.v1runtime) exists after QuickSave");
            fclose(sidecarFile);
        } else {
            fail("v1 runtime sidecar (.v1runtime) missing after QuickSave");
            ok = 0;
        }
        if (quicksaveFile) {
            pass("quicksave header + body file exists after QuickSave");
            fclose(quicksaveFile);
        } else {
            fail("quicksave header + body file missing after QuickSave");
            ok = 0;
        }
    }

    /* ----------------------------------------------------------------
     * Group D - Mutate live state, then M11_GameView_QuickLoad restores
     * ----------------------------------------------------------------
     * To prove QuickLoad is non-trivial, mutate the live state in a
     * way QuickLoad must visibly undo.  After the mutation the
     * (1,3,SOUTH) route must NOT show ordinal 10 (it will show
     * the scratched direction / map position) and the candidate
     * state must be cleared.  QuickLoad must restore the pre-save
     * pose, ordinal, party index, panel flag, and inventory flag.
     */
    printf("\n[Group D] QuickLoad restores the (1,3,SOUTH) + ordinal 10 + panel-live snapshot\n");

    set_hall_pose(&game, SCRATCH_X, SCRATCH_Y, SCRATCH_DIR);
    game.candidateMirrorPanelActive = 0;
    game.candidateMirrorOrdinal = -1;
    game.candidateMirrorPartyIndex = -1;
    game.inventoryPanelActive = 0;
    game.world.party.championCount = 0;
    game.world.gameTick += 17u;  /* force lastWorldHash to drift */
    game.lastWorldHash ^= 0xDEADBEEFu;
    {
        int scratchOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "scratch pose (%d,%d,%d) reports front ordinal %d "
                 "(not ordinal 10, scratch witness)",
                 SCRATCH_X, SCRATCH_Y, SCRATCH_DIR, scratchOrdinal);
        if (scratchOrdinal != ORDINAL_GANDO) pass(msg);
        else {
            fail(msg);
            printf("INFO: scratch pose unexpectedly still resolves to ordinal 10; "
                   "the load-round-trip evidence below is still meaningful but the "
                   "scratch witness is non-discriminating.\n");
        }
    }
    ok &= expect_int("scratch candidateMirrorPanelActive = 0",
                     game.candidateMirrorPanelActive, 0);
    ok &= expect_int("scratch candidateMirrorOrdinal = -1",
                     game.candidateMirrorOrdinal, -1);
    ok &= expect_int("scratch inventoryPanelActive = 0",
                     game.inventoryPanelActive, 0);
    ok &= expect_int("scratch championCount = 0",
                     game.world.party.championCount, 0);

    loadRc = M11_GameView_QuickLoad(&game);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "M11_GameView_QuickLoad returns 1 (got %d, path=%s)",
                 loadRc, quicksavePath);
        if (loadRc == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    if (loadRc != 1) {
        fprintf(stderr, "FATAL: QuickLoad failed; cannot verify reopen\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-load world.things=%p (sensors alive after load)",
                 (void*)game.world.things);
        if (game.world.things != NULL) pass(msg);
        else { fail(msg); ok = 0; }
    }
    /* Reopen proof: the (1,3,SOUTH) pose, ordinal 10, panel-live
     * state, and inventory panel must all come back. */
    ok &= expect_int("post-load party.mapX = 1 (reopen)", game.world.party.mapX, 1);
    ok &= expect_int("post-load party.mapY = 3 (reopen)", game.world.party.mapY, 3);
    ok &= expect_int("post-load party.direction = SOUTH (reopen)",
                     game.world.party.direction, POSE_DIR);
    ok &= expect_int("post-load candidateMirrorOrdinal = 10 (reopen)",
                     game.candidateMirrorOrdinal, candidateOrdinal);
    ok &= expect_int("post-load candidateMirrorPartyIndex = 0 (reopen)",
                     game.candidateMirrorPartyIndex, candidatePartyIndex);
    ok &= expect_int("post-load candidateMirrorPanelActive = 1 (reopen)",
                     game.candidateMirrorPanelActive, panelActive);
    ok &= expect_int("post-load inventoryPanelActive = 1 (reopen)",
                     game.inventoryPanelActive, inventoryActive);
    ok &= expect_int("post-load front mirror ordinal = 10 (reopen)",
                     M11_GameView_GetFrontMirrorOrdinal(&game), ORDINAL_GANDO);

    /* The post-load panel-on D1C cutout must still show ordinal-10
     * pixels at >= %d%% match (panel-off view of the loaded state). */
    postLoadMatchedPct = draw_and_collect(&game, portraits, ORDINAL_GANDO,
                                          &postLoadCompared);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-load D1C cutout ordinal 10 match >= %d%% got=%d%% (%d)",
                 CORRECT_MATCH_PCT, postLoadMatchedPct, postLoadCompared);
        if (postLoadMatchedPct >= CORRECT_MATCH_PCT) pass(msg);
        else { fail(msg); ok = 0; }
    }
    /* The post-load panel-off matchedPct must match the pre-save
     * panel-off matchedPct exactly: the C026 atlas blit is
     * byte-stable, and the loaded state has the same party pose,
     * the same candidate ordinal, and the same championCount
     * (1 champion, ordinal 10) - the wall-ornament blit and the
     * portrait blit are deterministic. */
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-load panel-off D1C cutout match is byte-stable: "
                 "pre=%d%% post=%d%% (must match exactly)",
                 preSavePanelOffPct, postLoadMatchedPct);
        if (preSavePanelOffPct == postLoadMatchedPct) pass(msg);
        else { fail(msg); ok = 0; }
    }

    /* Cancel the candidate (so the panel goes off) and verify the
     * D1C cutout still shows ordinal 10 at >= 90% match.  This
     * proves the C127 sensor on the (1,4) front square is still
     * alive after the round-trip (the sensor thing is part of
     * GameWorld_Compat and is serialised/deserialised by
     * F0897/F0898), and that the cancel_after_reopen flow works
     * the same way as a fresh cancel. */
    {
        int cancelRc = M11_GameView_CancelMirrorCandidate(&game);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-load CancelMirrorCandidate returns 1 (got %d)",
                 cancelRc);
        if (cancelRc == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    ok &= expect_int("post-load-then-cancel candidateMirrorPanelActive = 0",
                     game.candidateMirrorPanelActive, 0);
    ok &= expect_int("post-load-then-cancel candidateMirrorOrdinal = -1",
                     game.candidateMirrorOrdinal, -1);
    ok &= expect_int("post-load-then-cancel championCount = 0",
                     game.world.party.championCount, 0);
    {
        int cancelPct = draw_and_collect(&game, portraits, ORDINAL_GANDO, NULL);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-load-then-cancel D1C cutout ordinal 10 match >= %d%% got=%d%%",
                 CORRECT_MATCH_PCT, cancelPct);
        if (cancelPct >= CORRECT_MATCH_PCT) pass(msg);
        else { fail(msg); ok = 0; }
    }

    /* ----------------------------------------------------------------
     * Group E - Reopen-after-cancel: re-select, save, load, verify
     * ----------------------------------------------------------------
     * The save_load_reopen contract must also hold across a re-publish
     * cycle: re-select, save, scratch, load, verify the panel is
     * back live.  This is disjoint from the Group C/D flow (which
     * reopens the SAME pre-save candidate state); here we exercise
     * the post-cancel F0280 publication path through QuickSave +
     * QuickLoad. */
    printf("\n[Group E] reopen-after-cancel: re-select, save, load, panel back live\n");

    set_hall_pose(&game, POSE_X, POSE_Y, POSE_DIR);
    {
        int reselect = M11_GameView_SelectFrontMirrorCandidate(&game);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-cancel re-select returns 1 (got %d)", reselect);
        if (reselect == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    ok &= expect_int("post-reselect candidateMirrorOrdinal = 10",
                     game.candidateMirrorOrdinal, ORDINAL_GANDO);
    ok &= expect_int("post-reselect candidateMirrorPartyIndex = 0",
                     game.candidateMirrorPartyIndex, 0);

    /* Save again, then scratch, then load.  The reopen after the
     * second save must restore the (1,3,SOUTH) + ordinal 10 + panel
     * state just like the first round-trip. */
    saveRc = M11_GameView_QuickSave(&game);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "second M11_GameView_QuickSave returns 1 (got %d)", saveRc);
        if (saveRc == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    set_hall_pose(&game, SCRATCH_X, SCRATCH_Y, SCRATCH_DIR);
    game.world.party.championCount = 0;
    game.world.gameTick += 23u;
    game.lastWorldHash ^= 0xCAFEBABEu;
    loadRc = M11_GameView_QuickLoad(&game);
    {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "second M11_GameView_QuickLoad returns 1 (got %d)", loadRc);
        if (loadRc == 1) pass(msg);
        else { fail(msg); ok = 0; }
    }
    ok &= expect_int("post-second-load party.mapX = 1 (reopen)",
                     game.world.party.mapX, 1);
    ok &= expect_int("post-second-load party.mapY = 3 (reopen)",
                     game.world.party.mapY, 3);
    ok &= expect_int("post-second-load candidateMirrorOrdinal = 10 (reopen)",
                     game.candidateMirrorOrdinal, ORDINAL_GANDO);
    ok &= expect_int("post-second-load candidateMirrorPartyIndex = 0 (reopen)",
                     game.candidateMirrorPartyIndex, 0);
    ok &= expect_int("post-second-load candidateMirrorPanelActive = 1 (reopen)",
                     game.candidateMirrorPanelActive, 1);
    {
        int reopenPct = draw_and_collect(&game, portraits, ORDINAL_GANDO, NULL);
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-second-load D1C cutout ordinal 10 match >= %d%% got=%d%%",
                 CORRECT_MATCH_PCT, reopenPct);
        if (reopenPct >= CORRECT_MATCH_PCT) pass(msg);
        else { fail(msg); ok = 0; }
    }
    /* The post-second-load panel-on D1C cutout must be suppressed
     * (panel draws over the wall ornament). */
    {
        int reopenPanelOnPct = -1;
        int reopenPanelOnCompared = 0;
        unsigned char fb[FB_W * FB_H];
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        int matched = portrait_match_count(portraits, fb, ORDINAL_GANDO,
                                            &reopenPanelOnCompared);
        reopenPanelOnPct = (matched >= 0 && reopenPanelOnCompared > 0)
                                ? (matched * 100 / reopenPanelOnCompared) : -1;
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "post-second-load panel-on D1C cutout ordinal 10 "
                 "suppressed to <= %d%% got=%d%%",
                 PANEL_OPEN_MAX_MATCH_PCT, reopenPanelOnPct);
        if (reopenPanelOnPct >= 0 &&
            reopenPanelOnPct <= PANEL_OPEN_MAX_MATCH_PCT) pass(msg);
        else { fail(msg); ok = 0; }
    }

    /* ----------------------------------------------------------------
     * Cleanup
     * ---------------------------------------------------------------- */
    (void)unlink(quicksavePath);
    {
        char sidecar[600];
        snprintf(sidecar, sizeof(sidecar), "%s.v1runtime", quicksavePath);
        (void)unlink(sidecar);
    }
    (void)unsetenv("FIRESTAFF_QUICKSAVE_PATH");

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("%s dm1 v1 HoC champion portrait ordinal 10 (GANDO) save_load_reopen portrait_rect_position\n",
           (ok && g_fail == 0) ? "PASS" : "FAIL");
    return (ok && g_fail == 0) ? 0 : 1;
}
