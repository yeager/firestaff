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
 *       The C346 bitmap is blitted with kOrnD2Palette, then the
 *       m11_draw_dm1_front_mirror_backing fallback overlays a
 *       1-pixel BLACK outer ring + 1-pixel LIGHT_GRAY top/left
 *       border + 1-pixel GRAY bottom/right border + DARK_GRAY
 *       interior with 2-pixel inset.  The backing overlay keeps
 *       the frame opaque even when the extracted C346 bitmap is
 *       empty/transparent.
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
 *   - the C346 frame's outer 1-pixel BLACK ring is present at
 *     the destination box (80, 29, 64, 43) boundary, so the
 *     frame is contained inside its destination;
 *
 *   - the C346 frame's backing inner borders (LIGHT_GRAY top +
 *     left, GRAY bottom + right) are present at offset 1, so
 *     the backing overlay fired (the frame is the procedural
 *     D1C frame, not just an empty C346 blit);
 *
 *   - the C346 frame's DARK_GRAY interior fill is present in
 *     the ring between the inner borders and the portrait
 *     cutout, so the C026 portrait sprite sits on the
 *     procedurally drawn frame interior;
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
 *     covers the (1,2) and (1,5) routes at the shipped HALK
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
 *   - m11_draw_dm1_front_mirror_backing (C346 backing overlay)
 *   - m11_draw_dm1_front_mirror_route (C346+C026 stack)
 *   - m11_blit_scaled_palette_map_maybe_flip (C346 blit with
 *                                             kOrnD2Palette)
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
    /* C346 frame backing composition (m11_draw_dm1_front_mirror_backing):
     *   outer 1-pixel ring: BLACK (index 0)
     *   inner 1-pixel top+left border: LIGHT_GRAY (index 2)
     *   inner 1-pixel bottom+right border: GRAY (index 1)
     *   interior fill (2-pixel inset): DARK_GRAY (index 12)
     * The C026 portrait cutout (96, 35, 32, 29) sits INSIDE the
     * interior fill; the C026 sprite blits its own pixels on top
     * of the backing. */
    FRAME_OUTER_COLOR = 0,    /* M11_COLOR_BLACK */
    FRAME_TOPLEFT_COLOR = 2,  /* M11_COLOR_LIGHT_GRAY */
    FRAME_BOTRIGHT_COLOR = 1, /* M11_COLOR_GRAY */
    FRAME_FILL_COLOR = 12,    /* M11_COLOR_DARK_GRAY */
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
    /* The shipped DM1 V1 DUNGEON.DAT C127 sensor on (1,2) NORTH
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

/* Park the party at the (1,2) D1C front-mirror route facing
 * NORTH.  This is the C127 sensor position from the DM1 V1
 * DUNGEON.DAT shipped with the public PC 3.4 English release;
 * after seed_first_c127_data the front square reports ordinal
 * 19. */
