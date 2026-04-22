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

int main(void) {
    unsigned char framebufferA[320 * 200];
    unsigned char framebufferB[320 * 200];
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
    if (setenv("HOME", rootDir, 1) != 0 || setenv("SDL_VIDEODRIVER", "dummy", 1) != 0) {
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
    M11_ApplyStartupMenuRuntime(&state);

    smoke_record(&tally,
                 "SMOKE_01",
                 M11_Render_GetWindowMode() == M11_WINDOW_MODE_WINDOWED &&
                     M11_Render_GetPaletteLevel() == 0,
                 "default startup settings apply to render runtime");

    M12_StartupMenu_Draw(&state, framebufferA, 320, 200);

    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_DOWN);
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
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
                     state.settings.windowModeIndex == 1 &&
                     M11_Render_GetWindowMode() == M11_WINDOW_MODE_FULLSCREEN &&
                     M11_Render_GetPaletteLevel() == 1,
                 "changed settings update runtime window mode and palette");

    M12_StartupMenu_Draw(&state, framebufferB, 320, 200);
    smoke_record(&tally,
                 "SMOKE_03",
                 smoke_checksum(framebufferA, sizeof(framebufferA)) !=
                     smoke_checksum(framebufferB, sizeof(framebufferB)),
                 "language and presentation mode changes alter launcher output");

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
