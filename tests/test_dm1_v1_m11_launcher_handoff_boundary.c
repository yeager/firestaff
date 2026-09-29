#include "m11_qol_runtime.h"
#include "main_loop_m11.h"
/*
 * test_dm1_v1_m11_launcher_handoff_boundary.c
 *
 * M12 -> M11 DM1 V1 normal launcher handoff boundary.
 *
 * The source-order gates prove the ReDMCSB SWSH/TITLE/entrance sequence.
 * This focused C gate proves the production selected-menu path for a plain
 * DM1 start: M12_StartupMenu_GetLaunchIntent() ->
 * M11_GameView_OpenSelectedMenuEntry().
 *
 * Source-lock: ReDMCSB APPA.C FTL_SWSH -> FTL_TITL, TITLE.C F0437,
 * and ENTRANCE.C F0441. A launcher/CLI DM1 start must remain classified as
 * source-visible startup, while direct M11 test/dev starts are the explicit
 * game-view intro bypass.
 *
 * Skip-clean without user-supplied DM1 data. With ~/.firestaff/data staged,
 * this becomes a real launcher-to-DM1 handoff proof.
 */

#if defined(__APPLE__) && !defined(_DARWIN_C_SOURCE)
#define _DARWIN_C_SOURCE 1
#endif
#ifndef _POSIX_C_SOURCE
#define _POSIX_C_SOURCE 200809L
#endif

#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_PATH_SEP "\\"
#define TEST_GETPID() _getpid()
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_PATH_SEP "/"
#define TEST_GETPID() getpid()
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures = 0;
static int g_passed = 0;
static int g_skipped = 0;

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    } else {
        ++g_passed;
    }
}

static void expect_skip(const char* message) {
    fprintf(stderr, "SKIP: %s\n", message);
    ++g_skipped;
}

static void init_menu_without_gallery(M12_StartupMenuState* state,
                                      const char* data_dir,
                                      const char* game_id) {
    M12_StartupMenuInitOptions options;
    memset(&options, 0, sizeof(options));
    options.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(state, data_dir, game_id, &options);
}

static void dismiss_initial_message(M12_StartupMenuState* state) {
    if (state && state->view == M12_MENU_VIEW_MESSAGE) {
        M12_StartupMenu_HandleInput(state, M12_MENU_INPUT_ACCEPT);
    }
}

static int count_nonzero_pixels(const unsigned char* pixels, size_t count) {
    size_t i;
    int nonzero = 0;
    if (!pixels) {
        return 0;
    }
    for (i = 0; i < count; ++i) {
        if (pixels[i] != 0u) {
            ++nonzero;
        }
    }
    return nonzero;
}

static int message_area_is_black(const unsigned char* framebuffer,
                                 int framebuffer_width,
                                 int framebuffer_height) {
    int x;
    int y;

    if (!framebuffer || framebuffer_width < 320 || framebuffer_height < 200) {
        return 0;
    }
    /* ReDMCSB TEXT.C's PC message surface is C015 at y=173..199.  Until
     * a decoded TEXT.C producer is wired, generic M11 telemetry must not
     * become visible game text in that source-owned rectangle. */
    for (y = 173; y < 200; ++y) {
        for (x = 0; x < 320; ++x) {
            if (framebuffer[y * framebuffer_width + x] != 0u) {
                return 0;
            }
        }
    }
    return 1;
}

static int message_area_has_source_pixels(const unsigned char* framebuffer,
                                          int framebuffer_width,
                                          int framebuffer_height) {
    int x;
    int y;

    if (!framebuffer || framebuffer_width < 320 || framebuffer_height < 200) {
        return 0;
    }
    for (y = 173; y < 200; ++y) {
        for (x = 0; x < 320; ++x) {
            if (framebuffer[y * framebuffer_width + x] != 0u) {
                return 1;
            }
        }
    }
    return 0;
}

static void make_empty_data_dir(char out[512]) {
    int rc = snprintf(out, 512,
                      "%s%sfirestaff_dm1_launcher_empty_%ld",
                      (getenv("TMPDIR") ? getenv("TMPDIR") : "/tmp"),
                      TEST_PATH_SEP, (long)TEST_GETPID());
    if (rc > 0 && rc < 512) {
        (void)TEST_MKDIR(out);
    } else {
        out[0] = '\0';
    }
}

static const char* default_data_root(char fallback[512]) {
    const char* configured = getenv("FIRESTAFF_DATA");
    const char* home = getenv("HOME");
    if (configured && configured[0]) {
        return configured;
    }
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data", home);
    return fallback;
}

