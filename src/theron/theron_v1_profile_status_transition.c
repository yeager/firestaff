#include "theron_v1_profile_status_transition.h"

#include <string.h>

Theron_V1ProfileStatusTransition theron_v1_profile_status_transition(
    const char *raw_status,
    const char *media_status,
    const Theron_V1ProfileLaunchStatusSnapshot *trace) {
    Theron_V1ProfileStatusTransition transition = {
        "trace_required",
        0,
        0
    };

    if (raw_status && strcmp(raw_status, "raw_track_required_missing") == 0) {
        transition.status = "raw_track_required_missing";
    } else if (media_status && strcmp(media_status, "format_mismatch") == 0) {
        transition.status = "format_mismatch";
    } else if (trace && trace->status) {
        transition.status = trace->status;
        transition.startup_invoked = trace->startup_invoked;
    }
    transition.session_allocated = 0;
    return transition;
}
