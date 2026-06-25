/*
 * DM1 V1 Hall of Champions -- ordinal 4 wake_repaint /
 * portrait_rect_position runtime probe.
 *
 * Source lock:
 *   ReDMCSB DUNGEON.C:2573,2608-2612 maps a front-wall C127 sensor's
 *   sensorData into G0289.  DUNVIEW.C:3913-3928 blits the C026 champion
 *   portrait strip into the D1C mirror cutout at viewport (96,35,32,29).
 *   COMMAND.C:2336-2363 owns the rest/wake redraw path
 *   (G0300_B_PartyIsResting).  m11_game_view.c draws the RESTING overlay
 *   at framebuffer (100,70,120,30) and must remove it on wake.
 *
 * Scope: Firestaff runtime evidence using the real DM1 V1 asset pair.  The
 * probe retargets the shipped (1,2,NORTH) HALK C127 sensorData=1 to ordinal
 * 4 (LEIF) so the wake repaint path is locked for the row-0 / col-4 C026
 * atlas cell.  This does not claim DOS pixel parity.
 */
#include "asset_loader_m11.h"
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
    VIEWPORT_Y = 33,
    PORTRAIT_X = 96,
    PORTRAIT_Y = 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    D1C_ZONE_X = 80,
    D1C_ZONE_Y = 29,
    D1C_ZONE_W = 64,
    D1C_ZONE_H = 43,
    REST_X = 100,
    REST_Y = 70,
    REST_W = 120,
    REST_H = 30,
    ORDINAL_TARGET = 4,
    ORDINAL_SOURCE = 1,
    PORTRAIT_TOTAL = 24,
    WARM_MIN = 30,
    MATCH_MIN_PCT = 70,
    REST_BLACK_MIN = 30,
    WAKE_BLACK_TOLERANCE = 5,
    FLOAT_MAX_PCT = 35,
    CYCLE_COUNT = 4
};

static int g_pass = 0;
static int g_fail = 0;

#define PASS() do { ++g_pass; } while (0)
#define FAIL(fmt, ...) do { fprintf(stderr, "FAIL: " fmt "\n", ##__VA_ARGS__); ++g_fail; } while (0)

typedef struct RectEvidence {
    int warm;
    int compared;
    int matched;
    int pct;
} RectEvidence;

static int fb_x(int vpX) {
    return vpX;
}

static int fb_y(int vpY) {
    return VIEWPORT_Y + vpY;
}

static int pixel_is_warm(unsigned char idx) {
    return idx == 0x07 || idx == 0x08 || idx == 0x09 ||
           idx == 0x0A || idx == 0x0B || idx == 0x0E;
}

static void reset_pose(M11_GameViewState* game, int x, int y, int dir) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = x;
    game->world.party.mapY = y;
    game->world.party.direction = dir;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
    game->inventoryPanelActive = 0;
    game->resting = 0;
    game->partyDead = 0;
}

static int retarget_front_c127(M11_GameViewState* game) {
    int i;
    if (!game || !game->world.things || !game->world.things->sensors) {
        return 0;
    }
    for (i = 0; i < game->world.things->sensorCount; ++i) {
        if (game->world.things->sensors[i].sensorType != 127) continue;
        if ((int)game->world.things->sensors[i].sensorData != ORDINAL_SOURCE) continue;
        game->world.things->sensors[i].sensorData = (unsigned short)ORDINAL_TARGET;
        return 1;
    }
    return 0;
}

static void collect_rect(const M11_AssetSlot* portraits,
                         const unsigned char* fb,
                         RectEvidence* out) {
    int x, y;
    int dstX0 = fb_x(PORTRAIT_X);
    int dstY0 = fb_y(PORTRAIT_Y);
    int srcX0 = (ORDINAL_TARGET & 7) * PORTRAIT_W;
    int srcY0 = (ORDINAL_TARGET >> 3) * PORTRAIT_H;
    memset(out, 0, sizeof(*out));

    for (y = 0; y < PORTRAIT_H; ++y) {
        int dstY = dstY0 + y;
        if (dstY < 0 || dstY >= FB_H) continue;
        for (x = 0; x < PORTRAIT_W; ++x) {
            int dstX = dstX0 + x;
            unsigned char dstIdx;
            if (dstX < 0 || dstX >= FB_W) continue;
            dstIdx = M11_FB_DECODE_INDEX(fb[dstY * FB_W + dstX]);
            if (pixel_is_warm(dstIdx)) ++out->warm;
        }
    }

    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ORDINAL_TARGET < 0 || ORDINAL_TARGET >= PORTRAIT_TOTAL) {
        return;
    }
    for (y = 0; y < PORTRAIT_H; ++y) {
        int srcY = srcY0 + y;
        int dstY = dstY0 + y;
        if (srcY < 0 || srcY >= (int)portraits->height ||
            dstY < 0 || dstY >= FB_H) continue;
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = srcX0 + x;
            int dstX = dstX0 + x;
            unsigned char srcIdx;
            unsigned char dstIdx;
            if (srcX < 0 || srcX >= (int)portraits->width ||
                dstX < 0 || dstX >= FB_W) continue;
            srcIdx = (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (srcIdx == 1) continue;
            dstIdx = M11_FB_DECODE_INDEX(fb[dstY * FB_W + dstX]);
            ++out->compared;
            if (dstIdx == srcIdx) ++out->matched;
        }
    }
    if (out->compared > 0) out->pct = (out->matched * 100) / out->compared;
}

