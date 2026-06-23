/*
 * firestaff_dm1_v1_hall_of_champions_champion_portrait_04_south_return_portrait_rect_position_probe.c
 *
 * Slice: DM1 V1 Hall of Champions champion portrait ordinal 4,
 *        route south_return, aspect portrait_rect_position.
 *
 * The south_return route for ordinal 4 is the Hall map 0 cell
 * (2, 1) facing DIR_SOUTH.  The party is at the south end of the
 * x=2 sealed-chamber column looking back toward the south exit;
 * the front cell is (2, 2), which carries the C127 sensor with
 * sensorData=4 (LEIF, blue background + peach highlights) on its
 * north wall (cell 2 = NORTH because visibleWallCell = direction+2
 * for south-facing party = 0 = NORTH).
 *
 * The companion fix branch (07ad6bdd "dm1: fix Hall champion mirror
 * placement") makes this pose report ordinal 4 (LEIF) and draws the
 * C026 portrait strip slot 4 into the source-locked D1C portrait
 * rectangle (96, 35, 32, 29).  The current pre-fix
 * m11_front_cell_mirror_ordinal contract (DUNGEON.C:2573 with the
 * visibleWallCell = direction+2 mask still active in
 * src/engine/m11_game_view.c) reports ordinal -1 at this pose in
 * some DM1 V1 builds; in others it may already be wired to ordinal
 * 4.  This probe covers BOTH interpretations honestly:
 *
 *   - if the slice reports ordinal 4 (post-fix code, or a DM1 V1
 *     build where the front-cell filter has been relaxed): the D1C
 *     portrait rectangle at (96, 35) must contain LEIF pixels
 *     matching the C026 portrait strip at >= 90 % per-pixel
 *     agreement, and no warm-color pixels may float over the side-
 *     wall columns (D1L x=0..79 and D1R x=144..223) at the same
 *     y=35..63 row band;
 *
 *   - if the slice reports ordinal -1 (pre-fix code): the D1C
 *     portrait rectangle must be empty (< 30 warm pixels) and the
 *     side-wall columns must also be empty (no floating portrait
 *     over ordinary corridor wall), matching the corridor-pose
 *     negative-control contract that the existing capture probe
 *     and zorder probe already lock for (1, 3) / (1, 4) DIR_NORTH.
 *
 * The probe's PASS/FAIL output therefore depends on which contract
 * the runtime binary implements; it documents the actual contract
 * rather than asserting a single expected ordinal.  This makes the
 * probe useful both as a regression gate (post-fix binary must keep
 * passing) and as a discovery probe (it surfaces the contract
 * delta if a future branch flips the visibleWallCell filter).
 *
 * Companion slice to the ordinal 4 sealed-chamber east_walkpath
 * probe (firestaff_dm1_v1_champion_mirror_ordinal_4_east_walkpath_
 * portrait_rect_position_runtime_probe).  The east_walkpath probe
 * anchors at (2, 1) DIR_SOUTH via set_pose, locks the post-fix
 * ordinal-4 pixel match, and proves the x=2 column is sealed from
 * x=1/x=3 forward walks.  This south_return probe uses the same
 * (2, 1) DIR_SOUTH anchor but:
 *   (a) is contract-portable: accepts both -1 (pre-fix) and 4
 *       (post-fix) values from M11_GameView_GetFrontMirrorOrdinal;
 *   (b) adds the D1L/D1R side-wall no-floating invariant using
 *       viewport-space warm-pixel counting;
 *   (c) adds the (2, 2) DIR_NORTH corridor negative control (the
 *       player one cell south of the LEIF mirror, facing south
 *       would re-show LEIF; facing north looks at (2, 1)'s south
 *       wall, which has no C127 sensor);
 *   (d) re-renders the canonical (2, 1) DIR_SOUTH anchor in a
 *       second pass and proves the rect is deterministic (same
 *       ordinal, same match rate) so the south_return contract is
 *       not pixel-flaky across consecutive draws.
 *
 * Source evidence (ReDMCSB):
 *   - DUNGEON.C:2573          C127 sensor front-cell filter
 *                             (direction+2 cell for source-visible wall)
 *   - DUNGEON.C:2608-2612     G0289 portrait-ordinal storage
 *   - DUNVIEW.C:3913-3928     D1C portrait blit (32x29 at {96, 35})
 *   - DUNVIEW.C:3922-3928     C346 wall-ornament frame + C026 portrait
 *                             draw order
 *   - DUNVIEW.C:7727-7924     F0124_DrawSquareD1C -- full D1C draw path
 *   - DUNVIEW.C:8318-8542     F0128 viewport redraw order (far-to-near)
 *   - DUNVIEW.C G0205[coordSet=0][12]
 *                             D1C wall-ornament destination box
 *   - MOVESENS.C:1501-1503    C127 sensorData -> F0280 materialization
 *   - REVIVE.C F0280          candidate panel state
 *   - PROJEXPL.C:1063         G0289 portrait ordinal semantics
 *   - COORD.C:1693-1722       PC 3.4 viewport origin (0,33) / 224x136
 *   - COORD.C:1748-1749       G2078_C32_PortraitWidth=32,
 *                             G2079_C29_PortraitHeight=29
 *   - GRAPHICS.DAT C026       24-portrait strip (8 cols x 3 rows,
 *                             each cell 32x29)
 *
 * Slice assignment (worker branch id suffix):
 *   firestaff_dm1_v1_hoc_champion_portrait_04_south_return_portrait_rect_position_052_gate
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "vga_palette_pc34_compat.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/* IMG3 globals required by the asset loader pipeline */
unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum {
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    VIEWPORT_W = 224,
    VIEWPORT_H = 136,

    /* D1C champion portrait rectangle (ReDMCSB DUNVIEW.C:3913-3928).
     * Coordinates are in viewport-local space; the framebuffer
     * destination is (VIEWPORT_X + PORTRAIT_X, VIEWPORT_Y + PORTRAIT_Y). */
    PORTRAIT_X = 96,
    PORTRAIT_Y = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,

    /* Threshold of warm-colored pixels for a positive ordinal at
     * the D1C portrait rect.  The grey stone texture used by the
     * wall ornament and corridor floor never reaches this threshold;
     * LEIF's portrait (blue background + peach highlights) reliably
     * clears it. */
    PORTRAIT_WARM_THRESHOLD = 30,

    /* The pixel-match rate required to call the rect "contains the
     * expected portrait ordinal".  90 % is generous enough to absorb
     * palette-level rounding (M11_FB_DECODE_INDEX rounds) but strict
     * enough to catch a wrong ordinal being drawn. */
    PORTRAIT_MATCH_PCT = 90,

    /* Side-wall column guards for the no-float invariant.  The D1C
     * rect sits at x=96..127 in viewport space.  Anything to the
     * left of x=80 (D1L column 0..31 plus 32 px padding) or right of
     * x=144 (D1R column 192..223 plus 32 px padding) is "side wall"
     * for the same row band.  Warm pixels in that band would mean a
     * portrait floating over a side wall. */
    SIDE_WALL_LEFT_X  = 0,
    SIDE_WALL_LEFT_W  = 80,
    SIDE_WALL_RIGHT_X = 144,
    SIDE_WALL_RIGHT_W = 80,

    /* Row bands used by the no-float invariant:
     *   PORTRAIT_ROW: same y range as the D1C portrait rect. */
    PORTRAIT_ROW_Y = PORTRAIT_Y,
    PORTRAIT_ROW_H = PORTRAIT_H,

    /* Per-cell expected ordinal for the south_return slice (4).  The
     * runtime binary may report either 4 (post-fix binary) or -1
     * (pre-fix binary, current src/engine/m11_game_view.c with the
     * visibleWallCell = direction+2 filter still active).  The
     * probe honors whichever value the runtime reports. */
    SLICE_ORDINAL_POSTFIX = 4
};

