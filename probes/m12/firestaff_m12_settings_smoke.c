#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif
#include "main_loop_m11.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

typedef struct {
    int total;
    int passed;
} SmokeTally;

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static void smoke_record(SmokeTally* tally,
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

static unsigned long smoke_checksum(const unsigned char* data, size_t size) {
    unsigned long hash = 2166136261u;
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= (unsigned long)data[i];
        hash *= 16777619u;
    }
    return hash;
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

static void force_dm1_version_ready(M12_StartupMenuState* state, size_t versionIndex) {
    M12_AssetVersionStatus* version;
    if (!state || versionIndex >= M12_AssetStatus_GetVersionCount("dm1")) {
        return;
    }
    state->assetStatus.dm1Available = 1;
    state->entries[0].available = 1;
    version = &state->assetStatus.versions[0][versionIndex];
    version->matched = 1;
    snprintf(version->matchedPath, sizeof(version->matchedPath), "%s", "smoke://forced-dm1");
    snprintf(version->matchedMd5, sizeof(version->matchedMd5), "%s", "forced");
    state->gameOptions[0].versionIndex = (int)versionIndex;
}

int main(void) {
    unsigned char framebufferA[320 * 200];
    unsigned char framebufferB[320 * 200];
    unsigned char framebufferC[320 * 200];
    M12_StartupMenuState state;
    SmokeTally tally = {0, 0};
    char rootTemplate[] = "/tmp/firestaff-m12-smoke-XXXXXX";
    char firestaffDir[1024];
    char dataDir[1024];
    char graphicsPath[1024];
    char dungeonPath[1024];
    char cardsDir[1024];
    char cardPath[1024];
    char* rootDir = mkdtemp(rootTemplate);

    if (!rootDir) {
        perror("mkdtemp");
        return 2;
    }
    if (setenv("HOME", rootDir, 1) != 0 ||
        setenv("SDL_VIDEODRIVER", "dummy", 1) != 0 ||
        setenv("LANG", "fr_FR.UTF-8", 1) != 0) {
        perror("setenv");
        return 2;
    }

    snprintf(firestaffDir, sizeof(firestaffDir), "%s/.firestaff", rootDir);
    snprintf(dataDir, sizeof(dataDir), "%s/data", firestaffDir);
    snprintf(graphicsPath, sizeof(graphicsPath), "%s/GRAPHICS.DAT", dataDir);
    snprintf(dungeonPath, sizeof(dungeonPath), "%s/DUNGEON.DAT", dataDir);
    snprintf(cardsDir, sizeof(cardsDir), "%s/cards", dataDir);
    snprintf(cardPath, sizeof(cardPath), "%s/dm1.png", cardsDir);

    if (!make_dir(firestaffDir) || !make_dir(dataDir) || !make_dir(cardsDir)) {
        perror("mkdir");
        return 2;
    }
    make_file_with_text(graphicsPath, "ok");
    make_file_with_text(dungeonPath, "ok");
    make_file_with_text(cardPath, "placeholder image slot");

    if (M11_Render_Init(640, 400, M11_SCALE_2X) != M11_RENDER_OK) {
        fprintf(stderr, "render init failed\n");
        return 2;
    }

    M12_StartupMenu_InitWithDataDir(&state, dataDir);
    force_dm1_version_ready(&state, 0U);
    M11_ApplyStartupMenuRuntime(&state);

    smoke_record(&tally,
                 "SMOKE_00",
                 state.settings.languageIndex == 2 && state.languageExplicit == 0,
                 "system French auto-detection selects the startup-menu runtime locale before any explicit override");

    smoke_record(&tally,
                 "SMOKE_01",
                 M11_Render_GetWindowMode() == M11_WINDOW_MODE_WINDOWED &&
                     M11_Render_GetPaletteLevel() == 0 &&
                     M11_Render_GetScaleMode() == M11_SCALE_FIT &&
                     M11_Render_GetIntegerScaling() == 1 &&
                     M11_Render_GetScaleFilter() == M11_SCALE_FILTER_NEAREST &&
                     M11_Render_GetVSync() == M11_VSYNC_ON,
                 "default startup settings apply SDL3 render runtime preferences");

    M12_StartupMenu_Draw(&state, framebufferA, 320, 200);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_LEFT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M11_ApplyStartupMenuRuntime(&state);

    smoke_record(&tally,
                 "SMOKE_02",
                 state.settings.languageIndex == 1 &&
                     state.settings.graphicsIndex == 1 &&
                     state.settings.rendererBackendIndex == M12_RENDERER_BACKEND_SOFTWARE &&
                     state.settings.windowModeIndex == 1 &&
                     state.settings.scaleModeIndex == M11_SCALE_STRETCH &&
                     state.settings.integerScaling == 0 &&
                     state.settings.scalingFilterIndex == M11_SCALE_FILTER_LINEAR &&
                     state.settings.vsyncIndex == M11_VSYNC_OFF &&
                     M11_Render_GetWindowMode() == M11_WINDOW_MODE_FULLSCREEN &&
                     M11_Render_GetPaletteLevel() == 1 &&
                     M11_Render_GetScaleMode() == M11_SCALE_STRETCH &&
                     M11_Render_GetIntegerScaling() == 0 &&
                     M11_Render_GetScaleFilter() == M11_SCALE_FILTER_LINEAR &&
                     M11_Render_GetVSync() == M11_VSYNC_OFF,
                 "changed settings update runtime window mode, renderer backend state, palette, scale, filter, pixel snap, and vsync");

    M12_StartupMenu_Draw(&state, framebufferB, 320, 200);
    smoke_record(&tally,
                 "SMOKE_03",
                 smoke_checksum(framebufferA, sizeof(framebufferA)) !=
                     smoke_checksum(framebufferB, sizeof(framebufferB)),
                 "language and presentation mode changes alter launcher output");

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_BACK);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_UP);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_RIGHT);
    M12_StartupMenu_Draw(&state, framebufferC, 320, 200);
    smoke_record(&tally,
                 "SMOKE_03B",
                 state.gameOptions[0].languageIndex == 1 &&
                     smoke_checksum(framebufferA, sizeof(framebufferA)) !=
                         smoke_checksum(framebufferC, sizeof(framebufferC)),
                 "per-game language selection is reachable and changes the game-options render output");

    smoke_record(&tally,
                 "SMOKE_04",
                 M12_CardArt_HasImage(&state.cardArt[0]) == 1 &&
                     M12_CardArt_HasExternalFile(&state.cardArt[0]) == 1 &&
                     strcmp(M12_CardArt_GetFileName(&state.cardArt[0]), "dm1.png") == 0,
                 "card-art slot loader prefers external artwork over the built-in fallback");

    printf("# summary: %d/%d invariants passed\n", tally.passed, tally.total);
    M11_Render_Shutdown();
    return (tally.passed == tally.total) ? 0 : 1;
}
