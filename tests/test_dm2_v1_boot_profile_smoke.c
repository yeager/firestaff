#include "dm2_v1_boot.h"
#include "dm2_v1_startup_menu.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>

#if defined(_WIN32)
#include <direct.h>
#include <process.h>
#define TEST_MKDIR(path) _mkdir(path)
#define TEST_RMDIR(path) _rmdir(path)
#define TEST_PATH_SEP "\\"
#define TEST_GETPID() _getpid()
#else
#include <unistd.h>
#define TEST_MKDIR(path) mkdir((path), 0700)
#define TEST_RMDIR(path) rmdir(path)
#define TEST_PATH_SEP "/"
#define TEST_GETPID() getpid()
#endif

static int passed;
static int failed;

#define CHECK(cond, msg) do { \
    if (cond) { passed++; printf("  PASS: %s\n", msg); } \
    else { failed++; printf("  FAIL: %s\n", msg); } \
} while (0)

static int write_file(const char *path, const char *bytes)
{
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    fputs(bytes, f);
    fclose(f);
    return 1;
}

static int copy_file_bytes(const char *src, const char *dst)
{
    unsigned char buf[8192];
    FILE *in = fopen(src, "rb");
    FILE *out;
    size_t n;
    if (!in) return 0;
    out = fopen(dst, "wb");
    if (!out) {
        fclose(in);
        return 0;
    }
    while ((n = fread(buf, 1, sizeof(buf), in)) > 0U) {
        if (fwrite(buf, 1, n, out) != n) {
            fclose(in);
            fclose(out);
            return 0;
        }
    }
    fclose(in);
    fclose(out);
    return 1;
}

static void test_defaults(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    CHECK(strcmp(p.game_id, "dm2") == 0, "game id is dm2");
    CHECK(p.platform == DM2_PLATFORM_PC_EN, "default platform is PC English");
    CHECK(strcmp(p.platform_label, "PC English") == 0, "platform label is PC English");
    CHECK(strcmp(p.version_id, "pc-en") == 0, "version id is pc-en");
    CHECK(p.deterministic.tick_rate_hz == 18, "V1 tick rate is 18 Hz");
    CHECK(p.deterministic.tick_rate_hz_frac == 2, "fractional tick rate encodes 18.2 Hz");
    CHECK(p.deterministic.tick_ms == 55, "V1 tick quantum is 55 ms");
    CHECK(p.deterministic.dungeon_move_speed == 0x0080, "dungeon move speed Q8=0.5 sq/tick");
    CHECK(p.deterministic.max_champions == 4, "max champions is 4");
    CHECK(p.deterministic.max_party_members == 5, "max party members is 5");
    CHECK(p.deterministic.max_levels == 28, "max levels is 28 (PC English)");
    CHECK(p.deterministic.dungeon_seed == 257, "default dungeon seed is 257");
}

static void test_scan_missing_data(void)
{
    DM2_V1_BootProfile p;
    char diag[1024];
    size_t n;

    dm2_v1_boot_profile_init(&p);
    CHECK(dm2_v1_boot_scan_assets(&p, "/tmp/firestaff-dm2-v1-no-assets") == -1,
          "missing data does not verify");
    CHECK(p.assets_verified == 0, "assets_verified remains false without files");
    CHECK(p.graphics_path[0] == '\0', "graphics_path remains empty");
    CHECK(p.dungeon_path[0] == '\0', "dungeon_path remains empty");
    n = dm2_v1_diagnostic_report(&p, diag, sizeof(diag));
    CHECK(n > 0U && strstr(diag, "DM2 V1 Boot Profile") != NULL,
          "diagnostic report is populated");
}

static void test_probe_available(void)
{
    CHECK(dm2_v1_boot_probe_available("/tmp/firestaff-dm2-v1-no-assets") == 0,
          "probe_available is false without both DM2 assets");
}