static int mode_default_resolution(int mode) {
    if (mode == M12_PRESENTATION_V22_MODERN) {
        return M12_RES_2560x1440;
    }
    if (mode == M12_PRESENTATION_V21_UPSCALED ||
        mode == M12_PRESENTATION_V20_FILTERED) {
        return M12_RES_640x400;
    }
    return M12_RES_320x200;
}

static const char* mode_label(int mode) {
    switch (mode) {
        case M12_PRESENTATION_V1_ORIGINAL: return "DM1 V1 original";
        case M12_PRESENTATION_V20_FILTERED: return "DM1 V2.0 filtered";
        case M12_PRESENTATION_V21_UPSCALED: return "DM1 V2.1 upscaled";
        case M12_PRESENTATION_V22_MODERN: return "DM1 V2.2 modern";
        default: return "DM1 unknown presentation";
    }
}

static void expect_mode_true(int condition, int mode, const char* suffix) {
    char message[192];
    snprintf(message, sizeof(message), "%s %s", mode_label(mode), suffix);
    expect_true(condition, message);
}

/* Optional desktop-only evidence: use the window manager's real focus state,
 * never injected focus events. The ordinary headless gate remains unchanged. */
static int native_focus_wait(SDL_Window* window, int focused) {
    Uint64 deadline = SDL_GetTicks() + 2000U;
    do {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            if (event.type == SDL_EVENT_QUIT) return 0;
        }
        if (((SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0) == focused)
            return 1;
        SDL_Delay(10U);
    } while (SDL_GetTicks() < deadline);
    return 0;
}

static void run_native_focus_probe(M11_GameViewState* view,
                                     const M12_StartupMenuState* menu, int mode) {
    SDL_Window* window = NULL;
    const char* driver;
    void* world_copy = NULL;
    int before;
    unsigned int phase;
    int focused;
    int video_initialized = 0;
    if (!getenv("FIRESTAFF_NATIVE_FOCUS_PROBE") ||
        mode != M12_PRESENTATION_V1_ORIGINAL) return;
    if (!view->active) {
        expect_true(0, "native focus probe unavailable: original-media session not active");
        return;
    }
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO)) {
        fprintf(stderr, "Native focus video unavailable: %s\n", SDL_GetError());
        expect_true(0, "native focus probe requires a desktop video driver");
        return;
    }
    video_initialized = 1;
    driver = SDL_GetCurrentVideoDriver();
    if (!driver || strcmp(driver, "dummy") == 0 || strcmp(driver, "offscreen") == 0) {
        expect_true(0, "native focus probe unavailable with headless video");
        goto cleanup;
    }
    window = SDL_CreateWindow("Firestaff original-media focus probe", 640, 400,
                               SDL_WINDOW_HIDDEN | SDL_WINDOW_HIGH_PIXEL_DENSITY);
    if (!window) {
        fprintf(stderr, "Native focus window unavailable: %s\n", SDL_GetError());
        expect_true(0, "native focus probe creates a real desktop window");
        goto cleanup;
    }
    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);
    focused = native_focus_wait(window, 1);
    expect_true(focused, "native focus probe obtains actual SDL input focus within two seconds");
    if (!focused) goto cleanup;
    world_copy = malloc(sizeof(view->world));
    if (!world_copy) {
        expect_true(0, "native focus probe allocates original world snapshot");
        goto cleanup;
    }
    memcpy(world_copy, &view->world, sizeof(view->world));
    phase = view->v1FoodVblankPhase;
    before = SessionTimerRuntime_RemainingSeconds(&view->sessionTimerRuntime);
    (void)M11_GameView_TickSessionTimerMs(view, 500U);
    SDL_HideWindow(window);
    focused = native_focus_wait(window, 0);
    expect_true(focused, "native window hide causes actual SDL input focus loss");
    if (!focused) goto cleanup;
    M11_GameView_SetPauseReason(view, M11_GAME_PAUSE_REASON_FOCUS,
        M11_FocusPauseRequired(1,
            (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0, driver));
    expect_true(M11_GameView_IsPaused(view) && view->audioState.hostPaused,
                "actual native focus loss pauses original-media runtime and audio");
    (void)M11_GameView_AdvanceIdleTick(view);
    (void)M11_GameView_AdvanceFoodClockMs(view, 1000U);
    (void)M11_GameView_TickSessionTimerMs(view, 5000U);
    expect_true(memcmp(world_copy, &view->world, sizeof(view->world)) == 0 &&
                    phase == view->v1FoodVblankPhase &&
                    before == SessionTimerRuntime_RemainingSeconds(&view->sessionTimerRuntime) &&
                    view->sessionTimerRemainderMs == 500U,
                "native focus pause freezes original world, food clock and fractional timer");
    SDL_ShowWindow(window);
    SDL_RaiseWindow(window);
    focused = native_focus_wait(window, 1);
    expect_true(focused, "native window regains actual SDL input focus within two seconds");
    if (!focused) goto cleanup;
    M11_GameView_SetPauseReason(view, M11_GAME_PAUSE_REASON_FOCUS,
        M11_FocusPauseRequired(1,
            (SDL_GetWindowFlags(window) & SDL_WINDOW_INPUT_FOCUS) != 0, driver));
    expect_true(!M11_GameView_IsPaused(view) && !view->audioState.hostPaused &&
                    memcmp(world_copy, &view->world, sizeof(view->world)) == 0,
                "native focus return resumes audio without changing original world data");
    (void)M11_GameView_TickSessionTimerMs(view, 500U);
    expect_true(SessionTimerRuntime_RemainingSeconds(&view->sessionTimerRuntime) == before - 1 &&
                    view->sessionTimerRemainderMs == 0U,
                "native focus return resumes the retained fractional timer");
cleanup:
    M11_GameView_SetPauseReason(view, M11_GAME_PAUSE_REASON_FOCUS, 0);
    M11_GameView_InitFromMenuSessionTimer(view, menu);
    free(world_copy);
    if (window) SDL_DestroyWindow(window);
    if (video_initialized) SDL_QuitSubSystem(SDL_INIT_VIDEO);
}

