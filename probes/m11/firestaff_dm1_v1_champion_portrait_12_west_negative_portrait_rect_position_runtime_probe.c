/*
 * firestaff_dm1_v1_champion_portrait_12_west_negative_portrait_rect_position_runtime_probe.c
 *
 * DM1 V1 Hall of Champions slice 12: portrait ordinal 12, route
 * west_negative, aspect portrait_rect_position.
 *
 * The Hall of Champions carries a small set of C127 champion-portrait
 * sensors on the front wall of certain corridor cells (DUNGEON.C:2608-2612).
 * A brute-force exploration of the reference DM1 V1 PC 3.4 DUNGEON.DAT
 * (map 0, 8x8 grid, 4 directions) reveals the following C127 mirror
 * ordinals that route to a front-mirror candidate:
 *
 *   (2,1,E)=8  (2,1,S)=4          LEIF cluster
 *   (1,2,N)=1                     HALK
 *   (1,3,E)=18 (1,3,S)=10         SONJA / ZED
 *   (2,3,E)=19
 *   (0,4,E)=10
 *   (2,4,E)=6 (2,4,S)=15 (2,4,W)=10  MOPHUS cluster + ZED on east of (1,4)
 *   (1,5,N)=10 (1,5,S)=13         ZED / WUUF
 *   (0,6,E)=13
 *   (2,6,W)=13
 *   (3,6,N)=11 (3,6,W)=22
 *   (1,7,N)=13
 *   (2,7,S)=16
 *   (3,7,S)=3
 *
 * Ordinal 12 is NOT present anywhere in the reference Hall of
 * Champions — but it IS a valid C026 atlas cell (graphic 26 is
 * 256x87 = 8 cols x 3 rows of 32x29 portraits, ordinal 12 lives at
 * row 1, col 4 = atlas pixel (128, 29)-(160, 58), with 537 visible
 * non-blank pixels in the reference GRAPHICS.DAT). This slice is the
 * regression gate that says:
 *
 *   "No Hall of Champions pose may route ordinal 12 to the D1C
 *    portrait rectangle (96, 35, 32, 29) unless that pose's front
 *    cell has a C127 sensor with sensorData==12. In particular,
 *    no west-facing corridor pose may float ordinal 12 over the
 *    visible side wall."
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 normalizes M011_CELL(sensor) - partyDirection
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 portrait at fixed D1C box
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 draws the viewport far-to-near
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
    FB_W = 320,
    FB_H = 200,
    VIEWPORT_X = 0,
    VIEWPORT_Y = 33,
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    ATLAS_W = 256,
    ATLAS_H = 87,
    ATLAS_CELL_W = 32,
    ATLAS_CELL_H = 29,
    ORDINAL_12_ATLAS_X = 4 * ATLAS_CELL_W,
    ORDINAL_12_ATLAS_Y = 1 * ATLAS_CELL_H,
    PORTRAIT_MATCH_THRESHOLD = 90,
    NO_PORTRAIT_MATCH_THRESHOLD = 35
};

typedef struct WestPose {
    int mapX;
    int mapY;
    int expectedOrdinal;
    const char* label;
} WestPose;

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static int ordinal_match_pct(const M11_AssetSlot* portraits,
                             const unsigned char* fb,
                             int ordinal) {
    int x, y, matched = 0, compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels) return 0;
    for (y = 0; y < PORTRAIT_H; ++y) {
        for (x = 0; x < PORTRAIT_W; ++x) {
            int srcX = (ordinal % 8) * ATLAS_CELL_W + x;
            int srcY = (ordinal / 8) * ATLAS_CELL_H + y;
            if (srcX >= (int)portraits->width ||
                srcY >= (int)portraits->height) continue;
            unsigned char src =
                (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) continue;
            unsigned char dst =
                M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) ++matched;
        }
    }
    return (compared > 0) ? (matched * 100 / compared) : 0;
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

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ordinal12_atlas_x;
    int ordinal12_atlas_y;
    int ordinal12_atlas_valid;
    int ordinal12_visible_in_atlas;
    int no_ordinal12_floating_on_west = 1;
    int i;

    static const WestPose kWestPoses[] = {
        {1, 2, -1, "hall_start_west"},
        {1, 3, -1, "hall_corridor_1_west"},
        {1, 4, -1, "hall_corridor_2_west"},
        {1, 5, -1, "hall_end_west"},
        {2, 1, -1, "hall_leif_probe_west"},
        {2, 4, 10, "hall_mophus_probe_west_east_of_1_4"},
        {3, 3, -1, "hall_corridor_3_west"},
        {3, 5, -1, "hall_end_west_inner"}
    };
    int nWest = (int)(sizeof(kWestPoses) / sizeof(kWestPoses[0]));

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

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
        portraits->width < (unsigned int)ATLAS_W ||
        portraits->height < (unsigned int)ATLAS_H) {
        fprintf(stderr, "FAIL C026 champion portrait strip missing "
                "or wrong dimensions (got %ux%u, want >=%dx%d)\n",
                portraits ? portraits->width : 0u,
                portraits ? portraits->height : 0u,
                ATLAS_W, ATLAS_H);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 portrait ordinal 12 / west_negative / portrait_rect_position ===\n");

    ordinal12_atlas_x = ORDINAL_12_ATLAS_X;
    ordinal12_atlas_y = ORDINAL_12_ATLAS_Y;
    ordinal12_atlas_valid =
        (ordinal12_atlas_x + ATLAS_CELL_W <= (int)portraits->width) &&
        (ordinal12_atlas_y + ATLAS_CELL_H <= (int)portraits->height);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 12 atlas cell (%d,%d,%d,%d) fits inside C026 (%ux%u)",
                 ordinal12_atlas_x, ordinal12_atlas_y,
                 ATLAS_CELL_W, ATLAS_CELL_H,
                 portraits->width, portraits->height);
        CHECK(ordinal12_atlas_valid, msg);
    }

    ordinal12_visible_in_atlas = 0;
    if (ordinal12_atlas_valid) {
        int x, y;
        for (y = ordinal12_atlas_y; y < ordinal12_atlas_y + ATLAS_CELL_H; ++y) {
            for (x = ordinal12_atlas_x; x < ordinal12_atlas_x + ATLAS_CELL_W; ++x) {
                unsigned char p =
                    (unsigned char)(portraits->pixels[y * (int)portraits->width + x] & 0x0F);
                if (p != 0 && p != 1) {
                    ++ordinal12_visible_in_atlas;
                }
            }
        }
    }
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinal 12 atlas cell has visible pixels "
                 "(%d non-transparent non-blank, expected >= 100)",
                 ordinal12_visible_in_atlas);
        CHECK(ordinal12_visible_in_atlas >= 100, msg);
    }

    for (i = 0; i < nWest; ++i) {
        unsigned char fb[FB_W * FB_H];
        int ordinal;
        int ord12_match;

        set_pose(&game, kWestPoses[i].mapX, kWestPoses[i].mapY, DIR_WEST);
        ordinal = M11_GameView_GetFrontMirrorOrdinal(&game);
        {
            char msg[200];
            snprintf(msg, sizeof(msg),
                     "%s (%d,%d,W) front-mirror ordinal = %d (got %d)",
                     kWestPoses[i].label,
                     kWestPoses[i].mapX, kWestPoses[i].mapY,
                     kWestPoses[i].expectedOrdinal, ordinal);
            CHECK(ordinal == kWestPoses[i].expectedOrdinal, msg);
        }

        memset(fb, 0, sizeof(fb));
        M11_GameView_Draw(&game, fb, FB_W, FB_H);

        ord12_match = ordinal_match_pct(portraits, fb, 12);
        if (kWestPoses[i].expectedOrdinal == 12) {
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "%s (%d,%d,W) ordinal 12 match >= %d%% (got %d%%)",
                         kWestPoses[i].label,
                         kWestPoses[i].mapX, kWestPoses[i].mapY,
                         PORTRAIT_MATCH_THRESHOLD, ord12_match);
                CHECK(ord12_match >= PORTRAIT_MATCH_THRESHOLD, msg);
            }
        } else {
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "%s (%d,%d,W) ordinal 12 match < %d%% (got %d%%)",
                         kWestPoses[i].label,
                         kWestPoses[i].mapX, kWestPoses[i].mapY,
                         NO_PORTRAIT_MATCH_THRESHOLD, ord12_match);
                CHECK(ord12_match < NO_PORTRAIT_MATCH_THRESHOLD, msg);
            }
            if (ord12_match >= NO_PORTRAIT_MATCH_THRESHOLD) {
                no_ordinal12_floating_on_west = 0;
            }
        }
    }

    CHECK(no_ordinal12_floating_on_west,
          "ordinal 12 does not float over any west-facing Hall corridor pose");

    {
        struct {
            int mapX;
            int mapY;
            int dir;
            int expectedOrdinal;
            const char* label;
        } kPositive[] = {
            {1, 2, DIR_NORTH,  1, "halk_n_ord_1"},
            {2, 1, DIR_SOUTH,  4, "leif_s_ord_4"},
            {1, 3, DIR_EAST,  18, "sonja_e_ord_18"},
            {1, 5, DIR_NORTH, 10, "zed_n_ord_10"},
            {2, 4, DIR_SOUTH, 15, "mophus_s_ord_15"},
            {1, 5, DIR_SOUTH, 13, "wuuf_s_ord_13"}
        };
        int k;
        for (k = 0; k < (int)(sizeof(kPositive) / sizeof(kPositive[0])); ++k) {
            unsigned char fb[FB_W * FB_H];
            int ord;
            int ord12_match;
            int dominantOrdinal;
            int bestMatch;
            int ordIdx;
            set_pose(&game, kPositive[k].mapX, kPositive[k].mapY, kPositive[k].dir);
            ord = M11_GameView_GetFrontMirrorOrdinal(&game);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "positive %s (%d,%d,%d) front ordinal = %d (got %d)",
                         kPositive[k].label,
                         kPositive[k].mapX, kPositive[k].mapY, kPositive[k].dir,
                         kPositive[k].expectedOrdinal, ord);
                CHECK(ord == kPositive[k].expectedOrdinal, msg);
            }

            memset(fb, 0, sizeof(fb));
            M11_GameView_Draw(&game, fb, FB_W, FB_H);

            ord12_match = ordinal_match_pct(portraits, fb, 12);
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "positive %s ordinal 12 match < 25%% "
                         "(expected ordinal %d dominates; got ord12=%d%%)",
                         kPositive[k].label, kPositive[k].expectedOrdinal, ord12_match);
                CHECK(ord12_match < 25, msg);
            }

            dominantOrdinal = -1;
            bestMatch = -1;
            for (ordIdx = 0; ordIdx < 24; ++ordIdx) {
                int m = ordinal_match_pct(portraits, fb, ordIdx);
                if (m > bestMatch) {
                    bestMatch = m;
                    dominantOrdinal = ordIdx;
                }
            }
            {
                char msg[200];
                snprintf(msg, sizeof(msg),
                         "positive %s dominant portrait in D1C rect = ordinal %d "
                         "(expected %d, bestMatch=%d%%)",
                         kPositive[k].label, dominantOrdinal,
                         kPositive[k].expectedOrdinal, bestMatch);
                CHECK(dominantOrdinal == kPositive[k].expectedOrdinal &&
                      bestMatch >= PORTRAIT_MATCH_THRESHOLD,
                      msg);
            }
        }
    }

    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return (g_fail == 0) ? 0 : 1;
}
