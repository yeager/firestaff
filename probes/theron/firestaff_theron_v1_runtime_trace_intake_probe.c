#include "theron_v1_boot.h"
#include "theron_v1_startup_runtime_entry.h"

#include <stdio.h>
#include <string.h>

static int g_failures;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_failures;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

int main(void) {
    Theron_V1_BootTrack02RuntimeTraceIntakeReceipt receipt;
    Theron_V1_BootProfile profile;
    Theron_StartupFlow flow;
    Theron_V1_World world;
    Theron_StartupActionPlan plan;
    Theron_V1StartupRuntimeEntryResult runtime_result;
    Theron_StartupHostReceipt host_receipt;
    Theron_StartupStateReceipt state_receipt;
    Theron_V1_BootStartupLaunch launch;
    TrAssetBundle assets;
    Theron_V1_BootProfile profile_before;
    unsigned char hucard_byte = 0u;
    char runtime_receipt[128];

    theron_v1_boot_profile_init(&profile);
    check(!theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
              &profile),
          "Soul Room forcefield route stays closed without a live receipt");
    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    profile.track02_runtime_trace_handoff_ready = 1;
    profile.track02_runtime_trace_handoff.valid = 1;
    profile.track02_runtime_trace_handoff.variant = THERON_TRACK02_VARIANT_US_BIN;
    profile.track02_runtime_trace_handoff.stage3_track02_record = 0x4e0u;
    profile.track02_runtime_trace_handoff.stage3_user_data_hash = 1u;
    profile.track02_runtime_trace_handoff.cd_read_record_cl = 0xe0u;
    profile.track02_runtime_trace_handoff.cd_read_record_dl = 0x04u;
    profile.track02_runtime_trace_handoff.cd_read_record_ch = 0x00u;
    profile.track02_runtime_trace_handoff.stage3_raw_sector = 1u;
    profile.track02_runtime_trace_handoff.stage3_user_data_offset =
        THERON_TRACK02_RAW_SECTOR_BYTES + THERON_TRACK02_RAW_USER_DATA_OFFSET;
    profile.track02_runtime_trace_handoff.stage3_user_data_bytes =
        THERON_TRACK02_IPL_STAGE2_DYNAMIC_PAYLOAD_BYTES;
    profile.track02_runtime_trace_handoff.handler_address = 0xe736u;
    profile.track02_runtime_trace_handoff.cd_state_address = 0xe742u;
    profile.track02_runtime_trace_handoff.cd_state_branch_address = 0xe74cu;
    profile.track02_runtime_trace_handoff.branch.valid = 1;
    check(!theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
              &profile),
          "Soul Room route rejects a receipt with no trace provenance hash");
    snprintf(profile.track02_runtime_system_card_md5,
             sizeof(profile.track02_runtime_system_card_md5), "%s",
             "ff1a674273fe3540ccef576376407d1d");
    snprintf(profile.track02_runtime_trace_md5,
             sizeof(profile.track02_runtime_trace_md5), "%s",
             "04a75036e9d520bb983c5ed03b8d0182");
    check(!theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
              &profile),
          "Stage-three provenance alone cannot open the Soul Room route");
    profile.track02_initial_level_handoff.valid = 1;
    profile.track02_initial_level_handoff.variant = THERON_TRACK02_VARIANT_US_BIN;
    snprintf(profile.track02_initial_level_handoff.track02_md5,
             sizeof(profile.track02_initial_level_handoff.track02_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    profile.track02_initial_level_handoff.observed_track02_record = 0x0b52u;
    profile.track02_initial_level_handoff.coalesced_loader_cd_receipt_proven = 1;
    profile.track02_initial_level_handoff.initial_level_record_proven = 1;
    profile.track02_initial_level_handoff.complete_initial_level_envelope_proven = 1;
    profile.track02_initial_level_handoff.receipt_hash = 1u;
    check(!theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
              &profile),
          "Soul Room route rejects a receipt without the bound full payload witness");
    check(!theron_v1_boot_track02_capture_admission_allows_initial_level(
              &profile, &hucard_byte, 1u,
              THERON_DUNGEON_1_AKUTUBA, 0),
          "dungeon admission rejects an incomplete capture receipt before route load");
    theron_v1_boot_profile_init(&profile);
    /* 341dd6c6a split the forcefield entry by variant: only raw JP/US BIN
     * profiles take the strict capture-admission refusal (empty/ISO
     * identities route elsewhere).  Pin the fixture to a raw US BIN
     * identity so it exercises the intended admission gate. */
    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    memset(&flow, 0, sizeof(flow));
    memset(&world, 0, sizeof(world));
    memset(&plan, 0, sizeof(plan));
    check(!theron_v1_startup_runtime_enter_from_forcefield_boot_profile_with_host_receipts(
              &flow, &world, NULL, 0u, &profile, NULL, 0, &plan,
              &runtime_result, &host_receipt, &state_receipt,
              runtime_receipt, sizeof(runtime_receipt)) &&
              runtime_result.result == THERON_STARTUP_ERR_DUNGEON_ENTRY &&
              host_receipt.status &&
              strstr(host_receipt.status,
                     "AUTHENTIC CAPTURE ADMISSION REQUIRED") != NULL &&
              strstr(host_receipt.status,
                     "fallback visuals blocked") != NULL,
          "forcefield entry refuses to mutate Soul Room without capture admission");
    memset(&receipt, 0xa5, sizeof(receipt));
    check(!theron_v1_boot_track02_runtime_trace_intake_from_files(
              "/missing/track02.bin", "f23601102138f87c33025877767ebf76",
              "/missing/syscard3.pce", "ff1a674273fe3540ccef576376407d1d",
              "/missing/mednafen.trace", "00000000000000000000000000000000",
              &receipt) && !receipt.valid && !receipt.trace_file_hash_verified &&
              !receipt.trace_file_consumed &&
              !receipt.system_card_file_hash_verified &&
              !receipt.runtime_handoff.valid,
          "missing explicit trace preserves the fail-closed Track 02 handoff");
    check(!theron_v1_boot_track02_runtime_trace_intake_from_files(
              NULL, NULL, NULL, NULL, NULL, NULL, &receipt) && !receipt.valid &&
              !receipt.trace_file_hash_verified && !receipt.trace_file_consumed &&
              !receipt.runtime_handoff.valid,
          "absent runtime evidence input cannot construct a handoff");

    /* A failed capture retry must not tear down a previously authenticated
     * Soul Room boundary.  The missing manifest reaches the real intake
     * rejection path after the launch/profile preflight. */
    theron_v1_boot_profile_init(&profile);
    memset(&launch, 0, sizeof(launch));
    memset(&assets, 0, sizeof(assets));
    assets.hucard_rom = &hucard_byte;
    launch.profile = &profile;
    launch.assets = &assets;
    profile.assets_verified = 1;
    snprintf(profile.graphics_path, sizeof(profile.graphics_path), "%s",
             "/missing/authentic-track02.bin");
    snprintf(profile.graphics_md5, sizeof(profile.graphics_md5), "%s",
             THERON_TRACK02_MD5_US_BIN);
    profile.track02_runtime_trace_handoff_ready = 1;
    snprintf(profile.track02_runtime_system_card_md5,
             sizeof(profile.track02_runtime_system_card_md5), "%s",
             "ff1a674273fe3540ccef576376407d1d");
    snprintf(profile.track02_runtime_trace_md5,
             sizeof(profile.track02_runtime_trace_md5), "%s",
             "04a75036e9d520bb983c5ed03b8d0182");
    profile.track02_initial_level_handoff.valid = 1;
    profile.track02_initial_level_handoff.variant = THERON_TRACK02_VARIANT_US_BIN;
    profile.track02_initial_level_handoff.observed_track02_record = 0x0b52u;
    profile_before = profile;
    check(!theron_v1_boot_startup_launch_apply_track02_initial_level_capture_manifest_from_file(
              &launch, "/missing/authentic-capture.manifest") &&
              memcmp(&profile, &profile_before, sizeof(profile)) == 0,
          "rejected capture retry preserves the prior authenticated handoff");
    printf("--- %d failed ---\n", g_failures);
    return g_failures ? 1 : 0;
}
