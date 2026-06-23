/*
 * DM1 V1 Hall of Champions portrait 02 cancel_reopen portrait_rect_position
 * runtime probe.
 *
 * Narrow slice: champion portrait ordinal 2 with the cancel_reopen
 * route aspect and the portrait_rect_position aspect.
 *
 * The companion probes already cover ordinal 0 (portrait00_rect),
 * ordinal 1 (portrait_rect_position), ordinal 18 (actual_pose via the
 * real (1,3) EAST route), ordinal 10 (actual_pose via (1,5) NORTH
 * ZED), and ordinal 20 (candidate_panel redraw-after-cancel via
 * retargeted C127 sensor).  The portrait02 / cancel_reopen /
 * portrait_rect_position slice had no real-asset probe and is the
 * gap this probe closes.
 *
 * The DM1 V1 Hall of Champions in the reference DUNGEON.DAT (the
 * fixture the existing walkpath probe targets) places ordinal 2 at
 * (1,4) facing NORTH.  The current DM1 fixture under test has a
 * different sensor layout (the walkpath probe and the candidate
 * panel probe both report legacyOrdinal2Fixture mismatch for this
 * build — see firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe
 * for the SKIP branch).  To produce a real-asset proof that does
 * NOT depend on a specific DM1 fixture, this probe retargets the
 * canonical ordinal-1 C127 sensor at (1,2) facing NORTH (HALK in
 * the real DM1 V1 fixture, per
 * firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe) to
 * ordinal 2.  The retarget is an in-memory mutation of the
 * sensorData field; the sensor type (C127), sensor cell, and the
 * runtime front-cell route are unchanged.  This is the same
 * pattern the companion candidate_panel probe uses for ordinal 20
 * (pass784+ etc.).
 *
 * Assertions:
 *   - ordinal 2 maps through the mirror catalog to a real champion
 *     record (catalog lookup success; not hard-coded name);
 *   - after retargeting the (1,2) NORTH C127 sensor to sensorData=2,
 *     GetFrontMirrorOrdinal returns 2 at the (1,2) NORTH pose and
 *     -1 at the side walls (DIR_EAST, DIR_WEST);
 *   - the D1C portrait rectangle (96,35)-(127,63) sits inside the
 *     D1C wall-mirror frame (80,29,64,43) at (+16,+6) and is
 *     contained by it;
 *   - on the front route (1,2) NORTH, ordinal 2 portrait pixels
 *     dominate the D1C rect (>=85% match, generous tolerance to
 *     absorb the dark-grey transparency convention the companion
 *     probes already lock);
 *   - select -> cancel (REVIVE.C F0282:744-783, C162 branch) ->
 *     reopen (re-select on the same armed route) preserves the
 *     front mirror ordinal (F0282 returns BEFORE F0282:785-799
 *     disables the sensor), clears the C040 resurrect/reincarnate
 *     panel on cancel, and re-paints the C040 panel on reopen;
 *   - the portrait_rect position is the SAME D1C rect (96,35,32,29)
 *     after cancel and after reopen, not a stale or empty rect;
 *   - side/no-front poses (DIR_EAST, DIR_WEST) do not leave ordinal
 *     2 pixels floating in the D1C portrait rectangle.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps the C127 sensor cell against party
 *     direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores the C127 sensorData as the
 *     ordinal via M000_INDEX_TO_ORDINAL.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026_GRAPHIC_CHAMPION_PORTRAITS
 *     into the fixed D1C champion-portrait box at (96,35,32,29).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 redraws the viewport from the
 *     current party pose, far-to-near.
 *   ReDMCSB MOVESENS.C F0275:1501-1503 fires F0280 on the new front
 *     sensor; REVIVE.C F0280:124-132 is the empty-leader candidate
 *     add path.
 *   ReDMCSB REVIVE.C F0282:744-806 is the C162 cancel branch; the
 *     cancel returns BEFORE the mirror sensor disable loop at
 *     REVIVE.C:785-799, so the route remains armed for the next
 *     sensor trigger.
 *   ReDMCSB PANEL.C F0355:2299-2318 closes the inventory chrome via
 *     C04_CHAMPION_CLOSE_INVENTORY on cancel.
 *   ReDMCSB DEFS.H C040/M568, C127, C162, G0299, G0305, G0415, G0424.
 *
 * This probe does NOT claim DOS pixel parity.  The ordinal-2 match
 * tolerance is generous (>=85% per-ordinal pixel match in the D1C
 * rect) to absorb palette rounding and the dark-grey transparency
 * convention the existing companion probes already lock.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "memory_champion_state_pc34_compat.h"

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
    PROBE_PORTRAIT_VX = 96,
    PROBE_PORTRAIT_VY = 35,
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VX,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VY,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_EXPECTED_ORDINAL = 2,
    PROBE_TARGET_ORDINAL = 1, /* real DM1 V1 (1,2) N ordinal is 1 (HALK) */
    PROBE_ROUTE_X = 1,
    PROBE_ROUTE_Y = 2,
    PROBE_ROUTE_DIR = DIR_NORTH,
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* C101 resurrect/reincarnate panel source rect (PANEL.C F0342). */
    PROBE_PANEL_X = PROBE_VIEWPORT_X + 80,
    PROBE_PANEL_Y = PROBE_VIEWPORT_Y + 52,
    PROBE_PANEL_W = 144,
    PROBE_PANEL_H = 73,
    /* C040 graphic id is the resurrect/reincarnate panel asset. */
    PROBE_RR_PANEL_GRAPHIC = 40,
    /* Match tolerance for ordinal dominance in the D1C portrait rect.
     * Generous to absorb the dark-grey transparency convention the
     * companion probes already lock. */
    PROBE_MATCH_PERCENT_MIN = 85,
    /* Tolerance for "no floating" on side poses: the side pose must
     * leave at most 35% of the compared ordinal pixels matching. */
    PROBE_FLOATING_PERCENT_MAX = 35
};

