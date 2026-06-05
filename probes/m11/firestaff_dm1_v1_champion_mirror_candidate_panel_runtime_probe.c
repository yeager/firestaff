/*
 * DM1 V1 champion mirror candidate panel runtime probe.
 *
 * This is a real-asset pixel route for the Hall of Champions resurrect /
 * reincarnate candidate panel that appears on top of the viewport when a
 * mirror candidate is selected.  It broadens the existing Hall mirror /
 * no-floating coverage (champion_mirror_visibility, champion_mirror_zorder)
 * with the interaction-side gate: the panel graphic must be drawn on top
 * of the front mirror cell while a candidate is pending, and must be
 * cleared after resurrect/reincarnate confirm or cancel.
 *
 * Source evidence:
 *   ReDMCSB REVIVE.C F0282 (DM1 V1 MEDIA265_S20E) draws the
 *   resurrect/reincarnate panel once the candidate is appended; the panel
 *   asset is the C040 source graphic drawn at the source C101 zone
 *   (layout-696 centered at 80,52 inside the viewport).
 *   ReDMCSB CHAMPION.C F0280 appends the mirror candidate to the party;
 *   F0282 disable loop unlinks the mirror route's text entry on confirm;
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 champion portraits in G0289.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    PROBE_FB_W = 320,
    PROBE_FB_H = 200,
    PROBE_VIEWPORT_X = 0,
    PROBE_VIEWPORT_Y = 33,
    /* C101 source zone (PANEL.C F0342) is centered at (152,89) on the
     * 320x200 framebuffer.  That gives the (80,52,144,73) viewport-relative
     * rectangle we sample on top of the panel blit. */
    PROBE_PANEL_X = PROBE_VIEWPORT_X + 80,
    PROBE_PANEL_Y = PROBE_VIEWPORT_Y + 52,
    PROBE_PANEL_W = 144,
    PROBE_PANEL_H = 73,
    /* Resurrect/Reincarnate/Cancel panel graphic index in DM1 GRAPHICS.DAT.
     * The M11_GFX_PANEL_RESURRECT_REINCARNATE enum in m11_game_view.c is
     * file-scoped; the source-locked value 40 = C040 is the
     * PANEL.C F0342 candidate panel asset, drawn only while
     * candidateMirrorPanelActive is set. */
    PROBE_RR_PANEL_GRAPHIC = 40
};

typedef struct PanelMatch {
    int assetOpaque;   /* number of asset pixels that are not transparent */
    int assetDrawn;    /* asset pixels that ended up on the framebuffer */
    int leakedOpaque;  /* opaque asset pixels missing on the framebuffer */
    int assetWidth;
    int assetHeight;
} PanelMatch;

static PanelMatch match_panel(const M11_AssetSlot* panel,
                              const unsigned char* fb,
                              int fbW,
                              int fbH,
                              int panelX,
                              int panelY,
                              int transparentColor) {
    PanelMatch out;
    int x, y;
    memset(&out, 0, sizeof(out));
    if (!panel || !panel->loaded || !panel->pixels || !fb) {
        return out;
    }
    out.assetWidth = (int)panel->width;
    out.assetHeight = (int)panel->height;
    for (y = 0; y < out.assetHeight; ++y) {
        int fbY = panelY + y;
        if (fbY < 0 || fbY >= fbH) continue;
        for (x = 0; x < out.assetWidth; ++x) {
            int fbX = panelX + x;
            if (fbX < 0 || fbX >= fbW) continue;
            {
                unsigned char src = (unsigned char)(panel->pixels[y * out.assetWidth + x] & 0x0F);
                unsigned char dst =
                    M11_FB_DECODE_INDEX(fb[fbY * fbW + fbX]);
                if (src == transparentColor) {
                    continue;
                }
                ++out.assetOpaque;
                if (dst == src) {
                    ++out.assetDrawn;
                } else {
                    ++out.leakedOpaque;
                }
            }
        }
    }
    return out;
}

static int expect_int(const char* label, int got, int want) {
    if (got != want) {
        fprintf(stderr, "FAIL %s got=%d want=%d\n", label, got, want);
        return 0;
    }
    return 1;
}

static int expect_true(const char* label, int ok) {
    if (!ok) {
        fprintf(stderr, "FAIL %s\n", label);
        return 0;
    }
    return 1;
}

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
}

