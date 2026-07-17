#ifndef NEXUS_V1_PRS3_VDP1_CAPTURE_REPLAY_H
#define NEXUS_V1_PRS3_VDP1_CAPTURE_REPLAY_H

#include "nexus_v1_prs3_dgn_placement_adapter.h"

/* Monotonic receipt state for one external capture replay. It records only
 * ordering facts and is deliberately separate from any decoder state. */
typedef struct {
    int valid;
    uint64_t last_frame_sequence;
    uint64_t last_command_sequence;
    uint32_t last_descriptor_index;
} Nexus_V1_Prs3Vdp1CaptureReplayState;

typedef struct {
    Nexus_V1_Prs3DgnPlacementAdapterInput placement;
    const Nexus_V1_Prs3Vdp1CaptureBindingReceipt *capture_binding;
    const Nexus_V1_Prs3Vdp1ConsumerEvidenceReceipt *consumer_evidence;
} Nexus_V1_Prs3Vdp1CaptureReplayInput;

/* Admits one strictly later V10 capture observation and emits the existing
 * placement adapter receipt. Candidate spans and source identities remain
 * opaque; this function never interprets pixels, palette words, or geometry. */
int nexus_v1_prs3_vdp1_capture_replay_admit(
    Nexus_V1_Prs3Vdp1CaptureReplayState *state,
    const Nexus_V1_Prs3Vdp1CaptureReplayInput *input,
    Nexus_V1_Prs3DgnPlacementAdapterReceipt *out_receipt);

#endif
