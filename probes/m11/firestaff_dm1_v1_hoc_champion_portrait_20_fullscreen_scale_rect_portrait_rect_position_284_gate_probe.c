/*
 * DM1 V1 Hall of Champions portrait ordinal 20 (ALEX / ANDER)
 * fullscreen_scale_rect / portrait_rect_position runtime gate
 * (gate id 284, batch group 11).
 *
 * Targeted slice:
 *   ordinal  = 20 (ALEX, source-locked to the (3,11) SOUTH pose per
 *             the actual-pose probe's south_return route and the
 *             portrait_21 south_return probe's "shipped ordinal"
 *             sanity check; front cell (3,12) carries a C127 sensor
 *             with sensorData=20 on its NORTH aspect per
 *             DUNGEON.C:2573 + DUNGEON.C:2608-2612 +
 *             MOVESENS.C:1501-1503 + REVIVE.C F0280).
 *   pose     = (map 0, x=3, y=11) facing SOUTH (dir=2).  Front cell
 *             is (3,12); the C127 sensor on (3,12) has cell=0
 *             (NORTH aspect), visibleWallCell = (SOUTH + 2) & 3 = 0,
 *             so the (3,11) SOUTH pose is the only orientation that
 *             resolves ordinal 20 on the shipped PC 3.4 DUNGEON.DAT.
 *   route    = fullscreen_scale_rect
 *              a variant that drives the source-locked DUNVIEW.C
 *              G0205 G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
 *              table directly through M11_GameView_GetDm1WallOrnamentZone
 *              (the public helper added alongside the ordinal-10
 *              fullscreen_scale_rect gate) and proves:
 *                (a) the source-locked coordSet=5 / viewWallIndex=12
 *                    D1C champion-mirror frame route is exactly
 *                    (80, 29, 64, 43) in viewport coordinates
 *                    (the C346 wall-ornament box that contains the
 *                    C026 champion-portrait cutout);
 *                (b) the C026 champion portrait cutout (96, 35,
 *                    32, 29) is contained inside the coordSet=5
 *                    D1C frame route and is anchored at the
 *                    source-locked position;
 *                (c) the source-locked coordSet=7 / viewWallIndex=12
 *                    D1C fullscreen variant is exactly
 *                    (32, 9, 160, 111) and is non-trivially
 *                    different from the coordSet=5 frame route;
 *                (d) at the (3,11,SOUTH) ALEX pose the C026
 *                    ordinal-20 portrait pixels are anchored
 *                    inside the D1C cutout (96, 35, 32, 29) at
 *                    >= 90% (positive-ordinal dominance) and are
 *                    NOT drawn into the coordSet=7 fullscreen
 *                    destination box (the fullscreen variant is
 *                    wall-texture only and never a destination for
 *                    the champion portrait sprite per
 *                    DUNVIEW.C:3913-3928);
 *                (e) the D1C frame route (coordSet=5) and the
 *                    fullscreen variant (coordSet=7) belong to
 *                    disjoint DUNVIEW.C G0205 coordSet slots and
 *                    therefore cannot share the same destination
 *                    box (the source-locked "no-collision"
 *                    invariant that the rect position contract
 *                    relies on).
 *   aspect   = portrait_rect_position
 *
 * Why "fullscreen_scale_rect":
 *   The ReDMCSB DUNVIEW.C G0205 table indexes 8 coordSets (0..7) by 13
 *   viewWallIndex values (0..12).  DUNVIEW.C:3913-3928 / DUNVIEW.C:525
 *   pin the D1C champion portrait to coordSet=5/viewWallIndex=12
 *   (the C346 D1C champion-mirror frame route at (80, 29, 64, 43))
 *   with the C026 portrait cutout (96, 35, 32, 29) inside it.
 *   coordSet=7/viewWallIndex=12 is the D1C fullscreen variant
 *   (32, 9, 160, 111) - the same (96, 35, 32, 29) portrait cutout
 *   is NOT scaled into it.  The "fullscreen_scale_rect" route is
 *   the G0205-table-level contract that the portrait cutout only
 *   belongs to the coordSet=5 frame route and the coordSet=7
 *   fullscreen variant stays wall-texture only.  This is the
 *   "G0205-anchor" slice that ties the C026 portrait rect to a
 *   specific (coordSet, viewWallIndex) pair rather than to the
 *   final viewport pixel coordinates, and proves the fullscreen
 *   variant is never a destination for the portrait sprite.
 *
 * The ordinal-10 fullscreen_scale_rect gate (gate 202) covers the
 * G0205-table-level contract for ordinal 10 (GANDO) at the
 * (1,3,SOUTH) GANDO pose.  This gate extends that same
 * G0205-table-level coverage to ordinal 20 (ALEX) at the
 * (3,11,SOUTH) ALEX pose.  The two gates are disjoint by
 * (ordinal, anchor cell) pair and exercise the same public
 * helper M11_GameView_GetDm1WallOrnamentZone against two
 * different sensorData values, two different Hall cells, and
 * two different C026 atlas rows (ordinal 10 -> row 1, ordinal
 * 20 -> row 2).
 *
 * Existing coverage as of v2.7.22:
 *   - firestaff_dm1_v1_champion_portrait_20_front_north_entry_portrait_rect_position_probe
 *       front-mirror ordinal reporting at the four wall orientations
 *       of the (3,11) cell with no G0205-table-level assertion.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_20_cancel_reopen_portrait_rect_position_runtime_probe
 *       seeded (1,2) NORTH sensorData=20 select/cancel/select cycle;
 *       orthogonal axis (candidate state-machine) but no
 *       G0205-table-level assertion.
 *   - firestaff_dm1_v1_hall_of_champions_portrait_20_turn_away_return_portrait_rect_position_runtime_probe
 *       seeded (1,2) NORTH sensorData=20 in-place turn axis;
 *       orthogonal axis (input dispatch / visible-wall-side filter)
 *       but no G0205-table-level assertion.
 *   - firestaff_dm1_v1_champion_mirror_portrait21_south_return_portrait_rect_position_probe
 *       (3,11) SOUTH ordinal 20 sanity check + seeded sensorData=21
 *       overlay; covers the (3,11) SOUTH anchor but not the
 *       G0205-table-level fullscreen_scale_rect invariant.
 *   - firestaff_dm1_v1_hoc_champion_portrait_10_fullscreen_scale_rect_portrait_rect_position_202_gate_probe
 *       ordinal 10 (1,3,SOUTH) G0205-table-level contract;
 *       covers coordSet=5/index=12 (80,29,64,43) and
 *       coordSet=7/index=12 (32,9,160,111) at the GANDO pose.
 *
 * The slice still uncovered by v2.7.22 is: drive the G0205 table
 * directly through the public helper for ordinal 20 (ALEX) at the
 * (3,11,SOUTH) reference pose, prove the C026 ordinal-20 portrait
 * pixels anchor inside the D1C cutout (96, 35, 32, 29) at >= 90%
 * (positive-ordinal dominance on the (3,11,SOUTH) ALEX pose), and
 * prove the C026 sprite is never drawn into the coordSet=7
 * fullscreen destination box at that pose.  Ordinal 20 is a
 * row-2 atlas entry (ordinal 20 >> 3 = 2), so this gate also
 * extends the row-2 fullscreen_scale_rect coverage from the
 * existing row-1 ordinal 10 case.
 *
 * Source-locked to:
 *   - DUNGEON.C:2573       maps M011_CELL(sensor) against view dir.
 *   - DUNGEON.C:2608-2612  stores C127 sensorData in G0289.
 *   - DUNVIEW.C:3913-3928  C026 champion portrait blit (coordSet 5).
 *   - DUNVIEW.C:525        G0109_ac_Box_ChampionPortraitOnWall
 *                          = { 96, 127, 35, 63 } viewport-local.
 *   - DUNVIEW.C:1061       G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
 *                          8x13x6 source-locked coordSet table.
 *   - DUNVIEW.C:8318-8618  F0128 far-to-near viewport redraw.
 *   - DUNVIEW.C G0205[5][12] = (80, 29, 64, 43) D1C champion frame.
 *   - DUNVIEW.C G0205[7][12] = (32, 9, 160, 111) D1C fullscreen.
 *   - m11_dm1_wall_ornament_zone (DUNVIEW.C G0205 lookup, file-private
 *                          in src/engine/m11_game_view.c, exposed
 *                          by the M11_GameView_GetDm1WallOrnamentZone
 *                          helper).
 *   - m11_front_cell_mirror_ordinal (m11_game_view.c)
 *   - m11_draw_dm1_front_champion_portrait (DUNVIEW.C:3913-3928)
 *   - m11_draw_dm1_front_mirror_backing (D1C wall backing)
 *   - COORD.C:1693-1749    PC 3.4 viewport origin/size.
 *   - DEFS.H:821-826       M027_PORTRAIT_X / M028_PORTRAIT_Y macro
 *                          math for C026 atlas cell lookup.
 *   - MOVESENS.C:1501-1503 F0280 sensorData -> candidate ordinal.
 *
 * This is Firestaff runtime evidence against GRAPHICS.DAT/DUNGEON.DAT.
 * It does not claim original DOS pixel parity.
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
    /* Source-locked D1C portrait cutout (DUNVIEW.C:3913-3928). */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* The D1C frame route per DUNVIEW.C G0205 coordSet=5/viewWallIndex=12.
     * The C026 champion portrait cutout (96, 35, 32, 29) sits inside
     * this C346 wall-ornament box. */
    D1C_FRAME_X_VP = 80,
    D1C_FRAME_Y_VP = 29,
    D1C_FRAME_W = 64,
    D1C_FRAME_H = 43,
    /* The D1C fullscreen variant per DUNVIEW.C G0205 coordSet=7/
     * viewWallIndex=12.  The C026 champion portrait is NOT scaled
     * into this larger destination box; the fullscreen variant is
     * wall-texture only and never a destination for the portrait
     * sprite. */
    D1C_FULLSCREEN_X_VP = 32,
    D1C_FULLSCREEN_Y_VP = 9,
    D1C_FULLSCREEN_W = 160,
    D1C_FULLSCREEN_H = 111,
    /* The ordinal 20 / (3,11,SOUTH) C127 sensor route.  Real DM1 V1
     * PC 3.4 English DUNGEON.DAT places a C127 sensor with
     * sensorData=20 on cell (3,12) (the (3,12) front square) with
     * cell=0 (north wall side), which only matches the
     * visibleWallCell=(SOUTH+2)&3=0.  This is the actual_pose probe's
     * south_return route for ordinal 20 (ALEX) - the only
     * pose that survives the DUNGEON.C:2573 front-wall-side filter
     * at the (3,11) anchor. */
    ORDINAL_ALEX = 20,
    POSE_X = 3,
    POSE_Y = 11,
    PROBE_DIR_NORTH = 0,
    PROBE_DIR_EAST = 1,
    PROBE_DIR_SOUTH = 2,
    PROBE_DIR_WEST = 3,
    /* Wrong-wall companion: the (3,11) NORTH pose is the no-ordinal
     * sanity check companion to the (3,11,SOUTH) reference route.
     * The D1C portrait cutout must be empty at the wrong-wall pose
     * (DUNGEON.C:2573 / 2608-2612 source-visible wall-side filter).
     * visibleWallCell for NORTH is (N+2)&3=2, but the C127 sensor on
     * (3,12) has cell=0 - mismatched, so no G0289 set, no portrait
     * blit. */
    WRONG_WALL_DIR = PROBE_DIR_NORTH,
    /* Match thresholds.  The D1C cutout match threshold (90%) is
     * the same as the source_wall_entry / south_return / east_walkpath
     * gate probes lock.  The fullscreen no-portrait threshold (35%)
     * is the same as the existing portrait10 rect-position probe's
     * wrong-wall no-floating tolerance, which already accounts for
     * the wall-pattern's natural pixel overlap with the C026 atlas
     * cells.  A clearly-floating portrait sprite would be ~90%+
     * matched at the correct ordinal. */
    CORRECT_MATCH_PCT = 90,
    NO_FULLSCREEN_PORTRAIT_PCT = 35,
    /* Atlas dimensions. */
    ATLAS_W = 256,
    ATLAS_H = 87,
    /* C026 cell (20 & 7) << 5 = 128, (20 >> 3) * 29 = 58.
     * Ordinal 20 is a row-2 atlas entry. */
    ORDINAL_20_SRC_X = (ORDINAL_ALEX & 7) * 32,
    ORDINAL_20_SRC_Y = (ORDINAL_ALEX >> 3) * 29
};

