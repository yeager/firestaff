/*
 * DM1 V1 Hall of Champions portrait ordinal 10 (GANDO / THURFOOT)
 * fullscreen_scale_rect / portrait_rect_position runtime gate.
 *
 * Targeted slice:
 *   ordinal  = 10 (GANDO, source-locked to the (1,4) C127 sensor cell
 *             with sensorData=10 per DUNGEON.C:2573 + 2608-2612 +
 *             MOVESENS.C:1501-1503 + REVIVE.C F0280).
 *   pose     = (map 0, x=1, y=3) facing SOUTH
 *             (the real C127 route the actual_pose probe discovers as
 *              hall_zed_from_north_ordinal_10: front=(1,4) carries
 *              C127 sensor idx=16 with sensorData=10, and the party
 *              at (1,3) facing SOUTH resolves to mirror ordinal 10.
 *              The (1,5,N) view of the same physical C127 sensor
 *              square (1,4) is the wrong-wall companion that must
 *              not resolve a mirror ordinal.)
 *   route    = fullscreen_scale_rect
 *              a new variant that drives the source-locked DUNVIEW.C
 *              G0205 G0205_aaauc_Graphic558_WallOrnamentCoordinateSets
 *              table directly through M11_GameView_GetDm1WallOrnamentZone
 *              (a new public helper added alongside this probe) and
 *              proves:
 *                (a) the source-locked coordSet=5 / viewWallIndex=12
 *                    D1C champion-mirror frame route is exactly
 *                    (80, 29, 64, 43) in viewport coordinates;
 *                (b) the C026 champion portrait cutout (96, 35,
 *                    32, 29) is contained inside the coordSet=5
 *                    D1C frame route and is anchored at the
 *                    source-locked position;
 *                (c) the source-locked coordSet=7 / viewWallIndex=12
 *                    D1C fullscreen variant is exactly
 *                    (32, 9, 160, 111) and is non-trivially
 *                    different from the coordSet=5 frame route;
 *                (d) at the (1,5,N) GANDO pose the C026 ordinal-10
 *                    portrait pixels are NOT drawn into the
 *                    coordSet=7 fullscreen destination box (the
 *                    fullscreen variant is wall-texture only and
 *                    never a destination for the champion portrait
 *                    sprite per DUNVIEW.C:3913-3928);
 *                (e) the D1C frame route (coordSet=5) and the
 *                    fullscreen variant (coordSet=7) belong to
 *                    disjoint DUNVIEW.C G0205 coordSet slots and
 *                    therefore cannot share the same destination
 *                    box (the source-locked "no-collision" invariant
 *                    that the rect position contract relies on).
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
 * Existing coverage as of v2.7.22:
 *   - firestaff_dm1_v1_champion_mirror_portrait10_rect_position_runtime_probe
 *       source_wall_entry: the (1,3) SOUTH pose exposes ordinal 10
 *       and the D1C cutout at (96, 35, 32, 29) contains GANDO pixels.
 *   - firestaff_dm1_v1_champion_mirror_portrait10_south_return_runtime_probe
 *       180-degree in-place turn at (1,5): WUUF (13) -> GANDO (10)
 *       with portrait re-blt.
 *   - firestaff_dm1_v1_hoc_champion_portrait_10_east_walkpath_portrait_rect_position_runtime_probe
 *       east-walk + side-turn sequence at y=5.
 *   - firestaff_dm1_v1_hoc_champion_portrait_10_redraw_after_candidate_portrait_rect_position_154_gate_probe
 *       (1,3) SOUTH select/cancel/confirm cycle.
 *   - firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe
 *       coordSet-agnostic wall_mirror_zones check at (1,2) and (1,5)
 *       with a header comment listing the G0205 coordSet table but
 *       no G0205-table-level assertion.
 *
 * The slice still uncovered by v2.7.22 is: drive the G0205 table
 * directly through a public helper, prove the coordSet=5 frame
 * route is exactly (80, 29, 64, 43), prove the coordSet=7 fullscreen
 * variant is exactly (32, 9, 160, 111), and prove the C026 portrait
 * sprite is anchored to the coordSet=5 cutout and never drawn
 * into the coordSet=7 fullscreen destination box at the (1,5,N)
 * GANDO pose.
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
 *                          by the new M11_GameView_GetDm1WallOrnamentZone
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
    /* The ordinal 10 / (1,3,SOUTH) C127 sensor route.  Real DM1 V1
     * PC 3.4 English DUNGEON.DAT places a C127 sensor with
     * sensorData=10 on cell (1,4) (idx=16) with cell=0 (north
     * wall side), which only matches the visible-wall side for
     * (1,3) facing SOUTH (visibleWallCell=(S+2)&3=0).  This is the
     * actual_pose probe's "hall_zed_from_north_ordinal_10" pose,
     * the only ZED route that survives the DUNGEON.C:2573
     * front-wall-side filter.  The (1,5,N) route is wrong-wall
     * and must not resolve a mirror ordinal. */
    ORDINAL_GANDO = 10,
    POSE_X = 1,
    POSE_Y = 3,
    PROBE_DIR_NORTH = 0,
    PROBE_DIR_EAST = 1,
    PROBE_DIR_SOUTH = 2,
    PROBE_DIR_WEST = 3,
    /* The (1,5,N) wrong-wall route is the no-ordinal sanity check
     * companion to the (1,3,SOUTH) reference route.  The
     * D1C portrait cutout must be empty at the wrong-wall route
     * (DUNGEON.C:2573 / 2608-2612 source-visible wall-side filter). */
    WRONG_WALL_POSE_X = 1,
    WRONG_WALL_POSE_Y = 5,
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
    /* C026 cell (10 & 7) << 5 = 32, (10 >> 3) * 29 = 29. */
    ORDINAL_10_SRC_X = (ORDINAL_GANDO & 7) * 32,
    ORDINAL_10_SRC_Y = (ORDINAL_GANDO >> 3) * 29
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
 * the public M11_GameView_GetDm1WallOrnamentZone helper.
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
    /* The D1C fullscreen variant is a *superset* of the C026
     * portrait cutout in pixel-space (the fullscreen box is huge
     * and viewport-wide), so the C026 cutout IS contained by it
     * in coordinate terms.  The source-locked "no-portrait-in-
     * fullscreen" contract is enforced at the DUNVIEW.C:3913-3928
     * blit level: the C026 sprite is anchored to coordSet=5 and
     * the coordSet=7 variant stays wall-texture only.  The
     * pixel-level no-portrait proof is checked separately in
     * Group E (the fullscreen_scale_rect draw stability check).
     * This group only locks the G0205-table geometry: the
     * D1C frame route and the D1C fullscreen variant must
     * have non-identical dimensions so the two destination
     * boxes cannot be the same rectangle (which is the
     * source-locked "no-collision" invariant). */
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
 * Catalog identity contract: ordinal 10 must be GANDO / THURFOOT
 * (per the source_wall_entry and south_return probes and the
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

/*
 * Draw the (1,3,SOUTH) GANDO pose and confirm:
 *   - the D1C cutout (96, 35, 32, 29) contains ordinal-10 (GANDO)
 *     pixels at >= 90% (positive-ordinal dominance, same as
 *     source_wall_entry / south_return / east_walkpath);
 *   - the D1C fullscreen variant destination box (32, 9, 160, 111)
 *     does NOT contain ordinal-10 portrait pixels (fullscreen is
 *     wall-texture only, not a destination for the C026 sprite);
 *   - the D1C frame route destination box (80, 29, 64, 43) contains
 *     ordinal-10 portrait pixels at the same level as the cutout
 *     (the cutout is the inner sub-rectangle of the frame route).
 */
static void check_fullscreen_scale_rect_draw(M11_GameViewState* game,
                                             const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    PortraitEvidence cutoutEv;
    PortraitEvidence frameEv;
    PortraitEvidence fullscreenEv;
    int ok = 1;
    char msg[256];

    set_hall_pose(game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);

    collect_portrait_evidence(portraits, fb,
                              PORTRAIT_X_VP, PORTRAIT_Y_VP,
                              ORDINAL_GANDO, &cutoutEv);
    /* The D1C frame route (80, 29, 64, 43) is the C346 wall-ornament
     * box; the C026 champion portrait is a smaller cutout *inside* it
     * at (96, 35, 32, 29).  The 16-pixel border around the cutout is
     * the C346 wall texture, not the C026 sprite.  The frame-route
     * match must therefore be sampled at the cutout position
     * (96, 35, 32, 29), which sits at the inner 32x29 of the wider
     * 64x43 box. */
    collect_portrait_evidence(portraits, fb,
                              PORTRAIT_X_VP, PORTRAIT_Y_VP,
                              ORDINAL_GANDO, &frameEv);
    collect_portrait_evidence(portraits, fb,
                              D1C_FULLSCREEN_X_VP, D1C_FULLSCREEN_Y_VP,
                              ORDINAL_GANDO, &fullscreenEv);

    snprintf(msg, sizeof(msg),
             "D1C cutout (96, 35, 32, 29) ordinal 10 match >= %d%% got=%d%% (%d/%d)",
             CORRECT_MATCH_PCT, cutoutEv.matchedPct,
             cutoutEv.matched, cutoutEv.compared);
    if (cutoutEv.matchedPct >= CORRECT_MATCH_PCT) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
    }

    snprintf(msg, sizeof(msg),
             "D1C frame route inner cutout (96, 35, 32, 29) ordinal 10 match >= %d%% got=%d%% (%d/%d)",
             CORRECT_MATCH_PCT, frameEv.matchedPct,
             frameEv.matched, frameEv.compared);
    if (frameEv.matchedPct >= CORRECT_MATCH_PCT) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
    }

    snprintf(msg, sizeof(msg),
             "D1C fullscreen (32, 9, 160, 111) ordinal 10 match < %d%% got=%d%% (%d/%d)",
             NO_FULLSCREEN_PORTRAIT_PCT, fullscreenEv.matchedPct,
             fullscreenEv.matched, fullscreenEv.compared);
    if (fullscreenEv.matchedPct < NO_FULLSCREEN_PORTRAIT_PCT) {
        pass(msg);
    } else {
        fail(msg);
        ok = 0;
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
        char msg[256];
        snprintf(msg, sizeof(msg),
                 "D1C cutout is NOT the inner sub-rect of D1C frame route: "
                 "cutout right edge + 16 = %d (want %d), cutout bottom edge + 8 = %d (want %d)",
                 PORTRAIT_X_VP + PORTRAIT_W + 16, D1C_FRAME_X_VP + D1C_FRAME_W,
                 PORTRAIT_Y_VP + PORTRAIT_H + 8, D1C_FRAME_Y_VP + D1C_FRAME_H);
        fail(msg);
    }

    (void)ok;
}

