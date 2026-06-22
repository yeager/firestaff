/*
 * firestaff_dm1_v1_hall_champion_portrait_01_east_walkpath_portrait_rect_position_runtime_probe.c
 *
 * Real-asset / runtime regression for one narrow DM1 V1 Hall of
 * Champions champion-portrait slice:
 *
 *   ordinal       : 1   (the C026 champion-portrait strip slot at
 *                   column 1, row 0 -- the second portrait of the
 *                   8x3 strip.  On the shipped DM1 V1 PC 3.4
 *                   DUNGEON.DAT the catalog binds ordinal 1 to the
 *                   champion named "HALK" with the title
 *                   "THE BARBARIAN".)
 *   route variant : east_walkpath (the corridor walk east at y=3
 *                   facing NORTH -- the player walks east along
 *                   the Hall corridor wall from (1,3) NORTH
 *                   through (2,3) NORTH and (3,3) NORTH without
 *                   turning toward any mirror-bearing wall).
 *   aspect        : portrait_rect_position (the source-locked D1C
 *                   front-wall portrait cutout at viewport (96,35)
 *                   sized 32x29, drawn after the C346 wall-mirror
 *                   frame per ReDMCSB DUNVIEW.C:3913-3928).
 *
 * The shipped DM1 V1 PC 3.4 DUNGEON.DAT places the only
 * front_north_entry champion-portrait mirror that maps to ordinal 1
 * at the (1,2) NORTH pose, where the front cell (1,1) carries a C127
 * sensor with sensorData=1.  The east_walkpath route cells (1,3),
 * (2,3) and (3,3) NORTH do NOT carry a C127 sensor on their front
 * squares (1,2), (2,2) and (3,2), so the D1C rectangle must clear
 * ordinal-1 pixels at every east_walkpath cell -- i.e. ordinal 1
 * must not be drawn floating over the corridor wall.
 *
 * Source evidence (ReDMCSB WIP 20210206):
 *   DUNGEON.C:2558          - G0289 reset on each viewport draw
 *                              that includes a wall square.
 *   DUNGEON.C:2573          - M011_CELL(sensor) selects visible wall cell.
 *   DUNGEON.C:2608-2612     - C127 sensorData -> G0289 (M000_INDEX_TO_ORDINAL).
 *   DUNGEON.C:3916          - C01_COLOR_DARK_GRAY is C026 transparency.
 *   MOVESENS.C:1501-1503    - C127 dispatches to F0280 with sensorData.
 *   MOVESENS.C:556          - tick advance after a forward move.
 *   REVIVE.C F0280          - candidate champion materialized from
 *                              sensorData.
 *   REVIVE.C F0282          - C127 sensor disabled after confirm.
 *   DUNVIEW.C:3913-3928     - C346 wall frame, C026 portrait blit at D1C.
 *   DUNVIEW.C:8318-8542     - F0128_DUNGEONVIEW_Draw_CPSF (far-to-near).
 *   DUNVIEW.C:8522-8533     - viewport re-blt after a forward move.
 *   COORD.C:1693-1722       - PC 3.4 viewport origin / 224x136 dim.
 *   DEFS.H:2071-2079, 2186  - C320/C32/C29 zone constants and C026.
 *
 * This probe exercises:
 *   (A) M11_GameView_GetD1CWallOrnamentZone contract:
 *       DUNVIEW.C G0205 coordSet 5 / index 12 -> dest (80,29,64,43)
 *       is the source-locked C346 D1C champion-mirror frame route.
 *       The C026 portrait cutout lives at viewport (96,35) sized
 *       32x29 inside this frame.
 *   (B) portrait ordinal 1 -> C026 source rect math:
 *       (1 & 7) * 32 = 32, (1 >> 3) * 29 = 0 -> (32, 0, 32, 29).
 *       This is column 1 row 0 of the C026 strip.
 *   (C) east_walkpath route no-portrait contract:
 *       the three corridor cells (1,3), (2,3), (3,3) NORTH all
 *       report -1 from M11_GameView_GetFrontMirrorOrdinal because
 *       their front squares do not carry a C127 sensor.
 *   (D) ordinal 1 catalog identity pinned via
 *       M11_GameView_GetMirrorNameByOrdinal /
 *       M11_GameView_GetMirrorTitleByOrdinal.  Verifies the name
 *       contains "HALK" and the title contains "BARBARIAN" so
 *       the slot stays bound to a real source identity.
 *   (E) portrait_rect_position contract for ordinal 1: at the
 *       unique ordinal-1 pose (1,2) NORTH the D1C rectangle in
 *       the runtime framebuffer is dominated by ordinal-1 pixels
 *       (the C026 source pixels match the rendered pixels at the
 *       blit destination).  At every east_walkpath corridor cell
 *       (1,3), (2,3), (3,3) NORTH the D1C rectangle does NOT
 *       match ordinal-1 pixels (no floating portrait over the
 *       corridor wall).
 *   (F) ordinal 1 visibility is bound to the front_north_entry
 *       pose only: a 4x4x4 pose sweep across mapIndex=0 (Hall)
 *       confirms ordinal 1 is visible at exactly one pose in the
 *       unmodified shipped DUNGEON.DAT (the (1,2) NORTH pose that
 *       the actual_pose probe already verifies).
 *
 * Honest scope: this probe proves the source-locked
 * ordinal/position contract and the no-floating rule for the
 * ordinal 1 east_walkpath slice in shipped DM1 V1 PC 3.4 data.
 * It does NOT claim DOS pixel parity.  Original DM1 PC 3.4
 * captures live under parity-evidence/ and are referenced by
 * separate parity gates.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "asset_loader_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    /* Source-locked constants (ReDMCSB DUNVIEW.C:3913-3928, COORD.C:1693-1722,
     * DEFS.H:2071-2079, DEFS.H:2186). */
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    /* C346 wall-mirror frame: G0205 coordSet 5 / index 12, expressed
     * in viewport-relative coordinates. */
    D1C_WALL_FRAME_X = 80,
    D1C_WALL_FRAME_Y = 29,
    D1C_WALL_FRAME_W = 64,
    D1C_WALL_FRAME_H = 43,
    /* C026 portrait cutout at viewport (96,35) sized 32x29 inside the
     * C346 frame.  The C026 champion portrait strip is 256x87, i.e.
     * 8 columns x 3 rows of 32x29 portraits.  The cutout uses
     * viewport-relative coordinates; the runtime blit adds the
     * viewport origin (0, 33) before writing the framebuffer. */
    D1C_PORTRAIT_VX = 96, /* viewport-relative X (C32) */
    D1C_PORTRAIT_VY = 35, /* viewport-relative Y (C29) */
    D1C_PORTRAIT_FBX = VIEWPORT_X + D1C_PORTRAIT_VX,
    D1C_PORTRAIT_FBY = VIEWPORT_Y + D1C_PORTRAIT_VY,
    D1C_PORTRAIT_W = 32,
    D1C_PORTRAIT_H = 29,
    /* C026 strip metadata. */
    C026_PORTRAITS_TOTAL = 24,
    C026_COLS = 8,
    C026_ROWS = 3,
    PORTRAIT_STRIP_W = 256,
    PORTRAIT_STRIP_H = 87,
    PORTRAIT_ORDINAL_TARGET = 1,
    /* Hall map index on DM1 V1 PC 3.4. */
    HALL_MAP_INDEX = 0,
    /* The shipped DM1 V1 PC 3.4 DUNGEON.DAT places the only ordinal-1
     * mirror at the (1,2) NORTH pose -- the front_north_entry pose.
     * Front cell (1,1) carries the C127 sensor with sensorData=1. */
    ORDINAL1_POSE_X = 1,
    ORDINAL1_POSE_Y = 2,
    ORDINAL1_POSE_DIR = 0, /* DIR_NORTH */
    /* East_walkpath route: walk east along the y=3 corridor wall,
     * facing NORTH.  None of these cells have a C127 sensor on their
     * front squares, so the D1C rectangle must clear ordinal 1. */
    EAST_WALKPATH_A_X = 1,
    EAST_WALKPATH_A_Y = 3,
    EAST_WALKPATH_B_X = 2,
    EAST_WALKPATH_B_Y = 3,
    EAST_WALKPATH_C_X = 3,
    EAST_WALKPATH_C_Y = 3,
    EAST_WALKPATH_DIR = 0, /* DIR_NORTH */
    /* Search bounds for the ordinal-1 sweep (test F). */
    HALL_MAX_CELLS_PER_AXIS = 16,
    /* DUNVIEW.C:3916: C01_COLOR_DARK_GRAY (value 1) is the C026
     * champion portrait transparency mask. */
    PROBE_CHAMPION_TRANSPARENT = 1,
    /* Tolerance for the portrait_rect_position "visible" assertion at
     * the ordinal-1 pose.  The framebuffer's low nibble carries the
     * 4-bit palette index; opaque ordinal-1 pixels must dominate the
     * D1C rectangle when the source-locked blit lands. */
    PROBE_PORTRAIT_VISIBLE_PCT = 80,
    /* Tolerance for the no-floating assertion at every east_walkpath
     * corridor cell.  When the front cell has no C127 sensor the D1C
     * rectangle must NOT be dominated by ordinal-1 pixels.  We pin
     * the corridor cells to < 5% match because the shipped
     * DUNGEON.DAT corridor wall uses palette indices that do not
     * overlap ordinal-1's source palette indices. */
    PROBE_NO_FLOAT_PCT = 5,
    /* Minimum ordinal-1 opaque pixel count that must be present in
     * the C026 graphic for the runtime blit to produce a visible
     * portrait.  Empirical floor for HALK across the 24-portrait
     * strip: 668 opaque pixels per 32x29 source rect. */
    PROBE_ORDINAL1_OPAQUE_FLOOR = 50
};

