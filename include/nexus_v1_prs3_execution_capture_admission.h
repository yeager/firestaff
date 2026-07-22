#ifndef NEXUS_V1_PRS3_EXECUTION_CAPTURE_ADMISSION_H
#define NEXUS_V1_PRS3_EXECUTION_CAPTURE_ADMISSION_H

#include "nexus_v1_prs3_original_execution_import.h"

typedef struct {
    int valid;
    uint64_t output_bytes_fnv1a64;
    uint64_t menu_bpk_fnv1a64;
    uint64_t dm_bin_fnv1a64;
    uint64_t stream_fnv1a64;
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    char output_bytes_sha256[65];
    char menu_bpk_sha256[65];
    char dm_bin_sha256[65];
    char stream_sha256[65];
    uint64_t vdp1_capture_fnv1a64;
    char vdp1_capture_sha256[65];
    uint64_t last_output_write_sequence;
    uint64_t vdp1_command_sequence;
} Nexus_V1_Prs3ExecutionCaptureAuthentication;

typedef struct {
    const Nexus_V1_Prs3OriginalExecutionEvidenceReceipt *execution;
    const uint8_t *menu_bpk_bytes;
    size_t menu_bpk_byte_count;
    const uint8_t *dm_bin_bytes;
    size_t dm_bin_byte_count;
    const uint8_t *output_bytes;
    size_t output_byte_count;
    const uint8_t *vdp1_capture_bytes;
    size_t vdp1_capture_byte_count;
    const Nexus_V1_Prs3ExecutionCaptureAuthentication *authentication;
} Nexus_V1_Prs3ExecutionCaptureAdmissionInput;

typedef struct {
    int valid;
    uint32_t entry_index;
    uint32_t stream_offset;
    uint32_t stream_size;
    uint64_t stream_fnv1a64;
    uint64_t output_fnv1a64;
    uint64_t vdp1_capture_fnv1a64;
    int full_output_range_bound;
    int vdp1_capture_bound;
    int command_order_bound;
    int stream_identity_bound;
    int stream_bytes_bound;
    int original_assets_bound;
    int evidence_only;
    int decoder_promoted;
    int graphics_permitted;
} Nexus_V1_Prs3ExecutionCaptureAdmissionReceipt;

int nexus_v1_prs3_execution_capture_admit(
    const Nexus_V1_Prs3ExecutionCaptureAdmissionInput *input,
    Nexus_V1_Prs3ExecutionCaptureAdmissionReceipt *out_receipt);

#endif
