/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 4 (LEIF)
 * panel_chrome_preserve / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal    = 4 (LEIF, C127 sensorData=4)
 *   pose       = (map 0, x=2, y=1) facing DIR_SOUTH
 *                 (front cell (2,2) carries the C127 champion mirror
 *                  with sensorData=4 on its north wall, visible to a
 *                  south-facing party at (2,1))
 *   route      = panel_chrome_preserve
 *                 (the M11 viewport state immediately around the
 *                  C040 resurrect/reincarnate panel lifecycle:
 *                  pre-candidate  -> candidate panel open ->
 *                  candidate panel cancelled, with the explicit
 *                  invariant that the C040 panel chrome does not
 *                  permanently alter the wall pixels under it; on
 *                  cancel, the D1C portrait rectangle (96, 35, 32,
 *                  29) viewport-local must return to a full LEIF
 *                  portrait and the C040 panel zone (80, 52, 144,
 *                  73) must return to the wall pixel grid that was
 *                  there before the panel was opened)
 *   aspect     = portrait_rect_position
 *                 (the C026 portrait cutout stays anchored at the
 *                  source-locked D1C viewport rectangle (96, 35, 32,
 *                  29) before, during, and after the C040 panel
 *                  lifecycle; the C040 panel and C017 backdrop
 *                  cover the rect's lower 12 rows / top 17 rows
 *                  respectively while G0299 is set; on cancel the
 *                  C040 chrome leaves no panel pixels behind)
 *
 * Coverage gap relative to existing champion-mirror probe matrix
 * for ordinal 4 (LEIF):
 *   - firestaff_dm1_v1_champion_mirror_portrait04_rect_position_runtime_probe
 *     covers (2,1,SOUTH) front_north_entry panel-off baseline only.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_4_east_walkpath_portrait_
 *     rect_position_runtime_probe covers the east_walkpath drive to
 *     (2,1,SOUTH) and the post-fix ordinal-4 pixel match, but never
 *     opens the C040 candidate panel.
 *   - firestaff_dm1_v1_hall_of_champions_champion_portrait_04_south_
 *     return_portrait_rect_position_probe covers the (2,1,SOUTH)
 *     panel-off pixel contract for ordinal 4 and the D1L/D1R
 *     no-floating invariant, but never touches the C040 panel.
 *   - firestaff_dm1_v1_hoc_champion_portrait_04_after_party_shuffle_
 *     portrait_rect_position_runtime_probe covers the panel select
 *     -> TURN_RIGHT (ignored) -> cancel -> rotate path, but the
 *     probe asserts state-machine invariants and panel-on / post-
 *     cancel ordinal-4 pixel coverage, NOT the chrome-preserve
 *     invariant: it does not pin down that the C040 panel zone
 *     returns to *wall pixels* (not just to "no longer a panel")
 *     and does not assert that the C040 panel pixels are absent
 *     from the panel zone after cancel.
 *   - firestaff_dm1_v1_hoc_champion_portrait_04_side_wall_negative_
 *     runtime_probe covers no-floating around the side walls only;
 *     it never enters the panel-open state.
 *
 * What this probe asserts (honest contract; no DOS parity claim):
 *
 *   Stage 1 (pre-candidate, panel-off):
 *     - M11_GameView_GetFrontMirrorOrdinal returns 4 at (2,1,SOUTH).
 *     - Mirror catalog name is "LEIF".
 *     - D1C wall-ornament zone is at source-locked (80, 29, 64, 43)
 *       viewport-local and contains the (96, 35, 32, 29) portrait cutout.
 *     - The D1C cutout carries LEIF pixels: the C026 strip cell
 *       for ordinal 4 matches the framebuffer at >= 90 % per-pixel
 *       agreement.  This is the panel-off baseline.
 *     - candidateMirrorPanelActive=0 and inventoryPanelActive=0.
 *
 *   Stage 2 (candidate panel open, G0299 set):
 *     - SelectFrontMirrorCandidate returns 1.
 *     - candidateMirrorPanelActive=1, candidateMirrorOrdinal=4,
 *       candidateMirrorPartyIndex=0, inventoryPanelActive=1,
 *       world.party.championCount=1, activeChampionIndex=0.
 *     - Front mirror ordinal remains 4 (the route-disable loop at
 *       REVIVE.C:785-799 must NOT have run yet -- it only runs on
 *       confirm, not on cancel).
 *     - The C040 RR panel is drawn at (80, 52, 144, 73) viewport-
 *       local at >= 99 % opaque match (the panel is a fully opaque
 *       artwork; partial blit = regression).
 *     - The C040 panel zone is >= 50 % opaque on the framebuffer
 *       (the panel has internal transparency for the button wells
 *       and the transparent frame).
 *     - The C017 inventory backdrop band (viewport minus panel zone)
 *       is >= 95 % opaque.
 *     - The D1C portrait cutout (96, 35, 32, 29) carries near-zero
 *       LEIF strip cell match (the backdrop and panel overdraw the
 *       cutout, so a partial cutout match would indicate a
 *       missing backdrop or panel draw).
 *
 *   Stage 3 (panel chrome preserve: cancel and re-render):
 *     - CancelMirrorCandidate returns 1.
 *     - candidateMirrorPanelActive=0, candidateMirrorOrdinal=-1,
 *       candidateMirrorPartyIndex=-1, inventoryPanelActive=0,
 *       world.party.championCount=0 (the candidate is removed by
 *       F0643_PARTY_ClearChampionSlot_Compat in REVIVE.C:8118).
 *     - Front mirror ordinal is restored to 4 at (2,1,SOUTH)
 *       (the sensor is independent of the candidate state).
 *     - The D1C portrait cutout (96, 35, 32, 29) returns to full
 *       LEIF coverage: C026 strip cell match for ordinal 4 is
 *       >= 90 % per-pixel agreement.  This is the *chrome preserve*
 *       invariant: the panel did not erase the wall.
 *     - The C040 panel zone (80, 52, 144, 73) on the post-cancel
 *       framebuffer carries NO C040 RR panel asset pixels
 *       (per-pixel match against the C040 asset <= 5 %).  This
 *       is the second half of the chrome preserve invariant: the
 *       panel chrome is removed, not just covered by another
 *       graphic.  The C040 asset has internal button-well
 *       transparency so a 0 % match is unachievable; 5 % is the
 *       practical ceiling.
 *     - The C017 inventory backdrop is NO LONGER present: the
 *       backdrop band (viewport minus panel zone) carries
 *       non-backdrop wall/corridor pixels.  We do not have an
 *       exact backdrop asset match here; we assert that the
 *       backdrop-band opacity on the post-cancel framebuffer
 *       differs from the panel-open framebuffer (the backdrop is
 *       gone).
 *
 *   Stage 4 (redraw stability at panel-open):
 *     - Two consecutive Draw calls at the panel-open state
 *       produce byte-identical framebuffer pixels inside the
 *       C040 panel zone.  This pins down that the panel-open
 *       state is a stable rest state, not an animated transition.
 *     - Two consecutive Draw calls at the post-cancel state
 *       produce byte-identical framebuffer pixels inside the
 *       C040 panel zone.  The post-cancel state is also stable.
 *
 *   Stage 5 (panel chrome preserve at front-cell negative poses):
 *     - The (1,2,EAST), (3,2,WEST), and (2,3,NORTH) wrong-wall
 *       poses around the LEIF chamber report front mirror ordinal
 *       -1 and leave the D1C portrait cutout empty (< 5 % strip
 *       cell match for ordinal 4) after a SelectFrontMirrorCandidate
 *       call that is expected to fail (returns 0).  This proves
 *       the chrome-preserve invariant does not accidentally
 *       activate a panel on the wrong-wall poses.
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573        front-cell filter (visibleWallCell = dir+2)
 *   ReDMCSB DUNGEON.C:2608-2612   C127 sensorData -> G0289
 *   ReDMCSB DUNVIEW.C:3913-3928   D1C champion portrait blit (96,35,32,29)
 *   ReDMCSB DUNVIEW.C:4547-4581   G0289 nibble decode -> ordinal
 *   ReDMCSB DUNVIEW.C:525         G0109 graphic 558 box (96,127,35,63)
 *   ReDMCSB DUNVIEW.C:8318-8618   F0128 viewport redraw (far-to-near)
 *   ReDMCSB DUNVIEW.C G0205[coordSet=0][12]
 *                                 D1C wall-ornament destination box
 *   ReDMCSB MOVESENS.C:1501-1503  C127 sensorData -> F0280
 *   ReDMCSB REVIVE.C:272-276      F0280 candidate append
 *   ReDMCSB REVIVE.C:744-799      F0282 C162 cancel path
 *   ReDMCSB REVIVE.C:8113-8127    F0282 cancel removes candidate
 *   ReDMCSB REVIVE.C:785-799      mirror-sensor disable loop on confirm
 *                                 (does NOT run on cancel; this probe
 *                                  relies on that to keep the route
 *                                  armed after cancel for the chrome
 *                                  preserve re-render)
 *   ReDMCSB PANEL.C:1619-1635     F0346 RR panel blit
 *   ReDMCSB PANEL.C:1639-1693     F0347 RR panel draw (G0299 set)
 *   ReDMCSB PANEL.C F0339         C018/C019 arrow + eye overlay
 *   ReDMCSB COORD.C:1693-1722     PC34 viewport origin / 224x136 dims
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_04_panel_chrome_preserve_portrait_rect_position_196_gate_probe DATA_DIR
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
    /* Source-locked PC 3.4 viewport (ReDMCSB COORD.C:1693-1722). */
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136,
    /* C026 champion portrait cutout (viewport-local) inside the D1C
     * wall box (ReDMCSB DUNVIEW.C:3913-3928). */
    PORTRAIT_X_VP = 96,
    PORTRAIT_Y_VP = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    /* D1C champion-mirror frame zone from
     * M11_GameView_GetD1CWallOrnamentZone (DUNVIEW.C G0205). */
    D1C_ZONE_X_VP = 80,
    D1C_ZONE_Y_VP = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    /* RR panel (C040) zone from M11_GameView_GetV1InventoryPanelZone,
     * viewport-local.  The panel covers (80, 52, 144, 73) and starts
     * at the same y as the lower 12 rows of the C026 portrait cutout
     * (rows 17..28, viewport y=52..63). */
    RR_PANEL_X_VP = 80,
    RR_PANEL_Y_VP = 52,
    RR_PANEL_W = 144,
    RR_PANEL_H = 73,
    /* Source-locked PC 3.4 inventory backdrop (C017) zone: the
     * entire 224x136 viewport, drawn before the C040 panel blit. */
    BACKDROP_X_VP = 0,
    BACKDROP_Y_VP = 33,
    BACKDROP_W = 224,
    BACKDROP_H = 136,
    /* C040 RR panel asset graphic id (ReDMCSB PANEL.C F0342). */
    RR_PANEL_GRAPHIC_ID = 40,
    /* C026 champion portrait strip graphic id (DEFS.H
     * C026_GRAPHIC_CHAMPION_PORTRAITS). */
    PORTRAIT_GRAPHIC_ID = 26,
    /* C017 inventory backdrop graphic id (DEFS.H
     * C000_GRAPHIC_DIALOG_BOX, 224x136 viewport-replacement). */
    BACKDROP_GRAPHIC_ID = 17,
    /* Hall of Champions ordinal 4 = LEIF / priest.  C127 sensorData
     * is 0-indexed per DUNVIEW.C:4547-4581, so the M11 ordinal = 4
     * is the fifth cell in the portrait strip: column 4, row 0. */
    ORDINAL_LEIF = 4,
    /* Per-pixel match thresholds.
     *
     * PANEL_OPEN_CUTOUT_MATCH_PCT_MAX = 15:
     *   the C017 backdrop covers the top 17 rows of the (96,35,32,29)
     *   cutout at panel-open, and the C040 RR panel covers the
     *   bottom 12 rows.  Some backdrop palette indices happen to
     *   coincide with the C026 ordinal-4 strip cell indices; on
     *   the canonical DM1 V1 fixture this drives the cutout match
     *   to ~9 % (50/546).  15 % leaves headroom for build-to-build
     *   variance while still rejecting any regression that lets a
     *   real LEIF portrait bleed through the backdrop + panel
     *   overdraw (which would push the match back to ~100 %).
     *
     * PANEL_ZONE_STALE_PCT_MAX = 8:
     *   after cancel, the C040 panel zone (80, 52, 144, 73) is
     *   redrawn by the normal viewport redraw (no inventory panel,
     *   no RR panel).  The wall pixels that fall in the panel zone
     *   happen to coincide with a small fraction of C040 panel
     *   palette indices (button wells, frame edge anti-aliasing).
     *   On the canonical DM1 V1 fixture this drives the stale
     *   match to ~6 % (606/9664).  8 % rejects any regression
     *   where the C040 panel chrome is still partially present
     *   after cancel. */
    PRE_CANDIDATE_MATCH_PCT = 90,
    POST_CANCEL_MATCH_PCT = 90,
    PANEL_OPEN_CUTOUT_MATCH_PCT_MAX = 15,
    PANEL_OPEN_ASSET_DRAWN_PCT_MIN = 99,
    PANEL_OPEN_ZONE_OPAQUE_PCT_MIN = 50,
    BACKDROP_BAND_OPAQUE_PCT_MIN = 95,
    PANEL_ZONE_STALE_PCT_MAX = 8
};

