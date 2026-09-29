/* Authentic PC34 M11 selector music through the real entrance event loop. */
#include "dm1_entrance_m11.h"
#include "entrance_frontend_pc34_compat.h"
#include "firestaff/dm1/v1/startup_sequence_pc34_compat.h"
#include "main_loop_m11.h"
#include "m11_game_view.h"
#include "menu_startup_m12.h"
#include "render_sdl_m11.h"
#include "dm1_v1_f0740_f0743_music_source_pc34_compat.h"

#include <SDL3/SDL.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

enum Scenario {
    SCENARIO_CREDITS_THEN_QUIT,
    SCENARIO_ENTER,
    SCENARIO_RESUME,
    SCENARIO_EARLY_QUIT
};

typedef struct ScenarioState {
    enum Scenario scenario;
    int failures;
    int noAudio;
    uint64_t lastWaitMs;
    int waitSeen;
    int creditsRequested;
    int creditsSeen;
    int creditsDismissed;
    int quitRequested;
    int sourceMusicStarted;
    int sourceMusicStopped;
    int eventPushFailed;
    int playRequestsAtStart;
    int playRequestsAtCredits;
    const void* musicStreamAtCredits;
    int cursorAtCredits;
    uint64_t creditsObservedAtMs;
} ScenarioState;

static void check(ScenarioState* state, int condition, const char* message)
{
    if (condition) return;
    fprintf(stderr, "FAIL: %s\n", message);
    ++state->failures;
}

static void push_click(ScenarioState* state, int x, int y)
{
    SDL_Event event;
    int px, py, pw, ph;
    if (M11_Render_GetPresentRect(&px, &py, &pw, &ph) != M11_RENDER_OK) {
        check(state, 0, "presented entrance rectangle is available for source clicks");
        return;
    }
    /* Dummy SDL has unit drawable density. Map the original source point
     * into the real rendered rectangle, including aspect-correction bars. */
    x = px + (int)(((double)x + 0.5) * pw / 320.0);
    y = py + (int)(((double)y + 0.5) * ph / 200.0);
    memset(&event, 0, sizeof(event));
    event.type = SDL_EVENT_MOUSE_BUTTON_DOWN;
    event.button.button = SDL_BUTTON_LEFT;
    event.button.x = (float)x;
    event.button.y = (float)y;
    if (!SDL_PushEvent(&event)) {
        state->eventPushFailed = 1;
        ++state->failures;
    }
}

static void push_key(ScenarioState* state)
{
    SDL_Event event;
    memset(&event, 0, sizeof(event));
    event.type = SDL_EVENT_KEY_DOWN;
    event.key.windowID = SDL_GetWindowID(M11_Render_GetWindow());
    event.key.scancode = SDL_SCANCODE_RETURN;
    event.key.key = SDLK_RETURN;
    event.key.down = true;
    if (!SDL_PushEvent(&event)) {
        state->eventPushFailed = 1;
        ++state->failures;
    }
}

static void push_quit(ScenarioState* state)
{
    SDL_Event event;
    memset(&event, 0, sizeof(event));
    event.type = SDL_EVENT_QUIT;
    if (!SDL_PushEvent(&event)) {
        state->eventPushFailed = 1;
        ++state->failures;
    }
}

