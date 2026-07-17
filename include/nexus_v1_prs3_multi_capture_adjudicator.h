#ifndef NEXUS_V1_PRS3_MULTI_CAPTURE_ADJUDICATOR_H
#define NEXUS_V1_PRS3_MULTI_CAPTURE_ADJUDICATOR_H

#include "nexus_v1_prs3_original_execution_import.h"

#define NEXUS_V1_PRS3_MULTI_CAPTURE_MAX_EVIDENCE 16U

typedef enum {
    NEXUS_V1_PRS3_BIT_ORDER_UNSPECIFIED = 0,
    NEXUS_V1_PRS3_BIT_ORDER_LSB_FIRST_OBSERVED = 1,
    NEXUS_V1_PRS3_BIT_ORDER_MSB_FIRST_OBSERVED = 2
} Nexus_V1_Prs3ObservedBitOrder;

typedef enum {
    NEXUS_V1_PRS3_TERMINATION_UNSPECIFIED = 0,
    NEXUS_V1_PRS3_TERMINATION_OUTPUT_RANGE_OBSERVED = 1,
    NEXUS_V1_PRS3_TERMINATION_RETURN_OBSERVED = 2
} Nexus_V1_Prs3ObservedTermination;

/* Labels are externally reviewed observations only. They never change the
 * meaning of a PRS3 control bit or authorize decoder execution. */
typedef struct {
    const Nexus_V1_Prs3OriginalExecutionEvidenceReceipt *execution;
    uint32_t menu_bpk_mode;
    Nexus_V1_Prs3ObservedBitOrder observed_bit_order;
    Nexus_V1_Prs3ObservedTermination observed_termination;
} Nexus_V1_Prs3MultiCaptureEvidence;

typedef struct {
    const Nexus_V1_Prs3MultiCaptureEvidence *evidence;
    size_t evidence_count;
} Nexus_V1_Prs3MultiCaptureAdjudicationInput;

typedef struct {
    int valid;
    size_t evidence_count;
    size_t distinct_mode_count;
    Nexus_V1_Prs3ObservedBitOrder observed_bit_order;
    Nexus_V1_Prs3ObservedTermination observed_termination;
    int contracts_consistent;
    int contradictions_detected;
    int decoder_candidate_review_ready;
    int decoder_promoted;
    int render_promoted;
    int fallback_visuals_permitted;
} Nexus_V1_Prs3MultiCaptureAdjudicationReceipt;

/* Requires independently authenticated evidence from at least two MENU.BPK
 * modes. Contradictory bit-order/termination labels reject atomically. */
int nexus_v1_prs3_multi_capture_adjudicate(
    const Nexus_V1_Prs3MultiCaptureAdjudicationInput *input,
    Nexus_V1_Prs3MultiCaptureAdjudicationReceipt *out_receipt);

#endif
