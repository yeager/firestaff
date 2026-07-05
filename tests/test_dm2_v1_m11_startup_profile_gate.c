/*
 * test_dm2_v1_m11_startup_profile_gate.c -- DM2 V1 startup/profile M11 gate.
 *
 * Skip-safe real-data gate. Without local hash-verified DM2 data it exits 0;
 * with verified data it proves that M11_GameView_Start reaches the intended
 * DM2 V1 runtime boundary without claiming full playability.
 *
 * Source-lock: SKULL.ASM T520 (party placement after load), T560
 * (DUNGEON.DAT load completion). Firestaff boundary under test:
 * dm2_v1_boot_enter_game() -> M11_GAME_SOURCE_DM2_BOOT.
 */

#include "dm2_v1_boot.h"
#include "dm2_v1_game.h"
#include "dm2_v1_new_game.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_shop.h"
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

static int make_temp_dm2_root(char root[512], char dm2_dir[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_dm2_m11_profile_gate_%lu",
             (unsigned long)rand());
    if (TEST_MKDIR(root) != 0) {
        return 0;
    }
#else
    char tmpl[] = "/tmp/firestaff_dm2_m11_profile_gate_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
#endif
    snprintf(dm2_dir, 512, "%s%s%s", root, TEST_PATH_SEP, "dm2");
    if (TEST_MKDIR(dm2_dir) != 0) {
        (void)TEST_RMDIR(root);
        return 0;
    }
    return 1;
}

static void remove_temp_dm2_root(const char* root, const char* dm2_dir) {
    char graphics[512];
    char dungeon[512];
    snprintf(graphics, sizeof(graphics), "%s%sGRAPHICS.DAT",
             dm2_dir, TEST_PATH_SEP);
    snprintf(dungeon, sizeof(dungeon), "%s%sDUNGEON.DAT",
             dm2_dir, TEST_PATH_SEP);
    remove(graphics);
    remove(dungeon);
    (void)TEST_RMDIR(dm2_dir);
    (void)TEST_RMDIR(root);
}

static int make_temp_save_root(char root[512]) {
#ifdef _WIN32
    snprintf(root, 512, ".\\firestaff_dm2_m11_resume_%lu",
             (unsigned long)rand());
    return TEST_MKDIR(root) == 0;
#else
    char tmpl[] = "/tmp/firestaff_dm2_m11_resume_XXXXXX";
    char* made = mkdtemp(tmpl);
    if (!made) {
        return 0;
    }
    snprintf(root, 512, "%s", made);
    return 1;
#endif
}

static void remove_temp_save_root(const char* root) {
    char path[512];
    int i;
    for (i = 0; i < 10; ++i) {
        snprintf(path, sizeof(path), "%s%sSKSave%02d.dat",
                 root, TEST_PATH_SEP, i);
        remove(path);
    }
    snprintf(path, sizeof(path), "%s%sSKSave.bak", root, TEST_PATH_SEP);
    remove(path);
    (void)TEST_RMDIR(root);
}

static void fill_dm2_launch_spec(M11_GameLaunchSpec* spec,
                                 const char* data_dir) {
    memset(spec, 0, sizeof(*spec));
    spec->title = "DUNGEON MASTER II";
    spec->gameId = "dm2";
    spec->sourceId = "dm2";
    spec->dataDir = data_dir;
    spec->rendererBackend = M12_RENDERER_BACKEND_SOFTWARE;
    spec->presentationMode = M12_PRESENTATION_V1_ORIGINAL;
    spec->sourceKind = M11_GAME_SOURCE_BUILTIN_CATALOG;
}

