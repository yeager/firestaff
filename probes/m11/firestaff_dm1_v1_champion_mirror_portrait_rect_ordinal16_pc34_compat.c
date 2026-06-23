/*
 * DM1 V1 champion mirror portrait_rect_position ordinal 16
 *   + west_negative runtime regression probe.
 *
 * The existing
 *   firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe,
 *   firestaff_dm1_v1_champion_mirror_capture_probe,
 *   firestaff_dm1_v1_champion_mirror_visibility_runtime_probe,
 *   firestaff_dm1_v1_champion_mirror_walkpath_runtime_probe,
 *   firestaff_dm1_v1_champion_mirror_zorder_runtime_probe, and
 *   firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 * collectively cover ordinals {1, 2, 4, 10, 13, 15, 18, 19} on the
 * reference DM1 V1 DUNGEON.DAT. Ordinal 16 lives on a Hall of
 * Champions cell that none of the existing probes visit. This probe
 * covers that narrow ordinal-16 slice plus the matching west_negative
 * no-portrait route in the same map region.
 *
 * Reference ordinal-16 sensor layout (real DM1 V1 DUNGEON.DAT):
 *   (map 0, x=2, y=7, dir=SOUTH)  -> front=(2,8) carries a C127
 *                                    sensor with sensorData=16,
 *                                    mapped to M027/M028 atlas math
 *                                    (col = 16 & 7 = 0, row = 16>>3 = 2)
 *                                    so the C026 portrait strip
 *                                    shows the (col=0, row=2) portrait.
 *   (map 0, x=1, y=7, dir=WEST)   -> front=(0,7) has no C127 sensor
 *                                    so the front-mirror ordinal is
 *                                    -1 and the D1C portrait rectangle
 *                                    must NOT show a portrait sprite
 *                                    floating over the side wall
 *                                    (west_negative no-floating
 *                                    invariant).
 *   (map 0, x=2, y=7, dir=WEST)   -> front=(1,7) also has no C127
 *                                    sensor (the C127 sensor at
 *                                    (1,7) is owned by the
 *                                    NORTH-facing aspect with
 *                                    sensorData=13 and is rejected by
 *                                    the front-cell side filter per
 *                                    DUNGEON.C:2573 + DEFS.H:2552);
 *                                    this acts as a redundant
 *                                    west_negative guard against the
 *                                    Z-order / no-floating regression
 *                                    called out by the 2026-06-22
 *                                    pass1053 champion-mirror fix.
 *
 * Source evidence (ReDMCSB WIP20210206):
 *   DUNGEON.C:2573 - map M011_CELL(sensor) against view direction
 *   DUNGEON.C:2608-2612 - store C127 sensorData in G0289 (the
 *                          0..23 C026 portrait-atlas ordinal)
 *   DUNVIEW.C:3913-3928 - blit C026 champion portrait at the D1C
 *                          G0109 box { x=96, y=35, w=32, h=29 } using
 *                          (ordinal & 7) << 5 and (ordinal >> 3) * 29
 *                          with C01_COLOR_DARK_GRAY transparency
 *   DUNVIEW.C:525 - G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *                    = { 96, 127, 35, 63 } (x_min..x_max, y_min..y_max)
 *   DUNVIEW.C:8318-8542 F0128_DUNGEONVIEW_Draw_CPSF - viewport
 *                          redraw from party map/x/y/direction
 *                          (far-to-near, so the C127 D1C portrait
 *                          blit lands on top of the wall ornament,
 *                          not behind it)
 *   MOVESENS.C:1501-1503 - C127 sensorData flows into F0280
 *                          REVIVE.C F0280 - materializes the
 *                          candidate champion from sensorData
 *   COORD.C:1693-1722 - PC34 viewport origin / 224x136 dimensions
 *                       (so M11_VIEWPORT_X = 0, M11_VIEWPORT_Y = 33,
 *                       and the framebuffer D1C portrait rect is at
 *                       (96, 68)-(127, 96) - PROBE_PORTRAIT_* uses
 *                       the (PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y)
 *                       = (96, 68) origin to read the same bytes
 *                       that the C026 blit wrote).
 *
 * This probe does not claim DOS pixel parity. It verifies three
 * narrower contracts the source-locked M11 runtime must hold for the
 * ordinal-16 cell:
 *
 *   R1. (2,7) facing SOUTH reports front-mirror ordinal = 16, the
 *       D1C portrait rectangle bytes match ordinal-16 pixels (>= 70%
 *       opacity match against the C026 (col=0, row=2) strip cutout),
 *       and the wall-ornament frame rect (80, 29, 64, 43) is non-zero
 *       and equal to the source-locked M11_GameView_GetD1CWallOrnamentZone
 *       output.
 *   R2. (1,7) facing WEST reports front-mirror ordinal = -1 and the
 *       D1C portrait rectangle warm-color pixel count is below the
 *       no-portrait threshold (no floating portrait over the side
 *       wall). The negative match-percent against any C026 ordinal
 *       must also stay under 35% (the same threshold the existing
 *       zorder probe locks for corridor / side-wall poses).
 *   R3. (2,7) facing WEST reports front-mirror ordinal = -1 and the
 *       D1C portrait rectangle is empty of portrait pixels even
 *       though (1,7) carries a C127 sensor on its NORTH aspect
 *       (sensorData=13) - the front-cell side filter must reject
 *       the wrong-aspect sensor so the side-wall west view never
 *       blits a portrait at the D1C rect.
 *
 * Usage: firestaff_dm1_v1_champion_mirror_portrait_rect_ordinal16_pc34_compat DATA_DIR
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

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
    /* ReDMCSB DUNVIEW.C:525 G0109_auc_Graphic558_Box_ChampionPortraitOnWall
     * { 96, 127, 35, 63 } in viewport coords -> (96, 35)-(127, 63)
     * for the 32x29 C026 portrait cutout. */
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    /* ReDMCSB DUNVIEW.C G0205 coordSet 5 / index 12 (D1C champion-mirror
     * frame) -> kD1CDest = { 80, 29, 64, 43 }.  M11_GameView_GetD1CWallOrnamentZone
     * returns these four ints in viewport coords. */
    PROBE_D1C_FRAME_X = 80,
    PROBE_D1C_FRAME_Y = 29,
    PROBE_D1C_FRAME_W = 64,
    PROBE_D1C_FRAME_H = 43,
    /* Tunables - keep aligned with the existing zorder / visibility
     * probes so all Hall-of-Champions slices pass under the same
     * pixel thresholds. */
    PROBE_POSITIVE_MATCH_PCT = 70,
    PROBE_NEGATIVE_MATCH_PCT = 35,
    PROBE_NEGATIVE_WARM_THRESHOLD = 30
};

