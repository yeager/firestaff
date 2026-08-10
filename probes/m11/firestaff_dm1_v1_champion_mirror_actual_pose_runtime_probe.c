/*
 * DM1 V1 champion mirror actual-pose runtime probe.
 *
 * 2026-06-14 mail regression: Daniel reports a champion portrait appears
 * in the middle of the viewport at the Hall of Champions start.  The
 * root cause was that m11_front_cell_mirror_ordinal used the front-cell
 * TextString as the mirror identity.  In real DM1 V1 data the C127
 * champion-portrait sensor (DUNGEON.C:2573 / MOVESENS.C:1501-1503 /
 * REVIVE.C F0280) carries the mirror ordinal in its sensorData.  The
 * mirror route must only be selectable when a C127 sensor is present
 * on the front square, AND the route's ordinal must equal the C127
 * sensorData, not a TextString-derived catalog ordinal.
 *
 * This probe locks the actual mirror positions in real DM1 V1
 * DUNGEON.DAT and verifies the correct ordinals are returned and
 * that corridor poses (where no C127 sensor exists) return -1.
 *
 * Source evidence:
 *   ReDMCSB DUNGEON.C:2573 maps M011_CELL(sensor) against view dir
 *   ReDMCSB DUNGEON.C:2608-2612 stores C127 sensorData in G0289
 *   ReDMCSB DUNVIEW.C:3913-3928 blits D1C champion portrait
 *   ReDMCSB MOVESENS.C:1501-1503 passes C127 sensorData to F0280
 *   ReDMCSB REVIVE.C F0280 materializes the candidate from sensorData
 */
#include "m11_game_view.h"
#include "firestaff_dm1_probe_data_dir.h"
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
    /* DUNVIEW.C:3913-3928 blits C026 into the D1C front-wall box
     * (96,35)-(127,63), viewport-relative, with color 1 transparent. */
    PORTRAIT_X = VIEWPORT_X + 96,
    PORTRAIT_Y = VIEWPORT_Y + 35,
    PORTRAIT_W = 32,
    PORTRAIT_H = 29,
    PORTRAIT_TRANSPARENT = 1
};

typedef struct MirrorPose {
    int mapX;
    int mapY;
    int direction;
    int expectedOrdinal; /* -1 means no mirror */
    const char* label;
} MirrorPose;

typedef struct PortraitRectMatch {
    int bestOrdinal;
    int bestMatched;
    int expectedMatched;
    int compared;
} PortraitRectMatch;

static int g_pass = 0;
static int g_fail = 0;

#define PASS() do { printf("PASS\n"); g_pass++; } while(0)
#define FAIL(msg) do { printf("FAIL: %s\n", msg); g_fail++; } while(0)

static int check_pose(M11_GameViewState* game, const MirrorPose* pose) {
    int actual;
    printf("  TEST: %s pose=(%d,%d,%d) expected=%d ... ",
           pose->label, pose->mapX, pose->mapY, pose->direction,
           pose->expectedOrdinal);
    game->world.party.mapIndex = 0;
    game->world.party.mapX = pose->mapX;
    game->world.party.mapY = pose->mapY;
    game->world.party.direction = pose->direction;
    actual = M11_GameView_GetFrontMirrorOrdinal(game);
    if (actual == pose->expectedOrdinal) {
        PASS();
        return 1;
    }
    printf("FAIL got=%d want=%d\n", actual, pose->expectedOrdinal);
    g_fail++;
    return 0;
}

static void set_pose(M11_GameViewState* game, int mapX, int mapY, int direction) {
    game->world.party.mapIndex = 0;
    game->world.party.mapX = mapX;
    game->world.party.mapY = mapY;
    game->world.party.direction = direction;
    game->showDebugHUD = 0;
    game->candidateMirrorPanelActive = 0;
    game->candidateMirrorOrdinal = -1;
    game->candidateMirrorPartyIndex = -1;
}

