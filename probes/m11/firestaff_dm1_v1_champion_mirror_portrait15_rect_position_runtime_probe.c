/*
 * DM1 V1 Hall of Champions portrait 15 front_north_entry probe.
 *
 * Focused slice: champion portrait ordinal 15 on the front_north_entry
 * route (MOPHUS / "THE HEALER").  This narrow probe pins the assigned
 * portrait_rect_position invariant in one place:
 *
 *   - the real DM1 V1 C127 north-wall sensor on cell (2,5) is reached
 *     by party (2,4) facing SOUTH, with sensorData=15 stored in G0289;
 *   - ordinal 15 resolves through the mirror catalog to MOPHUS /
 *     "THE HEALER" (not DAROOU at ordinal 0, not GANDO at ordinal 10,
 *     not WUUF at ordinal 13);
 *   - C026 portrait ordinal 15 is drawn in the D1C source rectangle
 *     (96,35)-(127,63) in viewport coordinates, parented inside the
 *     C346 D1C wall-mirror frame (80,29)-(143,71);
 *   - the D1C portrait rectangle is the dominant C026 ordinal-15 blit
 *     in that viewport area, matching the source-locked 32x29 cell
 *     of the C026 8x3 champion-portrait strip;
 *   - turning away from that front route does not leave ordinal-15
 *     pixels floating in the same D1C portrait rectangle on ordinary
 *     side or back walls.
 *
 * "front_north_entry" = the party enters the (2,5) mirror cell from
 * the north by standing at (2,4) and facing south, so the source-
 * visible wall cell of the (2,5) C127 sensor is its NORTH wall.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps the C127 sensor cell against party
 *   direction so only the front-wall side of the wall sets G0289.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026_GRAPHIC_CHAMPION_PORTRAITS
 *   into G0109_auc_Graphic558_Box_ChampionPortraitOnWall
 *   = {96, 127, 35, 63} (the fixed D1C portrait-on-wall rectangle).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 redraws the viewport from the
 *   current party pose, far-to-near.
 *   ReDMCSB MOVESENS.C:1501-1503 / REVIVE.C F0280 use the same C127
 *   sensorData as the candidate champion ordinal.
 *
 * Cross-probe context (deliberately NOT duplicated here):
 *   - firestaff_dm1_v1_champion_mirror_actual_pose_runtime_probe locks
 *     the front-cell ordinal=15 lookup at (2,4) SOUTH and the matching
 *     HALK resurrect round-trip.
 *   - firestaff_dm1_v1_champion_mirror_zorder_reblt_runtime_probe
 *     locks the 192/192 pixel-match for ordinal 15 at the D1C rect
 *     after a sequence of valid + blocked reblits.
 *   - firestaff_dm1_v1_champion_mirror_capture_probe saves the
 *     (2,4) SOUTH ordinal-15 PPM and warm-pixel JSON row.
 *
 * This probe adds three things those don't fully cover together:
 *   1) the C346 wall-mirror frame zone (80,29)-(143,71) and the
 *      containment of the C026 portrait rectangle inside it;
 *   2) the mirror-catalog name+title strings for ordinal 15;
 *   3) the explicit no-floating regression from the ordinal-15 route
 *      when the party turns EAST/WEST/SOUTH around the (2,4) cell.
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
    /* G0109_auc_Graphic558_Box_ChampionPortraitOnWall = {96,127,35,63}
     * (ReDMCSB DUNVIEW.C:525).  This is the source-locked D1C portrait
     * rectangle blit destination; the portrait is 32x29 from the C026
     * 8x3 champion-portrait strip (DUNVIEW.C:3916). */
    PROBE_PORTRAIT_VX = 96,
    PROBE_PORTRAIT_VY = 35,
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VX,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VY,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_EXPECTED_ORDINAL = 15,
    /* C01_COLOR_DARK_GRAY transparency index used by the source blit
     * (DUNVIEW.C:3916 C01_COLOR_DARK_GRAY) — skipped from the
     * pixel-match comparison. */
    PROBE_CHAMPION_TRANSPARENT = 1
};

typedef struct PortraitMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} PortraitMatch;

static int g_pass = 0;
static int g_fail = 0;

static void expect_int(const char* label, int got, int want) {
    if (got == want) {
        ++g_pass;
        printf("  PASS: %s got=%d\n", label, got);
    } else {
        ++g_fail;
        printf("  FAIL: %s got=%d want=%d\n", label, got, want);
    }
}

