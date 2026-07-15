#include "nexus_v1_prs3_capture_trace_schema.h"
#include "nexus_v1_bpk_archive.h"
#include "asset_find_by_hash.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define NEXUS_V1_PRS3_CAPTURE_MENU_BPK_MD5 "c2776768ff25287c79013a1452253ca0"
#define NEXUS_V1_PRS3_CAPTURE_DM_BIN_MD5 "e88d60859f65f08fa622e1992b02280f"
#define NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES (1024U * 1024U)

static uint8_t *read_capture_file(const char *path, size_t max_size,
                                  size_t *out_size)
{
    FILE *file;
    long file_size;
    uint8_t *data;

    if (out_size) *out_size = 0U;
    if (!path || !out_size || !(file = fopen(path, "rb"))) return NULL;
    if (fseek(file, 0L, SEEK_END) != 0 || (file_size = ftell(file)) <= 0 ||
        (size_t)file_size > max_size || fseek(file, 0L, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (uint8_t *)malloc((size_t)file_size + 1U);
    if (!data || fread(data, 1U, (size_t)file_size, file) != (size_t)file_size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    data[file_size] = 0U;
    *out_size = (size_t)file_size;
    return data;
}

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

static uint64_t fnv1a64(const uint8_t *data, size_t size) {
    uint64_t hash = UINT64_C(1469598103934665603);
    size_t i;

    if (!data || size == 0U) return 0U;
    for (i = 0U; i < size; ++i) {
        hash ^= (uint64_t)data[i];
        hash *= UINT64_C(1099511628211);
    }
    return hash;
}

static int sh2_bra_target(uint16_t instruction, uint32_t instruction_offset,
                          uint32_t *out_target) {
    int32_t displacement;

    if (!out_target || (instruction & 0xf000U) != 0xa000U) return 0;
    displacement = (int32_t)(instruction & 0x0fffU);
    if ((displacement & 0x0800) != 0) displacement -= 0x1000;
    if (displacement < -((int32_t)instruction_offset + 2) / 2) return 0;
    *out_target = (uint32_t)((int32_t)instruction_offset + 4 +
                             displacement * 2);
    return 1;
}

static int sh2_movw_pc_literal_target(uint16_t instruction,
                                      uint32_t instruction_offset,
                                      uint32_t *out_target) {
    uint32_t displacement;

    if (!out_target || (instruction & 0xf000U) != 0x9000U) return 0;
    displacement = (uint32_t)(instruction & 0x00ffU) * 2U;
    *out_target = instruction_offset + 4U + displacement;
    return 1;
}

static int sh2_conditional_branch_target(uint16_t instruction,
                                         uint32_t instruction_offset,
                                         uint32_t *out_target) {
    int32_t displacement;

    if (!out_target || ((instruction & 0xff00U) != 0x8900U &&
                        (instruction & 0xff00U) != 0x8b00U &&
                        (instruction & 0xff00U) != 0x8d00U &&
                        (instruction & 0xff00U) != 0x8f00U)) return 0;
    displacement = (int32_t)(int8_t)(instruction & 0x00ffU);
    if (displacement < -((int32_t)instruction_offset + 4)) return 0;
    *out_target = (uint32_t)((int32_t)instruction_offset + 4 + displacement * 2);
    return 1;
}

static uint32_t read_be32(const uint8_t *p) {
    return ((uint32_t)p[0] << 24) | ((uint32_t)p[1] << 16) |
        ((uint32_t)p[2] << 8) | (uint32_t)p[3];
}

static uint16_t read_be16(const uint8_t *p) {
    return (uint16_t)(((uint16_t)p[0] << 8) | (uint16_t)p[1]);
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
        !read_u64(&cursor, "payload_first_read_sequence=", &out_receipt->payload_first_read_sequence) ||
        !read_u64(&cursor, "payload_last_read_sequence=", &out_receipt->payload_last_read_sequence) ||
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
        out_receipt->payload_first_read_sequence <
            out_receipt->opcode_first_sequence ||
        out_receipt->payload_first_read_sequence >
            out_receipt->payload_last_read_sequence ||
        out_receipt->payload_last_read_sequence >=
            out_receipt->decoder_return_sequence ||
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

int nexus_v1_prs3_capture_trace_schema_bind_assets(
    const Nexus_V1_Prs3CaptureTraceSchemaReceipt *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size,
    Nexus_V1_Prs3CaptureAssetBindingReceipt *out_receipt) {
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3CaptureAssetBindingReceipt receipt;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!trace || !trace->valid || !trace->complete_evidence ||
        !menu_bpk || menu_bpk_size == 0U || !dm_bin || dm_bin_size == 0U) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.trace_valid = 1;
    receipt.menu_bpk_matches =
        fnv1a64(menu_bpk, menu_bpk_size) == trace->menu_bpk_fnv1a64;
    receipt.dm_bin_matches =
        fnv1a64(dm_bin, dm_bin_size) == trace->dm_bin_fnv1a64;
    if (nexus_v1_bpk_archive_prs3_stream_plan(
            menu_bpk, menu_bpk_size, trace->entry_index, &plan) ==
        NEXUS_V1_BPK_PRS3_STREAM_OK) {
        receipt.entry_plan_matches =
            plan.stream_offset == trace->stream_offset &&
            plan.stream_size == trace->stream_size &&
            plan.expected_output_bytes == trace->expected_output_bytes;
    }
    receipt.asset_bound_capture =
        receipt.menu_bpk_matches && receipt.dm_bin_matches &&
        receipt.entry_plan_matches;
    receipt.valid = receipt.asset_bound_capture;
    /* A capture establishes a bounded observation, never an opcode grammar. */
    receipt.decoder_promotion_eligible = 0;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
    const char *text, size_t text_size,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3Sh2TransferTrace *out_trace,
    Nexus_V1_Prs3Sh2TransferReceipt *out_receipt) {
    const char *cursor;
    const size_t magic_size =
        sizeof(NEXUS_V1_PRS3_SH2_TRANSFER_TRACE_MAGIC) - 1U;
    Nexus_V1_Prs3Sh2TransferTrace trace;
    Nexus_V1_Prs3Sh2TransferReceipt receipt;
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;

    memset(&trace, 0, sizeof(trace));
    memset(&receipt, 0, sizeof(receipt));
    if (out_trace) *out_trace = trace;
    if (out_receipt) *out_receipt = receipt;
    if (!out_trace || !out_receipt || !text || text_size <= magic_size ||
        memcmp(text, NEXUS_V1_PRS3_SH2_TRANSFER_TRACE_MAGIC, magic_size) != 0 ||
        text[magic_size] != '\n') return 0;
    cursor = text + magic_size + 1U;
    if (!read_u64(&cursor, "menu_bpk_fnv1a64=", &trace.menu_bpk_fnv1a64) ||
        !read_u64(&cursor, "dm_bin_fnv1a64=", &trace.dm_bin_fnv1a64) ||
        !read_u32(&cursor, "entry_index=", &trace.entry_index) ||
        !read_u32(&cursor, "stream_offset=", &trace.stream_offset) ||
        !read_u32(&cursor, "stream_size=", &trace.stream_size) ||
        !read_u32(&cursor, "expected_output_bytes=", &trace.expected_output_bytes) ||
        !read_u32(&cursor, "payload_byte_offset=", &trace.payload_byte_offset) ||
        !read_u32(&cursor, "control_test_instruction_offset=", &trace.control_test_instruction_offset) ||
        !read_u32(&cursor, "zero_branch_instruction_offset=", &trace.zero_branch_instruction_offset) ||
        !read_u32(&cursor, "fallthrough_counter_decrement_offset=", &trace.fallthrough_counter_decrement_offset) ||
        !read_u32(&cursor, "observed_control_low_bit=", &trace.observed_control_low_bit) ||
        !read_u32(&cursor, "observed_zero_branch_taken=", &trace.observed_zero_branch_taken) ||
        !read_u32(&cursor, "input_instruction_offset=", &trace.input_instruction_offset) ||
        !read_u32(&cursor, "output_instruction_offset=", &trace.output_instruction_offset) ||
        !read_u64(&cursor, "input_read_sequence=", &trace.input_read_sequence) ||
        !read_u64(&cursor, "output_write_sequence=", &trace.output_write_sequence) ||
        !read_u32(&cursor, "output_byte_offset=", &trace.output_byte_offset) ||
        !read_u32(&cursor, "input_byte=", &trace.input_byte) ||
        !read_u32(&cursor, "output_byte=", &trace.output_byte) || *cursor != '\0' ||
        trace.input_byte > UINT8_MAX || trace.output_byte > UINT8_MAX ||
        trace.observed_control_low_bit > 1U || trace.observed_zero_branch_taken > 1U ||
        trace.input_read_sequence >= trace.output_write_sequence ||
        trace.output_byte_offset >= trace.expected_output_bytes) return 0;
    trace.valid = 1;
    receipt.trace_valid = 1;
    if (!menu_bpk || !dm_bin || !source_hash_verified ||
        !trace.menu_bpk_fnv1a64 || !trace.dm_bin_fnv1a64 ||
        fnv1a64(menu_bpk, menu_bpk_size) != trace.menu_bpk_fnv1a64 ||
        fnv1a64(dm_bin, dm_bin_size) != trace.dm_bin_fnv1a64 ||
        nexus_v1_bpk_archive_prs3_stream_plan(menu_bpk, menu_bpk_size,
                                               trace.entry_index, &plan) !=
            NEXUS_V1_BPK_PRS3_STREAM_OK ||
        nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, dm_bin_size, 1, &sh2) != 1) {
        *out_trace = trace; *out_receipt = receipt; return 0;
    }
    receipt.menu_bpk_matches = 1;
    receipt.dm_bin_matches = 1;
    receipt.entry_plan_matches = plan.stream_offset == trace.stream_offset &&
        plan.stream_size == trace.stream_size &&
        plan.expected_output_bytes == trace.expected_output_bytes;
    receipt.sh2_instruction_route_matches =
        trace.input_instruction_offset == sh2.stream_byte_read_offset &&
        trace.output_instruction_offset == sh2.output_byte_store_offset;
    receipt.nonzero_control_fallthrough_observed =
        trace.control_test_instruction_offset == sh2.control_low_bit_test_offset &&
        trace.zero_branch_instruction_offset == sh2.control_zero_branch_offset &&
        trace.fallthrough_counter_decrement_offset == sh2.control_zero_branch_offset + 2U &&
        trace.observed_control_low_bit == 1U &&
        trace.observed_zero_branch_taken == 0U;
    receipt.source_byte_matches = receipt.entry_plan_matches &&
        trace.payload_byte_offset < plan.stream_size &&
        menu_bpk[(size_t)plan.stream_offset + trace.payload_byte_offset] ==
            (uint8_t)trace.input_byte;
    receipt.observed_byte_transfer = receipt.source_byte_matches &&
        receipt.sh2_instruction_route_matches &&
        receipt.nonzero_control_fallthrough_observed &&
        trace.input_byte == trace.output_byte;
    /* External trace text alone cannot authenticate Saturn provenance. */
    receipt.original_saturn_provenance_verified = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_trace = trace;
    *out_receipt = receipt;
    return receipt.observed_byte_transfer;
}

int nexus_v1_prs3_token_grammar_nonzero_candidate_bind(
    const Nexus_V1_Prs3DecoderReadinessReceipt *decoder_review,
    const Nexus_V1_Prs3Vdp1CaptureReceipt *complete_trace,
    const Nexus_V1_Prs3Sh2TransferTrace *transfer_trace,
    const Nexus_V1_Prs3Sh2TransferReceipt *transfer_receipt,
    Nexus_V1_Prs3TokenGrammarReceipt *out_receipt) {
    Nexus_V1_Prs3TokenGrammarReceipt receipt;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!decoder_review || !complete_trace || !transfer_trace ||
        !transfer_receipt) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.decoder_review_bound = decoder_review->valid &&
        decoder_review->capture_source_bound &&
        decoder_review->complete_input_range_bound &&
        decoder_review->complete_output_range_bound &&
        decoder_review->control_branch_coverage_bound;
    receipt.nonzero_transfer_bound = transfer_receipt->observed_byte_transfer &&
        transfer_receipt->menu_bpk_matches && transfer_receipt->dm_bin_matches &&
        transfer_receipt->entry_plan_matches &&
        transfer_receipt->sh2_instruction_route_matches &&
        transfer_receipt->nonzero_control_fallthrough_observed &&
        complete_trace->entry_index == transfer_trace->entry_index &&
        complete_trace->stream_offset == transfer_trace->stream_offset &&
        complete_trace->stream_size == transfer_trace->stream_size &&
        complete_trace->expected_output_bytes == transfer_trace->expected_output_bytes;
    receipt.input_output_sequence_bound = receipt.nonzero_transfer_bound &&
        transfer_trace->input_read_sequence >=
            complete_trace->first_input_read_sequence &&
        transfer_trace->input_read_sequence <=
            complete_trace->last_input_read_sequence &&
        transfer_trace->output_write_sequence >=
            complete_trace->first_output_write_sequence &&
        transfer_trace->output_write_sequence <=
            complete_trace->last_output_write_sequence &&
        transfer_trace->payload_byte_offset < complete_trace->input_read_bytes &&
        transfer_trace->output_byte_offset < complete_trace->output_store_bytes;
    receipt.payload_byte_offset = transfer_trace->payload_byte_offset;
    receipt.output_byte_offset = transfer_trace->output_byte_offset;
    receipt.observed_input_byte = (uint8_t)transfer_trace->input_byte;
    receipt.observed_output_byte = (uint8_t)transfer_trace->output_byte;
    receipt.valid = receipt.decoder_review_bound && receipt.nonzero_transfer_bound &&
        receipt.input_output_sequence_bound;
    /* The current capture contracts bind bytes and control flow but do not
     * authenticate a Saturn producer or classify the byte as a PRS3 token. */
    receipt.original_saturn_execution_authenticated = 0;
    receipt.token_operation_proven = 0;
    receipt.opcode_grammar_proven = 0;
    receipt.decoder_ready = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_prs3_sh2_zero_side_trace_bind(
    const Nexus_V1_Prs3Sh2ZeroSideTrace *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3Sh2ZeroSideReceipt *out_receipt) {
    Nexus_V1_Prs3Sh2ZeroSideReceipt receipt;
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;
    uint32_t expected_merge;

    memset(&receipt, 0, sizeof(receipt));
    if (!out_receipt) return 0;
    *out_receipt = receipt;
    if (!trace || !menu_bpk || !dm_bin || !source_hash_verified ||
        trace->first_input_byte > UINT8_MAX || trace->second_input_byte > UINT8_MAX ||
        trace->first_input_read_sequence >= trace->second_input_read_sequence ||
        fnv1a64(menu_bpk, menu_bpk_size) != trace->menu_bpk_fnv1a64 ||
        fnv1a64(dm_bin, dm_bin_size) != trace->dm_bin_fnv1a64 ||
        nexus_v1_bpk_archive_prs3_stream_plan(menu_bpk, menu_bpk_size,
                                               trace->entry_index, &plan) !=
            NEXUS_V1_BPK_PRS3_STREAM_OK ||
        !nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, dm_bin_size, 1, &sh2)) return 0;
    receipt.menu_bpk_matches = receipt.dm_bin_matches = 1;
    receipt.entry_plan_matches = plan.stream_offset == trace->stream_offset &&
        plan.stream_size == trace->stream_size &&
        plan.expected_output_bytes == trace->expected_output_bytes;
    receipt.zero_side_instruction_route_matches =
        trace->zero_branch_instruction_offset == sh2.control_zero_branch_offset &&
        trace->zero_branch_target_offset == sh2.control_zero_branch_target_offset &&
        trace->counter_decrement_offset == trace->zero_branch_target_offset &&
        trace->first_input_instruction_offset == trace->zero_branch_target_offset + 8U &&
        trace->second_input_instruction_offset == trace->zero_branch_target_offset + 12U &&
        dm_bin[trace->counter_decrement_offset] == 0x7eU &&
        dm_bin[trace->counter_decrement_offset + 1U] == 0xfeU &&
        dm_bin[trace->first_input_instruction_offset] == 0x64U &&
        dm_bin[trace->first_input_instruction_offset + 1U] == 0xc4U &&
        dm_bin[trace->second_input_instruction_offset] == 0x67U &&
        dm_bin[trace->second_input_instruction_offset + 1U] == 0xc4U;
    receipt.source_bytes_match = receipt.entry_plan_matches &&
        trace->first_payload_byte_offset + 1U == trace->second_payload_byte_offset &&
        trace->second_payload_byte_offset < plan.stream_size &&
        menu_bpk[(size_t)plan.stream_offset + trace->first_payload_byte_offset] ==
            (uint8_t)trace->first_input_byte &&
        menu_bpk[(size_t)plan.stream_offset + trace->second_payload_byte_offset] ==
            (uint8_t)trace->second_input_byte;
    expected_merge = trace->first_input_byte |
        ((trace->second_input_byte << 4U) & 0x0f00U);
    receipt.observed_zero_side_merge = receipt.zero_side_instruction_route_matches &&
        receipt.source_bytes_match && trace->merged_control_value == expected_merge;
    receipt.original_saturn_provenance_verified = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.observed_zero_side_merge;
}

int nexus_v1_prs3_dm_bin_catalog_verified(
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3DmBinCatalogReceipt *out_receipt) {
    Nexus_V1_Prs3DmBinCatalogReceipt receipt;
    size_t offset;

    if (!out_receipt) return -1;
    memset(&receipt, 0, sizeof(receipt));
    if (!dm_bin || !source_hash_verified || dm_bin_size < 4U) {
        *out_receipt = receipt;
        return -1;
    }
    receipt.source_hash_verified = 1;
    for (offset = 0U; offset + 4U <= dm_bin_size; ++offset) {
        Nexus_V1_Prs3DmBinMarker *marker;
        if (memcmp(dm_bin + offset, "PRS3", 4U) != 0) continue;
        if (receipt.marker_count >= NEXUS_V1_PRS3_DM_BIN_MAX_MARKERS) {
            *out_receipt = receipt;
            return -1;
        }
        marker = &receipt.markers[receipt.marker_count++];
        marker->offset = (uint32_t)offset;
        if (dm_bin_size - offset < 16U) {
            ++receipt.truncated_marker_count;
            continue;
        }
        marker->header_complete = 1;
        marker->version = read_be32(dm_bin + offset + 4U);
        marker->declared_target_bytes = read_be32(dm_bin + offset + 8U);
        marker->first_frame_word = read_be32(dm_bin + offset + 12U);
        if (marker->version == 1U && marker->declared_target_bytes != 0U) {
            marker->kind = NEXUS_V1_PRS3_DM_BIN_MARKER_V1_RECORD;
            ++receipt.v1_record_count;
        } else {
            marker->kind = NEXUS_V1_PRS3_DM_BIN_MARKER_EXECUTABLE_BYTES;
            ++receipt.executable_marker_count;
        }
    }
    receipt.complete = receipt.marker_count > 0U &&
        receipt.truncated_marker_count == 0U;
    /* Framing cannot establish PRS3 opcodes or termination. */
    receipt.decoder_promoted = 0;
    *out_receipt = receipt;
    return 0;
}

int nexus_v1_prs3_cross_asset_frame_receipt_verified(
    const uint8_t *dm_bin, size_t dm_bin_size, int dm_bin_hash_verified,
    const uint8_t *menu_bpk, size_t menu_bpk_size, int menu_bpk_hash_verified,
    Nexus_V1_Prs3CrossAssetFrameReceipt *out_receipt) {
    Nexus_V1_Prs3CrossAssetFrameReceipt receipt;
    Nexus_V1_Prs3DmBinCatalogReceipt dm_catalog;
    Nexus_V1_BpkArchiveInfo menu_archive;
    uint32_t i;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!dm_bin || !menu_bpk || !dm_bin_hash_verified ||
        !menu_bpk_hash_verified ||
        nexus_v1_prs3_dm_bin_catalog_verified(
            dm_bin, dm_bin_size, 1, &dm_catalog) != 0 ||
        !dm_catalog.complete ||
        nexus_v1_bpk_archive_parse(menu_bpk, menu_bpk_size,
                                   &menu_archive) != 0) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.dm_bin_hash_verified = 1;
    receipt.menu_bpk_hash_verified = 1;
    receipt.dm_bin_marker_count = dm_catalog.marker_count;
    receipt.dm_bin_v1_record_count = dm_catalog.v1_record_count;
    receipt.menu_prs3_entry_count = menu_archive.prs3_payload_count;
    for (i = 0U; i < menu_archive.candidate_offset_count; ++i) {
        Nexus_V1_BpkPrs3StreamPlan plan;
        Nexus_V1_BpkPrs3Info info;
        int status = nexus_v1_bpk_archive_prs3_stream_plan(
            menu_bpk, menu_bpk_size, i, &plan);

        if (status == NEXUS_V1_BPK_PRS3_STREAM_ERR_NOT_PRS3) continue;
        if (status != NEXUS_V1_BPK_PRS3_STREAM_OK ||
            nexus_v1_bpk_archive_inspect_prs3(menu_bpk, menu_bpk_size, i,
                                               &info) != 0 ||
            !info.prs3_version_matches || !plan.header_first_readable ||
            plan.header_first_u32 == 0U) {
            ++receipt.menu_missing_frame_word_count;
            continue;
        }
        ++receipt.menu_v1_stream_count;
        for (uint32_t marker_index = 0U;
             marker_index < dm_catalog.marker_count; ++marker_index) {
            const Nexus_V1_Prs3DmBinMarker *marker =
                &dm_catalog.markers[marker_index];
            if (marker->kind == NEXUS_V1_PRS3_DM_BIN_MARKER_V1_RECORD &&
                marker->declared_target_bytes == plan.expected_output_bytes) {
                ++receipt.matching_declared_target_count;
                break;
            }
        }
    }
    receipt.outer_v1_framing_matches = receipt.dm_bin_v1_record_count > 0U &&
        receipt.menu_prs3_entry_count > 0U &&
        receipt.menu_v1_stream_count == receipt.menu_prs3_entry_count &&
        receipt.menu_missing_frame_word_count == 0U;
    /* No command execution capture is consumed here. */
    receipt.shared_opcode_grammar_proven = 0;
    receipt.decoder_promoted = 0;
    receipt.menu_handoff_authorized = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.outer_v1_framing_matches;
}

int nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
    const uint8_t *dm_bin, size_t dm_bin_size, int source_hash_verified,
    Nexus_V1_Prs3Sh2V1ExecutionReceipt *out_receipt) {
    enum {
        DM_BIN_V1_FRAME_OFFSET = 231668U,
        SH2_V1_CALLEE_OFFSET = 85376U,
        SH2_CONTROL_TEST_OFFSET = SH2_V1_CALLEE_OFFSET + 74U,
        SH2_STREAM_BYTE_READ_OFFSET = SH2_V1_CALLEE_OFFSET + 84U,
        SH2_OUTPUT_INDEX_COPY_OFFSET = SH2_V1_CALLEE_OFFSET + 86U,
        SH2_OUTPUT_BYTE_STORE_OFFSET = SH2_V1_CALLEE_OFFSET + 88U,
        SH2_LOOP_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 96U,
        SH2_LOOP_BODY_START_OFFSET = SH2_V1_CALLEE_OFFSET + 52U,
        SH2_LOOP_BODY_BYTE_COUNT =
            SH2_LOOP_BRANCH_OFFSET + 2U - SH2_LOOP_BODY_START_OFFSET,
        SH2_CONTROL_REENTRY_OFFSET = SH2_V1_CALLEE_OFFSET + 52U,
        SH2_CONTROL_SHIFT_OFFSET = SH2_V1_CALLEE_OFFSET + 54U,
        SH2_CONTROL_REFILL_TEST_OFFSET = SH2_V1_CALLEE_OFFSET + 56U,
        SH2_CONTROL_REFILL_SKIP_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 58U,
        SH2_CONTROL_REFILL_GUARD_OFFSET = SH2_V1_CALLEE_OFFSET + 60U,
        SH2_CONTROL_REFILL_FAILURE_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 62U,
        SH2_CONTROL_REFILL_BYTE_READ_OFFSET = SH2_V1_CALLEE_OFFSET + 64U,
        SH2_CONTROL_REFILL_MERGE_OFFSET = SH2_V1_CALLEE_OFFSET + 70U,
        SH2_CONTROL_LOW_BIT_TEST_OFFSET = SH2_V1_CALLEE_OFFSET + 74U,
        SH2_CONTROL_ZERO_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 76U,
        SH2_ZERO_FIRST_BYTE_READ_OFFSET = SH2_V1_CALLEE_OFFSET + 108U,
        SH2_ZERO_SECOND_BYTE_READ_OFFSET = SH2_V1_CALLEE_OFFSET + 112U,
        SH2_ZERO_MERGE_OR_OFFSET = SH2_V1_CALLEE_OFFSET + 124U,
        SH2_ZERO_INDEX_MASK_LITERAL_LOAD_OFFSET = SH2_V1_CALLEE_OFFSET + 50U,
        SH2_ZERO_INDEX_MASK_OFFSET = SH2_V1_CALLEE_OFFSET + 144U,
        SH2_ZERO_INDEXED_BYTE_READ_OFFSET = SH2_V1_CALLEE_OFFSET + 148U,
        SH2_ZERO_POST_READ_COMPARE_OFFSET = SH2_V1_CALLEE_OFFSET + 154U,
        SH2_ZERO_REPEAT_COUNTER_INCREMENT_OFFSET = SH2_V1_CALLEE_OFFSET + 156U,
        SH2_ZERO_REPEAT_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 158U,
        SH2_ZERO_REPEAT_DELAY_MASK_OFFSET = SH2_V1_CALLEE_OFFSET + 160U,
        SH2_ZERO_OUTER_LOOP_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 162U
    };
    Nexus_V1_Prs3Sh2V1ExecutionReceipt receipt;
    uint32_t offset;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!dm_bin || !source_hash_verified ||
        dm_bin_size < SH2_LOOP_BRANCH_OFFSET + 2U ||
        dm_bin_size < DM_BIN_V1_FRAME_OFFSET + 16U ||
        memcmp(dm_bin + DM_BIN_V1_FRAME_OFFSET, "PRS3", 4U) != 0 ||
        read_be32(dm_bin + DM_BIN_V1_FRAME_OFFSET + 4U) != 1U ||
        read_be32(dm_bin + DM_BIN_V1_FRAME_OFFSET + 8U) != 4096U ||
        read_be32(dm_bin + DM_BIN_V1_FRAME_OFFSET + 12U) != 997U) {
        *out_receipt = receipt;
        return 0;
    }

    receipt.source_hash_verified = 1;
    receipt.dm_bin_v1_frame_verified = 1;
    receipt.v1_callee_offset = SH2_V1_CALLEE_OFFSET;
    receipt.control_test_offset = SH2_CONTROL_TEST_OFFSET;
    receipt.stream_byte_read_offset = SH2_STREAM_BYTE_READ_OFFSET;
    receipt.output_index_copy_offset = SH2_OUTPUT_INDEX_COPY_OFFSET;
    receipt.output_byte_store_offset = SH2_OUTPUT_BYTE_STORE_OFFSET;
    receipt.loop_branch_offset = SH2_LOOP_BRANCH_OFFSET;
    receipt.loop_body_start_offset = SH2_LOOP_BODY_START_OFFSET;
    receipt.loop_body_byte_count = SH2_LOOP_BODY_BYTE_COUNT;
    receipt.control_test_instruction =
        read_be16(dm_bin + receipt.control_test_offset);
    receipt.stream_byte_read_instruction =
        read_be16(dm_bin + receipt.stream_byte_read_offset);
    receipt.output_byte_store_instruction =
        read_be16(dm_bin + receipt.output_byte_store_offset);
    receipt.loop_branch_instruction =
        read_be16(dm_bin + receipt.loop_branch_offset);
    receipt.sh2_loop_back_target_verified = sh2_bra_target(
        receipt.loop_branch_instruction, receipt.loop_branch_offset,
        &receipt.loop_back_target_offset) &&
        receipt.loop_back_target_offset == receipt.loop_body_start_offset;
    if (receipt.loop_body_start_offset <= dm_bin_size &&
        receipt.loop_body_byte_count <=
            dm_bin_size - receipt.loop_body_start_offset) {
        receipt.loop_body_fnv1a64 = fnv1a64(
            dm_bin + receipt.loop_body_start_offset,
            receipt.loop_body_byte_count);
    receipt.sh2_loop_body_bound = receipt.loop_body_fnv1a64 != 0U;
    receipt.control_reentry_offset = SH2_CONTROL_REENTRY_OFFSET;
    receipt.control_shift_offset = SH2_CONTROL_SHIFT_OFFSET;
    receipt.control_refill_test_offset = SH2_CONTROL_REFILL_TEST_OFFSET;
    receipt.control_refill_skip_branch_offset = SH2_CONTROL_REFILL_SKIP_BRANCH_OFFSET;
    receipt.control_refill_guard_offset = SH2_CONTROL_REFILL_GUARD_OFFSET;
    receipt.control_refill_failure_branch_offset = SH2_CONTROL_REFILL_FAILURE_BRANCH_OFFSET;
    receipt.control_refill_byte_read_offset = SH2_CONTROL_REFILL_BYTE_READ_OFFSET;
    receipt.control_refill_merge_offset = SH2_CONTROL_REFILL_MERGE_OFFSET;
    receipt.control_low_bit_test_offset = SH2_CONTROL_LOW_BIT_TEST_OFFSET;
    receipt.control_zero_branch_offset = SH2_CONTROL_ZERO_BRANCH_OFFSET;
    receipt.sh2_control_refill_verified =
        sh2_bra_target(read_be16(dm_bin + SH2_LOOP_BRANCH_OFFSET),
                       SH2_LOOP_BRANCH_OFFSET, &receipt.control_reentry_offset) &&
        receipt.control_reentry_offset == SH2_CONTROL_REENTRY_OFFSET &&
        read_be16(dm_bin + SH2_CONTROL_REENTRY_OFFSET) == 0x926aU &&
        sh2_movw_pc_literal_target(read_be16(dm_bin + SH2_CONTROL_REENTRY_OFFSET),
                                   SH2_CONTROL_REENTRY_OFFSET,
                                   &receipt.control_sentinel_literal_offset) &&
        receipt.control_sentinel_literal_offset + 2U <= dm_bin_size &&
        (receipt.control_sentinel_word = read_be16(
             dm_bin + receipt.control_sentinel_literal_offset)) == 0x0100U &&
        read_be16(dm_bin + SH2_CONTROL_SHIFT_OFFSET) == 0x4b21U &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_TEST_OFFSET) == 0x22b8U &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_SKIP_BRANCH_OFFSET) == 0x8b05U &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_GUARD_OFFSET) == 0x2ee8U &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_FAILURE_BRANCH_OFFSET) == 0x8932U &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_BYTE_READ_OFFSET) == 0x6bc4U &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_BYTE_READ_OFFSET + 2U) == 0x7effU &&
        read_be16(dm_bin + SH2_CONTROL_REFILL_MERGE_OFFSET) == 0x2b9bU &&
        read_be16(dm_bin + SH2_CONTROL_LOW_BIT_TEST_OFFSET) == 0x23b8U &&
        sh2_conditional_branch_target(
            read_be16(dm_bin + SH2_CONTROL_ZERO_BRANCH_OFFSET),
            SH2_CONTROL_ZERO_BRANCH_OFFSET,
            &receipt.control_zero_branch_target_offset) &&
        receipt.control_zero_branch_target_offset == SH2_V1_CALLEE_OFFSET + 100U;
    receipt.zero_first_byte_read_offset = SH2_ZERO_FIRST_BYTE_READ_OFFSET;
    receipt.zero_second_byte_read_offset = SH2_ZERO_SECOND_BYTE_READ_OFFSET;
    receipt.zero_merge_or_offset = SH2_ZERO_MERGE_OR_OFFSET;
    receipt.zero_index_mask_offset = SH2_ZERO_INDEX_MASK_OFFSET;
    receipt.zero_indexed_byte_read_offset = SH2_ZERO_INDEXED_BYTE_READ_OFFSET;
    receipt.sh2_zero_side_index_read_verified =
        read_be16(dm_bin + SH2_ZERO_FIRST_BYTE_READ_OFFSET) == 0x64c4U &&
        read_be16(dm_bin + SH2_ZERO_SECOND_BYTE_READ_OFFSET) == 0x67c4U &&
        read_be16(dm_bin + SH2_ZERO_MERGE_OR_OFFSET) == 0x243bU &&
        read_be16(dm_bin + SH2_ZERO_INDEX_MASK_LITERAL_LOAD_OFFSET) == 0x956aU &&
        sh2_movw_pc_literal_target(
            read_be16(dm_bin + SH2_ZERO_INDEX_MASK_LITERAL_LOAD_OFFSET),
            SH2_ZERO_INDEX_MASK_LITERAL_LOAD_OFFSET,
            &receipt.zero_index_mask_literal_offset) &&
        receipt.zero_index_mask_literal_offset + 2U <= dm_bin_size &&
        (receipt.zero_index_mask_word = read_be16(
             dm_bin + receipt.zero_index_mask_literal_offset)) == 0x0fffU &&
        read_be16(dm_bin + SH2_ZERO_INDEX_MASK_OFFSET) == 0x2059U &&
        read_be16(dm_bin + SH2_ZERO_INDEXED_BYTE_READ_OFFSET) == 0x01dcU;
    receipt.zero_post_read_compare_offset = SH2_ZERO_POST_READ_COMPARE_OFFSET;
    receipt.zero_repeat_counter_increment_offset = SH2_ZERO_REPEAT_COUNTER_INCREMENT_OFFSET;
    receipt.zero_repeat_branch_offset = SH2_ZERO_REPEAT_BRANCH_OFFSET;
    receipt.zero_repeat_delay_mask_offset = SH2_ZERO_REPEAT_DELAY_MASK_OFFSET;
    receipt.zero_outer_loop_branch_offset = SH2_ZERO_OUTER_LOOP_BRANCH_OFFSET;
    receipt.sh2_zero_side_repeat_control_verified =
        read_be16(dm_bin + SH2_ZERO_POST_READ_COMPARE_OFFSET) == 0x2a10U &&
        read_be16(dm_bin + SH2_ZERO_REPEAT_COUNTER_INCREMENT_OFFSET) == 0x7a01U &&
        read_be16(dm_bin + SH2_ZERO_REPEAT_BRANCH_OFFSET) == 0x8ff3U &&
        sh2_conditional_branch_target(
            read_be16(dm_bin + SH2_ZERO_REPEAT_BRANCH_OFFSET),
            SH2_ZERO_REPEAT_BRANCH_OFFSET,
            &receipt.zero_repeat_branch_target_offset) &&
        receipt.zero_repeat_branch_target_offset == SH2_V1_CALLEE_OFFSET + 136U &&
        read_be16(dm_bin + SH2_ZERO_REPEAT_DELAY_MASK_OFFSET) == 0x2659U &&
        sh2_bra_target(read_be16(dm_bin + SH2_ZERO_OUTER_LOOP_BRANCH_OFFSET),
                       SH2_ZERO_OUTER_LOOP_BRANCH_OFFSET,
                       &receipt.zero_outer_loop_target_offset) &&
        receipt.zero_outer_loop_target_offset == SH2_CONTROL_REENTRY_OFFSET;
    receipt.zero_side_linear_begin_offset =
        receipt.control_zero_branch_target_offset;
    receipt.zero_side_linear_end_offset = SH2_ZERO_OUTER_LOOP_BRANCH_OFFSET + 2U;
    if (receipt.zero_side_linear_end_offset <= dm_bin_size) {
        for (offset = receipt.zero_side_linear_begin_offset;
             offset + 2U <= receipt.zero_side_linear_end_offset; offset += 2U) {
            if (read_be16(dm_bin + offset) == 0x0d24U) {
                ++receipt.zero_side_output_store_instruction_count;
            }
        }
    }
    receipt.sh2_zero_side_has_no_direct_output_store =
        receipt.zero_side_linear_end_offset <= dm_bin_size &&
        receipt.zero_side_output_store_instruction_count == 0U;
    /* Static code never proves that R13 is a history window or that this
     * control branch copies bytes. Only a source-bound execution trace can. */
    receipt.zero_side_copy_or_backreference_proven = 0;
    }
    receipt.sh2_control_path_verified =
        receipt.control_test_instruction == 0x23b8U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 72U) == 0xe301U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 76U) == 0x890aU;
    receipt.sh2_stream_read_verified =
        receipt.stream_byte_read_instruction == 0x62c4U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 82U) == 0x7effU;
    receipt.sh2_output_store_verified =
        receipt.output_byte_store_instruction == 0x0d24U &&
        read_be16(dm_bin + SH2_OUTPUT_INDEX_COPY_OFFSET) == 0x6063U &&
        receipt.loop_branch_instruction == 0xafe8U;
    /* Nonzero control fallthrough's exact output predecessor: R6 is copied
     * into R0 immediately before the R2 byte store through R13/R0. This
     * source fact still does not bind R13 to a final decoded surface. */
    receipt.sh2_output_store_predecessor_verified =
        receipt.sh2_stream_read_verified && receipt.sh2_output_store_verified &&
        receipt.output_index_copy_offset + 2U == receipt.output_byte_store_offset &&
        read_be16(dm_bin + receipt.output_index_copy_offset) == 0x6063U;
    /* No original execution capture establishes these three bindings yet. */
    receipt.menu_frame_binding_proven = 0;
    receipt.vdp1_command_proven = 0;
    receipt.opcode_grammar_proven = 0;
    receipt.decoder_promoted = 0;
    *out_receipt = receipt;
    return receipt.sh2_control_path_verified && receipt.sh2_stream_read_verified &&
        receipt.sh2_output_store_verified && receipt.sh2_loop_back_target_verified &&
        receipt.sh2_loop_body_bound;
}

