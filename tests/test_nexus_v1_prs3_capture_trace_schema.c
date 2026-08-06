#include "nexus_v1_prs3_capture_trace_schema.h"
#include "nexus_v1_bpk_archive.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

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

static const char valid_vdp1_trace[] =
    "NEXUS_PRS3_SH2_VDP1_TRACE_V2\n"
    "menu_bpk_fnv1a64=%llx\n"
    "dm_bin_fnv1a64=%llx\n"
    "entry_index=0\nstream_offset=40\nstream_size=4\nexpected_output_bytes=4\n"
    "payload_ram_address=6010000\nfirst_input_read_address=6010000\n"
    "last_input_read_address=6010003\ninput_read_bytes=4\n"
    "payload_fnv1a64=%llx\n"
    "output_ram_address=6020000\nfirst_output_write_address=6020000\n"
    "last_output_write_address=6020003\noutput_write_bytes=4\noutput_fnv1a64=3\n"
    "first_opcode_sequence=10\nfirst_input_read_sequence=11\n"
    "last_input_read_sequence=12\nfirst_output_write_sequence=13\n"
    "last_output_write_sequence=14\ndecoder_return_sequence=15\n"
    "vdp1_command_sequence=16\nvdp1_command_address=5c00000\n"
    "vdp1_texture_source_address=6020000\nvdp1_texture_source_bytes=4\n"
    "vdp1_texture_first_read_sequence=17\nvdp1_texture_last_read_sequence=18\n"
    "vdp1_texture_first_read_address=6020000\nvdp1_texture_last_read_address=6020003\n"
    "vdp1_texture_read_bytes=4\nvdp1_texture_fnv1a64=3\n"
    "decoder_returned_success=1\ncapture_complete=1\n";

static const char valid_vdp1_v3_trace[] =
    "NEXUS_PRS3_SH2_VDP1_TRACE_V3\n"
    "menu_bpk_fnv1a64=%llx\n"
    "dm_bin_fnv1a64=%llx\n"
    "entry_index=0\nstream_offset=40\nstream_size=4\nexpected_output_bytes=4\n"
    "payload_ram_address=6010000\nfirst_input_read_address=6010000\n"
    "last_input_read_address=6010003\ninput_read_bytes=4\n"
    "payload_fnv1a64=%llx\n"
    "output_ram_address=6020000\nfirst_output_write_address=6020000\n"
    "last_output_write_address=6020003\noutput_write_bytes=4\noutput_fnv1a64=3\n"
    "first_opcode_sequence=10\nfirst_input_read_sequence=11\n"
    "last_input_read_sequence=12\nfirst_output_write_sequence=13\n"
    "last_output_write_sequence=14\ndecoder_return_sequence=15\n"
    "vdp1_command_sequence=16\nvdp1_command_address=5c00000\n"
    "vdp1_texture_source_address=6020000\nvdp1_texture_source_bytes=4\n"
    "vdp1_texture_first_read_sequence=17\nvdp1_texture_last_read_sequence=18\n"
    "vdp1_texture_first_read_address=6020000\nvdp1_texture_last_read_address=6020003\n"
    "vdp1_texture_read_bytes=4\nvdp1_texture_fnv1a64=3\n"
    "vdp1_command_first_read_sequence=17\nvdp1_command_last_read_sequence=17\n"
    "vdp1_command_first_read_address=5c00000\nvdp1_command_last_read_address=5c0001f\n"
    "vdp1_command_read_bytes=20\nvdp1_command_fnv1a64=4\n"
    "palette_first_read_sequence=17\npalette_last_read_sequence=18\n"
    "palette_first_read_address=5f00000\npalette_last_read_address=5f0001f\n"
    "palette_read_bytes=20\npalette_fnv1a64=5\n"
    "decoder_returned_success=1\ncapture_complete=1\n";

static const char valid_vdp1_v4_trace[] =
    "NEXUS_PRS3_SH2_VDP1_TRACE_V4\n"
    "menu_bpk_fnv1a64=%llx\ndm_bin_fnv1a64=%llx\n"
    "entry_index=0\nstream_offset=40\nstream_size=4\nexpected_output_bytes=4\n"
    "payload_ram_address=6010000\nfirst_input_read_address=6010000\n"
    "last_input_read_address=6010003\ninput_read_bytes=4\npayload_fnv1a64=%llx\n"
    "output_ram_address=6020000\nfirst_output_write_address=6020000\n"
    "last_output_write_address=6020003\noutput_write_bytes=4\noutput_fnv1a64=3\n"
    "first_opcode_sequence=10\nfirst_input_read_sequence=11\nlast_input_read_sequence=12\n"
    "first_output_write_sequence=13\nlast_output_write_sequence=14\ndecoder_return_sequence=15\n"
    "vdp1_command_sequence=16\nvdp1_command_address=5c00000\n"
    "vdp1_texture_source_address=6020000\nvdp1_texture_source_bytes=4\n"
    "vdp1_texture_first_read_sequence=17\nvdp1_texture_last_read_sequence=18\n"
    "vdp1_texture_first_read_address=6020000\nvdp1_texture_last_read_address=6020003\n"
    "vdp1_texture_read_bytes=4\nvdp1_texture_fnv1a64=3\n"
    "vdp1_command_first_read_sequence=17\nvdp1_command_last_read_sequence=17\n"
    "vdp1_command_first_read_address=5c00000\nvdp1_command_last_read_address=5c0001f\n"
    "vdp1_command_read_bytes=20\nvdp1_command_fnv1a64=4\n"
    "palette_first_read_sequence=17\npalette_last_read_sequence=18\n"
    "palette_first_read_address=5f00000\npalette_last_read_address=5f0001f\n"
    "palette_read_bytes=20\npalette_fnv1a64=5\n"
    "output_index_copy_instruction_offset=14dd6\noutput_store_instruction_offset=14dd8\n"
    "first_output_store_sequence=13\nlast_output_store_sequence=14\n"
    "first_output_store_address=6020000\nlast_output_store_address=6020003\n"
    "output_store_bytes=4\noutput_store_fnv1a64=3\n"
    "output_store_predecessor_observed=1\ncomplete_output_store_range_observed=1\n"
    "decoder_returned_success=1\ncapture_complete=1\n";

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

static int write_file_bytes(const char *path, const unsigned char *data,
                            size_t size) {
    FILE *file = fopen(path, "wb");
    if (!file) return 0;
    if (size > 0U && fwrite(data, 1U, size, file) != size) {
        fclose(file);
        return 0;
    }
    return fclose(file) == 0;
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

static void test_sh2_transfer_trace_gate(void) {
    unsigned char bpk[68];
    unsigned char dm_bin[85400];
    Nexus_V1_Prs3Sh2TransferTrace trace;
    Nexus_V1_Prs3Sh2TransferReceipt receipt;
    char text[1024];
    unsigned long long bpk_hash;
    unsigned long long dm_hash;

    make_bpk(bpk);
    memset(dm_bin, 0, sizeof(dm_bin));
    /* The real loader's receipt is intentionally hash-gated; this synthetic
     * fixture therefore proves syntax rejection rather than a decoder route. */
    bpk_hash = fnv1a64(bpk, sizeof(bpk));
    dm_hash = fnv1a64(dm_bin, sizeof(dm_bin));
    snprintf(text, sizeof(text),
             "NEXUS_PRS3_SH2_TRANSFER_TRACE_V1\n"
             "menu_bpk_fnv1a64=%llx\ndm_bin_fnv1a64=%llx\n"
             "entry_index=0\nstream_offset=34\nstream_size=10\n"
             "expected_output_bytes=4\npayload_byte_offset=0\n"
             "control_test_instruction_offset=0\nzero_branch_instruction_offset=0\n"
             "fallthrough_counter_decrement_offset=0\n"
             "observed_control_low_bit=1\nobserved_zero_branch_taken=0\n"
             "input_instruction_offset=0\noutput_instruction_offset=0\n"
             "input_read_sequence=10\noutput_write_sequence=11\n"
             "output_byte_offset=0\ninput_byte=50\noutput_byte=50\n",
             bpk_hash, dm_hash);
    expect(!nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
               text, strlen(text), bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
               1, &trace, &receipt) && trace.valid && receipt.trace_valid &&
               !receipt.observed_byte_transfer && !receipt.decoder_promoted &&
               !receipt.fallback_visuals_permitted,
           "unverified loader bytes cannot promote a PRS3 transfer observation");
    text[strlen(text) - 2U] = 'g';
    expect(!nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
               text, strlen(text), bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
               1, &trace, &receipt) && !trace.valid && !receipt.trace_valid,
           "malformed byte values fail closed before transfer binding");
}

static void test_dm_bin_prs3_catalog(void) {
    unsigned char fixture[48];
    Nexus_V1_Prs3DmBinCatalogReceipt receipt;
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[2048];
    FILE *file;
    long size;
    unsigned char *data;

    memset(fixture, 0, sizeof(fixture));
    memcpy(fixture + 4U, "PRS3", 4U);
    put_be32(fixture + 8U, 1U);
    put_be32(fixture + 12U, 4096U);
    put_be32(fixture + 16U, 997U);
    memcpy(fixture + 36U, "PRS3", 4U);
    expect(nexus_v1_prs3_dm_bin_catalog_verified(
               fixture, sizeof(fixture), 0, &receipt) != 0 &&
               !receipt.source_hash_verified,
           "unverified DM.BIN bytes never produce a PRS3 catalog");
    expect(nexus_v1_prs3_dm_bin_catalog_verified(
               fixture, sizeof(fixture), 1, &receipt) == 0 &&
               receipt.source_hash_verified && receipt.marker_count == 2U &&
               receipt.v1_record_count == 1U &&
               receipt.truncated_marker_count == 1U && !receipt.complete &&
               receipt.markers[0].kind == NEXUS_V1_PRS3_DM_BIN_MARKER_V1_RECORD &&
               receipt.markers[0].declared_target_bytes == 4096U &&
               receipt.markers[0].first_frame_word == 997U &&
               !receipt.decoder_promoted,
           "bounded PRS3 V1 framing is cataloged without a decoder");

    if (!data_dir || !data_dir[0]) return;
    snprintf(path, sizeof(path), "%s/DM.BIN", data_dir);
    file = fopen(path, "rb");
    if (!file) return;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data) {
        fclose(file);
        return;
    }
    if (fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return;
    }
    fclose(file);
    expect(nexus_v1_prs3_dm_bin_catalog_verified(
               data, (size_t)size, 1, &receipt) == 0 &&
               receipt.complete && receipt.marker_count == 2U &&
               receipt.executable_marker_count == 1U && receipt.v1_record_count == 1U &&
               receipt.truncated_marker_count == 0U &&
               receipt.markers[1].kind == NEXUS_V1_PRS3_DM_BIN_MARKER_V1_RECORD &&
               receipt.markers[1].version == 1U &&
               receipt.markers[1].declared_target_bytes == 4096U &&
               receipt.markers[1].first_frame_word == 997U &&
               !receipt.decoder_promoted,
           "retail DM.BIN has one executable marker and one bounded PRS3 V1 record");
    free(data);
}

