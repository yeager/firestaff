#include "theron_v1_irq2_live_trace_gate.h"

#include <stdio.h>
#include <string.h>

static int g_fail;
static int g_skip;

static void check(int condition, const char *name) {
    if (!condition) {
        ++g_fail;
        printf("[FAIL] %s\n", name);
    } else {
        printf("[PASS] %s\n", name);
    }
}

int main(void) {
    static const char valid_capture[] =
        "source=mednafen-pce-instrumented\n"
        "boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4 cd_1800=90\n"
        "post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27\n"
        "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 variant=us_bin record=0004e0\n"
        "dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00\n";
    static const char unrecognized_capture[] =
        "source=mednafen-pce-instrumented\n"
        "boot_pc=e98a physical_pc=0000e98a instruction=LDA $22A4 cd_1800=90\n"
        "post_e98a_controller_transfer_source_pc=e98e source_physical_pc=0000e98e instruction=JSR $EA27 next_pc=ea27 next_physical_pc=0000ea27\n"
        "dynamic_cd_read_transaction pc=4090 return_pc=4093 sector_count=01 destination=3800 record_register_mask=07 variant=unrecognized record=000001\n"
        "dynamic_cd_read_controller_state pc=e74c f5_after_cd_read=00 f5_at_irq2_entry=00 status_1802=00 status_1803=00 f2_before_merge=00 f2_at_branch=00\n";
    static const char controller_wait_capture[] =
        "source=mednafen-pce-instrumented\n"
        "post_latch_cd_baseline_pc=c897 cd_1800=d0 cd_1801=00 cd_1802=00 cd_1803=02 cd_1804=00\n"
        "c860_window_pc=c8c4 physical_pc=0000c8c4 instruction=LDA $222D  @ $222D = $00\n";
    Theron_V1Irq2LiveTrace absent_trace;
    Theron_V1Irq2LiveTrace parsed_trace;
    Theron_V1Irq2LiveBranchReceipt receipt;
    Theron_V1SystemCardControllerWaitReceipt wait_receipt;

    memset(&absent_trace, 0, sizeof(absent_trace));
    check(!theron_v1_irq2_live_branch_from_trace(&absent_trace, &receipt) &&
              !receipt.valid,
          "missing live trace cannot select an IRQ2 branch");
    check(theron_v1_irq2_live_trace_from_mednafen_capture(
              valid_capture, &parsed_trace) &&
              parsed_trace.variant == THERON_TRACK02_VARIANT_US_BIN &&
              parsed_trace.stage3_track02_record == 0x0004e0u &&
              theron_v1_irq2_live_branch_from_trace(&parsed_trace, &receipt) &&
              receipt.valid,
          "combined Mednafen receipt reaches the bounded Track 02 live-trace gate");
    check(!theron_v1_irq2_live_trace_from_mednafen_capture(
              "source=mednafen-pce-instrumented\n", &parsed_trace) &&
              !parsed_trace.magic,
          "missing combined receipt remains blocked");
    check(!theron_v1_irq2_live_trace_from_mednafen_capture(
              unrecognized_capture, &parsed_trace) && !parsed_trace.magic,
          "unrecognised dynamic record remains blocked");
    check(theron_v1_system_card_controller_wait_from_mednafen_capture(
              controller_wait_capture, &wait_receipt) && wait_receipt.valid &&
              wait_receipt.runtime_blocked && wait_receipt.command_1800 == 0xd0u &&
              wait_receipt.response_1803 == 0x02u &&
              wait_receipt.controller_state_222d == 0u,
          "authentic System Card controller wait is typed but cannot authorize runtime");
    check(!theron_v1_system_card_controller_wait_from_mednafen_capture(
              valid_capture, &wait_receipt) && !wait_receipt.valid,
          "a dynamic Track 02 capture is not misclassified as a controller wait");
    ++g_skip;
    printf("[SKIP] no authenticated Mednafen IRQ2 trace staged; branch remains unbound\n");
    printf("--- %d failed, %d skipped ---\n", g_fail, g_skip);
    return g_fail ? 1 : 0;
}
