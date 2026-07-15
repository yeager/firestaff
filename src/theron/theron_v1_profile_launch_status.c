#include "theron_v1_profile_launch_status.h"

Theron_V1ProfileLaunchStatusSnapshot theron_v1_profile_launch_status(
    const Theron_V1TraceProvenanceReceipt *trace) {
    Theron_V1ProfileLaunchStatusSnapshot snapshot = {
        theron_v1_trace_launch_status(trace),
        0
    };
    return snapshot;
}