static unsigned char *read_asset(const char *data_dir, const char *name,
                                 size_t *out_size) {
    char path[2048];
    FILE *file;
    long size;
    unsigned char *data;

    if (!data_dir || !data_dir[0] || !name || !out_size) return NULL;
    snprintf(path, sizeof(path), "%s/%s", data_dir, name);
    file = fopen(path, "rb");
    if (!file) return NULL;
    if (fseek(file, 0, SEEK_END) != 0 || (size = ftell(file)) <= 0 ||
        fseek(file, 0, SEEK_SET) != 0) {
        fclose(file);
        return NULL;
    }
    data = (unsigned char *)malloc((size_t)size);
    if (!data || fread(data, 1, (size_t)size, file) != (size_t)size) {
        free(data);
        fclose(file);
        return NULL;
    }
    fclose(file);
    *out_size = (size_t)size;
    return data;
}

static void test_cross_asset_prs3_frame_receipt(void) {
    unsigned char bpk[68];
    unsigned char dm_bin[32];
    Nexus_V1_Prs3CrossAssetFrameReceipt receipt;
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    unsigned char *real_dm_bin;
    unsigned char *real_menu_bpk;
    size_t real_dm_bin_size = 0U;
    size_t real_menu_bpk_size = 0U;

    make_bpk(bpk);
    memset(dm_bin, 0, sizeof(dm_bin));
    memcpy(dm_bin, "PRS3", 4U);
    put_be32(dm_bin + 4U, 1U);
    put_be32(dm_bin + 8U, 4U);
    put_be32(dm_bin + 12U, 4U);
    expect(!nexus_v1_prs3_cross_asset_frame_receipt_verified(
               dm_bin, sizeof(dm_bin), 1, bpk, sizeof(bpk), 0, &receipt) &&
               !receipt.menu_bpk_hash_verified,
           "unverified MENU.BPK never produces a cross-asset PRS3 receipt");
    expect(nexus_v1_prs3_cross_asset_frame_receipt_verified(
               dm_bin, sizeof(dm_bin), 1, bpk, sizeof(bpk), 1, &receipt) &&
               receipt.outer_v1_framing_matches &&
               receipt.dm_bin_v1_record_count == 1U &&
               receipt.menu_prs3_entry_count == 1U &&
               receipt.menu_v1_stream_count == 1U &&
               receipt.matching_declared_target_count == 1U &&
               !receipt.shared_opcode_grammar_proven &&
               !receipt.decoder_promoted && !receipt.menu_handoff_authorized &&
               !receipt.fallback_visuals_permitted,
           "matching V1 outer frames remain blocked from decode and menu handoff");

    real_dm_bin = read_asset(data_dir, "DM.BIN", &real_dm_bin_size);
    real_menu_bpk = read_asset(data_dir, "MENU.BPK", &real_menu_bpk_size);
    if (!real_dm_bin || !real_menu_bpk) {
        free(real_dm_bin);
        free(real_menu_bpk);
        return;
    }
    expect(nexus_v1_prs3_cross_asset_frame_receipt_verified(
               real_dm_bin, real_dm_bin_size, 1,
               real_menu_bpk, real_menu_bpk_size, 1, &receipt) &&
               receipt.dm_bin_marker_count == 2U &&
               receipt.dm_bin_v1_record_count == 1U &&
               receipt.menu_prs3_entry_count == 162U &&
               receipt.menu_v1_stream_count == 162U &&
               receipt.menu_missing_frame_word_count == 0U &&
               receipt.outer_v1_framing_matches &&
               !receipt.shared_opcode_grammar_proven &&
               !receipt.decoder_promoted && !receipt.menu_handoff_authorized,
           "retail DM.BIN and MENU.BPK prove outer V1 framing only");
    free(real_dm_bin);
    free(real_menu_bpk);
}

static void test_dm_bin_sh2_v1_execution_receipt(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_Prs3Sh2V1ExecutionReceipt receipt;
    unsigned char *dm_bin;
    unsigned char *damaged;
    size_t dm_bin_size = 0U;

    dm_bin = read_asset(data_dir, "DM.BIN", &dm_bin_size);
    if (!dm_bin) return;
    expect(nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
               dm_bin, dm_bin_size, 1, &receipt) &&
               receipt.dm_bin_v1_frame_verified &&
               receipt.v1_callee_offset == 85376U &&
               receipt.control_test_offset == 85450U &&
               receipt.stream_byte_read_offset == 85460U &&
               receipt.output_index_copy_offset == 85462U &&
               receipt.output_byte_store_offset == 85464U &&
               receipt.loop_branch_offset == 85472U &&
               receipt.control_reentry_offset == 85428U &&
               receipt.control_shift_offset == 85430U &&
               receipt.control_refill_byte_read_offset == 85440U &&
               receipt.control_refill_merge_offset == 85446U &&
               receipt.terminal_refill_guard_target_offset == 85542U &&
               receipt.terminal_nonzero_guard_target_offset == 85542U &&
               receipt.terminal_zero_guard_target_offset == 85542U &&
               receipt.terminal_result_offset == 85550U &&
               receipt.terminal_result_instruction == 0xe000U &&
               receipt.terminal_return_offset == 85564U &&
               receipt.control_sentinel_literal_offset == 85644U &&
               receipt.control_sentinel_word == 0x0100U &&
               receipt.control_low_bit_mask == 1U &&
               receipt.control_zero_branch_target_offset == 85476U &&
               receipt.zero_first_byte_read_offset == 85484U &&
               receipt.zero_second_byte_read_offset == 85488U &&
               receipt.zero_sequential_input_byte_count == 2U &&
               receipt.zero_upper_mask_literal_load_offset == 85420U &&
               receipt.zero_upper_mask_literal_offset == 85640U &&
               receipt.zero_upper_mask_word == 0x0f00U &&
               receipt.zero_second_byte_copy_offset == 85492U &&
               receipt.zero_first_shift_offset == 85494U &&
               receipt.zero_second_shift_offset == 85496U &&
               receipt.zero_upper_mask_and_offset == 85498U &&
               receipt.zero_merge_or_offset == 85500U &&
               receipt.zero_low_mask_load_offset == 85482U &&
               receipt.zero_low_mask_immediate == 15 &&
               receipt.zero_low_mask_and_offset == 85502U &&
               receipt.zero_low_fragment_increment_offset == 85504U &&
               receipt.zero_low_fragment_increment == 2 &&
               receipt.zero_merged_value_add_offset == 85506U &&
               receipt.zero_merged_branch_compare_offset == 85508U &&
               receipt.zero_merged_branch_offset == 85510U &&
               receipt.zero_merged_branch_target_offset == 85428U &&
               receipt.zero_index_mask_word == 0x0fffU &&
               receipt.zero_index_mask_offset == 85520U &&
               receipt.zero_indexed_byte_read_offset == 85524U &&
               receipt.zero_indexed_byte_base_register == 13U &&
               receipt.zero_indexed_byte_index_register == 0U &&
               receipt.zero_indexed_byte_destination_register == 1U &&
               receipt.zero_first_value_compare_offset == 85528U &&
               receipt.zero_first_value_compare_source_register == 1U &&
               receipt.zero_first_value_compare_destination_register == 3U &&
               receipt.zero_repeat_value_compare_source_register == 1U &&
               receipt.zero_repeat_value_compare_destination_register == 10U &&
               receipt.zero_post_read_compare_offset == 85530U &&
               receipt.zero_repeat_counter_increment_offset == 85532U &&
               receipt.zero_repeat_branch_offset == 85534U &&
               receipt.zero_repeat_compare_instruction == 0x2a10U &&
               receipt.zero_repeat_branch_instruction == 0x8ff3U &&
               receipt.zero_repeat_branch_target_offset == 85512U &&
               receipt.zero_outer_loop_target_offset == 85428U &&
               receipt.zero_side_linear_begin_offset == 85476U &&
               receipt.zero_side_linear_end_offset == 85540U &&
               receipt.zero_side_linear_byte_count == 64U &&
               receipt.zero_side_linear_fnv1a64 == UINT64_C(0xe0cc325e85a0e63f) &&
               receipt.zero_side_output_store_instruction_count == 0U &&
               receipt.control_test_instruction == 0x23b8U &&
               receipt.stream_byte_read_instruction == 0x62c4U &&
               receipt.output_byte_store_instruction == 0x0d24U &&
               receipt.loop_branch_instruction == 0xafe8U &&
               receipt.sh2_control_path_verified &&
               receipt.sh2_stream_read_verified && receipt.sh2_output_store_verified &&
               receipt.sh2_output_store_predecessor_verified &&
               receipt.sh2_control_refill_verified &&
               receipt.sh2_terminal_failure_path_proven &&
               receipt.sh2_control_low_bit_semantics_proven &&
               receipt.sh2_nonzero_direct_byte_path_proven &&
               receipt.nonzero_post_store_r6_increment_offset == 85466U &&
               receipt.nonzero_post_store_r6_increment == 1 &&
               receipt.nonzero_post_store_r6_mask_offset == 85470U &&
               receipt.nonzero_post_store_r6_mask_source_register == 5U &&
               receipt.nonzero_post_store_r6_mask_destination_register == 6U &&
               receipt.nonzero_control_reentry_branch_offset == 85472U &&
               receipt.nonzero_control_reentry_target_offset == 85428U &&
               receipt.sh2_nonzero_output_commit_reentry_proven &&
               receipt.nonzero_source_counter_decrement_offset == 85458U &&
               receipt.nonzero_source_counter_delta == -1 &&
               receipt.zero_source_counter_decrement_offset == 85476U &&
               receipt.zero_source_counter_delta == -2 &&
               receipt.sh2_control_dependent_source_consumption_proven &&
               receipt.sh2_zero_side_index_read_verified &&
               receipt.sh2_zero_side_two_byte_input_span_proven &&
               receipt.sh2_zero_byte_merge_order_proven &&
               receipt.sh2_zero_merged_branch_condition_proven &&
               receipt.sh2_zero_side_repeat_control_verified &&
               receipt.sh2_zero_repeat_termination_proven &&
               receipt.sh2_zero_indexed_byte_control_operands_proven &&
               receipt.sh2_zero_side_linear_route_verified &&
               receipt.sh2_zero_side_has_no_direct_output_store &&
               !receipt.zero_side_copy_or_backreference_proven &&
               !receipt.menu_frame_binding_proven && !receipt.vdp1_command_proven &&
               !receipt.opcode_grammar_proven && !receipt.decoder_promoted,
           "retail DM.BIN imports SH-2 control/read/store evidence without decode");
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85464U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_output_store_verified && !receipt.decoder_promoted,
               "one changed SH-2 output-store instruction rejects the receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85432U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_control_low_bit_semantics_proven &&
                   !receipt.sh2_nonzero_direct_byte_path_proven,
               "changed low-bit mask setup rejects the source byte-path receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85458U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_control_dependent_source_consumption_proven,
               "changed nonzero source debit rejects branch-local consumption receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85466U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_nonzero_output_commit_reentry_proven,
               "changed post-store R6 update rejects nonzero commit receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85456U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_terminal_failure_path_proven,
               "changed counter guard rejects the terminal-path receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85510U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_zero_merged_branch_condition_proven,
               "changed post-merge branch rejects the control-edge receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85498U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_zero_byte_merge_order_proven,
               "changed zero-side merge mask rejects the byte-order receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85488U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_zero_side_two_byte_input_span_proven,
               "changed second zero-side source read rejects the two-byte span");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85534U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_zero_repeat_termination_proven,
               "changed zero-side termination branch rejects the receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        damaged[85524U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_zero_indexed_byte_control_operands_proven,
               "changed indexed source operand rejects zero-side control receipt");
        free(damaged);
    }
    damaged = (unsigned char *)malloc(dm_bin_size);
    if (damaged) {
        memcpy(damaged, dm_bin, dm_bin_size);
        /* This word is inside the observed zero-side corridor but is not one
         * of the named read/merge instructions. The corridor fingerprint
         * must still reject it before an external trace can bind. */
        damaged[85506U] ^= 1U;
        expect(!nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
                   damaged, dm_bin_size, 1, &receipt) &&
                   !receipt.sh2_zero_side_linear_route_verified &&
                   !receipt.sh2_zero_side_has_no_direct_output_store,
               "changed unnamed zero-side corridor instruction rejects the receipt");
        free(damaged);
    }
    free(dm_bin);
}