typedef struct MirrorMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} MirrorMatch;

static int g_pass = 0;
static int g_fail = 0;

#define PASS() do { ++g_pass; } while (0)
#define FAIL(msg) do { fprintf(stderr, "FAIL: %s\n", msg); ++g_fail; } while (0)
#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; } \
    else      { fprintf(stderr, "FAIL: %s\n", msg); ++g_fail; } \
} while (0)

static int expect_int(const char* label, int got, int want) {
    char buf[160];
    snprintf(buf, sizeof(buf), "%s got=%d want=%d", label, got, want);
    CHECK(got == want, buf);
    return got == want;
}

static MirrorMatch match_d1c_portrait(const M11_AssetSlot* portraits,
                                     const unsigned char* fb,
                                     int expectedOrdinal) {
    MirrorMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int x;
        int y;
        int matched = 0;
        int compared = 0;
        for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
            for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
                unsigned char src;
                unsigned char dst;
                if (srcX >= (int)portraits->width ||
                    srcY >= (int)portraits->height) {
                    continue;
                }
                src = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                /* ReDMCSB DUNVIEW.C:3916 C01_COLOR_DARK_GRAY transparency
                 * mask - source palette index 1 is the dark-grey
                 * transparency key and must be skipped from the match. */
                if (src == 1) {
                    continue;
                }
                dst = M11_FB_DECODE_INDEX(
                    fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W + (PROBE_PORTRAIT_X + x)]);
                ++compared;
                if (dst == src) {
                    ++matched;
                }
            }
        }
        if (matched > out.bestMatched) {
            out.bestMatched = matched;
            out.bestOrdinal = ordinal;
        }
        if (ordinal == expectedOrdinal) {
            out.expectedMatched = matched;
            out.compared = compared;
        }
    }
    return out;
}

/* Count warm-color palette indices inside the D1C portrait rect.
 * Mirrors firestaff_dm1_v1_champion_mirror_capture_probe's heuristic:
 * C026 champion portrait sprites use {0x07, 0x08, 0x09, 0x0A, 0x0B,
 * 0x0E} (green/red/orange/peach/yellow/blue), so a positive-ordinal
 * pose must show >= 30 such pixels in the rect while a negative pose
 * (wall-only) must show < 30. */
