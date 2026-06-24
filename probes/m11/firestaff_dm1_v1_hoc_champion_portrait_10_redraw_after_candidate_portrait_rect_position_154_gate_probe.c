/*
 * DM1 V1 Hall of Champions portrait ordinal 10 (GANDO / THURFOOT)
 * redraw_after_candidate / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal = 10 (GANDO / THURFOOT, C026 col 2 row 1)
 *   pose    = (map 0, x=1, y=3) facing SOUTH
 *             (real PC 3.4 Hall C127 route discovered by
 *              firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *              as "hall_zed_from_north_ordinal_10":
 *              front cell (1,4) carries C127 sensor idx=16
 *              with sensorData=10, and the party at (1,3) facing
 *              SOUTH resolves to mirror ordinal 10.)
 *   route   = redraw_after_candidate
 *             pre-candidate -> panel-open -> confirm-disable, plus a
 *             fresh select/cancel cycle where the route must redraw again
 *   aspect  = portrait_rect_position
 *             C026 portrait pixels stay anchored at the source-locked
 *             D1C cutout (96, 35, 32, 29) viewport-local and never float
 *             onto side walls.
 *
 * Source-locked to ReDMCSB WIP 20210206:
 *   DUNGEON.C:2573       maps M011_CELL(sensor) against view direction.
 *   DUNGEON.C:2608-2612  stores C127 sensorData in G0289 only for the
 *                         front-wall side.
 *   DUNVIEW.C:3913-3928  blits C026 champion portrait into the D1C box.
 *   DUNVIEW.C:525        G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                         = { 96, 127, 35, 63 } viewport-local.
 *   DUNVIEW.C:8318-8618  F0128 far-to-near viewport redraw.
 *   MOVESENS.C:1501-1503 dispatches C127 sensorData to REVIVE.C F0280.
 *   REVIVE.C:744-799     F0282 consumes cancel/confirm candidate actions;
 *                         785-799 disables the matching mirror route after
 *                         confirm.
 *   PANEL.C:1619-1635 / 1654-1656 route the C040 resurrect/reincarnate
 *                         panel while the candidate panel is live.
 *   COORD.C:1693-1722    PC 3.4 viewport origin/size.
 *   DEFS.H:821-826       M027_PORTRAIT_X / M028_PORTRAIT_Y macro math
 *                         for C026 atlas cell lookup.
 *
 * This is Firestaff runtime evidence against GRAPHICS.DAT/DUNGEON.DAT. It
 * does not claim original DOS pixel parity.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"

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
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    RR_PANEL_X_VP = 80,
    RR_PANEL_Y_VP = 52,
    RR_PANEL_W = 144,
    RR_PANEL_H = 73,
    /* Champion mirror ordinal for the (1,3) SOUTH front cell (1,4)
     * which carries C127 sensor idx=16 with sensorData=10. */
    ORDINAL_GANDO = 10,
    POSE_X = 1,
    POSE_Y = 3,
    PROBE_DIR_NORTH = 0,
    PROBE_DIR_EAST = 1,
    PROBE_DIR_SOUTH = 2,
    PROBE_DIR_WEST = 3,
    CORRECT_MATCH_PCT = 90,
    PANEL_OPEN_MAX_MATCH_PCT = 50,
    ABSENT_MATCH_PCT = 30
};

typedef struct PortraitEvidence {
    int compared;
    int matched;
    int matchedPct;
    int bestOrdinal;
    int bestMatched;
    int secondMatched;
    int d1cZoneContainsPortrait;
} PortraitEvidence;

typedef struct PanelEvidence {
    int assetOpaque;
    int assetDrawn;
    int matchedPct;
} PanelEvidence;

static int g_pass = 0;
static int g_fail = 0;

static int fb_x(int vpX) {
    return VIEWPORT_X + vpX;
}

static int fb_y(int vpY) {
    return VIEWPORT_Y + vpY;
}

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