static void test_real_nonzero_transfer_trace_contract(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;
    Nexus_V1_Prs3Sh2TransferTrace trace;
    Nexus_V1_Prs3Sh2TransferReceipt receipt;
    unsigned char *menu;
    unsigned char *dm_bin;
    size_t menu_size = 0U;
    size_t dm_bin_size = 0U;
    unsigned int index;
    char text[1024];

    menu = read_asset(data_dir, "MENU.BPK", &menu_size);
    dm_bin = read_asset(data_dir, "DM.BIN", &dm_bin_size);
    if (!menu || !dm_bin) { free(menu); free(dm_bin); return; }
    for (index = 0U; index < 163U; ++index) {
        if (nexus_v1_bpk_archive_prs3_stream_plan(menu, menu_size, index, &plan) ==
            NEXUS_V1_BPK_PRS3_STREAM_OK) break;
    }
    if (index == 163U || !nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, dm_bin_size, 1, &sh2)) { free(menu); free(dm_bin); return; }
    snprintf(text, sizeof(text),
             "NEXUS_PRS3_SH2_TRANSFER_TRACE_V1\n"
             "menu_bpk_fnv1a64=%llx\ndm_bin_fnv1a64=%llx\n"
             "entry_index=%x\nstream_offset=%x\nstream_size=%x\n"
             "expected_output_bytes=%x\npayload_byte_offset=0\n"
             "control_test_instruction_offset=%x\nzero_branch_instruction_offset=%x\n"
             "fallthrough_counter_decrement_offset=%x\n"
             "observed_control_low_bit=1\nobserved_zero_branch_taken=0\n"
             "input_instruction_offset=%x\noutput_instruction_offset=%x\n"
             "input_read_sequence=10\noutput_write_sequence=11\n"
             "output_byte_offset=0\ninput_byte=%x\noutput_byte=%x\n",
             fnv1a64(menu, menu_size), fnv1a64(dm_bin, dm_bin_size), index,
             plan.stream_offset, plan.stream_size, plan.expected_output_bytes,
             sh2.control_low_bit_test_offset, sh2.control_zero_branch_offset,
             sh2.nonzero_source_counter_decrement_offset, sh2.stream_byte_read_offset,
             sh2.output_byte_store_offset, menu[plan.stream_offset], menu[plan.stream_offset]);
    expect(nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
               text, strlen(text), menu, menu_size, dm_bin, dm_bin_size, 1,
               &trace, &receipt) && receipt.observed_byte_transfer &&
               receipt.nonzero_control_fallthrough_observed &&
               !receipt.original_saturn_provenance_verified &&
               !receipt.decoder_promoted && !receipt.fallback_visuals_permitted,
           "real assets bind a claimed nonzero byte trace without promoting a decoder");
    memcpy(strstr(text, "fallthrough_counter_decrement_offset=") +
               strlen("fallthrough_counter_decrement_offset="),
           "14dce", sizeof("14dce") - 1U);
    expect(!nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
               text, strlen(text), menu, menu_size, dm_bin, dm_bin_size, 1,
               &trace, &receipt) && !receipt.observed_byte_transfer &&
               !receipt.nonzero_control_fallthrough_observed,
           "nonzero transfer rejects the zero-branch delay-slot offset");
    memcpy(strstr(text, "fallthrough_counter_decrement_offset=") +
               strlen("fallthrough_counter_decrement_offset="),
           "14dd2", sizeof("14dd2") - 1U);
    memcpy(strstr(text, "observed_control_low_bit=1"),
           "observed_control_low_bit=0", sizeof("observed_control_low_bit=0") - 1U);
    expect(!nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
               text, strlen(text), menu, menu_size, dm_bin, dm_bin_size, 1,
               &trace, &receipt) && !receipt.observed_byte_transfer,
           "zero-side claim cannot use the nonzero output byte path");
    free(menu);
    free(dm_bin);
}

static void test_real_zero_side_trace_contract(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Sh2V1ExecutionReceipt sh2;
    Nexus_V1_Prs3Sh2ZeroSideTrace trace;
    Nexus_V1_Prs3Sh2ZeroSideReceipt receipt;
    unsigned char *menu;
    unsigned char *dm_bin;
    size_t menu_size = 0U, dm_bin_size = 0U;
    unsigned int index;

    menu = read_asset(data_dir, "MENU.BPK", &menu_size);
    dm_bin = read_asset(data_dir, "DM.BIN", &dm_bin_size);
    if (!menu || !dm_bin) { free(menu); free(dm_bin); return; }
    for (index = 0U; index < 163U; ++index)
        if (nexus_v1_bpk_archive_prs3_stream_plan(menu, menu_size, index, &plan) ==
            NEXUS_V1_BPK_PRS3_STREAM_OK) break;
    if (index == 163U || !nexus_v1_prs3_dm_bin_sh2_v1_execution_receipt_verified(
            dm_bin, dm_bin_size, 1, &sh2)) { free(menu); free(dm_bin); return; }
    memset(&trace, 0, sizeof(trace));
    trace.menu_bpk_fnv1a64 = fnv1a64(menu, menu_size);
    trace.dm_bin_fnv1a64 = fnv1a64(dm_bin, dm_bin_size);
    trace.entry_index = index; trace.stream_offset = plan.stream_offset;
    trace.stream_size = plan.stream_size; trace.expected_output_bytes = plan.expected_output_bytes;
    trace.second_payload_byte_offset = 1U;
    trace.first_input_byte = menu[plan.stream_offset];
    trace.second_input_byte = menu[plan.stream_offset + 1U];
    trace.merged_control_value = trace.first_input_byte |
        ((trace.second_input_byte << 4U) & 0x0f00U);
    trace.zero_branch_instruction_offset = sh2.control_zero_branch_offset;
    trace.zero_branch_target_offset = sh2.control_zero_branch_target_offset;
    trace.counter_decrement_offset = trace.zero_branch_target_offset;
    trace.first_input_instruction_offset = trace.zero_branch_target_offset + 8U;
    trace.second_input_instruction_offset = trace.zero_branch_target_offset + 12U;
    trace.first_input_read_sequence = 10U; trace.second_input_read_sequence = 11U;
    expect(nexus_v1_prs3_sh2_zero_side_trace_bind(
               &trace, menu, menu_size, dm_bin, dm_bin_size, 1, &receipt) &&
               receipt.observed_zero_side_merge && !receipt.decoder_promoted &&
               !receipt.fallback_visuals_permitted,
           "real assets bind the zero-side merge without backreference semantics");
    ++trace.merged_control_value;
    expect(!nexus_v1_prs3_sh2_zero_side_trace_bind(
               &trace, menu, menu_size, dm_bin, dm_bin_size, 1, &receipt) &&
               !receipt.observed_zero_side_merge,
           "changed zero-side merge value fails closed");
    free(menu); free(dm_bin);
}