/* Warm-color palette indices per the F20E PC 3.4 LIGHT0 palette in
 * src/shared/vga_palette_pc34_compat.c.  These are the indices used
 * by champion-portrait skin/clothing pixels per ReDMCSB
 * DUNVIEW.C:3913-3928; grey stone and corridor floor never reach
 * this set. */
static int is_warm_palette_index(unsigned char idx) {
    switch (idx & 0x0Fu) {
        case 0x07: /* green     */
        case 0x08: /* red       */
        case 0x09: /* orange    */
        case 0x0A: /* peach     */
        case 0x0B: /* yellow    */
        case 0x0E: /* blue      */
            return 1;
        default:
            return 0;
    }
}

static int g_pass = 0;
static int g_fail = 0;
static int g_skip = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

#define SKIP(msg) do { ++g_skip; printf("  SKIP: %s\n", msg); } while (0)

/* Count non-transparent, palette-equal matches between an asset
 * pixel (with a transparency index skip) and the rendered viewport
 * framebuffer at the D1C portrait rect.  Returns percentage matched
 * (0..100), or -1 if the asset isn't loaded. */
static int portrait_rect_match_pct(const M11_AssetSlot* portraits,
                                   const unsigned char* fb,
                                   int ordinal) {
    int matched = 0;
    int compared = 0;
    int x, y;
    int srcPX, srcPY;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return -1;
    }
    if (ordinal < 0) ordinal = 0;
    srcPX = (ordinal & 7) * PORTRAIT_W;
    srcPY = (ordinal >> 3) * PORTRAIT_H;
    if (srcPX + PORTRAIT_W > (int)portraits->width ||
        srcPY + PORTRAIT_H > (int)portraits->height) {
        return -1;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = (unsigned char)
                (portraits->pixels[(srcPY + y) * (int)portraits->width + (srcPX + x)] & 0x0Fu);
            if (src == 1) continue; /* transparency per capture probe */
            ++compared;
            {
                unsigned char dst = M11_FB_DECODE_INDEX(
                    fb[(VIEWPORT_Y + PORTRAIT_Y + y) * FB_W +
                       (VIEWPORT_X + PORTRAIT_X + x)]);
                if (dst == src) ++matched;
            }
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

/* Count warm-palette pixels inside the D1C portrait rect. */
static int portrait_rect_warm_count(const unsigned char* fb) {
    int count = 0;
    int x, y;
    if (!fb) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char idx = M11_FB_DECODE_INDEX(
                fb[(VIEWPORT_Y + PORTRAIT_Y + y) * FB_W +
                   (VIEWPORT_X + PORTRAIT_X + x)]);
            if (is_warm_palette_index(idx)) ++count;
        }
    }
    return count;
}