static void test_scan_nested_data_dir(void)
{
    DM2_V1_BootProfile p;
    char root[256];
    char data_dir[300];
    char graphics_path[340];
    char dungeon_path[340];
    const char *tmp = getenv("TMPDIR");

    if (!tmp || !tmp[0]) tmp = getenv("TEMP");
    if (!tmp || !tmp[0]) tmp = ".";

    snprintf(root, sizeof(root), "%s%sfirestaff-dm2-v1-data-%ld",
             tmp, TEST_PATH_SEP, (long)TEST_GETPID());
    snprintf(data_dir, sizeof(data_dir), "%s%sdata", root, TEST_PATH_SEP);
    snprintf(graphics_path, sizeof(graphics_path), "%s%sgraphics.dat", data_dir, TEST_PATH_SEP);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s%sdungeon.dat", data_dir, TEST_PATH_SEP);

    (void)TEST_RMDIR(data_dir);
    (void)TEST_RMDIR(root);
    CHECK(TEST_MKDIR(root) == 0, "temp root for nested data dir created");
    CHECK(TEST_MKDIR(data_dir) == 0, "temp data dir created");
    CHECK(write_file(graphics_path, "fake-dm2-graphics") == 1,
          "nested graphics.dat fixture written");
    CHECK(write_file(dungeon_path, "fake-dm2-dungeon") == 1,
          "nested dungeon.dat fixture written");

    dm2_v1_boot_profile_init(&p);
    CHECK(dm2_v1_boot_scan_assets(&p, root) == 0,
          "scan_assets accepts extracted DOS data/ layout");
    CHECK(strstr(p.graphics_path, "data") != NULL,
          "graphics_path points into data/ layout");
    CHECK(strstr(p.dungeon_path, "data") != NULL,
          "dungeon_path points into data/ layout");
    CHECK(strstr(p.asset_root, "data") != NULL,
          "asset_root follows resolved data/ layout");
    CHECK(dm2_v1_boot_probe_available(root) == 0,
          "probe_available still rejects tiny synthetic fixtures");

    remove(graphics_path);
    remove(dungeon_path);
    (void)TEST_RMDIR(data_dir);
    (void)TEST_RMDIR(root);
}

static void test_scan_real_assets_by_hash_when_renamed(void)
{
    DM2_V1_BootProfile p;
    char root[256];
    char graphics_src[512];
    char dungeon_src[512];
    char graphics_dst[340];
    char dungeon_dst[340];
    const char *tmp = getenv("TMPDIR");
    const char *home = getenv("HOME");

    if (!tmp || !tmp[0]) tmp = getenv("TEMP");
    if (!tmp || !tmp[0]) tmp = ".";
    if (!home || !home[0]) {
        printf("  SKIP: HOME not set for optional real DM2 hash scan\n");
        return;
    }

    snprintf(graphics_src, sizeof(graphics_src), "%s/.firestaff/data/dm2/GRAPHICS.DAT", home);
    snprintf(dungeon_src, sizeof(dungeon_src), "%s/.firestaff/data/dm2/DUNGEON.DAT", home);
    {
        FILE *g = fopen(graphics_src, "rb");
        FILE *d = fopen(dungeon_src, "rb");
        if (!g || !d) {
            if (g) fclose(g);
            if (d) fclose(d);
            printf("  SKIP: optional real DM2 files not present\n");
            return;
        }
        fclose(g);
        fclose(d);
    }

    snprintf(root, sizeof(root), "%s%sfirestaff-dm2-hash-rename-%ld",
             tmp, TEST_PATH_SEP, (long)TEST_GETPID());
    snprintf(graphics_dst, sizeof(graphics_dst), "%s%snot-a-dm2-name.gfx", root, TEST_PATH_SEP);
    snprintf(dungeon_dst, sizeof(dungeon_dst), "%s%snot-a-dm2-name.map", root, TEST_PATH_SEP);

    (void)remove(graphics_dst);
    (void)remove(dungeon_dst);
    (void)TEST_RMDIR(root);
    CHECK(TEST_MKDIR(root) == 0, "temp root for renamed real DM2 files created");
    CHECK(copy_file_bytes(graphics_src, graphics_dst) == 1,
          "real DM2 graphics copied under arbitrary name");
    CHECK(copy_file_bytes(dungeon_src, dungeon_dst) == 1,
          "real DM2 dungeon copied under arbitrary name");

    dm2_v1_boot_profile_init(&p);
    CHECK(dm2_v1_boot_scan_assets(&p, root) == 0,
          "scan_assets accepts renamed real DM2 files by hash");
    CHECK(p.assets_verified == 1,
          "renamed real DM2 files are verified by hash");
    CHECK(strstr(p.graphics_path, "not-a-dm2-name.gfx") != NULL,
          "graphics_path is the arbitrary renamed file");
    CHECK(strstr(p.dungeon_path, "not-a-dm2-name.map") != NULL,
          "dungeon_path is the arbitrary renamed file");
    CHECK(dm2_v1_boot_probe_available(root) == 1,
          "probe_available accepts renamed real DM2 files by hash");

    remove(graphics_dst);
    remove(dungeon_dst);
    (void)TEST_RMDIR(root);
}