static void check_incomplete_required_files_block_m11(const char* label,
                                                      int seed_graphics,
                                                      int seed_dungeon) {
    char root[512];
    char dm2_dir[512];
    char path[512];
    DM2_V1_BootProfile preflight;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;

    expect_true(make_temp_dm2_root(root, dm2_dir),
                "created isolated DM2 incomplete-data root");
    if (seed_graphics) {
        snprintf(path, sizeof(path), "%s%sGRAPHICS.DAT", dm2_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-dm2-graphics"),
                    "seeded synthetic GRAPHICS.DAT");
    }
    if (seed_dungeon) {
        snprintf(path, sizeof(path), "%s%sDUNGEON.DAT", dm2_dir, TEST_PATH_SEP);
        expect_true(write_tiny_file(path, "not-real-dm2-dungeon"),
                    "seeded synthetic DUNGEON.DAT");
    }

    dm2_v1_boot_profile_init(&preflight);
    if (seed_graphics && seed_dungeon) {
        expect_true(dm2_v1_boot_scan_assets(&preflight, root) == 0,
                    "preflight sees both synthetic DM2 required files");
        expect_true(preflight.assets_verified == 0,
                    "preflight refuses unverified synthetic DM2 pair");
    } else {
        expect_true(dm2_v1_boot_scan_assets(&preflight, root) != 0,
                    "preflight refuses incomplete DM2 required file pair");
        expect_true(preflight.assets_verified == 0,
                    "incomplete DM2 profile is not hash verified");
    }
    dm2_v1_boot_cleanup(&preflight);

    fill_dm2_launch_spec(&spec, root);
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                label);
    expect_true(view.active == 0,
                "M11 incomplete/unverified DM2 launch leaves view inactive");
    expect_true(view.dm2BootProfile == NULL,
                "M11 incomplete/unverified DM2 launch does not retain boot profile");
    expect_true(view.dm2World == NULL,
                "M11 incomplete/unverified DM2 launch does not retain world pointer");
    expect_true(view.sourceKind != M11_GAME_SOURCE_DM2_BOOT,
                "M11 incomplete/unverified DM2 launch does not claim DM2 boot source");
    expect_true(strstr(view.lastOutcome, seed_graphics && seed_dungeon
                                      ? "DM2 ENTER GAME FAILED"
                                      : "DM2 ASSETS MISSING") != NULL,
                "M11 reports the expected DM2 launch blocker status");
    M11_GameView_Shutdown(&view);
    remove_temp_dm2_root(root, dm2_dir);
}

