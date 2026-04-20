#include "menu_startup_m12.h"

#include <stdio.h>
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

int main(void) {
    unsigned char framebuffer[320 * 200];
    M12_StartupMenuState state;
    ProbeTally tally = {0, 0};
    size_t litPixels = 0;
    size_t i;

    M12_StartupMenu_Init(&state);

    probe_record(&tally,
                 "INV_M12_01",
                 M12_StartupMenu_GetEntryCount() == 3,
                 "startup menu exposes three game entries");
    probe_record(&tally,
                 "INV_M12_02",
                 state.selectedIndex == 0 &&
                     state.view == M12_MENU_VIEW_MAIN &&
                     state.shouldExit == 0,
                 "startup state initialises to main menu at first row");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    probe_record(&tally,
                 "INV_M12_03",
                 state.selectedIndex == 1,
                 "down moves selection to second entry");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    probe_record(&tally,
                 "INV_M12_04",
                 state.selectedIndex == 0,
                 "up moves selection back to first entry");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_05",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "STARTING DUNGEON MASTER...") == 0,
                 "enter on available entry opens starting message");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    probe_record(&tally,
                 "INV_M12_06",
                 state.view == M12_MENU_VIEW_MAIN && state.shouldExit == 0,
                 "escape returns from message view to main menu");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_07",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "GAME DATA NOT FOUND") == 0,
                 "enter on unavailable entry opens missing-data message");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_Draw(&state, framebuffer, 320, 200);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0U) {
            ++litPixels;
        }
    }
    probe_record(&tally,
                 "INV_M12_08",
                 framebuffer[0] == 0U && litPixels > 0U,
                 "draw renders visible menu pixels while keeping background black");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    probe_record(&tally,
                 "INV_M12_09",
                 state.shouldExit == 1,
                 "escape on top-level requests exit");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    return (tally.passed == tally.total) ? 0 : 1;
}