/* Use the SONG.DAT bound by the authenticated selected installation. */
static void run_original_music_transport_probe(M11_GameViewState* view, int mode) {
    M11_AudioState* audio = &view->audioState;
    int master, sfx, music, ui;
    int effectBytes;
    int songBytes;
    expect_mode_true(audio->originalSongAvailable && audio->titleMusic.sampleCount > 0,
                     mode, "selected original SONG.DAT supplies the music transport test");
    if (!audio->originalSongAvailable || audio->titleMusic.sampleCount <= 0) return;
    (void)M11_Audio_GetVolumes(audio, &master, &sfx, &music, &ui);
    (void)M11_Audio_SetHostPaused(audio, 1);
    (void)M11_Audio_SetVolumes(audio, master, 100, music, ui);
    expect_mode_true(M11_Audio_EmitSourceSoundIndex(audio, 0), mode,
                     "authenticated SND3 effect supplies a nonempty independent queue");
    effectBytes = audio->sdlStream
        ? SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->sdlStream) : -1;
    (void)M11_Audio_SetTitleMusicEnabled(audio, 1);
    expect_mode_true(M11_Audio_PlayTitleMusic(audio) && audio->musicStream &&
                         audio->musicStream != audio->sdlStream, mode,
                     "authentic song uses a dedicated music stream");
    if (!audio->musicStream) {
        (void)M11_Audio_SetHostPaused(audio, 0);
        return;
    }
    songBytes = audio->titleMusic.sampleCount * (int)sizeof(float);
    expect_mode_true(SDL_AudioStreamDevicePaused((SDL_AudioStream*)audio->musicStream) &&
                         SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == songBytes,
                     mode, "starting music during host pause preserves a single authentic sequence");
    (void)M11_Audio_PlayTitleMusic(audio);
    expect_mode_true(SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == songBytes,
                     mode, "new music request replaces the queued song instead of appending it");
    (void)M11_Audio_SetVolumes(audio, 64, sfx, 32, ui);
    {
        float gain = SDL_GetAudioStreamGain((SDL_AudioStream*)audio->musicStream);
        expect_mode_true(gain > 0.124f && gain < 0.126f, mode,
                         "live master and music volume update already queued music");
    }
    (void)M11_Audio_RequestSourceMusicTrack(audio, 0);
    expect_mode_true(SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == 0 &&
                         M11_Audio_TitleMusicEnabled(audio), mode,
                     "source track zero stops music without changing user preference");
    (void)M11_Audio_RequestSourceMusicTrack(audio, 1);
    expect_mode_true(SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == songBytes,
                     mode, "positive source track request queues authentic song after stop");
    (void)M11_Audio_SetTitleMusicEnabled(audio, 0);
    expect_mode_true(SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == 0 &&
                         SDL_AudioStreamDevicePaused((SDL_AudioStream*)audio->musicStream) &&
                         SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->sdlStream) == effectBytes,
                     mode, "music off removes queued music and preserves the effects queue");
    (void)M11_Audio_SetHostPaused(audio, 0);
    expect_mode_true(SDL_AudioStreamDevicePaused((SDL_AudioStream*)audio->musicStream), mode,
                     "host resume cannot restart source-stopped music");
    (void)M11_Audio_SetTitleMusicEnabled(audio, 1);
    expect_mode_true(SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == 0,
                     mode, "enabling music does not resurrect an old queued track");
    expect_mode_true(M11_Audio_PlayTitleMusic(audio) &&
                         !SDL_AudioStreamDevicePaused((SDL_AudioStream*)audio->musicStream),
                     mode, "new music request starts the dedicated playback device");
    (void)M11_Audio_SetHostPaused(audio, 1);
    songBytes = SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream);
    SDL_Delay(10U);
    expect_mode_true(songBytes > 0 &&
                         SDL_GetAudioStreamQueued((SDL_AudioStream*)audio->musicStream) == songBytes,
                     mode, "host pause retains active authentic music PCM without consumption");
    (void)M11_Audio_SetHostPaused(audio, 0);
    expect_mode_true(!SDL_AudioStreamDevicePaused((SDL_AudioStream*)audio->musicStream),
                     mode, "host resume restarts music that it suspended");
    (void)M11_Audio_RequestSourceMusicTrack(audio, 0);
    (void)M11_Audio_SetVolumes(audio, master, sfx, music, ui);
}

