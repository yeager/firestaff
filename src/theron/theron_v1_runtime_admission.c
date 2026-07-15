#include "theron_v1_runtime_admission.h"

#include <stdio.h>
#include <string.h>

void theron_v1_runtime_admission_init(
    Theron_V1RuntimeAdmissionReceipt *out) {
    if (out) {
        out->attached = 0;
        out->admitted = 0;
    }
}

int theron_v1_runtime_admission_attach(
    Theron_V1RuntimeAdmissionReceipt *out,
    const char *trace_identity,
    int placeholder_or_synthetic) {
    if (!out) {
        return 0;
    }
    theron_v1_runtime_admission_init(out);
    if (!trace_identity || trace_identity[0] == '\0' ||
        placeholder_or_synthetic ||
        strstr(trace_identity, "placeholder") ||
        strstr(trace_identity, "synthetic")) {
        return 0;
    }
    out->attached = 1;
    out->admitted = 0;
    return 1;
}

int theron_v1_runtime_trace_identity_valid(
    const char *identity,
    const Theron_V1CaptureConfig *config) {
    char expected[96];
    int written;

    if (!identity || !config || !config->valid || !config->track02_hash ||
        !config->system_card_hash) {
        return 0;
    }
    written = snprintf(expected, sizeof(expected), "v3:%s:%s",
                       config->track02_hash, config->system_card_hash);
    if (written <= 0 || (size_t)written >= sizeof(expected)) {
        return 0;
    }
    return strcmp(identity, expected) == 0;
}

int theron_v1_trace_source_provenance(
    const char *source_id,
    const char *config_identity,
    Theron_V1TraceSourceProvenanceReceipt *out) {
    if (out) {
        out->valid = 0;
        out->runtime_admitted = 0;
    }
    if (!out || !source_id || source_id[0] == '\0' ||
        !config_identity || config_identity[0] == '\0' ||
        strcmp(source_id, config_identity) == 0 ||
        strstr(source_id, "placeholder") ||
        strstr(source_id, "synthetic")) {
        return 0;
    }
    out->valid = 1;
    out->runtime_admitted = 0;
    return 1;
}
