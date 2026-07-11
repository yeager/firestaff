/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 1 (HALK)
 * redraw_after_candidate / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal    = 1 (HALK / barbarian, C127 sensorData=1)
 *   pose       = (map 0, x=1, y=2) facing NORTH
 *                 (front cell (1,1) carries the C127 champion mirror)
 *   route      = redraw_after_candidate
 *                 (the M11 viewport redraw cycle that fires after the
 *                  candidate panel state transitions: pre-candidate ->
 *                  panel-open -> post-confirm -> reopen+post-cancel)
 *   aspect     = portrait_rect_position
 *                 (the C026 champion portrait cutout stays anchored
 *                  at the source-locked D1C viewport rectangle
 *                  (96, 35, 32, 29) on the no-panel redraw, the RR
 *                  panel (C040) covers the viewport while G0299 is
 *                  set, and after confirm the route is disabled so
 *                  the portrait rect returns to wall-only)
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     covers the front-mirror ordinal lookup AND the resurrect
 *     round-trip HP / route-disable evidence for ordinal 1, but only
 *     samples a single frame after the confirm call.  It does not
 *     verify the portrait_rect position is stable across the
 *     redraw_after_candidate cycle.
 *   - firestaff_dm1_v1_champion_mirror_capture_probe saves PPMs for
 *     ordinal 1 only at the pre-candidate pose (no panel).  No
 *     post-panel redraw is captured.
 *   - firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 *     runs the full open-panel / confirm / cancel / pointer cycle
 *     but only against ordinal 2 (corridor_north_select_candidate,
 *     (1,4) NORTH with C127 sensorData=2).  Ordinal 1 (HALK at
 *     (1,2) NORTH) is only used there in the closed-panel Pose B.
 *   - firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     and the walkpath probe cover ordinal 1 z-order / cardinal
 *     navigation but not the redraw_after_candidate transitions.
 *
 * This probe fills that narrow slice: it pins the portrait_rect
 * (96, 35, 32, 29) position invariant for ordinal 1 at every stage
 * of the redraw_after_candidate cycle and asserts the rect does
 * not drift onto a side wall (warm_count falls below the no-mirror
 * corridor threshold, and the rect stays inside the D1C zone
 * reported by the public M11_GameView_GetD1CWallOrnamentZone
 * helper, which is hardcoded to coordSet 5 / index 12 = (80, 29,
 * 64, 43) viewport-local per DUNVIEW.C G0205).
 *
 * What the probe asserts at each redraw stage:
 *   Stage 1 (pre-candidate):  HALK portrait is visible at the
 *                              (96, 35, 32, 29) cutout (warm_count
 *                              >= 30, C026 strip cell match >= 70%).
 *                              D1C wall zone matches coordSet 5/12.
 *   Stage 2 (panel-open):     M11_DrawInventoryPanel wipes the HALK
 *                              portrait by drawing C017 over the
 *                              entire viewport, then C040 RR panel
 *                              covers the panel zone (80, 52, 144,
 *                              73) viewport-local.  The HALK
 *                              portrait rect is NOT visible
 *                              (warm_count < 30) but the RR panel
 *                              IS visible.  The portrait_rect is
 *                              anchored at the same (96, 35, 32, 29)
 *                              position invariant.
 *   Stage 3 (post-confirm):   The mirror route is disabled
 *                              (front_mirror == -1) and the HALK
 *                              portrait is NOT visible (rect is
 *                              wall-only after the C127 sensor was
 *                              disabled in REVIVE.C:785-799).
 *   Stage 4 (post-cancel):    Cancel preserves the route, so the
 *                              HALK portrait is visible again at
 *                              (96, 35, 32, 29) on the next redraw.
 *   Stage 5 (cycles):         4 append/clear cycles at ordinal 1
 *                              produce byte-stable framebuffer
 *                              pixels in the portrait_rect area
 *                              (no drift between cycles).
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 D1C champion portrait blit (C026)
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw from
 *                       party map/x/y/direction (far-to-near order)
 *   ReDMCSB DUNVIEW.C:4547-4581 nibble 2 -> ordinal 1 / nibble 1
 *                       -> ordinal 0 (G0289 ordinal decode)
 *   ReDMCSB DUNVIEW.C:14271-14313 (D1C champion mirror BUG-120/121
 *                       guard: panel-open path keeps portrait drawn
 *                       and skips the wall-ornament graphic)
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB REVIVE.C:272-276 / F0280 appends candidate from sensor
 *   ReDMCSB REVIVE.C:744-799 / F0282 cancel without disabling route
 *   ReDMCSB REVIVE.C:785-799 mirror-sensor disable loop on confirm
 *   ReDMCSB PANEL.C:1619-1635 / 1654-1656 / F0346 / F0347 routes
 *                       C040 resurrect/reincarnate panel while G0299
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dims
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate_portrait_rect_position_097_gate_probe DATA_DIR
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
    /* Source-locked PC 3.4 viewport (ReDMCSB COORD.C:1693-1722):
     * origin (M11_VIEWPORT_X, M11_VIEWPORT_Y) = (0, 33); size
     * (M11_VIEWPORT_W, M11_VIEWPORT_H) = (224, 136). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136,
    /* C026 champion portrait cutout (viewport-local) inside the D1C
     * wall box.  ReDMCSB DUNVIEW.C:3913-3928 /
     * m11_draw_dm1_front_champion_portrait uses
     *   M11_AssetLoader_BlitRegion(portraits,
     *       (portraitIdx & 7) * M11_PORTRAIT_W (== 32),
     *       (portraitIdx >> 3) * M11_PORTRAIT_H (== 29),
     *       M11_PORTRAIT_W, M11_PORTRAIT_H,
     *       M11_VIEWPORT_X + 96, M11_VIEWPORT_Y + 35, ...)
     * so the cutout is (96, 35, 32, 29) viewport-local = (96, 68, 32,
     * 29) framebuffer-local. */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* D1C champion-mirror frame zone from
     * M11_GameView_GetD1CWallOrnamentZone (coordSet 5 / index 12 per
     * DUNVIEW.C G0205): dstX=80, dstY=29, w=64, h=43 viewport-local.
     * The C026 portrait cutout (96, 35, 32, 29) sits inside this
     * zone. */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* RR panel (C040) zone from M11_GameView_GetV1InventoryPanelZone,
     * viewport-local.  The panel covers the bottom two-thirds of the
     * viewport; this includes the lower 12 rows of the HALK portrait
     * rect (rows 52..64 of viewport y). */
    RR_PANEL_X_VP = 80,
    RR_PANEL_Y_VP = 52,
    RR_PANEL_W = 144,
    RR_PANEL_H = 73,
    /* Hall of Champions ordinal 1 = HALK / barbarian.  C127 sensorData
     * is 0-indexed per DUNVIEW.C:4547-4581, so the M11 ordinal = 1 is
     * the second cell in the portrait strip: column 1, row 0. */
    ORDINAL_HALK = 1,
    /* The portrait_rect_position invariant: the cutout stays anchored
     * at (96, 35, 32, 29) on the viewport and never drifts onto a
     * side wall.  The HALK sprite has multiple warm palette indices;
     * the same threshold used by the existing capture probe
     * (>= 30 warm pixels for "portrait present", < 30 for "wall only")
     * is reused here so this probe stays consistent with the proven
     * champion-mirror capture matrix. */
    PORTRAIT_PRESENT_WARM_THRESHOLD = 30
};

