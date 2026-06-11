#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;


static int m12_test_setenv(const char* name, const char* value) {
#ifdef _WIN32
    return _putenv_s(name, value);
#else
    return setenv(name, value, 1);
#endif
}

static int expect(int cond, const char* msg) {
    if (!cond) {
        fprintf(stderr, "FAIL: %s\n", msg);
        return 0;
    }
    return 1;
}

static int explored_cell_is_set(const M11_GameViewState *view, unsigned int cell) {
    if (!view || cell >= 1024U) {
        return 0;
    }
    return (view->exploredBits[cell / 32U] & (1U << (cell % 32U))) != 0;
}

int main(void) {
    const char* dataDir = getenv("FIRESTAFF_DM1_CANONICAL_DIR");
    char saveTemplate[] = "/tmp/firestaff-m11-resume-XXXXXX";
    char savePath[512];
    M11_GameViewState view;
    M11_GameViewState resumed;
    M11_GameLaunchSpec spec;
    int mapIndex, mapX, mapY, direction;
    const unsigned short leaderHandThing = (unsigned short)((THING_TYPE_WEAPON << 10) | 1U);
    const unsigned short openChestThing = (unsigned short)((THING_TYPE_CONTAINER << 10) | 2U);
    unsigned int revealedCell = (17U * 32U) + 9U;
    unsigned int currentCell;

    if (!dataDir || dataDir[0] == '\0') {
        dataDir = "/Users/bosse/.openclaw/data/firestaff-original-games/DM/_canonical/dm1";
    }
    if (access(dataDir, R_OK) != 0) {
        printf("skip: DM1 canonical dir not available: %s\n", dataDir);
        return 0;
    }
    if (!mkdtemp(saveTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", saveTemplate);
    m12_test_setenv("FIRESTAFF_QUICKSAVE_PATH", savePath);
    m12_test_setenv("HOME", saveTemplate);

    memset(&spec, 0, sizeof(spec));
    spec.title = "DUNGEON MASTER";
    spec.gameId = "dm1";
    spec.dataDir = dataDir;
    spec.sourceId = "dm1";
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&view);
    if (!expect(M11_GameView_Start(&view, &spec), "initial DM1 start should succeed")) return 1;

    view.world.party.mapIndex = 2;
    view.world.party.mapX = 11;
    view.world.party.mapY = 7;
    view.world.party.direction = 3;
    view.world.gameTick = 4242;
    currentCell = (unsigned int)(view.world.party.mapX * 32 + view.world.party.mapY);
    view.exploredBits[currentCell / 32U] |= (1U << (currentCell % 32U));
    view.exploredBits[revealedCell / 32U] |= (1U << (revealedCell % 32U));
    view.leaderHandObjectPresent = 1;
    view.leaderHandThing = leaderHandThing;
    view.leaderHandIconIndex = -1;
    view.v1OpenChestThing = openChestThing;
    view.v1OpenChestOpenedByEye = 1;
    if (!expect(F0891_ORCH_WorldHash_Compat(&view.world, &view.lastWorldHash),
                "world hash refresh should succeed before save")) return 1;
    if (!expect(M11_GameView_QuickSave(&view), "quick save should write valid state")) return 1;
    mapIndex = view.world.party.mapIndex;
    mapX = view.world.party.mapX;
    mapY = view.world.party.mapY;
    direction = view.world.party.direction;
    M11_GameView_Shutdown(&view);

    spec.savePath = savePath;
    M11_GameView_Init(&resumed);
    if (!expect(M11_GameView_Start(&resumed, &spec), "quick-resume DM1 start should load save")) return 1;
    if (!expect(resumed.world.party.mapIndex == mapIndex, "resumed mapIndex should match saved state")) return 1;
    if (!expect(resumed.world.party.mapX == mapX, "resumed mapX should match saved state")) return 1;
    if (!expect(resumed.world.party.mapY == mapY, "resumed mapY should match saved state")) return 1;
    if (!expect(resumed.world.party.direction == direction, "resumed direction should match saved state")) return 1;
    if (!expect(resumed.world.gameTick == 4242, "resumed gameTick should match saved state")) return 1;
    if (!expect(explored_cell_is_set(&resumed, revealedCell),
                "resumed explored tile should match saved reveal state")) return 1;
    if (!expect(explored_cell_is_set(&resumed, currentCell),
                "resumed current tile should stay marked explored")) return 1;
    if (!expect(M11_GameView_GetV1LeaderHandThing(&resumed) == leaderHandThing,
                "resumed leader hand should match saved V1 hand state")) return 1;
    if (!expect(M11_GameView_GetV1OpenChestThing(&resumed) == openChestThing,
                "resumed open chest should match saved V1 chest state")) return 1;
    if (!expect(resumed.v1OpenChestOpenedByEye == 1,
                "resumed open chest should preserve pressing-eye route state")) return 1;
    M11_GameView_Shutdown(&resumed);

    puts("ok: valid saved DM1 V1 state resumes with explored tiles and V1 hand/chest state intact");
    return 0;
}