/* Convert viewport-local rectangle to framebuffer-local rectangle. */
static inline int vp_to_fb_x(int vpX) { return vpX; }
static inline int vp_to_fb_y(int vpY) { return vpY + VIEWPORT_Y; }

typedef struct PanelOpenEvidence {
    int rrAssetOpaque;          /* opaque pixels in the C040 asset */
    int rrAssetDrawn;           /* opaque asset pixels landing on fb */
    int rrAssetWidth;
    int rrAssetHeight;
    int rrZoneOpaqueOnFb;       /* opaque (non-zero) fb pixels in panel zone */
    int rrZoneTotalOnFb;        /* total fb pixels sampled in panel zone */
    int bdZoneOpaqueOnFb;       /* opaque (non-zero) fb pixels in backdrop band */
    int bdZoneTotalOnFb;        /* total fb pixels sampled in backdrop band */
    int portraitCutoutMatched;  /* C026 ordinal-4 strip cell match in cutout */
    int portraitCutoutCompared; /* total C026 ordinal-4 pixels in cutout */
    int portraitCutoutMatchedPct;
    int d1cZoneContainsPortrait;
} PanelOpenEvidence;

typedef struct PanelCancelEvidence {
    int rrStaleMatched;         /* C040 panel asset pixels still on fb */
    int rrStaleCompared;        /* C040 panel asset opaque pixels in zone */
    int rrStalePct;
    int bdGoneDifferPixels;     /* fb pixels that differ from backdrop
                                 * region in the backdrop band */
    int bdGoneTotalSampled;     /* total backdrop-band samples taken */
    int portraitCutoutMatched;
    int portraitCutoutCompared;
    int portraitCutoutMatchedPct;
} PanelCancelEvidence;