static void expect_true(const char* label, int ok) {
    if (ok) {
        ++g_pass;
        printf("  PASS: %s\n", label);
    } else {
        ++g_fail;
        printf("  FAIL: %s\n", label);
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

static int ordinal_compared_count(const M11_AssetSlot* portraits, int ordinal) {
    int x;
    int y;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src != PROBE_CHAMPION_TRANSPARENT) {
                ++compared;
            }
        }
    }
    return compared;
}

static int count_ordinal_pixels_at(const M11_AssetSlot* portraits,
                                   const unsigned char* fb,
                                   int dstX,
                                   int dstY,
                                   int ordinal) {
    int x;
    int y;
    int matched = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb ||
        ordinal < 0 || ordinal >= 24) {
        return 0;
    }
    for (y = 0; y < PROBE_PORTRAIT_H; ++y) {
        for (x = 0; x < PROBE_PORTRAIT_W; ++x) {
            int srcX = (ordinal & 7) * PROBE_PORTRAIT_W + x;
            int srcY = (ordinal >> 3) * PROBE_PORTRAIT_H + y;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(dstY + y) * PROBE_FB_W + (dstX + x)]);
            if (src == PROBE_CHAMPION_TRANSPARENT) {
                continue;
            }
            if (dst == src) {
                ++matched;
            }
        }
    }
    return matched;
}

