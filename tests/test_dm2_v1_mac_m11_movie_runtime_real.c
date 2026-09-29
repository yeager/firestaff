#include "m11_game_view.h"
#include "dm2_v1_boot.h"
#include "dm2_v1_mac_sound.h"
#include "dm2_v1_mac_quicktime.h"
#include "menu_hit_m12.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL3/SDL.h>

static int explicit_no_audio_mode(void)
{
    const char* driver = getenv("SDL_AUDIODRIVER");
    return driver && strcmp(driver, "firestaff-no-such-driver") == 0;
}

static int no_audio_output(const M11_AudioState* audio)
{
    return !M11_Audio_IsAvailable(audio) && !audio->sdlStream && !audio->movieStream;
}

#define AUDIO_CHECK(condition, checkpoint) do { if (!(condition)) { \
    fprintf(stderr, "Mac movie audio checkpoint failed: %s (SDL: %s)\n", \
            checkpoint, SDL_GetError()); return 0; } } while (0)

static int dm2_version_id_is(int version_index, const char *expected)
{
    const char *version_id;
    if (version_index < 0 || !expected) return 0;
    version_id = M12_AssetStatus_GetVersionId("dm2", (size_t)version_index);
    return version_id && strcmp(version_id, expected) == 0;
}

static int check_live_movie_clock(M11_GameViewState *state, unsigned char *framebuffer)
{
    const uint64_t timeout = SDL_GetTicksNS() / UINT64_C(1000) + UINT64_C(5000000);
    const uint32_t first_frame = state->dm2MacMovieDecoder.frame_index;
    int held_frame = 0;
    /* Exercise the production monotonic clock before the later accelerated
     * traversal. No source clock or fast-forward flag is changed here. */
    while (state->dm2MacMovieActive &&
           state->dm2MacMovieDecoder.frame_index < first_frame + 8u) {
        uint32_t before = state->dm2MacMovieDecoder.frame_index;
        uint64_t now;
        M11_GameView_Draw(state, framebuffer, 320, 200);
        now = SDL_GetTicksNS() / UINT64_C(1000);
        if (!state->dm2MacMovieStartUs || now >= timeout ||
            state->dm2MacMovieDecoder.presentation_time_us > now - state->dm2MacMovieStartUs)
            return 0;
        if (state->dm2MacMovieDecoder.frame_index == before) held_frame = 1;
        SDL_Delay(1);
    }
    return state->dm2MacMovieActive && held_frame &&
           state->dm2MacMovieDecoder.frame_index >= first_frame + 8u;
}

static int check_normal_movie_eof(M11_GameViewState* state, unsigned char* framebuffer)
{
    DM2_V1_BootProfile* profile = (DM2_V1_BootProfile*)state->dm2BootProfile;
    const DM2_V1_MacMovieView* view = &profile->mac_movie_view[DM2_V1_MAC_MOVIE_CREDITS];
    DM2_V1_MacQuickTimeInfo info;
    uint64_t deadline;
    uint32_t guard;
    const int no_audio = explicit_no_audio_mode();
    if (!dm2_v1_mac_quicktime_inspect(view->bytes, view->size, &info) ||
        info.video_sample_count < 11u) return 0;
    state->bootProbeFastForward = 1;
    guard = info.video_sample_count + 1u;
    while (state->dm2MacMovieActive &&
           state->dm2MacMovieDecoder.frame_index < info.video_sample_count - 10u && guard-- > 0u)
        M11_GameView_Draw(state, framebuffer, 320, 200);
    if (!state->dm2MacMovieActive || !guard) return 0;
    /* Discard accelerated backlog; retain only actual final source frames. */
    if (!M11_Audio_StopDm2MacMovie(&state->audioState) ||
        !M11_Audio_SetHostPaused(&state->audioState, 1)) return 0;
    while (state->dm2MacMovieActive &&
           state->dm2MacMovieDecoder.frame_index < info.video_sample_count && guard-- > 0u)
        M11_GameView_Draw(state, framebuffer, 320, 200);
    if (!state->dm2MacMovieActive || !guard) return 0;
    state->bootProbeFastForward = 0;
    state->dm2MacMovieStartUs = SDL_GetTicksNS() / UINT64_C(1000) -
        state->dm2MacMovieDecoder.presentation_time_us;
    AUDIO_CHECK(no_audio ? no_audio_output(&state->audioState) :
        (state->audioState.movieStream &&
         SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.movieStream) > 0),
        "EOF authentic tail transport");
    M11_GameView_Draw(state, framebuffer, 320, 200);
    if (!state->dm2MacMovieActive) return 0;
    SDL_Delay((Uint32)(state->dm2MacMovieDecoder.frame_duration_us / 1000u + 2u));
    M11_GameView_Draw(state, framebuffer, 320, 200);
    AUDIO_CHECK(state->audioState.hostPaused &&
        (no_audio ? !state->dm2MacMovieActive : state->dm2MacMovieActive),
        "EOF source deadline versus paused output drain");
    if (!M11_Audio_SetHostPaused(&state->audioState, 0)) return 0;
    deadline = SDL_GetTicksNS() / UINT64_C(1000) + UINT64_C(3000000);
    while (state->dm2MacMovieActive && SDL_GetTicksNS() / UINT64_C(1000) < deadline) {
        M11_GameView_Draw(state, framebuffer, 320, 200);
        SDL_Delay(2U);
    }
    if (state->dm2MacMovieActive || !state->dm2MacMovieComplete ||
        (no_audio ? !no_audio_output(&state->audioState) :
         SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.movieStream) != 0)) return 0;
    puts(no_audio ? "PASS: no-device authentic Mac movie finishes at its source deadline" :
         "PASS: authentic Mac movie final frame and PCM drain finish in normal time");
    return 1;
}

