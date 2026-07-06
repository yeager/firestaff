/*
 * test_m11_nexus_startup_gate.c -- M11 Nexus startup ownership gate.
 *
 * Scope: startup handoff only. Empty or partial Nexus data must not leave
 * M11 in an active half-started state; a real extracted/ISO data directory,
 * when staged locally, must reach M11_GAME_SOURCE_NEXUS_DGN.
 *
 * Source: src/nexus/nexus_v1_launcher.c owns init/load-level sequencing;
 * src/engine/m11_game_view.c M11_GameView_StartNexus owns the M11 handoff.
 */

#include "m11_game_view.h"
#include "nexus_v1_champions.h"
#include "nexus_v1_engine.h"
#include "nexus_v1_launcher.h"
#include "nexus_v1_mechanics.h"
#include "nexus_v1_save.h"
#include "nexus_v1_ui_surfaces.h"
#include "nexus_v1_world.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
#include <stdlib.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#define TEST_PATH_SEP "\\"
#else
#include <sys/stat.h>
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#define TEST_PATH_SEP "/"
#endif

unsigned short G2157_;
unsigned char* G2159_puc_Bitmap_Source;
unsigned char* G2160_puc_Bitmap_Destination;

static int g_failures;

static void expect_true(int condition, const char* message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++g_failures;
    }
}

static int make_temp_root(char root[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_nexus_m11_startup_%lu",
             (unsigned long)rand());
    return TEST_MKDIR(root) == 0;
#else
    char tmpl[] = "/tmp/firestaff_nexus_m11_startup_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
    return 1;
#endif
}