static void park_d1c_front_route(M11_GameViewState* state) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = 1;
    state->world.party.mapY = 2;
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
     * (1,2,0)=19, for the redraw-stability check.  The C346
     * frame backing must be byte-stable across redraws (no
     * per-frame drift in the BLACK ring, LIGHT_GRAY/GRAY border,
     * or DARK_GRAY interior fill). */
    unsigned char fbA[FB_W * FB_H];
    unsigned char fbB[FB_W * FB_H];

    /* C346 frame composition counts. */
    int blackOuterTopEdge;       /* 1-pixel BLACK row at the top edge */
    int blackOuterBottomEdge;    /* 1-pixel BLACK row at the bottom edge */
    int blackOuterLeftEdge;      /* 1-pixel BLACK col at the left edge */
    int blackOuterRightEdge;     /* 1-pixel BLACK col at the right edge */
    int blackOuterCorners;       /* BLACK corner pixels (the 4 corners) */
    int lightGrayTopBorder;      /* 1-pixel LIGHT_GRAY top border row */
    int lightGrayLeftBorder;     /* 1-pixel LIGHT_GRAY left border col */
    int grayBottomBorder;        /* 1-pixel GRAY bottom border row */
    int grayRightBorder;         /* 1-pixel GRAY right border col */
    int darkGrayRingTop;         /* DARK_GRAY ring strip above the portrait */
    int darkGrayRingBottom;      /* DARK_GRAY ring strip below the portrait */
    int darkGrayRingLeft;        /* DARK_GRAY ring strip left of the portrait */
    int darkGrayRingRight;       /* DARK_GRAY ring strip right of the portrait */

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
    int darkGrayRingA;
    int darkGrayRingB;
    int diffDarkGrayRing;

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
     * Group B - park the party on the (1,2) NORTH route, seed the
     * C127 sensor to ordinal 19, and lock the D1C wall ornament
     * zone to coordSet 5 / index 12. */
    printf("\n[Group B] Park on (1,2,0) and seed C127 sensor to ordinal 19\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = 0;
    initialCount = state.world.party.championCount;

    /* Sanity: the unmodified route reports the shipped HALK
     * ordinal 1 before seed. */
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&state);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "shipped front-mirror ordinal at (1,2,0) = %d (expected "
                 "%d, HALK before seed)",
                 frontOrdinal, SHIPPED_HALK_ORDINAL);
        CHECK(frontOrdinal == SHIPPED_HALK_ORDINAL, msg);
    }

    /* Seed the (1,2) NORTH-route C127 sensor from HALK (1) to
     * ordinal 19. */
    seededSensor = seed_first_c127_data(&state,
                                         SHIPPED_HALK_ORDINAL,
                                         TARGET_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "seeded (1,2) NORTH C127 sensor from ordinal %d "
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
     * Group C - C346 frame backing at the (80, 29, 64, 43) boundary
     * ----------------------------------------------------------------
     * Render the framebuffer with no panel interaction and verify
     * the C346 backing is present at the frame destination box.
     * The backing is a 1-pixel BLACK outer ring + 1-pixel
     * LIGHT_GRAY top/left border + 1-pixel GRAY bottom/right
     * border + DARK_GRAY interior fill (2-pixel inset).  The
     * outer BLACK ring is the cleanest "frame is contained" proof:
     * it must be present at the top, bottom, left, and right
     * edges. */
    printf("\n[Group C] C346 frame backing: BLACK ring + LIGHT_GRAY/GRAY border at (80, 29, 64, 43)\n");

    park_d1c_front_route(&state);
    state.world.party.championCount = initialCount;
    memset(fbA, 0, sizeof(fbA));
    M11_GameView_Draw(&state, fbA, FB_W, FB_H);

    /* BLACK outer top edge: row y=D1C_FRAME_Y, full frame width
     * 64 pixels.  Expected >= 60 (the C346 bitmap may overwrite
     * some pixels if it has visible content; the backing is
     * drawn AFTER, so the BLACK ring is the dominant color on
     * the row, but the 4 corners are also overwritten by the
     * LIGHT_GRAY/GRAY borders in a small overlap region). */
    blackOuterTopEdge = rect_count_color(fbA,
                                         D1C_FRAME_X, D1C_FRAME_Y,
                                         D1C_FRAME_W, 1,
                                         FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame outer BLACK top edge has >= 50 pixels "
                 "(got %d / 64)",
                 blackOuterTopEdge);
        CHECK(blackOuterTopEdge >= 50, msg);
    }
    blackOuterBottomEdge = rect_count_color(fbA,
                                            D1C_FRAME_X,
                                            D1C_FRAME_Y + D1C_FRAME_H - 1,
                                            D1C_FRAME_W, 1,
                                            FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame outer BLACK bottom edge has >= 50 pixels "
                 "(got %d / 64)",
                 blackOuterBottomEdge);
        CHECK(blackOuterBottomEdge >= 50, msg);
    }
    blackOuterLeftEdge = rect_count_color(fbA,
                                           D1C_FRAME_X, D1C_FRAME_Y,
                                           1, D1C_FRAME_H,
                                           FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame outer BLACK left edge has >= 35 pixels "
                 "(got %d / 43)",
                 blackOuterLeftEdge);
        CHECK(blackOuterLeftEdge >= 35, msg);
    }
    blackOuterRightEdge = rect_count_color(fbA,
                                            D1C_FRAME_X + D1C_FRAME_W - 1,
                                            D1C_FRAME_Y,
                                            1, D1C_FRAME_H,
                                            FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame outer BLACK right edge has >= 35 pixels "
                 "(got %d / 43)",
                 blackOuterRightEdge);
        CHECK(blackOuterRightEdge >= 35, msg);
    }
    blackOuterCorners = rect_count_color(fbA,
                                         D1C_FRAME_X, D1C_FRAME_Y,
                                         1, 1,
                                         FRAME_OUTER_COLOR)
                      + rect_count_color(fbA,
                                         D1C_FRAME_X + D1C_FRAME_W - 1,
                                         D1C_FRAME_Y,
                                         1, 1,
                                         FRAME_OUTER_COLOR)
                      + rect_count_color(fbA,
                                         D1C_FRAME_X,
                                         D1C_FRAME_Y + D1C_FRAME_H - 1,
                                         1, 1,
                                         FRAME_OUTER_COLOR)
                      + rect_count_color(fbA,
                                         D1C_FRAME_X + D1C_FRAME_W - 1,
                                         D1C_FRAME_Y + D1C_FRAME_H - 1,
                                         1, 1,
                                         FRAME_OUTER_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame outer BLACK 4-corner sum >= 2 (got %d) - "
                 "frame is anchored at the destination corners",
                 blackOuterCorners);
        CHECK(blackOuterCorners >= 2, msg);
    }

    /* LIGHT_GRAY top border: row y=D1C_FRAME_Y+1, cols
     * [D1C_FRAME_X+1..D1C_FRAME_X+D1C_FRAME_W-2).  Width 62. */
    lightGrayTopBorder = rect_count_color(fbA,
                                           D1C_FRAME_X + 1, D1C_FRAME_Y + 1,
                                           D1C_FRAME_W - 2, 1,
                                           FRAME_TOPLEFT_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame LIGHT_GRAY top border has >= 50 pixels "
                 "(got %d / 62)",
                 lightGrayTopBorder);
        CHECK(lightGrayTopBorder >= 50, msg);
    }
    /* LIGHT_GRAY left border: col x=D1C_FRAME_X+1, rows
     * [D1C_FRAME_Y+1..D1C_FRAME_Y+D1C_FRAME_H-2).  Height 41. */
    lightGrayLeftBorder = rect_count_color(fbA,
                                            D1C_FRAME_X + 1, D1C_FRAME_Y + 1,
                                            1, D1C_FRAME_H - 2,
                                            FRAME_TOPLEFT_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame LIGHT_GRAY left border has >= 30 pixels "
                 "(got %d / 41)",
                 lightGrayLeftBorder);
        CHECK(lightGrayLeftBorder >= 30, msg);
    }
    /* GRAY bottom border: row y=D1C_FRAME_Y+D1C_FRAME_H-2, cols
     * [D1C_FRAME_X+1..D1C_FRAME_X+D1C_FRAME_W-2).  Width 62. */
    grayBottomBorder = rect_count_color(fbA,
                                        D1C_FRAME_X + 1,
                                        D1C_FRAME_Y + D1C_FRAME_H - 2,
                                        D1C_FRAME_W - 2, 1,
                                        FRAME_BOTRIGHT_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame GRAY bottom border has >= 50 pixels "
                 "(got %d / 62)",
                 grayBottomBorder);
        CHECK(grayBottomBorder >= 50, msg);
    }
    /* GRAY right border: col x=D1C_FRAME_X+D1C_FRAME_W-2, rows
     * [D1C_FRAME_Y+1..D1C_FRAME_Y+D1C_FRAME_H-2).  Height 41. */
    grayRightBorder = rect_count_color(fbA,
                                       D1C_FRAME_X + D1C_FRAME_W - 2,
                                       D1C_FRAME_Y + 1,
                                       1, D1C_FRAME_H - 2,
                                       FRAME_BOTRIGHT_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame GRAY right border has >= 30 pixels "
                 "(got %d / 41)",
                 grayRightBorder);
        CHECK(grayRightBorder >= 30, msg);
    }

    /* ----------------------------------------------------------------
     * Group D - DARK_GRAY interior fill ring around the portrait
     * ----------------------------------------------------------------
     * The C346 frame backing fills the interior (2-pixel inset)
     * with DARK_GRAY.  The C026 portrait (96, 35, 32, 29) sits
     * INSIDE the interior, so the four "ring" strips around the
     * portrait cutout (top, bottom, left, right) must show
     * DARK_GRAY pixels.  This proves the backing is drawn and
     * the C026 portrait does not extend into the backing area. */
    printf("\n[Group D] C346 DARK_GRAY interior fill ring around portrait cutout\n");

    /* Top ring: y=[D1C_FRAME_Y+2, D1C_PORTRAIT_Y) = (62+2..68) =
     * (64..68) framebuffer-relative, 4 rows; x=[D1C_FRAME_X+2,
     * D1C_FRAME_X+D1C_FRAME_W-2) = (82..142), 60 cols.  The
     * portrait cutout is at y=[68, 97) so this strip is fully
     * above the cutout.  4*60 = 240 pixels expected. */
    darkGrayRingTop = rect_count_color(fbA,
                                       D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                                       D1C_FRAME_W - 4, 4,
                                       FRAME_FILL_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame DARK_GRAY top ring (above portrait cutout) "
                 "has >= 100 pixels (got %d / 240)",
                 darkGrayRingTop);
        CHECK(darkGrayRingTop >= 100, msg);
    }
    /* Bottom ring: y=[D1C_PORTRAIT_Y+D1C_PORTRAIT_H,
     * D1C_FRAME_Y+D1C_FRAME_H-2) = (97..70+62-2) wait, let me
     * recompute.  D1C_PORTRAIT_Y = 68, D1C_PORTRAIT_H = 29, so
     * the bottom of the portrait is 97.  D1C_FRAME_Y = 62,
     * D1C_FRAME_H = 43, so D1C_FRAME_Y+D1C_FRAME_H-2 = 103.
     * Bottom ring y=[97, 103), 6 rows; x=[82, 142), 60 cols.
     * 6*60 = 360 pixels expected.  Note: the C040 panel may
     * cover the very bottom, so we only count up to the panel
     * boundary.  C040_PANEL_Y = 33+52 = 85.  So the ring bottom
     * is y=[97, min(103, 85)) which is empty.  We instead
     * sample a smaller range that's above the panel.  Since the
     * C040 panel covers y >= 85 framebuffer-relative, and the
     * portrait bottom is y=97 which is BELOW the panel top, the
     * entire portrait sits within the panel area, so the
     * "bottom ring" is fully covered by the panel.  This is the
     * expected behavior (the panel owns the lower frame ring
     * when no panel interaction is in progress - actually, the
     * panel is only drawn when active, and we are testing
     * panel-off).  Wait, actually, when the panel is OFF, the
     * C040 panel is NOT drawn - the area below the portrait
     * should be DARK_GRAY ring, not panel.  So we can sample
     * y=[97, 103).  6 rows * 60 cols = 360 pixels. */
    darkGrayRingBottom = rect_count_color(fbA,
                                          D1C_FRAME_X + 2,
                                          D1C_PORTRAIT_Y + D1C_PORTRAIT_H,
                                          D1C_FRAME_W - 4,
                                          D1C_FRAME_Y + D1C_FRAME_H - 2 -
                                              (D1C_PORTRAIT_Y + D1C_PORTRAIT_H),
                                          FRAME_FILL_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame DARK_GRAY bottom ring (below portrait cutout) "
                 "has >= 100 pixels (got %d)",
                 darkGrayRingBottom);
        CHECK(darkGrayRingBottom >= 100, msg);
    }
    /* Left ring: x=[D1C_FRAME_X+2, D1C_PORTRAIT_X) = (82..96),
     * 14 cols; y=[D1C_FRAME_Y+2, D1C_FRAME_Y+D1C_FRAME_H-2) =
     * (64..103), 39 rows.  Total 14*39 = 546 pixels expected. */
    darkGrayRingLeft = rect_count_color(fbA,
                                        D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                                        D1C_PORTRAIT_X - (D1C_FRAME_X + 2),
                                        D1C_FRAME_H - 4,
                                        FRAME_FILL_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame DARK_GRAY left ring (left of portrait cutout) "
                 "has >= 200 pixels (got %d / 546)",
                 darkGrayRingLeft);
        CHECK(darkGrayRingLeft >= 200, msg);
    }
    /* Right ring: x=[D1C_PORTRAIT_X+D1C_PORTRAIT_W, D1C_FRAME_X+D1C_FRAME_W-2)
     * = (128..142), 14 cols; y=(64..103), 39 rows.  Total 14*39 = 546
     * pixels expected. */
    darkGrayRingRight = rect_count_color(fbA,
                                         D1C_PORTRAIT_X + D1C_PORTRAIT_W,
                                         D1C_FRAME_Y + 2,
                                         D1C_FRAME_X + D1C_FRAME_W - 2 -
                                             (D1C_PORTRAIT_X + D1C_PORTRAIT_W),
                                         D1C_FRAME_H - 4,
                                         FRAME_FILL_COLOR);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame DARK_GRAY right ring (right of portrait cutout) "
                 "has >= 200 pixels (got %d / 546)",
                 darkGrayRingRight);
        CHECK(darkGrayRingRight >= 200, msg);
    }
    darkGrayRingA = darkGrayRingTop + darkGrayRingBottom +
                    darkGrayRingLeft + darkGrayRingRight;
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame DARK_GRAY total interior ring (sum) >= 600 "
                 "pixels (got %d)",
                 darkGrayRingA);
        CHECK(darkGrayRingA >= 600, msg);
    }

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
     * contained in the (96, 35, 32, 29) cutout.  The C346
     * frame ring around the cutout should have at most a few
     * warm pixels (which can come from the C346 bitmap if it
     * has skin-colored pixels in the mirror frame graphic).
     * Use a generous threshold to allow the C346 bitmap to
     * contribute a small number of warm pixels. */
    printf("\n[Group F] C026 portrait sprite contained in (96, 35, 32, 29) - no warm leak into frame ring\n");

    /* The C346 frame ring is (80, 29, 64, 43) MINUS the portrait
     * cutout (96, 35, 32, 29).  We sample four strips.  Sum the
     * warm pixels in the ring.  Expected: very low (the C346
     * bitmap is mostly gray/black, no skin tones). */
    warmInFrameRing = 0;
    /* Top ring warm: same rect as the top DARK_GRAY ring test. */
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
                 "C346 frame ring has < 50 warm pixels total (got %d) - "
                 "C026 portrait sprite does not leak warm colors into "
                 "the frame ring",
                 warmInFrameRing);
        CHECK(warmInFrameRing < 50, msg);
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
     * Group H - redraw stability: the C346 frame backing is
     * byte-stable across redraws of the same (1,2,0)=19 pose.
     * The C346 bitmap can vary if the runtime re-reads it from
     * the asset cache, but the backing is procedural and must
     * be deterministic.  The C026 portrait sprite is the only
     * thing that can change across redraws (and it shouldn't,
     * since the pose is fixed). */
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
    /* DARK_GRAY ring pixel count must be stable. */
    darkGrayRingB = 0;
    darkGrayRingB += rect_count_color(fbB,
                                      D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                                      D1C_FRAME_W - 4, 4,
                                      FRAME_FILL_COLOR);
    darkGrayRingB += rect_count_color(fbB,
                                      D1C_FRAME_X + 2,
                                      D1C_PORTRAIT_Y + D1C_PORTRAIT_H,
                                      D1C_FRAME_W - 4,
                                      D1C_FRAME_Y + D1C_FRAME_H - 2 -
                                          (D1C_PORTRAIT_Y + D1C_PORTRAIT_H),
                                      FRAME_FILL_COLOR);
    darkGrayRingB += rect_count_color(fbB,
                                      D1C_FRAME_X + 2, D1C_FRAME_Y + 2,
                                      D1C_PORTRAIT_X - (D1C_FRAME_X + 2),
                                      D1C_FRAME_H - 4,
                                      FRAME_FILL_COLOR);
    darkGrayRingB += rect_count_color(fbB,
                                      D1C_PORTRAIT_X + D1C_PORTRAIT_W,
                                      D1C_FRAME_Y + 2,
                                      D1C_FRAME_X + D1C_FRAME_W - 2 -
                                          (D1C_PORTRAIT_X + D1C_PORTRAIT_W),
                                      D1C_FRAME_H - 4,
                                      FRAME_FILL_COLOR);
    diffDarkGrayRing = (darkGrayRingA > darkGrayRingB)
                           ? (darkGrayRingA - darkGrayRingB)
                           : (darkGrayRingB - darkGrayRingA);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "C346 frame DARK_GRAY ring is byte-stable across "
                 "redraws (redraw-A=%d, redraw-B=%d, |diff|=%d <= 5)",
                 darkGrayRingA, darkGrayRingB, diffDarkGrayRing);
        CHECK(diffDarkGrayRing <= 5, msg);
    }

    /* ----------------------------------------------------------------
     * Group I - consolidated no-float invariant
     * ----------------------------------------------------------------
     * Single-line summary of the C346 frame composition +
     * C026 portrait sprite + no-float checks.  If the whole
     * no-float story holds, this line is the gate. */
    printf("\n[Group I] consolidated no-float invariant\n");
    {
        char msg[300];
        snprintf(msg, sizeof(msg),
                 "consolidated no-float: outer-BLACK top=%d bot=%d "
                 "left=%d right=%d corners=%d, top-LG=%d left-LG=%d "
                 "bot-G=%d right-G=%d, ring-DG=%d, match=%d%%, "
                 "warm-portrait=%d, warm-leak=%d (frame is anchored "
                 "at corners, all 4 borders present, ring fill >= 600, "
                 "portrait sprite fully drawn, no warm leak)",
                 blackOuterTopEdge, blackOuterBottomEdge,
                 blackOuterLeftEdge, blackOuterRightEdge,
                 blackOuterCorners,
                 lightGrayTopBorder, lightGrayLeftBorder,
                 grayBottomBorder, grayRightBorder,
                 darkGrayRingA, matchPortrait, warmPortrait,
                 warmInFrameRing);
        CHECK(blackOuterTopEdge >= 50 &&
              blackOuterBottomEdge >= 50 &&
              blackOuterLeftEdge >= 35 &&
              blackOuterRightEdge >= 35 &&
              blackOuterCorners >= 2 &&
              lightGrayTopBorder >= 50 &&
              lightGrayLeftBorder >= 30 &&
              grayBottomBorder >= 50 &&
              grayRightBorder >= 30 &&
              darkGrayRingA >= 600 &&
              matchPortrait >= 90 &&
              warmPortrait >= 100 &&
              warmInFrameRing < 50, msg);
    }

    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&state);
    return g_fail == 0 ? 0 : 1;
}
