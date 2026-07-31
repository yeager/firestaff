#include "dm2_v1_boot.h"
#include "dm2_v1_asset_loader.h"
#include "dm2_v1_runtime.h"
#include "dm2_v1_sound.h"
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

static int write_bytes(const char *path, const uint8_t *bytes, size_t size)
{
    FILE *f = fopen(path, "wb");
    if (!f) return 0;
    if (fwrite(bytes, 1, size, f) != size) {
        fclose(f);
        return 0;
    }
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

static int test_dm2_v2_render_fallback_callback(int party_dir,
                                                int party_x,
                                                int party_y,
                                                unsigned char *framebuffer,
                                                int fb_stride,
                                                int view_w,
                                                int view_h,
                                                void *userdata)
{
    int *called = (int *)userdata;
    (void)party_dir;
    (void)party_x;
    (void)party_y;
    (void)framebuffer;
    (void)fb_stride;
    (void)view_w;
    (void)view_h;
    if (called) {
        *called += 1;
    }
    return -1;
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
    CHECK(dm2_v1_boot_scan_assets(&p, root) == -1,
          "scan_assets rejects unverified extracted-name fixtures");
    CHECK(p.graphics_path[0] == '\0' && p.dungeon_path[0] == '\0',
          "unverified fixture paths are not retained");
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

static void test_enter_admits_map_without_complete_record_graph(void)
{
    DM2_V1_BootProfile p;
    uint8_t dungeon[512];
    char path[256];

    memset(dungeon, 0, sizeof(dungeon));
    /* A bounded PC G1 map with no declared DB pools. Its map bytes are
     * valid, while the optional record graph remains unavailable. */
    dungeon[2] = 0x47;
    dungeon[3] = 0x31;
    dungeon[4] = 44;
    dungeon[6] = 1;
    dungeon[44 + 8] = 0;
    dungeon[44 + 9] = 0;
    dungeon[sizeof(dungeon) - 1] = 0x20;
    snprintf(path, sizeof(path), "/tmp/firestaff-dm2-g1-boot-%ld.dat",
             (long)TEST_GETPID());
    remove(path);
    CHECK(write_bytes(path, dungeon, sizeof(dungeon)) == 1,
          "writes bounded G1 map fixture");

    dm2_v1_boot_profile_init(&p);
    p.assets_verified = 1;
    snprintf(p.dungeon_path, sizeof(p.dungeon_path), "%s", path);
    CHECK(dm2_v1_boot_enter_game(&p) == 0 &&
              p.dm2_state != NULL && p.dungeon_data != NULL,
          "boot admits a real-format map before record graph promotion");
    CHECK(p.dungeon_data != NULL &&
              dm2_v1_dungeon_get_tile_raw(
                  (const DM2_V1_DungeonData *)p.dungeon_data,
                  0, 0, 0) == 0x20 &&
              !dm2_v1_dungeon_validate_record_graph(
                  (const DM2_V1_DungeonData *)p.dungeon_data),
          "record graph remains a separate runtime capability");
    dm2_v1_boot_cleanup(&p);
    remove(path);
}

static void test_startup_launch_alloc_missing_data(void)
{
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_StartupHostReceipt failure_receipt;
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
    memset(&failure_receipt, 0xff, sizeof(failure_receipt));
    CHECK(dm2_v1_boot_startup_prepare_failure_host_receipt(
              &launch,
              &failure_receipt) == 1 &&
              failure_receipt.input_result ==
                  DM2_V1_STARTUP_HOST_INPUT_IGNORED &&
              failure_receipt.status_scope != NULL &&
              strcmp(failure_receipt.status_scope, "BOOT") == 0 &&
              failure_receipt.status != NULL &&
              strcmp(failure_receipt.status, "DM2 ASSETS MISSING") == 0,
          "startup launch failure receipt is boot-owned and M11-ready");
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
    DM2_V1_BootRuntimeReceipt before;
    DM2_V1_BootRuntimeReceipt after;
    DM2_V1_BootRuntimeActionReceipt action;
    DM2_V1_BootRuntimeRenderReceipt render_receipt;
    DM2_V1_BootRuntimeHudCaptureReceipt hud_capture;
    DM2_V1_RuntimeFrameOwnershipReceipt frame_ownership;
    DM2_V1_BootCreatureAtlasCaptureReceipt creature_atlas;
    DM2_V1_CompleteSupportReceipt complete_support;
    unsigned char framebuffer[320 * 200];
    uint32_t typed_hash = 0u;
    uint32_t typed_bytes = 0u;
    int v2_callback_count = 0;
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
    {
        DM2_V1_MusicQueueReceipt music;
        CHECK(dm2_v1_sound_queue_music(0, 1, &music) ==
                  DM2_V1_MUSIC_QUEUE_DECODER_BACKEND_UNAVAILABLE &&
                  music.asset_resolved == 1 && music.decoder_proven == 0 &&
                  strcmp(music.asset_path,
                         "GRAPHICS.DAT::GDAT(04,00,03,00)") == 0,
              "startup boot binds the original GDAT title-music owner");
    }
    CHECK(dm2_v1_boot_gdat_typed_raw_asset_proof(
              launch.profile,
              DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
              0,
              DM2_GDAT_ENTRY_TYPE_PAL_IRGB,
              DM2_GDAT_INTERFACE_PALETTE_FIELD,
              0x32544439u,
              &typed_hash,
              &typed_bytes) == 1 &&
              typed_hash != 0u &&
              typed_bytes > 0u,
          "boot GDAT typed parser loads skproject dt09 palette data");
    CHECK(dm2_v1_boot_gdat_typed_raw_asset_proof(
              launch.profile,
              DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
              0,
              DM2_GDAT_ENTRY_TYPE_PAL_16,
              DM2_GDAT_INTERFACE_PALETTE_FIELD,
              0x32543136u,
              &typed_hash,
              &typed_bytes) == 1 &&
              typed_hash != 0u &&
              typed_bytes > 0u,
          "boot GDAT typed parser loads skproject dtPalette16 data");
    CHECK(dm2_v1_boot_gdat_typed_raw_asset_proof(
              launch.profile,
              DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
              0,
              DM2_GDAT_ENTRY_TYPE_RAW7,
              DM2_GDAT_INTERFACE_RAW_LAYOUT_TABLE,
              0x32544437u,
              &typed_hash,
              &typed_bytes) == 1 &&
              typed_hash != 0u &&
              typed_bytes > 0u,
          "boot GDAT typed parser loads skproject dt07 interface data");
    CHECK(dm2_v1_boot_gdat_typed_raw_asset_proof(
              launch.profile,
              DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
              0,
              DM2_GDAT_ENTRY_TYPE_RAW7,
              DM2_GDAT_INTERFACE_RAW_ACTION_TABLE,
              0x32543732u,
              &typed_hash,
              &typed_bytes) == 1 &&
              typed_hash != 0u &&
              typed_bytes > 0u,
          "boot GDAT typed parser loads skproject dt07 interface action table");
    CHECK(dm2_v1_boot_gdat_typed_raw_asset_proof(
              launch.profile,
              DM2_GDAT_CATEGORY_INTERFACE_GENERAL,
              0,
              DM2_GDAT_ENTRY_TYPE_RAW6,
              0,
              0x32544436u,
              &typed_hash,
              &typed_bytes) == 0 &&
              typed_hash == 0u &&
              typed_bytes == 0u,
          "boot GDAT typed parser keeps absent skproject dt06 isolated");
    memset(&before, 0, sizeof(before));
    CHECK(dm2_v1_boot_runtime_capture(launch.profile, &before) == 1 &&
              before.runtime_ready == 1 &&
              before.party_x == 3 && before.party_y == 5 &&
              before.party_dir == 2,
          "boot runtime capture owns source G1 party receipt");
    memset(&after, 0, sizeof(after));
    CHECK(dm2_v1_boot_runtime_tick(launch.profile, &after) == 1 &&
              after.runtime_ready == 1 &&
              after.tick_count >= before.tick_count &&
              after.operation_result == 0,
          "boot runtime tick owns DM2 receipt update");
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&render_receipt, 0, sizeof(render_receipt));
    CHECK(dm2_v1_boot_runtime_render_frame(
              launch.profile,
              framebuffer,
              320,
              320,
              200,
              test_dm2_v2_render_fallback_callback,
              &v2_callback_count,
              &render_receipt) == 1 &&
              v2_callback_count == 1 &&
              render_receipt.v2_attempted == 1 &&
              render_receipt.v1_attempted == 1 &&
              render_receipt.v1_succeeded == 1 &&
              render_receipt.render_result == 0 &&
              render_receipt.startup_title_ready == 1 &&
              render_receipt.startup_profile_verified == 1 &&
              render_receipt.startup_hud_runtime_ready == 1 &&
              render_receipt.startup_render_ready == 1 &&
              render_receipt.runtime_hud_capture_ready == 1 &&
              render_receipt.runtime_hud_real_asset_ready == 1 &&
              render_receipt.runtime_hud_asset_portrait_count == 0 &&
              render_receipt.runtime_hud_fallback_portrait_count == 0 &&
              render_receipt.runtime_hud_raw_gdat_capture_ready == 1 &&
              render_receipt.runtime_hud_raw_portrait_count >= 4 &&
              render_receipt.runtime_hud_raw_portrait_hash != 0u &&
              render_receipt.runtime_hud_raw_portrait_byte_count > 0u &&
              render_receipt.runtime_hud_raw_core_hash != 0u &&
              render_receipt.runtime_hud_raw_core_byte_count > 0u &&
              render_receipt.runtime_hud_raw_interface_count >= 4 &&
              render_receipt.runtime_hud_decoded_gdat_capture_ready == 1 &&
              render_receipt.runtime_hud_decoded_portrait_count >= 4 &&
              render_receipt.runtime_hud_decoded_portrait_hash != 0u &&
              render_receipt.runtime_hud_decoded_portrait_pixel_count > 0u &&
              render_receipt.runtime_hud_decoded_core_hash != 0u &&
              render_receipt.runtime_hud_decoded_core_pixel_count > 0u &&
              render_receipt.runtime_hud_decoded_interface_count >= 4 &&
              render_receipt.runtime_hud_frame_hash != 0u &&
              render_receipt.runtime_hud_frame_pixel_count == 320u * 200u &&
              render_receipt.runtime_render_real_asset_ready == 1 &&
              render_receipt.runtime_render_asset_floor_ceiling_count >= 2 &&
              render_receipt.runtime_render_fallback_floor_ceiling_count == 0 &&
              render_receipt.runtime_render_asset_wall_count > 0 &&
              render_receipt.runtime_render_fallback_wall_count == 0 &&
              render_receipt.runtime_render_fallback_door_count == 0 &&
              render_receipt.runtime_render_fallback_item_count == 0 &&
              render_receipt.runtime_render_fallback_carried_item_count == 0 &&
              render_receipt.runtime_render_no_core_fallbacks == 1,
          "boot runtime render owns source GDAT frame/HUD receipt without invented portraits");
    memset(framebuffer, 0, sizeof(framebuffer));
    memset(&render_receipt, 0, sizeof(render_receipt));
    CHECK(dm2_v1_boot_runtime_render_frame(
              launch.profile,
              framebuffer,
              320,
              320,
              200,
              NULL,
              NULL,
              &render_receipt) == 1 &&
              render_receipt.v2_attempted == 0 &&
              render_receipt.v2_succeeded == 0 &&
              render_receipt.v1_attempted == 1 &&
              render_receipt.v1_succeeded == 1 &&
              render_receipt.render_result == 0 &&
              render_receipt.runtime_render_real_asset_ready == 1 &&
              render_receipt.runtime_render_no_core_fallbacks == 1,
          "direct boot render consumes source G1/GDAT without a procedural V2 viewport");
    memset(&frame_ownership, 0, sizeof(frame_ownership));
    (void)dm2_v1_runtime_last_frame_ownership(&frame_ownership);
    CHECK(frame_ownership.real_gdat_evidence_valid == 1 &&
              frame_ownership.viewport_raw_gdat_asset_count >= 5 &&
              frame_ownership.viewport_decoded_gdat_asset_count >= 5 &&
              frame_ownership.viewport_raw_gdat_hash != 0u &&
              frame_ownership.viewport_decoded_gdat_hash != 0u &&
              frame_ownership.viewport_raw_gdat_byte_count > 0u &&
              frame_ownership.viewport_decoded_gdat_pixel_count > 0u &&
              frame_ownership.gdat_scene_control_ready == 1 &&
              frame_ownership.gdat_scene_control_consumed > 0 &&
              frame_ownership.gdat_scene_material_index == 2 &&
              frame_ownership.gdat_scene_light_consumed > 0 &&
              frame_ownership.gdat_scene_control_hash != 0u &&
              (frame_ownership.gdat_scene_control_present_mask & 0x03u) == 0x03u,
          "runtime frame ownership consumes real GDAT scene and light controls");
    memset(&hud_capture, 0, sizeof(hud_capture));
    CHECK(dm2_v1_boot_runtime_hud_capture_receipt(
              launch.profile,
              &hud_capture) >= 0 &&
              hud_capture.render_sample_count >= 3 &&
              hud_capture.render_success_count >= 3 &&
              (hud_capture.sampled_direction_mask & 0x07) == 0x07 &&
              hud_capture.runtime_turn_count >= 3 &&
              hud_capture.unique_frame_hash_count > 0 &&
              hud_capture.min_asset_portrait_count == 0 &&
              hud_capture.total_fallback_portrait_count == 0 &&
              hud_capture.min_asset_floor_ceiling_count >= 2 &&
              hud_capture.min_asset_wall_count > 0 &&
              hud_capture.total_fallback_floor_ceiling_count == 0 &&
              hud_capture.total_fallback_wall_count == 0 &&
              hud_capture.total_fallback_door_count == 0 &&
              hud_capture.total_fallback_item_count == 0 &&
              hud_capture.total_fallback_carried_item_count == 0 &&
              hud_capture.no_core_render_fallbacks == 1 &&
              hud_capture.no_fallback_portraits == 1 &&
              hud_capture.first_runtime_hud_ready == 1 &&
              hud_capture.real_gdat_portrait_ready == 1 &&
              hud_capture.real_gdat_core_render_ready == 1 &&
              (hud_capture.real_gdat_runtime_hud_breadth_ready == 1 ||
               hud_capture.render_sample_count >= 3) &&
              hud_capture.raw_gdat_runtime_interface_count >= 4 &&
              hud_capture.decoded_gdat_runtime_interface_count >= 4 &&
              hud_capture.teleporter_map_chip_ready == 1 &&
              hud_capture.teleporter_map_chip_raw_hash != 0u &&
              hud_capture.teleporter_map_chip_raw_byte_count > 0u &&
              hud_capture.teleporter_map_chip_decoded_hash != 0u &&
              hud_capture.teleporter_map_chip_decoded_pixel_count > 0u &&
              hud_capture.dungeon_map_chip_ready == 1 &&
              hud_capture.dungeon_map_chip_graphicsset_count > 0 &&
              hud_capture.dungeon_map_chip_wall_count > 0 &&
              hud_capture.dungeon_map_chip_floor_count > 0 &&
              hud_capture.dungeon_map_chip_graphicsset_ready == 1 &&
              hud_capture.dungeon_map_chip_wall_ready == 1 &&
              hud_capture.dungeon_map_chip_floor_ready == 1 &&
              hud_capture.dungeon_map_chip_raw_hash != 0u &&
              hud_capture.dungeon_map_chip_raw_byte_count > 0u &&
              hud_capture.dungeon_map_chip_decoded_hash != 0u &&
              hud_capture.dungeon_map_chip_decoded_pixel_count > 0u &&
              hud_capture.dungeon_map_chip_graphicsset_raw_hash != 0u &&
              hud_capture.dungeon_map_chip_graphicsset_raw_byte_count > 0u &&
              hud_capture.dungeon_map_chip_graphicsset_decoded_hash != 0u &&
              hud_capture.dungeon_map_chip_graphicsset_decoded_pixel_count > 0u &&
              hud_capture.dungeon_map_chip_wall_raw_hash != 0u &&
              hud_capture.dungeon_map_chip_wall_raw_byte_count > 0u &&
              hud_capture.dungeon_map_chip_wall_decoded_hash != 0u &&
              hud_capture.dungeon_map_chip_wall_decoded_pixel_count > 0u &&
              hud_capture.dungeon_map_chip_floor_raw_hash != 0u &&
              hud_capture.dungeon_map_chip_floor_raw_byte_count > 0u &&
              hud_capture.dungeon_map_chip_floor_decoded_hash != 0u &&
              hud_capture.dungeon_map_chip_floor_decoded_pixel_count > 0u &&
              (hud_capture.graphicsset_word_values_ready == 0 ||
               (hud_capture.graphicsset_word_values_hash != 0u &&
                hud_capture.graphicsset_word_values_query_count >= 4u &&
                /* skproject only requires the scene's present word values;
                 * this real set has no optional AMBIANT_LIGHT row. */
                (hud_capture.graphicsset_word_values_present_mask & 0x13u) == 0x13u)) &&
              hud_capture.wall_gfx_image_offsets_ready == 1 &&
              hud_capture.wall_gfx_image_offsets_hash != 0u &&
              hud_capture.wall_gfx_image_offsets_query_count > 0u &&
              hud_capture.wall_gfx_image_offsets_nonzero_count > 0u &&
              hud_capture.wall_gfx_image_offsets_present_mask != 0u &&
              hud_capture.interface_action_table_ready == 1 &&
              hud_capture.interface_action_table_hash != 0u &&
              hud_capture.interface_action_table_byte_count > 0u &&
              hud_capture.interface_action_group_count > 0u &&
              hud_capture.interface_action_entry_count > 0u &&
              hud_capture.interface_font_table_ready == 1 &&
              hud_capture.interface_font_table_hash != 0u &&
              hud_capture.interface_font_table_byte_count == 0x300u &&
              hud_capture.interface_font_table_row_count == 6u &&
              hud_capture.interface_font_table_char_count == 128u &&
              hud_capture.interface_font_table_nonzero_byte_count > 0u &&
              hud_capture.interface_font_table_printable_char_count > 0u &&
              hud_capture.interface_palette_ready == 1 &&
              hud_capture.interface_palette_hash != 0u &&
              hud_capture.interface_palette_irgb_byte_count > 0u &&
              hud_capture.interface_palette_pal16_byte_count > 0u &&
              hud_capture.interface_palette_irgb_color_count > 0u &&
              hud_capture.interface_palette_pal16_color_count > 0u &&
              hud_capture.combined_frame_hash != 0u &&
              hud_capture.combined_pixel_count >= 3u * 320u * 200u,
          "boot runtime HUD capture proves real GDAT availability and frames across sampled directions");
    memset(&after, 0, sizeof(after));
    CHECK(dm2_v1_boot_runtime_capture(launch.profile, &after) == 1 &&
              after.party_x == 3 && after.party_y == 5 &&
              after.party_dir == 2,
          "directional HUD capture returns to the source G1 pose");
    {
        DM2_V1_InterfaceActionTable action_table;
        CHECK(dm2_v1_boot_interface_action_table(launch.profile,
                                                  &action_table) == 1 &&
                  action_table.valid == 1 &&
                  action_table.raw != NULL &&
                  action_table.raw_size > 0u &&
                  action_table.hash != 0u &&
                  action_table.group_count > 0u &&
                  action_table.entry_count > 0u &&
                  action_table.groups[0].primary_offset >=
                      1u + action_table.group_count &&
                  action_table.groups[0].secondary_offset >=
                      action_table.groups[0].primary_offset &&
                  action_table.tail_offset + action_table.tail_size ==
                      action_table.raw_size,
              "boot materializes skproject dt07/2 primary, secondary, and command-tail spans");
    }
    {
        DM2_V1_InterfaceRect14HostReceipt rect14_host;
        DM2_V1_LoadGdatInterface000AReceipt rect14_symbol;
        int rect14_ready = dm2_v1_boot_interface_rect14_host_receipt(
            launch.profile, &rect14_host);
        CHECK((rect14_ready == 0 && rect14_host.valid == 0) ||
                  (rect14_ready == 1 && rect14_host.valid == 1 &&
                   rect14_host.table_hash != 0u &&
                   rect14_host.row_count > 0u &&
                   rect14_host.placement_hash != 0u &&
                   rect14_host.placement_count >= rect14_host.row_count &&
                   rect14_host.rotated_cell_mask != 0u &&
                   rect14_host.max_stretched_size > 0u),
              "boot exposes optional skproject dt07/0A placements through the host receipt");
        CHECK((rect14_ready == 0 &&
               dm2_v1_boot_load_gdat_interface_00_0a_receipt(
                   launch.profile, &rect14_symbol) == 0) ||
              (dm2_v1_boot_load_gdat_interface_00_0a_receipt(
                   launch.profile, &rect14_symbol) == 1 &&
               rect14_symbol.valid &&
               rect14_symbol.host_receipt_consumed &&
               rect14_symbol.table_hash == rect14_host.table_hash &&
               rect14_symbol.row_count == rect14_host.row_count &&
               rect14_symbol.stride == 14u &&
               rect14_symbol.byte_count == rect14_host.row_count * 14u &&
               rect14_symbol.placement_hash == rect14_host.placement_hash &&
               rect14_symbol.placement_count == rect14_host.placement_count &&
               rect14_symbol.receipt_hash != 0u),
              "DM2_LOAD_GDAT_INTERFACE_00_0A consumes the exact Rect14 host proof");
    }
    CHECK(hud_capture.interface_rect14_ready == 0 ||
              (hud_capture.interface_rect14_hash != 0u &&
               hud_capture.interface_rect14_byte_count ==
                   hud_capture.interface_rect14_row_count * 14u &&
               hud_capture.interface_rect14_stride == 14u &&
               hud_capture.interface_rect14_row_count > 0u &&
               hud_capture.interface_rect14_image_field_count > 0u &&
               hud_capture.interface_rect14_stretch_field_count > 0u &&
               hud_capture.interface_rect14_placement_plan_ready == 1 &&
               hud_capture.interface_rect14_placement_hash != 0u &&
               hud_capture.interface_rect14_placement_count >=
                   hud_capture.interface_rect14_row_count &&
               hud_capture.interface_rect14_rotated_cell_mask != 0u &&
               hud_capture.interface_rect14_max_stretched_size > 0u),
          "boot runtime HUD capture validates optional skproject dt07/0x0A rect14 placement plan when present");
    memset(&creature_atlas, 0, sizeof(creature_atlas));
    CHECK(dm2_v1_boot_creature_atlas_capture_receipt(
              launch.profile,
              &creature_atlas) == 1 &&
              creature_atlas.valid == 1 &&
              creature_atlas.materialized_creature_index_count >= 4 &&
              creature_atlas.frame_parity_matrix_count >=
                  creature_atlas.materialized_creature_index_count * 16 &&
              creature_atlas.min_frame_count > 0 &&
              creature_atlas.raw_gdat_hash != 0u &&
              creature_atlas.raw_gdat_byte_count > 0u &&
              creature_atlas.decoded_gdat_hash != 0u &&
              creature_atlas.decoded_gdat_pixel_count > 0u &&
              creature_atlas.animation_attribution_count > 0 &&
              creature_atlas.animation_info_sequence_count > 0 &&
              creature_atlas.animation_frame_sequence_count > 0 &&
              creature_atlas.animation_table_hash != 0u &&
              creature_atlas.animation_table_byte_count > 0u &&
              creature_atlas.animation_table_ready == 1 &&
              creature_atlas.frame_parity_hash != 0u &&
              creature_atlas.atlas_material_hash != 0u,
          "boot creature atlas capture materializes skproject GDAT creature map-chip and animation-table routes");
    memset(&complete_support, 0, sizeof(complete_support));
    int complete_support_result =
        dm2_v1_boot_complete_support_receipt_from_runtime_state(
              launch.profile,
              1,
              launch.profile->save_root,
              1,
              1u,
              0,
              2,
              &complete_support);
    CHECK(complete_support_result == 1 &&
              complete_support.valid == 0 &&
              complete_support.save_corpus_scan_complete == 1 &&
              complete_support.save_corpus_hash != 0u &&
              complete_support.save_corpus_original_state_scan_complete == 1 &&
              complete_support.save_corpus_original_state_list_complete == 1 &&
              complete_support.save_corpus_original_state_candidate_count == 0 &&
              complete_support.save_corpus_original_state_parsed_candidate_count == 0 &&
              complete_support.save_corpus_original_state_rejected_candidate_count == 0 &&
              complete_support.save_corpus_original_state_hash != 0u &&
              complete_support.save_corpus_valid_candidate_count >=
                  complete_support.save_corpus_importable_candidate_count &&
              complete_support.complete_support_ready == 0 &&
              complete_support.complete_support_hash != 0u &&
              strcmp(complete_support.status, "complete-support-ready") != 0,
          "boot complete-support receipt blocks promotion until real original save state parses");
    memset(&action, 0, sizeof(action));
    CHECK(dm2_v1_boot_runtime_action_front_cell(
              launch.profile,
              after.party_dir,
              &action) == 1 &&
              action.runtime.runtime_ready == 1 &&
              action.status_scope != NULL &&
              strcmp(action.status_scope, "ACTION") == 0 &&
              action.status != NULL &&
              action.target_x >= 0 &&
              action.target_y >= 0,
          "boot runtime action owns DM2 front-cell receipt");
    memset(&after, 0, sizeof(after));
    CHECK(dm2_v1_boot_runtime_turn(launch.profile, 1, &after) == 1 &&
              after.operation_result == 0 &&
              after.party_dir == 3,
          "boot runtime turn owns DM2 receipt update");
    {
        int move_dir = after.party_dir;
        memset(&after, 0, sizeof(after));
        CHECK(dm2_v1_boot_runtime_move(launch.profile, move_dir, &after) == 1 &&
                  after.runtime_ready == 1 &&
                  (after.operation_result == 0 || after.operation_result == -1),
              "boot runtime move owns DM2 receipt update");
    }
    dm2_v1_boot_startup_launch_cleanup(&launch);
}

static void test_startup_host_facts_from_boot_profile(void)
{
    DM2_V1_BootProfile profile;
    DM2_V1_BootStartupLaunch launch;
    DM2_V1_BootRuntimeStartupSnapshot snapshot;
    DM2_V1_StartupHostFacts facts;
    DM2_V1_StartupLaunchReceipt launch_receipt;

    dm2_v1_boot_profile_init(&profile);
    dm2_v1_boot_set_save_root(&profile, "/tmp/firestaff-dm2-profile-saves");
    memset(&launch, 0, sizeof(launch));
    memset(&snapshot, 0, sizeof(snapshot));
    launch.profile = &profile;

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
              launch_receipt.host_receipt.status != NULL &&
              launch_receipt.runtime_handoff.valid &&
              strcmp(launch_receipt.runtime_handoff.animation,
                     "dm2-startup-menu") == 0 &&
              launch_receipt.runtime_handoff.initialize_hud_runtime == 1 &&
              launch_receipt.runtime_handoff.hud_runtime_ready == 1 &&
              launch_receipt.runtime_handoff.runtime_menu_ready == 1 &&
              launch_receipt.runtime_handoff.runtime_action_ready == 0 &&
              launch_receipt.runtime_handoff.first_hud_frame_ready == 0,
          "DM2 boot builds startup launch receipt with menu/HUD runtime handoff");
    snapshot.startup_menu_active = 0;
    snapshot.startup_save_root = "/tmp/firestaff-dm2-menu-saves";
    CHECK(dm2_v1_boot_startup_launch_from_launch_snapshot(
              &launch,
              &snapshot,
              &launch_receipt) == 1 &&
              launch_receipt.host_receipt.status != NULL,
          "DM2 boot builds startup launch receipt from launch-owned profile");
    CHECK(dm2_v1_boot_startup_launch_from_launch(
              &launch,
              &launch_receipt) == 1 &&
              launch_receipt.menu_state_receipt_valid &&
              launch_receipt.host_receipt.status != NULL,
          "DM2 boot builds initial startup launch receipt without M11 snapshot");
    launch.profile = NULL;
    CHECK(dm2_v1_boot_startup_launch_from_launch_snapshot(
              &launch,
              &snapshot,
              &launch_receipt) == 0,
          "DM2 boot launch-owned wrapper rejects missing profile");
    CHECK(dm2_v1_boot_startup_launch_from_launch(
              &launch,
              &launch_receipt) == 0,
          "DM2 boot initial launch wrapper rejects missing profile");
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
    printf("\n--- test_enter_admits_map_without_complete_record_graph ---\n");
    test_enter_admits_map_without_complete_record_graph();
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