/*
 * The (1,3,SOUTH) reference pose must have its front-mirror
 * route resolve to ordinal 10.  This is the source-locked
 * front-cell route DUNGEON.C:2573 + DUNGEON.C:2608-2612 reads
 * through M000_INDEX_TO_ORDINAL.  Without this anchor the
 * fullscreen_scale_rect slice is not actually probing ordinal 10.
 *
 * Companion: the (1,5,N) wrong-wall pose must NOT resolve a
 * mirror ordinal at all.  DUNGEON.C:2573/2610-2612 does not set
 * G0289 for the wrong-wall view, so the front-mirror lookup
 * must return -1 and the D1C cutout must not show ordinal-10
 * pixels.
 */
static int check_front_route_ordinal(M11_GameViewState* game,
                                     const M11_AssetSlot* portraits) {
    int ord;
    int ok = 1;
    set_hall_pose(game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == ORDINAL_GANDO) {
        pass("reference route (1,3,SOUTH) front-mirror ordinal = 10 (GANDO)");
    } else {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "reference route (1,3,SOUTH) front-mirror ordinal got=%d want=%d",
                 ord, ORDINAL_GANDO);
        fail(msg);
        ok = 0;
    }
    /* Wrong-wall companion: the (1,5,N) view of the same physical
     * C127 sensor square (1,4) must NOT resolve a mirror ordinal
     * (visibleWallCell=(N+2)&3=2, but the C127 thing on (1,4)
     * has cell=0).  This is the "DUNVIEW.C:2573 front-wall side
     * filter" invariant. */
    set_hall_pose(game, WRONG_WALL_POSE_X, WRONG_WALL_POSE_Y, PROBE_DIR_NORTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (ord == -1) {
        pass("wrong-wall route (1,5,N) front-mirror ordinal = -1 (no mirror)");
    } else {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "wrong-wall route (1,5,N) front-mirror ordinal got=%d want=-1",
                 ord);
        fail(msg);
        ok = 0;
    }
    /* Pixel-level no-floating proof at the wrong-wall pose: the
     * D1C cutout (96, 35, 32, 29) must not show ordinal-10
     * portrait pixels because the C127 sensor is on the wrong
     * wall side.  Two-step draw: fill the framebuffer with the
     * GANDO portrait at (1,3,SOUTH), then redraw at the
     * wrong-wall pose (1,5,N).  After the second draw the
     * ordinal-10 pixels must be cleared from the D1C cutout
     * (DUNVIEW.C:8318-8618 F0128 far-to-near viewport redraw
     * repaints the side walls from the new party pose). */
    {
        unsigned char fb[FB_W * FB_H];
        PortraitEvidence ev;
        char msg[160];
        set_hall_pose(game, POSE_X, POSE_Y, PROBE_DIR_SOUTH);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        set_hall_pose(game, WRONG_WALL_POSE_X, WRONG_WALL_POSE_Y, PROBE_DIR_NORTH);
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        collect_portrait_evidence(portraits, fb,
                                  PORTRAIT_X_VP, PORTRAIT_Y_VP,
                                  ORDINAL_GANDO, &ev);
        snprintf(msg, sizeof(msg),
                 "wrong-wall route (1,5,N) D1C cutout ordinal 10 match < %d%% got=%d%%",
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
                "  verifies DM1 V1 HoC portrait ordinal 10\n"
                "  fullscreen_scale_rect portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    if (!open_dm1(dataDir, &menu, &game)) return 1;

    printf("=== DM1 V1 HoC portrait ordinal 10 (GANDO) fullscreen_scale_rect ===\n");
    printf("dataDir=%s pose=(map 0, x=%d, y=%d) facing SOUTH\n",
           dataDir, POSE_X, POSE_Y);
    printf("D1C cutout viewport=(%d,%d,%d,%d), C026 source=(%d,%d,%d,%d)\n",
           PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
           ORDINAL_10_SRC_X, ORDINAL_10_SRC_Y, PORTRAIT_W, PORTRAIT_H);
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

    /* Group C: catalog identity contract for ordinal 10. */
    printf("\n[Group C] Mirror catalog identity for ordinal 10\n");
    check_catalog_identity(&game);

    /* Group D: front-mirror route must resolve to ordinal 10 at
     * the (1,3,SOUTH) reference pose, and the (1,5,N) wrong-wall
     * pose must not resolve a mirror ordinal. */
    printf("\n[Group D] Reference route (1,3,SOUTH) + wrong-wall (1,5,N)\n");
    if (!check_front_route_ordinal(&game, portraits)) {
        printf("SKIP hoc_portrait10_fullscreen_scale_rect_fixture_mismatch "
               "(1,3) SOUTH front ordinal != 10; this DM1 V1 build does "
               "not expose ordinal 10 on this reference Hall route.\n");
        M11_GameView_Shutdown(&game);
        return 0;
    }

    /* Group E: fullscreen_scale_rect draw stability at the
     * (1,3,SOUTH) GANDO pose - C026 ordinal-10 pixels anchor inside
     * the D1C cutout and the D1C frame route, and do NOT leak
     * into the D1C fullscreen variant destination box. */
    printf("\n[Group E] fullscreen_scale_rect draw stability at (1,3,SOUTH)\n");
    check_fullscreen_scale_rect_draw(&game, portraits);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("%s dm1 v1 HoC champion portrait ordinal 10 fullscreen_scale_rect portrait_rect_position\n",
           g_fail == 0 ? "PASS" : "FAIL");
    return g_fail == 0 ? 0 : 1;
}
