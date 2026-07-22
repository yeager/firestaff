#include "nexus_v1_prs3_original_execution_import.h"
#include "firestaff_x68k_media_receipt.h"

#include <string.h>

static uint64_t fnv1a64(const uint8_t *data, size_t size)
{
    uint64_t value = UINT64_C(1469598103934665603);
    size_t i;
    for (i = 0; i < size; ++i) { value ^= data[i]; value *= UINT64_C(1099511628211); }
    return value;
}

static int sha256_shape(const char *text)
{
    int i;
    if (!text || strlen(text) != 64U) return 0;
    for (i = 0; i < 64; ++i)
        if (!((text[i] >= '0' && text[i] <= '9') || (text[i] >= 'a' && text[i] <= 'f') ||
              (text[i] >= 'A' && text[i] <= 'F'))) return 0;
    return 1;
}

int nexus_v1_prs3_original_execution_evidence_import(
    const Nexus_V1_Prs3OriginalExecutionImportInput *input,
    Nexus_V1_Prs3OriginalExecutionEvidenceReceipt *out_receipt)
{
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace;
    const Nexus_V1_Prs3Vdp1CaptureBindingReceipt *binding;
    const Nexus_V1_Prs3OriginalExecutionAuthentication *auth;
    Nexus_V1_Prs3OriginalExecutionEvidenceReceipt receipt;
    uint64_t export_fnv;
    char export_sha256[65];

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt)); receipt.evidence_only = 1;
    if (!input || !(trace = input->trace) || !(binding = input->binding) ||
        !(auth = input->authentication) || !input->trace_export_bytes ||
        !input->trace_export_size || !auth->valid || !auth->mednafen_saturn_debugger_export ||
        !auth->independently_authenticated_original_saturn_execution ||
        !sha256_shape(auth->trace_export_sha256)) { *out_receipt = receipt; return 0; }
    export_fnv = fnv1a64(input->trace_export_bytes, input->trace_export_size);
    if (firestaff_x68k_media_receipt_sha256_hex(input->trace_export_bytes,
                                                 input->trace_export_size,
                                                 export_sha256,
                                                 sizeof(export_sha256)) != 0 ||
        strcmp(export_sha256, auth->trace_export_sha256) != 0 ||
        export_fnv != auth->trace_export_fnv1a64 || trace->schema_version != 10U ||
        !trace->valid || !trace->complete_capture || !trace->menu_bpk_fnv1a64 ||
        !trace->dm_bin_fnv1a64 || !trace->stream_size || !trace->expected_output_bytes ||
        trace->input_read_bytes != trace->stream_size ||
        trace->first_input_read_address != trace->payload_ram_address ||
        trace->last_input_read_address - trace->first_input_read_address + 1U != trace->input_read_bytes ||
        trace->output_write_bytes != trace->expected_output_bytes || !trace->output_fnv1a64 ||
        !trace->complete_output_store_range_observed ||
        trace->output_store_bytes != trace->expected_output_bytes ||
        trace->first_output_write_address != trace->output_ram_address ||
        trace->last_output_write_address - trace->first_output_write_address + 1U != trace->output_write_bytes ||
        trace->output_store_fnv1a64 != trace->output_fnv1a64 ||
        !trace->output_store_predecessor_observed ||
        trace->vdp1_command_sequence <= trace->last_output_write_sequence ||
        trace->vdp1_texture_source_address != trace->output_ram_address ||
        trace->vdp1_texture_source_bytes != trace->expected_output_bytes ||
        !trace->vdp1_texture_consumption_observed || !trace->vdp1_command_consumption_observed ||
        !binding->valid || !binding->trace_valid || !binding->menu_bpk_matches ||
        !binding->dm_bin_matches || !binding->entry_plan_matches || !binding->payload_span_matches ||
        !binding->exact_vdp1_handoff_observed || !binding->vdp1_texture_consumption_observed ||
        !binding->vdp1_command_consumption_observed || binding->decoder_promoted ||
        binding->fallback_visuals_permitted || trace->opcode_grammar_proven ||
        trace->decoder_promoted || trace->fallback_visuals_permitted) { *out_receipt = receipt; return 0; }
    receipt.valid = 1; receipt.entry_index = trace->entry_index;
    receipt.menu_bpk_fnv1a64 = trace->menu_bpk_fnv1a64; receipt.dm_bin_fnv1a64 = trace->dm_bin_fnv1a64;
    receipt.trace_export_fnv1a64 = export_fnv;
    receipt.stream_offset = trace->stream_offset;
    receipt.stream_size = trace->stream_size;
    receipt.payload_fnv1a64 = trace->payload_fnv1a64;
    receipt.input_read_bytes = trace->input_read_bytes;
    receipt.output_write_bytes = trace->output_write_bytes; receipt.output_fnv1a64 = trace->output_fnv1a64;
    receipt.vdp1_texture_source_address = trace->vdp1_texture_source_address;
    receipt.vdp1_texture_source_bytes = trace->vdp1_texture_source_bytes;
    receipt.last_output_write_sequence = trace->last_output_write_sequence;
    receipt.vdp1_command_sequence = trace->vdp1_command_sequence;
    receipt.complete_sh2_input_reads_bound = receipt.complete_output_range_bound =
        receipt.subsequent_vdp1_source_command_bound = receipt.authenticated_original_execution_bound = 1;
    *out_receipt = receipt; return 1;
}
