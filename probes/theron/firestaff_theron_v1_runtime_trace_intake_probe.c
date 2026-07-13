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
    char runtime_receipt[128];

    theron_v1_boot_profile_init(&profile);
    check(!theron_v1_boot_track02_runtime_trace_allows_soul_room_handoff(
              &profile),
          "Soul Room forcefield route stays closed without a live receipt");
    memset(&flow, 0, sizeof(flow));
    memset(&world, 0, sizeof(world));
    memset(&plan, 0, sizeof(plan));
    check(!theron_v1_startup_runtime_enter_from_forcefield_boot_profile_with_host_receipts(
              &flow, &world, NULL, 0u, &profile, NULL, 0, &plan,
              &runtime_result, &host_receipt, &state_receipt,
              runtime_receipt, sizeof(runtime_receipt)) &&
              runtime_result.result == THERON_STARTUP_ERR_DUNGEON_ENTRY &&
              host_receipt.status &&
              strcmp(host_receipt.status, "AUTHENTIC LIVE TRACE REQUIRED") == 0,
          "forcefield entry refuses to mutate Soul Room without live evidence");
    memset(&receipt, 0xa5, sizeof(receipt));
    check(!theron_v1_boot_track02_runtime_trace_intake_from_files(
              "/missing/track02.bin", "f23601102138f87c33025877767ebf76",
              "/missing/syscard3.pce", "ff1a674273fe3540ccef576376407d1d",
              "/missing/mednafen.trace", &receipt) && !receipt.valid &&
              !receipt.trace_file_consumed && !receipt.runtime_handoff.valid,
          "missing explicit trace preserves the fail-closed Track 02 handoff");
    check(!theron_v1_boot_track02_runtime_trace_intake_from_files(
              NULL, NULL, NULL, NULL, NULL, &receipt) && !receipt.valid &&
              !receipt.trace_file_consumed && !receipt.runtime_handoff.valid,
          "absent runtime evidence input cannot construct a handoff");
    printf("--- %d failed ---\n", g_failures);
    return g_failures ? 1 : 0;
}
