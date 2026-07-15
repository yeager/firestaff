#include "theron_v1_trace_acceptance.h"

Theron_V1TraceAcceptanceReceipt theron_v1_trace_acceptance(
    const Theron_V1TraceAcceptanceFacts *facts,
    const Theron_V1CaptureConfig *config,
    const char *trace_identity) {
    Theron_V1TraceAcceptanceReceipt receipt = {0, 0};

    if (facts && facts->valid && facts->raw_ready && facts->media_ready &&
        facts->stage3_ready && facts->v3_trace_validated &&
        theron_v1_capture_config_validate(config, config) &&
        theron_v1_runtime_trace_identity_valid(trace_identity, config)) {
        receipt.accepted = 1;
    }
    return receipt;
}

const char *theron_v1_trace_acceptance_status(
    const Theron_V1TraceAcceptanceFacts *facts,
    const Theron_V1TraceAcceptanceReceipt *receipt) {
    if (!facts || !facts->raw_ready || !facts->media_ready ||
        !facts->stage3_ready) {
        return "media_blocked";
    }
    if (!facts->v3_trace_validated) {
        return "trace_required";
    }
    return receipt && receipt->accepted && !receipt->session_allocated ?
        "trace_accepted_runtime_unavailable" : "trace_required";
}
