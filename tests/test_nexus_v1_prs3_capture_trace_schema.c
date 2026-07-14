#include "nexus_v1_prs3_capture_trace_schema.h"

#include <stdio.h>
#include <stdlib.h>
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

static const char valid_vdp1_trace[] =
    "NEXUS_PRS3_SH2_VDP1_TRACE_V1\n"
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

static void test_dm_bin_prs3_catalog(void) {
    unsigned char fixture[48];
    Nexus_V1_Prs3DmBinCatalogReceipt receipt;
    const char *data_dir = getenv("FIRESTAFF_NEXUS_DATA_DIR");
    char path[1024];
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
    char path[1024];
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
               receipt.output_byte_store_offset == 85464U &&
               receipt.loop_branch_offset == 85472U &&
               receipt.control_test_instruction == 0x23b8U &&
               receipt.stream_byte_read_instruction == 0x62c4U &&
               receipt.output_byte_store_instruction == 0x0d24U &&
               receipt.loop_branch_instruction == 0xafe8U &&
               receipt.sh2_control_path_verified &&
               receipt.sh2_stream_read_verified && receipt.sh2_output_store_verified &&
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
    free(dm_bin);
}

int main(void) {
    Nexus_V1_Prs3CaptureTraceSchemaReceipt receipt;
    Nexus_V1_Prs3CaptureAssetBindingReceipt binding;
    Nexus_V1_Prs3Vdp1CaptureReceipt vdp1_receipt;
    Nexus_V1_Prs3Vdp1CaptureBindingReceipt vdp1_binding;
    char malformed[sizeof(valid_trace)];
    char vdp1_malformed[sizeof(valid_vdp1_trace) + 64U];
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
               vdp1_binding.payload_span_matches &&
               !vdp1_receipt.opcode_grammar_proven &&
               !vdp1_binding.decoder_promoted &&
               !vdp1_binding.fallback_visuals_permitted,
           "complete SH-2 to VDP1 capture binds one exact frame without a decoder");
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

    test_dm_bin_prs3_catalog();
    test_cross_asset_prs3_frame_receipt();
    test_dm_bin_sh2_v1_execution_receipt();

    return failures ? 1 : 0;
}
