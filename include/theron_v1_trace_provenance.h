#ifndef THERON_V1_TRACE_PROVENANCE_H
#define THERON_V1_TRACE_PROVENANCE_H

#include "theron_v1_trace_acceptance.h"

typedef struct {
    int valid;
    int runtime_admitted;
    const char *status;
} Theron_V1TraceProvenanceReceipt;

Theron_V1TraceProvenanceReceipt theron_v1_trace_provenance(
    const Theron_V1TraceAcceptanceReceipt *accept,
    const Theron_V1TraceSourceProvenanceReceipt *source);

const char *theron_v1_trace_launch_status(
    const Theron_V1TraceProvenanceReceipt *receipt);

#endif
