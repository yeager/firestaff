#include "theron_v1_original_command_capture.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static void path(char out[1024], const char *dir, const char *suffix) {
    snprintf(out, 1024, "%s/trace%s", dir, suffix);
}

int main(void) {
    const char *dir = getenv("THERON_ORIGINAL_COMMAND_CAPTURE_DIR");
    Theron_V1OriginalCommandCaptureRequest request = {0};
    Theron_V1OriginalCommandCaptureReceipt receipt;
    char accepted_trace_md5[33];
    char trace[1024], code[1024], before[1024], after[1024], consumer[1024], transition[1024];
    if (!dir || !dir[0]) return 77;
    path(trace, dir, ".command-ram");
    path(code, dir, ".command-code");
    path(before, dir, ".command-before.ram");
    path(after, dir, ".command-after.ram");
    path(consumer, dir, ".main-ram-consumer");
    path(transition, dir, ".transition");
    request.command_trace_path = trace;
    request.command_code_path = code;
    request.command_before_ram_path = before;
    request.command_after_ram_path = after;
    request.main_ram_consumer_trace_path = consumer;
    request.transition_receipt_path = transition;
    request.expected_mednafen_md5 = "3731a8a78f91c5cc355546b27e7ba418";
    request.expected_track02_md5 = "ceb02343868f80cec899e9b239aff2da";
    request.expected_system_card_md5 = "ff1a674273fe3540ccef576376407d1d";
    request.expected_autoload_state_md5 = "f17f377df210b4a3ae904a13fb85a7f0";
    if (!theron_v1_original_command_capture_admit(&request, &receipt) ||
        !receipt.admitted || !receipt.source_identity_verified ||
        !receipt.input_edge_verified || !receipt.command_window_verified ||
        !receipt.consumer_window_verified ||
        !receipt.snapshots_verified || receipt.semantic_publication_allowed ||
        receipt.command_type != 0x50u || receipt.click_x != 0x3cu ||
        receipt.click_y != 0x78u || receipt.command_write_count != 65536u ||
        receipt.completion_sequence != 308u ||
        receipt.consumer_source_read_count != 0u ||
        strcmp(receipt.consumer_trace_md5,
               "84634112aaa47e0ada4f86d453697aa6") ||
        strcmp(receipt.code_md5, "036f62625740c7c887b0c588f2fa175b") ||
        strcmp(receipt.before_ram_md5, "a321518da370a456a36758b7c54a0cf1") ||
        strcmp(receipt.after_ram_md5, "2c4953862f6f6d52dbe2985ad0f9cf39")) {
        fputs("FAIL: authentic original command capture\n", stderr);
        return 1;
    }
    snprintf(accepted_trace_md5, sizeof(accepted_trace_md5), "%s",
             receipt.trace_md5);
    request.expected_track02_md5 = "00000000000000000000000000000000";
    memset(&receipt, 0xa5, sizeof(receipt));
    if (theron_v1_original_command_capture_admit(&request, &receipt) ||
        receipt.admitted || receipt.semantic_publication_allowed) {
        fputs("FAIL: wrong Track02 identity admitted\n", stderr);
        return 1;
    }
    request.expected_track02_md5 = "ceb02343868f80cec899e9b239aff2da";
    request.main_ram_consumer_trace_path = NULL;
    memset(&receipt, 0xa5, sizeof(receipt));
    if (theron_v1_original_command_capture_admit(&request, &receipt) ||
        receipt.admitted || receipt.consumer_window_verified) {
        fputs("FAIL: missing command consumer trace admitted\n", stderr);
        return 1;
    }
    printf("PASS: type=%02x click=%02x/%02x completion=%u trace=%s "
           "semantics=blocked\n", 0x50, 0x3c, 0x78, 308u,
           accepted_trace_md5);
    return 0;
}