static void test_real_v5_decoder_readiness_trace_contract(void) {
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    Nexus_V1_BpkPrs3StreamPlan plan;
    Nexus_V1_Prs3Vdp1CaptureReceipt trace;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt binding;
    Nexus_V1_Prs3DecoderReadinessReceipt readiness;
    Nexus_V1_Prs3Sh2TransferTrace transfer_trace;
    Nexus_V1_Prs3Sh2TransferReceipt transfer_receipt;
    Nexus_V1_Prs3TokenGrammarReceipt token_receipt;
    Nexus_V1_Prs3Sh2ZeroSideTrace zero_trace;
    Nexus_V1_Prs3Sh2ZeroSideReceipt zero_receipt;
    Nexus_V1_Prs3ControlGrammarReviewReceipt control_review;
    unsigned char *menu;
    unsigned char *dm_bin;
    size_t menu_size = 0U, dm_bin_size = 0U;
    unsigned int index;
    char text[4096];
    /* The real V8 receipt carries the full retail stream identity and is
     * materially larger than the compact fixture.  Leave room for the V9/V10
     * DGN fields appended below; truncation must be a test failure, never a
     * NULL-strstr crash. */
    char v9_text[8192];
    char transfer_text[1024];
    char *tail;
    uint32_t output_base = 0x06020000U;

    menu = read_asset(data_dir, "MENU.BPK", &menu_size);
    dm_bin = read_asset(data_dir, "DM.BIN", &dm_bin_size);
    if (!menu || !dm_bin) { free(menu); free(dm_bin); return; }
    for (index = 0U; index < 163U; ++index)
        if (nexus_v1_bpk_archive_prs3_stream_plan(menu, menu_size, index, &plan) ==
            NEXUS_V1_BPK_PRS3_STREAM_OK) break;
    if (index == 163U || plan.expected_output_bytes > 0x100000U) {
        free(menu); free(dm_bin); return;
    }
    snprintf(text, sizeof(text),
        "NEXUS_PRS3_SH2_VDP1_TRACE_V8\n"
        "menu_bpk_fnv1a64=%llx\ndm_bin_fnv1a64=%llx\n"
        "entry_index=%x\nstream_offset=%x\nstream_size=%x\nexpected_output_bytes=%x\n"
        "payload_ram_address=6010000\nfirst_input_read_address=6010000\n"
        "last_input_read_address=%x\ninput_read_bytes=%x\npayload_fnv1a64=%llx\n"
        "output_ram_address=%x\nfirst_output_write_address=%x\n"
        "last_output_write_address=%x\noutput_write_bytes=%x\noutput_fnv1a64=3\n"
        "first_opcode_sequence=10\nfirst_input_read_sequence=11\nlast_input_read_sequence=22\n"
        "first_output_write_sequence=13\nlast_output_write_sequence=23\ndecoder_return_sequence=30\n"
        "vdp1_command_sequence=31\nvdp1_command_address=5c00000\n"
        "vdp1_texture_source_address=%x\nvdp1_texture_source_bytes=%x\n"
        "vdp1_texture_first_read_sequence=32\nvdp1_texture_last_read_sequence=33\n"
        "vdp1_texture_first_read_address=%x\nvdp1_texture_last_read_address=%x\n"
        "vdp1_texture_read_bytes=%x\nvdp1_texture_fnv1a64=3\n"
        "vdp1_command_first_read_sequence=32\nvdp1_command_last_read_sequence=32\n"
        "vdp1_command_first_read_address=5c00000\nvdp1_command_last_read_address=5c0001f\n"
        "vdp1_command_read_bytes=20\nvdp1_command_fnv1a64=4\n"
        "palette_first_read_sequence=32\npalette_last_read_sequence=33\n"
        "palette_first_read_address=5f00000\npalette_last_read_address=5f0001f\n"
        "palette_read_bytes=20\npalette_fnv1a64=5\n"
        "output_index_copy_instruction_offset=14dd6\noutput_store_instruction_offset=14dd8\n"
        "first_output_store_sequence=13\nlast_output_store_sequence=23\n"
        "first_output_store_address=%x\nlast_output_store_address=%x\n"
        "output_store_bytes=%x\noutput_store_fnv1a64=3\n"
        "output_store_predecessor_observed=1\ncomplete_output_store_range_observed=1\n"
        "control_test_instruction_offset=14dca\n"
        "control_zero_branch_instruction_offset=14dcc\n"
        "control_zero_branch_target_offset=14de4\n"
        "control_branch_outcomes_mask=3\n"
        "nonzero_control_observation_count=1\nzero_control_observation_count=1\n"
        "first_control_sequence=13\nlast_control_sequence=22\n"
        "complete_control_branch_coverage_observed=1\n"
        "nonzero_counter_decrement_instruction_offset=14dd2\n"
        "zero_counter_decrement_instruction_offset=14de4\n"
        "nonzero_counter_before=5\nnonzero_counter_after=4\n"
        "zero_counter_before=6\nzero_counter_after=4\n"
        "nonzero_source_cursor_before=6010000\nnonzero_source_cursor_after=6010001\n"
        "zero_source_cursor_before=6010001\nzero_source_cursor_after=6010003\n"
        "nonzero_counter_decrement_sequence=14\nnonzero_input_read_sequence=15\n"
        "zero_counter_decrement_sequence=19\nzero_first_input_read_sequence=20\n"
        "zero_second_input_read_sequence=21\n"
        "dynamic_control_operands_observed=1\n"
        "nonzero_input_payload_byte_offset=0\n"
        "nonzero_observed_input_byte=%x\nnonzero_observed_output_byte=%x\n"
        "nonzero_output_store_instruction_offset=14dd8\n"
        "nonzero_output_byte_offset=0\nnonzero_output_address=%x\n"
        "nonzero_output_write_sequence=16\n"
        "dynamic_nonzero_byte_transfer_observed=1\n"
        "zero_first_input_instruction_offset=14dec\n"
        "zero_second_input_instruction_offset=14df0\n"
        "zero_first_input_payload_byte_offset=0\nzero_second_input_payload_byte_offset=1\n"
        "zero_observed_first_input_byte=%x\nzero_observed_second_input_byte=%x\n"
        "zero_observed_merged_control_value=%x\n"
        "dynamic_zero_source_merge_observed=1\n"
        "decoder_returned_success=1\ncapture_complete=1\n",
        fnv1a64(menu, menu_size), fnv1a64(dm_bin, dm_bin_size), index,
        plan.stream_offset, plan.stream_size, plan.expected_output_bytes,
        0x06010000U + plan.stream_size - 1U, plan.stream_size,
        fnv1a64(menu + plan.stream_offset, plan.stream_size), output_base,
        output_base, output_base + plan.expected_output_bytes - 1U,
        plan.expected_output_bytes, output_base, plan.expected_output_bytes,
        output_base, output_base + plan.expected_output_bytes - 1U,
        plan.expected_output_bytes, output_base,
        output_base + plan.expected_output_bytes - 1U, plan.expected_output_bytes,
        menu[plan.stream_offset], menu[plan.stream_offset], output_base,
        menu[plan.stream_offset], menu[plan.stream_offset + 1U],
        menu[plan.stream_offset] |
            ((uint32_t)menu[plan.stream_offset + 1U] << 4U & 0x0f00U));
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               text, strlen(text), &trace) &&
           nexus_v1_prs3_vdp1_capture_schema_bind_assets(
               &trace, menu, menu_size, dm_bin, dm_bin_size, &binding) &&
           binding.valid && binding.vdp1_command_consumption_observed &&
           binding.palette_consumption_observed && !binding.decoder_promoted &&
           !binding.fallback_visuals_permitted &&
           trace.schema_version == 8U && trace.dynamic_control_operands_observed &&
           trace.dynamic_nonzero_byte_transfer_observed &&
           trace.dynamic_zero_source_merge_observed,
           "V8 binds real zero-side source bytes and merge without decoding");
    tail = strstr(text, "decoder_returned_success=");
    expect(tail != NULL, "V8 fixture has a completion tail for V9 extension");
    if (tail) {
        size_t prefix = (size_t)(tail - text);
        memcpy(v9_text, text, prefix);
        v9_text[prefix] = '\0';
        v9_text[sizeof("NEXUS_PRS3_SH2_VDP1_TRACE_V") - 1U] = '9';
        expect(snprintf(v9_text + prefix, sizeof(v9_text) - prefix,
                 "dgn_fnv1a64=7\ndgn_descriptor_index=0\n"
                 "dgn_frame_sequence=40\ndgn_command_sequence=31\n"
                 "dgn_command_xa=0\ndgn_command_ya=0\n"
                 "dgn_command_xb=0\ndgn_command_yb=0\n"
                 "dgn_command_xc=0\ndgn_command_yc=0\n"
                 "dgn_command_xd=0\ndgn_command_yd=0\n%s", tail) >= 0 &&
               strlen(v9_text) < sizeof(v9_text),
               "V9 receipt extension is not truncated");
        expect(nexus_v1_prs3_vdp1_capture_schema_parse(
                   v9_text, strlen(v9_text), &trace) &&
                   trace.schema_version == 9U && trace.dgn_placement_observed &&
                   trace.dgn_fnv1a64 == 7U && trace.dgn_frame_sequence == 0x40U &&
                   trace.dgn_command_sequence == trace.vdp1_command_sequence,
               "V9 requires an explicit DGN descriptor/frame/command observation");
        {
            char *dgn_identity = strstr(v9_text, "dgn_fnv1a64=7");
            expect(dgn_identity != NULL, "V9 receipt retains DGN identity");
            if (dgn_identity) {
                memcpy(dgn_identity, "dgn_fnv1a64=0", 15U);
                expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
                           v9_text, strlen(v9_text), &trace),
                       "V9 rejects a missing DGN identity");
            }
        }
        memcpy(v9_text, text, prefix);
        v9_text[prefix] = '\0';
        v9_text[sizeof("NEXUS_PRS3_SH2_VDP1_TRACE_V") - 1U] = '1';
        v9_text[sizeof("NEXUS_PRS3_SH2_VDP1_TRACE_V")] = '0';
        {
            const size_t v8_magic_size =
                sizeof("NEXUS_PRS3_SH2_VDP1_TRACE_V8\n") - 1U;
            const size_t v10_prefix = prefix + 1U;
            /* V10 adds one byte to the magic. Shift the copied body and
             * restore its newline before appending the new fields. */
            memmove(v9_text + v8_magic_size + 1U,
                    v9_text + v8_magic_size,
                    prefix - v8_magic_size + 1U);
            v9_text[v8_magic_size] = '\n';
            expect(snprintf(v9_text + v10_prefix,
                            sizeof(v9_text) - v10_prefix,
                 "dgn_fnv1a64=7\ndgn_descriptor_index=0\n"
                 "dgn_frame_sequence=40\ndgn_command_sequence=31\n"
                 "dgn_command_xa=0\ndgn_command_ya=0\n"
                 "dgn_command_xb=0\ndgn_command_yb=0\n"
                 "dgn_command_xc=0\ndgn_command_yc=0\n"
                 "dgn_command_xd=0\ndgn_command_yd=0\n"
                 "dgn_descriptor_fnv1a64=9\n%s", tail) >= 0 &&
               strlen(v9_text) < sizeof(v9_text),
               "V10 receipt extension is not truncated");
        }
        expect(nexus_v1_prs3_vdp1_capture_schema_parse(
                   v9_text, strlen(v9_text), &trace) &&
                   trace.schema_version == 10U &&
                   trace.dgn_descriptor_fnv1a64 == 9U,
               "V10 requires the observed DGN descriptor FNV");
        {
            char *descriptor_identity =
                strstr(v9_text, "dgn_descriptor_fnv1a64=9");
            expect(descriptor_identity != NULL,
                   "V10 receipt retains descriptor identity");
            if (descriptor_identity) {
                memcpy(descriptor_identity, "dgn_descriptor_fnv1a64=0", 24U);
                expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
                           v9_text, strlen(v9_text), &trace),
                       "V10 rejects a missing DGN descriptor FNV");
                /* The negative parse intentionally clears its output. Restore
                 * the valid V10 observation before the readiness checks below. */
                memcpy(descriptor_identity, "dgn_descriptor_fnv1a64=9", 24U);
                {
                    int restored = nexus_v1_prs3_vdp1_capture_schema_parse(
                        v9_text, strlen(v9_text), &trace);
                    expect(restored && trace.schema_version == 10U && trace.valid,
                           "V10 valid receipt is restored after negative check");
                }
            }
        }
    }
    expect(nexus_v1_prs3_decoder_readiness_bind_capture(
               &trace, menu, menu_size, dm_bin, dm_bin_size, &readiness) &&
           readiness.valid && readiness.capture_source_bound &&
           readiness.complete_input_range_bound &&
           readiness.complete_output_range_bound &&
           readiness.control_branch_coverage_bound &&
           !readiness.original_saturn_execution_authenticated &&
           !readiness.opcode_grammar_proven && !readiness.decoder_ready &&
           !readiness.decoder_promoted && !readiness.fallback_visuals_permitted,
           "V6 reaches decoder-grammar review only, never a decoder route");
    snprintf(transfer_text, sizeof(transfer_text),
        "NEXUS_PRS3_SH2_TRANSFER_TRACE_V1\n"
        "menu_bpk_fnv1a64=%llx\ndm_bin_fnv1a64=%llx\n"
        "entry_index=%x\nstream_offset=%x\nstream_size=%x\n"
        "expected_output_bytes=%x\npayload_byte_offset=0\n"
        "control_test_instruction_offset=14dca\nzero_branch_instruction_offset=14dcc\n"
        "fallthrough_counter_decrement_offset=14dd2\n"
        "observed_control_low_bit=1\nobserved_zero_branch_taken=0\n"
        "input_instruction_offset=14dd4\noutput_instruction_offset=14dd8\n"
        "input_read_sequence=11\noutput_write_sequence=13\n"
        "output_byte_offset=0\ninput_byte=%x\noutput_byte=%x\n",
        fnv1a64(menu, menu_size), fnv1a64(dm_bin, dm_bin_size), index,
        plan.stream_offset, plan.stream_size, plan.expected_output_bytes,
        menu[plan.stream_offset], menu[plan.stream_offset]);
    expect(nexus_v1_prs3_sh2_transfer_trace_parse_and_bind(
               transfer_text, strlen(transfer_text), menu, menu_size, dm_bin,
               dm_bin_size, 1, &transfer_trace, &transfer_receipt) &&
           nexus_v1_prs3_token_grammar_nonzero_candidate_bind(
               &readiness, &trace, &transfer_trace, &transfer_receipt,
               &token_receipt) && token_receipt.valid &&
           token_receipt.decoder_review_bound &&
           token_receipt.nonzero_transfer_bound &&
           token_receipt.input_output_sequence_bound &&
           token_receipt.observed_input_byte == token_receipt.observed_output_byte &&
           !token_receipt.original_saturn_execution_authenticated &&
           !token_receipt.token_operation_proven &&
           !token_receipt.opcode_grammar_proven && !token_receipt.decoder_ready &&
           !token_receipt.decoder_promoted && !token_receipt.fallback_visuals_permitted,
           "one real MENU.BPK byte reaches opaque token review without grammar promotion");
    memset(&zero_trace, 0, sizeof(zero_trace));
    zero_trace.menu_bpk_fnv1a64 = fnv1a64(menu, menu_size);
    zero_trace.dm_bin_fnv1a64 = fnv1a64(dm_bin, dm_bin_size);
    zero_trace.entry_index = index;
    zero_trace.stream_offset = plan.stream_offset;
    zero_trace.stream_size = plan.stream_size;
    zero_trace.expected_output_bytes = plan.expected_output_bytes;
    zero_trace.first_payload_byte_offset = 0U;
    zero_trace.second_payload_byte_offset = 1U;
    zero_trace.zero_branch_instruction_offset = 0x14dccU;
    zero_trace.zero_branch_target_offset = 0x14de4U;
    zero_trace.counter_decrement_offset = 0x14de4U;
    zero_trace.first_input_instruction_offset = 0x14decU;
    zero_trace.second_input_instruction_offset = 0x14df0U;
    zero_trace.first_input_read_sequence = 0x11U;
    zero_trace.second_input_read_sequence = 0x12U;
    zero_trace.first_input_byte = menu[plan.stream_offset];
    zero_trace.second_input_byte = menu[plan.stream_offset + 1U];
    zero_trace.merged_control_value = zero_trace.first_input_byte |
        ((zero_trace.second_input_byte << 4U) & 0x0f00U);
    expect(nexus_v1_prs3_sh2_zero_side_trace_bind(
               &zero_trace, menu, menu_size, dm_bin, dm_bin_size, 1,
               &zero_receipt) && zero_receipt.observed_zero_side_merge,
           "real zero-side bytes bind to the original SH-2 merge route");
    (void)nexus_v1_prs3_control_grammar_review_bind(
        &readiness, &trace, &transfer_trace, &transfer_receipt,
        &zero_trace, &zero_receipt, &control_review);
    expect(control_review.decoder_review_bound,
           "combined review retains V5 decoder-review binding");
    expect(control_review.nonzero_path_bound,
           "combined review retains source-bound nonzero path");
    expect(zero_receipt.entry_plan_matches,
           "zero-side receipt retains the selected stream plan");
    expect(zero_trace.entry_index == trace.entry_index &&
           zero_trace.stream_offset == trace.stream_offset &&
           zero_trace.stream_size == trace.stream_size &&
           zero_trace.expected_output_bytes == trace.expected_output_bytes,
           "zero-side identity matches the V5 stream");
    expect(zero_trace.second_payload_byte_offset < trace.input_read_bytes &&
           zero_trace.first_input_read_sequence >= trace.first_input_read_sequence &&
           zero_trace.second_input_read_sequence <= trace.last_input_read_sequence,
           "zero-side range fits the V5 input interval");
    expect(control_review.zero_side_path_bound,
           "combined review retains source-bound zero-side path");
    expect(control_review.valid && control_review.control_branch_evidence_complete &&
           !control_review.original_saturn_execution_authenticated &&
           !control_review.token_grammar_proven && !control_review.decoder_ready &&
           !control_review.decoder_promoted &&
           !control_review.fallback_visuals_permitted,
           "both real low-bit paths join only as opaque grammar review evidence");
    zero_trace.second_input_read_sequence = 0x23U;
    expect(!nexus_v1_prs3_control_grammar_review_bind(
               &readiness, &trace, &transfer_trace, &transfer_receipt,
               &zero_trace, &zero_receipt, &control_review) && !control_review.valid,
           "combined review rejects a zero-side observation outside the complete input interval");
    transfer_trace.output_byte_offset = plan.expected_output_bytes;
    expect(!nexus_v1_prs3_token_grammar_nonzero_candidate_bind(
               &readiness, &trace, &transfer_trace, &transfer_receipt,
               &token_receipt) && !token_receipt.valid,
           "token review rejects a transfer offset not represented by its receipt");
    memcpy(strstr(text, "nonzero_output_store_instruction_offset=14dd8"),
           "nonzero_output_store_instruction_offset=14dd9",
           sizeof("nonzero_output_store_instruction_offset=14dd9") - 1U);
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               text, strlen(text), &trace) &&
           !nexus_v1_prs3_vdp1_capture_schema_bind_assets(
               &trace, menu, menu_size, dm_bin, dm_bin_size, &binding) &&
           !binding.valid,
           "V7 rejects a nonzero byte witness at the wrong retail store PC");
    memcpy(strstr(text, "nonzero_output_store_instruction_offset=14dd9"),
           "nonzero_output_store_instruction_offset=14dd8",
           sizeof("nonzero_output_store_instruction_offset=14dd8") - 1U);
    memcpy(strstr(text, "zero_first_input_instruction_offset=14dec"),
           "zero_first_input_instruction_offset=14ded",
           sizeof("zero_first_input_instruction_offset=14ded") - 1U);
    memcpy(strstr(text, "zero_second_input_instruction_offset=14df0"),
           "zero_second_input_instruction_offset=14df1",
           sizeof("zero_second_input_instruction_offset=14df1") - 1U);
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               text, strlen(text), &trace) &&
           !nexus_v1_prs3_vdp1_capture_schema_bind_assets(
               &trace, menu, menu_size, dm_bin, dm_bin_size, &binding) &&
           !binding.valid,
           "V8 rejects a zero-side witness at the wrong retail read PC");
    memcpy(strstr(text, "zero_first_input_instruction_offset=14ded"),
           "zero_first_input_instruction_offset=14dec",
           sizeof("zero_first_input_instruction_offset=14dec") - 1U);
    memcpy(strstr(text, "zero_second_input_instruction_offset=14df1"),
           "zero_second_input_instruction_offset=14df0",
           sizeof("zero_second_input_instruction_offset=14df0") - 1U);
    memcpy(strstr(text, "zero_second_input_payload_byte_offset=1"),
           "zero_second_input_payload_byte_offset=2",
           sizeof("zero_second_input_payload_byte_offset=2") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               text, strlen(text), &trace) && !trace.valid,
           "V8 rejects a non-adjacent zero-side source pair");
    memcpy(strstr(text, "zero_second_input_payload_byte_offset=2"),
           "zero_second_input_payload_byte_offset=1",
           sizeof("zero_second_input_payload_byte_offset=1") - 1U);
    memcpy(strstr(text, "nonzero_counter_after=4"),
           "nonzero_counter_after=5",
           sizeof("nonzero_counter_after=5") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               text, strlen(text), &trace) && !trace.valid,
           "V6 rejects a dynamic nonzero counter debit mismatch");
    memcpy(strstr(text, "nonzero_counter_after=5"),
           "nonzero_counter_after=4",
           sizeof("nonzero_counter_after=4") - 1U);
    memcpy(strstr(text, "control_branch_outcomes_mask=3"),
           "control_branch_outcomes_mask=1",
           sizeof("control_branch_outcomes_mask=1") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               text, strlen(text), &trace) && !trace.valid,
           "V5 rejects a purported complete stream missing one control outcome");
    free(menu); free(dm_bin);
}