static void observe_entrance(void* user, int phase, uint64_t activeWaitMs,
                             const M11_AudioState* audio)
{
    ScenarioState* state = (ScenarioState*)user;
    const uint64_t selectorStartMs =
        (60U * 1000U + DM1_V1_VGA_VBLANK_HZ - 1U) /
        DM1_V1_VGA_VBLANK_HZ;
    if (!state || !audio) return;

    if (phase == M11_ENTRANCE_PHASE_WAIT) {
        state->waitSeen = 1;
        state->lastWaitMs = activeWaitMs;
        if (audio->titleMusicPlayRequestCount > state->playRequestsAtStart) {
            if (!state->sourceMusicStarted) {
                state->sourceMusicStarted = 1;
                check(state, activeWaitMs >= selectorStartMs,
                      "selector SONG.DAT does not start before 60 VGA retraces");
            }
            check(state,
                  audio->titleMusicPlayRequestCount == state->playRequestsAtStart + 1,
                  "entrance wait starts the selected selector score once");
        } else {
            check(state, activeWaitMs < selectorStartMs,
                  "selector score remains stopped before the 60-retrace deadline");
        }

        if (state->scenario == SCENARIO_EARLY_QUIT && !state->quitRequested) {
            state->quitRequested = 1;
            push_quit(state);
            return;
        }
        if (state->scenario == SCENARIO_CREDITS_THEN_QUIT &&
            state->creditsDismissed && !state->quitRequested) {
            check(state, audio->musicStream == state->musicStreamAtCredits &&
                audio->titleMusicPlayRequestCount == state->playRequestsAtCredits,
                "return from Credits preserves the existing score owner");
            state->quitRequested = 1;
            push_click(state, 267, 117); /* ReDMCSB C434 entrance quit zone. */
            return;
        }
        if (state->scenario != SCENARIO_EARLY_QUIT &&
            !state->creditsRequested && !state->creditsDismissed &&
            activeWaitMs >= selectorStartMs + 150U) {
            state->creditsRequested = 1;
            if (state->scenario == SCENARIO_CREDITS_THEN_QUIT) {
                push_click(state, 270, 192); /* ReDMCSB C411 credits zone. */
            } else if (state->scenario == SCENARIO_ENTER) {
                push_click(state, 270, 52);  /* ReDMCSB C407 Enter zone. */
            } else {
                push_click(state, 270, 85);  /* ReDMCSB C409 Resume zone. */
            }
        }
        return;
    }

    if (phase == M11_ENTRANCE_PHASE_CREDITS) {
        if (!state->creditsSeen) {
            state->creditsSeen = 1;
            state->creditsObservedAtMs = activeWaitMs;
            state->playRequestsAtCredits = audio->titleMusicPlayRequestCount;
            state->musicStreamAtCredits = audio->musicStream;
            state->cursorAtCredits = audio->titleMusicCursor;
            check(state, audio->titleMusicLoopActive == !state->noAudio,
                  "selector audio loop remains active on authentic credits page");
            check(state, state->noAudio ? audio->musicStream == NULL : audio->musicStream != NULL,
                  "credits page retains the selected audio stream");
            check(state,
                  audio->titleMusicPlayRequestCount == state->playRequestsAtStart + 1,
                  "credits page does not restart the selector score");
        } else if (!state->creditsDismissed &&
                   activeWaitMs >= state->creditsObservedAtMs + 200U) {
            check(state, audio->musicStream == state->musicStreamAtCredits,
                  "credits wait preserves the same SDL music stream owner");
            check(state,
                  audio->titleMusicPlayRequestCount == state->playRequestsAtCredits,
                  "credits wait preserves the source playback request count");
            check(state, state->noAudio
                ? !audio->musicStream && !audio->titleMusicLoopActive
                : audio->titleMusicLoopActive == 1 &&
                    audio->titleMusicCursor > state->cursorAtCredits,
                  "selector SONG.DAT advances through credits without restarting");
            state->creditsDismissed = 1;
            push_key(state);
        }
        return;
    }

    if (phase == M11_ENTRANCE_PHASE_DOORS) {
        state->sourceMusicStopped =
            !audio->titleMusicLoopActive && audio->titleMusicCursor == 0;
        check(state, state->sourceMusicStopped,
              "leaving the selector stops and disarms its source music before doors");
    }
}

static int selected_song_path(const char* graphicsPath,
                              char* outPath, size_t outPathSize)
{
    static const char graphicsName[] = "GRAPHICS.DAT";
    static const char songName[] = "SONG.DAT";
    size_t length;
    if (!graphicsPath || !outPath || outPathSize == 0U) return 0;
    length = strlen(graphicsPath);
    if (length < sizeof(graphicsName) - 1U || length >= outPathSize ||
        strcmp(graphicsPath + length - (sizeof(graphicsName) - 1U),
               graphicsName) != 0) return 0;
    memcpy(outPath, graphicsPath, length + 1U);
    memcpy(outPath + length - (sizeof(graphicsName) - 1U), songName,
           sizeof(songName));
    return 1;
}

