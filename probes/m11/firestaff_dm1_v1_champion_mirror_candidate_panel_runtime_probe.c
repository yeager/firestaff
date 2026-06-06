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
 *   ReDMCSB PANEL.C:1619-1635 / F0346 blits C040 to C101, and
 *   PANEL.C:1654-1656 / F0347 routes to that panel while G0299 is set.
 *   ReDMCSB REVIVE.C:272-276 / F0280 appends the mirror candidate to
 *   the party; REVIVE.C:744-799 / F0282 cancels without disabling the
 *   route but disables the first mirror-square sensor on confirm;
 *   REVIVE.C:806-835 applies the source reincarnate branch after the
 *   common confirm route while C160 resurrect stays on the common confirm
 *   path through REVIVE.C:785-837; COMMAND.C:228-233, 508-511,
 *   1379-1431, and 1985-1991 define the panel mouse boxes, ignore
 *   non-matching panel clicks, and dispatch only C160/C161/C162 to
 *   REVIVE.C F0282; COMMAND.C:2159-2181 blocks status-box/inventory
 *   toggles while G0299 marks an open candidate panel; COMMAND.C:2336-2338
 *   keeps C145 rest disabled while G0299 is live;
 *   ReDMCSB DUNVIEW.C:3913-3928 and 8522-8533 restrict champion-portrait
 *   interaction evidence to the D1C front wall route;
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
    /* DUNVIEW.C:3913-3928 blits the D1C C026 champion portrait at this
     * source wall box before PANEL.C:1619-1635 draws C040 over the open
     * candidate panel.  The bottom of the mirror portrait overlaps the top
     * of C101, so this probe can assert the source draw order directly. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* COMMAND.C:228-233 / 508-511 route these panel boxes to C160/C161/C162.
     * The midpoints avoid border pixels and prove the open candidate panel
     * consumes the clicks before generic viewport routing can reinterpret
     * them. */
    PROBE_RESURRECT_CLICK_X = 130,
    PROBE_RESURRECT_CLICK_Y = 115,
    PROBE_PANEL_DEADZONE_CLICK_X = 160,
    PROBE_PANEL_DEADZONE_CLICK_Y = 115,
    PROBE_REINCARNATE_CLICK_X = 186,
    PROBE_REINCARNATE_CLICK_Y = 115,
    PROBE_CANCEL_CLICK_X = 160,
    PROBE_CANCEL_CLICK_Y = 151,
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

typedef struct PanelOverPortraitMatch {
    int compared;
    int panelWins;
    int portraitWins;
    int other;
} PanelOverPortraitMatch;

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