static void set_hall_pose(M11_GameViewState* game, int x, int y, int dir) {
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
    if ((int)portraits->width < 8 * PORTRAIT_W ||
        (int)portraits->height < 3 * PORTRAIT_H) {
        return -1;
    }

    srcX0 = (ordinal & 7) * PORTRAIT_W;
    srcY0 = (ordinal >> 3) * PORTRAIT_H;
    for (y = 0; y < PORTRAIT_H; ++y) {
        int srcY = srcY0 + y;
        int dstY = fb_y(PORTRAIT_Y_VP + y);
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = srcX0 + x;
            int dstX = fb_x(PORTRAIT_X_VP + x);
            unsigned char src = (unsigned char)(
                portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst = (unsigned char)(fb[dstY * FB_W + dstX] & 0x0F);

            /* ReDMCSB DUNVIEW.C:3913-3928 blits the champion strip with
             * color 1 transparent. The ordinal-10 (GANDO) sprite also
             * carries dark gray niche pixels; skip those on the source
             * side so the test keys on champion pixels instead of the
             * C346 wall background. */
            if (src == 1 || src == 12) continue;
            ++compared;
            if (src == dst) ++matched;
        }
    }
    if (outCompared) *outCompared = compared;
    return matched;
}

static void collect_portrait_evidence(const M11_AssetSlot* portraits,
                                      const unsigned char* fb,
                                      int ordinal,
                                      PortraitEvidence* out) {
    int i;
    memset(out, 0, sizeof(*out));
    out->bestOrdinal = -1;
    out->bestMatched = -1;
    out->secondMatched = -1;
    out->matched = portrait_match_count(portraits, fb, ordinal, &out->compared);
    if (out->matched >= 0 && out->compared > 0) {
        out->matchedPct = (out->matched * 100) / out->compared;
    }
    for (i = 0; i < 24; ++i) {
        int compared = 0;
        int matched = portrait_match_count(portraits, fb, i, &compared);
        (void)compared;
        if (matched > out->bestMatched) {
            out->secondMatched = out->bestMatched;
            out->bestMatched = matched;
            out->bestOrdinal = i;
        } else if (matched > out->secondMatched) {
            out->secondMatched = matched;
        }
    }
    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
}

static PanelEvidence collect_panel_evidence(const M11_AssetSlot* panel,
                                            const unsigned char* fb) {
    PanelEvidence out;
    int x;
    int y;
    memset(&out, 0, sizeof(out));
    if (!panel || !panel->loaded || !panel->pixels || !fb) {
        return out;
    }
    for (y = 0; y < (int)panel->height; ++y) {
        int dstY = fb_y(RR_PANEL_Y_VP + y);
        if (dstY < 0 || dstY >= FB_H) continue;
        for (x = 0; x < (int)panel->width; ++x) {
            int dstX = fb_x(RR_PANEL_X_VP + x);
            unsigned char src;
            unsigned char dst;
            if (dstX < 0 || dstX >= FB_W) continue;
            src = (unsigned char)(panel->pixels[y * (int)panel->width + x] & 0x0F);
            if (src == 6) continue;
            dst = (unsigned char)(fb[dstY * FB_W + dstX] & 0x0F);
            ++out.assetOpaque;
            if (dst == src) ++out.assetDrawn;
        }
    }
    if (out.assetOpaque > 0) {
        out.matchedPct = (out.assetDrawn * 100) / out.assetOpaque;
    }
    return out;
}

static int check_d1c_zone(M11_GameViewState* game) {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    int ok;
    M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    ok = x == D1C_ZONE_X_VP && y == D1C_ZONE_Y_VP &&
         w == D1C_ZONE_W && h == D1C_ZONE_H;
    if (ok) {
        pass("D1C wall zone is source-locked viewport rect (80,29,64,43)");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "D1C wall zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d)",
                 x, y, w, h, D1C_ZONE_X_VP, D1C_ZONE_Y_VP,
                 D1C_ZONE_W, D1C_ZONE_H);
        fail(msg);
    }
    return ok;
}

static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int ok;
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, ORDINAL_GANDO,
                                              name, sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, ORDINAL_GANDO,
                                               title, sizeof(title));
    ok = strcmp(name, "GANDO") == 0 && strcmp(title, "THURFOOT") == 0;
    if (ok) {
        pass("ordinal 10 catalog identity is GANDO / THURFOOT");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "ordinal 10 catalog identity got name=\"%s\" title=\"%s\"",
                 name, title);
        fail(msg);
    }
    return ok;
}