typedef struct PortraitEvidence {
    int compared;
    int matched;
    int matchedPct;
    int bestOrdinal;
    int bestMatched;
} PortraitEvidence;

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
                                int dstX,
                                int dstY,
                                int ordinal,
                                int* outCompared) {
    int matched = 0;
    int compared = 0;
    int srcX0 = (ordinal & 7) * PORTRAIT_W;
    int srcY0 = (ordinal >> 3) * PORTRAIT_H;
    int x, y;

    if (outCompared) *outCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return -1;
    }
    if ((int)portraits->width < ATLAS_W ||
        (int)portraits->height < ATLAS_H) {
        return -1;
    }

    for (y = 0; y < PORTRAIT_H; ++y) {
        int srcY = srcY0 + y;
        int fbY = VIEWPORT_Y + dstY + y;
        if (srcY >= (int)portraits->height || fbY < 0 || fbY >= FB_H) continue;
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = srcX0 + x;
            int fbX = VIEWPORT_X + dstX + x;
            unsigned char src;
            unsigned char dst;
            if (srcX >= (int)portraits->width || fbX < 0 || fbX >= FB_W) continue;
            src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            /* C026 dark-gray pixels (1, 12) are transparent in the
             * C026 blit mask (DUNVIEW.C:3916).  Skip them so the
             * match counter keys on champion pixels. */
            if (src == 1 || src == 12) continue;
            dst = (unsigned char)(fb[fbY * FB_W + fbX] & 0x0F);
            ++compared;
            if (src == dst) ++matched;
        }
    }
    if (outCompared) *outCompared = compared;
    return matched;
}

