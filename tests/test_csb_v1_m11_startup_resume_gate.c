/*
 * test_csb_v1_m11_startup_resume_gate.c -- CSB V1 startup/resume M11 gate.
 *
 * Verifies that M11_GameView_Start(gameId="csb") owns the CSB boot profile
 * and loads an optional CSB savePath through the CSB runtime, not through the
 * generic DM1 quick-resume branch.
 *
 * Source-lock:
 *   ReDMCSB ENTRANCE.C F0806 lines 409-441 (CSB entrance/runtime setup)
 *   ReDMCSB LOADSAVE.C F0435 lines 2721-2800 (save restore of GLOBAL_DATA,
 *     GameTime, party map/position/direction, leader and caster)
 */

#include "csb_v1_boot.h"
#include "csb_v1_runtime_pc34_compat.h"
#include "csb_v1_save_load_pc34_compat.h"
#include "m11_game_view.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
#include <direct.h>
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

static int write_tiny_file(const char* path, const char* bytes) {
    FILE* f = fopen(path, "wb");
    if (!f) {
        return 0;
    }
    fputs(bytes, f);
    fclose(f);
    return 1;
}

static int make_temp_csb_root(char root[512], char csb_dir[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_csb_m11_resume_gate_%lu",
             (unsigned long)rand());
    if (TEST_MKDIR(root) != 0) {
        return 0;
    }
#else
    char tmpl[] = "/tmp/firestaff_csb_m11_resume_gate_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
#endif
    snprintf(csb_dir, 512, "%s%s%s", root, TEST_PATH_SEP, "csb");
    if (TEST_MKDIR(csb_dir) != 0) {
        (void)TEST_RMDIR(root);
        return 0;
    }
    return 1;
}

static void remove_temp_csb_root(const char* root, const char* csb_dir) {
    char graphics[512];
    char dungeon[512];
    snprintf(graphics, sizeof(graphics), "%s%sGRAPHICS.DAT",
             csb_dir, TEST_PATH_SEP);
    snprintf(dungeon, sizeof(dungeon), "%s%sDUNGEON.DAT",
             csb_dir, TEST_PATH_SEP);
    remove(graphics);
    remove(dungeon);
    (void)TEST_RMDIR(csb_dir);
    (void)TEST_RMDIR(root);
}

static void fill_csb_launch_spec(M11_GameLaunchSpec* spec,
                                 const char* data_dir,
                                 const char* save_path) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "CHAOS STRIKES BACK";
    spec->gameId = "csb";
    spec->sourceId = "csb";
    spec->dataDir = data_dir;
    spec->savePath = save_path;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static void check_incomplete_required_files_block_m11(const char* label,
                                                      int seed_graphics,
                                                      int seed_dungeon) {
    char root[512];
    char csb_dir[512];
    char path[512];
    M11_GameLaunchSpec spec;
    M11_GameViewState view;

    expect_true(make_temp_csb_root(root, csb_dir),
                "created isolated CSB incomplete-data root");
    if (seed_graphics) {
        snprintf(path, sizeof(path), "%s%sGRAPHICS.DAT", csb_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-csb-graphics"),
                    "seeded synthetic CSB GRAPHICS.DAT");
    }
    if (seed_dungeon) {
        snprintf(path, sizeof(path), "%s%sDUNGEON.DAT", csb_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-csb-dungeon"),
                    "seeded synthetic CSB DUNGEON.DAT");
    }

    fill_csb_launch_spec(&spec, root, NULL);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0, label);
    expect_true(view.active == 0,
                "M11 incomplete/unverified CSB launch leaves view inactive");
    expect_true(view.csbBootProfile == NULL,
                "M11 incomplete/unverified CSB launch does not retain boot profile");
    expect_true(view.sourceKind != M11_GAME_SOURCE_CSB_BOOT,
                "M11 incomplete/unverified CSB launch does not claim CSB boot source");
    M11_GameView_Shutdown(&view);
    remove_temp_csb_root(root, csb_dir);
}

static const char* csb_data_dir(char fallback[512]) {
    const char* data_dir = getenv("FIRESTAFF_CSB_V1_DATA_DIR");
    const char* home;
    if (!data_dir || !data_dir[0]) {
        data_dir = getenv("FIRESTAFF_CSB_CANONICAL_DIR");
    }
    if (data_dir && data_dir[0]) {
        return data_dir;
    }
    home = getenv("HOME");
    if (!home || !home[0]) {
        return NULL;
    }
    snprintf(fallback, 512, "%s/.firestaff/data", home);
    return fallback;
}