static const char* dm2_data_dir(char fallback[512]) {
    const char* data_dir = getenv("FIRESTAFF_DM2_V1_DATA_DIR");
    const char* home;
    if (!data_dir || !data_dir[0]) {
        data_dir = getenv("FIRESTAFF_DM2_CANONICAL_DIR");
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

int main(void) {
    char fallback[512];
    const char* data_dir = dm2_data_dir(fallback);
    DM2_V1_BootProfile preflight;
    M11_GameLaunchSpec spec;
    M11_GameViewState view;
    DM2_V1_BootProfile* profile;
    DM2_V1_GameState* world;
    unsigned char framebuffer[320 * 200];
    char save_root[512];
    char save_path[512];
    DM2_V1_SessionState resume_session;

    check_incomplete_required_files_block_m11(
        "M11 blocks DM2 launch when GRAPHICS.DAT is present without DUNGEON.DAT",
        1, 0);
    check_incomplete_required_files_block_m11(
        "M11 blocks DM2 launch when DUNGEON.DAT is present without GRAPHICS.DAT",
        0, 1);
    check_incomplete_required_files_block_m11(
        "M11 blocks DM2 launch when required filenames exist but hashes are unknown",
        1, 1);

    if (!data_dir || !data_dir[0]) {
        puts("skip: no DM2 data directory configured");
        return g_failures == 0 ? 0 : 1;
    }

    dm2_v1_boot_profile_init(&preflight);
    if (dm2_v1_boot_scan_assets(&preflight, data_dir) != 0 ||
        !preflight.assets_verified) {
        printf("skip: no hash-verified DM2 V1 profile at %s\n", data_dir);
        return g_failures == 0 ? 0 : 1;
    }

    fill_dm2_launch_spec(&spec, data_dir);

    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 verified-profile start succeeds");
    expect_true(view.active == 1, "M11 view is active");
    expect_true(view.startedFromLauncher == 1, "M11 marks launcher start");
    expect_true(view.sourceKind == M11_GAME_SOURCE_DM2_BOOT,
                "M11 source kind is DM2 boot");
    expect_true(strcmp(view.sourceId, "dm2") == 0,
                "M11 source id is dm2");
    expect_true(view.dm2BootProfile != NULL,
                "M11 stores a DM2 boot profile");
    expect_true(view.dm2World != NULL,
                "M11 stores the DM2 V1 world pointer");
    expect_true(view.dm2State.level_loaded == 1,
                "M11 DM2 mirror state reports level loaded");
    expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                view.dm2State.party_dir == 0,
                "M11 DM2 mirror state reports the boot pose");
    expect_true(view.dm2State.tick_count == 0,
                "M11 DM2 mirror state starts at tick zero");
    expect_true(dm2_v1_runtime_has_dungeon_data() == 1,
                "DM2 V1 runtime singleton has the boot profile");
    expect_true(dm2_v1_runtime_get_party_x() == 15 &&
                dm2_v1_runtime_get_party_y() == 15 &&
                dm2_v1_runtime_get_party_dir() == 0,
                "DM2 V1 runtime accessors report the boot pose");
    expect_true(dm2_v1_runtime_get_tick_count() == 0,
                "DM2 V1 runtime tick counter starts at zero");

    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_RIGHT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 DM2 turn-right input redraws through runtime");
    expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                view.dm2State.party_dir == 1,
                "M11 DM2 turn-right rotates in place");
    expect_true(dm2_v1_runtime_get_party_x() == 15 &&
                dm2_v1_runtime_get_party_y() == 15 &&
                dm2_v1_runtime_get_party_dir() == 1,
                "DM2 runtime turn-right keeps boot position");
    expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_TURN_LEFT) ==
                    M11_GAME_INPUT_REDRAW,
                "M11 DM2 turn-left input redraws through runtime");
    expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                view.dm2State.party_dir == 0,
                "M11 DM2 turn-left restores boot facing");
    world = (DM2_V1_GameState*)view.dm2World;
    {
        int reputation_before = world ? world->reputation : -1;
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 action input redraws through runtime interaction");
        expect_true(strstr(view.lastOutcome, "DM2 SHOP") != NULL ||
                    strstr(view.lastOutcome, "DM2 INTERACT") != NULL ||
                    strstr(view.lastOutcome, "DM2 ACTUATOR") != NULL ||
                    strstr(view.lastOutcome, "DM2 NO TARGET") != NULL,
                    "M11 DM2 action reports a bounded runtime action status");
        expect_true(view.dm2State.party_x == 15 && view.dm2State.party_y == 15 &&
                    view.dm2State.party_dir == 0,
                    "M11 DM2 action does not move or rotate the party");
        if (world && reputation_before >= 0 &&
            strstr(view.lastOutcome, "DM2 INTERACT") != NULL) {
            expect_true(world->reputation == reputation_before + 1,
                        "DM2 runtime interaction mutates the boot-owned world");
        }
    }
    memset(framebuffer, 0, sizeof(framebuffer));
    M11_GameView_Draw(&view, framebuffer, 320, 200);
    expect_true(framebuffer[0] == 7,
                "M11 DM2 draw uses runtime viewport border, not text placeholder");
    expect_true(framebuffer[(199 * 320) + 319] == 1,
                "M11 DM2 draw preserves the runtime HUD strip after border draw");
    expect_true(framebuffer[(100 * 320) + 160] != 0,
                "M11 DM2 draw fills the runtime viewport body");

    profile = (DM2_V1_BootProfile*)view.dm2BootProfile;
    world = (DM2_V1_GameState*)view.dm2World;
    if (profile) {
        expect_true(profile->assets_verified == 1,
                    "boot profile remains hash verified");
        expect_true(strcmp(profile->game_id, "dm2") == 0,
                    "boot profile game id is dm2");
        expect_true(profile->dm2_state == view.dm2World,
                    "boot profile owns the M11 DM2 world pointer");
        expect_true(profile->dungeon_data != NULL,
                    "boot profile owns loaded dungeon data");
        expect_true(strcmp(profile->dungeon_path, view.dungeonPath) == 0,
                    "M11 dungeonPath mirrors the verified profile path");
    }
    if (world) {
        expect_true(world->party_x == 15 && world->party_y == 15 &&
                    world->party_dir == 0,
                    "DM2 V1 world starts at the source-locked boot pose");
        expect_true(world->current_level == 0,
                    "DM2 V1 world starts on level zero");
        world->gold = 375;
        dm2_v1_shop_reset_state();
        dm2_v1_runtime_set_position(0, 10, 6, 0);
        dm2_v1_runtime_set_outdoor(1);
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 action opens a runtime shop from the front square");
        expect_true(strstr(view.lastOutcome, "DM2 SHOP") != NULL,
                    "M11 DM2 shop action reports shop status");
        expect_true(dm2_v1_shop_is_active() == 1 &&
                    dm2_v1_shop_get_active_shop() == DM2_SHOP_ID_GENERAL,
                    "DM2 runtime shop action activates General Store");
        expect_true(dm2_v1_shop_get_party_gold() == 375u,
                    "DM2 runtime shop action syncs party gold into shop state");
        expect_true(view.dm2State.party_x == 10 && view.dm2State.party_y == 6 &&
                    view.dm2State.party_dir == 0,
                    "M11 DM2 shop action mirrors the runtime shop-facing pose");
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_ACTION) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 shop action buys the first stocked item");
        expect_true(strstr(view.lastOutcome, "DM2 SHOP BUY") != NULL,
                    "M11 DM2 shop buy reports transaction status");
        {
            int first_price =
                dm2_v1_shop_get_effective_price(DM2_SHOP_ID_GENERAL, 0);
            uint32_t expected_gold = 375u - (uint32_t)first_price;
            expect_true(dm2_v1_shop_get_party_gold() == expected_gold &&
                    dm2_v1_shop_get_state()->inventory_count == 1 &&
                    dm2_v1_shop_buy_count() == 1,
                    "DM2 shop buy mutates gold, inventory, and buy counter");
            expect_true(world->gold == (int)expected_gold,
                    "M11 DM2 shop buy mirrors gold back to the boot-owned world");
        }
        expect_true(M11_GameView_HandleInput(&view, M12_MENU_INPUT_BACK) ==
                        M11_GAME_INPUT_REDRAW,
                    "M11 DM2 back leaves active shop instead of returning to menu");
        expect_true(strstr(view.lastOutcome, "DM2 SHOP LEAVE") != NULL,
                    "M11 DM2 shop leave reports leave status");
        expect_true(dm2_v1_shop_is_active() == 0,
                    "DM2 shop leave clears active shop state");
    }

    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "DM2 M11 idle tick dispatches through the DM2 boundary");
    expect_true(view.dm2State.tick_count == 1,
                "DM2 M11 mirror tick advances once");
    expect_true(dm2_v1_runtime_get_tick_count() == 1,
                "DM2 V1 runtime tick advances once");
    expect_true(view.dm2State.party_x == dm2_v1_runtime_get_party_x() &&
                view.dm2State.party_y == dm2_v1_runtime_get_party_y() &&
                view.dm2State.party_dir == dm2_v1_runtime_get_party_dir(),
                "M11 DM2 mirror state stays aligned with runtime accessors");

    M11_GameView_Shutdown(&view);
    expect_true(view.dm2BootProfile == NULL && view.dm2World == NULL,
                "M11 shutdown clears DM2 boot ownership");

    expect_true(make_temp_save_root(save_root),
                "created isolated DM2 resume save root");
    memset(&resume_session, 0, sizeof(resume_session));
    dm2_v1_session_new(&resume_session);
    resume_session.game_tick = 42;
    resume_session.party_x = 23;
    resume_session.party_y = 11;
    resume_session.party_dir = 2;
    resume_session.party_level = 1;
    resume_session.outdoor_mode = 1;
    resume_session.time_of_day_minutes = 990;
    resume_session.rain_intensity = 60;
    expect_true(dm2_v1_session_save_slot(save_root,
                                         3,
                                         "M11 Resume",
                                         &resume_session) == 0,
                "wrote DM2 SKSave03.dat resume fixture");
    snprintf(save_path, sizeof(save_path), "%s%sSKSave03.dat",
             save_root, TEST_PATH_SEP);

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 savePath resume succeeds");
    expect_true(strstr(view.lastOutcome, "DM2 RESUMED") != NULL,
                "M11 DM2 savePath resume reports resumed status");
    expect_true(view.dm2State.party_x == 23 &&
                view.dm2State.party_y == 11 &&
                view.dm2State.party_dir == 2,
                "M11 DM2 resume mirrors saved party pose");
    expect_true(view.dm2State.tick_count == 42,
                "M11 DM2 resume mirrors saved game tick");
    expect_true(dm2_v1_runtime_get_party_x() == 23 &&
                dm2_v1_runtime_get_party_y() == 11 &&
                dm2_v1_runtime_get_party_dir() == 2,
                "DM2 runtime resume applies saved party pose");
    expect_true(dm2_v1_runtime_get_tick_count() == 42,
                "DM2 runtime resume applies saved tick");
    world = (DM2_V1_GameState*)view.dm2World;
    expect_true(world && world->current_level == 1 && world->outdoor == 1,
                "DM2 world resume applies saved level and outdoor flag");
    expect_true(M11_GameView_AdvanceIdleTick(&view) == M11_GAME_INPUT_REDRAW,
                "resumed DM2 idle tick still dispatches through runtime");
    expect_true(view.dm2State.tick_count == 43,
                "resumed DM2 tick advances from saved tick");
    M11_GameView_Shutdown(&view);

    resume_session.game_tick = 70;
    resume_session.party_x = 31;
    resume_session.party_y = 9;
    resume_session.party_dir = 3;
    resume_session.party_level = 2;
    resume_session.outdoor_mode = 0;
    resume_session.time_of_day_minutes = 450;
    resume_session.rain_intensity = 0;
    expect_true(dm2_v1_session_save_last_session(save_root,
                                                 "M11 Last",
                                                 &resume_session) == 0,
                "wrote DM2 SKSave.dat last-session fixture");
    snprintf(save_path, sizeof(save_path), "%s%sSKSave.dat",
             save_root, TEST_PATH_SEP);

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_path;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec),
                "M11 DM2 SKSave.dat resume succeeds");
    expect_true(strstr(view.lastOutcome, "DM2 RESUMED") != NULL,
                "M11 DM2 SKSave.dat reports resumed status");
    expect_true(view.dm2State.party_x == 31 &&
                view.dm2State.party_y == 9 &&
                view.dm2State.party_dir == 3,
                "M11 DM2 SKSave.dat mirrors saved party pose");
    expect_true(view.dm2State.tick_count == 70,
                "M11 DM2 SKSave.dat mirrors saved game tick");
    world = (DM2_V1_GameState*)view.dm2World;
    expect_true(world && world->current_level == 2 && world->outdoor == 0,
                "DM2 world SKSave.dat resume applies saved level");
    M11_GameView_Shutdown(&view);

    fill_dm2_launch_spec(&spec, data_dir);
    spec.savePath = save_root;
    M11_GameView_Init(&view);
    expect_true(M11_GameView_Start(&view, &spec) == 0,
                "M11 DM2 rejects invalid savePath shape");
    expect_true(strstr(view.lastOutcome, "DM2 RESUME PATH INVALID") != NULL,
                "M11 DM2 invalid savePath reports path blocker");
    expect_true(view.active == 0,
                "M11 DM2 invalid savePath leaves view inactive");
    M11_GameView_Shutdown(&view);
    remove_temp_save_root(save_root);

    if (g_failures) {
        fprintf(stderr, "DM2 V1 M11 startup/profile gate FAILED (%d failures)\n",
                g_failures);
        return 1;
    }
    puts("DM2 V1 M11 startup/profile gate passed");
    return 0;
}