int nexus_v1_prs3_vdp1_capture_schema_parse(
    const char *text, size_t text_size,
    Nexus_V1_Prs3Vdp1CaptureReceipt *out_receipt) {
    Nexus_V1_Prs3Vdp1CaptureReceipt receipt;
    uint32_t decoder_returned_success;
    uint32_t capture_complete;
    uint32_t output_store_predecessor_observed;
    uint32_t complete_output_store_range_observed;
    uint32_t complete_control_branch_coverage_observed;
    const char *cursor;
    size_t magic_size;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!text) return 0;
    magic_size = sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V5_MAGIC) - 1U;
    if (text_size > magic_size &&
        memcmp(text, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V5_MAGIC, magic_size) == 0 &&
        text[magic_size] == '\n') {
        receipt.schema_version = 5U;
    } else {
    magic_size = sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V4_MAGIC) - 1U;
    if (text_size > magic_size &&
        memcmp(text, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V4_MAGIC, magic_size) == 0 &&
        text[magic_size] == '\n') {
        receipt.schema_version = 4U;
    } else {
    magic_size = sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V3_MAGIC) - 1U;
    if (text_size > magic_size &&
        memcmp(text, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V3_MAGIC, magic_size) == 0 &&
        text[magic_size] == '\n') {
        receipt.schema_version = 3U;
    } else {
        magic_size = sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V2_MAGIC) - 1U;
        if (text_size > magic_size &&
            memcmp(text, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_V2_MAGIC, magic_size) == 0 &&
            text[magic_size] == '\n') {
        receipt.schema_version = 2U;
        } else {
        magic_size = sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC) - 1U;
        if (text_size <= magic_size ||
            memcmp(text, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC, magic_size) != 0 ||
            text[magic_size] != '\n') return 0;
        receipt.schema_version = 1U;
        }
    }
    }
    }
    cursor = text + magic_size + 1U;
    if (!read_u64(&cursor, "menu_bpk_fnv1a64=", &receipt.menu_bpk_fnv1a64) ||
        !read_u64(&cursor, "dm_bin_fnv1a64=", &receipt.dm_bin_fnv1a64) ||
        !read_u32(&cursor, "entry_index=", &receipt.entry_index) ||
        !read_u32(&cursor, "stream_offset=", &receipt.stream_offset) ||
        !read_u32(&cursor, "stream_size=", &receipt.stream_size) ||
        !read_u32(&cursor, "expected_output_bytes=", &receipt.expected_output_bytes) ||
        !read_u32(&cursor, "payload_ram_address=", &receipt.payload_ram_address) ||
        !read_u32(&cursor, "first_input_read_address=", &receipt.first_input_read_address) ||
        !read_u32(&cursor, "last_input_read_address=", &receipt.last_input_read_address) ||
        !read_u32(&cursor, "input_read_bytes=", &receipt.input_read_bytes) ||
        !read_u64(&cursor, "payload_fnv1a64=", &receipt.payload_fnv1a64) ||
        !read_u32(&cursor, "output_ram_address=", &receipt.output_ram_address) ||
        !read_u32(&cursor, "first_output_write_address=", &receipt.first_output_write_address) ||
        !read_u32(&cursor, "last_output_write_address=", &receipt.last_output_write_address) ||
        !read_u32(&cursor, "output_write_bytes=", &receipt.output_write_bytes) ||
        !read_u64(&cursor, "output_fnv1a64=", &receipt.output_fnv1a64) ||
        !read_u64(&cursor, "first_opcode_sequence=", &receipt.first_opcode_sequence) ||
        !read_u64(&cursor, "first_input_read_sequence=", &receipt.first_input_read_sequence) ||
        !read_u64(&cursor, "last_input_read_sequence=", &receipt.last_input_read_sequence) ||
        !read_u64(&cursor, "first_output_write_sequence=", &receipt.first_output_write_sequence) ||
        !read_u64(&cursor, "last_output_write_sequence=", &receipt.last_output_write_sequence) ||
        !read_u64(&cursor, "decoder_return_sequence=", &receipt.decoder_return_sequence) ||
        !read_u64(&cursor, "vdp1_command_sequence=", &receipt.vdp1_command_sequence) ||
        !read_u32(&cursor, "vdp1_command_address=", &receipt.vdp1_command_address) ||
        !read_u32(&cursor, "vdp1_texture_source_address=", &receipt.vdp1_texture_source_address) ||
        !read_u32(&cursor, "vdp1_texture_source_bytes=", &receipt.vdp1_texture_source_bytes) ||
        (receipt.schema_version >= 2U &&
         (!read_u64(&cursor, "vdp1_texture_first_read_sequence=", &receipt.vdp1_texture_first_read_sequence) ||
          !read_u64(&cursor, "vdp1_texture_last_read_sequence=", &receipt.vdp1_texture_last_read_sequence) ||
          !read_u32(&cursor, "vdp1_texture_first_read_address=", &receipt.vdp1_texture_first_read_address) ||
          !read_u32(&cursor, "vdp1_texture_last_read_address=", &receipt.vdp1_texture_last_read_address) ||
          !read_u32(&cursor, "vdp1_texture_read_bytes=", &receipt.vdp1_texture_read_bytes) ||
          !read_u64(&cursor, "vdp1_texture_fnv1a64=", &receipt.vdp1_texture_fnv1a64))) ||
        (receipt.schema_version >= 3U &&
         (!read_u64(&cursor, "vdp1_command_first_read_sequence=", &receipt.vdp1_command_first_read_sequence) ||
          !read_u64(&cursor, "vdp1_command_last_read_sequence=", &receipt.vdp1_command_last_read_sequence) ||
          !read_u32(&cursor, "vdp1_command_first_read_address=", &receipt.vdp1_command_first_read_address) ||
          !read_u32(&cursor, "vdp1_command_last_read_address=", &receipt.vdp1_command_last_read_address) ||
          !read_u32(&cursor, "vdp1_command_read_bytes=", &receipt.vdp1_command_read_bytes) ||
          !read_u64(&cursor, "vdp1_command_fnv1a64=", &receipt.vdp1_command_fnv1a64) ||
          !read_u64(&cursor, "palette_first_read_sequence=", &receipt.palette_first_read_sequence) ||
          !read_u64(&cursor, "palette_last_read_sequence=", &receipt.palette_last_read_sequence) ||
          !read_u32(&cursor, "palette_first_read_address=", &receipt.palette_first_read_address) ||
          !read_u32(&cursor, "palette_last_read_address=", &receipt.palette_last_read_address) ||
          !read_u32(&cursor, "palette_read_bytes=", &receipt.palette_read_bytes) ||
          !read_u64(&cursor, "palette_fnv1a64=", &receipt.palette_fnv1a64))) ||
        (receipt.schema_version >= 4U &&
         (!read_u32(&cursor, "output_index_copy_instruction_offset=", &receipt.output_index_copy_instruction_offset) ||
          !read_u32(&cursor, "output_store_instruction_offset=", &receipt.output_store_instruction_offset) ||
          !read_u64(&cursor, "first_output_store_sequence=", &receipt.first_output_store_sequence) ||
          !read_u64(&cursor, "last_output_store_sequence=", &receipt.last_output_store_sequence) ||
          !read_u32(&cursor, "first_output_store_address=", &receipt.first_output_store_address) ||
          !read_u32(&cursor, "last_output_store_address=", &receipt.last_output_store_address) ||
          !read_u32(&cursor, "output_store_bytes=", &receipt.output_store_bytes) ||
          !read_u64(&cursor, "output_store_fnv1a64=", &receipt.output_store_fnv1a64) ||
          !read_u32(&cursor, "output_store_predecessor_observed=", &output_store_predecessor_observed) ||
          !read_u32(&cursor, "complete_output_store_range_observed=", &complete_output_store_range_observed))) ||
        (receipt.schema_version >= 5U &&
         (!read_u32(&cursor, "control_test_instruction_offset=", &receipt.control_test_instruction_offset) ||
          !read_u32(&cursor, "control_zero_branch_instruction_offset=", &receipt.control_zero_branch_instruction_offset) ||
          !read_u32(&cursor, "control_zero_branch_target_offset=", &receipt.control_zero_branch_target_offset) ||
          !read_u32(&cursor, "control_branch_outcomes_mask=", &receipt.control_branch_outcomes_mask) ||
          !read_u32(&cursor, "nonzero_control_observation_count=", &receipt.nonzero_control_observation_count) ||
          !read_u32(&cursor, "zero_control_observation_count=", &receipt.zero_control_observation_count) ||
          !read_u64(&cursor, "first_control_sequence=", &receipt.first_control_sequence) ||
          !read_u64(&cursor, "last_control_sequence=", &receipt.last_control_sequence) ||
          !read_u32(&cursor, "complete_control_branch_coverage_observed=", &complete_control_branch_coverage_observed))) ||
        !read_u32(&cursor, "decoder_returned_success=", &decoder_returned_success) ||
        !read_u32(&cursor, "capture_complete=", &capture_complete) || *cursor != '\0') return 0;
    if (!receipt.menu_bpk_fnv1a64 || !receipt.dm_bin_fnv1a64 ||
        !receipt.stream_size || !receipt.expected_output_bytes ||
        !receipt.payload_ram_address ||
        receipt.first_input_read_address != receipt.payload_ram_address ||
        !receipt.input_read_bytes || receipt.input_read_bytes > receipt.stream_size ||
        receipt.last_input_read_address != receipt.payload_ram_address +
            receipt.input_read_bytes - 1U ||
        !receipt.payload_fnv1a64 ||
        !receipt.output_ram_address ||
        receipt.first_output_write_address != receipt.output_ram_address ||
        receipt.output_write_bytes != receipt.expected_output_bytes ||
        receipt.last_output_write_address != receipt.output_ram_address +
            receipt.output_write_bytes - 1U || !receipt.output_fnv1a64 ||
        !receipt.first_opcode_sequence ||
        receipt.first_input_read_sequence <= receipt.first_opcode_sequence ||
        receipt.first_input_read_sequence > receipt.last_input_read_sequence ||
        receipt.last_output_write_sequence < receipt.first_opcode_sequence ||
        receipt.first_output_write_sequence <= receipt.first_opcode_sequence ||
        receipt.first_output_write_sequence > receipt.last_output_write_sequence ||
        receipt.last_input_read_sequence >= receipt.decoder_return_sequence ||
        receipt.last_output_write_sequence >= receipt.decoder_return_sequence ||
        receipt.vdp1_command_sequence <= receipt.decoder_return_sequence ||
        !receipt.vdp1_command_address ||
        receipt.vdp1_texture_source_address != receipt.output_ram_address ||
        receipt.vdp1_texture_source_bytes != receipt.expected_output_bytes ||
        (receipt.schema_version >= 2U &&
         (!receipt.vdp1_texture_first_read_sequence ||
          receipt.vdp1_texture_first_read_sequence <= receipt.vdp1_command_sequence ||
          receipt.vdp1_texture_first_read_sequence > receipt.vdp1_texture_last_read_sequence ||
          receipt.vdp1_texture_first_read_address != receipt.output_ram_address ||
          !receipt.vdp1_texture_read_bytes ||
          receipt.vdp1_texture_read_bytes != receipt.expected_output_bytes ||
          receipt.vdp1_texture_last_read_address != receipt.output_ram_address +
              receipt.vdp1_texture_read_bytes - 1U ||
          receipt.vdp1_texture_fnv1a64 != receipt.output_fnv1a64)) ||
        (receipt.schema_version >= 3U &&
         (!receipt.vdp1_command_first_read_sequence ||
          receipt.vdp1_command_first_read_sequence <= receipt.vdp1_command_sequence ||
          receipt.vdp1_command_first_read_sequence > receipt.vdp1_command_last_read_sequence ||
          !receipt.vdp1_command_first_read_address ||
          !receipt.vdp1_command_read_bytes || !receipt.vdp1_command_fnv1a64 ||
          receipt.vdp1_command_last_read_address !=
              receipt.vdp1_command_first_read_address +
              receipt.vdp1_command_read_bytes - 1U ||
          !receipt.palette_first_read_sequence ||
          receipt.palette_first_read_sequence <= receipt.vdp1_command_sequence ||
          receipt.palette_first_read_sequence > receipt.palette_last_read_sequence ||
          !receipt.palette_first_read_address || !receipt.palette_read_bytes ||
          !receipt.palette_fnv1a64 ||
          receipt.palette_last_read_address != receipt.palette_first_read_address +
              receipt.palette_read_bytes - 1U)) ||
        (receipt.schema_version >= 4U &&
         (!receipt.output_index_copy_instruction_offset ||
          !receipt.output_store_instruction_offset ||
          receipt.output_index_copy_instruction_offset + 2U !=
              receipt.output_store_instruction_offset ||
          receipt.first_output_store_sequence != receipt.first_output_write_sequence ||
          receipt.last_output_store_sequence != receipt.last_output_write_sequence ||
          receipt.first_output_store_address != receipt.output_ram_address ||
          receipt.last_output_store_address != receipt.last_output_write_address ||
          receipt.output_store_bytes != receipt.expected_output_bytes ||
          receipt.output_store_fnv1a64 != receipt.output_fnv1a64 ||
          output_store_predecessor_observed != 1U ||
          complete_output_store_range_observed != 1U)) ||
        (receipt.schema_version >= 5U &&
         (!receipt.control_test_instruction_offset ||
          !receipt.control_zero_branch_instruction_offset ||
          !receipt.control_zero_branch_target_offset ||
          receipt.control_branch_outcomes_mask != 3U ||
          !receipt.nonzero_control_observation_count ||
          !receipt.zero_control_observation_count ||
          receipt.first_control_sequence <= receipt.first_opcode_sequence ||
          receipt.first_control_sequence > receipt.last_control_sequence ||
          receipt.last_control_sequence >= receipt.decoder_return_sequence ||
          complete_control_branch_coverage_observed != 1U)) ||
        decoder_returned_success != 1U || capture_complete != 1U) return 0;
    receipt.valid = 1;
    receipt.complete_capture = 1;
    receipt.exact_vdp1_handoff_observed = 1;
    receipt.vdp1_texture_consumption_observed = receipt.schema_version >= 2U;
    receipt.vdp1_command_consumption_observed = receipt.schema_version >= 3U;
    receipt.palette_consumption_observed = receipt.schema_version >= 3U;
    receipt.output_store_predecessor_observed = receipt.schema_version >= 4U;
    receipt.complete_output_store_range_observed = receipt.schema_version >= 4U;
    receipt.complete_control_branch_coverage_observed = receipt.schema_version >= 5U;
    receipt.opcode_grammar_proven = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return 1;
}