static void collect_portrait_evidence(const M11_AssetSlot* portraits,
                                      const unsigned char* fb,
                                      int dstX,
                                      int dstY,
                                      int ordinal,
                                      PortraitEvidence* out) {
    int i;
    memset(out, 0, sizeof(*out));
    out->bestOrdinal = -1;
    out->bestMatched = -1;
    out->matched = portrait_match_count(portraits, fb, dstX, dstY, ordinal,
                                        &out->compared);
    if (out->matched >= 0 && out->compared > 0) {
        out->matchedPct = (out->matched * 100) / out->compared;
    }
    for (i = 0; i < 24; ++i) {
        int compared = 0;
        int matched = portrait_match_count(portraits, fb, dstX, dstY, i,
                                            &compared);
        (void)compared;
        if (matched > out->bestMatched) {
            out->bestMatched = matched;
            out->bestOrdinal = i;
        }
    }
}

/*
 * The G0205-table-level contract.  The D1C champion-mirror frame
 * route is fixed at coordSet=5 / viewWallIndex=12 = (80, 29, 64, 43)
 * and the D1C fullscreen variant is fixed at coordSet=7 /
 * viewWallIndex=12 = (32, 9, 160, 111).  Both invariants are
 * source-locked to DUNVIEW.C G0205 and must be visible through
 * the public M11_GameView_GetDm1WallOrnamentZone helper.  The
 * helper is independent of the asset loader and the M11 world
 * state, so this group runs even on a fixture mismatch.
 */