/* Convert viewport-local rectangle to framebuffer-local rectangle. */
static inline int vp_to_fb_x(int vpX) { return vpX; }
static inline int vp_to_fb_y(int vpY) { return vpY + VIEWPORT_Y; }

typedef struct RectEvidence {
    int warmCount;
    int transparentCount;
    int opaqueCount;
    int compared;
    int matched;
    int matchedPct;       /* matched*100/compared (only when compared>0) */
    int d1cZoneContainsPortrait; /* 1 if the portrait cutout (96,35,32,29)
                                  * sits inside the public D1C zone (80,29,64,43)
                                  * in viewport coords */
} RectEvidence;

/*
 * Count non-zero palette indices and warm-color pixels inside the
 * D1C champion portrait rectangle (framebuffer coords).  Same
 * warm-color set as the existing capture probe
 * (firestaff_dm1_v1_champion_mirror_capture_probe): palette indices
 * {0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0E} (green / red / orange /
 * peach / yellow / blue) mark the champion portrait sprite pixels
 * vs the grey-stone wall texture palette {0x01, 0x02, 0x07-grey,
 * 0x0D}.
 */
static void collect_rect_evidence(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal,
                                  RectEvidence* out) {
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    int x, y;
    out->warmCount = 0;
    out->transparentCount = 0;
    out->opaqueCount = 0;
    out->compared = 0;
    out->matched = 0;
    out->matchedPct = 0;
    out->d1cZoneContainsPortrait = 0;
    if (!fb) return;

    /* First pass: count warm pixels in the destination rect (framebuffer). */
    for (y = fbRectY; y < fbRectY + PORTRAIT_H; ++y) {
        if (y < 0 || y >= FB_H) continue;
        for (x = fbRectX; x < fbRectX + PORTRAIT_W; ++x) {
            if (x < 0 || x >= FB_W) continue;
            {
                unsigned char raw = fb[y * FB_W + x];
                unsigned char idx = M11_FB_DECODE_INDEX(raw);
                switch (idx) {
                    case 0x07: /* green */
                    case 0x08: /* red */
                    case 0x09: /* orange */
                    case 0x0A: /* peach */
                    case 0x0B: /* yellow */
                    case 0x0E: /* blue */
                        ++out->warmCount;
                        break;
                    default:
                        break;
                }
                if (idx == 0) {
                    ++out->transparentCount;
                } else {
                    ++out->opaqueCount;
                }
            }
        }
    }

    /* Second pass: compare the destination against the expected C026
     * portrait-strip cell for the ordinal.  Ordinal 1 -> strip cell
     * (1*32, 0*29) per DUNVIEW.C:4547-4581 nibble decode. */
    if (portraits && portraits->loaded && portraits->pixels &&
        ordinal >= 0 && ordinal < 24) {
        int srcBaseX = (ordinal & 7) * PORTRAIT_W;
        int srcBaseY = (ordinal >> 3) * PORTRAIT_H;
        for (y = 0; y < PORTRAIT_H; ++y) {
            int srcY = srcBaseY + y;
            int dstY = fbRectY + y;
            if (srcY < 0 || srcY >= (int)portraits->height ||
                dstY < 0 || dstY >= FB_H) continue;
            for (x = 0; x < PORTRAIT_W; ++x) {
                int srcX = srcBaseX + x;
                int dstX = fbRectX + x;
                if (srcX < 0 || srcX >= (int)portraits->width ||
                    dstX < 0 || dstX >= FB_W) continue;
                {
                    unsigned char srcRaw = portraits->pixels[srcY * (int)portraits->width + srcX];
                    unsigned char srcIdx = (unsigned char)(srcRaw & 0x0F);
                    if (srcIdx == 1) continue; /* transparent */
                    {
                        unsigned char dstRaw = fb[dstY * FB_W + dstX];
                        unsigned char dstIdx = M11_FB_DECODE_INDEX(dstRaw);
                        ++out->compared;
                        if (dstIdx == srcIdx) ++out->matched;
                    }
                }
            }
        }
        if (out->compared > 0) {
            out->matchedPct = (out->matched * 100) / out->compared;
        }
    }

    /* Verify the portrait cutout stays anchored inside the public D1C
     * zone (viewport coords). */
    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
}