static PortraitRectMatch match_portrait_rect(const M11_AssetSlot* portraits,
                                             const unsigned char* fb,
                                             int expectedOrdinal) {
    PortraitRectMatch out;
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
        for (y = 0; y < PORTRAIT_H; ++y) {
            for (x = 0; x < PORTRAIT_W; ++x) {
                int srcX = (ordinal & 7) * PORTRAIT_W + x;
                int srcY = (ordinal >> 3) * PORTRAIT_H + y;
                unsigned char src =
                    (unsigned char)(portraits->pixels[srcY * (int)portraits->width + srcX] & 0x0F);
                unsigned char dst =
                    M11_FB_DECODE_INDEX(fb[(PORTRAIT_Y + y) * FB_W + (PORTRAIT_X + x)]);
                if (src == PORTRAIT_TRANSPARENT) {
                    continue;
                }
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

static int check_portrait_rect(M11_GameViewState* game,
                               const M11_AssetSlot* portraits,
                               const MirrorPose* pose) {
    unsigned char fb[FB_W * FB_H];
    PortraitRectMatch match;
    int actual;
    printf("  TEST: %s portrait rect pose=(%d,%d,%d) expected=%d ... ",
           pose->label, pose->mapX, pose->mapY, pose->direction,
           pose->expectedOrdinal);
    set_pose(game, pose->mapX, pose->mapY, pose->direction);
    actual = M11_GameView_GetFrontMirrorOrdinal(game);
    if (actual != pose->expectedOrdinal) {
        printf("FAIL ordinal got=%d want=%d\n", actual, pose->expectedOrdinal);
        g_fail++;
        return 0;
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    match = match_portrait_rect(portraits, fb, pose->expectedOrdinal);
    if (match.bestOrdinal != pose->expectedOrdinal ||
        match.compared <= 0 ||
        match.expectedMatched * 100 < match.compared * 90) {
        printf("FAIL best=%d matched=%d/%d bestMatched=%d\n",
               match.bestOrdinal, match.expectedMatched,
               match.compared, match.bestMatched);
        g_fail++;
        return 0;
    }
    printf("PASS best=%d matched=%d/%d\n",
           match.bestOrdinal, match.expectedMatched, match.compared);
    g_pass++;
    return 1;
}

static int check_no_stale_ordinal_in_rect(M11_GameViewState* game,
                                          const M11_AssetSlot* portraits,
                                          const MirrorPose* pose,
                                          int staleOrdinal) {
    unsigned char fb[FB_W * FB_H];
    PortraitRectMatch match;
    int actual;
    printf("  TEST: %s no ordinal %d in portrait rect ... ",
           pose->label, staleOrdinal);
    set_pose(game, pose->mapX, pose->mapY, pose->direction);
    actual = M11_GameView_GetFrontMirrorOrdinal(game);
    if (actual != pose->expectedOrdinal) {
        printf("FAIL ordinal got=%d want=%d\n", actual, pose->expectedOrdinal);
        g_fail++;
        return 0;
    }
    memset(fb, 0, sizeof(fb));
    M11_GameView_Draw(game, fb, FB_W, FB_H);
    match = match_portrait_rect(portraits, fb, staleOrdinal);
    if (match.compared > 0 &&
        match.expectedMatched * 100 >= match.compared * 35) {
        printf("FAIL staleMatched=%d/%d best=%d bestMatched=%d\n",
               match.expectedMatched, match.compared,
               match.bestOrdinal, match.bestMatched);
        g_fail++;
        return 0;
    }
    printf("PASS staleMatched=%d/%d best=%d\n",
           match.expectedMatched, match.compared, match.bestOrdinal);
    g_pass++;
    return 1;
}

static int check_resurrect_round_trip(M11_GameViewState* game,
                                      const MirrorPose* pose) {
    int initialCount, rc;
    struct ChampionState_Compat* newChamp;
    int i;
    if (pose->expectedOrdinal < 0) {
        printf("  SKIP: %s round-trip (no mirror)\n", pose->label);
        return 1;
    }
    printf("  TEST: %s resurrect round-trip ... ", pose->label);
    game->world.party.mapIndex = 0;
    game->world.party.mapX = pose->mapX;
    game->world.party.mapY = pose->mapY;
    game->world.party.direction = pose->direction;
    initialCount = game->world.party.championCount;
    rc = M11_GameView_SelectFrontMirrorCandidate(game);
    if (rc != 1) {
        printf("FAIL SelectFrontMirrorCandidate=%d\n", rc);
        g_fail++;
        return 0;
    }
    if (game->world.party.championCount != initialCount + 1) {
        printf("FAIL championCount=%d want=%d\n",
               game->world.party.championCount, initialCount + 1);
        g_fail++;
        return 0;
    }
    rc = M11_GameView_ConfirmMirrorCandidate(game, 0);
    if (rc != 1) {
        printf("FAIL ConfirmMirrorCandidate=%d\n", rc);
        g_fail++;
        return 0;
    }
    newChamp = &game->world.party.champions[initialCount];
    if (newChamp->hp.current == 0 || newChamp->hp.maximum == 0) {
        printf("FAIL new champion has zero HP (%d/%d)\n",
               newChamp->hp.current, newChamp->hp.maximum);
        g_fail++;
        return 0;
    }
    /* Advance 20 idle ticks; the new champion must survive */
    for (i = 0; i < 20; ++i) {
        (void)M11_GameView_AdvanceIdleTick(game);
        if (newChamp->hp.current == 0) {
            printf("FAIL new champion died at tick %d\n", i);
            g_fail++;
            return 0;
        }
    }
    if (game->partyDead) {
        printf("FAIL partyDead=1 after resurrection\n");
        g_fail++;
        return 0;
    }
    if (M11_GameView_GetFrontMirrorOrdinal(game) != -1) {
        printf("FAIL mirror route not disabled after confirm\n");
        g_fail++;
        return 0;
    }
    printf("PASS HP=%d/%d\n", newChamp->hp.current, newChamp->hp.maximum);
    g_pass++;
    return 1;
}

int main(int argc, char** argv) {
    const char* dataDir;
    char narrowed[1024];
    M12_StartupMenuState menu;
    M11_GameViewState game;
    const M11_AssetSlot* portraits;
    int ok = 1;

    /* Actual DM1 V1 PC 3.4 DUNGEON.DAT C127 mirror layout, verified by
     * independent dmweb-spec decode (DONE.md 2026-07-18 first-slice
     * entry): ordinal at (x,y) face — 1=(7,8)S, 4=(10,6)N, 10=(7,14)N,
     * 13=(7,17)N, 15=(11,11)N, 18=(10,13)W.  Front-mirror pose rule:
     * stand on the adjacent floor square on the mirror's own face
     * side, facing the wall; the three other sides of the mirror
     * square are wrong-wall poses and must return -1
     * (DUNGEON.C:2573 front-wall side filter).  The previous fixture
     * claimed a fake hall around (1,1)-(2,6) with TextString-derived
     * ordinals. */
    static const MirrorPose kPoses[] = {
        /* ordinal 1 (HALK): mirror (7,8) south face -> pose (7,9) NORTH */
        {7, 9, 0, 1,  "hall_halk_from_south_ordinal_1"},
        {7, 7, 2, -1, "hall_halk_wrong_wall_from_north"},
        {6, 8, 1, -1, "hall_halk_wrong_wall_from_west"},
        {8, 8, 3, -1, "hall_halk_wrong_wall_from_east"},
        /* ordinal 4 (LEIF): mirror (10,6) north face -> pose (10,5) SOUTH */
        {10, 5, 2, 4,  "hall_leif_from_north_ordinal_4"},
        {9, 6, 1, -1,  "hall_leif_wrong_wall_from_west"},
        {11, 6, 3, -1, "hall_leif_wrong_wall_from_east"},
        {10, 7, 0, -1, "hall_leif_wrong_wall_from_south"},
        /* ordinal 18 (SONJA): mirror (10,13) west face; viewing the
         * mirror square from the north is a wrong-wall pose. */
        {10, 12, 2, -1, "hall_sonja_wrong_wall_from_north"},
        /* ordinal 10 (THURFOOT): mirror (7,14) north face ->
         * pose (7,13) SOUTH (the earlier "ZED" label was
         * TextString-derived; the portrait10 probe corrected it). */
        {7, 13, 2, 10, "hall_thurfoot_from_north_ordinal_10"},
        {7, 15, 0, -1, "hall_thurfoot_wrong_wall_from_south"},
        {6, 14, 1, -1, "hall_thurfoot_wrong_wall_from_west"},
        {9, 13, 1, 18, "hall_sonja_from_west_ordinal_18"},
        {11, 13, 3, -1, "hall_sonja_wrong_wall_from_east"},
        /* ordinal 15 (MOPHUS): mirror (11,11) north face ->
         * pose (11,10) SOUTH */
        {11, 10, 2, 15, "hall_mophus_from_north_ordinal_15"},
        {11, 12, 0, -1, "hall_mophus_wrong_wall_from_south"},
        /* ordinal 13 (WUUF): mirror (7,17) north face ->
         * pose (7,16) SOUTH */
        {7, 16, 2, 13, "hall_wuuf_from_north_ordinal_13"},
        {7, 18, 0, -1, "hall_wuuf_wrong_wall_from_south"},
    };
    int i;

    if (argc < 2) {
        fprintf(stderr, "usage: %s DATA_DIR\n", argv[0]);
        return 2;
    }
    dataDir = argv[1];
    dataDir = firestaff_dm1_probe_narrow_data_dir(dataDir, narrowed, sizeof(narrowed));

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
        fprintf(stderr, "FAIL GRAPHICS.DAT champion portrait strip unavailable\n");
        M11_GameView_Shutdown(&game);
        return 1;
    }

    printf("=== DM1 V1 champion mirror actual-pose runtime probe ===\n");
    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        if (!check_pose(&game, &kPoses[i])) {
            ok = 0;
        }
    }
    /* Assigned ordinal-2/front_north_entry slice: the historical
     * (1,4,NORTH)=2 fixture was TextString-derived.  Source-visible PC34
     * C127 metadata leaves every wrong-wall pose with no mirror, so this
     * locks the D1C portrait box against a stale ordinal-2 sprite at the
     * (10,12) SOUTH wrong-wall pose.  The source-valid THURFOOT route is
     * (7,13,SOUTH)=10 under the DUNGEON.C visible-wall-side filter, and
     * proves the same DUNVIEW.C C026 portrait rectangle placement
     * positively. */
    if (!check_no_stale_ordinal_in_rect(&game, portraits, &kPoses[8], 2)) {
        ok = 0;
    }
    if (!check_portrait_rect(&game, portraits, &kPoses[9])) {
        ok = 0;
    }
    /* Round-trip: resurrect at (7,9) NORTH (HALK).  The champion
     * must be appended, the mirror must disable, and 20 idle ticks
     * later the new champion must still be alive. */
    if (!check_resurrect_round_trip(&game, &kPoses[0])) {
        ok = 0;
    }

    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