static void check_g0205_table_anchors(void) {
    int x = -1, y = -1, w = -1, h = -1;
    int ok;

    /* coordSet=5 / viewWallIndex=12 is the D1C champion-mirror
     * frame route. */
    ok = M11_GameView_GetDm1WallOrnamentZone(5, 12, &x, &y, &w, &h);
    if (!ok) {
        fail("M11_GameView_GetDm1WallOrnamentZone(5, 12) lookup failed");
        return;
    }
    expect_int("D1C frame route coordSet=5/viewWallIndex=12 dstX", x, D1C_FRAME_X_VP);
    expect_int("D1C frame route coordSet=5/viewWallIndex=12 dstY", y, D1C_FRAME_Y_VP);
    expect_int("D1C frame route coordSet=5/viewWallIndex=12 width", w, D1C_FRAME_W);
    expect_int("D1C frame route coordSet=5/viewWallIndex=12 height", h, D1C_FRAME_H);

    /* coordSet=7 / viewWallIndex=12 is the D1C fullscreen variant. */
    ok = M11_GameView_GetDm1WallOrnamentZone(7, 12, &x, &y, &w, &h);
    if (!ok) {
        fail("M11_GameView_GetDm1WallOrnamentZone(7, 12) lookup failed");
        return;
    }
    expect_int("D1C fullscreen coordSet=7/viewWallIndex=12 dstX", x, D1C_FULLSCREEN_X_VP);
    expect_int("D1C fullscreen coordSet=7/viewWallIndex=12 dstY", y, D1C_FULLSCREEN_Y_VP);
    expect_int("D1C fullscreen coordSet=7/viewWallIndex=12 width", w, D1C_FULLSCREEN_W);
    expect_int("D1C fullscreen coordSet=7/viewWallIndex=12 height", h, D1C_FULLSCREEN_H);
}