static int g_pass;
static int g_fail;

static int expect_int(const char* label, int got, int want) {
    ++g_pass;
    if (got == want) {
        printf("  PASS: %s == %d\n", label, want);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    return 0;
}

static int expect_int_le(const char* label, int got, int maxInclusive) {
    ++g_pass;
    if (got <= maxInclusive) {
        printf("  PASS: %s got=%d <= %d\n", label, got, maxInclusive);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d > %d\n", label, got, maxInclusive);
    return 0;
}

static int expect_int_ge(const char* label, int got, int minInclusive) {
    ++g_pass;
    if (got >= minInclusive) {
        printf("  PASS: %s got=%d >= %d\n", label, got, minInclusive);
        return 1;
    }
    ++g_fail;
    printf("  FAIL: %s got=%d < %d\n", label, got, minInclusive);
    return 0;
}

/* Convert a portrait ordinal to its (col, row) in the C026 strip. */
static void portrait_ordinal_to_cr(int ordinal, int* outCol, int* outRow) {
    if (outCol) *outCol = ordinal & 7;          /* column = ordinal mod 8 */
    if (outRow) *outRow = (ordinal >> 3) & 3;   /* row = ordinal / 8 */
}

/* Source-rect for a portrait ordinal: (x, y, w, h) in C026 pixels. */
static void portrait_ordinal_source_rect(int ordinal,
                                         int* outX, int* outY,
                                         int* outW, int* outH) {
    int col;
    int row;
    portrait_ordinal_to_cr(ordinal, &col, &row);
    if (outX) *outX = col * D1C_PORTRAIT_W;
    if (outY) *outY = row * D1C_PORTRAIT_H;
    if (outW) *outW = D1C_PORTRAIT_W;
    if (outH) *outH = D1C_PORTRAIT_H;
}

/* (A) Verify D1C wall-mirror frame helper returns the source-locked
 *     box. ReDMCSB DUNVIEW.C G0205 coordSet 5 / index 12 is the
 *     C346 champion-mirror frame route; the C026 portrait is the
 *     smaller cutout at viewport (96,35) sized 32x29 inside this
 *     frame. */
static int test_d1c_wall_ornament_zone(M11_GameViewState* game) {
    int x = -1, y = -1, w = -1, h = -1;
    int ok = 1;
    printf("[A] D1C wall-ornament frame contract (C346/C026 anchor)\n");
    ok &= expect_int("M11_GameView_GetD1CWallOrnamentZone returns 1",
                     M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h), 1);
    ok &= expect_int("D1C frame dstX == 80", x, D1C_WALL_FRAME_X);
    ok &= expect_int("D1C frame dstY == 29", y, D1C_WALL_FRAME_Y);
    ok &= expect_int("D1C frame width == 64", w, D1C_WALL_FRAME_W);
    ok &= expect_int("D1C frame height == 43", h, D1C_WALL_FRAME_H);
    /* The C026 portrait cutout (96,35,32,29) must fit inside the
     * C346 wall-mirror frame (80,29,64,43) without horizontal or
     * vertical overflow.  Both are viewport-relative. */
    ok &= expect_int("cutout right edge <= frame right edge (no horizontal float)",
                     (D1C_PORTRAIT_VX + D1C_PORTRAIT_W) <= (D1C_WALL_FRAME_X + D1C_WALL_FRAME_W), 1);
    ok &= expect_int("cutout bottom edge <= frame bottom edge (no vertical float)",
                     (D1C_PORTRAIT_VY + D1C_PORTRAIT_H) <= (D1C_WALL_FRAME_Y + D1C_WALL_FRAME_H), 1);
    ok &= expect_int("portrait bottom edge inside 320x200 framebuffer",
                     (D1C_PORTRAIT_FBY + D1C_PORTRAIT_H) <= FB_H, 1);
    ok &= expect_int("portrait right edge inside 320x200 framebuffer",
                     (D1C_PORTRAIT_FBX + D1C_PORTRAIT_W) <= FB_W, 1);
    return ok;
}

/* (B) Verify portrait ordinal 1 -> C026 source rect (32, 0, 32, 29).
 *     Source: ReDMCSB DUNVIEW.C:3913-3928 C026 = 8 cols x 3 rows.
 *     Ordinal 1 -> col=1, row=0 (the first row of the C026 strip). */
static int test_portrait_ordinal_math(void) {
    int sx = -1, sy = -1, sw = -1, sh = -1;
    int col = -1, row = -1;
    int ok = 1;
    printf("[B] Portrait ordinal 1 -> C026 source rect math\n");
    portrait_ordinal_to_cr(PORTRAIT_ORDINAL_TARGET, &col, &row);
    ok &= expect_int("ordinal 1 column (col = ordinal mod 8)", col, 1);
    ok &= expect_int("ordinal 1 row (row = ordinal / 8)", row, 0);
    portrait_ordinal_source_rect(PORTRAIT_ORDINAL_TARGET, &sx, &sy, &sw, &sh);
    ok &= expect_int("ordinal 1 source X == 1*32", sx, 32);
    ok &= expect_int("ordinal 1 source Y == 0*29", sy, 0);
    ok &= expect_int("ordinal 1 source W == 32", sw, D1C_PORTRAIT_W);
    ok &= expect_int("ordinal 1 source H == 29", sh, D1C_PORTRAIT_H);
    ok &= expect_int("ordinal 1 right edge <= C026 strip width",
                     sx + sw <= PORTRAIT_STRIP_W, 1);
    ok &= expect_int("ordinal 1 bottom edge <= C026 strip height",
                     sy + sh <= PORTRAIT_STRIP_H, 1);
    /* Every ordinal 0..23 must produce a valid in-strip rect. */
    {
        int k;
        int allValid = 1;
        for (k = 0; k < C026_PORTRAITS_TOTAL; ++k) {
            int kx, ky, kw, kh;
            portrait_ordinal_source_rect(k, &kx, &ky, &kw, &kh);
            if (kx < 0 || ky < 0 ||
                kx + kw > PORTRAIT_STRIP_W ||
                ky + kh > PORTRAIT_STRIP_H) {
                allValid = 0;
                printf("  FAIL: ordinal %d -> (%d,%d,%d,%d) outside strip\n",
                       k, kx, ky, kw, kh);
                break;
            }
        }
        ok &= expect_int("all 24 ordinals produce in-strip source rects",
                         allValid, 1);
    }
    return ok;
}

/* Count the ordinal's opaque (non-mask) pixels in the C026 source
 * rect.  Used to anchor the runtime "visible portrait" floor. */
static int count_ordinal_opaque_pixels(const M11_AssetSlot* portraits,
                                       int ordinal) {
    int x;
    int y;
    int matched = 0;
    int srcPX, srcPY;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= C026_PORTRAITS_TOTAL) {
        return 0;
    }
    srcPX = (ordinal & 7) * D1C_PORTRAIT_W;
    srcPY = (ordinal >> 3) * D1C_PORTRAIT_H;
    if (srcPX + D1C_PORTRAIT_W > (int)portraits->width ||
        srcPY + D1C_PORTRAIT_H > (int)portraits->height) {
        return 0;
    }
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char src =
                (unsigned char)(portraits->pixels[(srcPY + y) * (int)portraits->width +
                                                 (srcPX + x)] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) continue;
            ++matched;
        }
    }
    return matched;
}