static int write_file(const char* path, const char* bytes) {
    FILE* f = fopen(path, "wb");
    size_t n = bytes ? strlen(bytes) : 0u;
    int ok;
    if (!f) {
        return 0;
    }
    ok = n == 0u || fwrite(bytes, 1u, n, f) == n;
    fclose(f);
    return ok;
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

static int count_nonzero_region(const unsigned char* pixels,
                                int fb_w,
                                int fb_h,
                                int x,
                                int y,
                                int w,
                                int h) {
    int xx;
    int yy;
    int nonzero = 0;
    if (!pixels || fb_w <= 0 || fb_h <= 0 || w <= 0 || h <= 0) {
        return 0;
    }
    for (yy = 0; yy < h; ++yy) {
        int py = y + yy;
        if (py < 0 || py >= fb_h) continue;
        for (xx = 0; xx < w; ++xx) {
            int px = x + xx;
            if (px < 0 || px >= fb_w) continue;
            if (pixels[py * fb_w + px] != 0u) {
                ++nonzero;
            }
        }
    }
    return nonzero;
}

static void fill_nexus_spec(M11_GameLaunchSpec* spec, const char* data_dir) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "DUNGEON MASTER NEXUS";
    spec->gameId = "nexus";
    spec->sourceId = "nexus";
    spec->dataDir = data_dir;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static int set_test_home(const char* home) {
#ifdef _WIN32
    return _putenv_s("HOME", home ? home : "") == 0;
#else
    return home ? setenv("HOME", home, 1) == 0 : unsetenv("HOME") == 0;
#endif
}

static void cleanup_nexus_default_save_dir(const char* home) {
    char save_dir[512];
    char path[512];
    int i;
    char old_home[512];
    const char* old_home_env = getenv("HOME");

    if (!home || !home[0]) {
        return;
    }
    snprintf(old_home, sizeof(old_home), "%s",
             old_home_env ? old_home_env : "");
    (void)set_test_home(home);
    nexus_v1_save_default_dir(save_dir, sizeof(save_dir));
    for (i = 0; i < NEXUS_SAVE_MAX_SLOTS; ++i) {
        snprintf(path, sizeof(path), "%s%snexus_save_%02d.dat",
                 save_dir, TEST_PATH_SEP, i);
        remove(path);
    }
    (void)TEST_RMDIR(save_dir);
#ifndef _WIN32
    snprintf(path, sizeof(path), "%s/Library/Application Support/Firestaff/nexus",
             home);
    (void)TEST_RMDIR(path);
    snprintf(path, sizeof(path), "%s/Library/Application Support/Firestaff",
             home);
    (void)TEST_RMDIR(path);
    snprintf(path, sizeof(path), "%s/Library/Application Support", home);
    (void)TEST_RMDIR(path);
    snprintf(path, sizeof(path), "%s/Library", home);
    (void)TEST_RMDIR(path);
#endif
    if (old_home[0]) {
        (void)set_test_home(old_home);
    } else {
        (void)set_test_home(NULL);
    }
}

static void expect_face_loader_counts_real_vs_fallback(void) {
    Nexus_UI_Manager ui;
    unsigned char face_bytes[48 * 48];
    unsigned char short_bytes[16];
    int i;

    for (i = 0; i < (int)sizeof(face_bytes); ++i) {
        face_bytes[i] = (unsigned char)((i % 31) + 1);
    }
    memset(short_bytes, 3, sizeof(short_bytes));
    nexus_ui_manager_init(&ui);
    expect_true(nexus_ui_load_faces(&ui,
                                    face_bytes,
                                    0,
                                    (int)sizeof(face_bytes),
                                    0,
                                    48,
                                    48,
                                    NULL) > 0,
                "Nexus FACE loader reports real portrait copy");
    expect_true(nexus_ui_load_faces(&ui,
                                    short_bytes,
                                    0,
                                    (int)sizeof(short_bytes),
                                    1,
                                    48,
                                    48,
                                    NULL) == 0,
                "Nexus FACE loader reports fallback portrait separately");
    nexus_ui_manager_free(&ui);
}

static void expect_failed_start_is_inactive(const char* data_dir,
                                            const char* expected_status) {
    M11_GameViewState view;
    M11_GameLaunchSpec spec;

    fill_nexus_spec(&spec, data_dir);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "Nexus M11 startup rejects invalid data");
    expect_true(view.active == 0,
                "failed Nexus startup leaves M11 inactive");
    expect_true(view.startedFromLauncher == 0,
                "failed Nexus startup does not claim launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_BUILTIN_CATALOG,
                "failed Nexus startup does not retain Nexus sourceKind");
    expect_true(view.nexusEngine == NULL,
                "failed Nexus startup does not expose a Nexus engine");
    expect_true(strstr(view.lastOutcome, expected_status) != NULL,
                "failed Nexus startup reports expected blocker");
    M11_GameView_Shutdown(&view);
    nexus_v1_launcher_shutdown();
}