static void test_save_root_default(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    dm2_v1_boot_set_save_root(&p, NULL);
    CHECK(p.save_root[0] != '\0', "save_root is set from asset_root when NULL");
}

static void test_save_root_override(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    dm2_v1_boot_set_save_root(&p, "/tmp/firestaff-dm2-saves");
    CHECK(strcmp(p.save_root, "/tmp/firestaff-dm2-saves") == 0,
          "explicit save root is preserved");
}

static void test_enter_requires_assets(void)
{
    DM2_V1_BootProfile p;
    dm2_v1_boot_profile_init(&p);
    /* enter_game fails without verified assets (no files found) */
    CHECK(dm2_v1_boot_enter_game(&p) == -1,
          "enter_game rejects unverified profile (no files found)");
}

static void test_startup_launch_alloc_missing_data(void)
{
    DM2_V1_BootStartupLaunch launch;
    memset(&launch, 0, sizeof(launch));
    CHECK(dm2_v1_boot_startup_launch_alloc(
              "/tmp/firestaff-dm2-v1-no-assets", &launch) == 0,
          "startup launch allocation rejects missing assets");
    CHECK(launch.profile == NULL,
          "startup launch leaves profile NULL on missing assets");
    CHECK(launch.prepare_result ==
              DM2_V1_BOOT_STARTUP_PREPARE_SCAN_FAILED,
          "startup launch reports scan failure for missing data");
    CHECK(strcmp(dm2_v1_boot_startup_prepare_result_name(
                     launch.prepare_result),
                 "SCAN_FAILED") == 0,
          "startup launch failure has stable diagnostic name");
    CHECK(launch.failure_status_scope != NULL &&
              strcmp(launch.failure_status_scope, "BOOT") == 0,
          "startup launch missing-data failure has boot status scope");
    CHECK(launch.failure_status != NULL &&
              strcmp(launch.failure_status, "DM2 ASSETS MISSING") == 0,
          "startup launch missing-data failure has host status");
    dm2_v1_boot_startup_launch_cleanup(&launch);
}