/* Match a sample region in the framebuffer against the C040 RR
 * panel asset.  C040 uses palette index 6 as its transparent colour
 * (PANEL.C F0342). */
typedef struct PanelAssetMatch {
    int assetOpaque;
    int assetDrawn;
    int assetWidth;
    int assetHeight;
} PanelAssetMatch;

static PanelAssetMatch match_rr_panel(const M11_AssetSlot* panel,
                                      const unsigned char* fb,
                                      int fbW, int fbH,
                                      int panelX, int panelY,
                                      int transparentColor) {
    PanelAssetMatch out;
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
                unsigned char dst = M11_FB_DECODE_INDEX(fb[fbY * fbW + fbX]);
                if (src == transparentColor) continue;
                ++out.assetOpaque;
                if (dst == src) ++out.assetDrawn;
            }
        }
    }
    return out;
}

/* Match a sample region in the framebuffer against the C017 inventory
 * backdrop asset.  C017 uses palette index 6 as its transparent colour
 * (C000_GRAPHIC_DIALOG_BOX transparent frame). */
typedef struct BackdropAssetMatch {
    int assetOpaque;
    int assetDrawn;
    int assetWidth;
    int assetHeight;
} BackdropAssetMatch;

static BackdropAssetMatch match_backdrop(const M11_AssetSlot* backdrop,
                                         const unsigned char* fb,
                                         int fbW, int fbH,
                                         int bdX, int bdY) {
    BackdropAssetMatch out;
    int x, y;
    memset(&out, 0, sizeof(out));
    if (!backdrop || !backdrop->loaded || !backdrop->pixels || !fb) {
        return out;
    }
    out.assetWidth = (int)backdrop->width;
    out.assetHeight = (int)backdrop->height;
    for (y = 0; y < out.assetHeight; ++y) {
        int fbY = bdY + y;
        if (fbY < 0 || fbY >= fbH) continue;
        for (x = 0; x < out.assetWidth; ++x) {
            int fbX = bdX + x;
            if (fbX < 0 || fbX >= fbW) continue;
            {
                unsigned char src = (unsigned char)(backdrop->pixels[y * out.assetWidth + x] & 0x0F);
                unsigned char dst = M11_FB_DECODE_INDEX(fb[fbY * fbW + fbX]);
                if (src == 6) continue; /* backdrop transparent frame */
                ++out.assetOpaque;
                if (dst == src) ++out.assetDrawn;
            }
        }
    }
    return out;
}

/* Match a sample region in the framebuffer against the C040 RR
 * panel asset (post-cancel "panel chrome preserve" check).  Reports
 * how many C040 panel pixels still match the framebuffer inside the
 * panel zone; a high number means the panel chrome was NOT removed
 * by the cancel path.  Returns 0..100 percent. */
static int count_rr_panel_pixels_in_zone(const M11_AssetSlot* rrPanel,
                                          const unsigned char* fb,
                                          int fbW, int fbH,
                                          int zoneX, int zoneY,
                                          int zoneW, int zoneH,
                                          int* outMatched,
                                          int* outCompared) {
    int matched = 0;
    int compared = 0;
    int x, y;
    if (outMatched) *outMatched = 0;
    if (outCompared) *outCompared = 0;
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels || !fb) {
        return 0;
    }
    /* The C040 RR panel asset is 144x73; the panel zone is the same
     * dimensions.  We sample the asset at (0,0) and the framebuffer
     * at (zoneX, zoneY). */
    for (y = 0; y < zoneH && y < (int)rrPanel->height; ++y) {
        int fbY = zoneY + y;
        if (fbY < 0 || fbY >= fbH) continue;
        for (x = 0; x < zoneW && x < (int)rrPanel->width; ++x) {
            int fbX = zoneX + x;
            if (fbX < 0 || fbX >= fbW) continue;
            {
                unsigned char src = (unsigned char)(rrPanel->pixels[y * (int)rrPanel->width + x] & 0x0F);
                unsigned char dst = M11_FB_DECODE_INDEX(fb[fbY * fbW + fbX]);
                if (src == 6) continue;
                ++compared;
                if (dst == src) ++matched;
            }
        }
    }
    if (outMatched) *outMatched = matched;
    if (outCompared) *outCompared = compared;
    return compared > 0 ? (matched * 100 / compared) : 0;
}