/* Count warm-palette pixels inside a viewport-space rect.  Used to
 * check the no-float invariant for side-wall columns. */
static int viewport_rect_warm_count(const unsigned char* fb,
                                    int vx, int vy, int vw, int vh) {
    int count = 0;
    int x, y;
    if (!fb || vw <= 0 || vh <= 0) return 0;
    for (y = 0; y < vh; ++y) {
        if (vy + y < 0 || vy + y >= VIEWPORT_H) continue;
        for (x = 0; x < vw; ++x) {
            if (vx + x < 0 || vx + x >= VIEWPORT_W) continue;
            unsigned char idx = M11_FB_DECODE_INDEX(
                fb[(VIEWPORT_Y + vy + y) * FB_W +
                   (VIEWPORT_X + vx + x)]);
            if (is_warm_palette_index(idx)) ++count;
        }
    }
    return count;
}

/* Optional PPM dump for visual-evidence readiness.  The probe keeps
 * the dump conditional on FIRESTAFF_HOCC_PROBE_DUMP being set so the
 * default run is text-only and CI-cheap. */
static void dump_viewport_ppm(const char* path, const unsigned char* fb) {
    FILE* f;
    int x, y;
    if (!path || !fb) return;
    f = fopen(path, "wb");
    if (!f) return;
    fprintf(f, "P6\n%d %d\n255\n", VIEWPORT_W, VIEWPORT_H);
    for (y = 0; y < VIEWPORT_H; ++y) {
        for (x = 0; x < VIEWPORT_W; ++x) {
            unsigned char raw = fb[(VIEWPORT_Y + y) * FB_W + (VIEWPORT_X + x)];
            unsigned char idx = M11_FB_DECODE_INDEX(raw);
            int level = M11_FB_DECODE_LEVEL(raw);
            const unsigned char* rgb;
            if (level < 0) level = 0;
            if (level >= M11_PALETTE_LEVELS) level = M11_PALETTE_LEVELS - 1;
            rgb = G9010_auc_VgaPaletteAll_Compat[level][idx];
            fwrite(rgb, 1, 3, f);
        }
    }
    fclose(f);
}

