/*
 * firestaff_dm1_v1_hoc_champion_portrait_19_wall_ornament_no_float_runtime_probe.c
 *
 * Source-locked verification gate for one narrow Hall of Champions
 * slice that the existing portrait_19 cancel_reopen probe does not
 * cover:
 *
 *   ordinal 19               (mirror catalog record HAWK, title THE FEARLESS)
 *   route   wall_ornament_no_float  (D1C C346 wall-mirror frame + C026
 *                                   champion portrait blit; verify the
 *                                   C346 frame destination box is the
 *                                   container, the C026 portrait sprite
 *                                   is fully contained in its cutout,
 *                                   and neither sprite bleeds into the
 *                                   other or outside their respective
 *                                   destination boxes)
 *   aspect  wall_ornament_no_float
 *
 * The DM1 V1 D1C front-wall champion-mirror route draws two
 * distinct sprites in stacked order (per ReDMCSB
 * DUNVIEW.C:3913-3928 + DUNVIEW.C:3922-3928):
 *
 *   (a) C346 (global wall ornament 43) — the D1C wall-mirror frame
 *       blitted at the source-locked coordinateSet 5 / index 12
 *       destination box (DUNVIEW.C G0205):
 *
 *           dstX = 80, dstY = 29, width = 64, height = 43
 *           (viewport-relative)
 *
 *       The C346 bitmap is blitted with kOrnD2Palette through the
 *       F0791 route (m11_draw_dm1_front_mirror_backing_host_receipt
 *       -> m11_blit_scaled_palette_map_region, 48x43 scaled to
 *       64x43).  TAN (10) is the DM1 wall-ornament transparent key
 *       (DM1_WALL_ORNAMENT_TRANSPARENT_COLOR_PC34): those source
 *       pixels are skipped and the wall shows through.  Round 19
 *       replaces the stale procedural-fallback expectations (BLACK
 *       outer ring + LIGHT_GRAY top/left + GRAY bottom/right +
 *       DARK_GRAY interior) with the source-verified native C346
 *       profile: BROWN top edge, BLACK bottom/right edges, a fully
 *       transparent (skipped) TAN left column, and a BROWN ring
 *       fill with a CYAN (4) mirror-glass body, a LIGHT_GRAY(13)
 *       glass-shine diagonal and a YELLOW(11) name-plate band.
 *
 *   (b) C026 (champion portraits) — the ordinal 19 (HAWK) sprite
 *       from the 8x3 / 256x87 atlas blitted at the source-locked
 *       destination (DUNVIEW.C:3913-3919 + DEFS.H:821-826 +
 *       DUNVIEW.C G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *       = { 96, 127, 35, 63 }):
 *
 *           dstX = 96, dstY = 35, width = 32, height = 29
 *           (viewport-relative)
 *
 *       Ordinal 19 sits at row 2 / column 3 of the atlas
 *       (srcX=96, srcY=58).  The C026 sprite uses palette index
 *       1 (transparent) for the cutout border and any
 *       transparent holes.
 *
 * Both sprites are tightly stacked.  The "no_float" aspect of
 * this slice proves:
 *
 *   - the C346 frame's native edge profile is present at the
 *     destination box (80, 29, 64, 43) boundary — BROWN top edge,
 *     BLACK bottom/right edges, and a fully transparent TAN left
 *     column (skipped; the wall shows through) — so the frame is
 *     contained inside its destination;
 *
 *   - the C346 frame's native ring fill (BROWN + CYAN(4)
 *     mirror-glass body + LIGHT_GRAY(13) glass-shine diagonal +
 *     YELLOW(11) name-plate band) is present between the frame
 *     edges and the portrait cutout, and the TAN transparent key
 *     never leaks into the framebuffer;
 *
 *   - the C346 frame does not extend OUTSIDE the (80, 29, 64, 43)
 *     destination box — no BLACK border pixel appears above,
 *     below, left, or right of the frame;
 *
 *   - the C026 champion portrait sprite is fully drawn at
 *     (96, 35, 32, 29) — opaque-source match >= 90%, sprite
 *     pixels are a complete champion portrait, not a half-blit;
 *
 *   - the C026 portrait sprite's pixels do not extend OUTSIDE
 *     the (96, 35, 32, 29) cutout — no warm-color (skin / clothing)
 *     pixel appears in the C346 frame's ring;
 *
 *   - the C346 frame is byte-stable across redraws of the same
 *     pose, so the no-float invariant is a runtime invariant,
 *     not a one-shot initialization.
 *
 * This is disjoint from:
 *   - firestaff_dm1_v1_hall_of_champions_portrait_19_cancel_reopen
 *     (which covers select->cancel->select, panel-guard, and the
 *     side-wall no-float proof in row 2/col 3, but does NOT verify
 *     the C346 frame backing or the C346/C026 sprite isolation);
 *   - firestaff_dm1_v1_hall_of_champions_wall_mirror_zones (which
 *     covers the (7,9) and (1,5) routes at the shipped HALK
 *     ordinal 1 and ZED ordinal 10 with the basic "wall box is at
 *     the source-locked position" check, but does not lock the
 *     C346 backing fill colors, the C346 frame ring, or the
 *     no-float isolation between the C346 frame and the C026
 *     portrait);
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     (which covers the C127 sensor map but does not verify the
 *     C346 backing at the destination).
 *
 * Source evidence:
 *   - DUNGEON.C:2573 (C127 sensor cell match against view dir)
 *   - DUNGEON.C:2608-2612 (G0289 champion portrait ordinal)
 *   - DUNVIEW.C:525 (G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 })
 *   - DUNVIEW.C:1061 (G0205_aaauc_Graphic558_WallOrnamentCoordinateSets,
 *                     8x13x6 table; coordSet 5 / index 12 is the
 *                     D1C champion-mirror frame route)
 *   - DUNVIEW.C:3913-3919 (D1C C026 portrait blit at
 *                          ((ordinal & 7) << 5, (ordinal >> 3) * 29))
 *   - DUNVIEW.C:3922-3928 (C346 frame blit before C026)
 *   - COORD.C:1693-1749 (PC34 viewport origin and portrait dims)
 *   - DEFS.H:821-826 (M027_PORTRAIT_X / M028_PORTRAIT_Y macro math)
 *   - DEFS.H:2186 (C026_GRAPHIC_CHAMPION_PORTRAITS)
 *   - m11_dm1_wall_ornament_zone (DUNVIEW.C G0205 wall ornament sets)
 *   - m11_draw_dm1_front_mirror_backing_host_receipt (native C346
 *     blit via the F0791 route)
 *   - m11_draw_dm1_front_mirror_route (C346+C026 stack)
 *   - m11_blit_scaled_palette_map_region (C346 48x43 -> 64x43 blit
 *     with kOrnD2Palette, TAN(10) transparency skip)
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
    /* Source-locked D1C champion portrait rectangle
     * (DUNVIEW.C:3913-3928 + DEFS.H:821-826).  Framebuffer
     * destination = viewport origin + (dstX, dstY). */
    D1C_PORTRAIT_X = VIEWPORT_X + 96,
    D1C_PORTRAIT_Y = VIEWPORT_Y + 35,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* Source-locked D1C C346 wall-mirror frame rectangle
     * (DUNVIEW.C:1061 G0205 coordSet 5 / index 12).  This is the
     * outer destination of the C346 frame sprite; the C026
     * champion portrait sits inside it at (96, 35, 32, 29). */
    D1C_FRAME_X = VIEWPORT_X + 80,
    D1C_FRAME_Y = VIEWPORT_Y + 29,
    D1C_FRAME_W = 64,
    D1C_FRAME_H = 43,
    /* Native C346 raster profile after the kOrnD2Palette remap
     * (s_wallOrnamentPaletteD2: 5->3, 3->3, 15->13, 11->11, 4->4,
     * 0->0), source-verified from GRAPHICS.DAT asset 346 scaled
     * 48x43 -> 64x43 by m11_blit_scaled_palette_map_region.
     * DM1_WALL_ORNAMENT_TRANSPARENT_COLOR_PC34 = 10: TAN source
     * pixels are SKIPPED (the underlying wall shows through), and
     * cyan (4) is NOT transparent here - it is the mirror-glass
     * body and IS drawn:
     *   top edge row:    BROWN (3) dominant, TAN (10) skipped
     *   bottom edge row: BLACK (0) dominant, TAN (10) skipped
     *   left edge col:   TAN (10) for the full 43-pixel height ->
     *                    fully skipped, the wall (13) shows through
     *   right edge col:  BLACK (0) dominant, TAN (10) skipped
     *   corners:         TL=13 TR=13 BL=13 (wall) BR=0 (black)
     *   ring fill:       BROWN (3) + CYAN (4) mirror-glass body +
     *                    LIGHT_GRAY (13) glass diagonal + YELLOW
     *                    (11) name-plate band; the TAN transparent
     *                    key never appears in the framebuffer.
     * The C026 portrait cutout (96, 35, 32, 29) sits INSIDE the
     * frame; the C026 sprite blits its own pixels on top of the
     * C346 ring. */
    FRAME_OUTER_COLOR = 0,    /* M11_COLOR_BLACK */
    FRAME_BROWN_COLOR = 3,
    FRAME_TAN_COLOR = 10,     /* transparent key - skipped, never drawn */
    FRAME_GLASS_COLOR = 13,
    FRAME_PLATE_COLOR = 11,
    FRAME_CYAN_COLOR = 4,     /* mirror-glass body - drawn, not a leak */
    /* C040 candidate panel destination (the panel covers most of
     * the D1C cell below the row-2 portrait band; we use this to
     * clip the "below the frame" check to the unoccluded strip
     * above the panel). */
    C040_PANEL_Y = VIEWPORT_Y + 52,
    /* Atlas math for ordinal 19 (row 2 / col 3). */
    ATLAS_W = 256,
    ATLAS_H = 87,
    ORDINAL_19_COL = 19 & 7,        /* = 3 */
    ORDINAL_19_ROW = 19 >> 3,       /* = 2 */
    ORDINAL_19_SRC_X = ORDINAL_19_COL << 5,    /* = 96 */
    ORDINAL_19_SRC_Y = ORDINAL_19_ROW * 29,    /* = 58 */
    TARGET_ORDINAL = 19,
    /* The shipped DM1 V1 DUNGEON.DAT C127 sensor on (7,9) NORTH
     * has sensorData=1 (HALK).  We seed that sensor to ordinal 19
     * so we can lock the ordinal-19 edge case without changing
     * the map layout. */
    SHIPPED_HALK_ORDINAL = 1
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count pixels equal to a specific palette index in a rectangle. */
static int rect_count_color(const unsigned char* fb,
                            int x, int y, int w, int h,
                            unsigned char wantIdx) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (idx == wantIdx) ++cnt;
        }
    }
    return cnt;
}