static int count_rest_black(const unsigned char* fb) {
    int x, y, count = 0;
    for (y = REST_Y; y < REST_Y + REST_H; ++y) {
        for (x = REST_X; x < REST_X + REST_W; ++x) {
            if (x >= 0 && x < FB_W && y >= 0 && y < FB_H &&
                M11_FB_DECODE_INDEX(fb[y * FB_W + x]) == 0x00) {
                ++count;
            }
        }
    }
    return count;
}

static int require_portrait(const char* label, const RectEvidence* ev) {
    int ok = 1;
    if (ev->warm < WARM_MIN) {
        FAIL("%s warm portrait pixels %d < %d", label, ev->warm, WARM_MIN);
        ok = 0;
    }
    if (ev->compared <= 0 || ev->pct < MATCH_MIN_PCT) {
        FAIL("%s ordinal %d match %d%% (%d/%d) < %d%%",
             label, ORDINAL_TARGET, ev->pct, ev->matched, ev->compared,
             MATCH_MIN_PCT);
        ok = 0;
    }
    return ok;
}

static int require_d1c_zone(M11_GameViewState* game) {
    int x = 0, y = 0, w = 0, h = 0;
    M11_GameView_GetD1CWallOrnamentZone(game, &x, &y, &w, &h);
    if (x != D1C_ZONE_X || y != D1C_ZONE_Y || w != D1C_ZONE_W || h != D1C_ZONE_H) {
        FAIL("D1C zone got=(%d,%d,%d,%d) want=(%d,%d,%d,%d)",
             x, y, w, h, D1C_ZONE_X, D1C_ZONE_Y, D1C_ZONE_W, D1C_ZONE_H);
        return 0;
    }
    if (PORTRAIT_X < x || PORTRAIT_Y < y ||
        PORTRAIT_X + PORTRAIT_W > x + w ||
        PORTRAIT_Y + PORTRAIT_H > y + h) {
        FAIL("portrait rect not inside D1C zone");
        return 0;
    }
    return 1;
}

static int draw_pre_rest(M11_GameViewState* game,
                         const M11_AssetSlot* portraits,
                         unsigned char* fb,
                         int* baselineBlack) {
    RectEvidence ev;
    int front;
    int ok = 1;
    reset_pose(game, 1, 2, DIR_NORTH);
    front = M11_GameView_GetFrontMirrorOrdinal(game);
    if (front != ORDINAL_TARGET) {
        if (front != ORDINAL_SOURCE || !retarget_front_c127(game)) {
            fprintf(stderr,
                    "SKIP no retargetable ordinal-%d C127 sensor at the Hall entry (front=%d)\n",
                    ORDINAL_SOURCE, front);
            return -1;
        }
        reset_pose(game, 1, 2, DIR_NORTH);
        front = M11_GameView_GetFrontMirrorOrdinal(game);
        if (front != ORDINAL_TARGET) {
            fprintf(stderr, "SKIP retarget yielded front ordinal %d, not %d\n",
                    front, ORDINAL_TARGET);
            return -1;
        }
    }

    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect(portraits, fb, &ev);
    ok &= require_d1c_zone(game);
    ok &= require_portrait("pre_rest", &ev);
    *baselineBlack = count_rest_black(fb);
    if (game->resting != 0) {
        FAIL("pre_rest resting flag %d != 0", game->resting);
        ok = 0;
    }
    printf("  pre_rest front=%d warm=%d match=%d%% black=%d\n",
           front, ev.warm, ev.pct, *baselineBlack);
    return ok;
}