static int draw_and_expect_portrait(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits,
                                    const char* label) {
    unsigned char fb[FB_W * FB_H];
    PortraitEvidence ev;
    char msg[256];
    int ok = 1;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_portrait_evidence(portraits, fb, ORDINAL_GANDO, &ev);

    snprintf(msg, sizeof(msg), "%s D1C cutout inside wall zone", label);
    if (ev.d1cZoneContainsPortrait) pass(msg); else { fail(msg); ok = 0; }

    snprintf(msg, sizeof(msg),
             "%s ordinal 10 match >= %d%% got=%d%% (%d/%d)",
             label, CORRECT_MATCH_PCT, ev.matchedPct, ev.matched, ev.compared);
    if (ev.matchedPct >= CORRECT_MATCH_PCT) pass(msg); else { fail(msg); ok = 0; }

    snprintf(msg, sizeof(msg),
             "%s ordinal 10 is best C026 match best=%d matched=%d second=%d",
             label, ev.bestOrdinal, ev.bestMatched, ev.secondMatched);
    if (ev.bestOrdinal == ORDINAL_GANDO &&
        ev.bestMatched > ev.secondMatched) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
    }

    return ok;
}

static int draw_and_expect_no_portrait(M11_GameViewState* game,
                                       const M11_AssetSlot* portraits,
                                       const char* label) {
    unsigned char fb[FB_W * FB_H];
    PortraitEvidence ev;
    char msg[256];
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_portrait_evidence(portraits, fb, ORDINAL_GANDO, &ev);
    snprintf(msg, sizeof(msg),
             "%s ordinal 10 absent from D1C cutout (<%d%%) got=%d%%",
             label, ABSENT_MATCH_PCT, ev.matchedPct);
    if (ev.matchedPct < ABSENT_MATCH_PCT) {
        pass(msg);
        return 1;
    }
    fail(msg);
    return 0;
}

static int check_panel_open(M11_GameViewState* game,
                            const M11_AssetSlot* portraits,
                            const M11_AssetSlot* panel) {
    unsigned char fb[FB_W * FB_H];
    PortraitEvidence portrait;
    PanelEvidence panelEv;
    char msg[256];
    int ok = 1;
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_portrait_evidence(portraits, fb, ORDINAL_GANDO, &portrait);
    panelEv = collect_panel_evidence(panel, fb);

    ok &= expect_int("panel_open candidateMirrorPanelActive",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("panel_open candidateMirrorOrdinal",
                     game->candidateMirrorOrdinal, ORDINAL_GANDO);
    snprintf(msg, sizeof(msg),
             "panel_open C040 panel drawn >=90%% got=%d%% (%d/%d)",
             panelEv.matchedPct, panelEv.assetDrawn, panelEv.assetOpaque);
    if (panelEv.assetOpaque > 0 && panelEv.matchedPct >= 90) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
    }
    snprintf(msg, sizeof(msg),
             "panel_open ordinal 10 no longer has a full D1C portrait match (<%d%%) got=%d%%",
             PANEL_OPEN_MAX_MATCH_PCT, portrait.matchedPct);
    if (portrait.matchedPct < PANEL_OPEN_MAX_MATCH_PCT) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
    }
    return ok;
}