static const char* nexus_data_dir(char fallback[512]) {
    const char* data_dir = getenv("FIRESTAFF_NEXUS_V1_DATA_DIR");
    const char* home;
    if (data_dir && data_dir[0]) {
        return data_dir;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data/nexus", home);
    return fallback;
}

int main(void) {
    char empty_root[512];
    char partial_root[512];
    char partial_dm_bin[512];
    char real_fallback[512];
    const char* real_dir;

    expect_face_loader_counts_real_vs_fallback();

    if (make_temp_root(empty_root)) {
        expect_failed_start_is_inactive(empty_root, "NEXUS DATA ERROR");
        (void)TEST_RMDIR(empty_root);
    } else {
        expect_true(0, "created empty Nexus temp root");
    }

    if (make_temp_root(partial_root)) {
        snprintf(partial_dm_bin, sizeof(partial_dm_bin), "%s%sDM.BIN",
                 partial_root, TEST_PATH_SEP);
        expect_true(write_file(partial_dm_bin, "not-a-real-nexus-dm-bin"),
                    "seeded partial Nexus DM.BIN");
        expect_failed_start_is_inactive(partial_root, "NEXUS LEVEL ERROR");
        remove(partial_dm_bin);
        (void)TEST_RMDIR(partial_root);
    } else {
        expect_true(0, "created partial Nexus temp root");
    }

    real_dir = nexus_data_dir(real_fallback);
    if (real_dir && real_dir[0]) {
        M11_GameViewState view;
        M11_GameLaunchSpec spec;
        fill_nexus_spec(&spec, real_dir);
        M11_GameView_Init(&view);
        if (M11_GameView_Start(&view, &spec)) {
            char save_root[512];
            char save_path[512];
            Nexus_V1_World resume_world;
            Nexus_SaveResult save_result;
            int resume_fixture_ready = 0;
            unsigned char framebuffer[320 * 200];

            expect_true(view.active == 1,
                        "real Nexus startup leaves M11 active");
            expect_true(view.sourceKind == M11_GAME_SOURCE_NEXUS_DGN,
                        "real Nexus startup claims Nexus sourceKind");
            expect_true(view.nexusEngine != NULL,
                        "real Nexus startup exposes engine");
            expect_true(view.nexusState.level_loaded == 1,
                        "real Nexus startup loads level zero");
            expect_true(strstr(view.dungeonPath, "LEV00.DGN") != NULL,
                        "real Nexus startup exposes level path");
            expect_true(view.nexusState.title_active == 1,
                        "real Nexus startup enters title phase");
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 500,
                        "real Nexus title phase draws a nonblank frame");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_UP) ==
                            M11_GAME_INPUT_IGNORED,
                        "real Nexus title ignores movement input before explicit start");
            (void)M11_GameView_HandlePointerButton(
                &view, 40, 40, M11_DM1_MOUSE_MASK_RIGHT);
            expect_true(view.nexusState.title_active == 1 &&
                            view.nexusState.champion_select_active == 0 &&
                            view.nexusState.startup_save_select_active == 0,
                        "real Nexus title non-start input does not open startup menus");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus title phase advances on accept");
            expect_true(view.nexusState.title_active == 0,
                        "real Nexus title phase clears after input");
            if (view.nexusState.startup_save_select_active) {
                while (view.nexusState.startup_save_selected_row + 1 <
                       view.nexusState.startup_save_row_count) {
                    expect_true(M11_GameView_HandleInput(&view,
                                                         M12_MENU_INPUT_DOWN) ==
                                    M11_GAME_INPUT_REDRAW,
                                "real Nexus startup save menu moves toward NEW GAME");
                }
                expect_true(M11_GameView_HandleInput(&view,
                                                     M12_MENU_INPUT_ACCEPT) ==
                                M11_GAME_INPUT_REDRAW,
                            "real Nexus startup save menu NEW GAME enters champion selection");
            }
            expect_true(view.nexusState.champion_select_active == 1,
                        "real Nexus startup enters champion selection");
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 500,
                        "real Nexus champion selection draws a nonblank frame");
            expect_true(view.nexusEngine &&
                            nexus_v1_startup_faces_loaded_count(view.nexusEngine) > 0,
                        "real Nexus startup loads FACE.BIN champion portraits");
            expect_true(view.nexusEngine &&
                            nexus_v1_startup_faces_expected_count(view.nexusEngine) ==
                                view.nexusEngine->champions.champion_count &&
                            nexus_v1_startup_faces_fallback_count(view.nexusEngine) == 0 &&
                            nexus_v1_startup_faces_ready(view.nexusEngine),
                        "real Nexus startup FACE.BIN coverage matches roster without fallback");
            expect_true(count_nonzero_region(framebuffer, 320, 200,
                                             22, 38, 10, 10) > 0,
                        "real Nexus champion selection draws FACE.BIN portrait pixels");
            expect_true(M11_GameView_HandlePointer(&view, 24, 24, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection panel consumes non-row pointer hits");
            expect_true(view.nexusState.champion_select_active == 1 &&
                            view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 0,
                        "real Nexus champion selection panel hit does not recruit");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACCEPT) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection recruits selected champion");
            expect_true(view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 1,
                        "real Nexus champion selection fills party");
            expect_true(view.nexusState.champion_cursor == 1,
                        "real Nexus champion selection advances cursor after recruit");
            expect_true(M11_GameView_HandlePointer(&view, 24, 49, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection accepts pointer row click");
            expect_true(view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 2,
                        "real Nexus pointer selection recruits second champion");
            expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection Back removes last recruit");
            expect_true(view.nexusState.champion_select_active == 1 &&
                            view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 1 &&
                            view.nexusState.champion_cursor == 1,
                        "real Nexus champion selection Back keeps local cursor on removed champion");
            expect_true(M11_GameView_HandlePointer(&view, 24, 49, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection can reselect removed champion");
            expect_true(view.nexusEngine &&
                            view.nexusEngine->champions.party_count == 2,
                        "real Nexus champion selection restore second recruit after Back");
            expect_true(M11_GameView_HandlePointer(&view, 24, 184, 1) ==
                            M11_GAME_INPUT_REDRAW,
                        "real Nexus champion selection pointer footer starts dungeon");
            expect_true(view.nexusState.champion_select_active == 0,
                        "real Nexus champion selection clears for dungeon");
            memset(framebuffer, 0, sizeof(framebuffer));
            M11_GameView_Draw(&view, framebuffer, 320, 200);
            expect_true(count_nonzero_pixels(framebuffer, sizeof(framebuffer)) > 100,
                        "real Nexus dungeon phase draws a nonblank frame");

            if (view.nexusEngine && make_temp_root(save_root)) {
                snprintf(save_path, sizeof(save_path), "%s%snexus_resume.fnxs",
                         save_root, TEST_PATH_SEP);
                nexus_v1_world_init(&resume_world);
                nexus_v1_party_place(&resume_world, 0, 18, 12, 3);
                resume_world.world_tick = 77u;
                resume_world.state_hash = nexus_v1_world_hash(&resume_world);
                save_result = nexus_v1_save_full_to_path(
                    save_path,
                    resume_world.party_level,
                    resume_world.party_x,
                    resume_world.party_y,
                    resume_world.party_dir,
                    (uint32_t)resume_world.world_tick,
                    resume_world.state_hash,
                    &view.nexusEngine->champions,
                    &resume_world);
                expect_true(save_result == NEXUS_SAVE_OK,
                            "wrote Nexus FNXS resume fixture");
                resume_fixture_ready = (save_result == NEXUS_SAVE_OK);
            }
            M11_GameView_Shutdown(&view);
            nexus_v1_launcher_shutdown();

            if (resume_fixture_ready) {
                fill_nexus_spec(&spec, real_dir);
                spec.savePath = save_path;
                M11_GameView_Init(&view);
                expect_true(M11_GameView_Start(&view, &spec),
                            "M11 Nexus FNXS resume succeeds");
                expect_true(view.active == 1,
                            "resumed Nexus startup leaves M11 active");
                expect_true(view.sourceKind == M11_GAME_SOURCE_NEXUS_DGN,
                            "resumed Nexus startup claims Nexus sourceKind");
                expect_true(strstr(view.lastOutcome, "NEXUS RESUMED") != NULL,
                            "resumed Nexus startup reports resumed status");
                expect_true(view.nexusState.title_active == 0,
                            "resumed Nexus startup skips title phase");
                expect_true(view.nexusState.champion_select_active == 0,
                            "resumed Nexus startup skips champion selection");
                expect_true(view.nexusState.party_x == 18 &&
                            view.nexusState.party_y == 12 &&
                            view.nexusState.party_dir == 3,
                            "resumed Nexus startup mirrors saved party pose");
                expect_true(view.nexusState.tick_count == 77,
                            "resumed Nexus startup mirrors saved tick");
                expect_true(view.nexusEngine &&
                            view.nexusEngine->mechanics &&
                            view.nexusEngine->mechanics->party_x == 18 &&
                            view.nexusEngine->mechanics->party_y == 12 &&
                            view.nexusEngine->mechanics->party_dir == 3,
                            "resumed Nexus startup applies mechanics pose");
                M11_GameView_Shutdown(&view);
                nexus_v1_launcher_shutdown();
                remove(save_path);
                (void)TEST_RMDIR(save_root);
            }

            {
                char temp_home[512];
                char old_home[512];
                char default_save_dir[512];
                const char* old_home_env = getenv("HOME");
                Nexus_V1_SaveManager slot_mgr;
                Nexus_V1_ChampionPool slot_champions;
                Nexus_V1_World slot_world;
                int slot_fixture_ready = 0;

                snprintf(old_home, sizeof(old_home), "%s",
                         old_home_env ? old_home_env : "");
                if (make_temp_root(temp_home) && set_test_home(temp_home)) {
                    nexus_v1_save_default_dir(default_save_dir,
                                              sizeof(default_save_dir));
                    nexus_v1_champions_init(&slot_champions);
                    nexus_v1_world_init(&slot_world);
                    nexus_v1_party_place(&slot_world, 0, 19, 13, 2);
                    slot_world.world_tick = 91u;
                    slot_world.state_hash = nexus_v1_world_hash(&slot_world);
                    nexus_v1_save_init(&slot_mgr, default_save_dir);
                    slot_fixture_ready =
                        nexus_v1_save_full(&slot_mgr,
                                           3,
                                           slot_world.party_level,
                                           slot_world.party_x,
                                           slot_world.party_y,
                                           slot_world.party_dir,
                                           (uint32_t)slot_world.world_tick,
                                           slot_world.state_hash,
                                           &slot_champions,
                                           &slot_world) == NEXUS_SAVE_OK;
                    expect_true(slot_fixture_ready,
                                "wrote Nexus default-save slot fixture");
                    if (slot_fixture_ready) {
                        fill_nexus_spec(&spec, real_dir);
                        M11_GameView_Init(&view);
                        expect_true(M11_GameView_Start(&view, &spec),
                                    "M11 Nexus startup with default save slot succeeds");
                        expect_true(view.nexusState.title_active == 1,
                                    "M11 Nexus save-slot startup starts on title");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_DOWN) ==
                                        M11_GAME_INPUT_IGNORED,
                                    "M11 Nexus save-slot title ignores movement input");
                        expect_true(view.nexusState.title_active == 1 &&
                                        view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus save-slot title movement does not open save menu");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus title advances to save selection");
                        expect_true(view.nexusState.startup_save_select_active == 1,
                                    "M11 Nexus startup exposes save selection when slots exist");
                        expect_true(view.nexusState.startup_save_slot_mask ==
                                        (1u << 3),
                                    "M11 Nexus startup save selection sees slot 03");
                        memset(framebuffer, 0, sizeof(framebuffer));
                        M11_GameView_Draw(&view, framebuffer, 320, 200);
                        expect_true(count_nonzero_pixels(framebuffer,
                                                         sizeof(framebuffer)) > 500,
                                    "M11 Nexus startup save selection draws nonblank frame");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_BACK) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup save selection Back returns to title");
                        expect_true(view.nexusState.title_active == 1 &&
                                        view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus startup save selection Back restores title phase");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup title reopens save selection after Back");
                        expect_true(view.nexusState.startup_save_select_active == 1,
                                    "M11 Nexus startup save selection is active again after title");
                        expect_true(M11_GameView_HandlePointer(
                                        &view, 24, 20, 1) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup save panel consumes non-row pointer hits");
                        expect_true(view.nexusState.startup_save_select_active == 1 &&
                                        view.nexusState.startup_save_selected_row == 0,
                                    "M11 Nexus startup save panel hit keeps menu selection");
                        expect_true(M11_GameView_HandlePointer(
                                        &view, 24, 43, 1) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus startup pointer loads slot row");
                        expect_true(view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus startup save selection closes after load");
                        expect_true(strstr(view.lastOutcome, "NEXUS RESUMED") != NULL,
                                    "M11 Nexus startup slot load reports resumed status");
                        expect_true(view.nexusState.party_x == 19 &&
                                    view.nexusState.party_y == 13 &&
                                    view.nexusState.party_dir == 2,
                                    "M11 Nexus startup slot load mirrors saved pose");
                        expect_true(view.nexusState.tick_count == 91,
                                    "M11 Nexus startup slot load mirrors saved tick");
                        M11_GameView_Shutdown(&view);
                        nexus_v1_launcher_shutdown();

                        fill_nexus_spec(&spec, real_dir);
                        M11_GameView_Init(&view);
                        expect_true(M11_GameView_Start(&view, &spec),
                                    "M11 Nexus startup with save slot restarts for NEW GAME");
                        expect_true(view.nexusState.title_active == 1,
                                    "M11 Nexus save-slot NEW GAME path starts on title");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus save-slot NEW GAME path advances title");
                        expect_true(view.nexusState.startup_save_select_active == 1,
                                    "M11 Nexus save-slot NEW GAME path exposes save menu");
                        {
                            int tick_before = view.nexusState.tick_count;
                            int engine_tick_before = view.nexusEngine
                                ? view.nexusEngine->game.tick_count
                                : -1;
                            expect_true(M11_GameView_AdvanceIdleTick(&view) ==
                                            M11_GAME_INPUT_IGNORED,
                                        "M11 Nexus save-select blocks idle runtime tick");
                            expect_true(view.nexusState.startup_save_select_active == 1 &&
                                            view.nexusState.tick_count == tick_before &&
                                            view.nexusEngine &&
                                            view.nexusEngine->game.tick_count ==
                                                engine_tick_before,
                                        "M11 Nexus save-select keeps runtime tick frozen");
                        }
                        while (view.nexusState.startup_save_selected_row + 1 <
                               view.nexusState.startup_save_row_count) {
                            expect_true(M11_GameView_HandleInput(
                                            &view, M12_MENU_INPUT_DOWN) ==
                                            M11_GAME_INPUT_REDRAW,
                                        "M11 Nexus save-slot NEW GAME path moves down");
                        }
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus save-slot NEW GAME path accepts NEW GAME");
                        expect_true(view.nexusState.startup_save_select_active == 0,
                                    "M11 Nexus save-slot NEW GAME path closes save menu");
                        expect_true(view.nexusState.champion_select_active == 1,
                                    "M11 Nexus save-slot NEW GAME path enters champion selection");
                        expect_true(view.nexusEngine &&
                                        view.nexusEngine->champions.party_count == 0,
                                    "M11 Nexus save-slot NEW GAME path keeps empty new party");
                        memset(framebuffer, 0, sizeof(framebuffer));
                        M11_GameView_Draw(&view, framebuffer, 320, 200);
                        expect_true(count_nonzero_pixels(framebuffer,
                                                         sizeof(framebuffer)) > 500,
                                    "M11 Nexus save-slot NEW GAME path draws champion select");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_BACK) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus empty champion selection Back returns to save menu");
                        expect_true(view.nexusState.champion_select_active == 0 &&
                                        view.nexusState.startup_save_select_active == 1 &&
                                        view.nexusState.startup_save_selected_row + 1 ==
                                            view.nexusState.startup_save_row_count,
                                    "M11 Nexus empty champion selection Back keeps NEW GAME selected");
                        expect_true(M11_GameView_HandleInput(
                                        &view, M12_MENU_INPUT_ACCEPT) ==
                                        M11_GAME_INPUT_REDRAW,
                                    "M11 Nexus NEW GAME can re-enter champion selection after Back");
                        expect_true(view.nexusState.champion_select_active == 1 &&
                                        view.nexusEngine &&
                                        view.nexusEngine->champions.party_count == 0,
                                    "M11 Nexus re-entered champion selection remains a new party");
                        M11_GameView_Shutdown(&view);
                        nexus_v1_launcher_shutdown();
                    }
                    if (old_home[0]) {
                        (void)set_test_home(old_home);
                    }
                    cleanup_nexus_default_save_dir(temp_home);
                    (void)TEST_RMDIR(temp_home);
                } else {
                    if (old_home[0]) {
                        (void)set_test_home(old_home);
                    }
                    expect_true(0, "created isolated Nexus startup HOME");
                }
            }
        } else {
            printf("skip: no launchable Nexus V1 data at %s\n", real_dir);
            M11_GameView_Shutdown(&view);
            nexus_v1_launcher_shutdown();
        }
    }

    if (g_failures) {
        fprintf(stderr, "M11 Nexus startup gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: M11 Nexus startup gate");
    return 0;
}