static int pose_panel_closed(M11_GameViewState* game,
                             const M11_AssetSlot* rrPanel,
                             int mapX,
                             int mapY,
                             int dir,
                             const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    int ok = 1;

    set_pose(game, mapX, mapY, dir);
    ok &= expect_int("candidate panel off", game->candidateMirrorPanelActive, 0);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (rrPanel && rrPanel->loaded && rrPanel->pixels) {
        /* No candidate panel: at most 5% of the panel's opaque pixels may
         * be present.  This rejects a regression where the resurrect /
         * reincarnate panel is drawn while no candidate is pending. */
        if (match.assetOpaque > 0 &&
            match.assetDrawn * 100 > 5 * match.assetOpaque) {
            fprintf(stderr,
                    "FAIL %s RR panel leaked when candidate closed drawn=%d/%d (leaked=%d)\n",
                    label, match.assetDrawn, match.assetOpaque, match.leakedOpaque);
            ok = 0;
        }
        printf("%s panel-closed drawn=%d/%d asset=%dx%d\n",
               label, match.assetDrawn, match.assetOpaque,
               match.assetWidth, match.assetHeight);
    } else {
        printf("%s panel-closed rr-panel asset unavailable\n", label);
    }
    return ok;
}

static int pose_panel_open(M11_GameViewState* game,
                           const M11_AssetSlot* rrPanel,
                           int mapX,
                           int mapY,
                           int dir,
                           int expectedOrdinal,
                           const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    int ok = 1;

    set_pose(game, mapX, mapY, dir);
    if (M11_GameView_GetFrontMirrorOrdinal(game) != expectedOrdinal) {
        fprintf(stderr, "FAIL %s front ordinal got=%d want=%d\n",
                label, M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);
        ok = 0;
    }
    if (M11_GameView_SelectFrontMirrorCandidate(game) != 1) {
        fprintf(stderr, "FAIL %s SelectFrontMirrorCandidate returned 0\n", label);
        return 0;
    }
    ok &= expect_int("candidate panel on", game->candidateMirrorPanelActive, 1);
    ok &= expect_int("candidate ordinal recorded", game->candidateMirrorOrdinal, expectedOrdinal);
    ok &= expect_int("candidate party index 0", game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("candidate champion appended", game->world.party.championCount, 1);
    ok &= expect_int("inventory panel on", game->inventoryPanelActive, 1);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    ok &= expect_true("RR panel asset available", rrPanel && rrPanel->loaded &&
                      rrPanel->pixels && rrPanel->width == PROBE_PANEL_W &&
                      rrPanel->height == PROBE_PANEL_H);
    if (!ok) return 0;

    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    /* At least 90% of opaque panel pixels must end up on the framebuffer.
     * This catches a regression where the panel asset is loaded but the
     * candidate panel branch is skipped, or where the asset is drawn at
     * the wrong zone. */
    if (match.assetOpaque <= 0 || match.assetDrawn * 100 < 90 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel missing drawn=%d/%d (leaked=%d)\n",
                label, match.assetDrawn, match.assetOpaque, match.leakedOpaque);
        ok = 0;
    }
    printf("%s panel-open drawn=%d/%d asset=%dx%d championCount=%d\n",
           label, match.assetDrawn, match.assetOpaque,
           match.assetWidth, match.assetHeight,
           game->world.party.championCount);
    return ok;
}

static int check_confirm_resurrect(M11_GameViewState* game,
                                   const M11_AssetSlot* rrPanel,
                                   const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    int ok = 1;

    ok &= expect_int("pre-confirm panel on", game->candidateMirrorPanelActive, 1);
    if (M11_GameView_ConfirmMirrorCandidate(game, 0) != 1) {
        fprintf(stderr, "FAIL %s ConfirmMirrorCandidate returned 0\n", label);
        return 0;
    }
    ok &= expect_int("post-confirm panel off", game->candidateMirrorPanelActive, 0);
    ok &= expect_int("post-confirm inventory off", game->inventoryPanelActive, 0);
    ok &= expect_int("post-confirm champion kept", game->world.party.championCount, 1);
    ok &= expect_int("post-confirm ordinal cleared", game->candidateMirrorOrdinal, -1);
    ok &= expect_int("post-confirm front mirror disabled",
                     M11_GameView_GetFrontMirrorOrdinal(game), -1);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (rrPanel && rrPanel->loaded && rrPanel->pixels && match.assetOpaque > 0 &&
        match.assetDrawn * 100 > 5 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel leaked after resurrect confirm drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s resurrect-confirm drawn=%d/%d championCount=%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount);
    return ok;
}