static void test_real_v3_raw_sidecar_provenance_no_runtime(void) {
    const char *home = getenv("HOME");
    char data_dir[1024];
    char menu_path[1200];
    char dm_path[1200];
    char trace_path[1200];
    char output_path[1200];
    char command_path[1200];
    char palette_path[1200];
    char producer_path[1200];
    char attestation_path[1200];
    char ledger_path[1200];
    unsigned char *menu = NULL;
    unsigned char *dm_bin = NULL;
    unsigned char *output = NULL;
    unsigned char command[0x20];
    unsigned char palette[0x20];
    unsigned char producer[] = "mednafen-vdp1-capture-producer";
    size_t menu_size = 0U;
    size_t dm_bin_size = 0U;
    uint32_t entry_index;
    Nexus_V1_BpkPrs3StreamPlan plan;
    char trace[4096];
    char attestation[1024];
    int wrote = 0;
    Nexus_V1_Prs3Vdp1RawSidecarReceipt raw_sidecars;
    Nexus_V1_Prs3Vdp1ProvenanceReceipt provenance;
    Nexus_V1_Prs3Vdp1ProducerAttestationReceipt producer_attestation;
    Nexus_V1_Prs3Vdp1ReviewedUploadReceipt reviewed_upload;

    if (!home) {
        puts("SKIP: HOME is unset; no local Nexus PRS3 V3 sidecar provenance check");
        return;
    }
    snprintf(data_dir, sizeof(data_dir), "%s/.firestaff/data/nexus", home);
    snprintf(menu_path, sizeof(menu_path), "%s/MENU.BPK", data_dir);
    snprintf(dm_path, sizeof(dm_path), "%s/DM.BIN", data_dir);
    menu = read_asset(data_dir, "MENU.BPK", &menu_size);
    dm_bin = read_asset(data_dir, "DM.BIN", &dm_bin_size);
    if (!menu || !dm_bin) {
        puts("SKIP: local MENU.BPK/DM.BIN not present for PRS3 V3 sidecar provenance");
        free(menu);
        free(dm_bin);
        return;
    }

    memset(&plan, 0, sizeof(plan));
    for (entry_index = 0U; entry_index < 256U; ++entry_index) {
        if (nexus_v1_bpk_archive_prs3_stream_plan(
                menu, menu_size, entry_index, &plan) ==
                NEXUS_V1_BPK_PRS3_STREAM_OK &&
            plan.expected_output_bytes > 0U &&
            plan.expected_output_bytes <= 1024U * 1024U) {
            break;
        }
    }
    if (entry_index == 256U) {
        puts("SKIP: no bounded local MENU.BPK PRS3 stream for V3 sidecar provenance");
        free(menu);
        free(dm_bin);
        return;
    }

    output = (unsigned char *)calloc(plan.expected_output_bytes, 1U);
    if (!output) {
        free(menu);
        free(dm_bin);
        expect(0, "V3 sidecar output fixture allocates");
        return;
    }
    memset(command, 0, sizeof(command));
    memset(palette, 0, sizeof(palette));
    snprintf(trace, sizeof(trace),
             "NEXUS_PRS3_SH2_VDP1_TRACE_V3\n"
             "menu_bpk_fnv1a64=%llx\n"
             "dm_bin_fnv1a64=%llx\n"
             "entry_index=%x\nstream_offset=%x\nstream_size=%x\n"
             "expected_output_bytes=%x\n"
             "payload_ram_address=6010000\nfirst_input_read_address=6010000\n"
             "last_input_read_address=%x\ninput_read_bytes=%x\n"
             "payload_fnv1a64=%llx\n"
             "output_ram_address=6020000\nfirst_output_write_address=6020000\n"
             "last_output_write_address=%x\noutput_write_bytes=%x\n"
             "output_fnv1a64=%llx\n"
             "first_opcode_sequence=10\nfirst_input_read_sequence=11\n"
             "last_input_read_sequence=12\nfirst_output_write_sequence=13\n"
             "last_output_write_sequence=14\ndecoder_return_sequence=15\n"
             "vdp1_command_sequence=16\nvdp1_command_address=5c00000\n"
             "vdp1_texture_source_address=6020000\n"
             "vdp1_texture_source_bytes=%x\n"
             "vdp1_texture_first_read_sequence=17\n"
             "vdp1_texture_last_read_sequence=18\n"
             "vdp1_texture_first_read_address=6020000\n"
             "vdp1_texture_last_read_address=%x\n"
             "vdp1_texture_read_bytes=%x\nvdp1_texture_fnv1a64=%llx\n"
             "vdp1_command_first_read_sequence=17\n"
             "vdp1_command_last_read_sequence=17\n"
             "vdp1_command_first_read_address=5c00000\n"
             "vdp1_command_last_read_address=5c0001f\n"
             "vdp1_command_read_bytes=20\nvdp1_command_fnv1a64=%llx\n"
             "palette_first_read_sequence=17\npalette_last_read_sequence=18\n"
             "palette_first_read_address=5f00000\n"
             "palette_last_read_address=5f0001f\n"
             "palette_read_bytes=20\npalette_fnv1a64=%llx\n"
             "decoder_returned_success=1\ncapture_complete=1\n",
             fnv1a64(menu, menu_size), fnv1a64(dm_bin, dm_bin_size),
             entry_index, plan.stream_offset, plan.stream_size,
             plan.expected_output_bytes,
             0x6010000U + plan.stream_size - 1U, plan.stream_size,
             fnv1a64(menu + plan.stream_offset, plan.stream_size),
             0x6020000U + plan.expected_output_bytes - 1U,
             plan.expected_output_bytes, fnv1a64(output, plan.expected_output_bytes),
             plan.expected_output_bytes,
             0x6020000U + plan.expected_output_bytes - 1U,
             plan.expected_output_bytes, fnv1a64(output, plan.expected_output_bytes),
             fnv1a64(command, sizeof(command)), fnv1a64(palette, sizeof(palette)));

    snprintf(trace_path, sizeof(trace_path), "/tmp/nexus-v3-trace-%ld.txt",
             (long)getpid());
    snprintf(output_path, sizeof(output_path), "/tmp/nexus-v3-output-%ld.bin",
             (long)getpid());
    snprintf(command_path, sizeof(command_path), "/tmp/nexus-v3-command-%ld.bin",
             (long)getpid());
    snprintf(palette_path, sizeof(palette_path), "/tmp/nexus-v3-palette-%ld.bin",
             (long)getpid());
    snprintf(producer_path, sizeof(producer_path), "/tmp/nexus-v3-producer-%ld.bin",
             (long)getpid());
    snprintf(ledger_path, sizeof(ledger_path), "/tmp/nexus-v3-ledger-%ld.txt",
             (long)getpid());
    snprintf(attestation_path, sizeof(attestation_path),
             "/tmp/nexus-v3-attestation-%ld.txt", (long)getpid());
    snprintf(attestation, sizeof(attestation),
             "NEXUS_PRS3_V3_PRODUCER_ATTESTATION_V1\n"
             "producer_name=MEDNAFEN\n"
             "capture_mode=SH2_VDP1_BUS_TRACE\n"
             "original_saturn_execution=CLAIMED\n"
             "trace_fnv1a64=%llx\n"
             "output_fnv1a64=%llx\n"
             "vdp1_command_fnv1a64=%llx\n"
             "palette_fnv1a64=%llx\n"
             "producer_binary_fnv1a64=%llx\n",
             fnv1a64((const unsigned char *)trace, strlen(trace)),
             fnv1a64(output, plan.expected_output_bytes),
             fnv1a64(command, sizeof(command)),
             fnv1a64(palette, sizeof(palette)),
             fnv1a64(producer, sizeof(producer) - 1U));

    wrote = write_file_bytes(trace_path, (const unsigned char *)trace,
                             strlen(trace)) &&
            write_file_bytes(output_path, output, plan.expected_output_bytes) &&
            write_file_bytes(command_path, command, sizeof(command)) &&
            write_file_bytes(palette_path, palette, sizeof(palette)) &&
            write_file_bytes(producer_path, producer, sizeof(producer) - 1U) &&
            write_file_bytes(attestation_path,
                             (const unsigned char *)attestation,
                             strlen(attestation));
    expect(wrote, "V3 sidecar provenance fixtures write to temp files");
    if (wrote) {
        memset(&raw_sidecars, 0, sizeof(raw_sidecars));
        expect(nexus_v1_prs3_vdp1_capture_validate_raw_sidecars(
                   trace_path, menu_path, dm_path, output_path, command_path,
                   palette_path, &raw_sidecars) &&
                   raw_sidecars.trace_source_bound &&
                   raw_sidecars.output_sidecar_bound &&
                   raw_sidecars.vdp1_command_sidecar_bound &&
                   raw_sidecars.palette_sidecar_bound &&
                   raw_sidecars.raw_sidecars_bound &&
                   !raw_sidecars.capture_producer_authenticated &&
                   !raw_sidecars.runtime_import_permitted &&
                   !raw_sidecars.decoder_promoted &&
                   !raw_sidecars.fallback_visuals_permitted,
               "real MENU.BPK V3 raw sidecars bind but do not permit runtime import");
        memset(&provenance, 0, sizeof(provenance));
        expect(nexus_v1_prs3_vdp1_capture_write_provenance_ledger(
                   ledger_path, trace_path, menu_path, dm_path, output_path,
                   command_path, palette_path, producer_path, &raw_sidecars),
               "real MENU.BPK V3 provenance ledger writes from bound sidecars");
        expect(nexus_v1_prs3_vdp1_capture_validate_provenance(
                   ledger_path, trace_path, output_path, command_path,
                   palette_path, producer_path, &raw_sidecars, &provenance) &&
                   provenance.ledger_parsed &&
                   provenance.raw_sidecars_bound &&
                   provenance.trace_bytes_match &&
                   provenance.output_bytes_match &&
                   provenance.vdp1_command_bytes_match &&
                   provenance.palette_bytes_match &&
                   provenance.producer_binary_bound &&
                   provenance.provenance_complete &&
                   !provenance.capture_producer_authenticated &&
                   !provenance.runtime_import_permitted,
               "real MENU.BPK V3 provenance ledger remains evidence-only");
        memset(&producer_attestation, 0, sizeof(producer_attestation));
        expect(nexus_v1_prs3_vdp1_capture_validate_producer_attestation(
                   attestation_path, trace_path, output_path, command_path,
                   palette_path, producer_path, &raw_sidecars, &provenance,
                   &producer_attestation) &&
                   producer_attestation.attestation_file_read &&
                   producer_attestation.attestation_parsed &&
                   producer_attestation.raw_sidecars_bound &&
                   producer_attestation.provenance_complete &&
                   producer_attestation.producer_binary_bound &&
                   producer_attestation.capture_mode_declared &&
                   producer_attestation.original_saturn_execution_claimed &&
                   producer_attestation.artifact_hashes_bound &&
                   producer_attestation.workflow_complete &&
                   producer_attestation.independent_authentication_required &&
                   !producer_attestation.capture_producer_authenticated &&
                   !producer_attestation.runtime_import_permitted &&
                   !producer_attestation.decoder_promoted &&
                   !producer_attestation.fallback_visuals_permitted,
               "real MENU.BPK V3 producer attestation binds but does not authenticate Saturn execution");
        memset(&reviewed_upload, 0, sizeof(reviewed_upload));
        expect(nexus_v1_prs3_vdp1_capture_review_menu_bpk_upload(
                   &raw_sidecars, &provenance, &producer_attestation,
                   &reviewed_upload) &&
                   reviewed_upload.raw_sidecars_bound &&
                   reviewed_upload.provenance_complete &&
                   reviewed_upload.producer_attestation_bound &&
                   reviewed_upload.producer_binary_bound &&
                   reviewed_upload.artifact_hashes_bound &&
                   reviewed_upload.original_saturn_execution_claimed &&
                   reviewed_upload.independent_authentication_required &&
                   reviewed_upload.reviewed_upload_path_bound &&
                   reviewed_upload.menu_bpk_upload_reviewed &&
                   !reviewed_upload.original_saturn_capture_authenticated &&
                   !reviewed_upload.runtime_upload_permitted &&
                   !reviewed_upload.decoder_promoted &&
                   !reviewed_upload.fallback_visuals_permitted,
               "reviewed MENU.BPK upload path stays fail-closed without independent Saturn authentication");
        producer_attestation.artifact_hashes_bound = 0;
        expect(!nexus_v1_prs3_vdp1_capture_review_menu_bpk_upload(
                   &raw_sidecars, &provenance, &producer_attestation,
                   &reviewed_upload) &&
                   !reviewed_upload.reviewed_upload_path_bound &&
                   !reviewed_upload.menu_bpk_upload_reviewed &&
                   !reviewed_upload.runtime_upload_permitted &&
                   !reviewed_upload.fallback_visuals_permitted,
               "reviewed MENU.BPK upload path rejects attestation artifact drift");
    }

    remove(trace_path);
    remove(output_path);
    remove(command_path);
    remove(palette_path);
    remove(producer_path);
    remove(attestation_path);
    remove(ledger_path);
    free(output);
    free(menu);
    free(dm_bin);
}

