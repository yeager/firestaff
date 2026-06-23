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
#include "menu_startup_m12.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

enum { FB_W = 320, FB_H = 200 };

typedef struct MirrorPose {
    int mapX;
    int mapY;
    int direction;
    int expectedOrdinal; /* -1 means no mirror */
    const char* label;
} MirrorPose;

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
    M12_StartupMenuState menu;
    M11_GameViewState game;
    int ok = 1;

    /* Actual DM1 V1 DUNGEON.DAT mirror positions at (1,y) facing
     * NORTH/EAST/SOUTH/WEST.  Values were derived by walking the
     * C127 sensor data on the front square of each pose. */
    static const MirrorPose kPoses[] = {
        /* (1,2) facing N: front=(1,1) has C127 sensor idx=15 data=1 (HALK) */
        {1, 2, 0, 1,  "hall_start_north_ordinal_1"},
        /* Candidate poses around the (2,2) C127 sensor formerly assumed
         * to be visible from (1,2) EAST.  The source front-cell filter
         * decides which side, if any, owns LEIF. */
        {1, 2, 1, -1, "hall_start_east_wrong_wall_no_portrait"},
        {2, 1, 2, 4,  "hall_leif_from_north_ordinal_4"},
        {3, 2, 3, -1, "hall_leif_probe_from_east"},
        {2, 3, 0, -1, "hall_leif_probe_from_south"},
        /* (1,2) facing W: front=(0,2) has door, no mirror */
        {1, 2, 3, -1, "hall_start_west_no_portrait"},
        /* (1,3) facing N: front=(1,2) has only TextString, no C127 */
        {1, 3, 0, -1, "hall_corridor_north_no_portrait"},
        /* (1,3) facing E: front=(2,3) has C127 sensor idx=23 data=18 (SONJA) */
        {1, 3, 1, 18, "hall_corridor_east_ordinal_18"},
        /* (1,4) facing N: front=(1,3) has only TextString, no C127 */
        {1, 4, 0, -1, "hall_corridor_north_no_portrait_2"},
        /* (1,3) facing S: front=(1,4) has C127 sensor idx=16 data=10 (ZED)
         * on cell 0, which matches visibleWallCell=(S+2)&3. */
        {1, 3, 2, 10, "hall_zed_from_north_ordinal_10"},
        /* (1,5) facing N sees the same front square from the wrong side;
         * DUNGEON.C:2573/2610-2612 does not set G0289 for that view. */
        {1, 5, 0, -1, "hall_end_north_wrong_wall_no_portrait"},
        /* Same for the (2,5) MOPHUS sensor: (1,5) EAST is a wrong-wall
         * pose under the ReDMCSB front-wall side filter. */
        {1, 5, 1, -1, "hall_end_east_wrong_wall_no_portrait"},
        {2, 4, 2, 15, "hall_mophus_from_north_ordinal_15"},
        {3, 5, 3, -1, "hall_mophus_probe_from_east"},
        {2, 6, 0, -1, "hall_mophus_probe_from_south"},
        /* (1,5) facing S: front=(1,6) has C127 sensor idx=17 data=13 (WUUF) */
        {1, 5, 2, 13, "hall_end_south_ordinal_13"},
    };
    int i;

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

    printf("=== DM1 V1 champion mirror actual-pose runtime probe ===\n");
    for (i = 0; i < (int)(sizeof(kPoses) / sizeof(kPoses[0])); ++i) {
        if (!check_pose(&game, &kPoses[i])) {
            ok = 0;
        }
    }
    /* Round-trip: resurrect at (1,2) NORTH (HALK).  The champion
     * must be appended, the mirror must disable, and 20 idle ticks
     * later the new champion must still be alive. */
    if (!check_resurrect_round_trip(&game, &kPoses[0])) {
        ok = 0;
    }

    printf("=== %d passed, %d failed ===\n", g_pass, g_fail);
    M11_GameView_Shutdown(&game);
    return ok ? 0 : 1;
}