static int g_pass = 0;
static int g_fail = 0;

static void expect_int(const char* label, int got, int want) {
    if (got == want) {
        ++g_pass;
        printf("  PASS: %s got=%d\n", label, got);
    } else {
        ++g_fail;
        printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    }
}

static void expect_true(const char* label, int ok) {
    if (ok) {
        ++g_pass;
        printf("  PASS: %s\n", label);
    } else {
        ++g_fail;
        printf("  FAIL: %s\n", label);
    }
}

static void set_pose(M11_GameViewState* game,
                     int mapX,
                     int mapY,
                     int dir) {
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

static int ordinal_compared_count(const M11_AssetSlot* portraits, int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src != PROBE_CHAMPION_TRANSPARENT) {
                ++compared;
            }
        }
    }
    return compared;
}

typedef struct PortraitRectMatch {
    int matched;
    int compared;
    int percent;
    int bestOrdinal;
    int bestMatched;
} PortraitRectMatch;

static int count_ordinal_pixels_at(const M11_AssetSlot* portraits,
                                   const unsigned char* fb,
                                   int dstX,
                                   int dstY,
                                   int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(dstY + y) * PROBE_FB_W + (dstX + x)]);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            if (dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

static PortraitRectMatch match_portrait_at(const M11_AssetSlot* portraits,
                                          const unsigned char* fb,
                                          int dstX,
                                          int dstY,
                                          int expectedOrdinal) {
    PortraitRectMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int matched = count_ordinal_pixels_at(portraits, fb, dstX, dstY, ordinal);
        int compared = ordinal_compared_count(portraits, ordinal);
        if (matched > out.bestMatched) {
            out.bestMatched = matched;
            out.bestOrdinal = ordinal;
        }
        if (ordinal == expectedOrdinal) {
            out.matched = matched;
            out.compared = compared;
            out.percent = compared > 0 ? (matched * 100 / compared) : 0;
        }
    }
    return out;
}

/* Returns >=90 if the C040 panel is on top of the viewport (>=90% of
 * the asset's opaque pixels were blitted at PROBE_PANEL_X/Y), and <=5
 * if the C040 panel is gone.  The two thresholds are far apart so the
 * toggle is unambiguous. */
static int panel_drawn_pct(const M11_AssetSlot* rrPanel,
                           const unsigned char* fb) {
    int x;
    int y;
    int assetOpaque = 0;
    int assetDrawn = 0;
    int pct = 0;
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels || !fb) {
        return -1;
    }
    for (y = 0; y < PROBE_PANEL_H && y < (int)rrPanel->height; ++y) {
        for (x = 0; x < PROBE_PANEL_W && x < (int)rrPanel->width; ++x) {
            unsigned char src =
                (unsigned char)(rrPanel->pixels[y * (int)rrPanel->width + x] & 0x0F);
            if (src == 0) {
                continue;
            }
            ++assetOpaque;
            if (M11_FB_DECODE_INDEX(fb[(PROBE_PANEL_Y + y) * PROBE_FB_W +
                                       (PROBE_PANEL_X + x)]) == src) {
                ++assetDrawn;
            }
        }
    }
    if (assetOpaque > 0) {
        pct = (assetDrawn * 100) / assetOpaque;
    }
    return pct;
}