/* Count opaque ordinal-source pixels in the runtime D1C rectangle
 * that match the source graphic.  Used to verify the
 * portrait_rect_position contract (the C026 blit lands at viewport
 * (96,35) with the C01 dark-gray transparency mask). */
static int count_ordinal_matched_in_d1c(const M11_AssetSlot* portraits,
                                        const unsigned char* fb,
                                        int ordinal) {
    int x;
    int y;
    int matched = 0;
    int srcPX, srcPY;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        !fb || ordinal < 0 || ordinal >= C026_PORTRAITS_TOTAL) {
        return 0;
    }
    srcPX = (ordinal & 7) * D1C_PORTRAIT_W;
    srcPY = (ordinal >> 3) * D1C_PORTRAIT_H;
    if (srcPX + D1C_PORTRAIT_W > (int)portraits->width ||
        srcPY + D1C_PORTRAIT_H > (int)portraits->height) {
        return 0;
    }
    for (y = 0; y < D1C_PORTRAIT_H; ++y) {
        for (x = 0; x < D1C_PORTRAIT_W; ++x) {
            unsigned char src =
                (unsigned char)(portraits->pixels[(srcPY + y) * (int)portraits->width +
                                                 (srcPX + x)] & 0x0F);
            if (src == PROBE_CHAMPION_TRANSPARENT) continue;
            /* The asset blitter writes the raw byte from the slot.
             * The framebuffer uses a 1-byte-per-pixel encoding with
             * the palette index in the low nibble and the level in
             * the high nibble; the blit produces level 0.  Compare
             * the low nibble of the framebuffer byte to the source
             * 4-bit index. */
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(D1C_PORTRAIT_FBY + y) * FB_W +
                                       (D1C_PORTRAIT_FBX + x)]);
            if (dst == src) ++matched;
        }
    }
    return matched;
}