static int portrait_rect_warm_count(const unsigned char* fb) {
    int x;
    int y;
    int count = 0;
    if (!fb) return 0;
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            unsigned char idx = M11_FB_DECODE_INDEX(
                fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W + (PROBE_PORTRAIT_X + x)]);
            switch (idx) {
                case 0x07: case 0x08: case 0x09:
                case 0x0A: case 0x0B: case 0x0E:
                    ++count;
                    break;
                default:
                    break;
            }
        }
    }
    return count;
}

/* Count non-zero pixels in the D1C portrait rect.  Used to
 * distinguish "rect is empty (wall-only)" from "rect is filled
 * (portrait+frame)" before the warm-color split. */
static int portrait_rect_nonzero(const unsigned char* fb) {
    int x;
    int y;
    if (!fb) return 0;
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            if (fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W + (PROBE_PORTRAIT_X + x)] != 0) {
                return 1;
            }
        }
    }
    return 0;
}

/* Check the D1C wall-ornament frame rect matches the source-locked
 * coordSet 5 / index 12.  The frame surrounds the portrait cutout
 * and is drawn before the portrait blit, so a positive pose shows
 * both frame pixels AND portrait pixels in their respective rects. */
static void check_d1c_frame(M11_GameViewState* game,
                            const char* label) {
    int x = 0;
    int y = 0;
    int w = 0;
    int h = 0;
    if (!M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h)) {
        fprintf(stderr, "FAIL: %s D1C wall-ornament zone unavailable\n", label);
        ++g_fail;
        return;
    }
    {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "%s D1C frame x = %d (want %d)", label, x, PROBE_D1C_FRAME_X);
        CHECK(x == PROBE_D1C_FRAME_X, buf);
    }
    {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "%s D1C frame y = %d (want %d)", label, y, PROBE_D1C_FRAME_Y);
        CHECK(y == PROBE_D1C_FRAME_Y, buf);
    }
    {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "%s D1C frame w = %d (want %d)", label, w, PROBE_D1C_FRAME_W);
        CHECK(w == PROBE_D1C_FRAME_W, buf);
    }
    {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "%s D1C frame h = %d (want %d)", label, h, PROBE_D1C_FRAME_H);
        CHECK(h == PROBE_D1C_FRAME_H, buf);
    }
}

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

/* R1 - ordinal-16 portrait route at (2,7) facing SOUTH. */
static int check_ordinal16_front_route(M11_GameViewState* game,
                                       const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ord;
    int warm;
    int nonzero;
    MirrorMatch match;
    int ok = 1;

    set_pose(game, 2, 7, 2 /* DIR_SOUTH */);

    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (!expect_int("(2,7) SOUTH front mirror ordinal = 16", ord, 16)) {
        ok = 0;
    }

    check_d1c_frame(game, "(2,7) SOUTH");

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    nonzero = portrait_rect_nonzero(fb);
    if (!expect_int("(2,7) SOUTH D1C portrait rect is non-zero", nonzero, 1)) {
        ok = 0;
    }
    warm = portrait_rect_warm_count(fb);
    {
        char buf[160];
        snprintf(buf, sizeof(buf),
                 "(2,7) SOUTH D1C portrait rect warm_count=%d >= %d",
                 warm, PROBE_NEGATIVE_WARM_THRESHOLD);
        CHECK(warm >= PROBE_NEGATIVE_WARM_THRESHOLD, buf);
    }

    match = match_d1c_portrait(portraits, fb, 16);
    {
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(2,7) SOUTH D1C portrait rect bestOrdinal=%d expected=16",
                 match.bestOrdinal);
        CHECK(match.bestOrdinal == 16, buf);
    }
    {
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(2,7) SOUTH D1C ordinal-16 match=%d/%d (%d%%) >= %d%%",
                 match.expectedMatched, match.compared,
                 (match.compared > 0)
                     ? (int)((match.expectedMatched * 100) / match.compared)
                     : 0,
                 PROBE_POSITIVE_MATCH_PCT);
        CHECK(match.compared > 0 &&
              (match.expectedMatched * 100) >= (match.compared * PROBE_POSITIVE_MATCH_PCT),
              buf);
    }

    printf("(2,7) SOUTH ord=%d best=%d matched=%d/%d warm=%d nonzero=%d\n",
           ord, match.bestOrdinal, match.expectedMatched,
           match.compared, warm, nonzero);
    return ok;
}

/* R2 - west_negative at (1,7) facing WEST.  No front mirror; the
 * D1C portrait rectangle must NOT be filled with portrait pixels. */
