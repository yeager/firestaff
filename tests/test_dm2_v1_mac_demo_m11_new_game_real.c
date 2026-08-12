#include "m11_game_view.h"
#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_DEMO_ZIP");
    unsigned char framebuffer[320u * 200u];

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac demo ZIP environment is not set");
        return 0;
    }
    memset(&state, 0, sizeof(state));
    memset(&spec, 0, sizeof(spec));
    spec.title = "Dungeon Master II Macintosh First Chapter";
    spec.gameId = "dm2";
    spec.dataDir = zip;
    spec.sourceId = "mac-en-demo";
    spec.presentationWidth = 320;
    spec.presentationHeight = 200;
    spec.launcherOptionsBound = 1;
    M11_GameView_Init(&state);
    if (!M11_GameView_Start(&state, &spec) ||
        state.sourceKind != M11_GAME_SOURCE_DM2_BOOT ||
        !state.dm2BootProfile ||
        strcmp(((const DM2_V1_BootProfile *)state.dm2BootProfile)->version_id,
               "mac-en-demo") != 0 ||
        state.dm2MacMovieActive || !state.dm2State.startup_menu_active) {
        fprintf(stderr,
                "Mac demo M11 launch was not bound: start=%d source=%d version=%s movie=%d menu=%d\n",
                state.active, state.sourceKind,
                state.dm2BootProfile
                    ? ((const DM2_V1_BootProfile *)state.dm2BootProfile)->version_id
                    : "(none)",
                state.dm2MacMovieActive, state.dm2State.startup_menu_active);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED ||
        state.dm2State.startup_menu_active ||
        !state.dm2State.level_loaded ||
        !((const DM2_V1_BootProfile *)state.dm2BootProfile)
             ->source_game_load_session_ready) {
        fprintf(stderr,
                "Mac demo M11 New Game did not publish authentic session: menu=%d level=%d ready=%d\n",
                state.dm2State.startup_menu_active,
                state.dm2State.level_loaded,
                ((const DM2_V1_BootProfile *)state.dm2BootProfile)
                    ->source_game_load_session_ready);
        M11_GameView_Shutdown(&state);
        return 1;
    }
    M11_GameView_Shutdown(&state);
    puts("PASS: M11 starts the authentic DM2 Macintosh demo New Game from the original ZIP");
    return 0;
}
