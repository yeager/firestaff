#include "csb_v1_boot.h"
#include "csb_v1_save_load_pc34_compat.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#ifdef _WIN32
#include <direct.h>
#define FS_TEST_MKDIR(path) _mkdir(path)
#else
#define FS_TEST_MKDIR(path) mkdir((path), 0700)
#endif

static int failures;
#define CHECK(x) do { if (!(x)) { fprintf(stderr, "FAIL: %s\n", #x); ++failures; } } while (0)

static int write_native_dungeon(const char *path)
{
    unsigned char bytes[32] = {0};
    FILE *file;

    bytes[0] = 1; bytes[1] = 0;
    bytes[2] = 16; bytes[3] = 0;
    bytes[4] = 3; bytes[5] = 3;
    bytes[6] = 10; bytes[7] = 0;
    bytes[10] = 1; bytes[12] = 1; bytes[14] = 1;
    bytes[16] = 1; bytes[18] = 2; bytes[20] = 1;
    bytes[22] = 1; bytes[24] = 1; bytes[26] = 1;
    file = fopen(path, "wb");
    if (!file) return 0;
    if (fwrite(bytes, 1u, sizeof(bytes), file) != sizeof(bytes)) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
}

static void test_native_f0435_provenance(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_BootOriginalSaveRuntimeReceipt_PC34 receipt;
    char directory[128];
    char dungeon_path[ASSET_PATH_MAX];
    char graphics_path[ASSET_PATH_MAX];
    char save_path[ASSET_PATH_MAX];
    uint32_t game_time = 0u;

    snprintf(directory, sizeof(directory), "/tmp/firestaff-csb-f0435-%ld",
             (long)getpid());
    (void)FS_TEST_MKDIR(directory);
    snprintf(dungeon_path, sizeof(dungeon_path), "%s/DUNGEON.DAT", directory);
    snprintf(graphics_path, sizeof(graphics_path), "%s/GRAPHICS.DAT", directory);
    snprintf(save_path, sizeof(save_path), "%s/native.fsav", directory);
    CHECK(write_native_dungeon(dungeon_path));
    csb_v1_boot_profile_init(&profile);
    snprintf(profile.asset_root, sizeof(profile.asset_root), "%s", directory);
    snprintf(profile.dungeon_path, sizeof(profile.dungeon_path), "%s", dungeon_path);
    snprintf(profile.graphics_path, sizeof(profile.graphics_path), "%s", graphics_path);
    snprintf(profile.dungeon_md5, sizeof(profile.dungeon_md5),
             "%s", "6695d2acebce49f95db1d8f3a5c733de");
    profile.assets_verified = 1;
    profile.graphics_verified = 1;
    profile.dungeon_verified = 1;
    profile.variant_id = CSB_V1_VARIANT_PC34_EN;
    profile.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;
    profile.entrance_map_index = 255u;
    CHECK(csb_v1_boot_enter_game(&profile) == 0);
    CHECK(csb_v1_boot_runtime_save_game_to_path_pc34(
              &profile, save_path, &game_time) == CSB_V1_SAVE_OK);
    CHECK(csb_v1_boot_runtime_load_original_save_receipt_pc34(
              &profile, save_path, &receipt) && receipt.valid &&
          receipt.native_header_fnv1a != 0u &&
          receipt.source_identity_hash != 0u &&
          strcmp(receipt.dungeon_md5, profile.dungeon_md5) == 0 &&
          csb_v1_boot_original_save_runtime_receipt_current_pc34(
              &profile, &receipt));
    receipt.native_header_fnv1a ^= 1u;
    CHECK(!csb_v1_boot_original_save_runtime_receipt_current_pc34(
              &profile, &receipt));
    receipt.native_header_fnv1a ^= 1u;
    receipt.dungeon_md5[0] = receipt.dungeon_md5[0] == '0' ? '1' : '0';
    CHECK(!csb_v1_boot_original_save_runtime_receipt_current_pc34(
              &profile, &receipt));
    receipt.dungeon_md5[0] = profile.dungeon_md5[0];
    receipt.source_identity_hash ^= 1u;
    CHECK(!csb_v1_boot_original_save_runtime_receipt_current_pc34(
              &profile, &receipt));
    csb_v1_boot_cleanup(&profile);
    (void)remove(save_path);
    (void)remove(dungeon_path);
    (void)rmdir(directory);
}

