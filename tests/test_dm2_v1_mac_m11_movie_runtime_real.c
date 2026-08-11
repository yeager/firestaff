#include "m11_game_view.h"
#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

int main(void)
{
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    unsigned char framebuffer[320u * 200u];
    DM2_V1_StartupMenuAuxPointerLayout aux;
    int frame;

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
    M11_GameView_Init(&state);
    if (!M11_GameView_Start(&state, &spec) ||
        state.sourceKind != M11_GAME_SOURCE_DM2_BOOT ||
        !state.dm2BootProfile || !state.dm2MacMovieActive ||
        !state.dm2MacMovieDecoder.frame_ready ||
        state.dm2MacMovieDecoder.frame_index != 1u) {
        fprintf(stderr, "Mac M11 movie runtime was not bound: start=%d source=%d active=%d rejected=%d\n",
                state.active, state.sourceKind, state.dm2MacMovieActive,
                state.dm2MacMovieRejected);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    for (frame = 0; state.dm2MacMovieActive && frame < 2000; ++frame)
        M11_GameView_Draw(&state, framebuffer, 320, 200);

    memset(&aux, 0, sizeof(aux));
    if (state.dm2MacMovieActive ||
        !dm2_v1_boot_startup_menu_aux_pointer_layout(
            (DM2_V1_BootProfile *)state.dm2BootProfile, &aux) ||
        !aux.valid || aux.show_credits.w <= 0 || aux.show_credits.h <= 0 ||
        M11_GameView_HandlePointer(
            &state, aux.show_credits.x + aux.show_credits.w / 2,
            aux.show_credits.y + aux.show_credits.h / 2, 1) ==
            M11_GAME_INPUT_IGNORED ||
        !state.dm2MacMovieActive ||
        state.dm2MacMovieIndex != DM2_V1_MAC_MOVIE_CREDITS ||
        !state.dm2State.startup_credits_active) {
        fprintf(stderr, "Mac Credits.MooV route was not bound: title_active=%d frame=%d credits_active=%d index=%d rejected=%d\n",
                state.dm2MacMovieActive, frame,
                state.dm2State.startup_credits_active,
                state.dm2MacMovieIndex, state.dm2MacMovieRejected);
        M11_GameView_Shutdown(&state);
        return 1;
    }

    /* The authentic Mac input table closes credits on Return/Enter.  The
     * missing PC dismissal rectangle must not become a synthetic mouse hit. */
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED || state.dm2MacMovieActive ||
        state.dm2State.startup_credits_active) {
        fprintf(stderr, "Mac Credits.MooV did not close through Return/Enter\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }

    printf("PASS: M11 binds authentic Mac Title.MooV at startup: frame=%u\n",
           state.dm2MacMovieDecoder.frame_index);
    printf("PASS: M11 binds authentic Mac Credits.MooV and closes it with Return/Enter\n");
    M11_GameView_Shutdown(&state);
    return 0;
}
