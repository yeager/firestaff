#include "theron_v1_trace_provenance.h"

Theron_V1TraceProvenanceReceipt theron_v1_trace_provenance(
    const Theron_V1TraceAcceptanceReceipt *accept,
    const Theron_V1TraceSourceProvenanceReceipt *source) {
    Theron_V1TraceProvenanceReceipt receipt = {
        0,
        0,
        "trace_required"
    };

    if (accept && accept->accepted && source && source->valid) {
        receipt.valid = 1;
        receipt.runtime_admitted = 0;
        receipt.status = "trace_accepted_runtime_unavailable";
    }
    return receipt;
}

const char *theron_v1_trace_launch_status(
    const Theron_V1TraceProvenanceReceipt *receipt) {
    return receipt && receipt->valid && !receipt->runtime_admitted ?
        "runtime_unavailable" : "trace_required";
}