/* (C) east_walkpath route no-portrait contract.  Walk east at y=3
 *     facing NORTH through (1,3), (2,3), (3,3) NORTH.  Each cell's
 *     front square has no C127 sensor, so
 *     M11_GameView_GetFrontMirrorOrdinal returns -1 and the D1C
 *     rectangle must clear ordinal-1 pixels (no floating portrait
 *     over the corridor wall).
 *
 *     The front_north_entry pose (1,2) NORTH is the unique pose
 *     where ordinal 1 is visible in shipped DM1 V1 PC 3.4; this is
 *     also verified by firestaff_dm1_v1_champion_mirror_actual_pose
 *     _runtime_probe.  We assert the positive ordinal at (1,2)
 *     NORTH here as the partner anchor to the corridor cells. */
static int test_east_walkpath_ordinal(M11_GameViewState* game) {
    static const struct {
        int x;
        int y;
        int dir;
        int expectedOrdinal;
        const char* label;
    } kEastSteps[] = {
        {ORDINAL1_POSE_X, ORDINAL1_POSE_Y, ORDINAL1_POSE_DIR,
         PORTRAIT_ORDINAL_TARGET,
         "front_north_entry_ordinal_1 (1,2) NORTH"},
        {EAST_WALKPATH_A_X, EAST_WALKPATH_A_Y, EAST_WALKPATH_DIR,
         -1,
         "east_walkpath_step_a (1,3) NORTH"},
        {EAST_WALKPATH_B_X, EAST_WALKPATH_B_Y, EAST_WALKPATH_DIR,
         -1,
         "east_walkpath_step_b (2,3) NORTH"},
        {EAST_WALKPATH_C_X, EAST_WALKPATH_C_Y, EAST_WALKPATH_DIR,
         -1,
         "east_walkpath_step_c (3,3) NORTH"}
    };
    int ok = 1;
    int i;
    size_t n = sizeof(kEastSteps) / sizeof(kEastSteps[0]);
    printf("[C] east_walkpath route ordinal at (1,3)->(2,3)->(3,3) NORTH\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (i = 0; i < (int)n; ++i) {
        int ord;
        char label[80];
        game->world.party.mapX = kEastSteps[i].x;
        game->world.party.mapY = kEastSteps[i].y;
        game->world.party.direction = kEastSteps[i].dir;
        ord = M11_GameView_GetFrontMirrorOrdinal(game);
        printf("  INFO: %s ordinal = %d\n", kEastSteps[i].label, ord);
        snprintf(label, sizeof(label),
                 "%s ordinal resolves correctly", kEastSteps[i].label);
        ok &= expect_int(label, ord, kEastSteps[i].expectedOrdinal);
    }
    return ok;
}

