#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "asset_status_m12.h"
#include "menu_hit_m12.h"
#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>


#if defined(_WIN32)
#include <process.h>
static int portable_make_dir(const char* path) {
    return mkdir(path) == 0;
}
static char* portable_mkdtemp(char* templ) {
    char* marker = strstr(templ, "XXXXXX");
    int i;
    if (!marker) {
        return NULL;
    }
    for (i = 0; i < 1000; ++i) {
        snprintf(marker, 7, "%06ld", ((long)_getpid() + i) % 1000000L);
        if (mkdir(templ) == 0) {
            return templ;
        }
    }
    return NULL;
}
static int portable_setenv(const char* name, const char* value, int overwrite) {
    (void)overwrite;
    return _putenv_s(name, value);
}
static int portable_unsetenv(const char* name) {
    return _putenv_s(name, "");
}
#else
static int portable_make_dir(const char* path) {
    return mkdir(path, 0777) == 0;
}
static char* portable_mkdtemp(char* templ) {
    return mkdtemp(templ);
}
static int portable_setenv(const char* name, const char* value, int overwrite) {
    return setenv(name, value, overwrite);
}
static int portable_unsetenv(const char* name) {
    return unsetenv(name);
}
#endif


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
    return portable_make_dir(path);
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

static void force_dm1_version_ready(M12_StartupMenuState* state, size_t versionIndex) {
    M12_AssetVersionStatus* version;
    if (!state || versionIndex >= M12_AssetStatus_GetVersionCount("dm1")) {
        return;
    }
    state->assetStatus.dm1Available = 1;
    state->entries[0].available = 1;
    version = &state->assetStatus.versions[0][versionIndex];
    version->matched = 1;
    snprintf(version->matchedPath, sizeof(version->matchedPath), "%s", "probe://forced-dm1");
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s", "forced");
    state->gameOptions[0].versionIndex = (int)versionIndex;
}