static void test_title_capture_uses_source_owned_chaos_step(void)
{
    CSB_V1_BootProfile profile;
    CSB_V1_BootStartupVisualSequenceCaptureReceipt_PC34 capture;
    CSB_V1_BootRuntimeStartupSnapshot_PC34 snapshot;
    CSB_V1_BootStartupRenderViewReceipt_PC34 view;
    int capture_ok;

    csb_v1_boot_profile_init(&profile);
    snprintf(profile.asset_root, sizeof(profile.asset_root), "%s", "/tmp");
    snprintf(profile.graphics_path, sizeof(profile.graphics_path), "%s",
             "/tmp/firestaff_csb_GRAPHICS.DAT");
    snprintf(profile.dungeon_path, sizeof(profile.dungeon_path), "%s",
             "/tmp/firestaff_csb_DUNGEON.DAT");
    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
             "61fbfd56887c8bfe85ba4fb306fc2861");
    snprintf(profile.dungeon_md5, sizeof(profile.dungeon_md5), "%s",
             "6695d2acebce49f95db1d8f3a5c733de");
    profile.assets_verified = 1;
    profile.graphics_verified = 1;
    profile.dungeon_verified = 1;
    profile.variant_id = CSB_V1_VARIANT_PC34_EN;
    profile.graphics_kind = CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS;

    CHECK(csb_v1_startup_title_source_step_for_frame_pc34(59) == 1u);
    CHECK(csb_v1_startup_title_source_step_for_frame_pc34(60) == 2u);
    CHECK(csb_v1_startup_title_source_step_for_frame_pc34(77) == 19u);
    CHECK(csb_v1_startup_title_source_step_for_frame_pc34(79) == 21u);
    capture_ok = csb_v1_boot_startup_visual_sequence_capture_receipt_from_profile_pc34(
        &profile, &capture);
    CHECK(capture_ok && capture.valid &&
          capture.real_asset_matched && capture.title_chaos_zoom_capture_ready &&
          capture.title_chaos_hold_capture_ready &&
          capture.source_title_chaos_zoom_ticks == 20 &&
          capture.title_sample_hashes[1] != 0u &&
          capture.title_sample_hashes[2] != 0u);
    memset(&snapshot, 0, sizeof(snapshot));
    snapshot.boot_profile = &profile;
    snapshot.entrance_active = 1;
    snapshot.entrance_source_step = csb_v1_startup_entrance_wait_stage_pc34();
    snapshot.title_active = 1;
    snapshot.title_frame = 79;
    snapshot.title_source_step = 21;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot, &view) && view.title_chaos_visible &&
          view.title_chaos_hold_visible && !view.title_chaos_zoom_visible &&
          view.title_phase_tick == 0 &&
          view.title_phase_tick_count ==
              csb_v1_startup_title_chaos_hold_ticks_pc34());
    snapshot.title_frame = 80;
    snapshot.title_source_step = 21;
    CHECK(csb_v1_boot_startup_render_view_receipt_from_snapshot_pc34(
              &snapshot, &view) && view.title_chaos_visible &&
          view.title_chaos_hold_visible &&
          !view.title_strikes_back_visible);
    CHECK(csb_v1_startup_title_stage_for_frame_pc34(100) ==
          CSB_V1_STARTUP_STAGE_TITLE_STRIKES_BACK_PC34);
}

static void test_entrance_delay_owns_no_door_frame(void)
{
    CSB_V1_StartupCommandState_PC34 state;

    memset(&state, 0, sizeof(state));
    state.entrance_active = 1;
    CHECK(csb_v1_startup_begin_door_opening_pc34(
              &state, CSB_V1_STARTUP_ENTRANCE_COMMAND_ENTER_DUNGEON_PC34) &&
          state.opening_active && state.opening_step == 0 &&
          state.opening_delay_ticks ==
              csb_v1_startup_entrance_pre_open_delay_ticks_pc34());
}

static void test_startup_receipt_clears_graphics_md5_drift(void)
{
    CSB_V1_StartupRealReceipt receipt;

    CHECK(csb_v1_startup_real_receipt_from_profile_fields(
              "/tmp", "/tmp/GRAPHICS.DAT", "/tmp/DUNGEON.DAT",
              "61fbfd56887c94adc26888a9491c6611",
              "6695d2acebce49f95db1d8f3a5c733de", 123u, 456u,
              CSB_V1_VARIANT_PC34_EN, CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS, 4,
              1, 1, 1, &receipt) && receipt.receipt_hash != 0u);
    receipt.graphics_md5[0] = receipt.graphics_md5[0] == '0' ? '1' : '0';
    CHECK(!csb_v1_startup_real_receipt_recompute_hash(&receipt) &&
          receipt.receipt_hash == 0u && receipt.receipt_hash_hex[0] == '\0');
}