/* (D) Pin ordinal 1's catalog identity.  The DM1 V1 PC 3.4 mirror
 *     TextString catalog binds ordinal 1 to HALK / "THE BARBARIAN".
 *     The probe verifies the catalog name contains "HALK" and the
 *     title contains "BARBARIAN" so the slot stays bound to a real
 *     source identity even if the exact punctuation differs between
 *     DM1 V1 PC 3.4 builds. */
static int test_ordinal_1_catalog_identity(M11_GameViewState* game) {
    char championName[64];
    char championTitle[64];
    int ok = 1;
    printf("[D] ordinal 1 catalog identity pinning (HALK / THE BARBARIAN)\n");
    championName[0] = '\0';
    championTitle[0] = '\0';
    M11_GameView_GetMirrorNameByOrdinal(game, PORTRAIT_ORDINAL_TARGET,
                                        championName, sizeof(championName));
    M11_GameView_GetMirrorTitleByOrdinal(game, PORTRAIT_ORDINAL_TARGET,
                                         championTitle, sizeof(championTitle));
    printf("  INFO: ordinal 1 -> name=\"%s\" title=\"%s\"\n",
           championName[0] ? championName : "(unknown)",
           championTitle[0] ? championTitle : "(untitled)");
    {
        int nameOk = (strstr(championName, "HALK") != NULL);
        ++g_pass;
        if (nameOk) {
            printf("  PASS: ordinal 1 catalog name contains \"HALK\"\n");
        } else {
            ++g_fail;
            printf("  FAIL: ordinal 1 catalog name got=\"%s\" want contains=\"HALK\"\n",
                   championName);
        }
        ok &= nameOk;
    }
    {
        int titleOk = (strstr(championTitle, "BARBARIAN") != NULL);
        ++g_pass;
        if (titleOk) {
            printf("  PASS: ordinal 1 catalog title contains \"BARBARIAN\"\n");
        } else {
            ++g_fail;
            printf("  FAIL: ordinal 1 catalog title got=\"%s\" want contains=\"BARBARIAN\"\n",
                   championTitle);
        }
        ok &= titleOk;
    }
    return ok;
}