static void test_startup_launch_detach_runtime_receipt(void)
{
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_BootStartupRuntimeReceipt receipt;

    memset(&launch, 0, sizeof(launch));
    memset(&receipt, 0xff, sizeof(receipt));
    CHECK(dm2_v1_boot_startup_launch_detach_runtime(NULL, &receipt) == 0 &&
              receipt.profile == NULL &&
              receipt.dm2_state == NULL,
          "startup runtime detach rejects NULL launch and clears receipt");

    launch.profile = (DM2_V1_BootProfile *)calloc(1, sizeof(*launch.profile));
    CHECK(launch.profile != NULL,
          "startup runtime detach fixture allocates profile");
    if (!launch.profile) {
        return;
    }
    dm2_v1_boot_profile_init(launch.profile);
    launch.profile->dm2_state = (void *)0x1234;
    snprintf(launch.profile->graphics_md5,
             sizeof(launch.profile->graphics_md5),
             "25247ede4dab4c8aa2c293241f8f909e");
    snprintf(launch.profile->dungeon_path,
             sizeof(launch.profile->dungeon_path),
             "/tmp/firestaff_dm2_DUNGEON.DAT");

    memset(&receipt, 0, sizeof(receipt));
    CHECK(dm2_v1_boot_startup_launch_detach_runtime(&launch,
                                                    &receipt) == 1 &&
              receipt.profile != NULL &&
              receipt.dm2_state == (void *)0x1234 &&
              strcmp(receipt.boot_asset_md5,
                     "25247ede4dab4c8aa2c293241f8f909e") == 0 &&
              strcmp(receipt.dungeon_path,
                     "/tmp/firestaff_dm2_DUNGEON.DAT") == 0 &&
              strcmp(receipt.title,
                     "DUNGEON MASTER II: SKULLKEEP") == 0 &&
              strcmp(receipt.source_id, "dm2") == 0 &&
              receipt.initialize_v2_runtime == 1 &&
              receipt.initialize_hud_runtime == 1 &&
              receipt.initialize_touch_runtime == 1 &&
              launch.profile == NULL,
          "startup runtime detach transfers DM2 profile and M11 identity");

    receipt.profile->dm2_state = NULL;
    dm2_v1_boot_cleanup(receipt.profile);
    free(receipt.profile);
}

static void test_startup_launch_alloc_real_assets_when_available(void)
{
    DM2_V1_BootStartupLaunch launch;
    const char *home = getenv("HOME");
    char root[512];
    FILE *g;
    FILE *d;
    if (!home || !home[0]) {
        printf("  SKIP: HOME not set for optional real DM2 startup launch\n");
        return;
    }
    snprintf(root, sizeof(root), "%s/.firestaff/data/dm2", home);
    {
        char graphics[600];
        char dungeon[600];
        snprintf(graphics, sizeof(graphics), "%s/GRAPHICS.DAT", root);
        snprintf(dungeon, sizeof(dungeon), "%s/DUNGEON.DAT", root);
        g = fopen(graphics, "rb");
        d = fopen(dungeon, "rb");
    }
    if (!g || !d) {
        if (g) fclose(g);
        if (d) fclose(d);
        printf("  SKIP: optional real DM2 files not present\n");
        return;
    }
    fclose(g);
    fclose(d);
    memset(&launch, 0, sizeof(launch));
    CHECK(dm2_v1_boot_startup_launch_alloc(root, &launch) == 1,
          "startup launch allocates with real verified DM2 assets");
    CHECK(launch.prepare_result == DM2_V1_BOOT_STARTUP_PREPARE_OK,
          "startup launch reports OK for real verified DM2 assets");
    CHECK(launch.profile != NULL && launch.profile->dm2_state != NULL,
          "startup launch enters DM2 game state");
    CHECK(launch.runtime_bound == 1,
          "startup launch binds the V1 runtime singleton");
    CHECK(launch.profile != NULL && launch.profile->graphics_dat != NULL,
          "startup launch loads DM2 graphics handle");
    dm2_v1_boot_startup_launch_cleanup(&launch);
}

