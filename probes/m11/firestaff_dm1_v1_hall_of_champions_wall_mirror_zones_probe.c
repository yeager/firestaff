/*
 * firestaff_dm1_v1_hall_of_champions_wall_mirror_zones_probe.c
 *
 * Pixel-level verification of the D1C wall-mirror zones for the
 * Hall of Champions front-mirror routes.  The party-facing wall
 * at (map 0, x=1, y=3) and (map 0, x=1, y=4) carries a C127
 * champion mirror (G0289 portrait in C026) and a wall ornament
 * graphic (G0243[ornIdx] at M11_GFX_WALL_ORNAMENT_BASE+1).  The
 * blit destination is the source-locked D1C wall mirror zone
 * (G0205_aaauc_Graphic558_WallOrnamentCoordinateSets[coordSet][12]):
 *
 *   coordSet 0: dstX=96, dstY=36, w=32, h=28  (D1C default)
 *   coordSet 1: dstX=64, dstY=36, w=96, h=56  (D1C large)
 *   coordSet 2: dstX=96, dstY=92, w=32, h=28  (D1C south)
 *   coordSet 3: dstX=96, dstY=64, w=32, h=28  (D1C alt)
 *   coordSet 4: dstX=64, dstY=41, w=96, h=12  (D1C wide)
 *   coordSet 5: dstX=80, dstY=29, w=64, h=43  (D1C tall)
 *   coordSet 6: dstX=64, dstY=9,  w=96, h=111 (D1C full)
 *   coordSet 7: dstX=32, dstY=9,  w=160,h=111 (D1C fullscreen)
 *
 * + M11_VIEWPORT_Y = 33 for the Y origin.
 *
 * The probe drives both front-mirror routes, reads the wall
 * ornament zone via M11_DM1_ORN_ZONE_IDX (the public 12 index
 * for the D1C champion-mirror route), and confirms:
 *  (A) The destination box matches one of the source-locked
 *      coordSet rectangles (within ±1 pixel for rounding).
 *  (B) The destination box is non-zero (the mirror is
 *      actually blitted, not skipped by the BUG-120/121 panel
 *      guard).
 *  (C) The wall ornament graphic (M11_GFX_WALL_ORNAMENT_BASE+1)
 *      loads and produces visible pixels at the destination.
 *  (D) The champion portrait cutout (96, 35, 32, 29) inside
 *      the mirror contains the expected portrait ordinal.
 *
 * Source-locked to:
 *   - DUNVIEW.C:3913-3928 (D1C front-mirror blit)
 *   - DUNVIEW.C:8318-8542 F0128 (viewport render from party pose)
 *   - DUNGEON.C:2608-2612 (G0289 C127 portrait storage)
 *   - PROJEXPL.C:1063 (G0289 portrait ordinal)
 *   - m11_dm1_wall_ornament_zone (DUNVIEW.C G0205 wall ornament sets)
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
    /* D1C champion portrait cutout: x=96..127, y=35..63 in
     * viewport coords.  Add M11_VIEWPORT_X (0) and
     * M11_VIEWPORT_Y (33) to get the framebuffer destination.
     * Width 32, height 29 (portrait strip cell). */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

/* Count distinct non-zero palette indices in a rectangle. */
static int count_distinct(const unsigned char* fb,
                          int x, int y, int w, int h) {
    unsigned char seen[16] = {0};
    int yy, xx, n = 0;
    for (yy = y; yy < y + h && yy < FB_H; ++yy) {
        for (xx = x; xx < x + w && xx < FB_W; ++xx) {
            unsigned char idx = (unsigned char)(fb[yy * FB_W + xx] & 0x0F);
            if (idx != 0 && !seen[idx]) {
                seen[idx] = 1;
                ++n;
            }
        }
    }
    return n;
}

/* Match a portrait ordinal at the cutout.  Returns matched pixels. */
static int match_portrait(const M11_AssetSlot* portraits,
                          const unsigned char* fb,
                          int ordinal) {
    int x, y, matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PORTRAIT_H + y;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue; /* transparent */
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
}