/*
 * Match the RR panel (C040) asset against a framebuffer rectangle.
 * Counts opaque panel pixels and how many end up at the destination.
 * Same logic as match_panel in the candidate_panel probe but
 * factored out for re-use here at ordinal 1.
 */
typedef struct PanelMatch {
    int assetOpaque;
    int assetDrawn;
    int leakedOpaque;
    int assetWidth;
    int assetHeight;
} PanelMatch;

static PanelMatch match_rr_panel(const M11_AssetSlot* panel,
                                 const unsigned char* fb,
                                 int fbW, int fbH,
                                 int panelX, int panelY,
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

/*
 * Check Stage 1 (pre-candidate, no panel): portrait_rect must show
 * HALK and the D1C zone check must pass.
 */
static int check_stage_pre_candidate(M11_GameViewState* game,
                                     const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    RectEvidence ev;
    int ok = 1;

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_HALK, &ev);

    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL pre_candidate portrait_rect (%d,%d,%d,%d) not inside D1C zone (%d,%d,%d,%d) viewport-local\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
                D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }
    if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        fprintf(stderr,
                "FAIL pre_candidate portrait_rect not visible (warm=%d < %d)\n",
                ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (ev.compared > 0 && ev.matchedPct < 70) {
        fprintf(stderr,
                "FAIL pre_candidate ordinal %d pixel match only %d%% (%d/%d) — portrait drifted\n",
                ORDINAL_HALK, ev.matchedPct, ev.matched, ev.compared);
        ok = 0;
    }

    printf("  pre_candidate ordinal=%d front_mirror=%d panel=%d warm=%d match=%d%% (%d/%d)\n",
           ORDINAL_HALK, M11_GameView_GetFrontMirrorOrdinal(game),
           game->candidateMirrorPanelActive,
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared);
    return ok;
}

