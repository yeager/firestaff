/*
 * DM1 V1 Hall of Champions portrait 00 D1C rectangle runtime probe.
 *
 * The real PC 3.4 Hall north-entry route at (map 0, x=1, y=2, NORTH)
 * owns a C127 champion-portrait sensor on the front square.  The shipped
 * data uses sensorData=1 there (HALK), so this probe keeps the real asset
 * route and temporarily seeds that C127 sensor to sensorData=0 to lock the
 * ordinal-zero edge case.  This catches regressions where portrait index 0
 * is treated as false/absent instead of a valid C026 portrait strip entry.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensor data in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 at the fixed D1C wall box.
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 draws the viewport far-to-near.
 *   ReDMCSB REVIVE.C F0280 materializes the candidate from that ordinal.
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
    PROBE_PORTRAIT_X = PROBE_VIEWPORT_X + 96,
    PROBE_PORTRAIT_Y = PROBE_VIEWPORT_Y + 35,
    PROBE_PORTRAIT_W = 32,
    PROBE_PORTRAIT_H = 29,
    PROBE_PORTRAIT_ORDINAL = 0
};

static int g_pass = 0;
static int g_fail = 0;

#define CHECK(cond, msg) do { \
    if (cond) { ++g_pass; printf("  PASS: %s\n", msg); } \
    else      { ++g_fail; printf("  FAIL: %s\n", msg); } \
} while (0)

static int expect_int(const char* label, int got, int want) {
    char msg[160];
    snprintf(msg, sizeof(msg), "%s got=%d want=%d", label, got, want);
    CHECK(got == want, msg);
    return got == want;
}

static int expect_string(const char* label,
                         const char* got,
                         const char* want) {
    char msg[192];
    snprintf(msg, sizeof(msg), "%s got=\"%s\" want=\"%s\"",
             label, got ? got : "", want);
    CHECK(got && strcmp(got, want) == 0, msg);
    return got && strcmp(got, want) == 0;
}

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

static int portrait_match_percent(const M11_AssetSlot* portraits,
                                  const unsigned char* fb,
                                  int ordinal) {
    int x;
    int y;
    int matched = 0;
    int compared = 0;
    if (!portraits || !portraits->loaded || !portraits->pixels || !fb) {
        return 0;
    }
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
            src = (unsigned char)
                (portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
            if (src == 1) {
                continue;
            }
            dst = M11_FB_DECODE_INDEX(
                fb[(PROBE_PORTRAIT_Y + y) * PROBE_FB_W +
                   (PROBE_PORTRAIT_X + x)]);
            ++compared;
            if (dst == src) {
                ++matched;
            }
        }
    }
    return compared > 0 ? (matched * 100 / compared) : 0;
}

static void set_pose(M11_GameViewState* state,
                     int mapX,
                     int mapY,
                     int direction) {
    state->world.party.mapIndex = 0;
    state->world.party.mapX = mapX;
    state->world.party.mapY = mapY;
    state->world.party.direction = direction;
    state->showDebugHUD = 0;
    state->candidateMirrorOrdinal = -1;
    state->candidateMirrorPartyIndex = -1;
    state->candidateMirrorPanelActive = 0;
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    char name[64];
    int seededSensor;
    int frontOrdinal;
    int matchPct;
    int initialCount;
    int selectRc;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    printf("=== DM1 V1 champion mirror portrait 00 D1C rect probe ===\n");
    printf("dataDir=%s\n", dataDir);

    M12_StartupMenu_InitWithDataDir(&menu, dataDir, NULL);
    M11_GameView_Init(&game);
    if (!M11_GameView_OpenSelectedMenuEntry(&game, &menu)) {
        fprintf(stderr, "FAIL could not open DM1 V1 from %s\n", dataDir);
        M11_GameView_Shutdown(&game);
        return 1;
    }

    portraits = M11_AssetLoader_Load(&game.assetLoader,
        (unsigned int)M11_GameView_GetV1ChampionPortraitGraphicId());
    CHECK(portraits && portraits->loaded && portraits->pixels &&
          portraits->width >= 256 && portraits->height >= 87,
          "C026 champion portrait strip is loaded");

    name[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(&game,
        PROBE_PORTRAIT_ORDINAL, name, (int)sizeof(name));
    (void)expect_string("mirror ordinal 0 catalog name", name, "DAROOU");

    set_pose(&game, 1, 2, DIR_NORTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    (void)expect_int("real north-entry route starts at HALK ordinal",
                     frontOrdinal, 1);

    seededSensor = seed_first_c127_data(&game, 1, PROBE_PORTRAIT_ORDINAL);
    CHECK(seededSensor >= 0,
          "seeded real north-entry C127 sensor data from 1 to 0");

    set_pose(&game, 1, 2, DIR_NORTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    (void)expect_int("seeded north-entry route reports ordinal 0",
                     frontOrdinal, PROBE_PORTRAIT_ORDINAL);

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    matchPct = portrait_match_percent(portraits, fb, PROBE_PORTRAIT_ORDINAL);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "portrait ordinal 0 matches C026 in D1C rect >=90%% (got %d%%)",
                 matchPct);
        CHECK(matchPct >= 90, msg);
    }

    set_pose(&game, 1, 2, DIR_EAST);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    (void)expect_int("ordinary side-wall pose has no front route",
                     frontOrdinal, -1);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    matchPct = portrait_match_percent(portraits, fb, PROBE_PORTRAIT_ORDINAL);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "ordinary side-wall pose does not show portrait 0 in D1C rect (got %d%%)",
                 matchPct);
        CHECK(matchPct < 70, msg);
    }

    set_pose(&game, 1, 2, DIR_NORTH);
    initialCount = game.world.party.championCount;
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    (void)expect_int("ordinal 0 candidate selection succeeds", selectRc, 1);
    (void)expect_int("candidate panel keeps ordinal 0",
                     game.candidateMirrorOrdinal, PROBE_PORTRAIT_ORDINAL);
    (void)expect_int("candidate panel is active",
                     game.candidateMirrorPanelActive, 1);
    (void)expect_int("candidate champion appended",
                     game.world.party.championCount, initialCount + 1);

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