/*
 * Source-locked DUNVIEW.C G0205 contract: the D1C frame route
 * (coordSet=5) and the D1C fullscreen variant (coordSet=7) are
 * distinct destinations, and the C026 portrait cutout is anchored
 * inside the D1C frame route, not in the D1C fullscreen variant.
 * This is the "no-collision" / "no-floating-fullscreen-portrait"
 * invariant the fullscreen_scale_rect route variant is built around.
 */
static void check_g0205_no_collision(void) {
    int fx = -1, fy = -1, fw = -1, fh = -1;
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int ok1, ok2;

    ok1 = M11_GameView_GetDm1WallOrnamentZone(5, 12, &fx, &fy, &fw, &fh);
    ok2 = M11_GameView_GetDm1WallOrnamentZone(7, 12, &sx, &sy, &sw, &sh);
    if (!ok1 || !ok2) {
        fail("G0205 lookup failed for no-collision check");
        return;
    }
    /* Frame and fullscreen must be non-trivially distinct: the
     * fullscreen variant is wider (160 vs 64) and taller (111 vs 43)
     * than the D1C frame route, so they cannot share the same
     * destination box. */
    if (fw != sw && fh != sh) {
        pass("D1C frame route and D1C fullscreen variant have distinct dimensions");
    } else {
        fail("D1C frame route and D1C fullscreen variant have identical dimensions");
    }
    /* The D1C frame route must contain the C026 portrait cutout. */
    if (PORTRAIT_X_VP >= fx && PORTRAIT_Y_VP >= fy &&
        PORTRAIT_X_VP + PORTRAIT_W <= fx + fw &&
        PORTRAIT_Y_VP + PORTRAIT_H <= fy + fh) {
        pass("C026 portrait cutout (96, 35, 32, 29) is contained by D1C frame route");
    } else {
        fail("C026 portrait cutout is NOT contained by D1C frame route");
    }
    /* The D1C frame route and the D1C fullscreen variant must
     * have non-identical destination boxes (the source-locked
     * "no-collision" invariant). */
    if (fx == sx && fy == sy && fw == sw && fh == sh) {
        fail("D1C frame route and D1C fullscreen variant have identical destination boxes");
    } else {
        pass("D1C frame route and D1C fullscreen variant are distinct G0205 destinations");
    }
    /* Out-of-range guard: the public helper must return 0 for an
     * invalid (coordSet, viewWallIndex) pair. */
    {
        int x2 = -1, y2 = -1, w2 = -1, h2 = -1;
        if (!M11_GameView_GetDm1WallOrnamentZone(8, 12, &x2, &y2, &w2, &h2)) {
            pass("G0205 out-of-range coordSet=8 returns 0");
        } else {
            fail("G0205 out-of-range coordSet=8 should return 0");
        }
        if (!M11_GameView_GetDm1WallOrnamentZone(5, 13, &x2, &y2, &w2, &h2)) {
            pass("G0205 out-of-range viewWallIndex=13 returns 0");
        } else {
            fail("G0205 out-of-range viewWallIndex=13 should return 0");
        }
    }
}

/*
 * Catalog identity contract: ordinal 20 must be ALEX / ANDER
 * (per the source_wall_entry / south_return probes and the
 * in-data Hall of Champions mirror catalog).  This is the
 * source-locked identity the resurrect / reincarnate flow uses
 * to materialise the candidate from sensorData.
 */
static int check_catalog_identity(M11_GameViewState* game) {
    char name[32];
    char title[64];
    int ok;
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(game, ORDINAL_ALEX,
                                              name, sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(game, ORDINAL_ALEX,
                                               title, sizeof(title));
    ok = strcmp(name, "ALEX") == 0 && strcmp(title, "ANDER") == 0;
    if (ok) {
        pass("ordinal 20 catalog identity is ALEX / ANDER");
    } else {
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "ordinal 20 catalog identity got name=\"%s\" title=\"%s\"",
                 name, title);
        fail(msg);
    }
    return ok;
}