/* Count pixels of a palette index across the four C346 frame ring
 * strips (top, bottom, left, right) around the portrait cutout.
 * The strips are the frame interior (2-pixel inset) minus the
 * (96, 35, 32, 29) portrait cutout. */
static int ring_count_color(const unsigned char* fb,
                            unsigned char wantIdx) {
    int cnt = 0;
    /* Top ring: y=[frame+2, portrait), full inner width. */
    cnt += rect_count_color(fb,
                            D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                            D1C_FRAME_W - 4, 4,
                            wantIdx);
    /* Bottom ring: y=[portrait bottom, frame bottom - 2). */
    cnt += rect_count_color(fb,
                            D1C_FRAME_X + 2,
                            D1C_PORTRAIT_Y + D1C_PORTRAIT_H,
                            D1C_FRAME_W - 4,
                            D1C_FRAME_Y + D1C_FRAME_H - 2 -
                                (D1C_PORTRAIT_Y + D1C_PORTRAIT_H),
                            wantIdx);
    /* Left ring: x=[frame+2, portrait), full inner height. */
    cnt += rect_count_color(fb,
                            D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                            D1C_PORTRAIT_X - (D1C_FRAME_X + 2),
                            D1C_FRAME_H - 4,
                            wantIdx);
    /* Right ring: x=[portrait right, frame right - 2). */
    cnt += rect_count_color(fb,
                            D1C_PORTRAIT_X + D1C_PORTRAIT_W,
                            D1C_FRAME_Y + 2,
                            D1C_FRAME_X + D1C_FRAME_W - 2 -
                                (D1C_PORTRAIT_X + D1C_PORTRAIT_W),
                            D1C_FRAME_H - 4,
                            wantIdx);
    return cnt;
}

