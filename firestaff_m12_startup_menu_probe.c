#include "asset_status_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

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

static int make_file(const char* path) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    fputs("ok", fp);
    fclose(fp);
    return 1;
}

static void remove_if_present(const char* path) {
    if (path && path[0] != '\0') {
        unlink(path);
    }
}

int main(void) {
    unsigned char framebuffer[320 * 200];
    M12_StartupMenuState state;
    ProbeTally tally = {0, 0};
    size_t litPixels = 0;
    size_t i;
    char tempTemplate[] = "/tmp/firestaff-m12-probe-XXXXXX";
    char graphicsPath[1024];
    char dungeonPath[1024];
    char csbGraphicsPath[1024];
    char csbDungeonPath[1024];
    char dm2GraphicsPath[1024];
    char dm2DungeonPath[1024];
    char* tempDir = mkdtemp(tempTemplate);

    if (!tempDir) {
        perror("mkdtemp");
        return 2;
    }

    snprintf(graphicsPath, sizeof(graphicsPath), "%s/GRAPHICS.DAT", tempDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", tempDir);
    snprintf(csbGraphicsPath, sizeof(csbGraphicsPath), "%s/CSBGRAPH.DAT", tempDir);
    snprintf(csbDungeonPath, sizeof(csbDungeonPath), "%s/CSB.DAT", tempDir);
    snprintf(dm2GraphicsPath, sizeof(dm2GraphicsPath), "%s/DM2GRAPHICS.DAT", tempDir);
    snprintf(dm2DungeonPath, sizeof(dm2DungeonPath), "%s/DM2DUNGEON.DAT", tempDir);

    make_file(graphicsPath);
    make_file(dungeonPath);

    M12_StartupMenu_InitWithDataDir(&state, tempDir);

    probe_record(&tally,
                 "INV_M12_01",
                 M12_StartupMenu_GetEntryCount() == 4,
                 "startup menu exposes three games plus settings");
    probe_record(&tally,
                 "INV_M12_02",
                 state.selectedIndex == 0 &&
                     state.view == M12_MENU_VIEW_MAIN &&
                     state.shouldExit == 0 &&
                     state.entries[0].available == 1 &&
                     state.entries[1].available == 0 &&
                     state.entries[2].available == 0,
                 "startup state initialises to main menu with detected asset availability");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    probe_record(&tally,
                 "INV_M12_03",
                 state.selectedIndex == 1,
                 "down moves selection to second row");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    probe_record(&tally,
                 "INV_M12_04",
                 state.selectedIndex == 0,
                 "up moves selection back to first row");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_05",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "READY TO LAUNCH") == 0,
                 "enter on available entry opens ready message");

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
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_08",
                 state.view == M12_MENU_VIEW_SETTINGS,
                 "settings row opens settings screen");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_09",
                 state.settings.languageIndex == 1 &&
                     state.settings.graphicsIndex == 1 &&
                     state.settings.windowModeIndex == 1,
                 "settings screen cycles placeholder values from keyboard input");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_Draw(&state, framebuffer, 320, 200);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0U) {
            ++litPixels;
        }
    }
    probe_record(&tally,
                 "INV_M12_10",
                 framebuffer[0] != 0U && litPixels > 0U,
                 "draw renders structured non-empty menu pixels");

    make_file(csbGraphicsPath);
    make_file(csbDungeonPath);
    make_file(dm2GraphicsPath);
    make_file(dm2DungeonPath);
    M12_AssetStatus_Scan(&state.assetStatus, tempDir);
    probe_record(&tally,
                 "INV_M12_11",
                 state.assetStatus.dm1Available == 1 &&
                     state.assetStatus.csbAvailable == 1 &&
                     state.assetStatus.dm2Available == 1,
                 "asset scan promotes CSB and DM2 when expected files exist");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    probe_record(&tally,
                 "INV_M12_12",
                 state.shouldExit == 1,
                 "escape on top-level requests exit");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);

    remove_if_present(dm2DungeonPath);
    remove_if_present(dm2GraphicsPath);
    remove_if_present(csbDungeonPath);
    remove_if_present(csbGraphicsPath);
    remove_if_present(dungeonPath);
    remove_if_present(graphicsPath);
    rmdir(tempDir);

    return (tally.passed == tally.total) ? 0 : 1;
}
