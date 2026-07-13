#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>
#include <string.h>

static int failures;

static void expect(int condition, const char *message) {
    if (!condition) {
        fprintf(stderr, "FAIL: %s\n", message);
        ++failures;
    }
}

static const char valid_trace[] =
    "NEXUS_PRS3_SH2_TRACE_V1\n"
    "menu_bpk_fnv1a64=1\n"
    "dm_bin_fnv1a64=2\n"
    "entry_index=1\n"
    "stream_offset=20\n"
    "stream_size=10\n"
    "expected_output_bytes=8\n"
    "first_opcode_pc=100\n"
    "last_opcode_pc=102\n"
    "opcode_first_sequence=10\n"
    "opcode_last_sequence=11\n"
    "decoder_return_sequence=12\n"
    "capture_completion_sequence=13\n"
    "opcode_fetch_count=2\n"
    "payload_read_bytes=4\n"
    "output_write_bytes=8\n"
    "output_fnv1a64=3\n"
    "decoder_returned_success=1\n"
    "capture_complete=1\n";

int main(void) {
    Nexus_V1_Prs3CaptureTraceSchemaReceipt receipt;
    char malformed[sizeof(valid_trace)];

    expect(nexus_v1_prs3_capture_trace_schema_parse(
               valid_trace, strlen(valid_trace), &receipt) && receipt.valid &&
               receipt.complete_evidence && !receipt.decoder_promotion_eligible,
           "complete trace schema is accepted without codec promotion");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "output_fnv1a64="), "output_missing=", 15U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "missing output fingerprint is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "stream_size=10"), "stream_size=x0", 14U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "malformed stream evidence is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "opcode_last_sequence=11"),
           "opcode_last_sequence=10", 23U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "non-increasing opcode fetch sequence is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "decoder_return_sequence=12"),
           "decoder_return_sequence=11", 26U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "decoder return before the final opcode fetch is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "capture_completion_sequence=13"),
           "capture_completion_sequence=12", 30U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "capture completion at decoder return is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "opcode_first_sequence=10"),
           "opcode_missing_sequence=10", 24U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "missing opcode fetch sequence is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "output_write_bytes=8"),
           "output_write_bytes=7", 20U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "output length mismatch is rejected");

    return failures ? 1 : 0;
}