static void test_startup_host_facts_from_boot_profile(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_StartupHostFacts facts;
    DM2_V1_StartupLaunchReceipt launch_receipt;

    dm2_v1_boot_profile_init(&profile);
    dm2_v1_boot_set_save_root(&profile, "/tmp/firestaff-dm2-profile-saves");

    CHECK(dm2_v1_boot_startup_host_facts_from_runtime_state(
              &profile,
              1,
              "/tmp/firestaff-dm2-menu-saves",
              1,
              (1u << 2),
              2,
              &facts) == 1,
          "DM2 boot builds startup host facts from profile plus runtime state");
    CHECK(facts.startup_menu_active == 1 &&
              strcmp(facts.save_root, "/tmp/firestaff-dm2-menu-saves") == 0 &&
              strcmp(facts.fallback_save_root,
                     "/tmp/firestaff-dm2-profile-saves") == 0 &&
              facts.resume_available == 1 &&
              facts.slot_mask == (1u << 2) &&
              facts.selected_row == 2 &&
              strcmp(facts.scan_save_root,
                     "/tmp/firestaff-dm2-profile-saves") == 0,
          "DM2 boot owns save-root fallback and scan-root facts");
    CHECK(dm2_v1_boot_startup_host_facts_from_runtime_state(
              NULL, 0, NULL, 0, 0u, 0, &facts) == 1 &&
              facts.startup_menu_active == 0 &&
              facts.save_root &&
              facts.save_root[0] == '\0' &&
              facts.fallback_save_root == NULL &&
              facts.scan_save_root == NULL,
          "DM2 boot host facts tolerate missing optional profile");
    CHECK(dm2_v1_boot_startup_host_facts_from_runtime_state(
              &profile, 1, "", 0, 0u, 0, NULL) == 0,
          "DM2 boot host facts reject NULL output");
    CHECK(dm2_v1_boot_startup_launch_from_runtime_state(
              &profile,
              0,
              "/tmp/firestaff-dm2-menu-saves",
              0,
              0u,
              0,
              &launch_receipt) == 1 &&
              launch_receipt.menu_state_receipt_valid &&
              launch_receipt.host_receipt.status != NULL,
          "DM2 boot builds startup launch receipt from runtime state");
    CHECK(dm2_v1_boot_startup_launch_from_runtime_state(
              &profile, 0, NULL, 0, 0u, 0, NULL) == 0,
          "DM2 boot startup launch wrapper rejects NULL receipt");
}

static void test_source_evidence(void)
{
    const char *e = dm2_v1_boot_source_evidence();
    CHECK(e != NULL && strstr(e, "SKULL.ASM T560") != NULL,
          "source evidence cites SKULL.ASM T560");
    CHECK(e != NULL && strstr(e, "25247ede4dabb6a71e5dabdfbcd5907d") != NULL,
          "source evidence cites DM2 PC English GRAPHICS hash");
}

int main(void)
{
    printf("=== DM2 V1 Boot Profile Smoke Test ===\n\n");
/* ── defaults ── */
    printf("--- test_defaults ---\n");
    test_defaults();
/* ── scan with no assets --─ */
    printf("\n--- test_scan_missing_data ---\n");
    test_scan_missing_data();
/* ── probe availability --─ */
    printf("\n--- test_probe_available ---\n");
    test_probe_available();
/* ── extracted DOS data/ layout --─ */
    printf("\n--- test_scan_nested_data_dir ---\n");
    test_scan_nested_data_dir();
/* ── hash-first renamed real assets --─ */
    printf("\n--- test_scan_real_assets_by_hash_when_renamed ---\n");
    test_scan_real_assets_by_hash_when_renamed();
/* ── save root --─ */
    printf("\n--- test_save_root_default ---\n");
    test_save_root_default();
    printf("\n--- test_save_root_override ---\n");
    test_save_root_override();
/* ── enter game guard --─ */
    printf("\n--- test_enter_requires_assets ---\n");
    test_enter_requires_assets();
/* ── startup launch helper --─ */
    printf("\n--- test_startup_launch_alloc_missing_data ---\n");
    test_startup_launch_alloc_missing_data();
    printf("\n--- test_startup_launch_detach_runtime_receipt ---\n");
    test_startup_launch_detach_runtime_receipt();
    printf("\n--- test_startup_launch_alloc_real_assets_when_available ---\n");
    test_startup_launch_alloc_real_assets_when_available();
    printf("\n--- test_startup_host_facts_from_boot_profile ---\n");
    test_startup_host_facts_from_boot_profile();
/* ── source evidence --─ */
    printf("\n--- test_source_evidence ---\n");
    test_source_evidence();

    printf("\nPASSED: %d\nFAILED: %d\n", passed, failed);
    return failed == 0 ? 0 : 1;
}