static int check_cancel(M11_GameViewState* game,
                        const M11_AssetSlot* rrPanel,
                        const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    int ok = 1;

    ok &= expect_int("pre-cancel panel on", game->candidateMirrorPanelActive, 1);
    if (M11_GameView_CancelMirrorCandidate(game) != 1) {
        fprintf(stderr, "FAIL %s CancelMirrorCandidate returned 0\n", label);
        return 0;
    }
    ok &= expect_int("post-cancel panel off", game->candidateMirrorPanelActive, 0);
    ok &= expect_int("post-cancel inventory off", game->inventoryPanelActive, 0);
    ok &= expect_int("post-cancel champion removed", game->world.party.championCount, 0);
    ok &= expect_int("post-cancel ordinal cleared", game->candidateMirrorOrdinal, -1);
    ok &= expect_int("post-cancel party index cleared", game->candidateMirrorPartyIndex, -1);
    ok &= expect_int("post-cancel front mirror still visible",
                     M11_GameView_GetFrontMirrorOrdinal(game), 2);
    {
        int reFront = M11_GameView_GetFrontMirrorOrdinal(game);
        if (reFront < 0) {
            fprintf(stderr, "FAIL %s cancel disabled the mirror route drawn ordinal=%d\n",
                    label, reFront);
            ok = 0;
        }
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (rrPanel && rrPanel->loaded && rrPanel->pixels && match.assetOpaque > 0 &&
        match.assetDrawn * 100 > 5 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel leaked after cancel drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s cancel drawn=%d/%d championCount=%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    M12_StartupMenuState menu2;
    M11_GameViewState game2;
    const M11_AssetSlot* rrPanel;
    int ok = 1;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* The C040 RESURRECT_REINCARNATE panel is a single-frame asset drawn
     * only on top of an open candidate panel.  Loading it here once lets
     * every pose compare its target framebuffer against the real pixel
     * source rather than guessing from the inventory backdrop. */
    rrPanel = M11_AssetLoader_Load((M11_AssetLoader*)&game.assetLoader,
                                   (unsigned int)PROBE_RR_PANEL_GRAPHIC);
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C040 resurrect/reincarnate panel asset unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (rrPanel->width != PROBE_PANEL_W || rrPanel->height != PROBE_PANEL_H) {
        fprintf(stderr, "FAIL C040 panel asset size got=%ux%u want=%dx%d\n",
                rrPanel->width, rrPanel->height, PROBE_PANEL_W, PROBE_PANEL_H);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Pose A: corridor (1,4) facing north has mirror ordinal 2 on the
     * front cell.  No candidate is pending, so the panel must NOT be
     * drawn on top of the viewport. */
    ok &= pose_panel_closed(&game, rrPanel, 1, 4, DIR_NORTH,
                            "corridor_north_no_candidate");
    /* Pose B: hall start (1,3) facing north has mirror ordinal 1.  Same
     * rule: no panel without a candidate. */
    ok &= pose_panel_closed(&game, rrPanel, 1, 3, DIR_NORTH,
                            "hall_start_north_no_candidate");
    /* Pose C: facing east and west, no panel either. */
    ok &= pose_panel_closed(&game, rrPanel, 1, 4, DIR_EAST,
                            "corridor_east_no_candidate");
    ok &= pose_panel_closed(&game, rrPanel, 1, 4, DIR_WEST,
                            "corridor_west_no_candidate");

    /* Pose D: corridor (1,4) facing north, select front mirror candidate
     * (ordinal 2).  The RR panel must be drawn on top of the viewport
     * and a new champion must be appended to the party. */
    ok &= pose_panel_open(&game, rrPanel, 1, 4, DIR_NORTH, 2,
                          "corridor_north_select_candidate");
    /* Pose E: resurrect confirm keeps the champion, closes the panel, and
     * disables the mirror route. */
    ok &= check_confirm_resurrect(&game, rrPanel, "corridor_north_resurrect");

    /* Pose F: reopen the candidate on a fresh view, then cancel.  The
     * appended champion must be removed, the panel must be closed, and
     * the mirror route must still be available. */
    M12_StartupMenu_InitWithDataDir(&menu2, dataDir, NULL);
    M11_GameView_Init(&game2);
    if (!M11_GameView_OpenSelectedMenuEntry(&game2, &menu2)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for cancel pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game2, rrPanel, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_cancel")) {
        ok = 0;
    } else {
        ok &= check_cancel(&game2, rrPanel, "corridor_north_cancel");
    }
    M11_GameView_Shutdown(&game2);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror candidate panel runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
