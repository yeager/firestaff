#include "menu_startup_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static int expect(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        return 0;
    }
    return 1;
}

int main(void) {
    char directory[] = "/tmp/firestaff-nexus-menu-gate.XXXXXX";
    char cue[512];
    FILE* file;
    M12_StartupMenuState state;
    M12_StartupMenuInitOptions options;
    M12_StartupLaunchGate gate;
    M12_LaunchIntent intent;

    if (!mkdtemp(directory)) {
        perror("mkdtemp");
        return 1;
    }
    snprintf(cue, sizeof(cue), "%s/Dungeon Master Nexus (English).cue", directory);
    file = fopen(cue, "wb");
    if (!file) {
        perror("fopen");
        rmdir(directory);
        return 1;
    }
    fputs("FILE \"Dungeon Master Nexus (English).iso\" BINARY\n", file);
    fclose(file);

    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&state, "/tmp/firestaff-no-assets", NULL,
                                    &options);
    state.assetStatus.nexusAvailable = 1;
    state.entries[3].available = 1;
    state.assetStatus.versions[3][0].gameId = "nexus";
    state.assetStatus.versions[3][0].versionId = "test-nexus";
    state.assetStatus.versions[3][0].label = "Test Nexus";
    state.assetStatus.versions[3][0].shortLabel = "TEST";
    state.assetStatus.versions[3][0].matched = 1;
    state.gameOptions[3].versionIndex = 0;
    state.activatedIndex = 3;

    setenv("FIRESTAFF_NEXUS_MEDNAFEN", "/usr/bin/true", 1);
    setenv("FIRESTAFF_NEXUS_DISC", cue, 1);
    if (!expect(M12_StartupMenu_GetLaunchGate(&state, 3, &gate) == 1,
                "Nexus menu gate should build")) return 1;
    if (!expect(gate.canLaunch == 1,
                "verified external emulator and CUE should enable Nexus menu launch")) return 1;
    if (!expect(gate.blockedLabel && strcmp(gate.blockedLabel, "READY TO LAUNCH") == 0,
                "external Nexus menu route should show ready status")) return 1;
    if (!expect(gate.blockedDetail &&
                strcmp(gate.blockedDetail, "EXTERNAL RETAIL SATURN LAUNCH") == 0,
                "menu status must disclose external retail launch")) return 1;
    intent = M12_StartupMenu_GetLaunchIntent(&state);
    if (!expect(intent.valid == 1 && intent.gameId &&
                strcmp(intent.gameId, "nexus") == 0,
                "Nexus menu intent should remain valid for the external route")) return 1;

    unsetenv("FIRESTAFF_NEXUS_MEDNAFEN");
    unsetenv("FIRESTAFF_NEXUS_DISC");
    unlink(cue);
    rmdir(directory);
    printf("test_m12_nexus_mednafen_menu_gate: PASS\n");
    return 0;
}