static int draw_resting(M11_GameViewState* game,
                        const M11_AssetSlot* portraits,
                        unsigned char* fb) {
    RectEvidence ev;
    int black;
    int ok = 1;
    game->resting = 1;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect(portraits, fb, &ev);
    black = count_rest_black(fb);
    if (black < REST_BLACK_MIN) {
        FAIL("resting overlay black pixels %d < %d", black, REST_BLACK_MIN);
        ok = 0;
    }
    ok &= require_d1c_zone(game);
    if (game->resting != 1) {
        FAIL("resting flag %d != 1", game->resting);
        ok = 0;
    }
    printf("  resting front=%d warm=%d match=%d%% black=%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game), ev.warm, ev.pct, black);
    return ok;
}

static int draw_wake(M11_GameViewState* game,
                     const M11_AssetSlot* portraits,
                     unsigned char* fb,
                     int baselineBlack) {
    RectEvidence ev;
    int black;
    int ok = 1;
    game->resting = 0;
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect(portraits, fb, &ev);
    black = count_rest_black(fb);
    if (black > baselineBlack + WAKE_BLACK_TOLERANCE) {
        FAIL("wake black pixels %d > baseline %d + %d",
             black, baselineBlack, WAKE_BLACK_TOLERANCE);
        ok = 0;
    }
    ok &= require_d1c_zone(game);
    ok &= require_portrait("wake", &ev);
    if (game->resting != 0) {
        FAIL("wake resting flag %d != 0", game->resting);
        ok = 0;
    }
    printf("  wake front=%d warm=%d match=%d%% black=%d\n",
           M11_GameView_GetFrontMirrorOrdinal(game), ev.warm, ev.pct, black);
    return ok;
}

static int draw_cycles(M11_GameViewState* game, const M11_AssetSlot* portraits) {
    static unsigned char first[FB_W * FB_H];
    static unsigned char fb[FB_W * FB_H];
    RectEvidence baseEv;
    int cycle;
    int ok = 1;
    memset(&baseEv, 0, sizeof(baseEv));
    for (cycle = 0; cycle < CYCLE_COUNT; ++cycle) {
        RectEvidence ev;
        game->resting = 1;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        game->resting = 0;
        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(game, fb, FB_W, FB_H);
        collect_rect(portraits, fb, &ev);
        if (cycle == 0) {
            memcpy(first, fb, sizeof(first));
            baseEv = ev;
            ok &= require_portrait("cycle0", &ev);
        } else {
            if (memcmp(first, fb, sizeof(first)) != 0) {
                FAIL("cycle %d post-wake framebuffer drift", cycle + 1);
                ok = 0;
            }
            if (ev.warm != baseEv.warm || ev.pct != baseEv.pct) {
                FAIL("cycle %d evidence drift warm=%d/%d pct=%d/%d",
                     cycle + 1, ev.warm, baseEv.warm, ev.pct, baseEv.pct);
                ok = 0;
            }
        }
    }
    printf("  cycles=%d post_wake warm=%d match=%d%%\n",
           CYCLE_COUNT, baseEv.warm, baseEv.pct);
    return ok;
}

static int draw_side_negative(M11_GameViewState* game,
                              const M11_AssetSlot* portraits,
                              unsigned char* fb) {
    RectEvidence ev;
    int ok = 1;
    reset_pose(game, 1, 2, DIR_EAST);
    memset(fb, 0, FB_W * FB_H);
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    collect_rect(portraits, fb, &ev);
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        FAIL("side pose front ordinal %d != -1",
             M11_GameView_GetFrontMirrorOrdinal(game));
        ok = 0;
    }
    if (ev.compared > 0 && ev.matched * 100 >= ev.compared * FLOAT_MAX_PCT) {
        FAIL("side pose floats ordinal %d match=%d%% (%d/%d)",
             ORDINAL_TARGET, ev.pct, ev.matched, ev.compared);
        ok = 0;
    }
    printf("  side front=%d stale_match=%d%% (%d/%d)\n",
           M11_GameView_GetFrontMirrorOrdinal(game), ev.pct, ev.matched, ev.compared);
    return ok;
}

int main(int argc, char** argv) {
    const char* dataDir;
    static M12_StartupMenuState menu;
    static M11_GameViewState game;
    static unsigned char fb[FB_W * FB_H];
    const M11_AssetSlot* portraits;
    int rc;
    int baselineBlack = 0;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    setvbuf(stdout, NULL, _IONBF, 0);
    setvbuf(stderr, NULL, _IONBF, 0);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }
    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL C026 champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 HoC ordinal 4 wake_repaint portrait_rect_position ===\n");
    rc = draw_pre_rest(&game, portraits, fb, &baselineBlack);
    if (rc < 0) {
        M11_GameView_Shutdown(&game);
        printf("SKIP dm1 v1 HoC ordinal 4 wake_repaint portrait_rect_position\n");
        return 0;
    }
    if (rc) PASS();
    if (draw_resting(&game, portraits, fb)) PASS();
    if (draw_wake(&game, portraits, fb, baselineBlack)) PASS();
    if (draw_cycles(&game, portraits)) PASS();
    if (draw_side_negative(&game, portraits, fb)) PASS();

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