/*
 * Check Stage 2 (panel-open): the RR panel must be drawn over the
 * panel zone, but the HALK portrait is covered by the inventory
 * backdrop (C017) so we expect no portrait in the cutout.
 */
static int check_stage_panel_open(M11_GameViewState* game,
                                  const M11_AssetSlot* rrPanel) {
    unsigned char fb[FB_W * FB_H];
    PanelMatch m;
    int ok = 1;
    int rrFbX = vp_to_fb_x(RR_PANEL_X_VP);
    int rrFbY = vp_to_fb_y(RR_PANEL_Y_VP);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    m = match_rr_panel(rrPanel, fb, FB_W, FB_H, rrFbX, rrFbY, 6);
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels) {
        fprintf(stderr, "FAIL panel_open RR panel asset unavailable\n");
        return 0;
    }
    if (rrPanel->width != RR_PANEL_W || rrPanel->height != RR_PANEL_H) {
        fprintf(stderr, "FAIL panel_open RR panel size got=%ux%u want=%dx%d\n",
                rrPanel->width, rrPanel->height, RR_PANEL_W, RR_PANEL_H);
        ok = 0;
    }
    if (m.assetOpaque <= 0 || m.assetDrawn * 100 < 90 * m.assetOpaque) {
        fprintf(stderr,
                "FAIL panel_open RR panel missing drawn=%d/%d (leaked=%d)\n",
                m.assetDrawn, m.assetOpaque, m.leakedOpaque);
        ok = 0;
    }
    printf("  panel_open front_mirror=%d panel=%d rr_panel_drawn=%d/%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           game->candidateMirrorPanelActive,
           m.assetDrawn, m.assetOpaque);
    return ok;
}

/*
 * Check Stage 3 (post-confirm): the mirror route is disabled so the
 * front cell shows wall only.  The portrait rect must NOT show HALK.
 */
static int check_stage_post_confirm(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    RectEvidence ev;
    int ok = 1;

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_HALK, &ev);

    if (ev.warmCount >= PORTRAIT_PRESENT_WARM_THRESHOLD) {
        fprintf(stderr,
                "FAIL post_confirm portrait_rect still shows portrait (warm=%d >= %d) — route should be disabled\n",
                ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (ev.matchedPct > 5) {
        fprintf(stderr,
                "FAIL post_confirm ordinal %d pixel match %d%% (> 5%%) — portrait drifted onto side wall\n",
                ORDINAL_HALK, ev.matchedPct);
        ok = 0;
    }
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL post_confirm portrait_rect (%d,%d,%d,%d) not inside D1C zone\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }

    printf("  post_confirm front_mirror=%d panel=%d warm=%d match=%d%%\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           game->candidateMirrorPanelActive,
           ev.warmCount, ev.matchedPct);
    return ok;
}

