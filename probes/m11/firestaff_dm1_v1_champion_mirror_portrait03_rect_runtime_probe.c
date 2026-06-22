/*
 * DM1 V1 Hall of Champions portrait 03 D1C rectangle runtime probe.
 *
 * The real PC 3.4 Hall north-entry route at (map 0, x=1, y=2, NORTH)
 * owns a C127 champion-portrait sensor on the front square.  The shipped
 * data uses sensorData=1 there (HALK), so this probe keeps the real asset
 * route and temporarily seeds that C127 sensor to sensorData=3 to lock the
 * ordinal-3 portrait_rect_position slice.  This catches regressions where
 * portrait index 3 is treated as false/absent instead of a valid C026
 * portrait strip entry, or where the D1C portrait rectangle floats off the
 * C346 wall-mirror frame onto an ordinary side wall.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against party direction.
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensor data in G0289.
 *   ReDMCSB DUNVIEW.C:3913-3928 blits C026 into the D1C champion-portrait
 *     rectangle (96,35)-(127,63) parented at +16,+6 inside the C346 D1C
 *     wall-mirror frame (80,29,64,43).
 *   ReDMCSB DUNVIEW.C:8318-8542 F0128 redraws the viewport from the
 *     current party pose, far-to-near.
 *   ReDMCSB MOVESENS.C:1501-1503 / REVIVE.C F0280 use the same C127
 *     sensorData as the candidate champion ordinal.
 *   ReDMCSB ENDGAME.C:327-394 lists 24 PC 3.4 champion ordinals;
 *     ordinal 3 is the third TextString-parsed champion in the
 *     Hall of Champions mirror catalog (DAROOU=0, HALK=1, then 3).
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
    PROBE_PORTRAIT_ORDINAL = 3,
    PROBE_PORTRAIT_TRANSPARENT = 1,
    PROBE_FRAME_X = 80,
    PROBE_FRAME_Y = 29,
    PROBE_FRAME_W = 64,
    PROBE_FRAME_H = 43
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

static int expect_string_nonempty(const char* label,
                                  const char* got) {
    char msg[192];
    snprintf(msg, sizeof(msg), "%s got=\"%s\"",
             label, got ? got : "(null)");
    CHECK(got && got[0] != '\0', msg);
    return got && got[0] != '\0';
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
            if (src == PROBE_PORTRAIT_TRANSPARENT) {
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

static void dump_catalog(M11_GameViewState* game, int maxOrdinal) {
    int ord;
    printf("[catalog] DM1 V1 Hall mirror ordinal → name/title (first %d)\n",
           maxOrdinal);
    for (ord = 0; ord < maxOrdinal; ++ord) {
        char name[64];
        char title[64];
        int rc;
        name[0] = '\0';
        title[0] = '\0';
        rc = M11_GameView_GetMirrorNameByOrdinal(game, ord, name, (int)sizeof(name));
        if (rc > 0) {
            (void)M11_GameView_GetMirrorTitleByOrdinal(game, ord, title,
                                                       (int)sizeof(title));
        }
        printf("[catalog]   ordinal=%d name=\"%s\" title=\"%s\"\n",
               ord, name, title);
    }
}

int main(int argc, char** argv) {
    const char* dataDir;
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    unsigned char fb[PROBE_FB_W * PROBE_FB_H];
    char name[64];
    char title[64];
    int catalogCount;
    int seededSensor;
    int frontOrdinal;
    int matchPct;
    int ornX = -1, ornY = -1, ornW = -1, ornH = -1;
    int initialCount;
    int selectRc;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];

    printf("=== DM1 V1 champion mirror portrait 03 D1C rect probe ===\n");
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

    catalogCount = M11_GameView_GetMirrorCatalogCount(&game);
    CHECK(catalogCount >= 4,
          "Hall mirror catalog has at least 4 entries (ordinal 3 must exist)");
    printf("[catalog] ordinal count=%d\n", catalogCount);
    dump_catalog(&game, catalogCount < 8 ? catalogCount : 8);

    /* Ordinal 3 must resolve to a non-empty champion name so the seeded
     * C127 sensor data path is meaningful; ordinal 0 (DAROOU) and 1 (HALK)
     * ship on the real PC 3.4 DUNGEON.DAT, and ordinal 3 is the third
     * mirror catalog entry per ReDMCSB ENDGAME.C:327-394. */
    name[0] = '\0';
    title[0] = '\0';
    (void)M11_GameView_GetMirrorNameByOrdinal(&game, PROBE_PORTRAIT_ORDINAL,
                                              name, (int)sizeof(name));
    (void)M11_GameView_GetMirrorTitleByOrdinal(&game, PROBE_PORTRAIT_ORDINAL,
                                               title, (int)sizeof(title));
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "mirror ordinal 3 catalog name+title resolved (name=\"%s\" title=\"%s\")",
                 name, title);
        CHECK(expect_string_nonempty("mirror ordinal 3 catalog name", name) &&
              expect_string_nonempty("mirror ordinal 3 catalog title", title),
              msg);
    }
    printf("[catalog] ordinal 3 → name=\"%s\" title=\"%s\"\n", name, title);

    set_pose(&game, 1, 2, DIR_NORTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    (void)expect_int("real north-entry route starts at HALK ordinal",
                     frontOrdinal, 1);

    seededSensor = seed_first_c127_data(&game, 1, PROBE_PORTRAIT_ORDINAL);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "seeded real north-entry C127 sensor data from 1 to %d (sensor idx=%d)",
                 PROBE_PORTRAIT_ORDINAL, seededSensor);
        CHECK(seededSensor >= 0, msg);
    }

    set_pose(&game, 1, 2, DIR_NORTH);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    {
        char msg[160];
        snprintf(msg, sizeof(msg),
                 "seeded north-entry route reports ordinal %d", PROBE_PORTRAIT_ORDINAL);
        (void)expect_int("seeded north-entry route reports ordinal 3",
                         frontOrdinal, PROBE_PORTRAIT_ORDINAL);
        printf("  INFO: %s\n", msg);
    }

    /* D1C wall-mirror frame must match the source-locked (80,29,64,43)
     * and parent the portrait rectangle at +16,+6. */
    CHECK(M11_GameView_GetD1CWallOrnamentZone(&game, &ornX, &ornY, &ornW, &ornH) == 1,
          "D1C wall-mirror frame zone helper succeeds");
    (void)expect_int("D1C wall-mirror frame x", ornX, PROBE_FRAME_X);
    (void)expect_int("D1C wall-mirror frame y", ornY, PROBE_FRAME_Y);
    (void)expect_int("D1C wall-mirror frame width", ornW, PROBE_FRAME_W);
    (void)expect_int("D1C wall-mirror frame height", ornH, PROBE_FRAME_H);
    CHECK(PROBE_PORTRAIT_VX == ornX + 16,
          "D1C portrait rect x is frame x + 16 (96 == 80 + 16)");
    CHECK(PROBE_PORTRAIT_VY == ornY + 6,
          "D1C portrait rect y is frame y + 6 (35 == 29 + 6)");
    CHECK(PROBE_PORTRAIT_VX >= ornX &&
          PROBE_PORTRAIT_VY >= ornY &&
          PROBE_PORTRAIT_VX + PROBE_PORTRAIT_W <= ornX + ornW &&
          PROBE_PORTRAIT_VY + PROBE_PORTRAIT_H <= ornY + ornH,
          "D1C portrait rect is fully contained by C346 wall-mirror frame");

    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    matchPct = portrait_match_percent(portraits, fb, PROBE_PORTRAIT_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "portrait ordinal 3 (C026, name=\"%s\") matches D1C rect >=90%% (got %d%%)",
                 name, matchPct);
        CHECK(matchPct >= 90, msg);
    }

    /* Side poses: the D1C portrait rectangle must not float ordinal-3
     * pixels onto an ordinary side wall when the front route is gone. */
    set_pose(&game, 1, 2, DIR_EAST);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    (void)expect_int("ordinary east side-wall pose has no front route",
                     frontOrdinal, -1);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    matchPct = portrait_match_percent(portraits, fb, PROBE_PORTRAIT_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "east side-wall pose does not show portrait 3 in D1C rect (got %d%%)",
                 matchPct);
        CHECK(matchPct < 70, msg);
    }

    set_pose(&game, 1, 2, DIR_WEST);
    frontOrdinal = M11_GameView_GetFrontMirrorOrdinal(&game);
    (void)expect_int("ordinary west side-wall pose has no front route",
                     frontOrdinal, -1);
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(&game, fb, PROBE_FB_W, PROBE_FB_H);
    matchPct = portrait_match_percent(portraits, fb, PROBE_PORTRAIT_ORDINAL);
    {
        char msg[200];
        snprintf(msg, sizeof(msg),
                 "west side-wall pose does not show portrait 3 in D1C rect (got %d%%)",
                 matchPct);
        CHECK(matchPct < 70, msg);
    }

    /* Candidate-panel route: M11_GameView_SelectFrontMirrorCandidate
     * must keep ordinal 3 across the panel open, mirroring the REVIVE.C
     * F0280 materialization flow. */
    set_pose(&game, 1, 2, DIR_NORTH);
    initialCount = game.world.party.championCount;
    selectRc = M11_GameView_SelectFrontMirrorCandidate(&game);
    (void)expect_int("ordinal 3 candidate selection succeeds", selectRc, 1);
    (void)expect_int("candidate panel keeps ordinal 3",
                     game.candidateMirrorOrdinal, PROBE_PORTRAIT_ORDINAL);
    (void)expect_int("candidate panel is active",
                     game.candidateMirrorPanelActive, 1);
    (void)expect_int("candidate champion appended",
                     game.world.party.championCount, initialCount + 1);

    M11_GameView_Shutdown(&game);
    printf("=== Summary: %d passed, %d failed ===\n", g_pass, g_fail);
    return g_fail == 0 ? 0 : 1;
}