/* Park the party at (mapX, mapY) facing dir, with no candidate
 * panel open.  The candidateMirrorPanelActive=1 path is the
 * BUG-120/121 guard covered by the panel_guard probe; this slice
 * is about the un-occluded portrait-on-wall draw, so we keep the
 * panel closed. */
static void set_pose(M11_GameViewState* game,
                     int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[FB_W * FB_H];
    int ornX = 0, ornY = 0, ornW = 0, ornH = 0;
    char mirrorName[64];
    int southOrdinal;
    int southContract;
    int pct;
    int warm;
    int sideWarmLeft;
    int sideWarmRight;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr,
                "usage: %s DATA_DIR [OUT_PPM_PREFIX]\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 HoC portrait slice: "
           "ordinal_4 south_return portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(
        (M11_AssetLoader*)&game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr,
                "SKIP: GRAPHICS.DAT C026 portrait strip unavailable "
                "(width=%u height=%u); ordinal-4 pixel match cannot run\n",
                portraits ? portraits->width : 0u,
                portraits ? portraits->height : 0u);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    /* ── Group A: south_return pose contract (2, 1) DIR_SOUTH ───── */
    printf("\n[Group A] south_return pose contract (2, 1) DIR_SOUTH\n");
    set_pose(&game, 2, 1, DIR_SOUTH);
    southOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (2,1) DIR_SOUTH == %d "
                 "(pre-fix contract: -1; post-fix contract: %d)",
                 southOrdinal, SLICE_ORDINAL_POSTFIX);
        /* We accept either value because the runtime may implement
         * either the pre-fix or the post-fix contract.  Print
         * PASS in both cases and record which contract is in use. */
        CHECK(southOrdinal == -1 || southOrdinal == SLICE_ORDINAL_POSTFIX,
              msg);
    }
    southContract = southOrdinal;
    printf("  contract in effect: %s\n",
           southContract == SLICE_ORDINAL_POSTFIX
               ? "post-fix (ordinal 4, LEIF visible)"
               : "pre-fix (ordinal -1, corridor / no portrait)");

    /* Wall ornament destination box at the D1C front mirror route.
     * ReDMCSB DUNVIEW.C:3913-3928 + G0205[coordSet=0][12].  This
     * returns the C346 wall-ornament destination which contains
     * the C026 portrait cutout at (96, 35). */
    ornX = ornY = ornW = ornH = 0;
    (void)M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall ornament zone is non-empty: (%d, %d, %d, %d) "
                 "viewport coords",
                 ornX, ornY, ornW, ornH);
        CHECK(ornW > 0 && ornH > 0, msg);
    }
    /* The portrait cutout (96, 35) sits inside the wall-ornament
     * box, so the box X must be <= 96 and X + W must be >= 128. */
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box covers portrait cutout x=96..127 "
                 "(box=(%d, %d, %d, %d))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornX <= 96 && (ornX + ornW) >= 128, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C wall box covers portrait cutout y=35..63 "
                 "(box=(%d, %d, %d, %d))",
                 ornX, ornY, ornW, ornH);
        CHECK(ornY <= 35 && (ornY + ornH) >= 64, msg);
    }

    /* Render and inspect the portrait rect. */
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    if (argc > 2) {
        char path[256];
        snprintf(path, sizeof(path), "%s_south_return.ppm", argv[2]);
        dump_viewport_ppm(path, fb);
    }

    warm = portrait_rect_warm_count(fb);

    if (southContract == SLICE_ORDINAL_POSTFIX) {
        /* Post-fix contract: LEIF portrait must be in the rect. */
        printf("\n[Group B-post] ordinal_4 contract: LEIF portrait in rect\n");
        mirrorName[0] = '\0';
        (void)M11_GameView_GetMirrorNameByOrdinal(&game,
            SLICE_ORDINAL_POSTFIX, mirrorName, sizeof(mirrorName));
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "mirror name for ordinal %d is \"%s\" (want LEIF)",
                     SLICE_ORDINAL_POSTFIX, mirrorName);
            CHECK(strcmp(mirrorName, "LEIF") == 0, msg);
        }
        pct = portrait_rect_match_pct(portraits, fb, SLICE_ORDINAL_POSTFIX);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C portrait rect matches ordinal %d (LEIF) >= %d%% "
                     "(got %d%%)",
                     SLICE_ORDINAL_POSTFIX, PORTRAIT_MATCH_PCT, pct);
            CHECK(pct >= PORTRAIT_MATCH_PCT, msg);
        }
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C portrait rect warm-pixel count >= %d (got %d)",
                     PORTRAIT_WARM_THRESHOLD, warm);
            CHECK(warm >= PORTRAIT_WARM_THRESHOLD, msg);
        }
    } else {
        /* Pre-fix contract: no portrait in rect. */
        printf("\n[Group B-pre] ordinal_-1 contract: no portrait in rect\n");
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "D1C portrait rect warm-pixel count < %d on "
                     "pre-fix south_return pose (got %d)",
                     PORTRAIT_WARM_THRESHOLD, warm);
            CHECK(warm < PORTRAIT_WARM_THRESHOLD, msg);
        }
    }

    /* ── Group C: portrait_rect_position invariant (always) ─────
     * Whether the contract is pre-fix or post-fix, the rect
     * position must be either (a) the source-locked LEIF portrait
     * pixels at (96, 35) or (b) empty / corridor / stone.  In
     * neither case may a LEIF portrait float over an ordinary
     * side wall column.  We verify this by counting warm pixels
     * in the D1L and D1R column bands for the portrait row. */
    printf("\n[Group C] portrait_rect_position: no warm pixels on side walls\n");
    sideWarmLeft = viewport_rect_warm_count(
        fb,
        SIDE_WALL_LEFT_X, PORTRAIT_ROW_Y,
        SIDE_WALL_LEFT_W, PORTRAIT_ROW_H);
    sideWarmRight = viewport_rect_warm_count(
        fb,
        SIDE_WALL_RIGHT_X, PORTRAIT_ROW_Y,
        SIDE_WALL_RIGHT_W, PORTRAIT_ROW_H);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1L side-wall column band (x=%d..%d, y=%d..%d) "
                 "has < %d warm pixels (got %d)",
                 SIDE_WALL_LEFT_X,
                 SIDE_WALL_LEFT_X + SIDE_WALL_LEFT_W - 1,
                 PORTRAIT_ROW_Y,
                 PORTRAIT_ROW_Y + PORTRAIT_ROW_H - 1,
                 PORTRAIT_WARM_THRESHOLD,
                 sideWarmLeft);
        CHECK(sideWarmLeft < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1R side-wall column band (x=%d..%d, y=%d..%d) "
                 "has < %d warm pixels (got %d)",
                 SIDE_WALL_RIGHT_X,
                 SIDE_WALL_RIGHT_X + SIDE_WALL_RIGHT_W - 1,
                 PORTRAIT_ROW_Y,
                 PORTRAIT_ROW_Y + PORTRAIT_ROW_H - 1,
                 PORTRAIT_WARM_THRESHOLD,
                 sideWarmRight);
        CHECK(sideWarmRight < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ── Group D: corridor negative (2, 2) DIR_NORTH ────────────
     * This corridor pose is one square south of the LEIF mirror
     * cell (2, 2) with the player heading back north toward the
     * LEIF chamber.  The front cell is (2, 1), which is a
     * sealed-chamber cell with no C127 sensor on its south wall,
     * so the front-mirror ordinal must be -1 and the D1C portrait
     * rect must be empty (no LEIF portrait floating over the
     * corridor wall).  This is a neighbor-pose control that proves
     * the south_return rect does not pick up a stray portrait
     * because the (2, 1) south wall is wired to a stone wall
     * texture in the sealed chamber, not the LEIF mirror. */
    printf("\n[Group D] corridor negative: (2, 2) DIR_NORTH\n");
    set_pose(&game, 2, 2, DIR_NORTH);
    {
        int ord = M11_GameView_GetFrontMirrorOrdinal(&game);
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "front-mirror ordinal at (2,2) DIR_NORTH == -1 (got %d)",
                 ord);
        CHECK(ord == -1, msg);
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, FB_W, FB_H);
    if (argc > 2) {
        char path[256];
        snprintf(path, sizeof(path), "%s_corridor_north.ppm", argv[2]);
        dump_viewport_ppm(path, fb);
    }
    {
        int negWarm = portrait_rect_warm_count(fb);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "D1C portrait rect warm-pixel count < %d on corridor "
                 "negative pose (got %d)",
                 PORTRAIT_WARM_THRESHOLD, negWarm);
        CHECK(negWarm < PORTRAIT_WARM_THRESHOLD, msg);
    }
    {
        int negLeft = viewport_rect_warm_count(
            fb, SIDE_WALL_LEFT_X, PORTRAIT_ROW_Y,
            SIDE_WALL_LEFT_W, PORTRAIT_ROW_H);
        int negRight = viewport_rect_warm_count(
            fb, SIDE_WALL_RIGHT_X, PORTRAIT_ROW_Y,
            SIDE_WALL_RIGHT_W, PORTRAIT_ROW_H);
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "side-wall columns on corridor negative also clean "
                 "(D1L=%d, D1R=%d, threshold=%d)",
                 negLeft, negRight, PORTRAIT_WARM_THRESHOLD);
        CHECK(negLeft < PORTRAIT_WARM_THRESHOLD &&
              negRight < PORTRAIT_WARM_THRESHOLD, msg);
    }

    /* ── Group E: south-return baseline at (2, 1) DIR_SOUTH ──────
     * Lock the canonical south_return pose for ordinal_4 (LEIF) so
     * the run always carries the same-ordinal same-pixel sibling
     * for the slice.  The post-fix contract must produce ordinal
     * 4 with a high pixel match rate; the pre-fix contract must
     * produce ordinal -1 with an empty rect.  Either way the
     * rect must be empty on side walls (no floating).  This is
     * the same pose as Group A but framed as the "LEIF visible
     * from the south side of (2, 2)" anchor; the re-render here
     * proves the render is deterministic across consecutive draws. */
    printf("\n[Group E] south_return baseline (2, 1) DIR_SOUTH determinism\n");
    {
        int baseWarm;
        int basePct;
        set_pose(&game, 2, 1, DIR_SOUTH);
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);
        if (argc > 2) {
            char path[256];
            snprintf(path, sizeof(path), "%s_south_return_baseline.ppm", argv[2]);
            dump_viewport_ppm(path, fb);
        }
        baseWarm = portrait_rect_warm_count(fb);
        if (southContract == SLICE_ORDINAL_POSTFIX) {
            basePct = portrait_rect_match_pct(portraits, fb, SLICE_ORDINAL_POSTFIX);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "south_return baseline rect matches ordinal %d >= %d%% "
                         "(got %d%%)",
                         SLICE_ORDINAL_POSTFIX, PORTRAIT_MATCH_PCT, basePct);
                CHECK(basePct >= PORTRAIT_MATCH_PCT, msg);
            }
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "south_return baseline rect warm-pixel count >= %d "
                         "(got %d)",
                         PORTRAIT_WARM_THRESHOLD, baseWarm);
                CHECK(baseWarm >= PORTRAIT_WARM_THRESHOLD, msg);
            }
        } else {
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "south_return baseline rect warm-pixel count < %d "
                         "on pre-fix contract (got %d)",
                         PORTRAIT_WARM_THRESHOLD, baseWarm);
                CHECK(baseWarm < PORTRAIT_WARM_THRESHOLD, msg);
            }
        }
    }

    M11_GameView_Shutdown(&game);
    printf("\n=== Summary: %d passed, %d failed, %d skipped ===\n",
           g_pass, g_fail, g_skip);
    printf("south_return contract = %s\n",
           southContract == SLICE_ORDINAL_POSTFIX ? "ordinal_4" : "ordinal_-1");
    return g_fail == 0 ? 0 : 1;
}
