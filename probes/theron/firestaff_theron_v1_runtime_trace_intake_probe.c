#include "theron_v1_boot.h"

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