int nexus_v1_prs3_vdp1_capture_schema_bind_assets(
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size,
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt *out_receipt) {
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt receipt;
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!trace || !trace->valid || !trace->complete_capture || !menu_bpk ||
        !menu_bpk_size || !dm_bin || !dm_bin_size) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.trace_valid = 1;
    receipt.menu_bpk_matches = fnv1a64(menu_bpk, menu_bpk_size) ==
        trace->menu_bpk_fnv1a64;
    receipt.dm_bin_matches = fnv1a64(dm_bin, dm_bin_size) ==
        trace->dm_bin_fnv1a64;
    if (nexus_v1_bpk_archive_prs3_stream_plan(
            menu_bpk, menu_bpk_size, trace->entry_index, &plan) ==
        NEXUS_V1_BPK_PRS3_STREAM_OK) {
        receipt.entry_plan_matches = plan.stream_offset == trace->stream_offset &&
            plan.stream_size == trace->stream_size &&
            plan.expected_output_bytes == trace->expected_output_bytes;
        if (receipt.entry_plan_matches)
            receipt.payload_span_matches = fnv1a64(
                menu_bpk + plan.stream_offset, plan.stream_size) ==
                trace->payload_fnv1a64;
    }
    receipt.exact_vdp1_handoff_observed = receipt.menu_bpk_matches &&
        receipt.dm_bin_matches && receipt.entry_plan_matches &&
        receipt.payload_span_matches &&
        trace->exact_vdp1_handoff_observed;
    receipt.vdp1_texture_consumption_observed =
        receipt.exact_vdp1_handoff_observed &&
        trace->vdp1_texture_consumption_observed;
    receipt.vdp1_command_consumption_observed =
        receipt.exact_vdp1_handoff_observed &&
        trace->vdp1_command_consumption_observed;
    receipt.palette_consumption_observed =
        receipt.exact_vdp1_handoff_observed &&
        trace->palette_consumption_observed;
    if (trace->schema_version >= 4U) {
        int output_store_route_matches =
            nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                dm_bin, dm_bin_size, 1, &sh2) &&
            sh2.sh2_output_store_predecessor_verified &&
            trace->output_index_copy_instruction_offset ==
                sh2.output_index_copy_offset &&
            trace->output_store_instruction_offset ==
                sh2.output_byte_store_offset &&
            trace->output_store_predecessor_observed &&
            trace->complete_output_store_range_observed;
        receipt.exact_vdp1_handoff_observed =
            receipt.exact_vdp1_handoff_observed && output_store_route_matches;
        receipt.vdp1_texture_consumption_observed =
            receipt.vdp1_texture_consumption_observed && output_store_route_matches;
        receipt.vdp1_command_consumption_observed =
            receipt.vdp1_command_consumption_observed && output_store_route_matches;
        receipt.palette_consumption_observed =
            receipt.palette_consumption_observed && output_store_route_matches;
    }
    receipt.valid = receipt.exact_vdp1_handoff_observed;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_prs3_decoder_readiness_bind_capture(
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace,
    const uint8_t *menu_bpk, size_t menu_bpk_size,
    const uint8_t *dm_bin, size_t dm_bin_size,
    Nexus_V1_Prs3DecoderReadinessReceipt *out_receipt) {
    Nexus_V1_Prs3DecoderReadinessReceipt receipt;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt capture_binding;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!trace || trace->schema_version < 5U ||
        !nexus_v1_prs3_vdp1_capture_schema_bind_assets(
            trace, menu_bpk, menu_bpk_size, dm_bin, dm_bin_size,
            &capture_binding) ||
        !nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, dm_bin_size, 1, &sh2)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.capture_source_bound = capture_binding.valid;
    receipt.complete_input_range_bound =
        trace->input_read_bytes == trace->stream_size &&
        trace->first_input_read_address == trace->payload_ram_address &&
        trace->last_input_read_address == trace->payload_ram_address +
            trace->stream_size - 1U;
    receipt.complete_output_range_bound =
        trace->output_write_bytes == trace->expected_output_bytes &&
        trace->output_store_bytes == trace->expected_output_bytes &&
        trace->first_output_write_address == trace->output_ram_address &&
        trace->last_output_write_address == trace->output_ram_address +
            trace->expected_output_bytes - 1U &&
        trace->first_output_store_address == trace->output_ram_address &&
        trace->last_output_store_address == trace->output_ram_address +
            trace->expected_output_bytes - 1U;
    receipt.control_branch_coverage_bound =
        trace->complete_control_branch_coverage_observed &&
        trace->control_branch_outcomes_mask == 3U &&
        trace->nonzero_control_observation_count > 0U &&
        trace->zero_control_observation_count > 0U &&
        trace->control_test_instruction_offset == sh2.control_low_bit_test_offset &&
        trace->control_zero_branch_instruction_offset ==
            sh2.control_zero_branch_offset &&
        trace->control_zero_branch_target_offset ==
            sh2.control_zero_branch_target_offset;
    receipt.valid = receipt.capture_source_bound &&
        receipt.complete_input_range_bound && receipt.complete_output_range_bound &&
        receipt.control_branch_coverage_bound;
    /* A source-bound capture is not independent producer authentication, nor
     * does complete branch coverage assign PRS3 command grammar. */
    receipt.original_saturn_execution_authenticated = 0;
    receipt.opcode_grammar_proven = 0;
    receipt.decoder_ready = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_prs3_vdp1_capture_validate_files(
    const char *trace_path, const char *menu_bpk_path, const char *dm_bin_path,
    Nexus_V1_Prs3Vdp1CaptureFileReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1CaptureFileReceipt receipt;
    uint8_t *trace_data = NULL;
    uint8_t *menu_bpk = NULL;
    uint8_t *dm_bin = NULL;
    size_t trace_size = 0U;
    size_t menu_bpk_size = 0U;
    size_t dm_bin_size = 0U;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!trace_path || !menu_bpk_path || !dm_bin_path ||
        !asset_file_matches_md5(menu_bpk_path,
                                NEXUS_V1_PRS3_CAPTURE_MENU_BPK_MD5) ||
        !asset_file_matches_md5(dm_bin_path,
                                NEXUS_V1_PRS3_CAPTURE_DM_BIN_MD5)) {
        *out_receipt = receipt;
        return 0;
    }
    receipt.menu_bpk_original_hash_verified = 1;
    receipt.dm_bin_original_hash_verified = 1;
    trace_data = read_capture_file(trace_path, NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES,
                                   &trace_size);
    menu_bpk = read_capture_file(menu_bpk_path, 16U * 1024U * 1024U,
                                 &menu_bpk_size);
    dm_bin = read_capture_file(dm_bin_path, 16U * 1024U * 1024U,
                               &dm_bin_size);
    if (!trace_data || !menu_bpk || !dm_bin) goto done;
    receipt.trace_file_read = 1;
    receipt.v3_trace_parsed = nexus_v1_prs3_vdp1_capture_schema_parse(
        (const char *)trace_data, trace_size, &receipt.trace) &&
        receipt.trace.schema_version >= 3U;
    if (receipt.v3_trace_parsed) {
        receipt.source_bound_capture =
            nexus_v1_prs3_vdp1_capture_schema_bind_assets(
                &receipt.trace, menu_bpk, menu_bpk_size, dm_bin, dm_bin_size,
                &receipt.binding) &&
            receipt.binding.vdp1_command_consumption_observed &&
            receipt.binding.palette_consumption_observed;
    }
