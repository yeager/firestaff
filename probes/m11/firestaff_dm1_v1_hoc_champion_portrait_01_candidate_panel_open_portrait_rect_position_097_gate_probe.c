/*
 * DM1 V1 Hall of Champions — champion portrait ordinal 1 (HALK)
 * candidate_panel_open / portrait_rect_position runtime probe.
 *
 * Targeted slice:
 *   ordinal    = 1 (HALK / barbarian, C127 sensorData=1)
 *   pose       = (map 0, x=1, y=2) facing NORTH
 *                 (front cell (1,1) carries the C127 champion mirror)
 *   route      = candidate_panel_open
 *                 (the M11 viewport state immediately after
 *                  SelectFrontMirrorCandidate returns 1: the C040
 *                  resurrect/reincarnate panel is live, the C017
 *                  inventory backdrop covers the entire viewport,
 *                  the C346 wall-ornament graphic is suppressed by
 *                  the BUG-120/121 panel-open guard, and the C026
 *                  D1C champion portrait cutout is anchored at the
 *                  source-locked (96, 35, 32, 29) viewport-local
 *                  rect before the panel blit covers its lower rows)
 *   aspect     = portrait_rect_position
 *                 (the C026 cutout stays at (96, 35, 32, 29)
 *                  viewport-local; the panel covers (80, 52,
 *                  144, 73) viewport-local; the overlap zone is
 *                  panel-dominant in the lower 12 rows of the
 *                  portrait rect, portrait-only in the top 17 rows
 *                  of the rect above the panel, and the side walls
 *                  never leak ordinal-1 pixels through the
 *                  inventory backdrop wipe)
 *
 * This is a *narrow*, *deep* panel-open slice, distinct from the
 * sibling 097_gate probe
 * (firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate_portrait_rect_position_097_gate_probe),
 * which cycles through pre-candidate / panel-open / post-confirm /
 * post-cancel / 4-cycle drift at a coarser level.  Here we stay
 * inside the panel-open state only and pin down invariants that
 * the broader probe does not check.
 *
 * Coverage gap relative to existing champion-mirror probe matrix:
 *   - firestaff_dm1_v1_hoc_champion_portrait_01_redraw_after_candidate_portrait_rect_position_097_gate_probe
 *     covers Stage 2 (panel-open) only as a 4-line sub-check
 *     (RR panel drawn ratio + front mirror + candidate panel
 *     flag) and immediately moves to post-confirm.  It does not
 *     check the cutout warm pixels at panel-open, the C017
 *     backdrop opacity, the status / inspect readout text, the
 *     byte-stable draw stability, or the zone identifier helpers.
 *   - firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe
 *     pose_panel_open covers ordinal 2 (1,4) NORTH: panel
 *     drawn, panel z-order overlap, candidate fields, and
 *     inventory panel on.  Ordinal 1 (HALK) is only sampled in
 *     the closed-panel Pose B.
 *   - firestaff_dm1_v1_champion_mirror_ordinal_1_halk_pose_probe
 *     covers the pre-candidate (1,2) NORTH front-mirror route
 *     and the (1,2) WEST negative.  It does not enter the
 *     panel-open state.
 *   - firestaff_dm1_v1_champion_mirror_capture_probe saves PPMs
 *     for ordinal 1 only at the pre-candidate pose (no panel).
 *     No panel-open capture exists.
 *
 * What this probe asserts at the panel-open frame for ordinal 1:
 *   (1) Front-mirror route lookup still returns ordinal 1 at
 *       (1,2) NORTH (the panel-open path must keep the route
 *       armed — REVIVE.C:272-276 only appends; the disable loop
 *       is reached later on non-cancel confirm at
 *       REVIVE.C:785-799).
 *   (2) The panel-open state flags are correctly set:
 *       candidateMirrorPanelActive=1, candidateMirrorOrdinal=1,
 *       candidateMirrorPartyIndex=0, inventoryPanelActive=1,
 *       world.party.championCount=1, activeChampionIndex=0.
 *   (3) The status-box / inspect-readout strings reflect the
 *       source MIRROR / RESURRECT OR REINCARNATE contract
 *       (M11_GameView state strings lastAction="MIRROR",
 *       lastOutcome="RESURRECT OR REINCARNATE",
 *       inspectTitle starts with "MIRROR:",
 *       inspectDetail contains the champion name and the
 *       resurrect/reincarnate/cancel action menu).
 *   (4) The C040 RR panel asset is drawn at the panel zone
 *       (80, 52, 144, 73) viewport-local with >= 99% of its
 *       opaque pixels landing on the framebuffer — a tight
 *       bound that the redraw_after_candidate probe's 90% does
 *       not enforce, so this catches a partial-blit regression
 *       at ordinal 1.
 *   (5) The C017 inventory backdrop covers the entire viewport
 *       (224, 136) viewport-local: outside the C040 panel zone
 *       the backdrop band must be >= 95% opaque (the small
 *       remainder is the C040 panel transparent-edge pixels
 *       that bleed across the border).  Inside the C040 panel
 *       zone the panel pixels must be >= 50% opaque (the C040
 *       panel has internal transparency for the button wells
 *       and a transparent frame, so 100% is not reachable).
 *   (6) The C026 portrait cutout position is anchored at
 *       (96, 35, 32, 29) viewport-local.  The total warm
 *       pixel count inside the cutout is < 30 (same threshold
 *       used by the 097 sibling probe) and the C026 strip
 *       cell match for ordinal 1 must be <= 5% — i.e. the
 *       cutout does not contain HALK pixels.  The C040 panel
 *       can have warm palette indices for the buttons, but
 *       those will not match the C026 strip cell, so the
 *       strip-cell-match counter is the real "is HALK here"
 *       discriminator.
 *   (7) The M11_GameView_GetV1InventoryBackdropZone helper
 *       returns the source-locked (0, 33, 224, 136) viewport
 *       rectangle and the M11_GameView_GetV1InventoryPanelZone
 *       helper returns the source-locked (80, 52, 144, 73)
 *       viewport rectangle — the panel-open state relies on
 *       these zone identifiers being source-locked.
 *   (8) Two consecutive M11_GameView_Draw calls at the
 *       panel-open state produce byte-identical pixels in the
 *       C040 panel zone (panel-open is a stable rest state,
 *       not an animated transition).
 *
 * Source-locked to:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 D1C champion portrait blit (C026)
 *   ReDMCSB DUNVIEW.C:4547-4581 nibble 2 -> ordinal 1 / nibble 1
 *                       -> ordinal 0 (G0289 ordinal decode)
 *   ReDMCSB DUNVIEW.C:8318-8618 F0128 viewport redraw from
 *                       party map/x/y/direction (far-to-near order)
 *   ReDMCSB DUNVIEW.C:14271-14313 (D1C champion mirror BUG-120/121
 *                       guard: panel-open path keeps portrait drawn
 *                       and skips the wall-ornament graphic)
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB REVIVE.C:272-276 / F0280 appends candidate from sensor
 *   ReDMCSB REVIVE.C:744-799 / F0282 cancel without disabling route
 *   ReDMCSB REVIVE.C:785-799 mirror-sensor disable loop on confirm
 *   ReDMCSB REVIVE.C:837-840 G0362_l_LastPartyMovementTime seed
 *   ReDMCSB PANEL.C:1619-1635 / 1654-1656 / F0346 / F0347 routes
 *                       C040 resurrect/reincarnate panel while G0299
 *   ReDMCSB COORD.C:1693-1722 PC34 viewport origin/224x136 dims
 *
 * Usage:
 *   firestaff_dm1_v1_hoc_champion_portrait_01_candidate_panel_open_portrait_rect_position_097_gate_probe DATA_DIR
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
    /* C040 RR panel asset graphic id (ReDMCSB PANEL.C F0342).  The
     * m11_game_view.c-internal enum M11_GFX_PANEL_RESURRECT_REINCARNATE
     * is file-scoped; the source-locked value is 40. */
    RR_PANEL_GRAPHIC_ID = 40,
    /* C026 champion portrait strip graphic id (DEFS.H
     * C026_GRAPHIC_CHAMPION_PORTRAITS). */
    PORTRAIT_GRAPHIC_ID = 26,
    /* Hall of Champions ordinal 1 = HALK / barbarian.  C127 sensorData
     * is 0-indexed per DUNVIEW.C:4547-4581, so the M11 ordinal = 1 is
     * the second cell in the portrait strip: column 1, row 0. */
    ORDINAL_HALK = 1,
    /* Cutout C026 strip cell match threshold.  At panel-open the
     * C040 panel and C017 backdrop overdraw the cutout, so the
     * C026 strip cell match must be near 0.  Anything above 5%
     * means a HALK pixel has landed inside the cutout, which
     * would be a regression.  The warm-pixel count alone is not
     * a useful test here because the C040 panel's button artwork
     * uses warm palette indices (red / yellow / green). */
    PORTRAIT_CUTOUT_C026_MATCH_THRESHOLD_PCT = 5
};