int main(void) {
    Nexus_V1_Prs3CaptureTraceSchemaReceipt receipt;
    Nexus_V1_Prs3CaptureAssetBindingReceipt binding;
    Nexus_V1_Prs3Vdp1CaptureReceipt vdp1_receipt;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt vdp1_binding;
    char malformed[sizeof(valid_trace)];
    char vdp1_malformed[sizeof(valid_vdp1_v4_trace) + 64U];
    char bound_trace[2048];
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

    make_bpk(bpk);
    snprintf(bound_trace, sizeof(bound_trace), valid_vdp1_trace,
             fnv1a64(bpk, sizeof(bpk)), fnv1a64(dm_bin, sizeof(dm_bin)),
             fnv1a64(bpk + 64U, 4U));
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               bound_trace, strlen(bound_trace), &vdp1_receipt) &&
               nexus_v1_prs3_vdp1_capture_schema_bind_assets(
               &vdp1_receipt, bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
                   &vdp1_binding) && vdp1_binding.valid &&
               vdp1_binding.exact_vdp1_handoff_observed &&
               vdp1_receipt.schema_version == 2U &&
               vdp1_receipt.vdp1_texture_consumption_observed &&
               vdp1_binding.vdp1_texture_consumption_observed &&
               vdp1_binding.payload_span_matches &&
               !vdp1_receipt.opcode_grammar_proven &&
               !vdp1_binding.decoder_promoted &&
               !vdp1_binding.fallback_visuals_permitted,
           "complete SH-2 to VDP1 capture binds one exact frame without a decoder");
    snprintf(bound_trace, sizeof(bound_trace), valid_vdp1_v3_trace,
             fnv1a64(bpk, sizeof(bpk)), fnv1a64(dm_bin, sizeof(dm_bin)),
             fnv1a64(bpk + 64U, 4U));
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               bound_trace, strlen(bound_trace), &vdp1_receipt) &&
               nexus_v1_prs3_vdp1_capture_schema_bind_assets(
                   &vdp1_receipt, bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
                   &vdp1_binding) && vdp1_receipt.schema_version == 3U &&
               vdp1_receipt.vdp1_command_consumption_observed &&
               vdp1_receipt.palette_consumption_observed &&
               vdp1_binding.vdp1_command_consumption_observed &&
               vdp1_binding.palette_consumption_observed &&
               !vdp1_receipt.decoder_promoted &&
               !vdp1_binding.fallback_visuals_permitted,
           "V3 capture binds original command and palette read witnesses without decoding them");
    snprintf(bound_trace, sizeof(bound_trace), valid_vdp1_v4_trace,
             fnv1a64(bpk, sizeof(bpk)), fnv1a64(dm_bin, sizeof(dm_bin)),
             fnv1a64(bpk + 64U, 4U));
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               bound_trace, strlen(bound_trace), &vdp1_receipt) &&
               vdp1_receipt.schema_version == 4U &&
               vdp1_receipt.output_store_predecessor_observed &&
               vdp1_receipt.complete_output_store_range_observed &&
               !vdp1_receipt.decoder_promoted,
           "V4 accepts a complete output-store intake without decoder promotion");
    expect(!nexus_v1_prs3_vdp1_capture_schema_bind_assets(
               &vdp1_receipt, bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
               &vdp1_binding) && !vdp1_binding.valid,
           "V4 rejects a non-original DM.BIN before accepting store PCs");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "output_store_instruction_offset=14dd8"),
           "output_store_instruction_offset=14dd6",
           sizeof("output_store_instruction_offset=14dd6") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "V4 rejects output-store PCs without the source predecessor relation");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "last_output_store_address=6020003"),
           "last_output_store_address=6020004",
           sizeof("last_output_store_address=6020004") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "V4 rejects an output-store range that differs from the output receipt");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "palette_last_read_address=5f0001f"),
           "palette_last_read_address=5f00020",
           sizeof("palette_last_read_address=5f00020") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "V3 rejects a palette read span that is not contiguous");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(vdp1_malformed, NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC,
           sizeof(NEXUS_V1_PRS3_VDP1_CAPTURE_SCHEMA_MAGIC) - 1U);
    {
        char *texture_reads = strstr(
            vdp1_malformed, "vdp1_texture_first_read_sequence=");
        char *return_status = strstr(vdp1_malformed, "decoder_returned_success=");
        memmove(texture_reads, return_status, strlen(return_status) + 1U);
    }
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               vdp1_receipt.schema_version == 1U &&
               !vdp1_receipt.vdp1_texture_consumption_observed,
           "V1 captures remain address-only and cannot claim VDP1 consumption");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "vdp1_texture_fnv1a64=3"),
           "vdp1_texture_fnv1a64=4", sizeof("vdp1_texture_fnv1a64=4") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "VDP1 texture reads must fingerprint the decoder output bytes");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "vdp1_texture_first_read_address=6020000"),
           "vdp1_texture_first_read_address=6020001",
           sizeof("vdp1_texture_first_read_address=6020001") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "VDP1 texture reads must start at the decoder output base");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "payload_fnv1a64="), "payload_missing=",
           sizeof("payload_missing=") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "missing captured payload fingerprint rejects the VDP1 trace");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "payload_fnv1a64="),
           "payload_fnv1a64=4", sizeof("payload_fnv1a64=4") - 1U);
    expect(nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !nexus_v1_prs3_vdp1_capture_schema_bind_assets(
                   &vdp1_receipt, bpk, sizeof(bpk), dm_bin, sizeof(dm_bin),
                   &vdp1_binding) && !vdp1_binding.valid &&
               vdp1_binding.menu_bpk_matches &&
               !vdp1_binding.payload_span_matches,
           "a claimed SH-2 payload fingerprint must match the exact BPK span");
    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    {
        char *expected_bytes =
            strstr(vdp1_malformed, "expected_output_bytes=4");
        expected_bytes[strlen("expected_output_bytes=")] = '3';
    }
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "VDP1 source range mismatch rejects a supposedly complete capture");

    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "first_input_read_sequence=11"),
           "first_input_read_sequence=09",
           sizeof("first_input_read_sequence=09") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "VDP1 input interval before the first opcode is rejected");

    snprintf(vdp1_malformed, sizeof(vdp1_malformed), "%s", bound_trace);
    memcpy(strstr(vdp1_malformed, "first_output_write_sequence=13"),
           "first_output_write_sequence=09",
           sizeof("first_output_write_sequence=09") - 1U);
    expect(!nexus_v1_prs3_vdp1_capture_schema_parse(
               vdp1_malformed, strlen(vdp1_malformed), &vdp1_receipt) &&
               !vdp1_receipt.valid,
           "VDP1 output interval before the first opcode is rejected");

    {
        Nexus_V1_Prs3Vdp1CaptureFileReceipt file_receipt;
        Nexus_V1_Prs3Vdp1RawSidecarReceipt sidecar_receipt;
        Nexus_V1_Prs3Vdp1ProvenanceReceipt provenance_receipt;
        Nexus_V1_Prs3Vdp1ProducerAttestationReceipt attestation_receipt;
        Nexus_V1_Prs3Vdp1ReviewedUploadReceipt reviewed_upload_receipt;
        Nexus_V1_BpkPrs3DecodedOutputProofReceipt output_proof;
        Nexus_V1_Prs3Vdp1ReviewedOutputUploadReceipt output_upload;
        expect(!nexus_v1_prs3_vdp1_capture_validate_files(
                   "/missing/nexus-v3.trace", "/missing/MENU.BPK",
                   "/missing/DM.BIN", &file_receipt) &&
                   !file_receipt.trace_file_read &&
                   !file_receipt.menu_bpk_original_hash_verified &&
                   !file_receipt.dm_bin_original_hash_verified &&
                   !file_receipt.source_bound_capture &&
                   !file_receipt.runtime_import_permitted &&
                   !file_receipt.decoder_promoted &&
                   !file_receipt.fallback_visuals_permitted,
               "file validator rejects absent or non-canonical capture inputs without a runtime route");
        expect(!nexus_v1_prs3_vdp1_capture_validate_raw_sidecars(
                   "/missing/nexus-v3.trace", "/missing/MENU.BPK",
                   "/missing/DM.BIN", "/missing/output.bin",
                   "/missing/command.bin", "/missing/palette.bin",
                   &sidecar_receipt) &&
                   !sidecar_receipt.trace_source_bound &&
                   !sidecar_receipt.output_sidecar_bound &&
                   !sidecar_receipt.vdp1_command_sidecar_bound &&
                   !sidecar_receipt.palette_sidecar_bound &&
                   !sidecar_receipt.raw_sidecars_bound &&
                   !sidecar_receipt.capture_producer_authenticated &&
                   !sidecar_receipt.runtime_import_permitted &&
                   !sidecar_receipt.decoder_promoted &&
                   !sidecar_receipt.fallback_visuals_permitted,
               "raw sidecar admission rejects absent capture artifacts without a substitute route");
        expect(!nexus_v1_prs3_vdp1_capture_validate_provenance(
                   "/missing/ledger", "/missing/trace", "/missing/output",
                   "/missing/command", "/missing/palette", "/missing/producer",
                   &sidecar_receipt, &provenance_receipt) &&
                   !provenance_receipt.ledger_parsed &&
                   !provenance_receipt.provenance_complete &&
                   !provenance_receipt.producer_binary_bound &&
                   !provenance_receipt.capture_producer_authenticated &&
                   !provenance_receipt.runtime_import_permitted,
               "provenance gate rejects an absent raw capture ledger without authenticating a producer");
        expect(!nexus_v1_prs3_vdp1_capture_write_provenance_ledger(
                   "/tmp/nexus-v3-ledger", "/missing/trace", "/missing/MENU.BPK",
                   "/missing/DM.BIN", "/missing/output", "/missing/command",
                   "/missing/palette", "/missing/producer", &sidecar_receipt) &&
                   !sidecar_receipt.raw_sidecars_bound &&
                   !sidecar_receipt.runtime_import_permitted &&
                   !sidecar_receipt.decoder_promoted &&
                   !sidecar_receipt.fallback_visuals_permitted,
               "bundle writer rejects absent capture artifacts without writing a substitute ledger");
        expect(!nexus_v1_prs3_vdp1_capture_validate_producer_attestation(
                   "/missing/attestation", "/missing/trace", "/missing/output",
                   "/missing/command", "/missing/palette", "/missing/producer",
                   &sidecar_receipt, &provenance_receipt, &attestation_receipt) &&
                   !attestation_receipt.attestation_file_read &&
                   !attestation_receipt.workflow_complete &&
                   attestation_receipt.independent_authentication_required &&
                   !attestation_receipt.capture_producer_authenticated &&
                   !attestation_receipt.runtime_import_permitted &&
                   !attestation_receipt.decoder_promoted &&
                   !attestation_receipt.fallback_visuals_permitted,
               "producer attestation rejects absent evidence without authenticating a capture source");
        expect(!nexus_v1_prs3_vdp1_capture_review_menu_bpk_upload(
                   &sidecar_receipt, &provenance_receipt,
                   &attestation_receipt, &reviewed_upload_receipt) &&
                   reviewed_upload_receipt.independent_authentication_required &&
                   !reviewed_upload_receipt.raw_sidecars_bound &&
                   !reviewed_upload_receipt.provenance_complete &&
                   !reviewed_upload_receipt.producer_attestation_bound &&
                   !reviewed_upload_receipt.reviewed_upload_path_bound &&
                   !reviewed_upload_receipt.menu_bpk_upload_reviewed &&
                   !reviewed_upload_receipt.runtime_upload_permitted &&
                   !reviewed_upload_receipt.decoder_promoted &&
                   !reviewed_upload_receipt.fallback_visuals_permitted,
               "reviewed MENU.BPK upload path rejects missing evidence without a runtime route");
        memset(&output_proof, 0, sizeof(output_proof));
        output_proof.status =
            NEXUS_V1_BPK_PRS3_OUTPUT_PROOF_SOURCE_BOUND_NO_RUNTIME;
        output_proof.entry_index = 4U;
        output_proof.stream_offset = 0x200U;
        output_proof.stream_size = 0x40U;
        output_proof.expected_output_bytes = 0x80U;
        output_proof.observed_output_bytes = 0x80U;
        output_proof.observed_output_fnv1a64 = 0x12345678U;
        output_proof.length_matches = 1;
        output_proof.hash_matches = 1;
        output_proof.capture_source_bound = 1;
        output_proof.decoded_output_sidecar_bound = 1;
        output_proof.original_saturn_provenance_verified = 1;
        output_proof.decoded_output_proof_ready = 1;
        memset(&reviewed_upload_receipt, 0, sizeof(reviewed_upload_receipt));
        reviewed_upload_receipt.reviewed_upload_path_bound = 1;
        reviewed_upload_receipt.menu_bpk_upload_reviewed = 1;
        reviewed_upload_receipt.producer_attestation_bound = 1;
        reviewed_upload_receipt.producer_binary_bound = 1;
        reviewed_upload_receipt.artifact_hashes_bound = 1;
        reviewed_upload_receipt.original_saturn_execution_claimed = 1;
        reviewed_upload_receipt.independent_authentication_required = 1;
        expect(nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   output_upload.decoded_output_proof_bound &&
                   output_upload.decoded_output_sidecar_bound &&
                   output_upload.reviewed_upload_path_bound &&
                   output_upload.menu_bpk_upload_reviewed &&
                   output_upload.original_saturn_provenance_verified &&
                   output_upload.independent_authentication_required &&
                   !output_upload.original_saturn_capture_authenticated &&
                   output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.decoder_promoted &&
                   !output_upload.fallback_visuals_permitted &&
                   output_upload.entry_index == 4U &&
                   output_upload.stream_offset == 0x200U &&
                   output_upload.stream_size == 0x40U &&
                   output_upload.expected_output_bytes == 0x80U &&
                   output_upload.output_fnv1a64 == 0x12345678U,
               "reviewed PRS3 output/upload evidence remains source-bound no-runtime");
        reviewed_upload_receipt.runtime_upload_permitted = 1;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.reviewed_upload_path_bound &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review rejects a pre-promoted runtime upload route");
        reviewed_upload_receipt.runtime_upload_permitted = 0;
        output_proof.capture_source_bound = 0;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.decoded_output_proof_bound &&
                   output_upload.decoded_output_sidecar_bound &&
                   output_upload.reviewed_upload_path_bound &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.decoder_promoted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review requires retained PRS3 capture source binding");
        output_proof.capture_source_bound = 1;

        output_proof.length_matches = 0;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.decoded_output_proof_bound &&
                   output_upload.reviewed_upload_path_bound &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review requires retained PRS3 output length match");
        output_proof.length_matches = 1;

        output_proof.hash_matches = 0;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.decoded_output_proof_bound &&
                   output_upload.reviewed_upload_path_bound &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review requires retained PRS3 output hash match");
        output_proof.hash_matches = 1;

        output_proof.observed_output_bytes = 0x40U;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.decoded_output_proof_bound &&
                   output_upload.expected_output_bytes == 0x80U &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review rejects drift between observed and expected PRS3 output bytes");
        output_proof.observed_output_bytes = 0x80U;

        output_proof.observed_output_fnv1a64 = 0;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.decoded_output_proof_bound &&
                   output_upload.output_fnv1a64 == 0 &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review requires a concrete PRS3 output fingerprint");
        output_proof.observed_output_fnv1a64 = 0x12345678U;

        output_proof.fallback_visuals_permitted = 1;
        expect(!nexus_v1_prs3_vdp1_capture_review_output_upload(
                   &output_proof, &reviewed_upload_receipt,
                   &output_upload) &&
                   !output_upload.decoded_output_proof_bound &&
                   !output_upload.source_bound_no_runtime &&
                   !output_upload.runtime_upload_permitted &&
                   !output_upload.fallback_visuals_permitted,
               "output/upload review rejects PRS3 proof with fallback visuals");
    }

    test_dm_bin_prs3_catalog();
    test_cross_asset_prs3_frame_receipt();
    test_dm_bin_sh2_v1_execution_receipt();
    test_real_nonzero_transfer_trace_contract();
    test_real_zero_side_trace_contract();
    test_real_v5_decoder_readiness_trace_contract();
    test_real_v3_raw_sidecar_provenance_no_runtime();
    test_sh2_transfer_trace_gate();

    return failures ? 1 : 0;
}