done:
    free(trace_data);
    free(menu_bpk);
    free(dm_bin);
    /* An imported capture is diagnostic evidence only. */
    receipt.runtime_import_permitted = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.source_bound_capture;
}

int nexus_v1_prs3_vdp1_capture_validate_raw_sidecars(
    const char *trace_path, const char *menu_bpk_path, const char *dm_bin_path,
    const char *output_path, const char *vdp1_command_path,
    const char *palette_path, Nexus_V1_Prs3Vdp1RawSidecarReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1RawSidecarReceipt receipt;
    uint8_t *output = NULL;
    uint8_t *command = NULL;
    uint8_t *palette = NULL;
    size_t output_size = 0U;
    size_t command_size = 0U;
    size_t palette_size = 0U;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.trace_source_bound = nexus_v1_prs3_vdp1_capture_validate_files(
        trace_path, menu_bpk_path, dm_bin_path, &receipt.trace_file);
    if (!receipt.trace_source_bound || !output_path || !vdp1_command_path ||
        !palette_path) goto done;
    output = read_capture_file(output_path, 16U * 1024U * 1024U, &output_size);
    command = read_capture_file(vdp1_command_path, 64U * 1024U, &command_size);
    palette = read_capture_file(palette_path, 1024U * 1024U, &palette_size);
    if (!output || !command || !palette) goto done;
    receipt.output_sidecar_bound =
        output_size == receipt.trace_file.trace.expected_output_bytes &&
        fnv1a64(output, output_size) == receipt.trace_file.trace.output_fnv1a64;
    receipt.vdp1_command_sidecar_bound =
        command_size == receipt.trace_file.trace.vdp1_command_read_bytes &&
        fnv1a64(command, command_size) ==
            receipt.trace_file.trace.vdp1_command_fnv1a64;
    receipt.palette_sidecar_bound =
        palette_size == receipt.trace_file.trace.palette_read_bytes &&
        fnv1a64(palette, palette_size) == receipt.trace_file.trace.palette_fnv1a64;
    receipt.raw_sidecars_bound = receipt.output_sidecar_bound &&
        receipt.vdp1_command_sidecar_bound && receipt.palette_sidecar_bound;
done:
    free(output);
    free(command);
    free(palette);
    receipt.capture_producer_authenticated = 0;
    receipt.runtime_import_permitted = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.raw_sidecars_bound;
}

