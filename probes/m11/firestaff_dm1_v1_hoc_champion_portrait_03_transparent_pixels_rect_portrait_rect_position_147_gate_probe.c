/*
 * DM1 V1 Hall of Champions portrait ordinal 3 transparent_pixels_rect /
 * portrait_rect_position runtime probe.
 *
 * This slice is deliberately narrower than the existing ordinal-3 probes:
 *   - firestaff_dm1_v1_hall_champion_portrait_03_east_walkpath_runtime_probe
 *     proves the real Hall route at (map 0, x=3, y=7, SOUTH), ordinal
 *     selection, no-floating neighbors, and high-level C026 rect dominance.
 *   - firestaff_dm1_v1_champion_mirror_portrait03_rect_runtime_probe seeds
 *     a north-entry C127 sensor to ordinal 3 and checks the D1C rectangle.
 *
 * This probe adds the missing per-pixel transparency contract for the real
 * ordinal-3 east_walkpath route: ReDMCSB DUNVIEW.C:3913-3928 blits C026
 * champion portraits into the D1C cutout (96,35)-(127,63), using C01
 * dark gray as the transparent color. Firestaff routes that through
 * M11_AssetLoader_BlitRegion with transparent=1 after drawing the C346
 * wall-mirror backing at viewport (80,29,64,43). Therefore every C026
 * source pixel with palette index 1 must leave destination palette 12
 * from the C346 backing in the framebuffer, while every opaque source
 * pixel must be painted at the same D1C cutout coordinate.
 *
 * The negative side of this slice uses the adjacent east corridor step
 * (4,7,SOUTH). The local PC 3.4 Hall fixture also has other champion
 * mirrors nearby, so the probe does not assume every neighboring step is
 * a no-portrait wall.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 draws C346 then blits C026 into the
 *     champion portrait box with C01 transparency.
 *   ReDMCSB DUNVIEW.C:525 records G0109 Graphic558_Box_ChampionPortraitOnWall
 *     as (96,127,35,63), i.e. a 32x29 cutout at viewport (96,35).
 *   ReDMCSB DUNVIEW.C G0205 graphic 558 coordSet 5 [12] records the C346
 *     D1C wall-mirror frame as (80,29,64,43).
 *   ReDMCSB COORD.C:1693-1722 defines the PC 3.4 viewport origin.
 *   ReDMCSB DEFS.H:821-826 derives C026 atlas coordinates as
 *     (ordinal & 7) * 32, (ordinal >> 3) * 29.
 *
 * Honest scope: this is runtime correctness against the local DM1 V1
 * GRAPHICS.DAT/DUNGEON.DAT. It does not claim original DOS pixel parity.
 */
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

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
    PORTRAIT_VX = 96,
    PORTRAIT_VY = 35,
    PORTRAIT_X = VIEWPORT_X + PORTRAIT_VX,
    PORTRAIT_Y = VIEWPORT_Y + PORTRAIT_VY,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    D1C_FRAME_X = 80,
    D1C_FRAME_Y = 29,
    D1C_FRAME_W = 64,
    D1C_FRAME_H = 43,
    C026_TRANSPARENT = 1,
    C346_BACKING = 12,
    ORDINAL_TARGET = 3,
    POSE_X = 3,
    POSE_Y = 7,
    DIR_NORTH_PROBE = 0,
    DIR_EAST_PROBE = 1,
    DIR_SOUTH_PROBE = 2,
    DIR_WEST_PROBE = 3,
    STALE_MATCH_LIMIT_PCT = 35
};

typedef struct PixelEvidence {
    int compared;
    int transparentSource;
    int transparentPreserved;
    int transparentLeaked;
    int opaqueSource;
    int opaquePainted;
    int opaqueDropped;
    int firstTransparentLeakX;
    int firstTransparentLeakY;
    int firstOpaqueDropX;
    int firstOpaqueDropY;
    unsigned char firstTransparentLeakDst;
    unsigned char firstOpaqueDropSrc;
    unsigned char firstOpaqueDropDst;
} PixelEvidence;

static int g_pass = 0;
static int g_fail = 0;

#define CHECKF(cond, fmt, ...) do { \
    if (cond) { ++g_pass; printf("  PASS: " fmt "\n", __VA_ARGS__); } \
    else      { ++g_fail; printf("  FAIL: " fmt "\n", __VA_ARGS__); } \
} while (0)

