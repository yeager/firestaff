#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    int ok;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 0;
    }
    memset(&state, 0, sizeof(state));
    memset(&spec, 0, sizeof(spec));
    spec.title = "Dungeon Master II Macintosh";
    spec.gameId = "dm2";
    spec.dataDir = zip;
    spec.sourceId = "mac-en-retail";
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.launcherOptionsBound = 1;
    ok = (M11_GameView_Init(&state), M11_GameView_Start(&state, &spec));
    ok = ok && state.sourceKind == M11_GAME_SOURCE_DM2_BOOT &&
         state.dm2BootProfile != NULL && state.dm2MacMovieActive &&
         state.dm2MacMovieDecoder.frame_ready &&
         state.dm2MacMovieDecoder.frame_index == 1u;
    if (!ok) {
        fprintf(stderr, "Mac M11 movie runtime was not bound: start=%d source=%d active=%d rejected=%d\n",
                state.active, state.sourceKind, state.dm2MacMovieActive,
                state.dm2MacMovieRejected);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    printf("PASS: M11 binds authentic Mac Title.MooV at startup: frame=%u\n",
           state.dm2MacMovieDecoder.frame_index);
    M11_GameView_Shutdown(&state);
    return 0;
}
