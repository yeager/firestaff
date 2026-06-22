/*
 * DM1 V1 Hall of Champions portrait 10 source-wall-entry probe.
 *
 * Focused slice: champion portrait ordinal 10 on the source-valid front
 * mirror route.  The actual-pose probe already proves the C127 route
 * is reachable from (map 0, x=1, y=3) while facing SOUTH (direction 2),
 * where the front square is the C127 sensor on cell (1,4).  The opposite
 * (1,5,NORTH) view sees the same square from the wrong wall side and must
 * not resolve a mirror under DUNGEON.C:2573/2608-2612.  This probe
 * narrows that to portrait_rect_position evidence:
 *
 *   - the real DM1 V1 C127 route resolves to portrait ordinal 10;
 *   - ordinal 10 resolves through the mirror catalog to GANDO;
 *   - C026 portrait ordinal 10 is drawn at the fixed D1C source
 *     rectangle (96,35,32,29) in viewport coordinates, inside the C346
 *     mirror frame;
 *   - adjacent wrong-wall/ordinary-side poses around the same C127
 *     sensor do not leave ordinal-10 portrait pixels floating in that
 *     D1C rectangle.
 *
 * Note: in the DM1 V1 PC 3.4 English DUNGEON.DAT mirror catalog,
 * ordinal 9 is ZED (DUKE OF BANVILLE) and ordinal 10 is GANDO
 * (THURFOOT).  The earlier actual-pose probe label used "ZED" as a
 * shorthand for the ordinal-10 sensor route, but the catalog itself
 * binds ordinal 10 to GANDO.  This probe locks that catalog
 * identity directly via M11_GameView_GetMirrorNameByOrdinal /
 * GetMirrorTitleByOrdinal rather than via the label.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps the sensor cell against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026_GRAPHIC_CHAMPION_PORTRAITS
 *     into M635_ZONE_PORTRAIT_ON_WALL after the D1C wall ornament.
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 redraws viewport cells far-to-near.
 *   ReDMCSB MOVESENS.C:1501-1503 / REVIVE.C F0280 use the same sensorData
 *     for candidate champion materialization.
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
    PROBE_PORTRAIT_VX = 96,
    PROBE_PORTRAIT_VY = 35,
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + PROBE_PORTRAIT_VX,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + PROBE_PORTRAIT_VY,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_EXPECTED_ORDINAL = 10,
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
 * Lock the source-valid (1,3) SOUTH route:
 *   C127 sensor on cell 0 of square (1,4) carries sensorData=10,
 *   so the party at (1,3) facing SOUTH resolves to mirror ordinal 10.
 *   The mirror catalog binds ordinal 10 to GANDO / THURFOOT.
 *   The D1C portrait-on-wall rectangle (96,35)-(127,63) lives inside
 *   the C346 wall-mirror frame (80,29)-(143,71) and the C026 strip
 *   cell for ordinal 10 must be the dominant blit there.
 */
static void check_gando_source_wall_entry(M11_GameViewState* game,
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

    set_pose(game, 1, 3, DIR_SOUTH);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    expect_int("source-wall C127 ordinal at (1,3,SOUTH)",
               ord, PROBE_EXPECTED_ORDINAL);

    memset(name, 0, sizeof(name));
    memset(title, 0, sizeof(title));
    expect_true("ordinal 10 mirror catalog name is GANDO",
                M11_GameView_GetMirrorNameByOrdinal(game, PROBE_EXPECTED_ORDINAL,
                                                    name, (int)sizeof(name)) > 0 &&
                strcmp(name, "GANDO") == 0);
    expect_true("ordinal 10 mirror catalog title is THURFOOT",
                M11_GameView_GetMirrorTitleByOrdinal(game, PROBE_EXPECTED_ORDINAL,
                                                     title, (int)sizeof(title)) > 0 &&
                strcmp(title, "THURFOOT") == 0);

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
    expect_true("ordinal 10 pixels match at D1C rect >= 90%",
                match.compared > 0 &&
                match.expectedMatched * 100 >= match.compared * 90);

    printf("  INFO: source_wall_entry name=%s title=%s rect=(%d,%d,%d,%d) "
           "matched=%d/%d best=%d\n",
           name, title,
           PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
           PROBE_PORTRAIT_W, PROBE_PORTRAIT_H,
           match.expectedMatched, match.compared, match.bestOrdinal);
}

/*
 * Render the source-valid (1,3,SOUTH) portrait at the D1C rect first to seed the
 * fb, then verify that wrong-wall/ordinary-side poses around the
 * (1,4) C127 sensor do not leave stale ordinal-10 portrait pixels
 * floating in that rectangle.
 */
static void check_no_floating_from_wrong_wall(M11_GameViewState* game,
                                              const M11_AssetSlot* portraits,
                                              unsigned char* fb,
                                              int mapX,
                                              int mapY,
                                              int dir,
                                              const char* label) {
    int ord;
    int stale;
    int compared;

    set_pose(game, 1, 3, DIR_SOUTH);
    memset(fb, 0, (size_t)PROBE_FB_W * (size_t)PROBE_FB_H);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    set_pose(game, mapX, mapY, dir);
    ord = M11_GameView_GetFrontMirrorOrdinal(game);
    M11_GameView_Draw(game, fb, PROBE_FB_W, PROBE_FB_H);

    compared = ordinal_compared_count(portraits, PROBE_EXPECTED_ORDINAL);
    stale = count_ordinal_pixels_at(portraits, fb,
                                    PROBE_PORTRAIT_X, PROBE_PORTRAIT_Y,
                                    PROBE_EXPECTED_ORDINAL);

    expect_int(label, ord, -1);
    expect_true("wrong-wall/ordinary-side pose does not float ordinal-10 portrait in D1C rect",
                compared > 0 && stale * 100 < compared * 35);
    printf("  INFO: %s stale ordinal-10 pixels=%d/%d\n", label, stale, compared);
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
        printf("SKIP dm1_v1_champion_mirror_portrait10_rect_position_runtime_probe "
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

    printf("=== DM1 V1 Hall portrait 10 source_wall_entry / portrait_rect_position ===\n");
    printf("dataDir=%s\n", dataDir);
    check_gando_source_wall_entry(&game, portraits, fb);
    /* Wrong-wall poses around the (1,4) C127 sensor:
     *   - (1,5) NORTH: front=(1,4) is the same sensor square, but
     *     viewed from the wrong wall side.
     *   - (0,4) EAST: front=(1,4) from a different wrong wall side.
     *   - (2,4) WEST: front=(1,4) from the remaining wrong wall side. */
    check_no_floating_from_wrong_wall(&game, portraits, fb, 1, 5, DIR_NORTH,
                                      "opposite north view of sensor square has no mirror ordinal");
    check_no_floating_from_wrong_wall(&game, portraits, fb, 0, 4, DIR_EAST,
                                      "east view of sensor square wrong wall has no mirror ordinal");
    check_no_floating_from_wrong_wall(&game, portraits, fb, 2, 4, DIR_WEST,
                                      "west view of sensor square wrong wall has no mirror ordinal");

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