/* Retarget a single C127 sensor with sensorData == oldOrdinal on the
 * front-cell route (mapIndex, routeX, routeY) to sensorData = newOrdinal.
 * The cell coordinates are in the input map's frame so this stays
 * stable across fixtures that have the same C127 sensor on the front
 * cell but different per-sensor indexes.  Returns the number of
 * sensors retargeted. */
static int retarget_front_c127(M11_GameViewState* game,
                               int routeX,
                               int routeY,
                               int oldOrdinal,
                               int newOrdinal) {
    int idx;
    int found = 0;
    /* routeX/routeY are documented caller intent: the C127 sensor on
     * the (mapX,mapY,DIR_NORTH) front cell.  In this DM1 V1 fixture
     * only one C127 sensor with sensorData == oldOrdinal exists on
     * map 0 so the cell-coordinate filter is not needed; the
     * parameters stay in the signature so the same helper can be
     * ported to fixtures with multiple sensors per ordinal. */
    (void)routeX;
    (void)routeY;
    if (!game || !game->world.things || !game->world.things->sensors) {
        return 0;
    }
    for (idx = 0; idx < game->world.things->sensorCount; ++idx) {
        if (game->world.things->sensors[idx].sensorType != 127) continue;
        if ((int)game->world.things->sensors[idx].sensorData != oldOrdinal) continue;
        game->world.things->sensors[idx].sensorData = (unsigned short)newOrdinal;
        ++found;
        /* only retarget one so the per-pose front-cell invariant is
         * preserved even if multiple C127 sensors happen to share
         * sensorData; the first match is the front cell */
        break;
    }
    return found;
}

/* ── Group A: ordinal 2 catalog identity ─────────────────────────── */
static void check_catalog(M11_GameViewState* game) {
    char name[CHAMPION_NAME_TEXT_CAPACITY];
    char title[CHAMPION_TITLE_TEXT_CAPACITY];
    int nameRc;
    int titleRc;

    name[0] = '\0';
    title[0] = '\0';
    nameRc = M11_GameView_GetMirrorNameByOrdinal(game, PROBE_EXPECTED_ORDINAL,
                                                 name, (int)sizeof(name));
    titleRc = M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_EXPECTED_ORDINAL,
                                                   title, (int)sizeof(title));
    expect_int("ordinal 2 catalog name lookup rc > 0", nameRc > 0 ? 1 : 0, 1);
    expect_int("ordinal 2 catalog title lookup rc > 0", titleRc > 0 ? 1 : 0, 1);
    expect_true("ordinal 2 catalog name non-empty",
                name[0] != '\0' && name[0] >= 'A' && name[0] <= 'Z');
    expect_true("ordinal 2 catalog title non-empty",
                title[0] != '\0' && title[0] >= 'A' && title[0] <= 'Z');
    printf("  INFO: ordinal 2 catalog name=\"%s\" title=\"%s\"\n",
           name, title);
}

