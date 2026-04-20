#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct {
    int total;
    int passed;
} ProbeTally;

static void probe_record(ProbeTally* tally,
                         const char* id,
                         int ok,
                         const char* message) {
    tally->total += 1;
    if (ok) {
        tally->passed += 1;
        printf("PASS %s %s\n", id, message);
    } else {
        printf("FAIL %s %s\n", id, message);
    }
}

int main(int argc, char** argv) {
    const char* dataDir = NULL;
    M12_StartupMenuState menuState;
    M11_GameViewState gameView;
    ProbeTally tally = {0, 0};
    unsigned char framebuffer[320 * 200];
    uint32_t initialHash = 0;
    int initialDirection = 0;
    uint32_t initialTick = 0;
    size_t litPixels = 0;
    size_t i;

    if (argc > 1) {
        dataDir = argv[1];
    } else {
        dataDir = getenv("FIRESTAFF_DATA");
    }
    if (!dataDir || dataDir[0] == '\0') {
        static char fallback[1024];
        const char* home = getenv("HOME");
        if (!home || home[0] == '\0') {
            fprintf(stderr, "No HOME or FIRESTAFF_DATA available\n");
            return 2;
        }
        snprintf(fallback, sizeof(fallback), "%s/.firestaff/data", home);
        dataDir = fallback;
    }

    M12_StartupMenu_InitWithDataDir(&menuState, dataDir);
    probe_record(&tally,
                 "INV_GV_01",
                 menuState.selectedIndex == 0 &&
                     menuState.entries[0].available == 1 &&
                     menuState.entries[0].gameId &&
                     strcmp(menuState.entries[0].gameId, "dm1") == 0 &&
                     menuState.entries[0].sourceKind == M12_MENU_SOURCE_BUILTIN_CATALOG,
                 "launcher exposes DM1 as the default builtin launch source with verified assets");

    M11_GameView_Init(&gameView);
    probe_record(&tally,
                 "INV_GV_02",
                 M11_GameView_OpenSelectedMenuEntry(&gameView, &menuState) == 1 &&
                     gameView.active == 1 &&
                     strcmp(gameView.title, "DUNGEON MASTER") == 0 &&
                     strcmp(gameView.sourceId, "dm1") == 0,
                 "launcher selection transitions into a real game view through the source hook");

    initialHash = gameView.lastWorldHash;
    initialDirection = gameView.world.party.direction;
    initialTick = gameView.world.gameTick;

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&gameView, framebuffer, 320, 200);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0U) {
            ++litPixels;
        }
    }
    probe_record(&tally,
                 "INV_GV_03",
                 litPixels > 4000U,
                 "game view renders a non-empty dungeon-backed frame");

    probe_record(&tally,
                 "INV_GV_04",
                 M11_GameView_HandleInput(&gameView, M12_MENU_INPUT_RIGHT) == M11_GAME_INPUT_REDRAW &&
                     gameView.world.party.direction != initialDirection &&
                     gameView.world.gameTick == initialTick + 1 &&
                     gameView.lastWorldHash != initialHash,
                 "game view input turns the party through the real tick orchestrator");

    initialTick = gameView.world.gameTick;
    probe_record(&tally,
                 "INV_GV_05",
                 M11_GameView_HandleInput(&gameView, M12_MENU_INPUT_ACCEPT) == M11_GAME_INPUT_REDRAW &&
                     gameView.world.gameTick == initialTick + 1,
                 "accept advances an in-game wait tick distinct from menu navigation");

    probe_record(&tally,
                 "INV_GV_06",
                 M11_GameView_HandleInput(&gameView, M12_MENU_INPUT_BACK) == M11_GAME_INPUT_RETURN_TO_MENU,
                 "escape from the game view returns control to the launcher");

    M11_GameView_Shutdown(&gameView);
    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
