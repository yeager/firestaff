#include "menu_startup_m12.h"
#include "config_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>


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

static void force_dm1_available(M12_StartupMenuState* state) {
    state->entries[0].title = "DUNGEON MASTER";
    state->entries[0].gameId = "dm1";
    state->entries[0].kind = M12_MENU_ENTRY_GAME;
    state->entries[0].sourceKind = M12_MENU_SOURCE_BUILTIN_CATALOG;
    state->entries[0].available = 1;
    state->assetStatus.versions[0][0].gameId = "dm1";
    state->assetStatus.versions[0][0].versionId = "pc34";
    state->assetStatus.versions[0][0].label = "PC 3.4";
    state->assetStatus.versions[0][0].shortLabel = "PC34";
    state->assetStatus.versions[0][0].matched = 1;
    state->gameOptions[0].versionIndex = 0;
    state->settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    state->settings.rendererBackendIndex = M12_RENDERER_BACKEND_SOFTWARE;
    state->view = M12_MENU_VIEW_MAIN;
}

static int write_fake_quicksave(const char* path) {
    static const unsigned char hdr[16] = {
        'F','S','M','1','1','Q','S','1',
        4,0,0,0,
        0,0,0,0
    };
    static const unsigned char blob[4] = { 0, 0, 0, 0 };
    FILE* fp = fopen(path, "wb");
    if (!fp) return 0;
    if (fwrite(hdr, 1U, sizeof(hdr), fp) != sizeof(hdr) ||
        fwrite(blob, 1U, sizeof(blob), fp) != sizeof(blob) ||
        fclose(fp) != 0) {
        return 0;
    }
    return 1;
}

int main(void) {
    char tmpTemplate[] = "/tmp/firestaff-m12-qr-XXXXXX";
    char savePath[512];
    M12_StartupMenuState state;
    M12_LaunchIntent intent;

    if (!mkdtemp(tmpTemplate)) {
        perror("mkdtemp");
        return 1;
    }
    m12_test_setenv("HOME", tmpTemplate);

    M12_Config_SetLastSavePath("");
    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets");
    force_dm1_available(&state);
    if (!expect(state.quickResumeAvailable == 0, "no-save must disable quick Resume")) return 1;

    snprintf(savePath, sizeof(savePath), "%s/firestaff-dm1-quicksave.sav", tmpTemplate);
    if (!expect(write_fake_quicksave(savePath), "should write fake quicksave")) return 1;
    M12_Config_SetLastSavePath(savePath);

    M12_StartupMenu_InitWithDataDir(&state, "/tmp/firestaff-test-no-assets");
    force_dm1_available(&state);
    if (!expect(state.quickResumeAvailable == 1, "valid DM1 quicksave must enable quick Resume")) return 1;
    if (!expect(strcmp(state.quickResumeGameId, "dm1") == 0, "quick Resume should identify dm1")) return 1;
    if (!expect(strcmp(state.quickResumeSavePath, savePath) == 0, "quick Resume should retain save path")) return 1;

    state.selectedIndex = -1;
    M12_StartupMenu_HandleInput(&state, M12_MENU_INPUT_ACCEPT);
    if (!expect(state.launchRequested == 1, "Resume accept should request launch")) return 1;
    if (!expect(state.activatedIndex == 0, "Resume accept should activate DM1 slot")) return 1;

    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1, "Resume launch intent should be valid for matched DM1")) return 1;
    if (!expect(intent.savePath && strcmp(intent.savePath, savePath) == 0,
                "Resume launch intent must carry exact save path")) return 1;

    puts("ok: no-save disables Resume; valid DM1 quicksave enables Resume and carries save path");
    return 0;
}