static int check_west_negative_route_1_7_west(M11_GameViewState* game,
                                              const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ord;
    int warm;
    int nonzero;
    MirrorMatch match;
    int ok = 1;

    set_pose(game, 1, 7, 3 /* DIR_WEST */);

    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (!expect_int("(1,7) WEST front mirror ordinal = -1", ord, -1)) {
        ok = 0;
    }

    check_d1c_frame(game, "(1,7) WEST");

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    nonzero = portrait_rect_nonzero(fb);
    warm = portrait_rect_warm_count(fb);
    match = match_d1c_portrait(portraits, fb, 0);
    {
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(1,7) WEST bestOrdinal=%d must NOT dominate "
                 "(matched=%d/%d %d%% <= %d%%)",
                 match.bestOrdinal,
                 match.bestMatched, match.compared,
                 (match.compared > 0)
                     ? (int)((match.bestMatched * 100) / match.compared)
                     : 0,
                 PROBE_NEGATIVE_MATCH_PCT);
        CHECK((match.compared == 0) ||
              (match.bestMatched * 100) <= (match.compared * PROBE_NEGATIVE_MATCH_PCT),
              buf);
    }
    {
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(1,7) WEST warm_count=%d < %d (no floating portrait)",
                 warm, PROBE_NEGATIVE_WARM_THRESHOLD);
        CHECK(warm < PROBE_NEGATIVE_WARM_THRESHOLD, buf);
    }
    /* The frame is still drawn on the wall (it's a D1C box), but
     * the rect itself is dominated by grey stone - no portrait
     * pixels may dominate.  This is a no-floating regression guard. */
    (void)nonzero;

    printf("(1,7) WEST ord=%d best=%d matched=%d/%d warm=%d\n",
           ord, match.bestOrdinal, match.bestMatched,
           match.compared, warm);
    return ok;
}

/* R3 - west_negative at (2,7) facing WEST.  (1,7) carries a C127
 * sensor on its NORTH aspect (sensorData=13) but the source-locked
 * front-cell side filter must reject it on a WEST-facing view
 * (DUNGEON.C:2573 + DEFS.H:2552) so the D1C portrait rect stays
 * empty of portrait pixels - no floating over the side wall. */
static int check_west_negative_route_2_7_west(M11_GameViewState* game,
                                              const M11_AssetSlot* portraits) {
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    int ord;
    int warm;
    MirrorMatch match;
    int ok = 1;

    set_pose(game, 2, 7, 3 /* DIR_WEST */);

    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    if (!expect_int("(2,7) WEST front mirror ordinal = -1", ord, -1)) {
        ok = 0;
    }

    check_d1c_frame(game, "(2,7) WEST");

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    warm = portrait_rect_warm_count(fb);
    match = match_d1c_portrait(portraits, fb, 0);
    {
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(2,7) WEST bestOrdinal=%d must NOT dominate "
                 "(matched=%d/%d %d%% <= %d%%)",
                 match.bestOrdinal,
                 match.bestMatched, match.compared,
                 (match.compared > 0)
                     ? (int)((match.bestMatched * 100) / match.compared)
                     : 0,
                 PROBE_NEGATIVE_MATCH_PCT);
        CHECK((match.compared == 0) ||
              (match.bestMatched * 100) <= (match.compared * PROBE_NEGATIVE_MATCH_PCT),
              buf);
    }
    {
        char buf[200];
        snprintf(buf, sizeof(buf),
                 "(2,7) WEST warm_count=%d < %d (no floating portrait)",
                 warm, PROBE_NEGATIVE_WARM_THRESHOLD);
        CHECK(warm < PROBE_NEGATIVE_WARM_THRESHOLD, buf);
    }

    printf("(2,7) WEST ord=%d best=%d matched=%d/%d warm=%d\n",
           ord, match.bestOrdinal, match.bestMatched,
           match.compared, warm);
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
        fprintf(stderr, "FAIL: could not open DM1 V1 game view from %s\n",
                dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL: GRAPHICS.DAT C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 champion mirror portrait_rect_position ordinal 16 + west_negative ===\n");
    printf("dataDir=%s\n", dataDir);
    printf("Source: DUNVIEW.C:3913-3928 + DUNVIEW.C:525 (G0109 {96,127,35,63}) +\n");
    printf("        DUNGEON.C:2573 / 2608-2612 / DEFS.H:2552 / MOVESENS.C:1501-1503.\n");

    if (!check_ordinal16_front_route(&game, portraits)) ok = 0;
    if (!check_west_negative_route_1_7_west(&game, portraits)) ok = 0;
    if (!check_west_negative_route_2_7_west(&game, portraits)) ok = 0;

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed (ok=%d) ===\n", g_pass, g_fail, ok ? 1 : 0);
    return (g_fail == 0) ? 0 : 1;
}
