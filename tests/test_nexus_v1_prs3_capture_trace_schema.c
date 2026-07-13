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
    "payload_first_read_sequence=10\n"
    "payload_last_read_sequence=11\n"
    "decoder_return_sequence=12\n"
    "capture_completion_sequence=13\n"
    "opcode_fetch_count=2\n"
    "payload_read_bytes=4\n"
    "output_write_bytes=8\n"
    "output_fnv1a64=3\n"
    "decoder_returned_success=1\n"
    "capture_complete=1\n";

static void put_be32(unsigned char *p, unsigned int value) {
    p[0] = (unsigned char)(value >> 24);
    p[1] = (unsigned char)(value >> 16);
    p[2] = (unsigned char)(value >> 8);
    p[3] = (unsigned char)value;
}

static unsigned long long fnv1a64(const unsigned char *data, size_t size) {
    unsigned long long hash = 1469598103934665603ULL;
    size_t i;
    for (i = 0U; i < size; ++i) {
        hash ^= (unsigned long long)data[i];
        hash *= 1099511628211ULL;
    }
    return hash;
}

static void make_bpk(unsigned char data[68]) {
    memset(data, 0, 68U);
    memcpy(data, "BPPK", 4U);
    put_be32(data + 4U, 68U);
    memcpy(data + 12U, "BMPD", 4U);
    put_be32(data + 20U, 1U);
    put_be32(data + 24U, 32U);
    data[44U] = 0U; data[45U] = 2U;
    data[47U] = 2U;
    data[51U] = 6U;
    memcpy(data + 52U, "PRS3", 4U);
    put_be32(data + 56U, 1U);
    put_be32(data + 60U, 4U);
    put_be32(data + 64U, 4U);
}

int main(void) {
    Nexus_V1_Prs3CaptureTraceSchemaReceipt receipt;
    Nexus_V1_Prs3CaptureAssetBindingReceipt binding;
    char malformed[sizeof(valid_trace)];
    char bound_trace[1024];
    unsigned char bpk[68];
    static const unsigned char dm_bin[] = {0x53, 0x48, 0x32, 0x21};

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
    memcpy(strstr(malformed, "payload_first_read_sequence=10"),
           "payload_first_read_sequence=09", 30U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "payload read before the first opcode fetch is rejected");

    snprintf(malformed, sizeof(malformed), "%s", valid_trace);
    memcpy(strstr(malformed, "payload_last_read_sequence=11"),
           "payload_last_read_sequence=12", 29U);
    expect(!nexus_v1_prs3_capture_trace_schema_parse(
               malformed, strlen(malformed), &receipt) && !receipt.valid,
           "payload read at decoder return is rejected");

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

    make_bpk(bpk);
    snprintf(bound_trace, sizeof(bound_trace),
        "NEXUS_PRS3_SH2_TRACE_V1\n"
        "menu_bpk_fnv1a64=%llx\n"
        "dm_bin_fnv1a64=%llx\n"
        "entry_index=0\nstream_offset=40\nstream_size=4\n"
        "expected_output_bytes=4\nfirst_opcode_pc=100\nlast_opcode_pc=102\n"
        "opcode_first_sequence=10\nopcode_last_sequence=11\n"
        "payload_first_read_sequence=10\npayload_last_read_sequence=11\n"
        "decoder_return_sequence=12\ncapture_completion_sequence=13\n"
        "opcode_fetch_count=2\npayload_read_bytes=4\noutput_write_bytes=4\n"
        "output_fnv1a64=3\ndecoder_returned_success=1\ncapture_complete=1\n",
        fnv1a64(bpk, sizeof(bpk)), fnv1a64(dm_bin, sizeof(dm_bin)));
    expect(nexus_v1_prs3_capture_trace_schema_parse(
               bound_trace, strlen(bound_trace), &receipt) &&
               nexus_v1_prs3_capture_trace_schema_bind_assets(
                   &receipt, bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
                   &binding) && binding.valid && binding.asset_bound_capture &&
               !binding.decoder_promotion_eligible,
           "capture binds only to its exact BPK stream and DM.BIN bytes");

    bpk[64U] ^= 1U;
    expect(!nexus_v1_prs3_capture_trace_schema_bind_assets(
               &receipt, bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
               &binding) && !binding.valid && !binding.menu_bpk_matches,
           "one BPK byte invalidates an otherwise complete capture");

    return failures ? 1 : 0;
}