static PanelOverPortraitMatch match_panel_over_portrait(
    const M11_AssetSlot* panel,
    const M11_AssetSlot* portraits,
    const unsigned char* fb,
    int fbW,
    int fbH,
    int panelX,
    int panelY,
    int portraitX,
    int portraitY,
    int portraitOrdinal,
    int panelTransparentColor,
    int portraitTransparentColor) {
    PanelOverPortraitMatch out;
    int overlapX1;
    int overlapY1;
    int overlapX2;
    int overlapY2;
    int x;
    int y;

    memset(&out, 0, sizeof(out));
    if (!panel || !panel->loaded || !panel->pixels ||
        !portraits || !portraits->loaded || !portraits->pixels || !fb ||
        portraitOrdinal < 0) {
        return out;
    }

    overlapX1 = panelX > portraitX ? panelX : portraitX;
    overlapY1 = panelY > portraitY ? panelY : portraitY;
    overlapX2 = (panelX + (int)panel->width) <
                (portraitX + PROBE_PORTRAIT_W)
                    ? (panelX + (int)panel->width)
                    : (portraitX + PROBE_PORTRAIT_W);
    overlapY2 = (panelY + (int)panel->height) <
                (portraitY + PROBE_PORTRAIT_H)
                    ? (panelY + (int)panel->height)
                    : (portraitY + PROBE_PORTRAIT_H);

    for (y = overlapY1; y < overlapY2; ++y) {
        if (y < 0 || y >= fbH) continue;
        for (x = overlapX1; x < overlapX2; ++x) {
            int panelLocalX = x - panelX;
            int panelLocalY = y - panelY;
            int portraitLocalX = x - portraitX;
            int portraitLocalY = y - portraitY;
            int portraitSrcX = (portraitOrdinal & 7) * PROBE_PORTRAIT_W +
                               portraitLocalX;
            int portraitSrcY = (portraitOrdinal >> 3) * PROBE_PORTRAIT_H +
                               portraitLocalY;
            unsigned char panelSrc;
            unsigned char portraitSrc;
            unsigned char dst;

            if (x < 0 || x >= fbW ||
                panelLocalX < 0 || panelLocalX >= (int)panel->width ||
                panelLocalY < 0 || panelLocalY >= (int)panel->height ||
                portraitSrcX < 0 || portraitSrcX >= (int)portraits->width ||
                portraitSrcY < 0 || portraitSrcY >= (int)portraits->height) {
                continue;
            }

            panelSrc = (unsigned char)
                (panel->pixels[panelLocalY * (int)panel->width + panelLocalX] & 0x0F);
            portraitSrc = (unsigned char)
                (portraits->pixels[portraitSrcY * (int)portraits->width +
                                  portraitSrcX] & 0x0F);
            if (panelSrc == panelTransparentColor ||
                portraitSrc == portraitTransparentColor ||
                panelSrc == portraitSrc) {
                continue;
            }

            dst = M11_FB_DECODE_INDEX(fb[y * fbW + x]);
            ++out.compared;
            if (dst == panelSrc) {
                ++out.panelWins;
            } else if (dst == portraitSrc) {
                ++out.portraitWins;
            } else {
                ++out.other;
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

static int pose_select_miss(M11_GameViewState* game,
                            int mapX,
                            int mapY,
                            int dir,
                            const char* label) {
    int result;
    int ok = 1;
    char checkLabel[128];

    set_pose(game, mapX, mapY, dir);
    snprintf(checkLabel, sizeof(checkLabel), "%s no front mirror route", label);
    ok &= expect_int(checkLabel, M11_GameView_GetFrontMirrorOrdinal(game), -1);

    /* ReDMCSB REVIVE.C F0280 is reached only through the source champion
     * portrait route: after the D1C front-wall portrait is selected, it
     * sets G0299 and increments G0305 (REVIVE.C:272-276).  A side/no-front
     * pose must therefore leave the candidate and party state untouched. */
    result = M11_GameView_SelectFrontMirrorCandidate(game);
    snprintf(checkLabel, sizeof(checkLabel), "%s select rejected", label);
    ok &= expect_int(checkLabel, result, 0);
    snprintf(checkLabel, sizeof(checkLabel), "%s panel still off", label);
    ok &= expect_int(checkLabel, game->candidateMirrorPanelActive, 0);
    snprintf(checkLabel, sizeof(checkLabel), "%s ordinal still clear", label);
    ok &= expect_int(checkLabel, game->candidateMirrorOrdinal, -1);
    snprintf(checkLabel, sizeof(checkLabel), "%s party index still clear", label);
    ok &= expect_int(checkLabel, game->candidateMirrorPartyIndex, -1);
    snprintf(checkLabel, sizeof(checkLabel), "%s no candidate appended", label);
    ok &= expect_int(checkLabel, game->world.party.championCount, 0);
    snprintf(checkLabel, sizeof(checkLabel), "%s inventory still off", label);
    ok &= expect_int(checkLabel, game->inventoryPanelActive, 0);

    printf("%s select-miss result=%d championCount=%d candidate=%d/%d panel=%d\n",
           label, result, game->world.party.championCount,
           game->candidateMirrorOrdinal, game->candidateMirrorPartyIndex,
           game->candidateMirrorPanelActive);
    return ok;
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
                           const M11_AssetSlot* portraits,
                           int mapX,
                           int mapY,
                           int dir,
                           int expectedOrdinal,
                           const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    PanelOverPortraitMatch overlap;
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
    /* REVIVE.C:272-276 only records/appends the pending candidate; the
     * REVIVE.C:794-799 mirror-sensor disable loop is reached later by
     * F0282 on non-cancel confirm. */
    ok &= expect_int("pending candidate keeps front mirror route",
                     M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);

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
    ok &= expect_true("portrait strip available for panel overlap",
                      portraits && portraits->loaded && portraits->pixels &&
                      portraits->width >= 256 && portraits->height >= 87);
    if (!ok) return 0;

    overlap = match_panel_over_portrait(rrPanel, portraits, fb,
                                        PROBE_FB_W, PROBE_FB_H,
                                        PROBE_PANEL_X, PROBE_PANEL_Y,
                                        PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                                        expectedOrdinal,
                                        6, 1);
    /* Source order evidence: DUNVIEW.C:3913-3928 draws the D1C C026
     * champion portrait, then PANEL.C:1619-1635 / 1654-1656 draws C040
     * while G0299 is nonzero.  In the overlapping pixels where both assets
     * are opaque and differ, C040 must be the visible source color. */
    if (overlap.compared <= 0 ||
        overlap.panelWins * 100 < 90 * overlap.compared ||
        overlap.portraitWins * 100 > 5 * overlap.compared) {
        fprintf(stderr,
                "FAIL %s RR panel did not cover mirror portrait overlap panel=%d portrait=%d other=%d compared=%d\n",
                label, overlap.panelWins, overlap.portraitWins,
                overlap.other, overlap.compared);
        ok = 0;
    }

    printf("%s panel-open drawn=%d/%d asset=%dx%d championCount=%d overlapPanel=%d/%d\n",
           label, match.assetDrawn, match.assetOpaque,
           match.assetWidth, match.assetHeight,
           game->world.party.championCount,
           overlap.panelWins, overlap.compared);
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

static int check_reselect_after_cancel(M11_GameViewState* game,
                                       const M11_AssetSlot* rrPanel,
                                       int expectedOrdinal,
                                       const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    int ok = 1;

    /* ReDMCSB REVIVE.C:744-783 / F0282 cancel clears G0299 and removes the
     * appended candidate, then returns before the REVIVE.C:785-799 mirror
     * sensor-disable loop.  A second select in the same runtime view should
     * therefore run REVIVE.C:272-276 / F0280 again at the same party slot. */
    ok &= expect_int("pre-reselect panel off",
                     game->candidateMirrorPanelActive, 0);
    ok &= expect_int("pre-reselect champion count",
                     game->world.party.championCount, 0);
    ok &= expect_int("pre-reselect ordinal clear",
                     game->candidateMirrorOrdinal, -1);
    ok &= expect_int("pre-reselect party index clear",
                     game->candidateMirrorPartyIndex, -1);
    ok &= expect_int("pre-reselect front mirror still armed",
                     M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);

    if (M11_GameView_SelectFrontMirrorCandidate(game) != 1) {
        fprintf(stderr, "FAIL %s reselect returned 0\n", label);
        return 0;
    }
    ok &= expect_int("post-reselect panel on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("post-reselect inventory on",
                     game->inventoryPanelActive, 1);
    ok &= expect_int("post-reselect champion appended",
                     game->world.party.championCount, 1);
    ok &= expect_int("post-reselect ordinal recorded",
                     game->candidateMirrorOrdinal, expectedOrdinal);
    ok &= expect_int("post-reselect party index reset",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("post-reselect front mirror still armed",
                     M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (match.assetOpaque <= 0 || match.assetDrawn * 100 < 90 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel missing after cancel reselect drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s cancel-reselect drawn=%d/%d championCount=%d candidate=%d/%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount,
           game->candidateMirrorOrdinal, game->candidateMirrorPartyIndex);
    return ok;
}

static int check_pointer_reincarnate(M11_GameViewState* game,
                                     const M11_AssetSlot* rrPanel,
                                     const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    M11_GameInputResult inputResult;
    int ok = 1;

    ok &= expect_int("pre-reincarnate-click panel on",
                     game->candidateMirrorPanelActive, 1);
    /* ReDMCSB COMMAND.C:228-233 / 508-511 maps the panel's reincarnate
     * box to C161, then REVIVE.C:785-806 clears G0299 and disables the
     * mirror-square sensor before REVIVE.C:806-835 applies reincarnation. */
    inputResult = M11_GameView_HandlePointerButton(game,
                                                   PROBE_REINCARNATE_CLICK_X,
                                                   PROBE_REINCARNATE_CLICK_Y,
                                                   M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("reincarnate click redraw", (int)inputResult,
                     (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("post-reincarnate-click panel off",
                     game->candidateMirrorPanelActive, 0);
    ok &= expect_int("post-reincarnate-click inventory off",
                     game->inventoryPanelActive, 0);
    ok &= expect_int("post-reincarnate-click champion kept",
                     game->world.party.championCount, 1);
    ok &= expect_int("post-reincarnate-click ordinal cleared",
                     game->candidateMirrorOrdinal, -1);
    ok &= expect_int("post-reincarnate-click party index cleared",
                     game->candidateMirrorPartyIndex, -1);
    ok &= expect_int("post-reincarnate-click front mirror disabled",
                     M11_GameView_GetFrontMirrorOrdinal(game), -1);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (rrPanel && rrPanel->loaded && rrPanel->pixels && match.assetOpaque > 0 &&
        match.assetDrawn * 100 > 5 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel leaked after reincarnate click drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s reincarnate-click drawn=%d/%d championCount=%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount);
    return ok;
}

static int check_pointer_resurrect(M11_GameViewState* game,
                                   const M11_AssetSlot* rrPanel,
                                   const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    M11_GameInputResult inputResult;
    int ok = 1;

    ok &= expect_int("pre-resurrect-click panel on",
                     game->candidateMirrorPanelActive, 1);
    /* ReDMCSB COMMAND.C:228-233 / 508-511 maps the panel's resurrect
     * box to C160, COMMAND.C:1985-1991 dispatches it to REVIVE.C F0282,
     * and REVIVE.C:785-837 clears G0299 plus disables the mirror-square
     * sensor without taking the C161 reincarnate stat/rename branch. */
    inputResult = M11_GameView_HandlePointerButton(game,
                                                   PROBE_RESURRECT_CLICK_X,
                                                   PROBE_RESURRECT_CLICK_Y,
                                                   M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("resurrect click redraw", (int)inputResult,
                     (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("post-resurrect-click panel off",
                     game->candidateMirrorPanelActive, 0);
    ok &= expect_int("post-resurrect-click inventory off",
                     game->inventoryPanelActive, 0);
    ok &= expect_int("post-resurrect-click champion kept",
                     game->world.party.championCount, 1);
    ok &= expect_int("post-resurrect-click ordinal cleared",
                     game->candidateMirrorOrdinal, -1);
    ok &= expect_int("post-resurrect-click party index cleared",
                     game->candidateMirrorPartyIndex, -1);
    ok &= expect_int("post-resurrect-click front mirror disabled",
                     M11_GameView_GetFrontMirrorOrdinal(game), -1);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (rrPanel && rrPanel->loaded && rrPanel->pixels && match.assetOpaque > 0 &&
        match.assetDrawn * 100 > 5 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel leaked after resurrect click drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s resurrect-click drawn=%d/%d championCount=%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount);
    return ok;
}

static int check_pointer_cancel(M11_GameViewState* game,
                                const M11_AssetSlot* rrPanel,
                                const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    M11_GameInputResult inputResult;
    int ok = 1;

    ok &= expect_int("pre-cancel-click panel on",
                     game->candidateMirrorPanelActive, 1);
    /* ReDMCSB COMMAND.C:228-233 / 508-511 maps the cancel strip to C162,
     * COMMAND.C:1985-1991 dispatches it to REVIVE.C F0282, and
     * REVIVE.C:744-783 clears G0299 plus removes the appended candidate
     * without reaching the REVIVE.C:785-799 mirror-sensor disable path. */
    inputResult = M11_GameView_HandlePointerButton(game,
                                                   PROBE_CANCEL_CLICK_X,
                                                   PROBE_CANCEL_CLICK_Y,
                                                   M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("cancel click redraw", (int)inputResult,
                     (int)M11_GAME_INPUT_REDRAW);
    ok &= expect_int("post-cancel-click panel off",
                     game->candidateMirrorPanelActive, 0);
    ok &= expect_int("post-cancel-click inventory off",
                     game->inventoryPanelActive, 0);
    ok &= expect_int("post-cancel-click champion removed",
                     game->world.party.championCount, 0);
    ok &= expect_int("post-cancel-click ordinal cleared",
                     game->candidateMirrorOrdinal, -1);
    ok &= expect_int("post-cancel-click party index cleared",
                     game->candidateMirrorPartyIndex, -1);
    ok &= expect_int("post-cancel-click front mirror still visible",
                     M11_GameView_GetFrontMirrorOrdinal(game), 2);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (rrPanel && rrPanel->loaded && rrPanel->pixels && match.assetOpaque > 0 &&
        match.assetDrawn * 100 > 5 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel leaked after cancel click drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s cancel-click drawn=%d/%d championCount=%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount);
    return ok;
}

static int check_pointer_deadzone_ignored(M11_GameViewState* game,
                                          const M11_AssetSlot* rrPanel,
                                          int expectedOrdinal,
                                          const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    M11_GameInputResult inputResult;
    int ok = 1;

    ok &= expect_int("pre-deadzone-click panel on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("pre-deadzone-click ordinal",
                     game->candidateMirrorOrdinal, expectedOrdinal);
    ok &= expect_int("pre-deadzone-click party index",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("pre-deadzone-click champion appended",
                     game->world.party.championCount, 1);
    /* ReDMCSB COMMAND.C:1379-1431 / F0358 scans the panel input table
     * and returns C000 when no zone matches.  COMMAND.C:1985-1991 /
     * F0378 calls REVIVE.C F0282 only for a nonzero C160/C161/C162
     * command, so this between-button click must leave G0299/panel state
     * live and must not run the confirm or cancel cleanup paths. */
    inputResult = M11_GameView_HandlePointerButton(game,
                                                   PROBE_PANEL_DEADZONE_CLICK_X,
                                                   PROBE_PANEL_DEADZONE_CLICK_Y,
                                                   M11_DM1_MOUSE_MASK_LEFT);
    ok &= expect_int("deadzone click ignored", (int)inputResult,
                     (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("post-deadzone-click panel still on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("post-deadzone-click inventory still on",
                     game->inventoryPanelActive, 1);
    ok &= expect_int("post-deadzone-click champion still appended",
                     game->world.party.championCount, 1);
    ok &= expect_int("post-deadzone-click ordinal preserved",
                     game->candidateMirrorOrdinal, expectedOrdinal);
    ok &= expect_int("post-deadzone-click party index preserved",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("post-deadzone-click front mirror still visible",
                     M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (match.assetOpaque <= 0 || match.assetDrawn * 100 < 90 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel disappeared after ignored deadzone click drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s deadzone-click ignored drawn=%d/%d championCount=%d candidate=%d/%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount,
           game->candidateMirrorOrdinal, game->candidateMirrorPartyIndex);
    return ok;
}

static int check_rest_input_ignored(M11_GameViewState* game,
                                    const M11_AssetSlot* rrPanel,
                                    int expectedOrdinal,
                                    const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    M11_GameInputResult inputResult;
    int ok = 1;

    ok &= expect_int("pre-rest-input panel on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("pre-rest-input resting off", game->resting, 0);
    ok &= expect_int("pre-rest-input champion appended",
                     game->world.party.championCount, 1);
    /* ReDMCSB COMMAND.C:2336-2338 / CHANGE2_15_FIX gates C145 rest with
     * !G0299 while a candidate champion inventory is open.  Rest must not
     * start and must not run any confirm/cancel cleanup on the live panel. */
    inputResult = M11_GameView_HandleInput(game, M12_MENU_INPUT_REST_TOGGLE);
    ok &= expect_int("rest input ignored", (int)inputResult,
                     (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("post-rest-input panel still on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("post-rest-input inventory still on",
                     game->inventoryPanelActive, 1);
    ok &= expect_int("post-rest-input resting still off", game->resting, 0);
    ok &= expect_int("post-rest-input champion still appended",
                     game->world.party.championCount, 1);
    ok &= expect_int("post-rest-input ordinal preserved",
                     game->candidateMirrorOrdinal, expectedOrdinal);
    ok &= expect_int("post-rest-input party index preserved",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("post-rest-input front mirror still visible",
                     M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (match.assetOpaque <= 0 || match.assetDrawn * 100 < 90 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel disappeared after ignored rest input drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s rest-input ignored drawn=%d/%d championCount=%d resting=%d candidate=%d/%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount, game->resting,
           game->candidateMirrorOrdinal, game->candidateMirrorPartyIndex);
    return ok;
}

static int check_inventory_toggle_ignored(M11_GameViewState* game,
                                          const M11_AssetSlot* rrPanel,
                                          int expectedOrdinal,
                                          const char* label) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    PanelMatch match;
    M11_GameInputResult inputResult;
    int ok = 1;

    ok &= expect_int("pre-inventory-toggle panel on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("pre-inventory-toggle inventory on",
                     game->inventoryPanelActive, 1);
    ok &= expect_int("pre-inventory-toggle champion appended",
                     game->world.party.championCount, 1);
    /* ReDMCSB COMMAND.C:2159-2181 checks !G0299 before champion
     * status-box clicks or C007..C011 inventory toggles can run.  A
     * Firestaff inventory-toggle input while the C040 panel is live must
     * therefore be consumed by the candidate-panel guard without closing
     * inventory, changing the appended candidate, or disabling the mirror. */
    inputResult = M11_GameView_HandleInput(game, M12_MENU_INPUT_INVENTORY_TOGGLE);
    ok &= expect_int("inventory toggle ignored", (int)inputResult,
                     (int)M11_GAME_INPUT_IGNORED);
    ok &= expect_int("post-inventory-toggle panel still on",
                     game->candidateMirrorPanelActive, 1);
    ok &= expect_int("post-inventory-toggle inventory still on",
                     game->inventoryPanelActive, 1);
    ok &= expect_int("post-inventory-toggle champion still appended",
                     game->world.party.championCount, 1);
    ok &= expect_int("post-inventory-toggle ordinal preserved",
                     game->candidateMirrorOrdinal, expectedOrdinal);
    ok &= expect_int("post-inventory-toggle party index preserved",
                     game->candidateMirrorPartyIndex, 0);
    ok &= expect_int("post-inventory-toggle front mirror still visible",
                     M11_GameView_GetFrontMirrorOrdinal(game), expectedOrdinal);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    match = match_panel(rrPanel, fb, PROBE_FB_W, PROBE_FB_H,
                        PROBE_PANEL_X, PROBE_PANEL_Y, 6);
    if (match.assetOpaque <= 0 || match.assetDrawn * 100 < 90 * match.assetOpaque) {
        fprintf(stderr,
                "FAIL %s RR panel disappeared after ignored inventory toggle drawn=%d/%d\n",
                label, match.assetDrawn, match.assetOpaque);
        ok = 0;
    }
    printf("%s inventory-toggle ignored drawn=%d/%d championCount=%d candidate=%d/%d\n",
           label, match.assetDrawn, match.assetOpaque,
           game->world.party.championCount,
           game->candidateMirrorOrdinal, game->candidateMirrorPartyIndex);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    M12_StartupMenuState menu2;
    M11_GameViewState game2;
    M12_StartupMenuState menu3;
    M11_GameViewState game3;
    M12_StartupMenuState menu4;
    M11_GameViewState game4;
    M12_StartupMenuState menu5;
    M11_GameViewState game5;
    M12_StartupMenuState menu6;
    M11_GameViewState game6;
    M12_StartupMenuState menu7;
    M11_GameViewState game7;
    M12_StartupMenuState menu8;
    M11_GameViewState game8;
    const M11_AssetSlot* rrPanel;
    const M11_AssetSlot* portraits;
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
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
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
    /* Pose C2: actively selecting from a side/no-front route must not run
     * the F0280 candidate append path or open the panel. */
    ok &= pose_select_miss(&game, 1, 4, DIR_WEST,
                           "corridor_west_select_miss");

    /* Pose D: corridor (1,4) facing north, select front mirror candidate
     * (ordinal 2).  The RR panel must be drawn on top of the viewport
     * and a new champion must be appended to the party. */
    ok &= pose_panel_open(&game, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
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
    if (!pose_panel_open(&game2, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_cancel")) {
        ok = 0;
    } else {
        if (check_cancel(&game2, rrPanel, "corridor_north_cancel")) {
            ok &= check_reselect_after_cancel(&game2, rrPanel, 2,
                                              "corridor_north_cancel_reselect");
        } else {
            ok = 0;
        }
    }
    M11_GameView_Shutdown(&game2);

    /* Pose G: reopen once more and click the reincarnate half of the real
     * source panel box.  This covers the C161 pointer route separately
     * from the direct resurrect confirm and cancel calls above. */
    M12_StartupMenu_InitWithDataDir(&menu3, dataDir, NULL);
    M11_GameView_Init(&game3);
    if (!M11_GameView_OpenSelectedMenuEntry(&game3, &menu3)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for reincarnate click pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game3, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_reincarnate_click")) {
        ok = 0;
    } else {
        ok &= check_pointer_reincarnate(&game3, rrPanel,
                                        "corridor_north_reincarnate_click");
    }
    M11_GameView_Shutdown(&game3);

    /* Pose H: reopen once more and click the source resurrect box.  The
     * existing direct resurrect helper proves the confirm state transition;
     * this route proves COMMAND.C C160 pointer dispatch and that the click
     * is consumed by the candidate panel before generic viewport input. */
    M12_StartupMenu_InitWithDataDir(&menu4, dataDir, NULL);
    M11_GameView_Init(&game4);
    if (!M11_GameView_OpenSelectedMenuEntry(&game4, &menu4)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for resurrect click pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game4, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_resurrect_click")) {
        ok = 0;
    } else {
        ok &= check_pointer_resurrect(&game4, rrPanel,
                                      "corridor_north_resurrect_click");
    }
    M11_GameView_Shutdown(&game4);

    /* Pose I: reopen once more and click the source cancel strip.  The
     * direct cancel helper above proves the state transition; this route
     * proves COMMAND.C C162 pointer dispatch and preserves the mirror route
     * exactly like REVIVE.C's early cancel return. */
    M12_StartupMenu_InitWithDataDir(&menu5, dataDir, NULL);
    M11_GameView_Init(&game5);
    if (!M11_GameView_OpenSelectedMenuEntry(&game5, &menu5)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for cancel click pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game5, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_cancel_click")) {
        ok = 0;
    } else {
        ok &= check_pointer_cancel(&game5, rrPanel,
                                   "corridor_north_cancel_click");
    }
    M11_GameView_Shutdown(&game5);

    /* Pose J: reopen once more and click the source panel dead zone between
     * C160 and C161.  COMMAND.C F0358 should return C000 here, so F0378 must
     * not call REVIVE.C F0282 and the open candidate panel should remain
     * live with no confirm/cancel cleanup. */
    M12_StartupMenu_InitWithDataDir(&menu6, dataDir, NULL);
    M11_GameView_Init(&game6);
    if (!M11_GameView_OpenSelectedMenuEntry(&game6, &menu6)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for deadzone click pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game6, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_deadzone_click")) {
        ok = 0;
    } else {
        ok &= check_pointer_deadzone_ignored(&game6, rrPanel, 2,
                                             "corridor_north_deadzone_click");
    }
    M11_GameView_Shutdown(&game6);

    /* Pose K: reopen once more and try the source-rest command while the
     * candidate panel is live.  COMMAND.C CHANGE2_15_FIX blocks C145 while
     * G0299 is set, preventing rest from cloning or confirming the candidate. */
    M12_StartupMenu_InitWithDataDir(&menu7, dataDir, NULL);
    M11_GameView_Init(&game7);
    if (!M11_GameView_OpenSelectedMenuEntry(&game7, &menu7)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for rest input pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game7, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_rest_input")) {
        ok = 0;
    } else {
        ok &= check_rest_input_ignored(&game7, rrPanel, 2,
                                       "corridor_north_rest_input");
    }
    M11_GameView_Shutdown(&game7);

    /* Pose L: reopen once more and try the inventory toggle while C040 is
     * live.  COMMAND.C:2159-2181 gates champion status-box and inventory
     * toggles on !G0299, so the candidate panel remains the owner. */
    M12_StartupMenu_InitWithDataDir(&menu8, dataDir, NULL);
    M11_GameView_Init(&game8);
    if (!M11_GameView_OpenSelectedMenuEntry(&game8, &menu8)) {
        fprintf(stderr, "FAIL could not reopen DM1 V1 game view for inventory-toggle pose\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (!pose_panel_open(&game8, rrPanel, portraits, 1, 4, DIR_NORTH, 2,
                         "corridor_north_select_for_inventory_toggle")) {
        ok = 0;
    } else {
        ok &= check_inventory_toggle_ignored(&game8, rrPanel, 2,
                                             "corridor_north_inventory_toggle");
    }
    M11_GameView_Shutdown(&game8);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 champion mirror candidate panel runtime probe\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