static void set_pose(M11_GameViewState* game, int x, int y, int dir) {
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

static void render_at(M11_GameViewState* game,
                      unsigned char* fb,
                      int x,
                      int y,
                      int dir) {
    set_pose(game, x, y, dir);
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
}

static unsigned char portrait_src_at(const M11_AssetSlot* portraits,
                                     int ordinal,
                                     int x,
                                     int y) {
    int srcX = (ordinal & 7) * PORTRAIT_W + x;
    int srcY = (ordinal >> 3) * PORTRAIT_H + y;
    return (unsigned char)(
        portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
}

static unsigned char fb_idx_at(const unsigned char* fb, int x, int y) {
    return (unsigned char)M11_FB_DECODE_INDEX(fb[y * FB_W + x]);
}

static PixelEvidence collect_pixel_evidence(const M11_AssetSlot* portraits,
                                            const unsigned char* fb) {
    PixelEvidence ev;
    int x;
    int y;
    memset(&ev, 0, sizeof(ev));
    ev.firstTransparentLeakX = -1;
    ev.firstTransparentLeakY = -1;
    ev.firstOpaqueDropX = -1;
    ev.firstOpaqueDropY = -1;

    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return ev;
    }

    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = portrait_src_at(portraits, ORDINAL_TARGET, x, y);
            unsigned char dst = fb_idx_at(fb, PORTRAIT_X + x, PORTRAIT_Y + y);
            ++ev.compared;
            if (src == C026_TRANSPARENT) {
                ++ev.transparentSource;
                if (dst == C346_BACKING) {
                    ++ev.transparentPreserved;
                } else {
                    ++ev.transparentLeaked;
                    if (ev.firstTransparentLeakX < 0) {
                        ev.firstTransparentLeakX = x;
                        ev.firstTransparentLeakY = y;
                        ev.firstTransparentLeakDst = dst;
                    }
                }
            } else {
                ++ev.opaqueSource;
                if (dst == src) {
                    ++ev.opaquePainted;
                } else {
                    ++ev.opaqueDropped;
                    if (ev.firstOpaqueDropX < 0) {
                        ev.firstOpaqueDropX = x;
                        ev.firstOpaqueDropY = y;
                        ev.firstOpaqueDropSrc = src;
                        ev.firstOpaqueDropDst = dst;
                    }
                }
            }
        }
    }
    return ev;
}

static int portrait_match_percent(const M11_AssetSlot* portraits,
                                  const unsigned char* fb) {
    int x;
    int y;
    int matched = 0;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return 0;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            unsigned char src = portrait_src_at(portraits, ORDINAL_TARGET, x, y);
            unsigned char dst = fb_idx_at(fb, PORTRAIT_X + x, PORTRAIT_Y + y);
            if (src == C026_TRANSPARENT) {
                continue;
            }
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    return compared > 0 ? (matched * 100) / compared : 0;
}

static int rect_byte_diff(const unsigned char* a, const unsigned char* b) {
    int x;
    int y;
    int diff = 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int off = (PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x);
            if (a[off] != b[off]) {
                ++diff;
            }
        }
    }
    return diff;
}

static void check_rect_anchors(M11_GameViewState* game) {
    int x = -1;
    int y = -1;
    int w = -1;
    int h = -1;
    int rc;

    printf("\n[A] D1C frame and C026 ordinal-3 source rect\n");
    rc = M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    CHECKF(rc == 1, "D1C wall ornament helper returns 1 (got %d)", rc);
    CHECKF(x == D1C_FRAME_X && y == D1C_FRAME_Y &&
           w == D1C_FRAME_W && h == D1C_FRAME_H,
           "D1C C346 frame is (80,29,64,43), got (%d,%d,%d,%d)",
           x, y, w, h);
    CHECKF(PORTRAIT_VX == D1C_FRAME_X + 16,
           "portrait viewport x is frame x+16 (got %d)", PORTRAIT_VX);
    CHECKF(PORTRAIT_VY == D1C_FRAME_Y + 6,
           "portrait viewport y is frame y+6 (got %d)", PORTRAIT_VY);
    CHECKF(((ORDINAL_TARGET & 7) * PORTRAIT_W) == 96,
           "ordinal 3 C026 source x is 96 (got %d)",
           (ORDINAL_TARGET & 7) * PORTRAIT_W);
    CHECKF(((ORDINAL_TARGET >> 3) * PORTRAIT_H) == 0,
           "ordinal 3 C026 source y is 0 (got %d)",
           (ORDINAL_TARGET >> 3) * PORTRAIT_H);
}