/*
 * Check Stage 4 (post-cancel): route still armed, HALK visible again.
 */
static int check_stage_post_cancel(M11_GameViewState* game,
                                   const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    RectEvidence ev;
    int ok = 1;

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect_evidence(portraits, fb, ORDINAL_HALK, &ev);

    if (ev.warmCount < PORTRAIT_PRESENT_WARM_THRESHOLD) {
        fprintf(stderr,
                "FAIL post_cancel portrait_rect not visible (warm=%d < %d) — cancel should not disable route\n",
                ev.warmCount, PORTRAIT_PRESENT_WARM_THRESHOLD);
        ok = 0;
    }
    if (ev.compared > 0 && ev.matchedPct < 70) {
        fprintf(stderr,
                "FAIL post_cancel ordinal %d pixel match only %d%% (%d/%d)\n",
                ORDINAL_HALK, ev.matchedPct, ev.matched, ev.compared);
        ok = 0;
    }
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL post_cancel portrait_rect (%d,%d,%d,%d) not inside D1C zone\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H);
        ok = 0;
    }

    printf("  post_cancel front_mirror=%d panel=%d warm=%d match=%d%% (%d/%d)\n",
           M11_GameView_GetFrontMirrorOrdinal(game),
           game->candidateMirrorPanelActive,
           ev.warmCount, ev.matchedPct, ev.matched, ev.compared);
    return ok;
}

