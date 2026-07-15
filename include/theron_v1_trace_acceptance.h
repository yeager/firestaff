#ifndef THERON_V1_TRACE_ACCEPTANCE_H
#define THERON_V1_TRACE_ACCEPTANCE_H

#include "theron_v1_capture_config.h"
#include "theron_v1_runtime_admission.h"

typedef struct {
    int valid;
    int raw_ready;
    int media_ready;
    int stage3_ready;
    int v3_trace_validated;
} Theron_V1TraceAcceptanceFacts;

typedef struct {
    int accepted;
    int session_allocated;
} Theron_V1TraceAcceptanceReceipt;

Theron_V1TraceAcceptanceReceipt theron_v1_trace_acceptance(
    const Theron_V1TraceAcceptanceFacts *facts,
    const Theron_V1CaptureConfig *config,
    const char *trace_identity);

const char *theron_v1_trace_acceptance_status(
    const Theron_V1TraceAcceptanceFacts *facts,
    const Theron_V1TraceAcceptanceReceipt *receipt);

#endif
