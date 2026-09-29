#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "menu_hit_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

static int dm2_version_id_is(int version_index, const char *expected)
{
    const char *version_id;
    if (version_index < 0 || !expected) return 0;
    version_id = M12_AssetStatus_GetVersionId("dm2", (size_t)version_index);
    return version_id && strcmp(version_id, expected) == 0;
}

int main(void)
{
    M11_GameViewState state;
    M12_StartupMenuState menuState;
    M12_StartupMenuInitOptions menuOptions;
    M12_LaunchIntent launchIntent;
    M11_BootProbeReceipt runtimeReceipt;
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
    /* Start through the real M12 card flow. The historical test built an
     * M11 spec directly, which skipped retail archive admission, platform
     * selection and the launch gate that users reach before this Mac movie. */
    memset(&menuOptions, 0, sizeof(menuOptions));
    menuOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menuState, zip, "dm2", &menuOptions);
    if (menuState.view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(&menuState, M12_MENU_INPUT_ACCEPT);
    }
    if (menuState.view != M12_MENU_VIEW_MAIN ||
        !M12_AssetStatus_GameAvailable(&menuState.assetStatus, "dm2") ||
        !M12_ModernMenu_HandlePointer(&menuState, 1645, 262, 1, NULL) ||
        menuState.view != M12_MENU_VIEW_GAME_OPTIONS ||
        menuState.gameCardFlowStage != 0 ||
        !M12_ModernMenu_HandlePointer(&menuState, 410, 679, 1, NULL) ||
        menuState.gameCardFlowStage != 1 ||
        menuState.gameOptions[2].architectureIndex != M12_ARCH_MAC ||
        !dm2_version_id_is(menuState.gameOptions[2].versionIndex,
                           "mac-en-retail") ||
        !M12_ModernMenu_HandlePointer(&menuState, 1458, 405, 1, NULL) ||
        menuState.gameCardFlowStage != 2 ||
        !M12_ModernMenu_HandlePointer(&menuState, 960, 919, 1, NULL)) {
        fprintf(stderr,
                "M12 did not admit and select the Mac retail launch path: "
                "view=%d stage=%d architecture=%d version=%d launch=%d\n",
                menuState.view, menuState.gameCardFlowStage,
                menuState.gameOptions[2].architectureIndex,
                menuState.gameOptions[2].versionIndex,
                menuState.launchRequested);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    launchIntent = M12_StartupMenu_GetLaunchIntent(&menuState);
    if (!menuState.launchRequested || !launchIntent.valid ||
        !launchIntent.gameId || strcmp(launchIntent.gameId, "dm2") != 0 ||
        !launchIntent.versionId || strcmp(launchIntent.versionId,
                                           "mac-en-retail") != 0 ||
        launchIntent.options.architectureIndex != M12_ARCH_MAC) {
        fprintf(stderr,
                "M12 Mac launch intent was not valid: launch=%d valid=%d "
                "game=%s version=%s architecture=%d\n",
                menuState.launchRequested, launchIntent.valid,
                launchIntent.gameId ? launchIntent.gameId : "(null)",
                launchIntent.versionId ? launchIntent.versionId : "(null)",
                launchIntent.options.architectureIndex);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    M11_GameView_Init(&state);
    if (!M11_GameView_OpenSelectedMenuEntry(&state, &menuState) ||
        state.sourceKind != M11_GAME_SOURCE_DM2_BOOT ||
        !state.dm2BootProfile || !state.dm2MacMovieActive ||
        !state.dm2MacMovieDecoder.frame_ready ||
        state.dm2MacMovieDecoder.frame_index != 1u) {
        fprintf(stderr, "Mac M11 movie runtime was not bound: start=%d source=%d active=%d rejected=%d\n",
                state.active, state.sourceKind, state.dm2MacMovieActive,
                state.dm2MacMovieRejected);
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    puts("PASS: M12 game card -> Mac platform -> Custom options -> verified launch intent");

    if (M11_GameView_AdvanceIdleTick(&state) != M11_GAME_INPUT_REDRAW ||
        !state.dm2MacMovieActive) {
        fprintf(stderr,
                "M11 Mac title movie did not request a presented idle frame: active=%d\n",
                state.dm2MacMovieActive);
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }

    memset(framebuffer, 0, sizeof(framebuffer));
    /* M11_GameView_Draw uses the host monotonic clock to honour each
     * authentic QuickTime frame duration.  A tight headless loop otherwise
     * redraws frame 1 thousands of times without advancing the source
     * movie, making the following menu click occur before the Mac title
     * event loop has returned. */
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    {
        uint32_t pausedFrameIndex = state.dm2MacMovieDecoder.frame_index;
        uint64_t originalStartUs = state.dm2MacMovieStartUs;
        SessionTimerRuntime_Init(&state.sessionTimerRuntime, 15);
        (void)M11_GameView_TickSessionTimerMs(&state, 900000);
        state.bootProbeFastForward = 1;
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        state.bootProbeFastForward = 0;
        if (!state.sessionTimerForcedPauseDialogActive ||
            state.dm2MacMovieDecoder.frame_index != pausedFrameIndex ||
            !state.audioState.hostPaused ||
            (state.audioState.sdlStream &&
             !SDL_AudioStreamDevicePaused((SDL_AudioStream*)state.audioState.sdlStream))) {
            fprintf(stderr, "Timer pause did not freeze authentic Mac movie/audio\n");
            M11_GameView_Shutdown(&state);
            M12_StartupMenu_Destroy(&menuState);
            return 1;
        }
        SDL_Delay(2);
        M11_GameView_ClearSessionTimerForcedPause(&state);
        if (state.dm2MacMovieStartUs <= originalStartUs || state.audioState.hostPaused) {
            fprintf(stderr, "Timer resume did not rebase Mac movie clock/release audio\n");
            M11_GameView_Shutdown(&state);
            M12_StartupMenu_Destroy(&menuState);
            return 1;
        }
        SessionTimerRuntime_Init(&state.sessionTimerRuntime, 0);
        puts("PASS: timer pause freezes authentic Mac movie/audio and rebases resume");
    }
    {
        uint32_t pausedFrameIndex = state.dm2MacMovieDecoder.frame_index;
        uint64_t originalStartUs = state.dm2MacMovieStartUs;
        int queuedSamples = state.audioState.queuedSampleCount;
        M11_GameView_SetPauseReason(&state, M11_GAME_PAUSE_REASON_FOCUS, 1);
        M11_GameView_SetPauseReason(&state, M11_GAME_PAUSE_REASON_FOCUS, 1);
        state.bootProbeFastForward = 1;
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        state.bootProbeFastForward = 0;
        if (!M11_GameView_IsPaused(&state) ||
            state.dm2MacMovieDecoder.frame_index != pausedFrameIndex ||
            state.dm2MacMovieStartUs != originalStartUs ||
            state.audioState.queuedSampleCount != queuedSamples ||
            !state.audioState.hostPaused ||
            (state.audioState.sdlStream &&
             !SDL_AudioStreamDevicePaused((SDL_AudioStream*)state.audioState.sdlStream))) {
            fprintf(stderr, "Focus pause did not freeze authentic Mac movie decoding/audio\n");
            M11_GameView_Shutdown(&state);
            M12_StartupMenu_Destroy(&menuState);
            return 1;
        }
        SDL_Delay(2U);
        M11_GameView_SetPauseReason(&state, M11_GAME_PAUSE_REASON_FOCUS, 0);
        if (M11_GameView_IsPaused(&state) || state.audioState.hostPaused ||
            state.dm2MacMovieStartUs <= originalStartUs ||
            state.dm2MacMovieDecoder.frame_index != pausedFrameIndex) {
            fprintf(stderr, "Focus resume did not rebase Mac movie clock without decoding ahead\n");
            M11_GameView_Shutdown(&state);
            M12_StartupMenu_Destroy(&menuState);
            return 1;
        }
        /* The existing original-frame clock step below checks actual resume;
         * no substitute video/audio samples are introduced. */
        state.dm2MacMovieStartUs = SDL_GetTicksNS() / UINT64_C(1000) -
            state.dm2MacMovieDecoder.presentation_time_us -
            state.dm2MacMovieDecoder.frame_duration_us - 1u;
        M11_GameView_Draw(&state, framebuffer, 320, 200);
        if (state.dm2MacMovieDecoder.frame_index <= pausedFrameIndex) {
            fprintf(stderr, "Authentic Mac movie did not decode the next frame after focus resume\n");
            M11_GameView_Shutdown(&state);
            M12_StartupMenu_Destroy(&menuState);
            return 1;
        }
        puts("PASS: focus pause freezes authentic Mac movie/audio and resumes source decoding");
    }
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
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }

    /* The authentic Mac input table closes credits on Return/Enter.  The
     * missing PC dismissal rectangle must not become a synthetic mouse hit. */
    if (M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED || state.dm2MacMovieActive ||
        state.dm2State.startup_credits_active) {
        fprintf(stderr, "Mac Credits.MooV did not close through Return/Enter\n");
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }

    memset(&menu, 0, sizeof(menu));
    if (!dm2_v1_boot_startup_menu_pointer_layout(
            (DM2_V1_BootProfile *)state.dm2BootProfile, &menu) ||
        !menu.valid ||
        M11_GameView_HandleInput(&state, M12_MENU_INPUT_ACCEPT) ==
            M11_GAME_INPUT_IGNORED || !state.dm2State.startup_menu_active ||
        !dm2_v1_boot_prepared_new_game_world_readonly(
            (DM2_V1_BootProfile *)state.dm2BootProfile) ||
        /* The source GAME_LOAD path first publishes a private mirror
         * selection view. A click in its original 224x136 viewport is what
         * selects the actual DB3 mirror and permits the session handoff. */
        M11_GameView_HandlePointer(&state, 112, 100, 1) ==
            M11_GAME_INPUT_IGNORED || state.dm2State.startup_menu_active ||
        !state.dm2State.level_loaded ||
        !((DM2_V1_BootProfile *)state.dm2BootProfile)
             ->source_game_load_session_ready) {
        fprintf(stderr, "Mac New Game did not publish authentic STARTEND\n");
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    puts("PASS: M11 publishes authentic Mac New Game STARTEND session");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&state, framebuffer, 320, 200);
    memset(&runtimeReceipt, 0, sizeof(runtimeReceipt));
    if (!M11_GameView_GetBootProbeReceipt(&state, &runtimeReceipt) ||
        !runtimeReceipt.dm2RuntimeFrameAccepted ||
        !runtimeReceipt.dm2RuntimeRealAssetsReady ||
        !runtimeReceipt.dm2RuntimeNoCoreFallbacks ||
        runtimeReceipt.dm2RuntimeFallbackDrawCount != 0) {
        fprintf(stderr,
                "Mac M11 did not present a source-owned gameplay frame "
                "(accepted=%d real=%d noFallbacks=%d fallbackDraws=%d)\n",
                runtimeReceipt.dm2RuntimeFrameAccepted,
                runtimeReceipt.dm2RuntimeRealAssetsReady,
                runtimeReceipt.dm2RuntimeNoCoreFallbacks,
                runtimeReceipt.dm2RuntimeFallbackDrawCount);
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    puts("PASS: M12 Mac launch reaches an admitted source-owned gameplay frame");

    printf("PASS: M11 binds authentic Mac Title.MooV at startup: frame=%u\n",
           state.dm2MacMovieDecoder.frame_index);
    printf("PASS: M11 binds authentic Mac Credits.MooV and closes it with Return/Enter\n");
    M11_GameView_Shutdown(&state);
    M12_StartupMenu_Destroy(&menuState);
    return 0;
}