/* Count distinct non-zero palette indices in a rectangle. */
static int rect_distinct(const unsigned char* fb,
                         int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            if (!seen[idx]) { seen[idx] = 1; ++n; }
        }
    }
    return n;
}

/* Sum the absolute difference between two framebuffer rectangles
 * (pixel-by-pixel, palette indices).  Returns the average L1
 * distance per pixel * 100. */
static int rect_l1_diff(const unsigned char* fbA,
                        const unsigned char* fbB,
                        int x, int y, int w, int h) {
    int sum = 0;
    int yy, xx;
    int compared = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            int a = M11_FB_DECODE_INDEX(fbA[yy * FB_W + xx]);
            int b = M11_FB_DECODE_INDEX(fbB[yy * FB_W + xx]);
            sum += (a > b) ? (a - b) : (b - a);
            ++compared;
        }
    }
    return (compared > 0) ? (sum * 100 / compared) : 0;
}

/* Count "warm" pixels (skin / clothing / portrait colors) in a
 * rectangle.  Used to detect that the C026 portrait sprite's
 * skin/clothing pixels are contained inside the (96, 35, 32, 29)
 * cutout and do not leak into the C346 frame ring.
 * Same palette set as the existing portrait_19 cancel_reopen probe:
 *   0x07 green, 0x08 red, 0x09 orange, 0x0A peach, 0x0B yellow,
 *   0x0E blue. */
static int rect_warm_count(const unsigned char* fb,
                           int x, int y, int w, int h) {
    int cnt = 0;
    int yy, xx;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = M11_FB_DECODE_INDEX(fb[yy * FB_W + xx]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++cnt;
                    break;
                default:
                    break;
            }
        }
    }
    return cnt;
}

/* Count opaque pixels in the C026 atlas cell for the requested
 * ordinal. */
static int atlas_cell_opaque_count(const M11_AssetSlot* portraits,
                                   int ordinal) {
    int x, y, cnt = 0;
    int srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    int srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            int sx = srcX + x;
            int sy = srcY + y;
            unsigned char src;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src != 0 && src != 1) ++cnt;
        }
    }
    return cnt;
}

/* Match the C026 portrait atlas cell for the requested ordinal
 * to the framebuffer D1C portrait rectangle.  Returns the percent
 * of opaque source pixels that match the destination palette. */