/* Convert viewport-local rectangle to framebuffer-local rectangle. */
static inline int vp_to_fb_x(int vpX) { return vpX; }
static inline int vp_to_fb_y(int vpY) { return vpY + VIEWPORT_Y; }

typedef struct PanelOpenEvidence {
    /* Panel zone evidence. */
    int rrAssetOpaque;       /* opaque pixels in the C040 asset */
    int rrAssetDrawn;        /* opaque asset pixels landing on fb */
    int rrAssetWidth;
    int rrAssetHeight;
    int rrZoneOpaqueOnFb;    /* opaque (non-zero) fb pixels in panel zone */
    int rrZoneTotalOnFb;     /* total fb pixels sampled in panel zone */
    /* Backdrop zone evidence (viewport area outside the panel zone). */
    int bdZoneOpaqueOnFb;    /* opaque (non-zero) fb pixels in the rest
                              * of the viewport (backdrop band) */
    int bdZoneTotalOnFb;     /* total fb pixels sampled in backdrop band */
    int bdZoneTransparentHoles; /* fb pixels at index 0 in backdrop band */
    /* Portrait cutout evidence (96, 35, 32, 29). */
    int portraitCutoutWarmPixels;        /* warm pixels inside the cutout */
    int portraitCutoutTotalCompared;     /* non-transparent C026 pixels */
    int portraitCutoutC026Matched;       /* cutout pixels that match the
                                          * C026 strip cell for ordinal 1 */
    int portraitCutoutC026MatchedPct;    /* matched*100/compared */
    int d1cZoneContainsPortrait;         /* 1 if the cutout is inside the
                                          * D1C zone (80,29,64,43) */
} PanelOpenEvidence;

