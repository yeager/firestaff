#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>
#include <string.h>

static int read_u64(const char **cursor, const char *label, uint64_t *out) {
    unsigned long long value;
    int consumed = 0;

    if (!cursor || !*cursor || !label || !out ||
        strncmp(*cursor, label, strlen(label)) != 0 ||
        sscanf(*cursor + strlen(label), "%llx%n", &value, &consumed) != 1 ||
        (*cursor)[strlen(label) + (size_t)consumed] != '\n') return 0;
    *out = (uint64_t)value;
    *cursor += strlen(label) + (size_t)consumed + 1U;
    return 1;
}

static int read_u32(const char **cursor, const char *label, uint32_t *out) {
    uint64_t value;
    if (!read_u64(cursor, label, &value) || value > UINT32_MAX) return 0;
    *out = (uint32_t)value;
    return 1;
}

int nexus_v1_prs3_capture_trace_schema_parse(
    const char *text, size_t text_size,
    Nexus_V1_Prs3CaptureTraceSchemaReceipt *out_receipt) {
    const char *cursor;
    uint32_t returned_success;
    uint32_t capture_complete;
    const size_t magic_size = sizeof(NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_MAGIC) - 1U;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    if (!text || text_size <= magic_size ||
        memcmp(text, NEXUS_V1_PRS3_CAPTURE_TRACE_SCHEMA_MAGIC, magic_size) != 0 ||
        text[magic_size] != '\n') return 0;
    cursor = text + magic_size + 1U;
    if (!read_u64(&cursor, "menu_bpk_fnv1a64=", &out_receipt->menu_bpk_fnv1a64) ||
        !read_u64(&cursor, "dm_bin_fnv1a64=", &out_receipt->dm_bin_fnv1a64) ||
        !read_u32(&cursor, "entry_index=", &out_receipt->entry_index) ||
        !read_u32(&cursor, "stream_offset=", &out_receipt->stream_offset) ||
        !read_u32(&cursor, "stream_size=", &out_receipt->stream_size) ||
        !read_u32(&cursor, "expected_output_bytes=", &out_receipt->expected_output_bytes) ||
        !read_u32(&cursor, "first_opcode_pc=", &out_receipt->first_opcode_pc) ||
        !read_u32(&cursor, "last_opcode_pc=", &out_receipt->last_opcode_pc) ||
        !read_u64(&cursor, "opcode_first_sequence=", &out_receipt->opcode_first_sequence) ||
        !read_u64(&cursor, "opcode_last_sequence=", &out_receipt->opcode_last_sequence) ||
        !read_u64(&cursor, "decoder_return_sequence=", &out_receipt->decoder_return_sequence) ||
        !read_u64(&cursor, "capture_completion_sequence=", &out_receipt->capture_completion_sequence) ||
        !read_u32(&cursor, "opcode_fetch_count=", &out_receipt->opcode_fetch_count) ||
        !read_u32(&cursor, "payload_read_bytes=", &out_receipt->payload_read_bytes) ||
        !read_u32(&cursor, "output_write_bytes=", &out_receipt->output_write_bytes) ||
        !read_u64(&cursor, "output_fnv1a64=", &out_receipt->output_fnv1a64) ||
        !read_u32(&cursor, "decoder_returned_success=", &returned_success) ||
        !read_u32(&cursor, "capture_complete=", &capture_complete) ||
        *cursor != '\0') return 0;
    if (!out_receipt->menu_bpk_fnv1a64 || !out_receipt->dm_bin_fnv1a64 ||
        !out_receipt->stream_size || !out_receipt->expected_output_bytes ||
        !out_receipt->first_opcode_pc || !out_receipt->last_opcode_pc ||
        !out_receipt->opcode_fetch_count || !out_receipt->payload_read_bytes ||
        (out_receipt->opcode_fetch_count == 1U
            ? out_receipt->opcode_first_sequence !=
                  out_receipt->opcode_last_sequence
            : out_receipt->opcode_first_sequence >=
                  out_receipt->opcode_last_sequence) ||
        out_receipt->opcode_last_sequence >=
            out_receipt->decoder_return_sequence ||
        out_receipt->decoder_return_sequence >=
            out_receipt->capture_completion_sequence ||
        out_receipt->payload_read_bytes > out_receipt->stream_size ||
        out_receipt->output_write_bytes != out_receipt->expected_output_bytes ||
        !out_receipt->output_fnv1a64 || returned_success != 1U ||
        capture_complete != 1U) return 0;
    out_receipt->valid = 1;
    out_receipt->complete_evidence = 1;
    /* This standalone syntax gate never connects to a codec. */
    out_receipt->decoder_promotion_eligible = 0;
    return 1;
}