int nexus_v1_prs3_vdp1_capture_inspect_command_sidecar(
    const Nexus_V1_Prs3Vdp1RawSidecarReceipt *raw_sidecars,
    const uint8_t *vdp1_command, size_t vdp1_command_size,
    Nexus_V1_Prs3Vdp1CommandSidecarReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1CommandSidecarReceipt receipt;
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!raw_sidecars || !vdp1_command) {
        *out_receipt = receipt;
        return 0;
    }
    trace = &raw_sidecars->trace_file.trace;
    receipt.capture_source_bound = raw_sidecars->raw_sidecars_bound &&
        raw_sidecars->trace_file.source_bound_capture &&
        raw_sidecars->trace_file.v3_trace_parsed && trace->valid &&
        trace->schema_version == 3U &&
        trace->vdp1_command_consumption_observed;
    receipt.command_sidecar_hash_bound = receipt.capture_source_bound &&
        raw_sidecars->vdp1_command_sidecar_bound &&
        vdp1_command_size == trace->vdp1_command_read_bytes &&
        fnv1a64(vdp1_command, vdp1_command_size) ==
            trace->vdp1_command_fnv1a64;
    receipt.complete_vdp1_command_record = receipt.command_sidecar_hash_bound &&
        vdp1_command_size == NEXUS_V1_VDP1_COMMAND_BYTES;
    if (receipt.complete_vdp1_command_record &&
        nexus_v1_vdp1_texture_command_parse(vdp1_command,
            (int)vdp1_command_size, &receipt.command) == 0) {
        receipt.command_format_parsed = 1;
    }
    receipt.valid = receipt.command_format_parsed;
    /* The sidecar is source-bound but not independently authenticated, and
     * packet fields alone cannot define texture/palette decoding. */
    receipt.original_saturn_capture_verified = 0;
    receipt.pixel_format_proven = 0;
    receipt.palette_format_proven = 0;
    receipt.decoder_promoted = 0;
    receipt.runtime_import_permitted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.valid;
}