static int build_runtime_resume_save(const char* data_dir,
                                     const char* save_path,
                                     CSB_V1_RuntimeProfile* expected) {
    CSB_V1_BootProfile boot;
    csb_v1_boot_profile_init(&boot);
    if (csb_v1_boot_scan_assets(&boot, data_dir) != 0 ||
        !boot.assets_verified) {
        csb_v1_boot_cleanup(&boot);
        return 0;
    }
    if (csb_v1_boot_enter_game(&boot) != 0) {
        csb_v1_boot_cleanup(&boot);
        return 0;
    }

    boot.runtime.party_x = CSB_V1_START_PARTY_X + 2;
    boot.runtime.party_y = CSB_V1_START_PARTY_Y + 1;
    boot.runtime.party_dir = CSB_V1_DIR_EAST;
    boot.runtime.leader_index = 0;
    boot.runtime.magic_caster_index = 1;
    boot.runtime.tick_count = 7U;
    boot.runtime.game_time = 7U;
    boot.runtime.total_play_ms = 7ULL * (uint64_t)CSB_V1_TICK_MS_NOMINAL;
    boot.runtime.party_state.PartyMapX = boot.runtime.party_x;
    boot.runtime.party_state.PartyMapY = boot.runtime.party_y;
    boot.runtime.party_state.PartyDirection = boot.runtime.party_dir;
    boot.runtime.party_state.LeaderIndex = boot.runtime.leader_index;
    boot.runtime.party_state.MagicCasterIndex = boot.runtime.magic_caster_index;

    if (csb_v1_runtime_save_game_to_path(&boot.runtime, save_path) !=
        CSB_V1_SAVE_OK) {
        csb_v1_boot_cleanup(&boot);
        return 0;
    }
    if (expected) {
        *expected = boot.runtime;
        expected->dungeon_handle = NULL;
    }
    csb_v1_boot_cleanup(&boot);
    return 1;
}

int main(void) {
    char fallback[512];
    char save_tmpl[] = "/tmp/firestaff_csb_m11_resume_XXXXXX";
    char save_path[560];
    const char* data_dir = csb_data_dir(fallback);
    CSB_V1_BootProfile preflight;
    CSB_V1_RuntimeProfile expected;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    CSB_V1_BootProfile* profile;

    check_incomplete_required_files_block_m11(
        "M11 blocks CSB launch when GRAPHICS.DAT is present without DUNGEON.DAT",
        1, 0);
    check_incomplete_required_files_block_m11(
        "M11 blocks CSB launch when DUNGEON.DAT is present without GRAPHICS.DAT",
        0, 1);
    check_incomplete_required_files_block_m11(
        "M11 blocks CSB launch when required filenames exist but hashes are unknown",
        1, 1);

    if (!data_dir || !data_dir[0]) {
        puts("skip: no CSB data directory configured");
        return g_failures == 0 ? 0 : 1;
    }

    csb_v1_boot_profile_init(&preflight);
    if (csb_v1_boot_scan_assets(&preflight, data_dir) != 0 ||
        !preflight.assets_verified) {
        printf("skip: no hash-verified CSB V1 profile at %s\n", data_dir);
        csb_v1_boot_cleanup(&preflight);
        return g_failures == 0 ? 0 : 1;
    }
    csb_v1_boot_cleanup(&preflight);

#ifdef _WIN32
    snprintf(save_path, sizeof(save_path), ".\\firestaff-csb-m11-resume.sav");
#else
    {
        int fd = mkstemp(save_tmpl);
        if (fd < 0) {
            fprintf(stderr, "FAIL: could not create temporary save path\n");
            return 1;
        }
        close(fd);
        snprintf(save_path, sizeof(save_path), "%s.sav", save_tmpl);
        remove(save_tmpl);
    }
#endif

    memset(&expected, 0, sizeof(expected));
    expect_true(build_runtime_resume_save(data_dir, save_path, &expected),
                "built CSB runtime save fixture from verified assets");

    fill_csb_launch_spec(&spec, data_dir, save_path);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 CSB verified-profile resume start succeeds");
    expect_true(view.active == 1, "M11 CSB view is active");
    expect_true(view.startedFromLauncher == 1, "M11 marks CSB launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_CSB_BOOT,
                "M11 source kind is CSB boot");
    expect_true(strcmp(view.sourceId, "csb") == 0,
                "M11 source id is csb");
    expect_true(view.csbBootProfile != NULL,
                "M11 stores a CSB boot profile");
    expect_true(view.csbState.level_loaded == 1,
                "M11 CSB mirror state reports level loaded");
    expect_true(view.csbState.party_x == expected.party_x &&
                view.csbState.party_y == expected.party_y &&
                view.csbState.party_dir == expected.party_dir,
                "M11 CSB mirror state follows resumed party pose");
    expect_true(view.csbState.tick_count == (int)expected.tick_count,
                "M11 CSB mirror state follows resumed tick count");

    profile = (CSB_V1_BootProfile*)view.csbBootProfile;
    if (profile) {
        expect_true(profile->assets_verified == 1,
                    "CSB boot profile remains hash verified");
        expect_true(strcmp(profile->game_id, "csb") == 0,
                    "CSB boot profile game id is csb");
        expect_true(profile->runtime.party_x == expected.party_x &&
                    profile->runtime.party_y == expected.party_y &&
                    profile->runtime.party_dir == expected.party_dir,
                    "CSB runtime restored party pose from savePath");
        expect_true(profile->runtime.magic_caster_index ==
                    expected.magic_caster_index,
                    "CSB runtime restored magic caster from savePath");
        expect_true(profile->runtime.game_time == expected.game_time,
                    "CSB runtime restored game time from savePath");
    }

    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "CSB M11 idle tick dispatches through the CSB runtime boundary");
    expect_true(view.csbState.tick_count == (int)expected.tick_count + 1,
                "CSB M11 mirror tick advances once");

    M11_GameView_Shutdown(&view);
    expect_true(view.csbBootProfile == NULL,
                "M11 shutdown clears CSB boot ownership");
    remove(save_path);

    if (g_failures) {
        fprintf(stderr, "CSB V1 M11 startup/resume gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("ok: CSB V1 M11 startup/resume gate");
    return 0;
}