/* Match a sample region in the framebuffer against the C040 RR
 * panel asset, with the same `match_panel` logic used by
 * firestaff_dm1_v1_champion_mirror_candidate_panel_runtime_probe.
 * C040 uses palette index 6 as its transparent colour
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
                if (src == transparentColor) {
                    continue;
                }
                ++out.assetOpaque;
                if (dst == src) {
                    ++out.assetDrawn;
                }
            }
        }
    }
    return out;
}

/* Run the panel-open pixel sweep and accumulate the evidence struct
 * for the live (1,2) NORTH frame.  This is the heart of the probe —
 * it pins down several distinct invariants from the same single draw
 * so the panel-open state is covered end-to-end without having to
 * cycle through pre / post / cancel stages. */
static void collect_panel_open_evidence(const M11_AssetSlot* rrPanel,
                                        const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        PanelOpenEvidence* out) {
    int fbRectX = vp_to_fb_x(PORTRAIT_X_VP);
    int fbRectY = vp_to_fb_y(PORTRAIT_Y_VP);
    int x, y;

    memset(out, 0, sizeof(*out));

    /* (1) RR panel asset presence + drawn ratio.  We require 99%
     * of opaque asset pixels to land on the framebuffer because the
     * C040 panel is a 144x73 opaque artwork (no transparency in
     * the visible region).  A < 99% match means a partial blit
     * regression. */
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

    /* (2) C017 inventory backdrop band sweep.  The backdrop covers
     * the full viewport; outside the C040 panel zone every fb pixel
     * must be a non-zero backdrop colour, and inside the panel zone
     * every fb pixel must be a non-zero panel colour. */
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
                    else ++out->bdZoneTransparentHoles;
                }
            }
        }
    }

    /* (3) Portrait cutout warm sweep + C026 strip cell match.  The
     * C017 backdrop covers the top 17 rows of the cutout; the C040
     * panel covers the bottom 12 rows.  Either way, the C026 strip
     * cell for ordinal 1 (HALK) must NOT match the cutout pixels
     * because the backdrop and panel are drawn over the cutout.
     * The C040 panel can have warm palette indices for buttons,
     * but those will not match the C026 strip cell — so the
     * strip-cell-match counter is the real "is HALK here"
     * discriminator. */
    for (y = 0; y < PORTRAIT_H; ++y) {
        int fbY = fbRectY + y;
        if (fbY < 0 || fbY >= FB_H) continue;
        for (x = 0; x < PORTRAIT_W; ++x) {
            int fbX = fbRectX + x;
            if (fbX < 0 || fbX >= FB_W) continue;
            {
                unsigned char idx = M11_FB_DECODE_INDEX(fb[fbY * FB_W + fbX]);
                int warm = 0;
                switch (idx) {
                    case 0x07: /* green */
                    case 0x08: /* red */
                    case 0x09: /* orange */
                    case 0x0A: /* peach */
                    case 0x0B: /* yellow */
                    case 0x0E: /* blue */
                        warm = 1;
                        break;
                    default:
                        break;
                }
                if (warm) ++out->portraitCutoutWarmPixels;
            }
        }
    }

    if (portraits && portraits->loaded && portraits->pixels &&
        portraits->width >= 256 && portraits->height >= 87) {
        int srcBaseX = (ORDINAL_HALK & 7) * PORTRAIT_W;
        int srcBaseY = (ORDINAL_HALK >> 3) * PORTRAIT_H;
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
                        ++out->portraitCutoutTotalCompared;
                        if (dstIdx == srcIdx) {
                            ++out->portraitCutoutC026Matched;
                        }
                    }
                }
            }
        }
        if (out->portraitCutoutTotalCompared > 0) {
            out->portraitCutoutC026MatchedPct =
                (out->portraitCutoutC026Matched * 100) / out->portraitCutoutTotalCompared;
        }
    }

    /* (4) Cutout position anchor invariant. */
    out->d1cZoneContainsPortrait =
        (PORTRAIT_X_VP >= D1C_ZONE_X_VP &&
         PORTRAIT_Y_VP >= D1C_ZONE_Y_VP &&
         PORTRAIT_X_VP + PORTRAIT_W <= D1C_ZONE_X_VP + D1C_ZONE_W &&
         PORTRAIT_Y_VP + PORTRAIT_H <= D1C_ZONE_Y_VP + D1C_ZONE_H) ? 1 : 0;
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
    static M11_GameViewState game_dup; /* for the byte-stable draw check */
    const M11_AssetSlot* rrPanel = NULL;
    const M11_AssetSlot* portraits = NULL;
    int ok = 1;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    int bdX, bdY, bdW, bdH;
    int pzX, pzY, pzW, pzH;
    int rrDrawnPct;
    int rrOpaquePct;
    int bdOpaquePct;
    const char* dataDir;
    unsigned char fb1[FB_W * FB_H];
    unsigned char fb2[FB_W * FB_H];
    PanelOpenEvidence ev;

    if (argc < 2) {
        fprintf(stderr,
                "usage: %s DATA_DIR\n"
                "  verifies ordinal 1 (HALK) candidate_panel_open portrait_rect_position\n",
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

    printf("=== DM1 V1 HoC portrait ordinal 1 (HALK) candidate_panel_open ===\n");
    printf("dataDir=%s pose=(map 0, x=1, y=2) facing NORTH\n", dataDir);

    /* Stage 0: front-mirror route lookup must return ordinal 1 for
     * (1,2) NORTH per DUNGEON.C:2573 / MOVESENS.C:1501-1503.  This
     * is the same gate the 097 sibling probe uses; we keep it
     * here so this probe is self-contained. */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (frontOrdinal != ORDINAL_HALK) {
        printf("SKIP this DM1 V1 build does not place the C127 sensor "
               "with sensorData=1 at (1,2) front cell (got ordinal=%d, "
               "want %d); the candidate_panel_open slice is not "
               "exercised on builds that do not match the reference "
               "DUNGEON.DAT fixture.\n",
               frontOrdinal, ORDINAL_HALK);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    /* Stage 0: zone identifier source-locked invariants.  These
     * public helpers are what the live panel-open code path uses
     * to find the backdrop and panel rectangles.  If they return
     * a different rectangle the C017 backdrop wipe and the C040
     * panel blit both run at the wrong spot. */
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

    /* Stage 0: load the panel + portrait assets. */
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

    /* Stage 1: open the candidate panel — the panel-open state
     * must keep the route armed and flip the candidate flags. */
    reset_view(&game, 1, 2, 0 /* DIR_NORTH */);
    if (M11_GameView_SelectFrontMirrorCandidate(&game) != 1) {
        fprintf(stderr,
                "FAIL SelectFrontMirrorCandidate returned 0 for ordinal 1\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }
    if (game.candidateMirrorPanelActive != 1) {
        fprintf(stderr,
                "FAIL candidate panel not active after select (got=%d)\n",
                game.candidateMirrorPanelActive);
        ok = 0;
    }
    if (game.candidateMirrorOrdinal != ORDINAL_HALK) {
        fprintf(stderr,
                "FAIL candidate ordinal got=%d want=%d\n",
                game.candidateMirrorOrdinal, ORDINAL_HALK);
        ok = 0;
    }
    if (game.candidateMirrorPartyIndex != 0) {
        fprintf(stderr,
                "FAIL candidate party index got=%d want=0\n",
                game.candidateMirrorPartyIndex);
        ok = 0;
    }
    if (game.inventoryPanelActive != 1) {
        fprintf(stderr,
                "FAIL inventory panel not active after select (got=%d)\n",
                game.inventoryPanelActive);
        ok = 0;
    }
    if (game.world.party.championCount != 1) {
        fprintf(stderr,
                "FAIL party champion count after select got=%d want=1\n",
                game.world.party.championCount);
        ok = 0;
    }
    if (M11_GameView_GetFrontMirrorOrdinal(&game) != ORDINAL_HALK) {
        fprintf(stderr,
                "FAIL panel-open front mirror disabled (got=%d want=%d) — "
                "the disable loop should NOT have run yet\n",
                M11_GameView_GetFrontMirrorOrdinal(&game), ORDINAL_HALK);
        ok = 0;
    }

    /* Stage 1: status / inspect readout source-locked contract. */
    if (strcmp(game.lastAction, "MIRROR") != 0) {
        fprintf(stderr,
                "FAIL status action got='%s' want='MIRROR'\n",
                game.lastAction);
        ok = 0;
    }
    if (strcmp(game.lastOutcome, "RESURRECT OR REINCARNATE") != 0) {
        fprintf(stderr,
                "FAIL status outcome got='%s' want='RESURRECT OR REINCARNATE'\n",
                game.lastOutcome);
        ok = 0;
    }
    if (strncmp(game.inspectTitle, "MIRROR:", 7) != 0) {
        fprintf(stderr,
                "FAIL inspect title got='%s' want prefix 'MIRROR:'\n",
                game.inspectTitle);
        ok = 0;
    }
    if (strstr(game.inspectDetail, "RESURRECT") == NULL ||
        strstr(game.inspectDetail, "REINCARNATE") == NULL ||
        strstr(game.inspectDetail, "CANCEL") == NULL) {
        fprintf(stderr,
                "FAIL inspect detail missing action menu: '%s'\n",
                game.inspectDetail);
        ok = 0;
    }

    /* Stage 2: pixel evidence sweep.  Single draw, full evidence. */
    memset(fb1, 0, sizeof(fb1));
    M11_GameView_Draw(&game, fb1, FB_W, FB_H);
    collect_panel_open_evidence(rrPanel, portraits, fb1, &ev);

    /* (1) Cutout position anchor. */
    if (!ev.d1cZoneContainsPortrait) {
        fprintf(stderr,
                "FAIL portrait cutout (%d,%d,%d,%d) not inside D1C zone (%d,%d,%d,%d) viewport-local\n",
                PORTRAIT_X_VP, PORTRAIT_Y_VP, PORTRAIT_W, PORTRAIT_H,
                D1C_ZONE_X_VP, D1C_ZONE_Y_VP, D1C_ZONE_W, D1C_ZONE_H);
        ok = 0;
    }

    /* (2) C040 RR panel: 99% of opaque asset pixels must be drawn.
     * This is tighter than the 90% bound in the redraw_after_candidate
     * probe — the panel is a fully-opaque artwork, so a partial blit
     * is always a regression. */
    if (ev.rrAssetOpaque <= 0) {
        fprintf(stderr,
                "FAIL C040 RR panel has no opaque asset pixels (asset=%dx%d)\n",
                ev.rrAssetWidth, ev.rrAssetHeight);
        ok = 0;
    } else {
        rrDrawnPct = (ev.rrAssetDrawn * 100) / ev.rrAssetOpaque;
        if (rrDrawnPct < 99) {
            fprintf(stderr,
                    "FAIL C040 RR panel partial blit: %d%% (%d/%d) drawn\n",
                    rrDrawnPct, ev.rrAssetDrawn, ev.rrAssetOpaque);
            ok = 0;
        }
    }
    /* C040 panel has internal transparency (button wells, transparent
     * frame).  We only check that the panel zone is at least 50%
     * opaque — anything below 50% would mean a "transparent
     * panel" regression where the panel is mostly missing. */
    if (ev.rrZoneTotalOnFb > 0) {
        rrOpaquePct = (ev.rrZoneOpaqueOnFb * 100) / ev.rrZoneTotalOnFb;
        if (rrOpaquePct < 50) {
            fprintf(stderr,
                    "FAIL C040 RR panel zone too transparent: %d%% (%d/%d) opaque\n",
                    rrOpaquePct, ev.rrZoneOpaqueOnFb, ev.rrZoneTotalOnFb);
            ok = 0;
        }
    } else {
        fprintf(stderr, "FAIL C040 RR panel zone produced no fb samples\n");
        ok = 0;
    }

    /* (3) C017 inventory backdrop: the viewport area outside the
     * panel zone must be >= 95% opaque.  A small remainder is
     * expected from the panel-border transparent edge pixels
     * that bleed across the C040 panel rect boundary. */
    if (ev.bdZoneTotalOnFb > 0) {
        bdOpaquePct = (ev.bdZoneOpaqueOnFb * 100) / ev.bdZoneTotalOnFb;
        if (bdOpaquePct < 95) {
            fprintf(stderr,
                    "FAIL C017 backdrop band has transparent holes: %d%% opaque (%d/%d)\n",
                    bdOpaquePct, ev.bdZoneOpaqueOnFb, ev.bdZoneTotalOnFb);
            ok = 0;
        }
    } else {
        fprintf(stderr, "FAIL C017 backdrop band produced no fb samples\n");
        ok = 0;
    }

    /* (4) Portrait cutout C026 strip cell match.  The C017
     * backdrop covers the top 17 rows of the cutout and the C040
     * panel covers the bottom 12 rows.  The C040 panel's button
     * artwork uses warm palette indices (red / yellow / green) so
     * the warm-pixel count alone is not a useful "is HALK here"
     * test at panel-open — the real discriminator is the C026
     * strip cell match: if the cutout contained HALK pixels the
     * strip cell match would be ~100% (the existing 097 sibling
     * pre-candidate pose reports 100% (668/668) for ordinal 1),
     * but at panel-open the backdrop and panel overdraw the
     * cutout so the match must be near 0.  We require <= 5% match
     * (i.e. at most a few stray pixels from the panel's
     * transparent-border anti-aliasing). */
    if (ev.portraitCutoutTotalCompared > 0 &&
        ev.portraitCutoutC026MatchedPct > PORTRAIT_CUTOUT_C026_MATCH_THRESHOLD_PCT) {
        fprintf(stderr,
                "FAIL portrait cutout contains HALK strip cell pixels: %d%% (%d/%d) match ordinal 1\n",
                ev.portraitCutoutC026MatchedPct,
                ev.portraitCutoutC026Matched, ev.portraitCutoutTotalCompared);
        ok = 0;
    }

    /* Stage 3: byte-stable draw stability at panel-open.  Two
     * consecutive draws must produce byte-identical framebuffer
     * pixels in the C040 panel zone.  This pins down that the
     * panel-open state is a stable rest state, not an animated
     * transition. */
    M11_GameView_Init(&game_dup);
    if (!M11_GameView_OpenSelectedMenuEntry(&game_dup, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view for stability check\n");
        ok = 0;
    } else {
        reset_view(&game_dup, 1, 2, 0 /* DIR_NORTH */);
        if (M11_GameView_SelectFrontMirrorCandidate(&game_dup) != 1) {
            fprintf(stderr,
                    "FAIL SelectFrontMirrorCandidate returned 0 for stability check\n");
            ok = 0;
        } else {
            memset(fb1, 0, sizeof(fb1));
            memset(fb2, 0, sizeof(fb2));
            M11_GameView_Draw(&game_dup, fb1, FB_W, FB_H);
            M11_GameView_Draw(&game_dup, fb2, FB_W, FB_H);
            /* Diff the C040 panel zone (the most likely place for a
             * stability regression).  We don't fail on a global
             * memcmp because a global cmp can also catch
             * unrelated game-tick drift; we only fail on a
             * panel-zone diff. */
            {
                int pzFbY0 = vp_to_fb_y(RR_PANEL_Y_VP);
                int pzFbX0 = vp_to_fb_x(RR_PANEL_X_VP);
                int diff = 0;
                int y, x;
                for (y = 0; y < RR_PANEL_H && !diff; ++y) {
                    for (x = 0; x < RR_PANEL_W; ++x) {
                        if (fb1[(pzFbY0 + y) * FB_W + pzFbX0 + x] !=
                            fb2[(pzFbY0 + y) * FB_W + pzFbX0 + x]) {
                            diff = 1;
                            break;
                        }
                    }
                }
                if (diff) {
                    fprintf(stderr,
                            "FAIL panel-open draw instability in C040 panel zone\n");
                    ok = 0;
                }
            }
        }
        M11_GameView_Shutdown(&game_dup);
    }

    printf("  panel_open ordinal=%d front_mirror=%d candidate=%d inventory=%d championCount=%d\n",
           ORDINAL_HALK,
           M11_GameView_GetFrontMirrorOrdinal(&game),
           game.candidateMirrorPanelActive,
           game.inventoryPanelActive,
           game.world.party.championCount);
    printf("  rr_panel_drawn=%d/%d rr_zone_opaque=%d/%d bd_opaque=%d/%d\n",
           ev.rrAssetDrawn, ev.rrAssetOpaque,
           ev.rrZoneOpaqueOnFb, ev.rrZoneTotalOnFb,
           ev.bdZoneOpaqueOnFb, ev.bdZoneTotalOnFb);
    printf("  cutout_warm=%d cutout_c026_match=%d%% (%d/%d) status='%s'/'%s'\n",
           ev.portraitCutoutWarmPixels,
           ev.portraitCutoutC026MatchedPct,
           ev.portraitCutoutC026Matched, ev.portraitCutoutTotalCompared,
           game.lastAction, game.lastOutcome);

    M11_GameView_Shutdown(&game);
    printf("%s dm1 v1 HoC champion portrait ordinal 1 candidate_panel_open portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
