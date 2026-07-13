#include "nexus_v1_prs3_capture_trace_schema.h"
#include "nexus_v1_bpk_archive.h"

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
        SH2_OUTPUT_BYTE_STORE_OFFSET = SH2_V1_CALLEE_OFFSET + 88U,
        SH2_LOOP_BRANCH_OFFSET = SH2_V1_CALLEE_OFFSET + 96U
    };
    Nexus_V1_Prs3Sh2V1ExecutionReceipt receipt;

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
    receipt.output_byte_store_offset = SH2_OUTPUT_BYTE_STORE_OFFSET;
    receipt.loop_branch_offset = SH2_LOOP_BRANCH_OFFSET;
    receipt.control_test_instruction =
        read_be16(dm_bin + receipt.control_test_offset);
    receipt.stream_byte_read_instruction =
        read_be16(dm_bin + receipt.stream_byte_read_offset);
    receipt.output_byte_store_instruction =
        read_be16(dm_bin + receipt.output_byte_store_offset);
    receipt.loop_branch_instruction =
        read_be16(dm_bin + receipt.loop_branch_offset);
    receipt.sh2_control_path_verified =
        receipt.control_test_instruction == 0x23b8U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 72U) == 0xe301U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 76U) == 0x890aU;
    receipt.sh2_stream_read_verified =
        receipt.stream_byte_read_instruction == 0x62c4U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 82U) == 0x7effU;
    receipt.sh2_output_store_verified =
        receipt.output_byte_store_instruction == 0x0d24U &&
        read_be16(dm_bin + SH2_V1_CALLEE_OFFSET + 86U) == 0x6063U &&
        receipt.loop_branch_instruction == 0xafe8U;
    /* No original execution capture establishes these three bindings yet. */
    receipt.menu_frame_binding_proven = 0;
    receipt.vdp1_command_proven = 0;
    receipt.opcode_grammar_proven = 0;
    receipt.decoder_promoted = 0;
    *out_receipt = receipt;
    return receipt.sh2_control_path_verified && receipt.sh2_stream_read_verified &&
        receipt.sh2_output_store_verified;
}

int nexus_v1_prs3_vdp1_capture_schema_parse(
    const char *text, size_t text_size,
    Nexus_V1_Prs3Vdp1CaptureReceipt *out_receipt) {
    Nexus_V1_Prs3Vdp1CaptureReceipt receipt;
    uint32_t decoder_returned_success;
    uint32_t capture_complete;
    const char *cursor;
    const size_t magic_size =
        sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC) - 1U;

    if (!out_receipt) return 0;
    memset(out_receipt, 0, sizeof(*out_receipt));
    memset(&receipt, 0, sizeof(receipt));
    if (!text || text_size <= magic_size ||
        memcmp(text, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC, magic_size) != 0 ||
        text[magic_size] != '\n') return 0;
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
        !read_u32(&cursor, "output_ram_address=", &receipt.output_ram_address) ||
        !read_u32(&cursor, "first_output_write_address=", &receipt.first_output_write_address) ||
        !read_u32(&cursor, "last_output_write_address=", &receipt.last_output_write_address) ||
        !read_u32(&cursor, "output_write_bytes=", &receipt.output_write_bytes) ||
        !read_u64(&cursor, "output_fnv1a64=", &receipt.output_fnv1a64) ||
        !read_u64(&cursor, "first_opcode_sequence=", &receipt.first_opcode_sequence) ||
        !read_u64(&cursor, "last_input_read_sequence=", &receipt.last_input_read_sequence) ||
        !read_u64(&cursor, "last_output_write_sequence=", &receipt.last_output_write_sequence) ||
        !read_u64(&cursor, "decoder_return_sequence=", &receipt.decoder_return_sequence) ||
        !read_u64(&cursor, "vdp1_command_sequence=", &receipt.vdp1_command_sequence) ||
        !read_u32(&cursor, "vdp1_command_address=", &receipt.vdp1_command_address) ||
        !read_u32(&cursor, "vdp1_texture_source_address=", &receipt.vdp1_texture_source_address) ||
        !read_u32(&cursor, "vdp1_texture_source_bytes=", &receipt.vdp1_texture_source_bytes) ||
        !read_u32(&cursor, "decoder_returned_success=", &decoder_returned_success) ||
        !read_u32(&cursor, "capture_complete=", &capture_complete) || *cursor != '\0') return 0;
    if (!receipt.menu_bpk_fnv1a64 || !receipt.dm_bin_fnv1a64 ||
        !receipt.stream_size || !receipt.expected_output_bytes ||
        !receipt.payload_ram_address ||
        receipt.first_input_read_address != receipt.payload_ram_address ||
        !receipt.input_read_bytes || receipt.input_read_bytes > receipt.stream_size ||
        receipt.last_input_read_address != receipt.payload_ram_address +
            receipt.input_read_bytes - 1U ||
        !receipt.output_ram_address ||
        receipt.first_output_write_address != receipt.output_ram_address ||
        receipt.output_write_bytes != receipt.expected_output_bytes ||
        receipt.last_output_write_address != receipt.output_ram_address +
            receipt.output_write_bytes - 1U || !receipt.output_fnv1a64 ||
        !receipt.first_opcode_sequence ||
        receipt.last_input_read_sequence < receipt.first_opcode_sequence ||
        receipt.last_output_write_sequence < receipt.first_opcode_sequence ||
        receipt.last_input_read_sequence >= receipt.decoder_return_sequence ||
        receipt.last_output_write_sequence >= receipt.decoder_return_sequence ||
        receipt.vdp1_command_sequence <= receipt.decoder_return_sequence ||
        !receipt.vdp1_command_address ||
        receipt.vdp1_texture_source_address != receipt.output_ram_address ||
        receipt.vdp1_texture_source_bytes != receipt.expected_output_bytes ||
        decoder_returned_success != 1U || capture_complete != 1U) return 0;
    receipt.valid = 1;
    receipt.complete_capture = 1;
    receipt.exact_vdp1_handoff_observed = 1;
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
    }
    receipt.exact_vdp1_handoff_observed = receipt.menu_bpk_matches &&
        receipt.dm_bin_matches && receipt.entry_plan_matches &&
        trace->exact_vdp1_handoff_observed;
    receipt.valid = receipt.exact_vdp1_handoff_observed;
    receipt.decoder_promoted = 0;
    receipt.fallback_visuals_permitted = 0;
    *out_receipt = receipt;
    return receipt.valid;
}