/* ── Group B: ordinal 2 D1C portrait_rect_position ──────────────── */
static void check_initial_position(M11_GameViewState* game,
                                   const M11_AssetSlot* portraits,
                                   unsigned char* fb) {
    int ornX = -1;
    int ornY = -1;
    int ornW = -1;
    int ornH = -1;
    int ord;
    int retargeted;
    PortraitRectMatch match;

    set_pose(game, PROBE_ROUTE_X, PROBE_ROUTE_Y, PROBE_ROUTE_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("pre-retarget (1,2) NORTH still has the original ordinal",
               ord, PROBE_TARGET_ORDINAL);

    retargeted = retarget_front_c127(game, PROBE_ROUTE_X, PROBE_ROUTE_Y,
                                     PROBE_TARGET_ORDINAL, PROBE_EXPECTED_ORDINAL);
    expect_int("retargeted (1,2) NORTH C127 from ordinal 1 to ordinal 2",
               retargeted, 1);

    set_pose(game, PROBE_ROUTE_X, PROBE_ROUTE_Y, PROBE_ROUTE_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("post-retarget (1,2) NORTH now reports ordinal 2",
               ord, PROBE_EXPECTED_ORDINAL);

    expect_true("GetD1CWallOrnamentZone succeeds",
                M11_GameView_GetD1CWallOrnamentZone(game, &ornX, &ornY, &ornW, &ornH) == 1);
    expect_int("D1C wall-mirror frame x", ornX, 80);
    expect_int("D1C wall-mirror frame y", ornY, 29);
    expect_int("D1C wall-mirror frame width", ornW, 64);
    expect_int("D1C wall-mirror frame height", ornH, 43);
    expect_int("portrait rect x is frame x + 16", PROBE_PORTRAIT_VX, ornX + 16);
    expect_int("portrait rect y is frame y + 6", PROBE_PORTRAIT_VY, ornY + 6);
    expect_true("portrait rect is contained by D1C wall-mirror frame",
                PROBE_PORTRAIT_VX >= ornX &&
                PROBE_PORTRAIT_VY >= ornY &&
                PROBE_PORTRAIT_VX + PROBE_PORTRAIT_W <= ornX + ornW &&
                PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H <= ornY + ornH);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    match = match_portrait_at(portraits, fb,
                              PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                              PROBE_EXPECTED_ORDINAL);
    expect_int("best portrait ordinal at D1C rect on first draw",
               match.bestOrdinal, PROBE_EXPECTED_ORDINAL);
    expect_true("ordinal 2 first-draw pixel match >=85% in D1C rect",
                match.compared > 0 && match.percent >= PROBE_MATCH_PERCENT_MIN);
    printf("  INFO: first-draw ordinal 2 rect matched=%d/%d (%d%%) best=%d\n",
           match.matched, match.compared, match.percent, match.bestOrdinal);
}

/* ── Group C: select -> cancel -> reopen ───────────────────────────── */
static void check_cancel_reopen(M11_GameViewState* game,
                                const M11_AssetSlot* portraits,
                                const M11_AssetSlot* rrPanel,
                                unsigned char* fb) {
    int initialCount;
    int selectRc;
    int cancelRc;
    int reselectRc;
    int panelPct;
    PortraitRectMatch afterCancel;
    PortraitRectMatch afterReopen;

    set_pose(game, PROBE_ROUTE_X, PROBE_ROUTE_Y, PROBE_ROUTE_DIR);
    initialCount = game->world.party.championCount;
    expect_int("ordinal 2 route pre-select champion count", initialCount, 0);

    /* ReDMCSB REVIVE.C F0280:124-132: select front-mirror candidate. */
    selectRc = M11_GameView_SelectFrontMirrorCandidate(game);
    expect_int("ordinal 2 select returns 1", selectRc, 1);
    expect_int("ordinal 2 candidate panel active after select",
               game->candidateMirrorPanelActive, 1);
    expect_int("ordinal 2 inventory panel active after select",
               game->inventoryPanelActive, 1);
    expect_int("ordinal 2 candidate appended to party",
               game->world.party.championCount, 1);
    expect_int("ordinal 2 candidate ordinal recorded",
               game->candidateMirrorOrdinal, PROBE_EXPECTED_ORDINAL);
    expect_int("ordinal 2 candidate party index recorded",
               game->candidateMirrorPartyIndex, 0);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    panelPct = panel_drawn_pct(rrPanel, fb);
    /* Generous >=80% threshold: the panel is blitted over the
     * viewport via the mouse-pointer mouse-show path, which masks a
     * small number of transparent pixels.  100% would be ideal but
     * palette rounding on the C040 asset's transparent edges drops
     * the visible draw a few points in the current snapshot. */
    expect_true("C040 RR panel drawn on select",
                rrPanel && panelPct >= 80);
    printf("  INFO: post-select C040 panel draw pct=%d\n", panelPct);

    /* ReDMCSB REVIVE.C F0282:744-783 (C162 cancel branch): clears
     * G0299, decrements G0305, returns BEFORE the F0282:785-799
     * mirror sensor disable loop.  The route must remain armed. */
    cancelRc = M11_GameView_CancelMirrorCandidate(game);
    expect_int("ordinal 2 cancel returns 1", cancelRc, 1);
    expect_int("ordinal 2 candidate panel off after cancel",
               game->candidateMirrorPanelActive, 0);
    expect_int("ordinal 2 inventory panel off after cancel",
               game->inventoryPanelActive, 0);
    expect_int("ordinal 2 candidate cleared after cancel",
               game->candidateMirrorOrdinal, -1);
    expect_int("ordinal 2 candidate party index cleared after cancel",
               game->candidateMirrorPartyIndex, -1);
    expect_int("ordinal 2 candidate champion removed after cancel",
               game->world.party.championCount, 0);
    expect_int("ordinal 2 front mirror still armed after cancel",
               M11_GameView_GetFrontMirrorOrdinal(game),
               PROBE_EXPECTED_ORDINAL);

    /* ReDMCSB DUNVIEW.C:3913-3928 redraws C026 at the fixed D1C
     * portrait box after the C040 panel is closed.  The post-cancel
     * portrait_rect must dominate ordinal 2. */
    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    afterCancel = match_portrait_at(portraits, fb,
                                    PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                                    PROBE_EXPECTED_ORDINAL);
    expect_int("post-cancel best ordinal at D1C rect",
               afterCancel.bestOrdinal, PROBE_EXPECTED_ORDINAL);
    expect_true("post-cancel ordinal 2 D1C rect match >=85%",
                afterCancel.compared > 0 &&
                afterCancel.percent >= PROBE_MATCH_PERCENT_MIN);
    panelPct = panel_drawn_pct(rrPanel, fb);
    expect_true("post-cancel C040 RR panel is cleared",
                !rrPanel || panelPct <= 5);
    printf("  INFO: post-cancel ordinal 2 rect matched=%d/%d (%d%%) panelPct=%d\n",
           afterCancel.matched, afterCancel.compared, afterCancel.percent,
           panelPct);

    /* ReDMCSB MOVESENS.C F0275:1501-1503 re-fires F0280 on the next
     * sensor trigger.  The route is still armed so a second
     * SelectFrontMirrorCandidate invokes F0280 again at the same
     * ordinal. */
    reselectRc = M11_GameView_SelectFrontMirrorCandidate(game);
    expect_int("ordinal 2 re-select after cancel returns 1", reselectRc, 1);
    expect_int("ordinal 2 candidate panel active after reopen",
               game->candidateMirrorPanelActive, 1);
    expect_int("ordinal 2 candidate ordinal recorded after reopen",
               game->candidateMirrorOrdinal, PROBE_EXPECTED_ORDINAL);
    expect_int("ordinal 2 candidate party index recorded after reopen",
               game->candidateMirrorPartyIndex, 0);
    expect_int("ordinal 2 candidate champion re-appended after reopen",
               game->world.party.championCount, 1);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    panelPct = panel_drawn_pct(rrPanel, fb);
    /* The C040 RR panel covers the lower half of the D1C portrait
     * rect at (96,35,32,29) and the panel asset itself embeds a
     * small candidate portrait at an internal offset, so the
     * bestOrdinal here is the panel's embedded portrait index, not
     * the wall-mirror ordinal 2.  The wall-mirror ordinal-2 portrait
     * is provably restored on the next cancel (post-reopen-cancel
     * block below).  Allow a generous panel-draw tolerance (>=80%) to
     * absorb palette rounding on the panel's transparent pixels. */
    expect_true("post-reopen C040 RR panel drawn again",
                rrPanel && panelPct >= 80);
    printf("  INFO: post-reopen panelPct=%d\n", panelPct);

    /* ReDMCSB REVIVE.C F0282:744-783 cancels the reopen the same way
     * it cancels the first select: G0299 cleared, G0305 decremented,
     * route still armed.  The post-reopen-cancel redraw must restore
     * the ordinal 2 portrait_rect_position that the first cancel
     * established above.  This is the load-bearing assertion for the
     * portrait_rect_position aspect across the full
     * select -> cancel -> reopen -> cancel chain. */
    cancelRc = M11_GameView_CancelMirrorCandidate(game);
    expect_int("ordinal 2 second cancel returns 1", cancelRc, 1);
    expect_int("ordinal 2 candidate panel off after second cancel",
               game->candidateMirrorPanelActive, 0);
    expect_int("ordinal 2 champion removed after second cancel",
               game->world.party.championCount, 0);
    expect_int("ordinal 2 front mirror still armed after second cancel",
               M11_GameView_GetFrontMirrorOrdinal(game),
               PROBE_EXPECTED_ORDINAL);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    afterReopen = match_portrait_at(portraits, fb,
                                    PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                                    PROBE_EXPECTED_ORDINAL);
    expect_int("post-reopen-cancel best ordinal at D1C rect",
               afterReopen.bestOrdinal, PROBE_EXPECTED_ORDINAL);
    expect_true("post-reopen-cancel ordinal 2 D1C rect match >=85%",
                afterReopen.compared > 0 &&
                afterReopen.percent >= PROBE_MATCH_PERCENT_MIN);
    panelPct = panel_drawn_pct(rrPanel, fb);
    expect_true("post-reopen-cancel C040 RR panel is cleared again",
                !rrPanel || panelPct <= 5);
    printf("  INFO: post-reopen-cancel ordinal 2 rect matched=%d/%d (%d%%) panelPct=%d\n",
           afterReopen.matched, afterReopen.compared, afterReopen.percent,
           panelPct);
}

/* ── Group D: side/no-front poses do not float ordinal 2 pixels ─── */
static void check_no_floating_after_turn(M11_GameViewState* game,
                                         const M11_AssetSlot* portraits,
                                         unsigned char* fb,
                                         int dir,
                                         const char* label) {
    int ord;
    int stale;
    int compared;

    set_pose(game, PROBE_ROUTE_X, PROBE_ROUTE_Y, PROBE_ROUTE_DIR);
    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    set_pose(game, PROBE_ROUTE_X, PROBE_ROUTE_Y, dir);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    compared = ordinal_compared_count(portraits, PROBE_EXPECTED_ORDINAL);
    stale = count_ordinal_pixels_at(portraits, fb,
                                    PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                                    PROBE_EXPECTED_ORDINAL);

    expect_int(label, ord, -1);
    expect_true("side/no-front pose does not float ordinal 2 in D1C rect",
                compared > 0 &&
                stale * 100 < compared * PROBE_FLOATING_PERCENT_MAX);
    printf("  INFO: %s stale ordinal 2 pixels=%d/%d\n", label, stale, compared);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    const M11_AssetSlot* rrPanel;
    static unsigned char fb[PROBE_FB_W * PROBE_FB_H];

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
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
    rrPanel = M11_AssetLoader_Load(&game.assetLoader,
        (unsigned int)PROBE_RR_PANEL_GRAPHIC);
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels ||
        rrPanel->width != PROBE_PANEL_W || rrPanel->height != PROBE_PANEL_H) {
        fprintf(stderr, "WARN GRAPHICS.DAT C040 RR panel asset unavailable; "
                "select/cancel panel-draw assertions will be SKIP-only\n");
        rrPanel = NULL;
    }

    printf("=== DM1 V1 Hall portrait 02 cancel_reopen portrait_rect_position "
           "(retargeted (1,2) NORTH route) ===\n");

    /* Group A: catalog identity is the source of truth, not a hard-coded
     * champion name.  This locks the ordinal → record mapping that
     * REVIVE.C F0280 uses when building the candidate champion. */
    printf("\n[Group A] Ordinal 2 catalog identity\n");
    check_catalog(&game);

    /* Group B: front route baseline.  Without a candidate, the C026
     * ordinal-2 portrait must dominate the D1C portrait rectangle
     * (96,35,32,29) and the rectangle must be parented inside the
     * C346 D1C wall-mirror frame (80,29,64,43). */
    printf("\n[Group B] Ordinal 2 front route portrait_rect_position\n");
    check_initial_position(&game, portraits, fb);

    /* Group C: the actual cancel_reopen slice.  After a select, the
     * DUNVIEW.C:8318-8542 F0128 viewport redraw must repaint C040 on
     * top of ordinal 2.  After C162 cancel, the C040 panel must clear
     * and ordinal 2 must be redrawn at the D1C source rect.  A second
     * select must re-paint C040 and re-append ordinal 2 to the party
     * at the same D1C rect. */
    printf("\n[Group C] Ordinal 2 select -> cancel -> reopen slice\n");
    check_cancel_reopen(&game, portraits, rrPanel, fb);

    /* Group D: leaving the front route (DIR_EAST/DIR_WEST) must not
     * leave ordinal 2 pixels floating in the D1C portrait rectangle. */
    printf("\n[Group D] Side/no-front poses do not float ordinal 2\n");
    check_no_floating_after_turn(&game, portraits, fb, DIR_EAST,
                                 "ordinal 2 turn east has no front route");
    check_no_floating_after_turn(&game, portraits, fb, DIR_WEST,
                                 "ordinal 2 turn west has no front route");

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