int nexus_v1_prs3_vdp1_capture_validate_provenance(
    const char *ledger_path, const char *trace_path, const char *output_path,
    const char *vdp1_command_path, const char *palette_path,
    const char *producer_binary_path,
    const Nexus_V1_Prs3Vdp1RawSidecarReceipt *raw_sidecars,
    Nexus_V1_Prs3Vdp1ProvenanceReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1ProvenanceReceipt receipt;
    uint8_t *ledger = NULL, *trace = NULL, *output = NULL, *command = NULL;
    uint8_t *palette = NULL, *producer = NULL;
    size_t ledger_size = 0U, trace_size = 0U, output_size = 0U;
    size_t command_size = 0U, palette_size = 0U, producer_size = 0U;
    const char *cursor;
    uint64_t trace_hash, output_hash, command_hash, palette_hash, producer_hash;
    const char magic[] = "NEXUS_PRS3_V3_PROVENANCE_V1\n";

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!raw_sidecars || !raw_sidecars->raw_sidecars_bound || !ledger_path ||
        !trace_path || !output_path || !vdp1_command_path || !palette_path ||
        !producer_binary_path) goto done;
    receipt.raw_sidecars_bound = 1;
    ledger = read_capture_file(ledger_path, NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES,
                               &ledger_size);
    trace = read_capture_file(trace_path, NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES,
                              &trace_size);
    output = read_capture_file(output_path, 16U * 1024U * 1024U, &output_size);
    command = read_capture_file(vdp1_command_path, 64U * 1024U, &command_size);
    palette = read_capture_file(palette_path, 1024U * 1024U, &palette_size);
    producer = read_capture_file(producer_binary_path, 512U * 1024U * 1024U,
                                 &producer_size);
    if (!ledger || !trace || !output || !command || !palette || !producer ||
        ledger_size <= sizeof(magic) - 1U ||
        memcmp(ledger, magic, sizeof(magic) - 1U) != 0) goto done;
    cursor = (const char *)ledger + sizeof(magic) - 1U;
    if (!read_u64(&cursor, "trace_fnv1a64=", &trace_hash) ||
        !read_u64(&cursor, "output_fnv1a64=", &output_hash) ||
        !read_u64(&cursor, "vdp1_command_fnv1a64=", &command_hash) ||
        !read_u64(&cursor, "palette_fnv1a64=", &palette_hash) ||
        !read_u64(&cursor, "producer_binary_fnv1a64=", &producer_hash) ||
        *cursor != '\0') goto done;
    receipt.ledger_parsed = 1;
    receipt.trace_bytes_match = trace_hash == fnv1a64(trace, trace_size);
    receipt.output_bytes_match = output_hash == fnv1a64(output, output_size);
    receipt.vdp1_command_bytes_match =
        command_hash == fnv1a64(command, command_size);
    receipt.palette_bytes_match = palette_hash == fnv1a64(palette, palette_size);
    receipt.producer_binary_bound =
        producer_hash == fnv1a64(producer, producer_size);
    receipt.provenance_complete = receipt.trace_bytes_match &&
        receipt.output_bytes_match && receipt.vdp1_command_bytes_match &&
        receipt.palette_bytes_match && receipt.producer_binary_bound;
