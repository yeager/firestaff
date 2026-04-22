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

static int file_contains(const char* path, const char* needle) {
    FILE* fp;
    char line[1024];
    if (!path || !needle || needle[0] == '\0') {
        return 0;
    }
    fp = fopen(path, "rb");
    if (!fp) {
        return 0;
    }
    while (fgets(line, sizeof(line), fp) != NULL) {
        if (strstr(line, needle) != NULL) {
            fclose(fp);
            return 1;
        }
    }
    fclose(fp);
    return 0;
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
                 state.view == M12_MENU_VIEW_GAME_OPTIONS &&
                     state.activatedIndex == 0,
                 "enter on available entry opens game options");

    probe_record(&tally,
                 "INV_M12_06B",
                 state.gameOptions[0].cheatsEnabled == 0 &&
                     state.gameOptions[0].gameSpeed == 1 &&
                     M12_GameOptions_SpeedHotkeysEnabled(&state.gameOptions[0]) == 0,
                 "game options default to cheats off, speed normal, hotkeys disabled");

    /* Enable cheats and verify speed hotkeys unlock */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* cheats row */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* toggle on */
    probe_record(&tally,
                 "INV_M12_06C",
                 state.gameOptions[0].cheatsEnabled == 1 &&
                     M12_GameOptions_SpeedHotkeysEnabled(&state.gameOptions[0]) == 1,
                 "enabling cheats unlocks speed hotkeys");

    /* Cycle speed to FASTER */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* speed row */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* NORMAL→FASTER */
    probe_record(&tally,
                 "INV_M12_06D",
                 state.gameOptions[0].gameSpeed == 2,
                 "game speed cycles to faster with cheats on");

    /* Disable cheats: speed resets, hotkeys disabled */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP); /* back to cheats row */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* toggle off */
    probe_record(&tally,
                 "INV_M12_06E",
                 state.gameOptions[0].cheatsEnabled == 0 &&
                     state.gameOptions[0].gameSpeed == 1 &&
                     M12_GameOptions_SpeedHotkeysEnabled(&state.gameOptions[0]) == 0,
                 "disabling cheats resets speed to normal and disables hotkeys");

    /* Navigate to launch and press accept */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* speed */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* aspect */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* resolution */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* launch row */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_06F",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "READY TO LAUNCH") == 0,
                 "launch from game options opens ready message");

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

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK); /* to main */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* dm2 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* csb */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* dm1 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* open DM1 options */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* patch: ORIGINAL -> PATCHED */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);   /* cheats */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* OFF -> ON */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);   /* speed */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* NORMAL -> FASTER */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);   /* aspect */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* ORIGINAL -> 4:3 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);   /* resolution */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* 320x200 -> 640x400 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);   /* back to main */

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_Init(&reloaded);
    probe_record(&tally,
                 "INV_M12_11",
                 reloaded.settings.languageIndex == 1 &&
                     reloaded.settings.graphicsIndex == 1 &&
                     reloaded.settings.windowModeIndex == 1 &&
                     reloaded.gameOptions[0].usePatch == 1 &&
                     reloaded.gameOptions[0].cheatsEnabled == 1 &&
                     reloaded.gameOptions[0].gameSpeed == 2 &&
                     reloaded.gameOptions[0].aspectRatio == 1 &&
                     reloaded.gameOptions[0].resolution == 1 &&
                     reloaded.gameOptions[1].usePatch == 0 &&
                     reloaded.gameOptions[1].cheatsEnabled == 0 &&
                     reloaded.gameOptions[1].gameSpeed == 1 &&
                     reloaded.gameOptions[2].usePatch == 0 &&
                     reloaded.gameOptions[2].cheatsEnabled == 0 &&
                     reloaded.gameOptions[2].gameSpeed == 1 &&
                     M12_StartupMenu_GetRenderPaletteLevel(&reloaded) == 1 &&
                     file_contains(configPath, "presentation_mode_index = 1") &&
                     strcmp(M12_AssetStatus_GetDataDir(&reloaded.assetStatus), dataDir) == 0,
                 "settings and per-game options persist across reloads without cross-game bleed, including presentation mode");

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
                     "language and presentation mode selections change the rendered launcher output");
    }

    /* === Mode-constraint and launch-intent tests === */
    {
        M12_StartupMenuState modeState;
        M12_LaunchIntent intent;

        /* V1 mode: aspect and resolution locked to original */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        /* Select DM1 (available) and open game options */
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        probe_record(&tally,
                     "INV_M12_14",
                     modeState.view == M12_MENU_VIEW_GAME_OPTIONS &&
                         modeState.gameOptions[0].aspectRatio == M12_ASPECT_ORIGINAL &&
                         modeState.gameOptions[0].resolution == M12_RES_320x200,
                     "V1 mode enforces original aspect and 320x200 resolution on entry");

        /* Try cycling aspect in V1 — should stay locked */
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_ASPECT;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_RIGHT);
        probe_record(&tally,
                     "INV_M12_15",
                     modeState.gameOptions[0].aspectRatio == M12_ASPECT_ORIGINAL,
                     "V1 mode blocks aspect ratio cycling");

        /* Try cycling resolution in V1 — should stay locked */
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_RESOLUTION;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_RIGHT);
        probe_record(&tally,
                     "INV_M12_16",
                     modeState.gameOptions[0].resolution == M12_RES_320x200,
                     "V1 mode blocks resolution cycling");

        /* Row-locked query */
        probe_record(&tally,
                     "INV_M12_17",
                     M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_ASPECT, M12_PRESENTATION_V1_ORIGINAL) == 1 &&
                         M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_RESOLUTION, M12_PRESENTATION_V1_ORIGINAL) == 1 &&
                         M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_PATCH, M12_PRESENTATION_V1_ORIGINAL) == 0 &&
                         M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_ASPECT, M12_PRESENTATION_V2_ENHANCED_2D) == 0 &&
                         M12_GameOptions_RowLockedByMode(M12_GAME_OPT_ROW_RESOLUTION, M12_PRESENTATION_V2_ENHANCED_2D) == 0,
                     "RowLockedByMode reports correct constraints per mode");

        /* V2 mode: aspect and resolution cycle freely */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V2_ENHANCED_2D;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_ASPECT;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_RIGHT);
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_RESOLUTION;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_RIGHT);
        probe_record(&tally,
                     "INV_M12_18",
                     modeState.gameOptions[0].aspectRatio == M12_ASPECT_4_3 &&
                         modeState.gameOptions[0].resolution == M12_RES_640x400,
                     "V2 mode allows free aspect and resolution cycling");

        /* Launch intent for V1 */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_19",
                     intent.valid == 1 &&
                         intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL &&
                         strcmp(intent.gameId, "dm1") == 0 &&
                         intent.options.aspectRatio == M12_ASPECT_ORIGINAL &&
                         intent.options.resolution == M12_RES_320x200,
                     "V1 launch intent carries correct mode and constrained options");

        /* Launch intent for V2 with explicit resolution set */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V2_ENHANCED_2D;
        /* Reset DM1 resolution to 0 so we can verify cycling works */
        modeState.gameOptions[0].resolution = M12_RES_320x200;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_RESOLUTION;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_RIGHT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_20",
                     intent.valid == 1 &&
                         intent.presentationMode == M12_PRESENTATION_V2_ENHANCED_2D &&
                         strcmp(intent.gameId, "dm1") == 0 &&
                         intent.options.resolution == M12_RES_640x400,
                     "V2 launch intent carries enhanced resolution options");

        /* V3 launch intent is invalid (coming soon) */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V3_MODERN_3D;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_21",
                     intent.valid == 0,
                     "V3 launch intent is invalid (coming soon)");

        /* V3 launch button shows coming-soon message */
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT; /* launch row */
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        probe_record(&tally,
                     "INV_M12_22",
                     modeState.view == M12_MENU_VIEW_MESSAGE &&
                         strcmp(modeState.messageLine2, "COMING SOON") == 0,
                     "V3 launch attempt shows COMING SOON message");

        /* Presentation mode API */
        modeState.settings.graphicsIndex = M12_PRESENTATION_V2_ENHANCED_2D;
        probe_record(&tally,
                     "INV_M12_23",
                     M12_StartupMenu_GetPresentationMode(&modeState) == M12_PRESENTATION_V2_ENHANCED_2D &&
                         strcmp(M12_StartupMenu_GetPresentationModeLabel(&modeState), "V2 ENHANCED 2D") == 0,
                     "presentation mode API returns correct mode and label");
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