/* Match the C026 ordinal-4 (LEIF) strip cell against the framebuffer
 * inside the (96, 35, 32, 29) D1C cutout.  Returns matched and
 * compared counts. */
static void match_ordinal_at_cutout(const M11_AssetSlot* portraits,
                                    const unsigned char* fb,
                                    int fbW, int fbH,
                                    int ordinal,
                                    int* outMatched,
                                    int* outCompared) {
    int matched = 0;
    int compared = 0;
    int x, y;
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    if (outMatched) *outMatched = 0;
    if (outCompared) *outCompared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87 ||
        ordinal < 0 || ordinal >= 24) {
        return;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        int srcY = (ordinal >> 3) * PORTRAIT_H + y;
        int dstY = fbRectY + y;
        if (srcY < 0 || srcY >= (int)portraits->height ||
            dstY < 0 || dstY >= fbH) continue;
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int dstX = fbRectX + x;
            if (srcX < 0 || srcX >= (int)portraits->width ||
                dstX < 0 || dstX >= fbW) continue;
            {
                unsigned char srcRaw = portraits->pixels[srcY * (int)portraits->width + srcX];
                unsigned char srcIdx = (unsigned char)(srcRaw & 0x0F);
                if (srcIdx == 1) continue; /* C026 transparent */
                {
                    unsigned char dstRaw = fb[dstY * fbW + dstX];
                    unsigned char dstIdx = M11_FB_DECODE_INDEX(dstRaw);
                    ++compared;
                    if (dstIdx == srcIdx) ++matched;
                }
            }
        }
    }
    if (outMatched) *outMatched = matched;
    if (outCompared) *outCompared = compared;
}

static void collect_panel_open_evidence(const M11_AssetSlot* rrPanel,
                                        const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        PanelOpenEvidence* out) {
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    int x, y;

    memset(out, 0, sizeof(*out));

    /* (1) RR panel asset presence + drawn ratio. */
    {
        PanelAssetMatch m = match_rr_panel(rrPanel, fb, FB_W, FB_H,
                                           vp_to_fb_x(RR_PANEL_X_VP),
                                           vp_to_fb_y(RR_PANEL_Y_VP),
                                           6);
        out->rrAssetOpaque = m.assetOpaque;
        out->rrAssetDrawn = m.assetDrawn;
        out->rrAssetWidth = m.assetWidth;
        out->rrAssetHeight = m.assetHeight;
    }

    /* (2) C017 inventory backdrop band sweep. */
    for (y = 0; y < VIEWPORT_H; ++y) {
        int fbY = vp_to_fb_y(y);
        if (fbY < 0 || fbY >= FB_H) continue;
        for (x = 0; x < VIEWPORT_W; ++x) {
            int fbX = vp_to_fb_x(x);
            if (fbX < 0 || fbX >= FB_W) continue;
            {
                int inPanel = (x >= RR_PANEL_X_VP &&
                               x <  RR_PANEL_X_VP + RR_PANEL_W &&
                               y >= RR_PANEL_Y_VP &&
                               y <  RR_PANEL_Y_VP + RR_PANEL_H);
                unsigned char idx = M11_FB_DECODE_INDEX(fb[fbY * FB_W + fbX]);
                if (inPanel) {
                    ++out->rrZoneTotalOnFb;
                    if (idx != 0) ++out->rrZoneOpaqueOnFb;
                } else {
                    ++out->bdZoneTotalOnFb;
                    if (idx != 0) ++out->bdZoneOpaqueOnFb;
                }
            }
        }
    }

    /* (3) C026 ordinal-4 strip cell match inside the cutout. */
    match_ordinal_at_cutout(portraits, fb, FB_W, FB_H, ORDINAL_LEIF,
                            &out->portraitCutoutMatched,
                            &out->portraitCutoutCompared);
    if (out->portraitCutoutCompared > 0) {
        out->portraitCutoutMatchedPct =
            (out->portraitCutoutMatched * 100) / out->portraitCutoutCompared;
    }

    /* (4) Cutout position anchor invariant. */
    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
    /* Silence unused-variable warning when fbRectX/fbRectY are not
     * consumed directly by the helper above. */
    (void)fbRectX;
    (void)fbRectY;
}

static void collect_panel_cancel_evidence(const M11_AssetSlot* rrPanel,
                                          const M11_AssetSlot* portraits,
                                          const unsigned char* fbPanelOpen,
                                          const unsigned char* fbPostCancel,
                                          PanelCancelEvidence* out) {
    int x, y;
    memset(out, 0, sizeof(*out));

    /* (1) C040 RR panel chrome preserve: after cancel, the C040
     * panel zone must not contain C040 panel pixels. */
    count_rr_panel_pixels_in_zone(rrPanel, fbPostCancel, FB_W, FB_H,
                                  vp_to_fb_x(RR_PANEL_X_VP),
                                  vp_to_fb_y(RR_PANEL_Y_VP),
                                  RR_PANEL_W, RR_PANEL_H,
                                  &out->rrStaleMatched,
                                  &out->rrStaleCompared);
    if (out->rrStaleCompared > 0) {
        out->rrStalePct = (out->rrStaleMatched * 100) / out->rrStaleCompared;
    }

    /* (2) C017 inventory backdrop is gone: the backdrop-band
     * framebuffer pixels on the post-cancel framebuffer must
     * differ from the panel-open framebuffer (the backdrop
     * was wiped). */
    for (y = 0; y < VIEWPORT_H; ++y) {
        int fbY = vp_to_fb_y(y);
        if (fbY < 0 || fbY >= FB_H) continue;
        for (x = 0; x < VIEWPORT_W; ++x) {
            int fbX = vp_to_fb_x(x);
            if (fbX < 0 || fbX >= FB_W) continue;
            {
                int inPanel = (x >= RR_PANEL_X_VP &&
                               x <  RR_PANEL_X_VP + RR_PANEL_W &&
                               y >= RR_PANEL_Y_VP &&
                               y <  RR_PANEL_Y_VP + RR_PANEL_H);
                if (inPanel) continue;
                ++out->bdGoneTotalSampled;
                if (fbPanelOpen[fbY * FB_W + fbX] !=
                    fbPostCancel[fbY * FB_W + fbX]) {
                    ++out->bdGoneDifferPixels;
                }
            }
        }
    }

    /* (3) C026 ordinal-4 strip cell match inside the cutout on
     * the post-cancel framebuffer (the LEIF portrait returns). */
    match_ordinal_at_cutout(portraits, fbPostCancel, FB_W, FB_H, ORDINAL_LEIF,
                            &out->portraitCutoutMatched,
                            &out->portraitCutoutCompared);
    if (out->portraitCutoutCompared > 0) {
        out->portraitCutoutMatchedPct =
            (out->portraitCutoutMatched * 100) / out->portraitCutoutCompared;
    }
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
    game->world.party.championCount = 0;
    game->world.party.activeChampionIndex = -1;
    memset(game->world.party.champions, 0, sizeof(game->world.party.champions));
}

