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

static int make_dir(const char* path) {
    return mkdir(path, 0777) == 0;
}

static int make_file_with_text(const char* path, const char* text) {
    FILE* fp = fopen(path, "wb");
    if (!fp) {
        return 0;
    }
    fputs(text, fp);
    fclose(fp);
    return 1;
}

static int file_exists(const char* path) {
    struct stat st;
    return path && path[0] != '\0' && stat(path, &st) == 0;
}

static void remove_if_present(const char* path) {
    if (path && path[0] != '\0') {
        unlink(path);
    }
}

int main(void) {
    unsigned char framebuffer[320 * 200];
    M12_StartupMenuState state;
    M12_StartupMenuState reloaded;
    ProbeTally tally = {0, 0};
    size_t litPixels = 0;
    size_t i;
    char rootTemplate[] = "/tmp/firestaff-m12-probe-XXXXXX";
    char dataDir[1024];
    char firestaffDir[1024];
    char configPath[1024];
    char graphicsPath[1024];
    char dungeonPath[1024];
    char csbGraphicsPath[1024];
    char csbDungeonPath[1024];
    char dm2GraphicsPath[1024];
    char dm2DungeonPath[1024];
    char bogusGraphicsPath[1024];
    char cardsDir[1024];
    char cardPath[1024];
    char* rootDir = mkdtemp(rootTemplate);

    if (!rootDir) {
        perror("mkdtemp");
        return 2;
    }

    if (setenv("HOME", rootDir, 1) != 0) {
        perror("setenv");
        return 2;
    }

    snprintf(firestaffDir, sizeof(firestaffDir), "%s/.firestaff", rootDir);
    snprintf(dataDir, sizeof(dataDir), "%s/data", firestaffDir);
    snprintf(configPath, sizeof(configPath), "%s/startup-menu.toml", firestaffDir);
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", dataDir);
    snprintf(csbGraphicsPath, sizeof(csbGraphicsPath), "%s/CSBGRAPH.DAT", dataDir);
    snprintf(csbDungeonPath, sizeof(csbDungeonPath), "%s/CSB.DAT", dataDir);
    snprintf(dm2GraphicsPath, sizeof(dm2GraphicsPath), "%s/DM2GRAPHICS.DAT", dataDir);
    snprintf(dm2DungeonPath, sizeof(dm2DungeonPath), "%s/DM2DUNGEON.DAT", dataDir);
    snprintf(bogusGraphicsPath, sizeof(bogusGraphicsPath), "%s/GRAPHICS_BAD.DAT", dataDir);
    snprintf(cardsDir, sizeof(cardsDir), "%s/cards", dataDir);
    snprintf(cardPath, sizeof(cardPath), "%s/dm1.png", cardsDir);

    if (!make_dir(firestaffDir) || !make_dir(dataDir) || !make_dir(cardsDir)) {
        perror("mkdir");
        return 2;
    }

    make_file_with_text(graphicsPath, "ok");
    make_file_with_text(dungeonPath, "ok");

    M12_StartupMenu_InitWithDataDir(&state, dataDir);

    probe_record(&tally,
                 "INV_M12_01",
                 M12_StartupMenu_GetEntryCount() == 4,
                 "startup menu exposes three games plus settings");
    probe_record(&tally,
                 "INV_M12_02",
                 file_exists(configPath) &&
                     state.selectedIndex == 0 &&
                     state.view == M12_MENU_VIEW_MAIN &&
                     state.shouldExit == 0 &&
                     M12_CardArt_HasImage(&state.cardArt[0]) == 1 &&
                     M12_CardArt_HasExternalFile(&state.cardArt[0]) == 0 &&
                     state.entries[0].available == 1 &&
                     state.entries[1].available == 0 &&
                     state.entries[2].available == 0,
                 "startup state loads defaults, writes config, detects only MD5-known DM1 assets, and seeds built-in card art");

    make_file_with_text(cardPath, "future art slot");
    M12_StartupMenu_InitWithDataDir(&state, dataDir);
    probe_record(&tally,
                 "INV_M12_02B",
                 M12_CardArt_HasImage(&state.cardArt[0]) == 1 &&
                     M12_CardArt_HasExternalFile(&state.cardArt[0]) == 1 &&
                     strcmp(M12_CardArt_GetFileName(&state.cardArt[0]), "dm1.png") == 0,
                 "card-art loader promotes external slot files when present");

    make_file_with_text(csbGraphicsPath, "ok");
    make_file_with_text(csbDungeonPath, "ok");
    make_file_with_text(dm2GraphicsPath, "ok");
    make_file_with_text(dm2DungeonPath, "ok");
    make_file_with_text(bogusGraphicsPath, "not-known");
    M12_AssetStatus_Scan(&state.assetStatus, dataDir);
    probe_record(&tally,
                 "INV_M12_03",
                 state.assetStatus.dm1Available == 1 &&
                     state.assetStatus.csbAvailable == 0 &&
                     state.assetStatus.dm2Available == 0 &&
                     M12_AssetStatus_GameHasCompleteHashSet("dm1") == 1 &&
                     M12_AssetStatus_GameHasCompleteHashSet("csb") == 0 &&
                     M12_AssetStatus_GameHasCompleteHashSet("dm2") == 0 &&
                     M12_AssetStatus_GameKnownHashCount("csb") == 8U &&
                     M12_AssetStatus_GameKnownHashCount("dm2") == 19U,
                 "asset scan uses MD5 matches and exposes expanded graphics checksum coverage");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    probe_record(&tally,
                 "INV_M12_04",
                 state.selectedIndex == 1,
                 "down moves selection to second row");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    probe_record(&tally,
                 "INV_M12_05",
                 state.selectedIndex == 0,
                 "up moves selection back to first row");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_06",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "READY TO LAUNCH") == 0,
                 "enter on available entry opens ready message");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    probe_record(&tally,
                 "INV_M12_07",
                 state.view == M12_MENU_VIEW_MAIN && state.shouldExit == 0,
                 "escape returns from message view to main menu");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_08",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "VALIDATOR SCAFFOLD ONLY") == 0,
                 "enter on scaffold-only entry explains missing verified hashes");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_09",
                 state.view == M12_MENU_VIEW_SETTINGS,
                 "settings row opens settings screen");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_10",
                 state.settings.languageIndex == 1 &&
                     state.settings.graphicsIndex == 1 &&
                     state.settings.windowModeIndex == 1,
                 "settings screen cycles persisted values from keyboard input");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_Init(&reloaded);
    probe_record(&tally,
                 "INV_M12_11",
                 reloaded.settings.languageIndex == 1 &&
                     reloaded.settings.graphicsIndex == 1 &&
                     reloaded.settings.windowModeIndex == 1 &&
                     M12_StartupMenu_GetRenderPaletteLevel(&reloaded) == 1 &&
                     strcmp(M12_AssetStatus_GetDataDir(&reloaded.assetStatus), dataDir) == 0,
                 "settings, runtime palette mapping, and data dir persist across reloads");

    M12_StartupMenu_Draw(&state, framebuffer, 320, 200);
    for (i = 0; i < sizeof(framebuffer); ++i) {
        if (framebuffer[i] != 0U) {
            ++litPixels;
        }
    }
    probe_record(&tally,
                 "INV_M12_12",
                 framebuffer[0] != 0U && litPixels > 2000U,
                 "draw renders structured non-empty menu pixels for the visual pass");

    {
        unsigned char localized[320 * 200];
        unsigned char english[320 * 200];
        unsigned long checksumA = 0UL;
        unsigned long checksumB = 0UL;
        for (i = 0; i < sizeof(localized); ++i) {
            localized[i] = framebuffer[i];
        }
        reloaded.settings.languageIndex = 0;
        reloaded.settings.graphicsIndex = 0;
        M12_StartupMenu_Draw(&reloaded, english, 320, 200);
        for (i = 0; i < sizeof(localized); ++i) {
            checksumA = (checksumA * 131UL) + localized[i];
            checksumB = (checksumB * 131UL) + english[i];
        }
        probe_record(&tally,
                     "INV_M12_12B",
                     checksumA != checksumB,
                     "language and graphics selections change the rendered launcher output");
    }

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    probe_record(&tally,
                 "INV_M12_13",
                 state.shouldExit == 1,
                 "escape on top-level requests exit");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);

    remove_if_present(bogusGraphicsPath);
    remove_if_present(dm2DungeonPath);
    remove_if_present(dm2GraphicsPath);
    remove_if_present(csbDungeonPath);
    remove_if_present(csbGraphicsPath);
    remove_if_present(cardPath);
    remove_if_present(dungeonPath);
    remove_if_present(graphicsPath);
    remove_if_present(configPath);
    rmdir(cardsDir);
    rmdir(dataDir);
    rmdir(firestaffDir);
    rmdir(rootDir);

    return (tally.passed == tally.total) ? 0 : 1;
}