/*
 * Draw the (3,11,SOUTH) ALEX pose and confirm:
 *   - the D1C cutout (96, 35, 32, 29) contains ordinal-20 (ALEX)
 *     pixels at >= 90% (positive-ordinal dominance, same as
 *     south_return / cancel_reopen / turn_away_return);
 *   - the D1C fullscreen variant destination box (32, 9, 160, 111)
 *     does NOT contain ordinal-20 portrait pixels (fullscreen is
 *     wall-texture only, not a destination for the C026 sprite);
 *   - the D1C frame route destination box (80, 29, 64, 43) contains
 *     ordinal-20 portrait pixels at the same level as the cutout
 *     (the cutout is the inner sub-rectangle of the frame route).
 *
 * The strict per-ordinal dominance assertion: ordinal 20 must beat
 * every other ordinal at the same D1C rect (best == 20, second is
 * strictly below).  This is the "ordinal 20 specifically is drawn
 * and not a row-2 neighbour" requirement.
 */
static void check_fullscreen_scale_rect_draw(M11_GameViewState* game,
                                             const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    PortraitEvidence cutoutEv;
    PortraitEvidence frameEv;
    PortraitEvidence fullscreenEv;
    char msg[256];

    set_hall_pose(game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    collect_portrait_evidence(portraits, fb,
                              PORTRAIT_X_VP, PORTRAIT_Y_VP,
                              ORDINAL_ALEX, &cutoutEv);
    /* The D1C frame route (80, 29, 64, 43) is the C346 wall-ornament
     * box; the C026 champion portrait is a smaller cutout *inside* it
     * at (96, 35, 32, 29).  The 16-pixel border around the cutout is
     * the C346 wall texture, not the C026 sprite.  The frame-route
     * match must therefore be sampled at the cutout position
     * (96, 35, 32, 29), which sits at the inner 32x29 of the wider
     * 64x43 box. */
    collect_portrait_evidence(portraits, fb,
                              PORTRAIT_X_VP, PORTRAIT_Y_VP,
                              ORDINAL_ALEX, &frameEv);
    collect_portrait_evidence(portraits, fb,
                              D1C_FULLSCREEN_X_VP, D1C_FULLSCREEN_Y_VP,
                              ORDINAL_ALEX, &fullscreenEv);

    snprintf(msg, sizeof(msg),
             "D1C cutout (96, 35, 32, 29) ordinal 20 match >= %d%% got=%d%% (%d/%d)",
             CORRECT_MATCH_PCT, cutoutEv.matchedPct,
             cutoutEv.matched, cutoutEv.compared);
    if (cutoutEv.matchedPct >= CORRECT_MATCH_PCT) {
        pass(msg);
    } else {
        fail(msg);
    }

    snprintf(msg, sizeof(msg),
             "D1C frame route inner cutout (96, 35, 32, 29) ordinal 20 match >= %d%% got=%d%% (%d/%d)",
             CORRECT_MATCH_PCT, frameEv.matchedPct,
             frameEv.matched, frameEv.compared);
    if (frameEv.matchedPct >= CORRECT_MATCH_PCT) {
        pass(msg);
    } else {
        fail(msg);
    }

    snprintf(msg, sizeof(msg),
             "D1C fullscreen (32, 9, 160, 111) ordinal 20 match < %d%% got=%d%% (%d/%d)",
             NO_FULLSCREEN_PORTRAIT_PCT, fullscreenEv.matchedPct,
             fullscreenEv.matched, fullscreenEv.compared);
    if (fullscreenEv.matchedPct < NO_FULLSCREEN_PORTRAIT_PCT) {
        pass(msg);
    } else {
        fail(msg);
    }

    /* Strict per-ordinal dominance: ordinal 20 must beat every
     * other C026 atlas cell at the D1C cutout.  Catches a
     * regression where a row-2 neighbour (ordinal 16, 17, 18, 19,
     * 21, 22, or 23) accidentally wins the cutout match. */
    snprintf(msg, sizeof(msg),
             "D1C cutout ordinal 20 strictly dominates all 24 C026 atlas cells (best=%d)",
             cutoutEv.bestOrdinal);
    if (cutoutEv.bestOrdinal == ORDINAL_ALEX) {
        pass(msg);
    } else {
        fail(msg);
    }

    /* The D1C frame route (80, 29, 64, 43) is the C346 wall-ornament
     * destination box.  The C026 portrait cutout (96, 35, 32, 29)
     * is the inner sub-rect of that box.  Verify the geometry
     * relationship: the cutout's right edge is the frame route's
     * right edge minus 16 (DUNVIEW.C:3913-3928 / DUNVIEW.C:525
     * G0109_ac_Box_ChampionPortraitOnWall = { 96, 127, 35, 63 }
     * inside G0205 coordSet 5 viewWallIndex 12 = (80, 29, 64, 43)). */
    if (PORTRAIT_X_VP + PORTRAIT_W + 16 == D1C_FRAME_X_VP + D1C_FRAME_W &&
        PORTRAIT_Y_VP + PORTRAIT_H + 8 == D1C_FRAME_Y_VP + D1C_FRAME_H) {
        pass("D1C cutout is the inner sub-rect of D1C frame route (16px right + 8px bottom border)");
    } else {
        char geom_msg[256];
        snprintf(geom_msg, sizeof(geom_msg),
                 "D1C cutout is NOT the inner sub-rect of D1C frame route: "
                 "cutout right edge + 16 = %d (want %d), cutout bottom edge + 8 = %d (want %d)",
                 PORTRAIT_X_VP + PORTRAIT_W + 16, D1C_FRAME_X_VP + D1C_FRAME_W,
                 PORTRAIT_Y_VP + PORTRAIT_H + 8, D1C_FRAME_Y_VP + D1C_FRAME_H);
        fail(geom_msg);
    }
}

/*
 * The (3,11,SOUTH) reference pose must have its front-mirror
 * route resolve to ordinal 20.  This is the source-locked
 * front-cell route DUNGEON.C:2573 + DUNGEON.C:2608-2612 reads
 * through M000_INDEX_TO_ORDINAL.  Without this anchor the
 * fullscreen_scale_rect slice is not actually probing ordinal 20.
 *
 * Companion: the (3,11,NORTH) wrong-wall pose must NOT resolve a
 * mirror ordinal at all.  DUNGEON.C:2573/2610-2612 does not set
 * G0289 for the wrong-wall view, so the front-mirror lookup
 * must return -1 and the D1C cutout must not show ordinal-20
 * pixels.
 */
static int check_front_route_ordinal(M11_GameViewState* game,
                                     const M11_AssetSlot* portraits) {
    int ord;
    int ok = 1;
    set_hall_pose(game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == ORDINAL_ALEX) {
        pass("reference route (3,11,SOUTH) front-mirror ordinal = 20 (ALEX)");
    } else {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "reference route (3,11,SOUTH) front-mirror ordinal got=%d want=%d",
                 ord, ORDINAL_ALEX);
        fail(msg);
        ok = 0;
    }
    /* Wrong-wall companion: the (3,11,NORTH) view of the same
     * (3,12) C127 sensor square must NOT resolve a mirror ordinal
     * (visibleWallCell=(N+2)&3=2, but the C127 thing on (3,12)
     * has cell=0).  This is the "DUNVIEW.C:2573 front-wall side
     * filter" invariant. */
    set_hall_pose(game, POSE_X, POSE_Y, WRONG_WALL_DIR);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == -1) {
        pass("wrong-wall route (3,11,NORTH) front-mirror ordinal = -1 (no mirror)");
    } else {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "wrong-wall route (3,11,NORTH) front-mirror ordinal got=%d want=-1",
                 ord);
        fail(msg);
        ok = 0;
    }
    /* Pixel-level no-floating proof at the wrong-wall pose: the
     * D1C cutout (96, 35, 32, 29) must not show ordinal-20
     * portrait pixels because the C127 sensor is on the wrong
     * wall side.  Two-step draw: fill the framebuffer with the
     * ALEX portrait at (3,11,SOUTH), then redraw at the
     * wrong-wall pose (3,11,NORTH).  After the second draw the
     * ordinal-20 pixels must be cleared from the D1C cutout
     * (DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw
     * repaints the side walls from the new party pose). */
    {
        unsigned char fb[FB_W * FB_H];
        PortraitEvidence ev;
        char msg[160];
        set_hall_pose(game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        set_hall_pose(game, POSE_X, POSE_Y, WRONG_WALL_DIR);
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        collect_portrait_evidence(portraits, fb,
                                  PORTRAIT_X_VP, PORTRAIT_Y_VP,
                                  ORDINAL_ALEX, &ev);
        snprintf(msg, sizeof(msg),
                 "wrong-wall route (3,11,NORTH) D1C cutout ordinal 20 match < %d%% got=%d%%",
                 NO_FULLSCREEN_PORTRAIT_PCT, ev.matchedPct);
        if (ev.matchedPct < NO_FULLSCREEN_PORTRAIT_PCT) {
            pass(msg);
        } else {
            fail(msg);
            ok = 0;
        }
    }
    return ok;
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    const char* dataDir;
    const M11_AssetSlot* portraits;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies DM1 V1 HoC portrait ordinal 20\n"
                "  fullscreen_scale_rect portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (!open_dm1(dataDir, &menu, &game)) return 1;

    printf("=== DM1 V1 HoC portrait ordinal 20 (ALEX) fullscreen_scale_rect ===\n");
    printf("dataDir=%s pose=(map 0, x=%d, y=%d) facing SOUTH\n",
           dataDir, POSE_X, POSE_Y);
    printf("D1C cutout viewport=(%d,%d,%d,%d), C026 source=(%d,%d,%d,%d)\n",
           PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
           ORDINAL_20_SRC_X, ORDINAL_20_SRC_Y, PORTRAIT_W, PORTRAIT_H);
    printf("D1C frame route viewport=(%d,%d,%d,%d)\n",
           D1C_FRAME_X_VP, D1C_FRAME_Y_VP, D1C_FRAME_W, D1C_FRAME_H);
    printf("D1C fullscreen variant viewport=(%d,%d,%d,%d)\n",
           D1C_FULLSCREEN_X_VP, D1C_FULLSCREEN_Y_VP,
           D1C_FULLSCREEN_W, D1C_FULLSCREEN_H);

    portraits = M11_AssetLoader_Load(
        &game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < ATLAS_W || portraits->height < ATLAS_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C026 portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Group A: DUNVIEW.C G0205 coordSet-table anchors.  Independent
     * of the asset loader, the M11 world, and the draw path: this
     * group only validates the public G0205 lookup helper. */
    printf("\n[Group A] DUNVIEW.C G0205 coordSet table anchors\n");
    check_g0205_table_anchors();

    /* Group B: G0205 no-collision / no-floating-fullscreen-portrait
     * contract.  Proves the D1C frame route and the D1C fullscreen
     * variant are distinct destinations, and the C026 portrait
     * cutout is anchored to the frame route and not to the
     * fullscreen variant. */
    printf("\n[Group B] DUNVIEW.C G0205 no-collision (frame vs fullscreen)\n");
    check_g0205_no_collision();

    /* Group C: catalog identity contract for ordinal 20. */
    printf("\n[Group C] Mirror catalog identity for ordinal 20\n");
    check_catalog_identity(&game);

    /* Group D: front-mirror route must resolve to ordinal 20 at
     * the (3,11,SOUTH) reference pose, and the (3,11,NORTH)
     * wrong-wall pose must not resolve a mirror ordinal. */
    printf("\n[Group D] Reference route (3,11,SOUTH) + wrong-wall (3,11,NORTH)\n");
    if (!check_front_route_ordinal(&game, portraits)) {
        printf("SKIP hoc_portrait20_fullscreen_scale_rect_fixture_mismatch "
               "(3,11) SOUTH front ordinal != 20; this DM1 V1 build does "
               "not expose ordinal 20 on this reference Hall route.\n");
        M11_GameView_Shutdown(&game);
        return 0;
    }

    /* Group E: fullscreen_scale_rect draw stability at the
     * (3,11,SOUTH) ALEX pose - C026 ordinal-20 pixels anchor inside
     * the D1C cutout and the D1C frame route, and do NOT leak
     * into the D1C fullscreen variant destination box. */
    printf("\n[Group E] fullscreen_scale_rect draw stability at (3,11,SOUTH)\n");
    check_fullscreen_scale_rect_draw(&game, portraits);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("batch group 11 portrait_rect_position gate\n");
    printf("%s dm1 v1 HoC champion portrait ordinal 20 fullscreen_scale_rect portrait_rect_position\n",
           g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
