#ifndef NEXUS_V1_PRS3_ORIGINAL_EXECUTION_IMPORT_H
#define NEXUS_V1_PRS3_ORIGINAL_EXECUTION_IMPORT_H

#include "nexus_v1_prs3_capture_trace_schema.h"

/* External review is explicit input, never inferred from the schema text. */
typedef struct {
    int valid;
    int mednafen_saturn_debugger_export;
    int independently_authenticated_original_saturn_execution;
    uint64_t trace_export_fnv1a64;
    char trace_export_sha256[65];
} Nexus_V1_Prs3OriginalExecutionAuthentication;

typedef struct {
    const Nexus_V1_Prs3Vdp1CaptureReceipt *trace;
    const Nexus_V1_Prs3Vdp1CaptureBindingReceipt *binding;
    const uint8_t *trace_export_bytes;
    size_t trace_export_size;
    const Nexus_V1_Prs3OriginalExecutionAuthentication *authentication;
} Nexus_V1_Prs3OriginalExecutionImportInput;

typedef struct {
    int valid;
    uint32_t entry_index;
    uint64_t menu_bpk_fnv1a64;
    uint64_t dm_bin_fnv1a64;
    uint64_t trace_export_fnv1a64;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint64_t payload_fnv1a64;
    uint32_t input_read_bytes;
    uint32_t output_write_bytes;
    uint64_t output_fnv1a64;
    uint32_t vdp1_texture_source_address;
    uint32_t vdp1_texture_source_bytes;
    uint64_t last_output_write_sequence;
    uint64_t vdp1_command_sequence;
    int complete_sh2_input_reads_bound;
    int complete_output_range_bound;
    int subsequent_vdp1_source_command_bound;
    int authenticated_original_execution_bound;
    int evidence_only;
    int decoder_promoted;
    int graphics_permitted;
} Nexus_V1_Prs3OriginalExecutionEvidenceReceipt;

/* Imports one complete authenticated V10 trace receipt. It does not derive a
 * PRS3 grammar, decoder state, pixel format, palette meaning, or draw route. */
int nexus_v1_prs3_original_execution_evidence_import(
    const Nexus_V1_Prs3OriginalExecutionImportInput *input,
    Nexus_V1_Prs3OriginalExecutionEvidenceReceipt *out_receipt);

#endif