static void test_startup_receipt_dungeon_drift_requires_reissue(void)
{
    CSB_V1_StartupRealReceipt receipt;

    CHECK(csb_v1_startup_real_receipt_from_profile_fields(
              "/tmp", "/tmp/GRAPHICS.DAT", "/tmp/DUNGEON.DAT",
              "61fbfd56887c94adc26888a9491c6611",
              "6695d2acebce49f95db1d8f3a5c733de", 123u, 456u,
              CSB_V1_VARIANT_PC34_EN, CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS, 4,
              1, 1, 1, &receipt) && receipt.receipt_hash != 0u);
    receipt.dungeon_md5[0] = receipt.dungeon_md5[0] == '0' ? '1' : '0';
    CHECK(!csb_v1_startup_real_receipt_recompute_hash(&receipt) &&
          receipt.invalidated && receipt.receipt_hash == 0u);
    receipt.dungeon_md5[0] = '6';
    CHECK(!csb_v1_startup_real_receipt_recompute_hash(&receipt));
    CHECK(csb_v1_startup_real_receipt_from_profile_fields(
              "/tmp", "/tmp/GRAPHICS.DAT", "/tmp/DUNGEON.DAT",
              "61fbfd56887c94adc26888a9491c6611",
              "6695d2acebce49f95db1d8f3a5c733de", 123u, 456u,
              CSB_V1_VARIANT_PC34_EN, CSB_V1_ASSET_GFX_ARCHIVE_GRAPHICS, 4,
              1, 1, 1, &receipt) && !receipt.invalidated &&
          receipt.receipt_hash != 0u);
}

int main(void)
{
    CSB_V1_SaveHeader built, read;
    unsigned char raw[512];
    CSB_V1_StartupRealReceipt receipt;
    CSB_V1_BootProfile profile;
    const char *root = getenv("FIRESTAFF_CSB_DATA_DIR");

    CHECK(csb_v1_save_header_build(&built, CSB_V1_SAVE_MAGIC_CSB, 0x1234u,
                                   1u, 2, 3, 0, 1, 2, 99u, 0u) == 0);
    memcpy(raw, &built, sizeof(raw));
    CHECK(csb_v1_save_header_read(&read, raw) == 0 &&
          read.Magic == CSB_V1_SAVE_MAGIC_CSB);
    raw[511] ^= 1u;
    CHECK(csb_v1_save_header_read(&read, raw) != 0);

    test_native_f0435_provenance();
    test_title_capture_uses_source_owned_chaos_step();
    test_entrance_delay_owns_no_door_frame();
    test_startup_receipt_clears_graphics_md5_drift();
    test_startup_receipt_dungeon_drift_requires_reissue();

    csb_v1_startup_real_receipt_init(&receipt);
    if (!root || !root[0]) {
        puts("SKIP: FIRESTAFF_CSB_DATA_DIR is not set");
        return failures ? 1 : 0;
    }
    if (
        csb_v1_startup_real_scan_and_receipt(root, 4, &receipt) !=
            CSB_V1_STARTUP_REAL_OK || !receipt.matched) {
        puts("SKIP: no known-hash CSB graphics+dungeon corpus");
        return failures ? 1 : 0;
    }
    CHECK(csb_v1_boot_profile_from_startup_real_receipt_pc34(&receipt, &profile));
    if (profile.assets_verified) csb_v1_boot_cleanup(&profile);
    receipt.graphics_md5[0] = receipt.graphics_md5[0] == '0' ? '1' : '0';
    CHECK(!csb_v1_boot_profile_from_startup_real_receipt_pc34(&receipt, &profile));
    receipt = (CSB_V1_StartupRealReceipt){0};
    CHECK(csb_v1_startup_real_scan_and_receipt(root, 4, &receipt) == CSB_V1_STARTUP_REAL_OK && receipt.matched);
    receipt.dungeon_md5[0] = receipt.dungeon_md5[0] == '0' ? '1' : '0';
    CHECK(!csb_v1_boot_profile_from_startup_real_receipt_pc34(&receipt, &profile));
    return failures ? 1 : 0;
}