static int run_scenario(const char* dataDir, enum Scenario scenario,
                        const char* label, int expectNoAudio)
{
    M12_StartupMenuState menu;
    M12_StartupMenuInitOptions menuOptions;
    M11_GameViewState view;
    DM1_V1_StartupHandoffPostLaunchPlan_PC34 plan;
    DM1_V1_F0740F0743MusicSourcePc34 songSource;
    ScenarioState state;
    const M12_AssetVersionStatus* version;
    int pc34Index;
    int result;
    char expectedSongPath[M11_GAME_VIEW_PATH_CAPACITY];

    memset(&menuOptions, 0, sizeof(menuOptions));
    menuOptions.skipScreenshotGalleryScan = 1;
    M12_StartupMenu_InitWithOptions(&menu, dataDir, "dm1", &menuOptions);
    pc34Index = M12_AssetStatus_FindVersionIndex("dm1", "pc34-en");
    version = pc34Index >= 0
        ? M12_AssetStatus_GetVersion(&menu.assetStatus, "dm1",
                                     (size_t)pc34Index)
        : NULL;
    if (!version || !version->matched) {
        fprintf(stderr, "FAIL: authenticated PC34 edition is missing for %s\n", label);
        M12_StartupMenu_Destroy(&menu);
        return 1;
    }

    menu.selectedIndex = 0;
    menu.activatedIndex = 0;
    menu.launchRequested = 1;
    menu.gameOptions[0].architectureIndex = M12_ARCH_PC;
    menu.gameOptions[0].versionIndex = pc34Index;
    menu.gameOptions[0].presentationModeIndex = M12_PRESENTATION_V1_ORIGINAL;
    menu.settings.graphicsIndex = M12_PRESENTATION_V1_ORIGINAL;

    M11_GameView_Init(&view);
    if (!M11_GameView_OpenSelectedMenuEntry(&view, &menu)) {
        fprintf(stderr, "FAIL: authentic PC34 M11 open failed for %s\n", label);
        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);
        return 1;
    }
    if (!view.active || !view.assetsAvailable ||
        !selected_song_path(view.assetLoader.graphicsDatPath,
                            expectedSongPath, sizeof(expectedSongPath)) ||
        !dm1_v1_f0740_f0743_bind_song_dat_pc34(expectedSongPath,
                                               &songSource) ||
        !view.audioState.originalSongAvailable ||
        strcmp(view.audioState.originalSongDatPath, expectedSongPath) != 0) {
        fprintf(stderr,
                "FAIL: selected PC34 GRAPHICS.DAT did not bind its authenticated SONG.DAT for %s\n",
                label);
        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);
        return 1;
    }
    if (expectNoAudio) {
        if (M11_Audio_IsAvailable(&view.audioState)) {
            fprintf(stderr, "FAIL: no-audio scenario unexpectedly opened a device\n");
            M11_GameView_Shutdown(&view);
            M12_StartupMenu_Destroy(&menu);
            return 1;
        }
    } else if (!M11_Audio_IsAvailable(&view.audioState)) {
        fprintf(stderr, "FAIL: SDL dummy audio device did not open for %s\n", label);
        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);
        return 1;
    }
    (void)M11_GameView_SetMusicEnabled(&view, 1);

    memset(&plan, 0, sizeof(plan));
    if (!dm1_v1_startup_handoff_post_launch_plan_pc34("dm1", &plan) ||
        !plan.entrance_full_start_receipt.valid ||
        !plan.media_receipt.handled ||
        plan.media_receipt.platform != DM1_V1_STARTUP_MEDIA_PLATFORM_PC34 ||
        !plan.media_receipt.play_swsh || !plan.media_receipt.play_title ||
        !plan.media_receipt.play_entrance) {
        fprintf(stderr, "FAIL: PC34 startup media receipt is invalid for %s\n", label);
        M11_GameView_Shutdown(&view);
        M12_StartupMenu_Destroy(&menu);
        return 1;
    }

    memset(&state, 0, sizeof(state));
    state.scenario = scenario;
    state.noAudio = expectNoAudio;
    state.playRequestsAtStart = view.audioState.titleMusicPlayRequestCount;
    /* A bounded test watchdog. It remains long enough for source events and
     * only replaces the no-input policy; all observed input is pushed through
     * SDL after the real entrance loop begins. */
    result = M11_Entrance_RunSourceTransition(
        &view, 10000, &plan.entrance_full_start_receipt,
        &plan.media_receipt, observe_entrance, &state);

    fprintf(stderr, "selector timing %s: last_wait=%llu credits_start=%llu\n",
            label, (unsigned long long)state.lastWaitMs,
            (unsigned long long)state.creditsObservedAtMs);
    check(&state, state.lastWaitMs < 5000U,
          "source input completes before the automatic Enter watchdog");
    if (scenario == SCENARIO_CREDITS_THEN_QUIT) {
        check(&state, state.creditsSeen && state.creditsDismissed,
              "credits page was entered and dismissed with a fresh key");
        check(&state, result == ENTRANCE_COMPAT_COMMAND_PATH_QUIT,
              "entrance Quit button returns the source Quit command");
    } else if (scenario == SCENARIO_ENTER) {
        check(&state, result == ENTRANCE_COMPAT_COMMAND_PATH_ENTER,
              "source Enter click completes authentic entrance transition");
        check(&state, state.sourceMusicStopped,
              "source music is stopped before the Enter door animation");
    } else if (scenario == SCENARIO_RESUME) {
        check(&state, result == ENTRANCE_COMPAT_COMMAND_PATH_RESUME,
              "source Resume click returns the authentic Resume command");
    } else {
        check(&state, result == ENTRANCE_COMPAT_COMMAND_PATH_QUIT,
              "early SDL quit event exits the entrance transition");
        check(&state, !state.sourceMusicStarted,
              "early quit before selector deadline does not start the score");
    }
    check(&state, state.waitSeen && state.quitRequested + state.creditsRequested > 0,
          "real M11 entrance wait received source-routed SDL input");
    check(&state, !state.eventPushFailed,
          "SDL accepted observer-injected source input");
    if (scenario != SCENARIO_EARLY_QUIT) {
        check(&state, state.sourceMusicStarted,
              "authenticated selector score started after its VGA wait");
    }
    check(&state, !view.audioState.titleMusicLoopActive &&
                      view.audioState.titleMusicCursor == 0 &&
                      (!view.audioState.musicStream ||
                       SDL_GetAudioStreamQueued(
                           (SDL_AudioStream*)view.audioState.musicStream) == 0),
          "every source entrance exit stops the selector stream");
    if (state.failures) {
        fprintf(stderr, "FAIL: %s had %d observer assertion failures\n",
                label, state.failures);
    } else {
        printf("PASS: %s\n", label);
    }

    M11_GameView_Shutdown(&view);
    M12_StartupMenu_Destroy(&menu);
    return state.failures ? 1 : 0;
}