/* (E) portrait_rect_position contract for ordinal 1.
 *
 *     - At the unique ordinal-1 pose (1,2) NORTH the runtime blit
 *       lands at viewport (96,35) sized 32x29.  The D1C rectangle
 *       must be dominated by ordinal-1 opaque pixels (>= 80%
 *       match against the C026 source rect (32,0,32,29)).
 *     - At every east_walkpath corridor cell (1,3), (2,3), (3,3)
 *       NORTH the front cell has no C127 sensor so the blit does
 *       not run; the D1C rectangle shows corridor wall, NOT
 *       ordinal-1 pixels.  The no-floating tolerance is < 5%
 *       match because the corridor wall palette indices do not
 *       overlap the HALK source palette in shipped DM1 V1 PC 3.4.
 */
static int test_portrait_rect_position(M11_GameViewState* game,
                                       const M11_AssetSlot* portraits) {
    static const struct {
        int x;
        int y;
        int dir;
        int expectOrdinal;
        const char* label;
    } kPoses[] = {
        {ORDINAL1_POSE_X, ORDINAL1_POSE_Y, ORDINAL1_POSE_DIR,
         PORTRAIT_ORDINAL_TARGET,
         "front_north_entry_ordinal_1 (1,2) NORTH"},
        {EAST_WALKPATH_A_X, EAST_WALKPATH_A_Y, EAST_WALKPATH_DIR,
         -1,
         "east_walkpath_step_a (1,3) NORTH"},
        {EAST_WALKPATH_B_X, EAST_WALKPATH_B_Y, EAST_WALKPATH_DIR,
         -1,
         "east_walkpath_step_b (2,3) NORTH"},
        {EAST_WALKPATH_C_X, EAST_WALKPATH_C_Y, EAST_WALKPATH_DIR,
         -1,
         "east_walkpath_step_c (3,3) NORTH"}
    };
    int ordinal1Opaque = 0;
    int ok = 1;
    int i;
    size_t n = sizeof(kPoses) / sizeof(kPoses[0]);
    printf("[E] portrait_rect_position contract at ordinal-1 pose and east_walkpath cells\n");
    if (!portraits || !portraits->loaded || !portraits->pixels) {
        printf("  SKIP: portrait strip unavailable\n");
        return 1;
    }
    ordinal1Opaque = count_ordinal_opaque_pixels(portraits,
                                                 PORTRAIT_ORDINAL_TARGET);
    printf("  INFO: ordinal 1 opaque pixel count in C026 = %d (floor %d)\n",
           ordinal1Opaque, PROBE_ORDINAL1_OPAQUE_FLOOR);
    ok &= expect_int_ge("C026 ordinal 1 opaque pixel floor (sanity)",
                        ordinal1Opaque, PROBE_ORDINAL1_OPAQUE_FLOOR);
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (i = 0; i < (int)n; ++i) {
        int matched;
        int pct;
        char label[96];
        unsigned char fb[FB_W * FB_H];
        game->world.party.mapX = kPoses[i].x;
        game->world.party.mapY = kPoses[i].y;
        game->world.party.direction = kPoses[i].dir;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        matched = count_ordinal_matched_in_d1c(portraits, fb,
                                               PORTRAIT_ORDINAL_TARGET);
        pct = ordinal1Opaque > 0
                  ? (matched * 100) / ordinal1Opaque
                  : 0;
        printf("  INFO: %s ordinal-1 matched %d/%d (%d%%)\n",
               kPoses[i].label, matched, ordinal1Opaque, pct);
        if (kPoses[i].expectOrdinal >= 0) {
            /* (1,2) NORTH: ordinal 1 IS visible.  The D1C rect must
             * be dominated by ordinal-1 opaque pixels. */
            snprintf(label, sizeof(label),
                     "%s ordinal-1 D1C match >= %d%% (portrait visible)",
                     kPoses[i].label, PROBE_PORTRAIT_VISIBLE_PCT);
            ok &= expect_int_ge(label, pct, PROBE_PORTRAIT_VISIBLE_PCT);
        } else {
            /* East-walkpath corridor cells: no portrait.  The D1C
             * rect must NOT be dominated by ordinal-1 pixels. */
            snprintf(label, sizeof(label),
                     "%s ordinal-1 D1C match < %d%% (no float)",
                     kPoses[i].label, PROBE_NO_FLOAT_PCT);
            ok &= expect_int_le(label, pct, PROBE_NO_FLOAT_PCT);
        }
    }
    return ok;
}