static void run_launcher_handoff_for_mode(M12_StartupMenuState* menu, int mode) {
    M12_LaunchIntent intent;
    M11_GameViewState launcher_view;
    unsigned char framebuffer[320 * 200];
    int expected_resolution = mode_default_resolution(mode);
    int speed = mode % 3;
    int cheats = mode != M12_PRESENTATION_V22_MODERN;
    const int speedMultipliers[] = { 50, 100, 150 };

    expect_mode_true(!M11_FocusPauseRequired(1, 0, "dummy") &&
                         !M11_FocusPauseRequired(0, 0, "cocoa") &&
                         !M11_FocusPauseRequired(1, 1, "cocoa") &&
                         M11_FocusPauseRequired(1, 0, "cocoa"), mode,
                     "focus policy honors the setting and excludes headless video");
    {
        int count = 0;
        SDL_AudioDeviceID* devices;
        expect_mode_true(SDL_InitSubSystem(SDL_INIT_AUDIO), mode,
                         "audio device selection can enumerate the actual SDL backend");
        devices = SDL_GetAudioPlaybackDevices(&count);
        expect_mode_true(devices && count > 0, mode,
                         "audio device selection has a real SDL device name");
        if (devices && count > 0) {
            const char* name = SDL_GetAudioDeviceName(devices[0]);
            if (name) snprintf(menu->settings.audioDeviceName,
                sizeof(menu->settings.audioDeviceName), "%s", name);
        }
        SDL_free(devices);
    }
    menu->selectedIndex = 0;
    menu->activatedIndex = 0;
    menu->launchRequested = 1;
    menu->settings.graphicsIndex = mode;
    menu->settings.sessionTimerIndex = 1;
    menu->gameOptions[0].presentationModeIndex = mode;
    menu->gameOptions[0].resolution = expected_resolution;
    menu->gameOptions[0].cheatsEnabled = cheats;
    menu->gameOptions[0].gameSpeed = speed;
    M11_QolRuntime_SetSpeedMultiplier(200);
    menu->settings.autoMapEnabled = mode % 2;
    M11_QolRuntime_SetAutoMapEnabled(!menu->settings.autoMapEnabled);
    menu->settings.minimapEnabled = mode % 2;
    menu->settings.minimapSize = 64 + mode * 32;
    menu->settings.minimapCorner = mode;
    menu->settings.combatLogEnabled = (mode + 1) % 2;
    menu->settings.combatLogMaxLines = 50 + mode * 50;
    M11_QolRuntime_SetMinimapEnabled(!menu->settings.minimapEnabled);
    M11_QolRuntime_SetMinimapLayout(256, (mode + 1) % 4);
    M11_QolRuntime_SetCombatLogEnabled(!menu->settings.combatLogEnabled);
    M11_QolRuntime_SetCombatLogMaxLines(500);

    intent = M12_StartupMenu_GetLaunchIntent(menu);
    expect_mode_true(intent.valid == 1, mode,
                     "M12 launch intent is valid with real staged data");
    expect_mode_true(intent.presentationMode == mode, mode,
                     "M12 launch intent preserves presentation mode");
    expect_mode_true(intent.options.presentationModeIndex == mode, mode,
                     "M12 launch intent preserves option presentation mode");
    expect_mode_true(intent.options.resolution == expected_resolution, mode,
                     "M12 launch intent preserves expected resolution");
    if (!intent.valid) {
        return;
    }

    M11_GameView_Init(&launcher_view);
    expect_mode_true(M11_GameView_OpenSelectedMenuEntry(&launcher_view, menu) == 1,
                     mode, "M11 opens through M12 selected-menu entry");
    run_native_focus_probe(&launcher_view, menu, mode);
    run_original_music_transport_probe(&launcher_view, mode);
    expect_mode_true(launcher_view.audioState.sdlStream &&
        strcmp(SDL_GetAudioDeviceName(SDL_GetAudioStreamDevice((SDL_AudioStream*)
            launcher_view.audioState.sdlStream)), menu->settings.audioDeviceName) == 0,
        mode, "M12 audio device name reaches the M11 effects stream");
    expect_mode_true(launcher_view.audioState.musicStream &&
        strcmp(SDL_GetAudioDeviceName(SDL_GetAudioStreamDevice((SDL_AudioStream*)
            launcher_view.audioState.musicStream)), menu->settings.audioDeviceName) == 0,
        mode, "M12 audio device name reaches the independent song stream");
    expect_mode_true(launcher_view.audioState.cddaStream &&
        strcmp(SDL_GetAudioDeviceName(SDL_GetAudioStreamDevice((SDL_AudioStream*)
            launcher_view.audioState.cddaStream)), menu->settings.audioDeviceName) == 0,
        mode, "M12 audio device name reaches the independent CDDA stream");
    expect_mode_true(M11_QolRuntime_GetSpeedMultiplier() ==
                         (cheats ? speedMultipliers[speed] : 100), mode,
                     "M11 applies selected speed and cheats gate to live timing");
    expect_mode_true(M11_QolRuntime_GetAutoMapEnabled() ==
                         menu->settings.autoMapEnabled, mode,
                     "M11 applies current automap preference to visit recording");
    expect_mode_true(M11_QolRuntime_GetMinimapEnabled() == menu->settings.minimapEnabled &&
                         M11_QolRuntime_GetMinimapSize() == menu->settings.minimapSize &&
                         M11_QolRuntime_GetMinimapCorner() == menu->settings.minimapCorner, mode,
                     "M11 applies current minimap preferences");
    expect_mode_true(M11_QolRuntime_GetCombatLogEnabled() == menu->settings.combatLogEnabled &&
                         M11_QolRuntime_GetCombatLogMaxLines() == menu->settings.combatLogMaxLines, mode,
                     "M11 applies current combat log preferences");
    {
        int frame;
        int before = SessionTimerRuntime_RemainingSeconds(&launcher_view.sessionTimerRuntime);
        for (frame = 0; frame < 125; ++frame)
            (void)M11_GameView_TickSessionTimerMs(&launcher_view, 16);
        expect_mode_true(SessionTimerRuntime_RemainingSeconds(&launcher_view.sessionTimerRuntime) == before - 2 &&
                             launcher_view.sessionTimerRemainderMs == 0, mode,
                         "125 normal 16ms frames advance session timer by two seconds");
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, 999);
        expect_mode_true(SessionTimerRuntime_RemainingSeconds(&launcher_view.sessionTimerRuntime) == before - 2, mode,
                         "session timer retains a fractional second");
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, 1);
        expect_mode_true(SessionTimerRuntime_RemainingSeconds(&launcher_view.sessionTimerRuntime) == before - 3, mode,
                         "session timer carries fractional time across frames");
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, 999);
        M11_GameView_InitFromMenuSessionTimer(&launcher_view, menu);
        expect_mode_true(launcher_view.sessionTimerRemainderMs == 0 &&
                             SessionTimerRuntime_RemainingSeconds(&launcher_view.sessionTimerRuntime) == before, mode,
                         "new session resets elapsed and fractional timer time");
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, (uint32_t)before * 1000U);
        expect_mode_true(launcher_view.sessionTimerForcedPauseDialogActive, mode,
                         "session timer reaches forced pause with original media");
        expect_mode_true(launcher_view.audioState.hostPaused &&
                             (!launcher_view.audioState.sdlStream ||
                              SDL_AudioStreamDevicePaused((SDL_AudioStream*)launcher_view.audioState.sdlStream)), mode,
                         "forced pause suspends the original-media SDL audio owner");
        {
            M11_ForcedPauseDialogLayout layout;
            M11_GameView_GetForcedPauseDialogLayout(&launcher_view, 320, 200, &layout);
            M11_GameView_Draw(&launcher_view, framebuffer, 320, 200);
            expect_mode_true(framebuffer[layout.boxY * 320 + layout.boxX] == 2, mode,
                             "forced pause dialog is drawn over original-media frame");
        }
        {
            unsigned int tick = launcher_view.world.gameTick;
            uint64_t phase = launcher_view.v1FoodVblankPhase;
            expect_mode_true(M11_GameView_AdvanceIdleTick(&launcher_view) == M11_GAME_INPUT_IGNORED &&
                                 launcher_view.world.gameTick == tick, mode,
                             "forced pause blocks source idle simulation");
            expect_mode_true(M11_GameView_AdvanceFoodClockMs(&launcher_view, 1000) == M11_GAME_INPUT_IGNORED &&
                                 launcher_view.v1FoodVblankPhase == phase, mode,
                             "forced pause freezes source food-clock phase");
            expect_mode_true(M11_GameView_HandlePointerButton(&launcher_view, 100, 100, 1) == M11_GAME_INPUT_IGNORED &&
                                 M11_GameView_HandlePointerButtonRelease(&launcher_view, 100, 100, 1) == M11_GAME_INPUT_IGNORED, mode,
                             "forced pause blocks pointer ingress behind the modal");
        }
        M11_GameView_InitFromMenuSessionTimer(&launcher_view, menu);
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, 500);
        M11_GameView_SetPauseReason(&launcher_view, M11_GAME_PAUSE_REASON_FOCUS, 1);
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, 5000);
        expect_mode_true(M11_GameView_IsPaused(&launcher_view) &&
                             launcher_view.sessionTimerRemainderMs == 500 &&
                             SessionTimerRuntime_RemainingSeconds(&launcher_view.sessionTimerRuntime) == before, mode,
                         "focus pause preserves timer seconds and fractional remainder");
        M11_GameView_SetPauseReason(&launcher_view, M11_GAME_PAUSE_REASON_FOCUS, 0);
        (void)M11_GameView_TickSessionTimerMs(&launcher_view, (uint32_t)before * 1000U);
        M11_GameView_SetPauseReason(&launcher_view, M11_GAME_PAUSE_REASON_FOCUS, 1);
        M11_GameView_SetPauseReason(&launcher_view, M11_GAME_PAUSE_REASON_FOCUS, 0);
        expect_mode_true(M11_GameView_IsPaused(&launcher_view) &&
                             launcher_view.sessionTimerForcedPauseDialogActive &&
                             launcher_view.audioState.hostPaused, mode,
                         "focus return does not release the timer pause");
        M11_GameView_SetPauseReason(&launcher_view, M11_GAME_PAUSE_REASON_FOCUS, 1);
        M11_GameView_ClearSessionTimerForcedPause(&launcher_view);
        expect_mode_true(M11_GameView_IsPaused(&launcher_view) &&
                             launcher_view.audioState.hostPaused &&
                             !launcher_view.sessionTimerForcedPauseDialogActive, mode,
                         "clearing timer pause preserves overlapping focus pause");
        M11_GameView_SetPauseReason(&launcher_view, M11_GAME_PAUSE_REASON_FOCUS, 0);
        expect_mode_true(!M11_GameView_IsPaused(&launcher_view) &&
                             !launcher_view.audioState.hostPaused, mode,
                         "last pause owner releases the original-media audio runtime");
        M11_GameView_InitFromMenuSessionTimer(&launcher_view, menu);
    }
    expect_mode_true(launcher_view.startedFromLauncher == 1, mode,
                     "M11 marks startup as launcher-started");
    expect_mode_true(launcher_view.active == 1, mode,
                     "M11 launcher handoff leaves view active");
    expect_mode_true(launcher_view.sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG,
                     mode, "M11 launcher handoff claims builtin catalog source");
    expect_mode_true(strcmp(launcher_view.sourceId, "dm1") == 0, mode,
                     "M11 launcher handoff preserves sourceId dm1");
    expect_mode_true(launcher_view.presentationMode == mode, mode,
                     "M11 launcher handoff preserves presentation mode");
    expect_mode_true(launcher_view.presentationWidth == intent.resolutionWidth,
                     mode, "M11 launcher handoff preserves presentation width");
    expect_mode_true(launcher_view.presentationHeight == intent.resolutionHeight,
                     mode, "M11 launcher handoff preserves presentation height");
    expect_mode_true(M11_GameView_Dm1StartupIntroBypassed(&launcher_view) == 0,
                     mode, "M11 launcher handoff does not mark intro bypass");
    expect_mode_true(launcher_view.assetsAvailable == 1, mode,
                     "M11 launcher handoff opens GRAPHICS.DAT assets");
    expect_mode_true(launcher_view.dungeonPath[0] != '\0', mode,
                     "M11 launcher handoff records a DUNGEON.DAT path");
    expect_mode_true(launcher_view.world.dungeon != NULL, mode,
                     "M11 launcher handoff owns a loaded dungeon model");
    expect_mode_true(launcher_view.mirrorCatalogAvailable == 1, mode,
                     "M11 launcher handoff builds the HoC mirror catalog");
    expect_mode_true(launcher_view.mirrorCatalog.count > 0, mode,
                     "M11 launcher handoff exposes HoC mirror candidates");

    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&launcher_view, framebuffer, 320, 200);
    expect_mode_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 1000,
                     mode, "M11 launcher frame draws nonblank pixels");
    M11_MessageLog_Push(&launcher_view.messageLog,
                        "READY: CLICK CENTER TO ADVANCE", 0);
    M11_GameView_Draw(&launcher_view, framebuffer, 320, 200);
    expect_mode_true(message_area_is_black(framebuffer, 320, 200), mode,
                     "M11 host telemetry cannot draw into source-owned C015");
    /* C015 (y=173..199) belongs to the DOS/Atari/Amiga PC34 TEXT.C path.
     * FM Towns uses its independently verified PIC-library menu font and
     * dynamic-menu surface, so it must not be judged by a PC34 pixel
     * rectangle merely because it was the first authentic variant found in
     * a broad data root. */
    if (!launcher_view.dm1FmtownsStartupReceiptValid) {
        dm1_v1_text_set_game_time(&launcher_view.dm1V1TextMessage,
                                  (long)launcher_view.world.gameTick);
        dm1_v1_text_print_message(&launcher_view.dm1V1TextMessage,
                                  DM1_V1_COLOR_WHITE, "SOURCE MESSAGE");
        M11_GameView_Draw(&launcher_view, framebuffer, 320, 200);
        expect_mode_true(message_area_has_source_pixels(framebuffer, 320, 200), mode,
                         "decoded TEXT.C rows draw in source-owned C015");
    } else {
        expect_mode_true(launcher_view.dm1FmtownsStartupReceiptValid == 1, mode,
                         "FM Towns uses its verified native startup route");
    }

    M11_GameView_Shutdown(&launcher_view);
}