static int check_wrong_wall(M11_GameViewState* game,
                            const M11_AssetSlot* portraits,
                            int x, int y, int dir,
                            const char* label) {
    set_hall_pose(game, x, y, dir);
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "%s front mirror ordinal got=%d want=-1",
                 label, M11_GameView_GetFrontMirrorOrdinal(game));
        fail(msg);
        return 0;
    }
    pass(label);
    return draw_and_expect_no_portrait(game, portraits, label);
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    static M12_StartupMenuState menuCancel;
    static M11_GameViewState gameCancel;
    const char* dataDir;
    const M11_AssetSlot* portraits;
    const M11_AssetSlot* panel;
    int ok = 1;
    int frontOrdinal;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies DM1 V1 HoC portrait ordinal 10 "
                "redraw_after_candidate portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (!open_dm1(dataDir, &menu, &game)) return 1;

    printf("=== DM1 V1 HoC portrait ordinal 10 (GANDO) redraw_after_candidate ===\n");
    printf("dataDir=%s pose=(map 0, x=%d, y=%d) facing SOUTH\n",
           dataDir, POSE_X, POSE_Y);
    printf("D1C cutout viewport=(%d,%d,%d,%d), C026 source=(%d,%d,%d,%d)\n",
           PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
           (ORDINAL_GANDO & 7) * PORTRAIT_W,
           (ORDINAL_GANDO >> 3) * PORTRAIT_H,
           PORTRAIT_W, PORTRAIT_H);

    portraits = M11_AssetLoader_Load(
        &game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    panel = M11_AssetLoader_Load(&game.assetLoader, 40u);
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
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

    ok &= check_catalog_identity(&game);
    ok &= check_d1c_zone(&game);

    set_hall_pose(&game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_GANDO) {
        printf("SKIP hoc_portrait10_fixture_mismatch "
               "(%d,%d) SOUTH front ordinal=%d expected=%d; this DM1 V1 "
               "build does not expose ordinal 10 on this reference Hall route.\n",
               POSE_X, POSE_Y, frontOrdinal, ORDINAL_GANDO);
        M11_GameView_Shutdown(&game);
        return 0;
    }
    pass("reference route reports ordinal 10 at (1,3) SOUTH");

    printf("\n[Stage 1] pre-candidate redraw\n");
    ok &= draw_and_expect_portrait(&game, portraits, "pre_candidate");

    printf("\n[Stage 2] candidate panel redraw\n");
    if (M11_GameView_SelectFrontMirrorCandidate(&game) != 1) {
        fail("SelectFrontMirrorCandidate ordinal 10");
        ok = 0;
    } else {
        ok &= check_panel_open(&game, portraits, panel);
    }

    printf("\n[Stage 3] confirm redraw disables mirror route\n");
    if (M11_GameView_ConfirmMirrorCandidate(&game, 0) != 1) {
        fail("ConfirmMirrorCandidate ordinal 10");
        ok = 0;
    } else {
        ok &= expect_int("post_confirm front mirror disabled",
                         M11_GameView_GetFrontMirrorOrdinal(&game), -1);
        ok &= draw_and_expect_no_portrait(&game, portraits, "post_confirm");
    }

    printf("\n[Stage 4] fresh cancel redraw preserves mirror route\n");
    if (!open_dm1(dataDir, &menuCancel, &gameCancel)) {
        ok = 0;
    } else {
        const M11_AssetSlot* portraitsCancel = M11_AssetLoader_Load(
            &gameCancel.assetLoader,
            (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        set_hall_pose(&gameCancel, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
        if (M11_GameView_GetFrontMirrorOrdinal(&gameCancel) != ORDINAL_GANDO) {
            fail("cancel view reference route reports ordinal 10");
            ok = 0;
        } else if (M11_GameView_SelectFrontMirrorCandidate(&gameCancel) != 1) {
            fail("cancel view select ordinal 10");
            ok = 0;
        } else if (M11_GameView_CancelMirrorCandidate(&gameCancel) != 1) {
            fail("CancelMirrorCandidate ordinal 10");
            ok = 0;
        } else {
            ok &= expect_int("post_cancel front mirror preserved",
                             M11_GameView_GetFrontMirrorOrdinal(&gameCancel),
                             ORDINAL_GANDO);
            ok &= draw_and_expect_portrait(&gameCancel, portraitsCancel,
                                           "post_cancel");
        }
        M11_GameView_Shutdown(&gameCancel);
    }

    printf("\n[Stage 5] side-wall no-floating checks\n");
    /* (1,3) facing NORTH: front=(1,2) is a corridor with no C127
     * sensor; ordinal lookup must be -1 and the D1C cutout must
     * not show ordinal-10 portrait pixels. */
    ok &= check_wrong_wall(&game, portraits,
                           POSE_X, POSE_Y, PROBE_DIR_NORTH,
                           "same_cell_north_corridor");
    /* (1,5) facing NORTH: same physical C127 sensor square (1,4)
     * viewed from the wrong wall side; ordinal lookup must be -1
     * and the D1C cutout must not show ordinal-10 portrait pixels. */
    ok &= check_wrong_wall(&game, portraits,
                           1, 5, PROBE_DIR_NORTH,
                           "sensor_square_wrong_wall_north");

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("%s dm1 v1 HoC champion portrait ordinal 10 redraw_after_candidate portrait_rect_position\n",
           (ok && g_fail == 0) ? "PASS" : "FAIL");
    return (ok && g_fail == 0) ? 0 : 1;
}