static PortraitMatch match_portrait_at(const M11_AssetSlot* portraits,
                                       const unsigned char* fb,
                                       int dstX,
                                       int dstY,
                                       int expectedOrdinal) {
    PortraitMatch out;
    int ordinal;
    memset(&out, 0, sizeof(out));
    out.bestOrdinal = -1;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return out;
    }
    for (ordinal = 0; ordinal < 24; ++ordinal) {
        int matched = count_ordinal_pixels_at(portraits, fb, dstX, dstY, ordinal);
        int compared = ordinal_compared_count(portraits, ordinal);
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

/*
 * Lock the (2,4) SOUTH front_north_entry route:
 *   C127 sensor on the north wall of cell (2,5) carries
 *   sensorData=15, so the party at (2,4) facing SOUTH resolves to
 *   mirror ordinal 15.  The mirror catalog binds ordinal 15 to
 *   MOPHUS / "THE HEALER".  The D1C portrait-on-wall rectangle
 *   (96,35)-(127,63) lives inside the C346 wall-mirror frame
 *   (80,29)-(143,71) and the C026 strip cell for ordinal 15 must
 *   be the dominant blit there.
 */
static void check_mophus_front_north_entry(M11_GameViewState* game,
                                           const M11_AssetSlot* portraits,
                                           unsigned char* fb) {
    int ord;
    int ornX = -1;
    int ornY = -1;
    int ornW = -1;
    int ornH = -1;
    char name[32];
    char title[64];
    PortraitMatch match;

    /* front_north_entry: party enters the (2,5) MOPHUS cell from the
     * north by standing at (2,4) and facing south.  DIR_SOUTH=2. */
    set_pose(game, 2, 4, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("front_north_entry C127 ordinal at (2,4,SOUTH)",
               ord, PROBE_EXPECTED_ORDINAL);

    memset(name, 0, sizeof(name));
    memset(title, 0, sizeof(title));
    expect_true("ordinal 15 mirror catalog name is MOPHUS",
                M11_GameView_GetMirrorNameByOrdinal(game, PROBE_EXPECTED_ORDINAL,
                                                    name, (int)sizeof(name)) > 0 &&
                strcmp(name, "MOPHUS") == 0);
    expect_true("ordinal 15 mirror catalog title is THE HEALER",
                M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_EXPECTED_ORDINAL,
                                                     title, (int)sizeof(title)) > 0 &&
                strcmp(title, "THE HEALER") == 0);

    expect_true("D1C wall-mirror frame zone helper succeeds",
                M11_GameView_GetD1CWallOrnamentZone(game, &ornX, &ornY, &ornW, &ornH) == 1);
    expect_int("D1C wall-mirror frame x", ornX, 80);
    expect_int("D1C wall-mirror frame y", ornY, 29);
    expect_int("D1C wall-mirror frame width", ornW, 64);
    expect_int("D1C wall-mirror frame height", ornH, 43);
    expect_int("portrait rect x is frame x + 16", PROBE_PORTRAIT_VX, ornX + 16);
    expect_int("portrait rect y is frame y + 6", PROBE_PORTRAIT_VY, ornY + 6);
    expect_true("portrait rect is contained by D1C wall-mirror frame",
                PROBE_PORTRAIT_VX >= ornX &&
                PROBE_PORTRAIT_VY >= ornY &&
                PROBE_PORTRAIT_VX + PROBE_PORTRAIT_W <= ornX + ornW &&
                PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H <= ornY + ornH);

    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    match = match_portrait_at(portraits, fb,
                              PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                              PROBE_EXPECTED_ORDINAL);
    expect_int("best portrait ordinal at D1C rect", match.bestOrdinal,
               PROBE_EXPECTED_ORDINAL);
    expect_true("ordinal 15 pixels match at D1C rect >= 90%",
                match.compared > 0 &&
                match.expectedMatched * 100 >= match.compared * 90);

    printf("  INFO: front_north_entry name=%s title=%s rect=(%d,%d,%d,%d) "
           "matched=%d/%d best=%d\n",
           name, title,
           PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           match.expectedMatched, match.compared, match.bestOrdinal);
}

/*
 * Seed the framebuffer with the (2,4) SOUTH ordinal-15 portrait
 * route, then verify that wrong-wall/ordinary-side poses around the
 * (2,4) cell do not leave stale ordinal-15 portrait pixels floating
 * in the same D1C portrait rectangle.
 *
 *   - (2,4) NORTH: front cell (2,3) has no C127 sensor on its south
 *     wall, so the source front-wall filter rejects the route and the
 *     front-cell ordinal is -1.
 *   - (2,4) EAST: front cell (3,4) is the side of the Hall row, no
 *     C127 sensor on its west wall.
 *   - (2,4) WEST: front cell (1,4) is the corridor cell, no C127
 *     sensor on its east wall (the (1,4) C127 sensor on the south
 *     wall belongs to the (1,5) NORTH ordinal-10 route).
 *
 * Each of these wrong-wall poses must not leave stale ordinal-15
 * portrait pixels floating in the D1C portrait rectangle.
 */
static void check_no_floating_from_wrong_wall(M11_GameViewState* game,
                                              const M11_AssetSlot* portraits,
                                              unsigned char* fb,
                                              int mapX,
                                              int mapY,
                                              int dir,
                                              const char* label) {
    int stale;
    int compared;
    int seedOrdinal;

    /* Seed: render the (2,4) SOUTH ordinal-15 portrait route first so
     * the framebuffer has a live ordinal-15 portrait at the D1C rect.
     * If the wrong-wall pose clears that area correctly, the count
     * drops back to the noise floor. */
    set_pose(game, 2, 4, DIR_SOUTH);
    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);
    seedOrdinal = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_true("seed: front_north_entry ordinal-15 route is active before turn",
                seedOrdinal == PROBE_EXPECTED_ORDINAL);

    /* Now turn away from the ordinal-15 route and re-render. */
    set_pose(game, mapX, mapY, dir);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    compared = ordinal_compared_count(portraits, PROBE_EXPECTED_ORDINAL);
    stale = count_ordinal_pixels_at(portraits, fb,
                                    PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                                    PROBE_EXPECTED_ORDINAL);

    /* After turning away from the ordinal-15 route, the D1C portrait
     * rectangle must not still carry the ordinal-15 portrait.  Allow
     * a small noise floor (< 35% of ordinal-15 non-transparent pixels)
     * since the wall texture can share palette indices with the
     * portrait sprite edges. */
    expect_true(label, compared > 0 && stale * 100 < compared * 35);
    printf("  INFO: %s stale ordinal-15 pixels=%d/%d\n", label, stale, compared);
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    static unsigned char fb[PROBE_FB_W * PROBE_FB_H];

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    if (!M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        printf("SKIP dm1_v1_champion_mirror_portrait15_rect_position_runtime_probe "
               "no hash-verified DM1 data under %s\n", dataDir);
        return 0;
    }
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open selected DM1 V1 game view from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
                                     (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    if (!portraits || !portraits->loaded || !portraits->pixels ||
        portraits->width < 256 || portraits->height < 87) {
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 Hall portrait 15 front_north_entry / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);
    check_mophus_front_north_entry(&game, portraits, fb);
    /* Wrong-wall poses around the (2,4) ordinal-15 sensor cell: */
    check_no_floating_from_wrong_wall(&game, portraits, fb, 2, 4, DIR_NORTH,
                                      "front_north_entry back-side ordinary wall does not float ordinal-15 portrait in D1C rect");
    check_no_floating_from_wrong_wall(&game, portraits, fb, 2, 4, DIR_EAST,
                                      "front_north_entry east-side wrong wall does not float ordinal-15 portrait in D1C rect");
    check_no_floating_from_wrong_wall(&game, portraits, fb, 2, 4, DIR_WEST,
                                      "front_north_entry west-side wrong wall does not float ordinal-15 portrait in D1C rect");

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