static void run_source_order_boundary(void) {
    const char* evidence = dm1_v1_startup_sequence_source_evidence_pc34();
    expect_true(dm1_v1_startup_sequence_source_order_valid_pc34(),
                "DM1 startup source order is valid");
    expect_true(evidence && strstr(evidence, "SWSH.C") &&
                    strstr(evidence, "TITLE.C") &&
                    strstr(evidence, "ENTRANCE.C"),
                "DM1 startup source evidence names swoosh/title/entrance");
    expect_true(dm1_v1_startup_launch_path_bypasses_intro_pc34(
                    DM1_V1_STARTUP_LAUNCH_PATH_LAUNCHER_PC34) == 0,
                "DM1 launcher startup does not bypass intro");
    expect_true(dm1_v1_startup_launch_path_bypasses_intro_pc34(
                    DM1_V1_STARTUP_LAUNCH_PATH_DIRECT_GAME_VIEW_PC34) == 1,
                "DM1 direct game-view helper remains the bypass path");
}

static void run_empty_launcher_boundary(void) {
    M12_StartupMenuState menu;
    M12_LaunchIntent intent;
    const M12_MenuEntry* entry;
    char empty_dir[512];

    make_empty_data_dir(empty_dir);
    expect_true(empty_dir[0] != '\0',
                "DM1 launcher handoff empty data dir path was created");

    init_menu_without_gallery(&menu, empty_dir, "dm1");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 0);
    expect_true(entry != NULL,
                "M12 exposes a DM1 menu entry at game slot 0");
    expect_true(entry && entry->gameId && strcmp(entry->gameId, "dm1") == 0,
                "M12 game slot 0 gameId is \"dm1\"");
    expect_true(entry && entry->available == 0,
                "DM1 entry is unavailable when required assets are absent");

    menu.selectedIndex = 0;
    menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;
    intent = M12_StartupMenu_GetLaunchIntent(&menu);
    expect_true(intent.gameId && strcmp(intent.gameId, "dm1") == 0,
                "DM1 launch intent carries gameId=\"dm1\"");
    expect_true(intent.valid == 0,
                "DM1 launch intent is invalid when assets are absent");

    M12_StartupMenu_Destroy(&menu);
}