int main(void)
{
    const char* dataDir = getenv("FIRESTAFF_DM1_DATA_DIR");
    struct stat dataInfo;
    int noAudio = getenv("FIRESTAFF_TEST_NO_AUDIO") != NULL;
    int failures = 0;
    if (!dataDir || !dataDir[0]) {
        puts("SKIP: FIRESTAFF_DM1_DATA_DIR is not selected");
        return 77;
    }
    if (stat(dataDir, &dataInfo) != 0) {
        printf("SKIP: selected DM1 data path does not exist: %s\n", dataDir);
        return 77;
    }
    if (noAudio && SDL_setenv_unsafe("SDL_AUDIODRIVER",
                                     "firestaff-no-such-audio-device", 1) != 0) {
        fprintf(stderr, "FAIL: cannot select the no-audio SDL driver: %s\n",
                SDL_GetError());
        return 1;
    }
    if (!SDL_InitSubSystem(SDL_INIT_VIDEO |
                           (noAudio ? 0U : SDL_INIT_AUDIO))) {
        fprintf(stderr, "FAIL: SDL subsystem init: %s\n", SDL_GetError());
        return 1;
    }
    if (M11_Render_Init(320, 200, M11_SCALE_FIT) != M11_RENDER_OK) {
        fprintf(stderr, "FAIL: M11 renderer init: %s\n", SDL_GetError());
        SDL_QuitSubSystem(SDL_INIT_VIDEO |
                          (noAudio ? 0U : SDL_INIT_AUDIO));
        return 1;
    }

    failures += run_scenario(dataDir, SCENARIO_CREDITS_THEN_QUIT,
        "real PC34 selector survives credits and stops on Quit", noAudio);
    failures += run_scenario(dataDir, SCENARIO_ENTER,
        "real PC34 selector stops on Enter", noAudio);
    failures += run_scenario(dataDir, SCENARIO_RESUME,
        "real PC34 selector stops on Resume", noAudio);
    failures += run_scenario(dataDir, SCENARIO_EARLY_QUIT,
        "real PC34 early quit before music deadline", noAudio);

    M11_Render_Shutdown();
    SDL_QuitSubSystem(SDL_INIT_VIDEO |
                      (noAudio ? 0U : SDL_INIT_AUDIO));
    if (failures) return 1;
    return 0;
}
