#ifndef THERON_V1_PROFILE_LAUNCH_STATUS_H
#define THERON_V1_PROFILE_LAUNCH_STATUS_H

#include "theron_v1_trace_provenance.h"

typedef struct {
    const char *status;
    int startup_invoked;
} Theron_V1ProfileLaunchStatusSnapshot;

Theron_V1ProfileLaunchStatusSnapshot theron_v1_profile_launch_status(
    const Theron_V1TraceProvenanceReceipt *trace);

#endif