static int check_credits_audio_cancel(M11_GameViewState* state,
                                      unsigned char* framebuffer)
{
    DM2_V1_BootProfile* profile = (DM2_V1_BootProfile*)state->dm2BootProfile;
    DM2_V1_MacSoundSample sample;
    unsigned int hash = 2166136261u;
    size_t i;
    int before = 0;
    const int no_audio = explicit_no_audio_mode();
    const uint32_t first_frame = state->dm2MacMovieDecoder.frame_index;
    AUDIO_CHECK(profile && (no_audio ? no_audio_output(&state->audioState) :
        M11_Audio_IsAvailable(&state->audioState)), "credits output mode");
    AUDIO_CHECK(M11_Audio_SetHostPaused(&state->audioState, 1), "credits host pause");
    /* Queue isolation needs an audible domain irrespective of saved preferences. */
    AUDIO_CHECK(M11_Audio_SetVolumes(&state->audioState, 128, 128, 128, 128),
        "audible isolation volumes");
    /* Keep both real streams paused at SDL only; the movie still decodes
     * through normal Draw, so Return exercises actual queued credits PCM. */
    state->bootProbeFastForward = 1;
    for (i = 0; i < 8u && (no_audio ||
             state->dm2MacMovieDecoder.frame_index <= first_frame ||
             !state->audioState.movieStream ||
             SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.movieStream) <= 0); ++i)
        M11_GameView_Draw(state, framebuffer, 320, 200);
    state->bootProbeFastForward = 0;
    fprintf(stderr, "Credits PCM checkpoint: first=%u frame=%u draws=%zu samples=%d "
        "queued=%d active=%d backend=%d master=%d hostPaused=%d resume=%d\n",
        (unsigned)first_frame, (unsigned)state->dm2MacMovieDecoder.frame_index, i,
        state->audioState.dm2MacMoviePcm.sampleCount,
        state->audioState.movieStream ?
            SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.movieStream) : -1,
        state->dm2MacMovieActive, (int)state->audioState.backend,
        state->audioState.masterVolume, state->audioState.hostPaused,
        state->audioState.hostResumeMovieStream);
    AUDIO_CHECK(state->dm2MacMovieActive &&
        state->dm2MacMovieDecoder.frame_index > first_frame &&
        state->audioState.dm2MacMoviePcm.sampleCount > 0 &&
        (no_audio ? no_audio_output(&state->audioState) :
         (state->audioState.movieStream &&
          SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.movieStream) > 0)),
        "credits authentic PCM decoded and transported");
    AUDIO_CHECK(dm2_v1_mac_sound_find(profile->mac_sound_resource_fork[DM2_V1_MAC_SOUND_GENERAL],
            profile->mac_sound_resource_fork_size[DM2_V1_MAC_SOUND_GENERAL], 10001, &sample) == 0 &&
        sample.valid && sample.sample_data_size, "original snd resource lookup");
    for (i = 0; i < sample.sample_data_size; ++i) {
        hash ^= sample.sample_data[i];
        hash *= 16777619u;
    }
    AUDIO_CHECK(M11_Audio_PlayDm2MacSndPcm(&state->audioState,
            (const int8_t*)sample.sample_data, (int)sample.sample_data_size,
            (int)((sample.sample_rate_fixed + 0x8000u) >> 16), sample.resource_id,
            hash ? hash : 1u), "original snd playback receipt");
    AUDIO_CHECK(state->audioState.dm2MacSndAccepted &&
        state->audioState.dm2MacSndPcm.sampleCount > 0, "authentic snd resource accepted");
    if (!no_audio) {
        before = SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.sdlStream);
        AUDIO_CHECK(before > 0, "authentic SFX queue populated");
    }
    AUDIO_CHECK(M11_GameView_HandleInput(state, M12_MENU_INPUT_ACCEPT) !=
        M11_GAME_INPUT_IGNORED && !state->dm2MacMovieActive &&
        !state->dm2State.startup_credits_active && !state->audioState.hostResumeMovieStream,
        "Return cancels credits source and pending resume");
    AUDIO_CHECK(no_audio ? no_audio_output(&state->audioState) :
        (SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.movieStream) == 0 &&
         SDL_AudioStreamDevicePaused((SDL_AudioStream*)state->audioState.movieStream) &&
         SDL_GetAudioStreamQueued((SDL_AudioStream*)state->audioState.sdlStream) == before),
        "cancel clears movie only");
    AUDIO_CHECK(M11_Audio_SetHostPaused(&state->audioState, 0) &&
        (no_audio ? no_audio_output(&state->audioState) :
         SDL_AudioStreamDevicePaused((SDL_AudioStream*)state->audioState.movieStream)),
        "host resume cannot restart cancelled movie");
    puts(no_audio ? "PASS: no-device authentic Credits decoding and cancellation" :
         "PASS: authentic Credits cancellation clears movie PCM and preserves queued original snd resource");
    return 1;
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
    if (!check_live_movie_clock(&state, framebuffer)) {
        fprintf(stderr, "Mac title failed live source-deadline pacing\n");
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    puts("PASS: Mac title holds source deadlines and advances under the live clock");
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
            (state.audioState.movieStream &&
             !SDL_AudioStreamDevicePaused((SDL_AudioStream*)state.audioState.movieStream))) {
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
            (state.audioState.movieStream &&
             !SDL_AudioStreamDevicePaused((SDL_AudioStream*)state.audioState.movieStream))) {
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
    state.bootProbeFastForward = 1;
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

    state.bootProbeFastForward = 0;
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

    if (!check_credits_audio_cancel(&state, framebuffer)) {
        fprintf(stderr, "Mac credits audio cancellation did not isolate authentic film/SFX queues\n");
        M11_GameView_Shutdown(&state);
        M12_StartupMenu_Destroy(&menuState);
        return 1;
    }
    /* Reopen the same real credits movie: its previous queue must be empty. */
    if (M11_GameView_HandlePointer(&state, aux.show_credits.x + aux.show_credits.w / 2,
            aux.show_credits.y + aux.show_credits.h / 2, 1) == M11_GAME_INPUT_IGNORED ||
        !state.dm2MacMovieActive ||
        (explicit_no_audio_mode() ? !no_audio_output(&state.audioState) :
         (!state.audioState.movieStream ||
          SDL_GetAudioStreamQueued((SDL_AudioStream*)state.audioState.movieStream) != 0))) {
        fprintf(stderr, "Mac credits reopen retained prior movie PCM\n");
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

    if (M11_GameView_HandlePointer(&state, aux.show_credits.x + aux.show_credits.w / 2,
            aux.show_credits.y + aux.show_credits.h / 2, 1) == M11_GAME_INPUT_IGNORED ||
        !check_normal_movie_eof(&state, framebuffer)) {
        fprintf(stderr, "Mac normal movie EOF/drain lifecycle failed\n");
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
