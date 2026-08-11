#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    const char *archive = getenv("FIRESTAFF_CSB_FMTOWNS_ARCHIVE");
    const char *language = getenv("FIRESTAFF_CSB_FMTOWNS_GAME_LANGUAGE");
    const int japanese = language && strcmp(language, "ja") == 0;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;

    if (!archive || !archive[0]) {
        puts("SKIP: FIRESTAFF_CSB_FMTOWNS_ARCHIVE not set");
        return 0;
    }
    memset(&spec, 0, sizeof(spec));
    spec.title = "CHAOS STRIKES BACK";
    spec.gameId = "csb";
    spec.sourceId = "csb";
    spec.dataDir = archive;
    spec.csbFmtownsJapanese = japanese;
    spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    M11_GameView_Init(&view);
    if (!M11_GameView_Start(&view, &spec)) {
        fprintf(stderr, "FAIL: packed CSB FM Towns M11 start\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    if (!view.csbBootProfile || !view.active) {
        fprintf(stderr, "FAIL: packed CSB FM Towns M11 runtime ownership\n");
        M11_GameView_Shutdown(&view);
        return 1;
    }
    puts("PASS: packed CSB FM Towns M11 starts from original ZIP");
    M11_GameView_Shutdown(&view);
    return 0;
}