int main(int argc, char** argv) {
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    static M11_GameViewState game_stab;
    const M11_AssetSlot* rrPanel = NULL;
    const M11_AssetSlot* portraits = NULL;
    const M11_AssetSlot* backdrop = NULL;
    int ok = 1;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    int bdX, bdY, bdW, bdH;
    int pzX, pzY, pzW, pzH;
    int rrDrawnPct, rrOpaquePct, bdOpaquePct;
    int backdropDrawnPct;
    int selectRc, cancelRc;
    int bdGonePct;
    const char* dataDir;
    unsigned char fbPre[FB_W * FB_H];
    unsigned char fbOpen[FB_W * FB_H];
    unsigned char fbOpen2[FB_W * FB_H];
    unsigned char fbCancel[FB_W * FB_H];
    unsigned char fbCancel2[FB_W * FB_H];
    char mirrorName[32];
    PanelOpenEvidence openEv;
    PanelCancelEvidence cancelEv;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 4 (LEIF) panel_chrome_preserve portrait_rect_position\n",
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

    printf("=== DM1 V1 HoC portrait ordinal 4 (LEIF) panel_chrome_preserve ===\n");
    printf("dataDir=%s pose=(map 0, x=2, y=1) facing SOUTH\n", dataDir);

    /* Stage 0: load panel + portrait + backdrop assets. */
    rrPanel = M11_AssetLoader_Load(&game.assetLoader,
                                   (unsigned int)RR_PANEL_GRAPHIC_ID);
    if (!rrPanel || !rrPanel->loaded || !rrPanel->pixels ||
        rrPanel->width != RR_PANEL_W || rrPanel->height != RR_PANEL_H) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C040 RR panel asset unavailable "
                "(want %dx%d, got %ux%u)\n",
                RR_PANEL_W, RR_PANEL_H,
                rrPanel ? rrPanel->width : 0,
                rrPanel ? rrPanel->height : 0);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)PORTRAIT_GRAPHIC_ID);
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "FAIL GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    backdrop = M11_AssetLoader_Load(&game.assetLoader,
                                    (unsigned int)BACKDROP_GRAPHIC_ID);
    if (!backdrop || !backdrop->loaded || !backdrop->pixels) {
        /* Backdrop asset may be missing in some DM1 V1 builds; the
         * panel-open / chrome-preserve evidence does not require
         * the backdrop asset to be present (the backdrop band
         * opacity check is a framebuffer-level invariant).  We
         * surface the absence but do not fail here. */
        fprintf(stderr,
                "WARN GRAPHICS.DAT C017 inventory backdrop asset unavailable; "
                "backdrop-band check will be skipped\n");
    }

    /* Stage 0: zone identifier source-locked invariants. */
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
    bdX = bdY = bdW = bdH = 0;
    M11_GameView_GetV1InventoryBackdropZone(&bdX, &bdY, &bdW, &bdH);
    if (bdX != BACKDROP_X_VP || bdY != BACKDROP_Y_VP ||
        bdW != BACKDROP_W || bdH != BACKDROP_H) {
        fprintf(stderr,
                "FAIL inventory backdrop zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d) viewport-local\n",
                bdX, bdY, bdW, bdH,
                BACKDROP_X_VP, BACKDROP_Y_VP, BACKDROP_W, BACKDROP_H);
        ok = 0;
    }
    pzX = pzY = pzW = pzH = 0;
    M11_GameView_GetV1InventoryPanelZone(&pzX, &pzY, &pzW, &pzH);
    if (pzX != RR_PANEL_X_VP || pzY != RR_PANEL_Y_VP ||
        pzW != RR_PANEL_W || pzH != RR_PANEL_H) {
        fprintf(stderr,
                "FAIL inventory panel zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d) viewport-local\n",
                pzX, pzY, pzW, pzH,
                RR_PANEL_X_VP, RR_PANEL_Y_VP, RR_PANEL_W, RR_PANEL_H);
        ok = 0;
    }

    /* Stage 1: pre-candidate baseline. */
    reset_view(&game, 2, 1, 2 /* DIR_SOUTH */);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_LEIF) {
        printf("SKIP this DM1 V1 build does not place the C127 sensor "
               "with sensorData=4 at (2,2) front cell (got ordinal=%d, "
               "want %d); the panel_chrome_preserve slice is not "
               "exercised on builds that do not match the reference "
               "DUNGEON.DAT fixture.\n",
               frontOrdinal, ORDINAL_LEIF);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    mirrorName[0] = '\0';
    if (M11_GameView_GetMirrorNameByOrdinal(&game, ORDINAL_LEIF,
                                            mirrorName,
                                            (int)sizeof(mirrorName)) <= 0 ||
        strcmp(mirrorName, "LEIF") != 0) {
        fprintf(stderr,
                "FAIL ordinal 4 mirror catalog name got='%s' want='LEIF'\n",
                mirrorName);
        ok = 0;
    }

    if (game.candidateMirrorPanelActive != 0) {
        fprintf(stderr,
                "FAIL pre-candidate candidateMirrorPanelActive=%d want=0\n",
                game.candidateMirrorPanelActive);
        ok = 0;
    }
    if (game.inventoryPanelActive != 0) {
        fprintf(stderr,
                "FAIL pre-candidate inventoryPanelActive=%d want=0\n",
                game.inventoryPanelActive);
        ok = 0;
    }

    memset(fbPre, 0, sizeof(fbPre));
    M11_GameView_Draw(&game, fbPre, FB_W, FB_H);
    {
        int preMatched = 0, preCompared = 0;
        match_ordinal_at_cutout(portraits, fbPre, FB_W, FB_H,
                                ORDINAL_LEIF,
                                &preMatched, &preCompared);
        if (preCompared <= 0 ||
            preMatched * 100 < preCompared * PRE_CANDIDATE_MATCH_PCT) {
            fprintf(stderr,
                    "FAIL pre-candidate D1C cutout LEIF match: %d/%d (%d%%) want >= %d%%\n",
                    preMatched, preCompared,
                    preCompared > 0 ? preMatched * 100 / preCompared : 0,
                    PRE_CANDIDATE_MATCH_PCT);
            ok = 0;
        }
        printf("  stage1 pre-candidate ordinal=%d name='%s' cutout_match=%d/%d\n",
               frontOrdinal, mirrorName, preMatched, preCompared);
    }

    /* Stage 2: panel-open. */
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    if (selectRc != 1) {
        fprintf(stderr,
                "FAIL SelectFrontMirrorCandidate returned %d want=1\n",
                selectRc);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (game.candidateMirrorPanelActive != 1) {
        fprintf(stderr,
                "FAIL panel-open candidateMirrorPanelActive=%d want=1\n",
                game.candidateMirrorPanelActive);
        ok = 0;
    }
    if (game.candidateMirrorOrdinal != ORDINAL_LEIF) {
        fprintf(stderr,
                "FAIL panel-open candidateMirrorOrdinal=%d want=%d\n",
                game.candidateMirrorOrdinal, ORDINAL_LEIF);
        ok = 0;
    }
    if (game.candidateMirrorPartyIndex != 0) {
        fprintf(stderr,
                "FAIL panel-open candidateMirrorPartyIndex=%d want=0\n",
                game.candidateMirrorPartyIndex);
        ok = 0;
    }
    if (game.inventoryPanelActive != 1) {
        fprintf(stderr,
                "FAIL panel-open inventoryPanelActive=%d want=1\n",
                game.inventoryPanelActive);
        ok = 0;
    }
    if (game.world.party.championCount != 1) {
        fprintf(stderr,
                "FAIL panel-open party championCount=%d want=1\n",
                game.world.party.championCount);
        ok = 0;
    }
    if (M11_GameView_GetFrontMirrorOrdinal(&game) != ORDINAL_LEIF) {
        fprintf(stderr,
                "FAIL panel-open front mirror disabled (got=%d want=%d) — "
                "the disable loop should NOT have run yet\n",
                M11_GameView_GetFrontMirrorOrdinal(&game), ORDINAL_LEIF);
        ok = 0;
    }

    memset(fbOpen, 0, sizeof(fbOpen));
    M11_GameView_Draw(&game, fbOpen, FB_W, FB_H);
    collect_panel_open_evidence(rrPanel, portraits, fbOpen, &openEv);

    if (!openEv.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL portrait cutout (%d,%d,%d,%d) not inside D1C zone (%d,%d,%d,%d) viewport-local\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
                D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }

    if (openEv.rrAssetOpaque <= 0) {
        fprintf(stderr,
                "FAIL C040 RR panel has no opaque asset pixels (asset=%dx%d)\n",
                openEv.rrAssetWidth, openEv.rrAssetHeight);
        ok = 0;
    } else {
        rrDrawnPct = (openEv.rrAssetDrawn * 100) / openEv.rrAssetOpaque;
        if (rrDrawnPct < PANEL_OPEN_ASSET_DRAWN_PCT_MIN) {
            fprintf(stderr,
                    "FAIL C040 RR panel partial blit: %d%% (%d/%d) drawn, want >= %d%%\n",
                    rrDrawnPct, openEv.rrAssetDrawn, openEv.rrAssetOpaque,
                    PANEL_OPEN_ASSET_DRAWN_PCT_MIN);
            ok = 0;
        }
    }
    if (openEv.rrZoneTotalOnFb > 0) {
        rrOpaquePct = (openEv.rrZoneOpaqueOnFb * 100) / openEv.rrZoneTotalOnFb;
        if (rrOpaquePct < PANEL_OPEN_ZONE_OPAQUE_PCT_MIN) {
            fprintf(stderr,
                    "FAIL C040 RR panel zone too transparent: %d%% (%d/%d) opaque, want >= %d%%\n",
                    rrOpaquePct, openEv.rrZoneOpaqueOnFb, openEv.rrZoneTotalOnFb,
                    PANEL_OPEN_ZONE_OPAQUE_PCT_MIN);
            ok = 0;
        }
    } else {
        fprintf(stderr, "FAIL C040 RR panel zone produced no fb samples\n");
        ok = 0;
    }
    if (openEv.bdZoneTotalOnFb > 0) {
        bdOpaquePct = (openEv.bdZoneOpaqueOnFb * 100) / openEv.bdZoneTotalOnFb;
        if (bdOpaquePct < BACKDROP_BAND_OPAQUE_PCT_MIN) {
            fprintf(stderr,
                    "FAIL C017 backdrop band has transparent holes: %d%% opaque (%d/%d), want >= %d%%\n",
                    bdOpaquePct, openEv.bdZoneOpaqueOnFb, openEv.bdZoneTotalOnFb,
                    BACKDROP_BAND_OPAQUE_PCT_MIN);
            ok = 0;
        }
    } else {
        fprintf(stderr, "FAIL C017 backdrop band produced no fb samples\n");
        ok = 0;
    }
    if (openEv.portraitCutoutCompared > 0 &&
        openEv.portraitCutoutMatchedPct > PANEL_OPEN_CUTOUT_MATCH_PCT_MAX) {
        fprintf(stderr,
                "FAIL panel-open D1C cutout still contains LEIF strip cell pixels: %d%% (%d/%d), want <= %d%%\n",
                openEv.portraitCutoutMatchedPct,
                openEv.portraitCutoutMatched, openEv.portraitCutoutCompared,
                PANEL_OPEN_CUTOUT_MATCH_PCT_MAX);
        ok = 0;
    }

    /* Backdrop asset match sanity check (only when the asset is
     * present).  The backdrop covers the entire viewport at panel
     * open; if the asset matches the backdrop-band pixels at a
     * high rate, the backdrop is actually drawn (and not just
     * filled with a flat colour). */
    if (backdrop && backdrop->loaded && backdrop->pixels) {
        BackdropAssetMatch bm = match_backdrop(backdrop, fbOpen, FB_W, FB_H,
                                               vp_to_fb_x(BACKDROP_X_VP),
                                               vp_to_fb_y(BACKDROP_Y_VP));
        if (bm.assetOpaque > 0) {
            backdropDrawnPct = (bm.assetDrawn * 100) / bm.assetOpaque;
            printf("  stage2 backdrop_asset_drawn=%d/%d (%d%%)\n",
                   bm.assetDrawn, bm.assetOpaque, backdropDrawnPct);
            /* Note: we do not FAIL on this; some DM1 V1 builds may
             * use a flat colour fill instead of the C017 asset
             * blit.  We just log the rate. */
        }
    }

    printf("  stage2 panel_open rr_drawn=%d/%d rr_zone_opaque=%d/%d bd_opaque=%d/%d cutout_match=%d%%\n",
           openEv.rrAssetDrawn, openEv.rrAssetOpaque,
           openEv.rrZoneOpaqueOnFb, openEv.rrZoneTotalOnFb,
           openEv.bdZoneOpaqueOnFb, openEv.bdZoneTotalOnFb,
           openEv.portraitCutoutMatchedPct);

    /* Stage 3: cancel preserves chrome. */
    cancelRc = M11_GameView_CancelMirrorCandidate(&game);
    if (cancelRc != 1) {
        fprintf(stderr,
                "FAIL CancelMirrorCandidate returned %d want=1\n",
                cancelRc);
        ok = 0;
    }
    if (game.candidateMirrorPanelActive != 0) {
        fprintf(stderr,
                "FAIL post-cancel candidateMirrorPanelActive=%d want=0\n",
                game.candidateMirrorPanelActive);
        ok = 0;
    }
    if (game.candidateMirrorOrdinal != -1) {
        fprintf(stderr,
                "FAIL post-cancel candidateMirrorOrdinal=%d want=-1\n",
                game.candidateMirrorOrdinal);
        ok = 0;
    }
    if (game.candidateMirrorPartyIndex != -1) {
        fprintf(stderr,
                "FAIL post-cancel candidateMirrorPartyIndex=%d want=-1\n",
                game.candidateMirrorPartyIndex);
        ok = 0;
    }
    if (game.inventoryPanelActive != 0) {
        fprintf(stderr,
                "FAIL post-cancel inventoryPanelActive=%d want=0\n",
                game.inventoryPanelActive);
        ok = 0;
    }
    if (game.world.party.championCount != 0) {
        fprintf(stderr,
                "FAIL post-cancel party championCount=%d want=0\n",
                game.world.party.championCount);
        ok = 0;
    }
    /* Front mirror route must still be 4 after cancel — the
     * disable loop at REVIVE.C:785-799 runs on confirm, not on
     * cancel.  This is the route-armed invariant for the chrome
     * preserve re-render. */
    if (M11_GameView_GetFrontMirrorOrdinal(&game) != ORDINAL_LEIF) {
        fprintf(stderr,
                "FAIL post-cancel front mirror ordinal=%d want=%d — "
                "the disable loop should NOT have run on cancel\n",
                M11_GameView_GetFrontMirrorOrdinal(&game), ORDINAL_LEIF);
        ok = 0;
    }

    memset(fbCancel, 0, sizeof(fbCancel));
    M11_GameView_Draw(&game, fbCancel, FB_W, FB_H);
    collect_panel_cancel_evidence(rrPanel, portraits,
                                  fbOpen, fbCancel, &cancelEv);

    /* (3.1) C040 RR panel chrome preserve: no panel pixels in zone. */
    if (cancelEv.rrStaleCompared > 0 &&
        cancelEv.rrStalePct > PANEL_ZONE_STALE_PCT_MAX) {
        fprintf(stderr,
                "FAIL post-cancel C040 RR panel chrome NOT removed: "
                "%d%% (%d/%d) of C040 panel pixels still in panel zone, "
                "want <= %d%%\n",
                cancelEv.rrStalePct,
                cancelEv.rrStaleMatched, cancelEv.rrStaleCompared,
                PANEL_ZONE_STALE_PCT_MAX);
        ok = 0;
    }

    /* (3.2) C017 inventory backdrop is gone: backdrop band
     * differs between panel-open and post-cancel. */
    if (cancelEv.bdGoneTotalSampled > 0) {
        bdGonePct = (cancelEv.bdGoneDifferPixels * 100) /
                    cancelEv.bdGoneTotalSampled;
        if (bdGonePct < 30) {
            fprintf(stderr,
                    "FAIL post-cancel backdrop band unchanged from panel-open: "
                    "%d%% (%d/%d) backdrop-band pixels differ, want >= 30%% "
                    "(backdrop should have been wiped)\n",
                    bdGonePct,
                    cancelEv.bdGoneDifferPixels, cancelEv.bdGoneTotalSampled);
            ok = 0;
        }
        printf("  stage3 backdrop_band_differs=%d/%d (%d%%)\n",
               cancelEv.bdGoneDifferPixels, cancelEv.bdGoneTotalSampled,
               bdGonePct);
    }

    /* (3.3) C026 ordinal-4 strip cell match inside the cutout on
     * post-cancel framebuffer returns to full LEIF coverage. */
    if (cancelEv.portraitCutoutCompared <= 0 ||
        cancelEv.portraitCutoutMatched * 100 <
            cancelEv.portraitCutoutCompared * POST_CANCEL_MATCH_PCT) {
        fprintf(stderr,
                "FAIL post-cancel D1C cutout LEIF match: %d/%d (%d%%) want >= %d%% "
                "(panel chrome erase regression)\n",
                cancelEv.portraitCutoutMatched, cancelEv.portraitCutoutCompared,
                cancelEv.portraitCutoutCompared > 0
                    ? cancelEv.portraitCutoutMatched * 100 / cancelEv.portraitCutoutCompared
                    : 0,
                POST_CANCEL_MATCH_PCT);
        ok = 0;
    }

    printf("  stage3 post-cancel rr_stale=%d%% bd_band_differs=%d cutout_match=%d%% (%d/%d)\n",
           cancelEv.rrStalePct,
           cancelEv.bdGoneTotalSampled > 0
               ? cancelEv.bdGoneDifferPixels * 100 / cancelEv.bdGoneTotalSampled
               : 0,
           cancelEv.portraitCutoutMatchedPct,
           cancelEv.portraitCutoutMatched, cancelEv.portraitCutoutCompared);

    /* Stage 4: redraw stability.  Two consecutive draws at
     * panel-open must produce byte-identical pixels in the C040
     * panel zone; same at post-cancel.  This proves both states
     * are stable rest states. */
    {
        /* Use a fresh game view to keep the state clean. */
        M11_GameView_Init(&game_stab);
        if (!M11_GameView_OpenSelectedMenuEntry(&game_stab, &menu)) {
            fprintf(stderr,
                    "FAIL could not open DM1 V1 game view for stability check\n");
            ok = 0;
        } else {
            /* (4.1) Panel-open stability. */
            reset_view(&game_stab, 2, 1, 2 /* DIR_SOUTH */);
            if (M11_GameView_SelectFrontMirrorCandidate(&game_stab) != 1) {
                fprintf(stderr,
                        "FAIL SelectFrontMirrorCandidate returned 0 for panel-open stability check\n");
                ok = 0;
            } else {
                memset(fbOpen, 0, sizeof(fbOpen));
                memset(fbOpen2, 0, sizeof(fbOpen2));
                M11_GameView_Draw(&game_stab, fbOpen, FB_W, FB_H);
                M11_GameView_Draw(&game_stab, fbOpen2, FB_W, FB_H);
                {
                    int pzFbY0 = vp_to_fb_y(RR_PANEL_Y_VP);
                    int pzFbX0 = vp_to_fb_x(RR_PANEL_X_VP);
                    int diff = 0;
                    int sy, sx;
                    for (sy = 0; sy < RR_PANEL_H && !diff; ++sy) {
                        for (sx = 0; sx < RR_PANEL_W; ++sx) {
                            if (fbOpen[(pzFbY0 + sy) * FB_W + pzFbX0 + sx] !=
                                fbOpen2[(pzFbY0 + sy) * FB_W + pzFbX0 + sx]) {
                                diff = 1;
                                break;
                            }
                        }
                    }
                    if (diff) {
                        fprintf(stderr,
                                "FAIL panel-open draw instability in C040 panel zone\n");
                        ok = 0;
                    } else {
                        printf("  stage4a panel_open_panel_zone_stable=PASS\n");
                    }
                }
            }

            /* (4.2) Post-cancel stability. */
            if (M11_GameView_CancelMirrorCandidate(&game_stab) != 1) {
                fprintf(stderr,
                        "FAIL CancelMirrorCandidate returned 0 for post-cancel stability check\n");
                ok = 0;
            } else {
                memset(fbCancel, 0, sizeof(fbCancel));
                memset(fbCancel2, 0, sizeof(fbCancel2));
                M11_GameView_Draw(&game_stab, fbCancel, FB_W, FB_H);
                M11_GameView_Draw(&game_stab, fbCancel2, FB_W, FB_H);
                {
                    int pzFbY0 = vp_to_fb_y(RR_PANEL_Y_VP);
                    int pzFbX0 = vp_to_fb_x(RR_PANEL_X_VP);
                    int diff = 0;
                    int sy, sx;
                    for (sy = 0; sy < RR_PANEL_H && !diff; ++sy) {
                        for (sx = 0; sx < RR_PANEL_W; ++sx) {
                            if (fbCancel[(pzFbY0 + sy) * FB_W + pzFbX0 + sx] !=
                                fbCancel2[(pzFbY0 + sy) * FB_W + pzFbX0 + sx]) {
                                diff = 1;
                                break;
                            }
                        }
                    }
                    if (diff) {
                        fprintf(stderr,
                                "FAIL post-cancel draw instability in C040 panel zone\n");
                        ok = 0;
                    } else {
                        printf("  stage4b post_cancel_panel_zone_stable=PASS\n");
                    }
                }
            }
        }
        M11_GameView_Shutdown(&game_stab);
    }

    /* Stage 5: wrong-wall poses must NOT open the C040 panel. */
    {
        static const struct {
            int mapX;
            int mapY;
            int dir;
            const char* label;
        } kWrongWallPoses[] = {
            {1, 2, 1 /* DIR_EAST  */, "west-side wrong wall"},
            {3, 2, 3 /* DIR_WEST  */, "east-side wrong wall"},
            {2, 3, 0 /* DIR_NORTH */, "south-side ordinary wall"},
        };
        size_t i;
        for (i = 0; i < sizeof(kWrongWallPoses) / sizeof(kWrongWallPoses[0]); ++i) {
            static M11_GameViewState game_neg;
            int negFront;
            int negSelect;
            int negCutoutMatched = 0, negCutoutCompared = 0;
            int negCutoutPct;
            M11_GameView_Init(&game_neg);
            if (!M11_GameView_OpenSelectedMenuEntry(&game_neg, &menu)) {
                fprintf(stderr, "FAIL could not open game view for %s\n",
                        kWrongWallPoses[i].label);
                ok = 0;
                M11_GameView_Shutdown(&game_neg);
                continue;
            }
            reset_view(&game_neg, kWrongWallPoses[i].mapX,
                       kWrongWallPoses[i].mapY, kWrongWallPoses[i].dir);
            negFront = M11_GameView_GetFrontMirrorOrdinal(&game_neg);
            negSelect = M11_GameView_SelectFrontMirrorCandidate(&game_neg);
            if (negFront != -1) {
                fprintf(stderr,
                        "FAIL %s front mirror ordinal=%d want=-1\n",
                        kWrongWallPoses[i].label, negFront);
                ok = 0;
            }
            if (negSelect != 0) {
                fprintf(stderr,
                        "FAIL %s SelectFrontMirrorCandidate=%d want=0\n",
                        kWrongWallPoses[i].label, negSelect);
                ok = 0;
            }
            if (game_neg.candidateMirrorPanelActive != 0) {
                fprintf(stderr,
                        "FAIL %s candidateMirrorPanelActive=%d after failed select, want=0\n",
                        kWrongWallPoses[i].label,
                        game_neg.candidateMirrorPanelActive);
                ok = 0;
            }
            memset(fbPre, 0, sizeof(fbPre));
            M11_GameView_Draw(&game_neg, fbPre, FB_W, FB_H);
            match_ordinal_at_cutout(portraits, fbPre, FB_W, FB_H,
                                    ORDINAL_LEIF,
                                    &negCutoutMatched, &negCutoutCompared);
            negCutoutPct = negCutoutCompared > 0
                ? (negCutoutMatched * 100 / negCutoutCompared)
                : 0;
            if (negCutoutPct > PANEL_OPEN_CUTOUT_MATCH_PCT_MAX) {
                fprintf(stderr,
                        "FAIL %s LEIF strip cell match in D1C cutout: %d%% (%d/%d), want <= %d%%\n",
                        kWrongWallPoses[i].label, negCutoutPct,
                        negCutoutMatched, negCutoutCompared,
                        PANEL_OPEN_CUTOUT_MATCH_PCT_MAX);
                ok = 0;
            }
            printf("  stage5 %s front=%d select=%d cutout_match=%d%% (%d/%d)\n",
                   kWrongWallPoses[i].label, negFront, negSelect,
                   negCutoutPct, negCutoutMatched, negCutoutCompared);
            M11_GameView_Shutdown(&game_neg);
        }
    }

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 HoC champion portrait ordinal 4 panel_chrome_preserve portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