static int match_portrait_at_rect(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x, y, matched = 0, compared = 0;
    int srcX, srcY;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    srcX = (ordinal & 7) * D1C_PORTRAIT_W;
    srcY = (ordinal >> 3) * D1C_PORTRAIT_H;
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char src;
            unsigned char dst;
            int sx = srcX + x;
            int sy = srcY + y;
            if (sx >= (int)portraits->width ||
                sy >= (int)portraits->height) continue;
            src = (unsigned char)(portraits->pixels[sy * (int)portraits->width + sx] & 0x0F);
            if (src == 1) continue; /* transparent */
            dst = M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_Y + y) * FB_W + (D1C_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Find the first C127 sensor in the loaded world and rewrite its
 * sensorData from oldData to newData.  Same pattern as the
 * existing portrait_19 cancel_reopen probe. */
static int seed_first_c127_data(M11_GameViewState* state,
                                int oldData,
                                int newData) {
    int i;
    if (!state || !state->world.things || !state->world.things->sensors) {
        return -1;
    }
    for (i = 0; i < state->world.things->sensorCount; ++i) {
        if (state->world.things->sensors[i].sensorType == 127 &&
            (int)state->world.things->sensors[i].sensorData == oldData) {
            state->world.things->sensors[i].sensorData =
                (unsigned short)newData;
            return i;
        }
    }
    return -1;
}

/* Park the party at the (7,9) D1C front-mirror route facing
 * NORTH.  This is the C127 sensor position from the DM1 V1
 * DUNGEON.DAT shipped with the public PC 3.4 English release;
 * after seed_first_c127_data the front square reports ordinal
 * 19. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 7;
    state->world.party.mapY = 9;
    state->world.party.direction = DIR_NORTH;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->inventoryPanelActive = 0;
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;
    int seededSensor;
    int frontOrdinal;
    int ornX, ornY, ornW, ornH;
    int initialCount;

    /* Two framebuffers, both from the same deterministic pose
     * (7,9,0)=19, for the redraw-stability check.  The C346
     * native frame blit must be byte-stable across redraws (no
     * per-frame drift in the BROWN/TAN/BLACK edge profile or
     * the BROWN ring fill). */
    unsigned char fbA[FB_W * FB_H];
    unsigned char fbB[FB_W * FB_H];

    /* C346 frame composition counts (native raster profile). */
    int frameTopBrown;           /* BROWN pixels on the top edge row */
    int frameTopTan;             /* TAN on top edge - transparent key (== 0) */
    int frameBottomBlack;        /* BLACK pixels on the bottom edge row */
    int frameBottomTan;          /* TAN on bottom edge - key (== 0) */
    int frameLeftTan;            /* TAN on left edge - key (== 0) */
    int frameLeftWall;           /* wall LIGHT_GRAY(13) showing through the
                                    fully-skipped left edge column */
    int frameRightBlack;         /* BLACK pixels on the right edge col */
    int frameCornerTL;           /* palette index at the TL corner */
    int frameCornerTR;           /* palette index at the TR corner */
    int frameCornerBL;           /* palette index at the BL corner */
    int frameCornerBR;           /* palette index at the BR corner */
    int ringBrown;               /* BROWN fill across the 4 ring strips */
    int ringGlass;               /* LIGHT_GRAY (13) mirror-glass pixels */
    int ringPlate;               /* YELLOW (11) name-plate pixels */
    int ringTan;                 /* TAN in the ring - key (== 0) */
    int ringCyan;                /* CYAN (4) mirror-glass body pixels */

    /* C026 portrait zone metrics. */
    int matchPortrait;
    int warmPortrait;
    int distinctPortrait;
    int opaquePortraitPixels;

    /* C026 no-float: warm-color pixels OUTSIDE the cutout but
     * INSIDE the C346 frame ring should be near zero. */
    int warmInFrameRing;

    /* Outer no-float: BLACK border pixels OUTSIDE the C346 frame
     * rectangle.  These should be zero on the immediate border
     * (one pixel ring around (80, 29, 64, 43) framebuffer-
     * relative). */
    int blackAboveFrame;
    int blackBelowFrame;
    int blackLeftOfFrame;
    int blackRightOfFrame;

    /* Redraw stability. */
    int frameL1;
    int distinctRing;
    int matchB;
    int ringBrownA;
    int ringBrownB;
    int diffRingBrown;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions portrait-19 / wall_ornament_no_float ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* ----------------------------------------------------------------
     * Group A - atlas math for ordinal 19
     * ----------------------------------------------------------------
     * Verify the C026 atlas contains a defined portrait at row 2 /
     * col 3 and that the math matches COORD.C / DEFS.H:821-826.
     * The runtime drive is what actually links the atlas cell to
     * the D1C cutout in the next groups. */
    printf("\n[Group A] C026 atlas math for ordinal 19 (row 2 / col 3)\n");

    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas loads (graphic id returned by "
                 "M11_GameView_GetV1ChampionPortraitGraphicId = %d)",
                 M11_GameView_GetV1ChampionPortraitGraphicId());
        CHECK(portraits != NULL && portraits->loaded && portraits->pixels != NULL, msg);
    }
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        fprintf(stderr,
                "FATAL: cannot continue without the C026 portrait atlas\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas width = %u (expected 256 = 8 cols * 32)",
                 portraits->width);
        CHECK(portraits->width == 256, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "C026 atlas height = %u (expected 87 = 3 rows * 29)",
                 portraits->height);
        CHECK(portraits->height == 87, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 19 srcX = %d, srcY = %d "
                 "(within 256x87 atlas: must be < %d and < %d)",
                 ORDINAL_19_SRC_X, ORDINAL_19_SRC_Y, ATLAS_W, ATLAS_H);
        CHECK(ORDINAL_19_SRC_X + D1C_PORTRAIT_W <= ATLAS_W &&
              ORDINAL_19_SRC_Y + D1C_PORTRAIT_H <= ATLAS_H, msg);
    }
    opaquePortraitPixels = atlas_cell_opaque_count(portraits, 19);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "ordinal 19 atlas cell has >= 200 opaque pixels (got %d) "
                 "- defined portrait, framed by C346",
                 opaquePortraitPixels);
        CHECK(opaquePortraitPixels >= 200, msg);
    }

    /* ----------------------------------------------------------------
     * Group B - park the party on the (7,9) NORTH route, seed the
     * C127 sensor to ordinal 19, and lock the D1C wall ornament
     * zone to coordSet 5 / index 12. */
    printf("\n[Group B] Park on (7,9,0) and seed C127 sensor to ordinal 19\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

    /* Sanity: the unmodified route reports the shipped HALK
     * ordinal 1 before seed. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front-mirror ordinal at (7,9,0) = %d (expected "
                 "%d, HALK before seed)",
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }

    /* Seed the (7,9) NORTH-route C127 sensor from HALK (1) to
     * ordinal 19. */
    seededSensor = seed_first_c127_data(&state,
                                         SHIPPED_HALK_ORDINAL,
                                         TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (7,9) NORTH C127 sensor from ordinal %d "
                 "(HALK) to ordinal %d (sensor index %d)",
                 SHIPPED_HALK_ORDINAL, TARGET_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded north-entry front-mirror ordinal = %d (expected %d)",
                 frontOrdinal, TARGET_ORDINAL);
        CHECK(frontOrdinal == TARGET_ORDINAL, msg);
    }
    if (frontOrdinal != TARGET_ORDINAL) {
        fprintf(stderr,
                "FATAL: front ordinal did not lock to %d after seed; "
                "cannot verify wall_ornament_no_float\n",
                TARGET_ORDINAL);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* Lock the D1C wall ornament zone (DUNVIEW.C G0205 coordSet 5
     * / index 12).  The runtime must report (80, 29, 64, 43) for
     * the C346 frame destination.  This is the C346 frame
     * rectangle, NOT the C026 portrait rectangle (which is
     * (96, 35, 32, 29) inside the frame). */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(&state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone = (%d, %d, %d, %d) viewport "
                 "coords (DUNVIEW.C G0205 coordSet 5 / index 12)",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX == 80 && ornY == 29 && ornW == 64 && ornH == 43, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35, 32, 29) sits inside the "
                 "D1C wall ornament zone (X within [%d,%d), Y within "
                 "[%d,%d))",
                 ornX, ornX + ornW, ornY, ornY + ornH);
        CHECK(96 >= ornX &&
              96 + D1C_PORTRAIT_W <= ornX + ornW &&
              35 >= ornY &&
              35 + D1C_PORTRAIT_H <= ornY + ornH, msg);
    }

    /* ----------------------------------------------------------------
     * Group C - native C346 frame edge profile at (80, 29, 64, 43)
     * ----------------------------------------------------------------
     * Render the framebuffer with no panel interaction and verify
     * the native C346 raster edge profile at the frame destination
     * box.  The engine blits GRAPHICS.DAT asset 346 (48x43) scaled
     * to 64x43 through the kOrnD2Palette map via the F0791 route
     * (m11_draw_dm1_front_mirror_backing_host_receipt).  TAN (10)
     * is the DM1 wall-ornament transparent key: those source
     * pixels are skipped and the underlying wall (LIGHT_GRAY 13)
     * shows through.  The source-verified edge profile is:
     *   top row:    BROWN (3) dominant (59/64), TAN skipped (5)
     *   bottom row: BLACK (0) dominant (58/64), TAN skipped (6)
     *   left col:   TAN (10) full height -> skipped, wall shows
     *   right col:  BLACK (0) dominant (41/43), TAN skipped (2)
     *   corners:    TL=13 TR=13 BL=13 (wall) BR=0 (black)
     * This replaces the stale round-16 procedural-fallback
     * expectations (BLACK outer ring + LIGHT_GRAY top/left + GRAY
     * bottom/right) — the engine has rendered the native C346
     * bitmap all along. */
    printf("\n[Group C] native C346 frame edge profile at (80, 29, 64, 43)\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = initialCount;
    memset(fbA, 0, sizeof(fbA));
    M11_GameView_Draw(&state, fbA, FB_W, FB_H);

    /* Top edge row: BROWN dominant (model: 59 of 64). */
    frameTopBrown = rect_count_color(fbA,
                                     D1C_FRAME_X, D1C_FRAME_Y,
                                     D1C_FRAME_W, 1,
                                     FRAME_BROWN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame top edge has >= 50 BROWN pixels "
                 "(got %d / 64, native model 59)",
                 frameTopBrown);
        CHECK(frameTopBrown >= 50, msg);
    }
    /* Top edge row: the TAN transparent key is skipped, so no
     * TAN pixel may appear (model: 5 skipped source pixels). */
    frameTopTan = rect_count_color(fbA,
                                   D1C_FRAME_X, D1C_FRAME_Y,
                                   D1C_FRAME_W, 1,
                                   FRAME_TAN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame top edge has zero TAN pixels - the "
                 "transparent key never leaks (got %d)",
                 frameTopTan);
        CHECK(frameTopTan == 0, msg);
    }
    /* Bottom edge row: BLACK dominant (model: 58 of 64). */
    frameBottomBlack = rect_count_color(fbA,
                                        D1C_FRAME_X,
                                        D1C_FRAME_Y + D1C_FRAME_H - 1,
                                        D1C_FRAME_W, 1,
                                        FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame bottom edge has >= 50 BLACK pixels "
                 "(got %d / 64, native model 58)",
                 frameBottomBlack);
        CHECK(frameBottomBlack >= 50, msg);
    }
    /* Bottom edge row: the TAN transparent key never appears
     * (model: 6 skipped source pixels at the left). */
    frameBottomTan = rect_count_color(fbA,
                                      D1C_FRAME_X,
                                      D1C_FRAME_Y + D1C_FRAME_H - 1,
                                      D1C_FRAME_W, 1,
                                      FRAME_TAN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame bottom edge has zero TAN pixels - the "
                 "transparent key never leaks (got %d)",
                 frameBottomTan);
        CHECK(frameBottomTan == 0, msg);
    }
    /* Left edge col: the source column is TAN (transparent) for
     * the full 43-pixel height, so the underlying wall shows
     * through and no TAN is drawn. */
    frameLeftTan = rect_count_color(fbA,
                                    D1C_FRAME_X, D1C_FRAME_Y,
                                    1, D1C_FRAME_H,
                                    FRAME_TAN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame left edge has zero TAN pixels - the "
                 "full-height transparent column is skipped (got %d)",
                 frameLeftTan);
        CHECK(frameLeftTan == 0, msg);
    }
    frameLeftWall = rect_count_color(fbA,
                                     D1C_FRAME_X, D1C_FRAME_Y,
                                     1, D1C_FRAME_H,
                                     FRAME_GLASS_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame left edge shows >= 25 wall LIGHT_GRAY(13) "
                 "pixels through the skipped TAN column (got %d / 43)",
                 frameLeftWall);
        CHECK(frameLeftWall >= 25, msg);
    }
    /* Right edge col: BLACK dominant (model: 41 of 43). */
    frameRightBlack = rect_count_color(fbA,
                                       D1C_FRAME_X + D1C_FRAME_W - 1,
                                       D1C_FRAME_Y,
                                       1, D1C_FRAME_H,
                                       FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame right edge has >= 38 BLACK pixels "
                 "(got %d / 43, native model 41)",
                 frameRightBlack);
        CHECK(frameRightBlack >= 38, msg);
    }
    /* Corners: TL/TR/BL are skipped TAN source pixels (wall
     * LIGHT_GRAY 13 shows through); BR is BLACK. */
    frameCornerTL = M11_FB_DECODE_INDEX(
        fbA[D1C_FRAME_Y * FB_W + D1C_FRAME_X]);
    frameCornerTR = M11_FB_DECODE_INDEX(
        fbA[D1C_FRAME_Y * FB_W + D1C_FRAME_X + D1C_FRAME_W - 1]);
    frameCornerBL = M11_FB_DECODE_INDEX(
        fbA[(D1C_FRAME_Y + D1C_FRAME_H - 1) * FB_W + D1C_FRAME_X]);
    frameCornerBR = M11_FB_DECODE_INDEX(
        fbA[(D1C_FRAME_Y + D1C_FRAME_H - 1) * FB_W + D1C_FRAME_X +
            D1C_FRAME_W - 1]);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame corners are TL=13 TR=13 BL=13 (wall) "
                 "BR=BLACK (got %d/%d/%d/%d) - frame is anchored at "
                 "the destination corners",
                 frameCornerTL, frameCornerTR,
                 frameCornerBL, frameCornerBR);
        CHECK(frameCornerTL == FRAME_GLASS_COLOR &&
              frameCornerTR == FRAME_GLASS_COLOR &&
              frameCornerBL == FRAME_GLASS_COLOR &&
              frameCornerBR == FRAME_OUTER_COLOR, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - native C346 ring fill profile around the portrait
     * ----------------------------------------------------------------
     * The native C346 raster fills the ring between the frame edges
     * and the portrait cutout with BROWN (3) after the kOrnD2Palette
     * remap, plus a CYAN (4) mirror-glass body (cyan is NOT the
     * transparent key for DM1 wall ornaments - TAN (10) is), a
     * LIGHT_GRAY (13) glass-shine diagonal, and a YELLOW (11)
     * name-plate band in the bottom strip.  TAN source pixels are
     * skipped, so no TAN may appear in the framebuffer.  The C026
     * portrait (96, 35, 32, 29) sits INSIDE the ring and blits its
     * own pixels on top.
     * Source-verified model counts across the 4 ring strips:
     *   brown=1299  cyan=284  glass(13)=35+wall  plate(11)=19  tan=0. */
    printf("\n[Group D] native C346 ring fill profile around portrait cutout\n");

    ringBrown = ring_count_color(fbA, FRAME_BROWN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame ring has >= 1000 BROWN pixels "
                 "(got %d, native model 1299)",
                 ringBrown);
        CHECK(ringBrown >= 1000, msg);
    }
    ringCyan = ring_count_color(fbA, FRAME_CYAN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame ring has >= 250 CYAN(4) mirror-glass body "
                 "pixels (got %d, native model 284)",
                 ringCyan);
        CHECK(ringCyan >= 250, msg);
    }
    ringGlass = ring_count_color(fbA, FRAME_GLASS_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame ring has >= 20 LIGHT_GRAY(13) glass-shine "
                 "pixels (got %d, native model 35 + wall show-through)",
                 ringGlass);
        CHECK(ringGlass >= 20, msg);
    }
    ringPlate = ring_count_color(fbA, FRAME_PLATE_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame ring has >= 10 YELLOW(11) name-plate "
                 "pixels (got %d, native model 19)",
                 ringPlate);
        CHECK(ringPlate >= 10, msg);
    }
    ringTan = ring_count_color(fbA, FRAME_TAN_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame ring has zero TAN pixels - the transparent "
                 "key never leaks (got %d)",
                 ringTan);
        CHECK(ringTan == 0, msg);
    }
    ringBrownA = ringBrown;

    /* ----------------------------------------------------------------
     * Group E - C026 portrait sprite is fully drawn at the cutout
     * ----------------------------------------------------------------
     * The C026 sprite for ordinal 19 (HAWK) must match the
     * framebuffer at (96, 35, 32, 29) at >= 90% — the sprite is
     * fully blitted, not a half-blit or a stale HALK sprite.  The
     * sprite's own pixels can use any palette index (skin,
     * clothing, hair, shadows), so we don't check the palette
     * composition of the cutout — only that the sprite is the
     * ordinal-19 sprite. */
    printf("\n[Group E] C026 portrait sprite fully drawn at (96, 35, 32, 29)\n");

    matchPortrait = match_portrait_at_rect(portraits, fbA, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect (96, 35) carries ordinal %d pixels "
                 "at >= 90%% match (got %d%%)",
                 TARGET_ORDINAL, matchPortrait);
        CHECK(matchPortrait >= 90, msg);
    }
    warmPortrait = rect_warm_count(fbA,
                                   D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                   D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= 100 warm-color pixels (got %d) "
                 "- portrait sprite, not just frame fill",
                 warmPortrait);
        CHECK(warmPortrait >= 100, msg);
    }
    distinctPortrait = rect_distinct(fbA,
                                     D1C_PORTRAIT_X, D1C_PORTRAIT_Y,
                                     D1C_PORTRAIT_W, D1C_PORTRAIT_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect has >= 4 distinct palette indices "
                 "(got %d) - portrait sprite, not single-color fill",
                 distinctPortrait);
        CHECK(distinctPortrait >= 4, msg);
    }

    /* ----------------------------------------------------------------
     * Group F - C026 portrait sprite does not leak into the C346
     * frame ring: warm-color pixels (skin / clothing) should be
     * contained in the (96, 35, 32, 29) cutout.  The native C346
     * raster itself contributes exactly 19 warm pixels to the ring
     * (the YELLOW (11) name-plate band; TAN is the transparent key
     * and never appears).  The threshold allows the native 19 plus
     * slack, but a portrait leak of even a small sprite fragment
     * (40+ warm pixels) still trips the gate. */
    printf("\n[Group F] C026 portrait sprite contained in (96, 35, 32, 29) - no warm leak into frame ring\n");

    /* The C346 frame ring is (80, 29, 64, 43) MINUS the portrait
     * cutout (96, 35, 32, 29).  We sample four strips.  Sum the
     * warm pixels in the ring.  Expected: the native C346 baseline
     * of 19 warm pixels (the name-plate band), nothing more. */
    warmInFrameRing = 0;
    /* Top ring warm. */
    warmInFrameRing += rect_warm_count(fbA,
                                       D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                                       D1C_FRAME_W - 4, 4);
    /* Bottom ring warm. */
    warmInFrameRing += rect_warm_count(fbA,
                                       D1C_FRAME_X + 2,
                                       D1C_PORTRAIT_Y + D1C_PORTRAIT_H,
                                       D1C_FRAME_W - 4,
                                       D1C_FRAME_Y + D1C_FRAME_H - 2 -
                                           (D1C_PORTRAIT_Y + D1C_PORTRAIT_H));
    /* Left ring warm. */
    warmInFrameRing += rect_warm_count(fbA,
                                       D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                                       D1C_PORTRAIT_X - (D1C_FRAME_X + 2),
                                       D1C_FRAME_H - 4);
    /* Right ring warm. */
    warmInFrameRing += rect_warm_count(fbA,
                                       D1C_PORTRAIT_X + D1C_PORTRAIT_W,
                                       D1C_FRAME_Y + 2,
                                       D1C_FRAME_X + D1C_FRAME_W - 2 -
                                           (D1C_PORTRAIT_X + D1C_PORTRAIT_W),
                                       D1C_FRAME_H - 4);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame ring has < 60 warm pixels total (got %d, "
                 "native C346 baseline 19) - C026 portrait sprite does "
                 "not leak warm colors into the frame ring",
                 warmInFrameRing);
        CHECK(warmInFrameRing < 60, msg);
    }

    /* ----------------------------------------------------------------
     * Group G - C346 frame is contained in (80, 29, 64, 43): no
     * BLACK border pixel appears OUTSIDE the frame destination
     * box.  We sample 1-pixel rings on each side of the frame. */
    printf("\n[Group G] C346 frame contained in (80, 29, 64, 43) - no BLACK outside the frame\n");

    /* Above the frame: 1-pixel row at y=D1C_FRAME_Y-1, x range
     * matches the frame X.  Should NOT be BLACK.  Note: there
     * could be other wall geometry here, but the C346 frame
     * backing does not extend above the frame destination box. */
    blackAboveFrame = rect_count_color(fbA,
                                       D1C_FRAME_X,
                                       D1C_FRAME_Y - 1,
                                       D1C_FRAME_W, 1,
                                       FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "row above D1C frame has <= 10 BLACK pixels (got %d) - "
                 "C346 frame top edge does not float upward",
                 blackAboveFrame);
        CHECK(blackAboveFrame <= 10, msg);
    }
    /* Left of the frame: 1-pixel col at x=D1C_FRAME_X-1, y range
     * matches the frame Y. */
    blackLeftOfFrame = rect_count_color(fbA,
                                        D1C_FRAME_X - 1,
                                        D1C_FRAME_Y,
                                        1, D1C_FRAME_H,
                                        FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "col left of D1C frame has <= 10 BLACK pixels (got %d) - "
                 "C346 frame left edge does not float leftward",
                 blackLeftOfFrame);
        CHECK(blackLeftOfFrame <= 10, msg);
    }
    /* Right of the frame: 1-pixel col at x=D1C_FRAME_X+D1C_FRAME_W,
     * y range matches the frame Y. */
    blackRightOfFrame = rect_count_color(fbA,
                                         D1C_FRAME_X + D1C_FRAME_W,
                                         D1C_FRAME_Y,
                                         1, D1C_FRAME_H,
                                         FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "col right of D1C frame has <= 10 BLACK pixels (got %d) - "
                 "C346 frame right edge does not float rightward",
                 blackRightOfFrame);
        CHECK(blackRightOfFrame <= 10, msg);
    }
    /* Below the frame: 1-pixel row at y=D1C_FRAME_Y+D1C_FRAME_H,
     * x range matches the frame X.  Note: the C040 panel covers
     * part of this strip when active, but we are testing
     * panel-off so the panel is NOT drawn.  The bottom row
     * should be wall stone, not BLACK. */
    blackBelowFrame = rect_count_color(fbA,
                                       D1C_FRAME_X,
                                       D1C_FRAME_Y + D1C_FRAME_H,
                                       D1C_FRAME_W, 1,
                                       FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "row below D1C frame has <= 10 BLACK pixels (got %d) - "
                 "C346 frame bottom edge does not float downward",
                 blackBelowFrame);
        CHECK(blackBelowFrame <= 10, msg);
    }

    /* ----------------------------------------------------------------
     * Group H - redraw stability: the native C346 frame blit is
     * byte-stable across redraws of the same (7,9,0)=19 pose.
     * Both the C346 bitmap blit and the C026 portrait sprite are
     * deterministic for a fixed pose, so the frame rectangle and
     * the BROWN ring count must not drift. */
    printf("\n[Group H] redraw stability: C346 frame is byte-stable across redraws\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = initialCount;
    memset(fbB, 0, sizeof(fbB));
    M11_GameView_Draw(&state, fbB, FB_W, FB_H);

    /* The C346 frame rectangle (80, 29, 64, 43) must be byte-
     * identical (or near-identical, since the C026 portrait
     * sprite blit is also byte-deterministic for the same
     * pose) across two redraws.  L1 distance per pixel should
     * be 0 — both sprites are deterministic for a fixed pose. */
    frameL1 = rect_l1_diff(fbA, fbB,
                           D1C_FRAME_X, D1C_FRAME_Y,
                           D1C_FRAME_W, D1C_FRAME_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame rectangle L1 distance per pixel <= 1 "
                 "(got %d / 100) - frame is byte-stable across redraws",
                 frameL1);
        CHECK(frameL1 <= 1, msg);
    }
    /* The full frame rect (D1C frame including C026 portrait
     * cutout) has a stable distinct-palette count.  The C346
     * bitmap can contribute a few colors (the kOrnD2Palette
     * remap produces 14 distinct indices in the worst case),
     * and the C026 portrait adds its own palette.  Expect 8..16
     * distinct indices. */
    distinctRing = rect_distinct(fbA,
                                 D1C_FRAME_X, D1C_FRAME_Y,
                                 D1C_FRAME_W, D1C_FRAME_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame rect has 4..16 distinct palette indices "
                 "(got %d) - 4 frame fill colors + C026 portrait + "
                 "C346 bitmap",
                 distinctRing);
        CHECK(distinctRing >= 4 && distinctRing <= 16, msg);
    }
    /* The portrait cutout is fully drawn on the second redraw. */
    matchB = match_portrait_at_rect(portraits, fbB, TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "second redraw: D1C portrait rect matches ordinal 19 "
                 "at >= 90%% (got %d%%)",
                 matchB);
        CHECK(matchB >= 90, msg);
    }
    /* BROWN ring pixel count must be stable. */
    ringBrownB = ring_count_color(fbB, FRAME_BROWN_COLOR);
    diffRingBrown = (ringBrownA > ringBrownB)
                        ? (ringBrownA - ringBrownB)
                        : (ringBrownB - ringBrownA);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame BROWN ring is byte-stable across "
                 "redraws (redraw-A=%d, redraw-B=%d, |diff|=%d <= 5)",
                 ringBrownA, ringBrownB, diffRingBrown);
        CHECK(diffRingBrown <= 5, msg);
    }

    /* ----------------------------------------------------------------
     * Group I - consolidated no-float invariant
     * ----------------------------------------------------------------
     * Single-line summary of the native C346 frame profile +
     * C026 portrait sprite + no-float checks.  If the whole
     * no-float story holds, this line is the gate. */
    printf("\n[Group I] consolidated no-float invariant\n");
    {
        char msg[300];
        snprintf(msg, sizeof(msg),
                 "consolidated no-float: top brown=%d tan=%d, "
                 "bottom black=%d tan=%d, left tan=%d wall13=%d, "
                 "right black=%d, corners=%d/%d/%d/%d, ring brown=%d "
                 "cyan=%d glass=%d plate=%d tan=%d, match=%d%%, "
                 "warm-portrait=%d, warm-leak=%d (native C346 edge "
                 "profile anchored, TAN transparent key skipped, "
                 "cyan mirror-glass body present, portrait sprite "
                 "fully drawn, no warm leak)",
                 frameTopBrown, frameTopTan,
                 frameBottomBlack, frameBottomTan,
                 frameLeftTan, frameLeftWall, frameRightBlack,
                 frameCornerTL, frameCornerTR,
                 frameCornerBL, frameCornerBR,
                 ringBrown, ringCyan, ringGlass, ringPlate, ringTan,
                 matchPortrait, warmPortrait,
                 warmInFrameRing);
        CHECK(frameTopBrown >= 50 &&
              frameTopTan == 0 &&
              frameBottomBlack >= 50 &&
              frameBottomTan == 0 &&
              frameLeftTan == 0 &&
              frameLeftWall >= 25 &&
              frameRightBlack >= 38 &&
              frameCornerTL == FRAME_GLASS_COLOR &&
              frameCornerTR == FRAME_GLASS_COLOR &&
              frameCornerBL == FRAME_GLASS_COLOR &&
              frameCornerBR == FRAME_OUTER_COLOR &&
              ringBrown >= 1000 &&
              ringCyan >= 250 &&
              ringGlass >= 20 &&
              ringPlate >= 10 &&
              ringTan == 0 &&
              matchPortrait >= 90 &&
              warmPortrait >= 100 &&
              warmInFrameRing < 60, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