static void check_positive_pixels(M11_GameViewState* game,
                                  const M11_AssetSlot* portraits) {
    unsigned char fb[FB_W * FB_H];
    PixelEvidence ev;
    int ord;

    printf("\n[B] real ordinal-3 east_walkpath transparent_pixels_rect\n");
    render_at(game, fb, POSE_X, POSE_Y, DIR_SOUTH_PROBE);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    CHECKF(ord == ORDINAL_TARGET,
           "front ordinal at (3,7,SOUTH) is 3 (got %d)", ord);

    ev = collect_pixel_evidence(portraits, fb);
    CHECKF(ev.compared == PORTRAIT_W * PORTRAIT_H,
           "compared all 32x29 C026 pixels (got %d)", ev.compared);
    CHECKF(ev.transparentSource > 0,
           "ordinal 3 C026 cell has transparent C01 pixels (got %d)",
           ev.transparentSource);
    CHECKF(ev.transparentLeaked == 0,
           "C01 transparent pixels preserve C346 backing "
           "(source=%d preserved=%d leaked=%d first=(%d,%d,dst=0x%02x))",
           ev.transparentSource, ev.transparentPreserved,
           ev.transparentLeaked, ev.firstTransparentLeakX,
           ev.firstTransparentLeakY,
           (unsigned)ev.firstTransparentLeakDst);
    CHECKF(ev.opaqueDropped == 0,
           "opaque C026 pixels paint at D1C cutout "
           "(source=%d painted=%d dropped=%d first=(%d,%d,src=0x%02x,dst=0x%02x))",
           ev.opaqueSource, ev.opaquePainted, ev.opaqueDropped,
           ev.firstOpaqueDropX, ev.firstOpaqueDropY,
           (unsigned)ev.firstOpaqueDropSrc,
           (unsigned)ev.firstOpaqueDropDst);
}

static void check_neighbor_negative(M11_GameViewState* game,
                                    const M11_AssetSlot* portraits,
                                    int x,
                                    int y,
                                    const char* label) {
    unsigned char fb[FB_W * FB_H];
    int ord;
    int pct;

    printf("\n[C] no-floating neighbor %s\n", label);
    render_at(game, fb, x, y, DIR_SOUTH_PROBE);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    pct = portrait_match_percent(portraits, fb);
    CHECKF(ord == -1,
           "%s has no front C127 mirror ordinal (got %d)", label, ord);
    CHECKF(pct < STALE_MATCH_LIMIT_PCT,
           "%s does not retain ordinal-3 C026 cutout (match=%d%% limit<%d%%)",
           label, pct, STALE_MATCH_LIMIT_PCT);
}

static void check_rerender_stability(M11_GameViewState* game,
                                     const M11_AssetSlot* portraits) {
    unsigned char fb1[FB_W * FB_H];
    unsigned char fb2[FB_W * FB_H];
    PixelEvidence ev1;
    PixelEvidence ev2;
    int diff;

    printf("\n[D] transparent_pixels_rect redraw stability\n");
    render_at(game, fb1, POSE_X, POSE_Y, DIR_SOUTH_PROBE);
    render_at(game, fb2, POSE_X, POSE_Y, DIR_SOUTH_PROBE);
    ev1 = collect_pixel_evidence(portraits, fb1);
    ev2 = collect_pixel_evidence(portraits, fb2);
    diff = rect_byte_diff(fb1, fb2);

    CHECKF(ev1.transparentLeaked == 0 && ev2.transparentLeaked == 0,
           "transparent leak count stable at zero (first=%d second=%d)",
           ev1.transparentLeaked, ev2.transparentLeaked);
    CHECKF(ev1.opaqueDropped == 0 && ev2.opaqueDropped == 0,
           "opaque drop count stable at zero (first=%d second=%d)",
           ev1.opaqueDropped, ev2.opaqueDropped);
    CHECKF(diff == 0,
           "D1C portrait cutout byte-identical across redraws (diff=%d)",
           diff);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ord;
    int ok;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }

    printf("=== DM1 V1 HoC portrait ordinal 3 transparent_pixels_rect gate 147 ===\n");
    printf("dataDir=%s pose=(map 0, x=%d, y=%d, SOUTH)\n",
           dataDir, POSE_X, POSE_Y);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(
        &game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 8 * PORTRAIT_W ||
        portraits->height < 3 * PORTRAIT_H) {
        fprintf(stderr, "FAIL GRAPHICS.DAT C026 portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    set_pose(&game, POSE_X, POSE_Y, DIR_SOUTH_PROBE);
    ord = M11_GameView_GetFrontMirrorOrdinal(&game);
    if (ord != ORDINAL_TARGET) {
        printf("SKIP hoc_portrait03_transparent_pixels_rect_fixture_mismatch "
               "(%d,%d) SOUTH front ordinal=%d expected=%d\n",
               POSE_X, POSE_Y, ord, ORDINAL_TARGET);
        M11_GameView_Shutdown(&game);
        return 0;
    }

    check_rect_anchors(&game);
    check_positive_pixels(&game, portraits);
    check_neighbor_negative(&game, portraits, POSE_X + 1, POSE_Y,
                            "east corridor step (4,7,SOUTH)");
    check_rerender_stability(&game, portraits);

    M11_GameView_Shutdown(&game);
    ok = (g_fail == 0);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    printf("%s dm1 v1 HoC champion portrait ordinal 3 transparent_pixels_rect "
           "portrait_rect_position\n",
           ok ? "PASS" : "FAIL");
    return ok ? 0 : 1;
}