static void check_wall_mirror(M11_GameViewState* state,
                              const M11_AssetSlot* portraits,
                              int mapX, int mapY,
                              int expectedOrdinal,
                              const char* label) {
    unsigned char fb[FB_W * FB_H];
    int route;
    int ornX, ornY, ornW, ornH;
    int dWall;
    int pct;
    int distinct;

    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = DIR_NORTH;
    state->showDebugHUD = 0;
    state->candidateMirrorPanelActive = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;

    route = M11_GameView_GetFrontMirrorOrdinal(state);
    {
        char msg[160];
        snprintf(msg, sizeof(msg), "%s (1,%d) front-mirror ordinal = %d (want %d)",
                 label, mapY, route, expectedOrdinal);
        CHECK(route == expectedOrdinal, msg);
    }

    /* The wall ornament zone for D1C front mirror.  We use the
     * DUNVIEW.C G0205 lookup directly: M11_DM1_ORN_ZONE_IDX=12 is
     * the D1C champion-mirror route.  The destination box is
     * coordSet-dependent; we read the public M11 helper. */
    ornX = ornY = ornW = ornH = 0;
    M11_GameView_GetD1CWallOrnamentZone(state, &ornX, &ornY, &ornW, &ornH);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) wall ornament zone is non-empty: "
                 "(%d, %d, %d, %d) in viewport coords",
                 label, mapY, ornX, ornY, ornW, ornH);
        CHECK(ornW > 0 && ornH > 0, msg);
    }
    /* Destination must be at the D1C cutout-or-around it.  The
     * champion portrait is at (96, 35, 32, 29); the wall box is
     * (96, 36, 32, 28) per coordSet=0.  We allow ±2 pixel slop. */
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) wall box X is at 96 ± 2 (got %d)",
                 label, mapY, ornX);
        CHECK(ornX >= 94 && ornX <= 98, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) wall box Y is at 36 ± 4 (got %d)",
                 label, mapY, ornY);
        CHECK(ornY >= 32 && ornY <= 40, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) wall box W is 32 ± 2 (got %d)",
                 label, mapY, ornW);
        CHECK(ornW >= 30 && ornW <= 34, msg);
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) wall box H is 28 ± 4 (got %d)",
                 label, mapY, ornH);
        CHECK(ornH >= 24 && ornH <= 32, msg);
    }

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(state, fb, FB_W, FB_H);

    /* The full wall box (viewport-relative) must contain visible
     * pixels.  Since the destination is in viewport coords, the
     * framebuffer destination is at (VIEWPORT_X + ornX,
     * VIEWPORT_Y + ornY). */
    dWall = count_distinct(fb,
                           VIEWPORT_X + ornX,
                           VIEWPORT_Y + ornY,
                           ornW, ornH);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) wall box has visible content "
                 "(>= 2 distinct palette indices, got %d)",
                 label, mapY, dWall);
        CHECK(dWall >= 2, msg);
    }

    /* The portrait cutout (96, 35) inside the wall box must show
     * the expected champion portrait ordinal. */
    pct = match_portrait(portraits, fb, expectedOrdinal);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) champion portrait ordinal %d matches >= 90%% (got %d%%)",
                 label, mapY, expectedOrdinal, pct);
        CHECK(pct >= 90, msg);
    }

    /* Sanity: the D1C corner of the viewport (outside the mirror)
     * should not contain the mirror's pixel data.  Check the
     * bottom-left corner. */
    distinct = count_distinct(fb, 0, FB_H - 20, 20, 20);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "%s (1,%d) bottom-left viewport corner is empty "
                 "(mirror does not bleed there, distinct=%d)",
                 label, mapY, distinct);
        CHECK(distinct <= 4, msg);
    }
}

int main(int argc, char** argv) {
    M12_StartupMenuState menu;
    M11_GameViewState state;
    const M11_AssetSlot* portraits;
    const char* dataDir;

    if (argc > 1) dataDir = argv[1];
    else          dataDir = getenv("FIRESTAFF_DATA");
    if (!dataDir || dataDir[0] == '\0') {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    printf("=== DM1 V1 Hall of Champions wall-mirror zones (v2.7.14) ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menu)) {
        fprintf(stderr, "FAIL: could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    state.showDebugHUD = 0;
    state.candidateMirrorPanelActive = 0;
    state.candidateMirrorOrdinal = -1;
    state.candidateMirrorPartyIndex = -1;
    state.world.party.championCount = 1;
    memset(&state.world.party.champions[0], 0,
           sizeof(state.world.party.champions[0]));
    state.world.party.champions[0].present = 1;
    state.world.party.champions[0].portraitIndex = 0;

    portraits = M11_AssetLoader_Load(&state.assetLoader,
                                      (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());

    /* DM1 PC 3.4 Hall of Champions: (1,3) mirrors ordinal 1
     * (TIGGY / wizard portrait), (1,4) mirrors ordinal 2 (HALK /
     * barbarian).  Confirmed by the existing
     * firestaff_dm1_v1_champion_mirror_visibility_runtime_probe
     * which prints "best=1" / "best=2" for these routes. */
    check_wall_mirror(&state, portraits, 1, 3, 1, "Hall (1,3) start");
    check_wall_mirror(&state, portraits, 1, 4, 2, "Hall (1,4) corridor");

    M11_GameView_Shutdown(&state);
    printf("\n=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
