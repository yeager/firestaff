#include "nexus_v1_prs3_multi_capture_adjudicator.h"

#include <string.h>

int nexus_v1_prs3_multi_capture_adjudicate(
    const Nexus_V1_Prs3MultiCaptureAdjudicationInput *input,
    Nexus_V1_Prs3MultiCaptureAdjudicationReceipt *out_receipt)
{
    Nexus_V1_Prs3MultiCaptureAdjudicationReceipt receipt;
    Nexus_V1_Prs3ObservedBitOrder bit_order = NEXUS_V1_PRS3_BIT_ORDER_UNSPECIFIED;
    Nexus_V1_Prs3ObservedTermination termination = NEXUS_V1_PRS3_TERMINATION_UNSPECIFIED;
    size_t i, j, modes = 0;

    if (!out_receipt) return 0;
    memset(&receipt, 0, sizeof(receipt));
    if (!input || !input->evidence || input->evidence_count < 2U ||
        input->evidence_count > NEXUS_V1_PRS3_MULTI_CAPTURE_MAX_EVIDENCE) {
        *out_receipt = receipt; return 0;
    }
    for (i = 0; i < input->evidence_count; ++i) {
        const Nexus_V1_Prs3MultiCaptureEvidence *row = &input->evidence[i];
        const Nexus_V1_Prs3OriginalExecutionEvidenceReceipt *execution = row->execution;
        int new_mode = 1;
        if (!execution || !execution->valid || !execution->authenticated_original_execution_bound ||
            !execution->complete_sh2_input_reads_bound || !execution->complete_output_range_bound ||
            !execution->subsequent_vdp1_source_command_bound || !execution->evidence_only ||
            execution->decoder_promoted || execution->graphics_permitted ||
            !execution->input_read_bytes || !execution->output_write_bytes ||
            !execution->output_fnv1a64 || !execution->vdp1_texture_source_bytes ||
            row->observed_bit_order == NEXUS_V1_PRS3_BIT_ORDER_UNSPECIFIED ||
            row->observed_termination == NEXUS_V1_PRS3_TERMINATION_UNSPECIFIED) {
            *out_receipt = receipt; return 0;
        }
        if (bit_order && bit_order != row->observed_bit_order) {
            receipt.contradictions_detected = 1; *out_receipt = receipt; return 0;
        }
        if (termination && termination != row->observed_termination) {
            receipt.contradictions_detected = 1; *out_receipt = receipt; return 0;
        }
        bit_order = row->observed_bit_order; termination = row->observed_termination;
        for (j = 0; j < i; ++j) {
            if (input->evidence[j].execution->entry_index == execution->entry_index ||
                input->evidence[j].execution->trace_export_fnv1a64 == execution->trace_export_fnv1a64) {
                *out_receipt = receipt; return 0;
            }
            if (input->evidence[j].menu_bpk_mode == row->menu_bpk_mode) {
                *out_receipt = receipt; return 0;
            }
        }
        if (new_mode) ++modes;
    }
    if (modes < 2U) { *out_receipt = receipt; return 0; }
    receipt.valid = receipt.contracts_consistent = receipt.decoder_candidate_review_ready = 1;
    receipt.evidence_count = input->evidence_count; receipt.distinct_mode_count = modes;
    receipt.observed_bit_order = bit_order; receipt.observed_termination = termination;
    *out_receipt = receipt; return 1;
}