static void run_real_launcher_handoff_if_available(void) {
    M12_StartupMenuState menu;
    M11_GameViewState direct_view;
    M11_GameLaunchSpec direct_spec;
    const M12_MenuEntry* entry;
    const M12_AssetVersionStatus* direct_version;
    char real_dir[512];
    char direct_data_dir[M12_ASSET_DATA_DIR_CAPACITY];
    const char* data_dir = default_data_root(real_dir);
    int direct_version_index;

    if (!data_dir || !data_dir[0]) {
        expect_skip("HOME is unset; no default Firestaff data root");
        return;
    }

    init_menu_without_gallery(&menu, data_dir, "dm1");
    dismiss_initial_message(&menu);
    entry = M12_StartupMenu_GetEntry(&menu, 0);
    if (!entry || !entry->available ||
        !M12_AssetStatus_GameAvailable(&menu.assetStatus, "dm1")) {
        M12_StartupMenu_Destroy(&menu);
        expect_skip("no launchable DM1 data under default data root");
        return;
    }

    run_launcher_handoff_for_mode(&menu, M12_PRESENTATION_V1_ORIGINAL);
    run_launcher_handoff_for_mode(&menu, M12_PRESENTATION_V20_FILTERED);
    run_launcher_handoff_for_mode(&menu, M12_PRESENTATION_V21_UPSCALED);
    run_launcher_handoff_for_mode(&menu, M12_PRESENTATION_V22_MODERN);

    direct_version_index = M12_AssetStatus_FindFirstMatchedVersionForArchitecture(
        &menu.assetStatus, "dm1", M12_ARCH_AUTO);
    direct_version = direct_version_index >= 0
        ? M12_AssetStatus_GetVersion(&menu.assetStatus, "dm1",
                                     (size_t)direct_version_index)
        : NULL;
    if (!direct_version || !direct_version->versionId ||
        !M12_AssetStatus_PrepareDM1RuntimeVersion(
            &menu.assetStatus, direct_version->versionId, direct_data_dir,
            sizeof(direct_data_dir))) {
        M12_StartupMenu_Destroy(&menu);
        expect_skip("selected DM1 edition has no direct runtime source path");
        return;
    }

    memset(&direct_spec, 0, sizeof(direct_spec));
    direct_spec.title = "DUNGEON MASTER";
    direct_spec.gameId = "dm1";
    direct_spec.sourceId = "dm1";
    direct_spec.dataDir = direct_data_dir;
    direct_spec.verifiedAssetPath = direct_version->matchedPath;
    direct_spec.verifiedAssetMd5 = direct_version->matchedMd5;
    direct_spec.rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    direct_spec.presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    direct_spec.sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;

    M11_GameView_Init(&direct_view);
    expect_true(M11_GameView_Start(&direct_view, &direct_spec) == 1,
                "M11 direct DM1 game-view start succeeds with real data");
    expect_true(M11_GameView_Dm1StartupIntroBypassed(&direct_view) == 1,
                "M11 direct DM1 game-view start is the explicit intro bypass");

    M11_GameView_Shutdown(&direct_view);
    M12_StartupMenu_Destroy(&menu);
}

int main(void) {
    printf("=== DM1 V1 M12/M11 launcher handoff boundary ===\n");

    run_source_order_boundary();
    run_empty_launcher_boundary();
    run_real_launcher_handoff_if_available();

    printf("\nDM1 V1 M12/M11 launcher handoff boundary: %d passed, %d failed, %d skipped\n",
           g_passed, g_failures, g_skipped);
    if (g_failures) {
        fprintf(stderr,
                "DM1 V1 M12/M11 launcher handoff boundary FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: DM1 V1 M12/M11 launcher handoff boundary is wired");
    return 0;
}