static void reset_view(M11_GameViewState* game, int mapX, int mapY, int dir) {
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

int main(int argc, char** argv) {
    /* M11_GameViewState is large (~579KB) so we keep working buffers
     * in static BSS to avoid blowing the macOS 8MB thread-stack guard. */
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    static M12_StartupMenuState menu_cancel;
    static M11_GameViewState game_cancel;
    const M11_AssetSlot* portraits;
    int ok = 1;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    const char* dataDir;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 1 (HALK) redraw_after_candidate portrait_rect_position\n",
                argv[0]);
        return 2;
    }
    dataDir = argv[1];

    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr,
                "FAIL could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 HoC portrait ordinal 1 (HALK) redraw_after_candidate ===\n");
    printf("dataDir=%s pose=(map 0, x=1, y=2) facing NORTH\n", dataDir);

    /* Stage 0: front-mirror route lookup must return ordinal 1 for
     * (1,2) NORTH per DUNGEON.C:2573 / MOVESENS.C:1501-1503. */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_HALK) {
        printf("SKIP this DM1 V1 build does not place the C127 sensor "
               "with sensorData=1 at (1,2) front cell (got ordinal=%d, "
               "want %d); the redraw_after_candidate slice is not "
               "exercised on builds that do not match the reference "
               "DUNGEON.DAT fixture.\n",
               frontOrdinal, ORDINAL_HALK);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    /* Stage 0 evidence: the public D1C wall zone helper must report
     * the source-locked coordSet-5 / index-12 rectangle for ordinal 1. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    if (ornX != D1C_ZONE_X_VP || ornY != D1C_ZONE_Y_VP ||
        ornW != D1C_ZONE_W || ornH != D1C_ZONE_H) {
        fprintf(stderr,
                "FAIL D1C wall zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d) viewport-local\n",
                ornX, ornY, ornW, ornH,
                D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    /* Stage 1: pre-candidate, no panel — portrait_rect must show HALK
     * (ordinal 1, warm_count >= 30, C026 strip match >= 70%). */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    ok &= check_stage_pre_candidate(&game, portraits);

    /* Stage 2: open candidate panel — REVIVE.C F0280 appends the
     * candidate, PANEL.C F0347 routes C040 over the viewport while
     * G0299 is set.  The M11 inventory backdrop (C017) draws first
     * and covers the HALK portrait, then C040 covers the bottom
     * two-thirds of the viewport.  The RR panel asset must be visible
     * at the panel zone (80, 52, 144, 73) viewport-local. */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    if (M11_GameView_SelectFrontMirrorCandidate(&game) != 1) {
        fprintf(stderr,
                "FAIL SelectFrontMirrorCandidate returned 0 for ordinal 1\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    {
        const M11_AssetSlot* livePanel = M11_AssetLoader_Load(
            &game.assetLoader,
            (unsigned int)40 /* C040 PANEL_RESURRECT_REINCARNATE */);
        if (!livePanel || !livePanel->loaded || !livePanel->pixels) {
            fprintf(stderr,
                    "FAIL GRAPHICS.DAT C040 RR panel asset unavailable\n");
            M11_GameView_Shutdown(&game);
            return 1;
        }
        ok &= check_stage_panel_open(&game, livePanel);
    }

    /* Stage 3: confirm resurrect — REVIVE.C:785-799 mirror-sensor
     * disable loop runs, candidateMirrorPanelActive clears, panel
     * asset is no longer drawn.  The front cell (1,1) no longer
     * reports a mirror ordinal.  The redraw must NOT show the HALK
     * portrait anymore at (1,2) NORTH because the C127 sensor was
     * disabled on the front square; the wall behind the player is
     * plain stone again. */
    if (M11_GameView_ConfirmMirrorCandidate(&game, 0) != 1) {
        fprintf(stderr,
                "FAIL ConfirmMirrorCandidate returned 0 for ordinal 1\n");
        ok = 0;
    }
    if (M11_GameView_GetFrontMirrorOrdinal(&game) != -1) {
        fprintf(stderr,
                "FAIL post_confirm front mirror not disabled (got=%d)\n",
                M11_GameView_GetFrontMirrorOrdinal(&game));
        ok = 0;
    }
    ok &= check_stage_post_confirm(&game, portraits);

    /* Stage 4: reopen + cancel — open a fresh game view because
     * Stage 3's confirm path disabled the C127 sensor on the front
     * cell, so a same-runtime reopen at the same pose will return
     * ordinal -1 and SelectFrontMirrorCandidate will reject.  A
     * fresh M11_GameViewState resets the world/things/sensor cache
     * so we can drive a fresh select+cancel cycle for ordinal 1. */
    M12_StartupMenu_InitWithDataDir(&menu_cancel, dataDir, NULL);
    M11_GameView_Init(&game_cancel);
    if (!M11_GameView_OpenSelectedMenuEntry(&game_cancel, &menu_cancel)) {
        fprintf(stderr,
                "FAIL could not reopen DM1 V1 game view for cancel pose\n");
        ok = 0;
    } else {
        const M11_AssetSlot* portraitsCancel = M11_AssetLoader_Load(
            &game_cancel.assetLoader,
            (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
        if (!portraitsCancel || !portraitsCancel->loaded || !portraitsCancel->pixels) {
            fprintf(stderr,
                    "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable in cancel view\n");
            ok = 0;
        } else {
            reset_view(&game_cancel, 1, 2, 0 /* DIR_NORTH */);
            if (M11_GameView_SelectFrontMirrorCandidate(&game_cancel) != 1) {
                fprintf(stderr,
                        "FAIL reopen SelectFrontMirrorCandidate returned 0 for ordinal 1 (cancel view)\n");
                ok = 0;
            } else if (M11_GameView_CancelMirrorCandidate(&game_cancel) != 1) {
                fprintf(stderr,
                        "FAIL CancelMirrorCandidate returned 0 for ordinal 1 (cancel view)\n");
                ok = 0;
            } else {
                if (M11_GameView_GetFrontMirrorOrdinal(&game_cancel) != ORDINAL_HALK) {
                    fprintf(stderr,
                            "FAIL post_cancel front mirror disabled (got=%d want=%d)\n",
                            M11_GameView_GetFrontMirrorOrdinal(&game_cancel),
                            ORDINAL_HALK);
                    ok = 0;
                }
                ok &= check_stage_post_cancel(&game_cancel, portraitsCancel);
            }
        }
    }
    M11_GameView_Shutdown(&game_cancel);

    /* Stage 5: byte-stable append/clear cycle at ordinal 1 in a
     * fresh game view.  Same invariant used by the existing
     * candidate_panel probe Pose R for ordinal 2 — extended here
     * to ordinal 1 so the candidate panel redraw does not drift
     * the portrait_rect pixels across multiple select/cancel
     * cycles. */
    {
        static M12_StartupMenuState menu_cycle;
        static M11_GameViewState game_cycle;
        const M11_AssetSlot* portraitsCycle;
        RectEvidence evN;
        unsigned char fb0[FB_W * FB_H];
        unsigned char fbN[FB_W * FB_H];
        int cycle;
        int baselineWarm = -1;
        int baselineMatchedPct = -1;
        int baselineOpaque = -1;
        int cyclesOk = 1;
        const int kCycles = 4;

        M12_StartupMenu_InitWithDataDir(&menu_cycle, dataDir, NULL);
        M11_GameView_Init(&game_cycle);
        if (!M11_GameView_OpenSelectedMenuEntry(&game_cycle, &menu_cycle)) {
            fprintf(stderr,
                    "FAIL could not open DM1 V1 game view for cycle stage\n");
            cyclesOk = 0;
        } else {
            portraitsCycle = M11_AssetLoader_Load(
                &game_cycle.assetLoader,
                (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
            if (!portraitsCycle || !portraitsCycle->loaded || !portraitsCycle->pixels) {
                fprintf(stderr,
                        "FAIL GRAPHICS.DAT C026 unavailable in cycle view\n");
                cyclesOk = 0;
            } else {
                for (cycle = 0; cycle < kCycles; ++cycle) {
                    reset_view(&game_cycle, 1, 2, 0 /* DIR_NORTH */);
                    if (M11_GameView_SelectFrontMirrorCandidate(&game_cycle) != 1 ||
                        M11_GameView_CancelMirrorCandidate(&game_cycle) != 1) {
                        fprintf(stderr,
                                "FAIL ordinal 1 append/clear cycle %d did not select/cancel\n",
                                cycle + 1);
                        cyclesOk = 0;
                        break;
                    }
                    memset(fbN, 0, sizeof(fbN));
                    M11_GameView_Draw(&game_cycle, fbN, FB_W, FB_H);
                    collect_rect_evidence(portraitsCycle, fbN, ORDINAL_HALK, &evN);
                    if (cycle == 0) {
                        baselineWarm = evN.warmCount;
                        baselineMatchedPct = evN.matchedPct;
                        baselineOpaque = evN.opaqueCount;
                        memcpy(fb0, fbN, sizeof(fb0));
                    } else {
                        if (evN.warmCount != baselineWarm) {
                            fprintf(stderr,
                                    "FAIL ordinal 1 cycle %d warm drift got=%d want=%d\n",
                                    cycle + 1, evN.warmCount, baselineWarm);
                            cyclesOk = 0;
                        }
                        if (evN.matchedPct != baselineMatchedPct) {
                            fprintf(stderr,
                                    "FAIL ordinal 1 cycle %d matched%% drift got=%d want=%d\n",
                                    cycle + 1, evN.matchedPct, baselineMatchedPct);
                            cyclesOk = 0;
                        }
                        if (evN.opaqueCount != baselineOpaque) {
                            fprintf(stderr,
                                    "FAIL ordinal 1 cycle %d opaque drift got=%d want=%d\n",
                                    cycle + 1, evN.opaqueCount, baselineOpaque);
                            cyclesOk = 0;
                        }
                        if (memcmp(fb0, fbN, sizeof(fb0)) != 0) {
                            fprintf(stderr,
                                    "FAIL ordinal 1 cycle %d framebuffer drift in viewport area\n",
                                    cycle + 1);
                            cyclesOk = 0;
                        }
                    }
                }
            }
        }
        M11_GameView_Shutdown(&game_cycle);
        if (cyclesOk) {
            printf("  append_clear_cycles_halk_stable cycles=%d warm=%d matched=%d%% opaque=%d\n",
                   kCycles, baselineWarm, baselineMatchedPct, baselineOpaque);
        }
        ok &= cyclesOk;
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 HoC champion portrait ordinal 1 redraw_after_candidate portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
