#ifndef NEXUS_V1_PRS3_DECODER_ADMISSION_H
#define NEXUS_V1_PRS3_DECODER_ADMISSION_H

#include <stddef.h>
#include <stdint.h>

#include "nexus_v1_bpk_archive.h"
#include "nexus_v1_prs3_capture_trace_schema.h"

typedef enum {
    NEXUS_V1_PRS3_DECODER_ADMISSION_READY_BLOCKED = 0,
    NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_INPUT = 1,
    NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_SOURCE = 2,
    NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_EXECUTABLE = 3,
    NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_MENU_STREAM = 4,
    NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_DIFFERENTIAL = 5,
    NEXUS_V1_PRS3_DECODER_ADMISSION_BLOCKED_OUTPUT_PROOF = 6,
    NEXUS_V1_PRS3_DECODER_ADMISSION_READY_DECODER = 7
} Nexus_V1_Prs3DecoderAdmissionStatus;

typedef struct {
    const uint8_t *menu_bpk;
    size_t menu_bpk_size;
    int menu_bpk_source_verified;
    const uint8_t *dm_bin;
    size_t dm_bin_size;
    int dm_bin_source_verified;
    const uint8_t *decoded_output;
    size_t decoded_output_size;
    uint64_t decoded_output_fnv1a64;
    int decoded_output_source_bound;
    int original_saturn_provenance_verified;
    int opcode_grammar_proven;
} Nexus_V1_Prs3DecoderAdmissionInput;

typedef struct {
    Nexus_V1_Prs3DecoderAdmissionStatus status;
    int menu_bpk_source_verified;
    int dm_bin_source_verified;
    int dm_bin_v1_loader_bound;
    uint32_t dm_bin_v1_callee_offset;
    uint32_t dm_bin_control_test_offset;
    uint32_t dm_bin_nonzero_read_offset;
    uint32_t dm_bin_output_store_offset;
    uint32_t dm_bin_zero_first_read_offset;
    uint32_t dm_bin_zero_second_read_offset;
    int nonzero_direct_byte_path_proven;
    int zero_side_two_byte_merge_proven;
    int zero_side_has_no_direct_output_store;
    int menu_bpk_v1_streams_bound;
    uint32_t menu_bpk_entry_count;
    uint32_t menu_bpk_prs3_stream_count;
    uint32_t first_stream_entry_index;
    uint32_t first_stream_offset;
    uint32_t first_stream_size;
    uint32_t first_expected_output_bytes;
    uint32_t lsb_trial_evaluated;
    uint32_t lsb_trial_complete_exact;
    uint32_t lsb_trial_complete_trailing;
    uint32_t lsb_trial_failures;
    uint32_t msb_trial_evaluated;
    uint32_t msb_trial_complete_exact;
    uint32_t msb_trial_complete_trailing;
    uint32_t msb_trial_failures;
    int simple_lsb_msb_decoder_disproven;
    Nexus_V1_BpkPrs3DecodedOutputProofReceipt output_proof;
    int expected_output_bound;
    int original_saturn_provenance_verified;
    int opcode_grammar_proven;
    int decoder_ready;
    int decoder_promoted;
    uint32_t decoded_pixels_emitted;
    int runtime_upload_permitted;
    int structure2_pixel_intake_permitted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3DecoderAdmissionReceipt;

int nexus_v1_prs3_decoder_admission_evaluate(
    const Nexus_V1_Prs3DecoderAdmissionInput *input,
    Nexus_V1_Prs3DecoderAdmissionReceipt *out_receipt);

const char *nexus_v1_prs3_decoder_admission_status_name(
    Nexus_V1_Prs3DecoderAdmissionStatus status);

#endif