done:
    free(ledger); free(trace); free(output); free(command); free(palette);
    free(producer);
    receipt.capture_producer_authenticated = 0;
    receipt.runtime_import_permitted = 0;
    *out_receipt = receipt;
    return receipt.provenance_complete;
}

int nexus_v1_prs3_vdp1_capture_write_provenance_ledger(
    const char *ledger_path, const char *trace_path,
    const char *menu_bpk_path, const char *dm_bin_path,
    const char *output_path, const char *vdp1_command_path,
    const char *palette_path, const char *producer_binary_path,
    Nexus_V1_Prs3Vdp1RawSidecarReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1RawSidecarReceipt receipt;
    uint8_t *trace = NULL, *output = NULL, *command = NULL, *palette = NULL;
    uint8_t *producer = NULL;
    size_t trace_size = 0U, output_size = 0U, command_size = 0U;
    size_t palette_size = 0U, producer_size = 0U;
    char temporary_path[1024];
    FILE *file = NULL;
    int written = 0;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!ledger_path || !trace_path || !menu_bpk_path || !dm_bin_path ||
        !output_path || !vdp1_command_path || !palette_path ||
        !producer_binary_path || strcmp(ledger_path, trace_path) == 0 ||
        strcmp(ledger_path, output_path) == 0 ||
        strcmp(ledger_path, vdp1_command_path) == 0 ||
        strcmp(ledger_path, palette_path) == 0 ||
        strcmp(ledger_path, producer_binary_path) == 0) goto done;
    if (!nexus_v1_prs3_vdp1_capture_validate_raw_sidecars(
            trace_path, menu_bpk_path, dm_bin_path, output_path,
            vdp1_command_path, palette_path, &receipt)) goto done;
    trace = read_capture_file(trace_path, NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES,
                              &trace_size);
    output = read_capture_file(output_path, 16U * 1024U * 1024U, &output_size);
    command = read_capture_file(vdp1_command_path, 64U * 1024U, &command_size);
    palette = read_capture_file(palette_path, 1024U * 1024U, &palette_size);
    producer = read_capture_file(producer_binary_path, 512U * 1024U * 1024U,
                                 &producer_size);
    if (!trace || !output || !command || !palette || !producer ||
        snprintf(temporary_path, sizeof(temporary_path), "%s.tmp.%ld",
                 ledger_path, (long)getpid()) >= (int)sizeof(temporary_path))
        goto done;
    file = fopen(temporary_path, "wb");
    if (!file) goto done;
    if (fprintf(file,
                "NEXUS_PRS3_V3_PROVENANCE_V1\n"
                "trace_fnv1a64=%llx\n"
                "output_fnv1a64=%llx\n"
                "vdp1_command_fnv1a64=%llx\n"
                "palette_fnv1a64=%llx\n"
                "producer_binary_fnv1a64=%llx\n",
                (unsigned long long)fnv1a64(trace, trace_size),
                (unsigned long long)fnv1a64(output, output_size),
                (unsigned long long)fnv1a64(command, command_size),
                (unsigned long long)fnv1a64(palette, palette_size),
                (unsigned long long)fnv1a64(producer, producer_size)) < 0) {
        fclose(file);
        file = NULL;
        remove(temporary_path);
        goto done;
    }
    if (fclose(file) != 0) {
        file = NULL;
        remove(temporary_path);
        goto done;
    }
    file = NULL;
    if (rename(temporary_path, ledger_path) != 0) {
        remove(temporary_path);
        goto done;
    }
    written = 1;
done:
    if (file) {
        fclose(file);
        remove(temporary_path);
    }
    free(trace); free(output); free(command); free(palette); free(producer);
    *out_receipt = receipt;
    return written;
}

int nexus_v1_prs3_vdp1_capture_validate_producer_attestation(
    const char *attestation_path, const char *trace_path,
    const char *output_path, const char *vdp1_command_path,
    const char *palette_path, const char *producer_binary_path,
    const Nexus_V1_Prs3Vdp1RawSidecarReceipt *raw_sidecars,
    const Nexus_V1_Prs3Vdp1ProvenanceReceipt *provenance,
    Nexus_V1_Prs3Vdp1ProducerAttestationReceipt *out_receipt)
{
    Nexus_V1_Prs3Vdp1ProducerAttestationReceipt receipt;
    uint8_t *attestation = NULL, *trace = NULL, *output = NULL, *command = NULL;
    uint8_t *palette = NULL, *producer = NULL;
    size_t attestation_size = 0U, trace_size = 0U, output_size = 0U;
    size_t command_size = 0U, palette_size = 0U, producer_size = 0U;
    const char *cursor;
    const char magic[] = NEXUS_V1_PRS3_VDP1_PRODUCER_ATTESTATION_MAGIC "\n";
    uint64_t trace_hash, output_hash, command_hash, palette_hash, producer_hash;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    receipt.independent_authentication_required = 1;
    if (!raw_sidecars || !provenance || !raw_sidecars->raw_sidecars_bound ||
        !provenance->provenance_complete || !attestation_path || !trace_path ||
        !output_path || !vdp1_command_path || !palette_path ||
        !producer_binary_path) goto done;
    receipt.raw_sidecars_bound = 1;
    receipt.provenance_complete = 1;
    attestation = read_capture_file(attestation_path,
                                    NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES,
                                    &attestation_size);
    trace = read_capture_file(trace_path, NEXUS_V1_PRS3_CAPTURE_TRACE_MAX_BYTES,
                              &trace_size);
    output = read_capture_file(output_path, 16U * 1024U * 1024U, &output_size);
    command = read_capture_file(vdp1_command_path, 64U * 1024U, &command_size);
    palette = read_capture_file(palette_path, 1024U * 1024U, &palette_size);
    producer = read_capture_file(producer_binary_path, 512U * 1024U * 1024U,
                                 &producer_size);
    if (!attestation || !trace || !output || !command || !palette || !producer ||
        attestation_size <= sizeof(magic) - 1U ||
        memcmp(attestation, magic, sizeof(magic) - 1U) != 0) goto done;
    receipt.attestation_file_read = 1;
    cursor = (const char *)attestation + sizeof(magic) - 1U;
    if (strncmp(cursor, "producer_name=MEDNAFEN\n", 23U) != 0) goto done;
    cursor += 23U;
    if (strncmp(cursor, "capture_mode=SH2_VDP1_BUS_TRACE\n", 32U) != 0) goto done;
    cursor += 32U;
    if (strncmp(cursor, "original_saturn_execution=CLAIMED\n", 34U) != 0) goto done;
    cursor += 34U;
    if (!read_u64(&cursor, "trace_fnv1a64=", &trace_hash) ||
        !read_u64(&cursor, "output_fnv1a64=", &output_hash) ||
        !read_u64(&cursor, "vdp1_command_fnv1a64=", &command_hash) ||
        !read_u64(&cursor, "palette_fnv1a64=", &palette_hash) ||
        !read_u64(&cursor, "producer_binary_fnv1a64=", &producer_hash) ||
        *cursor != '\0') goto done;
    receipt.attestation_parsed = 1;
    receipt.capture_mode_declared = 1;
    receipt.original_saturn_execution_claimed = 1;
    receipt.producer_binary_bound = producer_hash == fnv1a64(producer, producer_size);
    receipt.artifact_hashes_bound = trace_hash == fnv1a64(trace, trace_size) &&
        output_hash == fnv1a64(output, output_size) &&
        command_hash == fnv1a64(command, command_size) &&
        palette_hash == fnv1a64(palette, palette_size);
    receipt.workflow_complete = receipt.producer_binary_bound &&
        receipt.artifact_hashes_bound;
done:
    free(attestation); free(trace); free(output); free(command); free(palette);
    free(producer);
    receipt.capture_producer_authenticated = 0;
    receipt.runtime_import_permitted = 0;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.workflow_complete;
}