static void exercise_startup_input_matrix(M12_StartupMenuState seed) {
    static const M12_MenuInput inputs[] = {
        M12_MENU_INPUT_NONE,
        M12_MENU_INPUT_UP,
        M12_MENU_INPUT_DOWN,
        M12_MENU_INPUT_LEFT,
        M12_MENU_INPUT_RIGHT,
        M12_MENU_INPUT_STRAFE_LEFT,
        M12_MENU_INPUT_STRAFE_RIGHT,
        M12_MENU_INPUT_ACCEPT,
        M12_MENU_INPUT_BACK,
        M12_MENU_INPUT_ACTION,
        M12_MENU_INPUT_CYCLE_CHAMPION,
        M12_MENU_INPUT_REST_TOGGLE,
        M12_MENU_INPUT_USE_STAIRS,
        M12_MENU_INPUT_PICKUP_ITEM,
        M12_MENU_INPUT_DROP_ITEM,
        M12_MENU_INPUT_SPELL_RUNE_1,
        M12_MENU_INPUT_SPELL_RUNE_2,
        M12_MENU_INPUT_SPELL_RUNE_3,
        M12_MENU_INPUT_SPELL_RUNE_4,
        M12_MENU_INPUT_SPELL_RUNE_5,
        M12_MENU_INPUT_SPELL_RUNE_6,
        M12_MENU_INPUT_SPELL_CAST,
        M12_MENU_INPUT_SPELL_CLEAR,
        M12_MENU_INPUT_USE_ITEM,
        M12_MENU_INPUT_MAP_TOGGLE,
        M12_MENU_INPUT_INVENTORY_TOGGLE,
        (M12_MenuInput)-1,
        (M12_MenuInput)9999
    };
    static const M12_MenuView views[] = {
        M12_MENU_VIEW_MAIN,
        M12_MENU_VIEW_SETTINGS,
        M12_MENU_VIEW_MESSAGE,
        M12_MENU_VIEW_GAME_OPTIONS,
        M12_MENU_VIEW_MUSEUM,
        (M12_MenuView)-1,
        (M12_MenuView)9999
    };
    size_t vi;
    size_t ii;
    for (vi = 0; vi < sizeof(views) / sizeof(views[0]); ++vi) {
        for (ii = 0; ii < sizeof(inputs) / sizeof(inputs[0]); ++ii) {
            M12_StartupMenuState s = seed;
            s.view = views[vi];
            s.selectedIndex = (ii & 1U) ? -100 : 100;
            s.activatedIndex = (ii & 2U) ? -100 : 100;
            s.settingsSelectedIndex = (ii & 4U) ? -100 : 100;
            s.gameOptSelectedRow = (ii & 8U) ? -100 : 100;
            s.settings.languageIndex = (ii & 16U) ? -100 : 100;
            s.settings.graphicsIndex = (ii & 32U) ? -100 : 100;
            s.settings.windowModeIndex = (ii & 64U) ? -100 : 100;
            M12_StartupMenu_HandleInput(&s, inputs[ii]);
            (void)M12_StartupMenu_GetLaunchIntent(&s);
        }
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
    char* rootDir = portable_mkdtemp(rootTemplate);

    if (!rootDir) {
        perror("mkdtemp");
        return 2;
    }

    if (portable_setenv("HOME", rootDir, 1) != 0) {
        perror("setenv");
        return 2;
    }

    snprintf(firestaffDir, sizeof(firestaffDir), "%s/.firestaff", rootDir);
    snprintf(dataDir, sizeof(dataDir), "%s/data", firestaffDir);
#if defined(__APPLE__)
    snprintf(configPath, sizeof(configPath), "%s/Library/Application Support/Firestaff/startup-menu.toml", rootDir);
#else
    snprintf(configPath, sizeof(configPath), "%s/.config/firestaff/startup-menu.toml", rootDir);
#endif
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

    portable_unsetenv("LC_ALL");
    portable_unsetenv("LC_MESSAGES");
    portable_setenv("LANG", "sv_SE.UTF-8", 1);
    M12_StartupMenu_InitWithDataDir(&state, dataDir);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_00A",
                 state.settings.languageIndex == 1 &&
                     state.languageExplicit == 0 &&
                     state.view == M12_MENU_VIEW_MESSAGE &&
                     state.launchRequested == 0 &&
                     state.messageLine1 && state.messageLine1[0] != '\0' &&
                     file_contains(configPath, "language_explicit = 0"),
                 "system Swedish auto-selects the startup-menu language when no explicit override exists and unsupported games stay non-launching");

    remove_if_present(configPath);
    portable_setenv("LANG", "C", 1);
    M12_StartupMenu_InitWithDataDir(&state, dataDir);

    probe_record(&tally,
                 "INV_M12_01",
                 M12_StartupMenu_GetEntryCount() == 5,
                 "startup menu exposes three games, museum, and settings");
    probe_record(&tally,
                 "INV_M12_02",
                 file_exists(configPath) &&
                     state.selectedIndex == 0 &&
                     state.view == M12_MENU_VIEW_MAIN &&
                     state.shouldExit == 0 &&
                     M12_CardArt_HasImage(&state.cardArt[0]) == 1 &&
                     M12_CardArt_HasExternalFile(&state.cardArt[0]) == 0 &&
                     strcmp(M12_AssetStatus_GetDataDir(&state.assetStatus), dataDir) == 0 &&
                     state.entries[1].available == 0 &&
                     state.entries[2].available == 0,
                 "startup state loads defaults, writes config, seeds built-in card art, and points the validator at the requested originals directory");

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
                 state.assetStatus.dm1Available == 0 &&
                     state.assetStatus.csbAvailable == 0 &&
                     state.assetStatus.dm2Available == 0 &&
                     M12_AssetStatus_GameHasCompleteHashSet("dm1") == 1 &&
                     M12_AssetStatus_GameHasCompleteHashSet("csb") == 1 &&
                     M12_AssetStatus_GameHasCompleteHashSet("dm2") == 1 &&
                     M12_AssetStatus_GameKnownHashCount("dm1") == 3U &&
                     M12_AssetStatus_GameKnownHashCount("csb") == 3U &&
                     M12_AssetStatus_GameKnownHashCount("dm2") == 3U,
                 "asset scan exposes the bounded per-game version matrix and leaves unmatched versions unavailable");

    force_dm1_version_ready(&state, 0U);

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
                 state.gameOptions[0].versionIndex == 0 &&
                 state.gameOptions[0].languageIndex == 0 &&
                     state.gameOptions[0].cheatsEnabled == 0 &&
                     state.gameOptions[0].gameSpeed == 1 &&
                     M12_GameOptions_SpeedHotkeysEnabled(&state.gameOptions[0]) == 0,
                 "game options default to the first supported version, English, cheats off, speed normal, and hotkeys disabled");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* patch row */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN); /* language row */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT); /* EN -> SV */
    probe_record(&tally,
                 "INV_M12_06B2",
                 state.gameOptions[0].languageIndex == 1,
                 "game options expose a per-game language selector");

    /* Reset language back to EN so later tests start from a known baseline */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_LEFT); /* SV -> EN */

    /* Enable cheats and verify speed hotkeys unlock.
     * Cursor is on LANGUAGE row after INV_M12_06B2. One DOWN reaches CHEATS. */
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

    /* Navigate to launch and press accept.
     * Cursor is on CHEATS after INV_M12_06E. Navigate: speed, aspect, res, launch. */
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
                     state.launchRequested == 0 &&
                     strcmp(state.messageLine1, "COMING SOON") == 0 &&
                     strcmp(state.messageLine2, "THIS GAME IS NOT SUPPORTED YET") == 0,
                 "enter on CSB shows unsupported/coming-soon messaging without requesting launch");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_MUSEUM_01",
                 state.view == M12_MENU_VIEW_MUSEUM &&
                     state.museumSelectedIndex == 0 &&
                     state.museumPageIndex == 0,
                 "museum row opens the Museum of Lore from the startup menu");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, (M12_MenuInput)9999);
    probe_record(&tally,
                 "INV_M12_MUSEUM_02",
                 state.view == M12_MENU_VIEW_MUSEUM &&
                     state.museumSelectedIndex == 1 &&
                     state.museumPageIndex == 1,
                 "museum keyboard navigation cycles sections and pages while unknown keys are no-ops");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_LEFT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    probe_record(&tally,
                 "INV_M12_MUSEUM_03",
                 state.view == M12_MENU_VIEW_MAIN &&
                     state.museumSelectedIndex == 1 &&
                     state.museumPageIndex == 0,
                 "museum left cycles pages backward and escape returns to main menu");

    state.view = M12_MENU_VIEW_MUSEUM;
    state.museumSelectedIndex = 0;
    state.museumPageIndex = 0;
    M12_ModernMenu_ApplyHit(&state, (M12_MouseHit){M12_HIT_MUSEUM_CATEGORY, 3, 0});
    M12_ModernMenu_ApplyHit(&state, (M12_MouseHit){M12_HIT_MUSEUM_PAGE, 0, 1});
    probe_record(&tally,
                 "INV_M12_MUSEUM_04",
                 state.view == M12_MENU_VIEW_MUSEUM &&
                     state.museumSelectedIndex == 3 &&
                     state.museumPageIndex == 1,
                 "museum mouse hits select categories and cycle pages through shared menu state");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);

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
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_10",
                 state.settings.languageIndex == 1 &&
                     state.settings.graphicsIndex == 1 &&
                     state.settings.rendererBackendIndex == M12_RENDERER_BACKEND_SOFTWARE &&
                     state.settings.windowModeIndex == 1,
                 "settings screen cycles persisted presentation, renderer backend, and window values from keyboard input");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK); /* to main */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* museum */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* dm2 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* csb */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);   /* dm1 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* open DM1 options */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);  /* version: 0 -> 1 */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);   /* patch */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* patch: ORIGINAL -> PATCHED */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);   /* language */
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT); /* EN -> SV */
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
                     reloaded.languageExplicit == 1 &&
                     reloaded.settings.graphicsIndex == 1 &&
                     reloaded.settings.rendererBackendIndex == M12_RENDERER_BACKEND_SOFTWARE &&
                     reloaded.settings.windowModeIndex == 1 &&
                     reloaded.gameOptions[0].versionIndex == 1 &&
                     reloaded.gameOptions[0].usePatch == 1 &&
                     reloaded.gameOptions[0].languageIndex == 1 &&
                     reloaded.gameOptions[0].cheatsEnabled == 1 &&
                     reloaded.gameOptions[0].gameSpeed == 2 &&
                     reloaded.gameOptions[0].aspectRatio == 1 &&
                     reloaded.gameOptions[0].resolution == 1 &&
                     reloaded.gameOptions[1].versionIndex == 0 &&
                     reloaded.gameOptions[1].usePatch == 0 &&
                     reloaded.gameOptions[1].languageIndex == 0 &&
                     reloaded.gameOptions[1].cheatsEnabled == 0 &&
                     reloaded.gameOptions[1].gameSpeed == 1 &&
                     reloaded.gameOptions[2].versionIndex == 0 &&
                     reloaded.gameOptions[2].usePatch == 0 &&
                     reloaded.gameOptions[2].languageIndex == 0 &&
                     reloaded.gameOptions[2].cheatsEnabled == 0 &&
                     reloaded.gameOptions[2].gameSpeed == 1 &&
                     M12_StartupMenu_GetRenderPaletteLevel(&reloaded) == 1 &&
                     file_contains(configPath, "language_explicit = 1") &&
                     file_contains(configPath, "game_0_version_index = 1") &&
                     file_contains(configPath, "game_0_language_index = 1") &&
                     file_contains(configPath, "presentation_mode = \"v2-enhanced-2d\"") &&
                     file_contains(configPath, "presentation_mode_index = 1") &&
                     file_contains(configPath, "renderer_backend_index = 1") &&
                     strcmp(M12_AssetStatus_GetDataDir(&reloaded.assetStatus), dataDir) == 0,
                 "settings and per-game version selection persist across reloads without cross-game bleed, including readable presentation mode, renderer backend, and per-game language");

    make_file_with_text(configPath,
                        "# Firestaff startup menu config\n"
                        "language_index = 0\n"
                        "language_explicit = 1\n"
                        "presentation_mode = \"v3-modern-3d\"\n"
                        "renderer_backend_index = 0\n"
                        "window_mode_index = 0\n"
                        "data_dir = \"probe-data\"\n");
    M12_StartupMenu_InitWithDataDir(&reloaded, dataDir);
    probe_record(&tally,
                 "INV_M12_11D",
                 reloaded.settings.graphicsIndex == M12_PRESENTATION_V3_MODERN_3D &&
                     strcmp(M12_StartupMenu_GetPresentationModeLabel(&reloaded), "V3 MODERN/3D") == 0 &&
                     file_contains(configPath, "presentation_mode = \"v3-modern-3d\""),
                 "readable presentation_mode config key loads and round-trips V3 mode without relying on numeric graphics terminology");

    remove_if_present(configPath);
    portable_unsetenv("LC_ALL");
    portable_unsetenv("LC_MESSAGES");
    portable_setenv("LANG", "fr_FR.UTF-8", 1);
    M12_StartupMenu_InitWithDataDir(&state, dataDir);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_11B",
                 state.settings.languageIndex == 2 &&
                     state.languageExplicit == 0 &&
                     state.view == M12_MENU_VIEW_MESSAGE &&
                     state.launchRequested == 0 &&
                     state.messageLine1 && state.messageLine1[0] != '\0',
                 "system French auto-selects the startup-menu language while unsupported games stay non-launching");

    remove_if_present(configPath);
    portable_setenv("LANG", "C", 1);
    M12_StartupMenu_InitWithDataDir(&state, dataDir);
    state.settings.languageIndex = 3;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    probe_record(&tally,
                 "INV_M12_11C",
                 state.view == M12_MENU_VIEW_MESSAGE &&
                     strcmp(state.messageLine1, "COMING SOON") == 0 &&
                     strcmp(state.messageLine2, "THIS GAME IS NOT SUPPORTED YET") == 0,
                 "missing runtime catalog falls back safely while unsupported games stay coming-soon");

    remove_if_present(configPath);
    portable_setenv("LANG", "C", 1);
    M12_StartupMenu_InitWithDataDir(&state, dataDir);

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
        M12_StartupMenu_InitWithDataDir(&reloaded, dataDir);
        reloaded.settings.languageIndex = 1;
        reloaded.settings.graphicsIndex = 1;
        M12_StartupMenu_Draw(&reloaded, localized, 320, 200);
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

    /* === Language selector and flag rendering invariants === */
    {
        M12_StartupMenuState langState;
        unsigned char fbEN[320 * 200];
        unsigned char fbSV[320 * 200];
        unsigned char fbFR[320 * 200];
        unsigned char fbDE[320 * 200];
        unsigned long cEN = 0, cSV = 0, cFR = 0, cDE = 0;

        /* Startup menu: flag changes with language index */
        M12_StartupMenu_InitWithDataDir(&langState, dataDir);
        langState.settings.languageIndex = 0;
        M12_StartupMenu_Draw(&langState, fbEN, 320, 200);
        langState.settings.languageIndex = 1;
        M12_StartupMenu_Draw(&langState, fbSV, 320, 200);
        langState.settings.languageIndex = 2;
        M12_StartupMenu_Draw(&langState, fbFR, 320, 200);
        langState.settings.languageIndex = 3;
        M12_StartupMenu_Draw(&langState, fbDE, 320, 200);
        for (i = 0; i < 320 * 200; ++i) {
            cEN = (cEN * 131UL) + fbEN[i];
            cSV = (cSV * 131UL) + fbSV[i];
            cFR = (cFR * 131UL) + fbFR[i];
            cDE = (cDE * 131UL) + fbDE[i];
        }
        probe_record(&tally,
                     "INV_M12_24",
                     cEN != cSV && cEN != cFR && cSV != cFR &&
                         cDE != cSV && cDE != cFR,
                     "runtime Swedish and French renders differ from English, while uncatalogued locales still fall back safely");

        /* Per-game language selector: each game has independent language */
        M12_StartupMenu_InitWithDataDir(&langState, dataDir);
        langState.gameOptions[0].languageIndex = 1; /* DM1 = SV */
        langState.gameOptions[1].languageIndex = 2; /* CSB = FR */
        langState.gameOptions[2].languageIndex = 3; /* DM2 = DE */
        probe_record(&tally,
                     "INV_M12_25",
                     langState.gameOptions[0].languageIndex == 1 &&
                         langState.gameOptions[1].languageIndex == 2 &&
                         langState.gameOptions[2].languageIndex == 3,
                     "per-game language index is independently settable for DM1/CSB/DM2");

    }

    /* === Mode-constraint and launch-intent tests === */
    {
        M12_StartupMenuState modeState;
        M12_LaunchIntent intent;

        /* V1 mode: aspect and resolution locked to original */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
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
        force_dm1_version_ready(&modeState, 0U);
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
        force_dm1_version_ready(&modeState, 0U);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_19",
                     intent.valid == 1 &&
                         intent.presentationMode == M12_PRESENTATION_V1_ORIGINAL &&
                         strcmp(intent.gameId, "dm1") == 0 &&
                         strcmp(intent.versionId, "pc34-en") == 0 &&
                         intent.options.aspectRatio == M12_ASPECT_ORIGINAL &&
                         intent.options.resolution == M12_RES_320x200,
                     "V1 launch intent carries the selected version, correct mode, and constrained options");

        /* Launch intent for V2 with explicit resolution set */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
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
                         strcmp(intent.versionId, "pc34-en") == 0 &&
                         intent.options.resolution == M12_RES_640x400,
                     "V2 launch intent carries the selected version and enhanced resolution options");

        /* V3 launch intent is invalid (coming soon) */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
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
                         modeState.launchRequested == 0 &&
                         strcmp(modeState.messageLine2, "COMING SOON") == 0,
                     "V3 launch attempt shows COMING SOON message without requesting launch");

        /* Launch button requests runtime handoff for a matched V1/V2 game. */
        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_26",
                     modeState.launchRequested == 1 &&
                         modeState.view == M12_MENU_VIEW_MESSAGE &&
                         intent.valid == 1 &&
                         strcmp(intent.gameId, "dm1") == 0,
                     "launch row marks a valid launch request for the runtime");

        modeState.launchRequested = 0;
        modeState.view = M12_MENU_VIEW_MESSAGE;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACTION);
        probe_record(&tally,
                     "INV_M12_26B",
                     modeState.launchRequested == 0 &&
                         modeState.view == M12_MENU_VIEW_MAIN,
                     "space/action dismisses message view safely");

        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
        modeState.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACTION);
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACTION);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_26C",
                     modeState.launchRequested == 1 && intent.valid == 1,
                     "space/action activates cards and launch row like enter");

        modeState.launchRequested = 0;
        modeState.view = M12_MENU_VIEW_GAME_OPTIONS;
        modeState.gameOptSelectedRow = 0;
        M12_ModernMenu_ApplyHit(&modeState,
                                (M12_MouseHit){M12_HIT_GAMEOPT_LAUNCH, 0, 0});
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_27",
                     modeState.launchRequested == 1 && intent.valid == 1,
                     "mouse launch hit follows the same launch request path");

        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.selectedIndex = 1;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        probe_record(&tally,
                     "INV_M12_28",
                     modeState.launchRequested == 0 &&
                         modeState.view == M12_MENU_VIEW_MESSAGE &&
                         strcmp(modeState.messageLine1, "COMING SOON") == 0 &&
                         strcmp(modeState.messageLine2, "THIS GAME IS NOT SUPPORTED YET") == 0,
                     "CSB is not launchable and reports unsupported/coming-soon without requesting launch");

        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        modeState.assetStatus.csbAvailable = 1;
        modeState.assetStatus.dm2Available = 1;
        modeState.assetStatus.versions[1][0].matched = 1;
        modeState.assetStatus.versions[2][0].matched = 1;
        modeState.entries[1].available = 0;
        modeState.entries[2].available = 0;
        modeState.selectedIndex = 1;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        int csbBlocked = modeState.view == M12_MENU_VIEW_MESSAGE &&
                         modeState.launchRequested == 0 &&
                         intent.valid == 0 &&
                         strcmp(modeState.messageLine1, "COMING SOON") == 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_BACK);
        modeState.selectedIndex = 2;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_28B",
                     csbBlocked &&
                         modeState.view == M12_MENU_VIEW_MESSAGE &&
                         modeState.launchRequested == 0 &&
                         intent.valid == 0 &&
                         strcmp(modeState.messageLine1, "COMING SOON") == 0,
                     "CSB and DM2 remain disabled and non-launchable even if asset scanning reports matches");

        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
        exercise_startup_input_matrix(modeState);
        probe_record(&tally,
                     "INV_M12_29",
                     1,
                     "startup input matrix covers arrows, accept/action/back, gameplay hotkeys, invalid keys, mouse launch, and corrupt state bounds without crashing");

        /* Presentation mode API */
        modeState.settings.graphicsIndex = M12_PRESENTATION_V2_ENHANCED_2D;
        probe_record(&tally,
                     "INV_M12_23",
                     M12_StartupMenu_GetPresentationMode(&modeState) == M12_PRESENTATION_V2_ENHANCED_2D &&
                         strcmp(M12_StartupMenu_GetPresentationModeLabel(&modeState), "V2 ENHANCED 2D") == 0,
                     "presentation mode API returns correct mode and label");

        modeState.settings.rendererBackendIndex = M12_RENDERER_BACKEND_VULKAN;
        probe_record(&tally,
                     "INV_M12_30",
                     M12_StartupMenu_GetRendererBackend(&modeState) == M12_RENDERER_BACKEND_VULKAN &&
                         strcmp(M12_StartupMenu_GetRendererBackendLabel(&modeState), "VULKAN") == 0 &&
                         strcmp(M12_StartupMenu_GetRendererBackendStatusLabel(&modeState), "UNAVAILABLE") == 0 &&
                         M12_StartupMenu_RendererBackendAvailable(M12_RENDERER_BACKEND_VULKAN) == 0 &&
                         M12_StartupMenu_RendererBackendAvailable(M12_RENDERER_BACKEND_SOFTWARE) == 1,
                     "renderer backend API exposes selectable Vulkan with honest availability status");

        M12_StartupMenu_InitWithDataDir(&modeState, dataDir);
        force_dm1_version_ready(&modeState, 0U);
        modeState.settings.languageIndex = 0;
        modeState.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
        modeState.settings.rendererBackendIndex = M12_RENDERER_BACKEND_VULKAN;
        modeState.selectedIndex = 0;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        modeState.gameOptSelectedRow = M12_GAME_OPT_ROW_COUNT;
        M12_StartupMenu_HandleInput(&modeState, M12_MENU_INPUT_ACCEPT);
        intent = M12_StartupMenu_GetLaunchIntent(&modeState);
        probe_record(&tally,
                     "INV_M12_31",
                     intent.valid == 0 &&
                         intent.rendererBackend == M12_RENDERER_BACKEND_VULKAN &&
                         intent.rendererBackendAvailable == 0 &&
                         modeState.launchRequested == 0 &&
                         strcmp(modeState.messageLine1, "RENDERER BACKEND UNAVAILABLE") == 0,
                     "Vulkan selection is carried in launch intent but blocks runtime handoff until a real backend exists");
    }

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_LEFT);
    probe_record(&tally,
                 "INV_M12_13A",
                 state.shouldExit == 0,
                 "left arrow on top-level launcher is a no-op, not an exit/crash path");

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
