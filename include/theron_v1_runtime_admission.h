#ifndef THERON_V1_RUNTIME_ADMISSION_H
#define THERON_V1_RUNTIME_ADMISSION_H

#include "theron_v1_capture_config.h"

typedef struct {
    int attached;
    int admitted;
} Theron_V1RuntimeAdmissionReceipt;

typedef struct {
    int valid;
    int runtime_admitted;
} Theron_V1TraceSourceProvenanceReceipt;

void theron_v1_runtime_admission_init(
    Theron_V1RuntimeAdmissionReceipt *out);

int theron_v1_runtime_admission_attach(
    Theron_V1RuntimeAdmissionReceipt *out,
    const char *trace_identity,
    int placeholder_or_synthetic);

int theron_v1_runtime_trace_identity_valid(
    const char *identity,
    const Theron_V1CaptureConfig *config);

int theron_v1_trace_source_provenance(
    const char *source_id,
    const char *config_identity,
    Theron_V1TraceSourceProvenanceReceipt *out);

#endif
