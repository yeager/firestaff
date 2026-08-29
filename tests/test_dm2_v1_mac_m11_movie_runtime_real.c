#include "m11_game_view.h"
#include "dm2_v1_boot.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

int main(void)
{
    M11_GameViewState state;
    M11_GameLaunchSpec spec;
    const char *zip = getenv("FIRESTAFF_DM2_MAC_EN_ZIP");
    unsigned char framebuffer[320u * 200u];
    DM2_V1_StartupMenuAuxPointerLayout aux;
    DM2_V1_StartupMenuPointerLayout menu;
    int frame;

    if (!zip || !zip[0]) {
        puts("SKIP: DM2 Mac ZIP environment is not set");
        return 77;
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
    /* M11_GameView_Draw uses the host monotonic clock to honour each
     * authentic QuickTime frame duration.  A tight headless loop otherwise
     * redraws frame 1 thousands of times without advancing the source
     * movie, making the following menu click occur before the Mac title
     * event loop has returned. */
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    for (frame = 0; state.dm2MacMovieActive && frame < 10000; ++frame) {
        /* Advance the test clock by one source frame.  This keeps the
         * production path wall-clock based while avoiding a multi-second
         * wait for the complete retail title movie in CI. */
        state.dm2MacMovieStartUs =
            SDL_GetTicksNS() / UINT64_C(1000) -
            state.dm2MacMovieDecoder.presentation_time_us -
            state.dm2MacMovieDecoder.frame_duration_us - 1u;
        M11_GameView_Draw(&state, framebuffer, 320, 200);
    }

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

    memset(&menu, 0, sizeof(menu));
    if (!dm2_v1_boot_startup_menu_pointer_layout(
            (DM2_V1_BootProfile *)state.dm2BootProfile, &menu) ||
        !menu.valid ||
        M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED || state.dm2State.startup_menu_active ||
        !state.dm2State.level_loaded ||
        !((DM2_V1_BootProfile *)state.dm2BootProfile)
             ->source_game_load_session_ready) {
        fprintf(stderr, "Mac New Game did not publish authentic STARTEND\n");
        M11_GameView_Shutdown(&state);
        return 1;
    }
    puts("PASS: M11 publishes authentic Mac New Game STARTEND session");

    printf("PASS: M11 binds authentic Mac Title.MooV at startup: frame=%u\n",
           state.dm2MacMovieDecoder.frame_index);
    printf("PASS: M11 binds authentic Mac Credits.MooV and closes it with Return/Enter\n");
    M11_GameView_Shutdown(&state);
    return 0;
}