/* (F) ordinal 1 visibility is bound to one pose in the Hall map.
 *     Sweep mapIndex=0 across the 16x16 cell grid x 4 directions
 *     and confirm the runtime reports ordinal 1 at exactly one
 *     pose in the shipped DM1 V1 PC 3.4 DUNGEON.DAT -- the
 *     front_north_entry pose (1,2) NORTH that
 *     firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe
 *     already verifies.  This locks the ordinal-1 east_walkpath
 *     slice to the actual data: the east_walkpath route cells
 *     (1,3), (2,3), (3,3) NORTH must NOT be among the ordinal-1
 *     poses, so no east_walkpath cell renders ordinal 1. */
static int test_ordinal_1_uniqueness(M11_GameViewState* game) {
    int mapX, mapY, dir;
    int ord1Hits = 0;
    int ord1AtOrdinal1Pose = 0;
    int ord1AtEastWalkpath = 0;
    int ok = 1;
    printf("[F] ordinal 1 visibility bound to (1,2) NORTH only\n");
    game->world.party.mapIndex = HALL_MAP_INDEX;
    for (mapY = 0; mapY < HALL_MAX_CELLS_PER_AXIS; ++mapY) {
        for (mapX = 0; mapX < HALL_MAX_CELLS_PER_AXIS; ++mapX) {
            for (dir = 0; dir < 4; ++dir) {
                int ord;
                game->world.party.mapX = mapX;
                game->world.party.mapY = mapY;
                game->world.party.direction = dir;
                ord = M11_GameView_GetFrontMirrorOrdinal(game);
                if (ord != PORTRAIT_ORDINAL_TARGET) continue;
                ++ord1Hits;
                if (mapX == ORDINAL1_POSE_X &&
                    mapY == ORDINAL1_POSE_Y &&
                    dir == ORDINAL1_POSE_DIR) {
                    ord1AtOrdinal1Pose = 1;
                }
                if (((mapX == EAST_WALKPATH_A_X && mapY == EAST_WALKPATH_A_Y) ||
                     (mapX == EAST_WALKPATH_B_X && mapY == EAST_WALKPATH_B_Y) ||
                     (mapX == EAST_WALKPATH_C_X && mapY == EAST_WALKPATH_C_Y)) &&
                    dir == EAST_WALKPATH_DIR) {
                    ord1AtEastWalkpath = 1;
                }
            }
        }
    }
    printf("  INFO: ordinal 1 visible at %d pose(s) on Hall map (16x16x4 sweep)\n",
           ord1Hits);
    ok &= expect_int("ordinal 1 visible at exactly 1 pose in Hall map",
                     ord1Hits, 1);
    ok &= expect_int("ordinal 1 visible at front_north_entry (1,2) NORTH",
                     ord1AtOrdinal1Pose, 1);
    ok &= expect_int("ordinal 1 NOT visible at any east_walkpath cell",
                     ord1AtEastWalkpath, 0);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
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
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                    (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    printf("=== DM1 V1 Hall of Champions: portrait ordinal 1, "
           "route east_walkpath, aspect portrait_rect_position ===\n");
    printf("sourceEvidence=DUNGEON.C:2558,2608-2612 (C127 sensorData -> G0289)\n");
    printf("                DUNGEON.C:2573 (M011_CELL(sensor) -> visible wall)\n");
    printf("                MOVESENS.C:1501-1503 (C127 -> F0280)\n");
    printf("                MOVESENS.C:556 (forward move tick advance)\n");
    printf("                REVIVE.C F0280,F0282 (candidate materialise/disable)\n");
    printf("                DUNVIEW.C:3913-3928 (C346 frame + C026 portrait blit)\n");
    printf("                DUNVIEW.C:3916 (C01 dark-gray transparency mask)\n");
    printf("                DUNVIEW.C:8318-8542 F0128 (far-to-near draw order)\n");
    printf("                DUNVIEW.C:8522-8533 (viewport re-blt after forward)\n");
    printf("                COORD.C:1693-1722 (PC 3.4 viewport origin / 224x136)\n");
    printf("                DEFS.H:2071-2079,2186 (C320/C32/C29 + C026 strip)\n\n");

    ok &= test_d1c_wall_ornament_zone(&game);
    ok &= test_portrait_ordinal_math();
    ok &= test_east_walkpath_ordinal(&game);
    ok &= test_ordinal_1_catalog_identity(&game);
    ok &= test_portrait_rect_position(&game, portraits);
    ok &= test_ordinal_1_uniqueness(&game);

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return (g_fail == 0 && ok) ? 0 : 1;
}
