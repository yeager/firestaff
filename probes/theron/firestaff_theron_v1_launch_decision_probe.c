#include "theron_v1_launch_decision.h"

int main(void) {
    Theron_V1ProfileMediaAudioStatus missing = {
        "raw_track_required_missing",
        "missing",
        0,
        0
    };
    Theron_V1ProfileMediaAudioStatus ready = {
        "raw_track_required_ready",
        "missing",
        0,
        0
    };
    Theron_V1LaunchMediaGateReceipt blocked = {
        .status = missing.raw_track_status,
        .launch_blocked = 1,
        .raw_capture_required = 1
    };
    Theron_V1LaunchMediaGateReceipt ok = {
        .status = ready.raw_track_status,
        .launch_blocked = 0,
        .raw_capture_required = 0,
        .iso_end_blocked = 0
    };
    Theron_V1LaunchDecisionReceipt r =
        theron_v1_launch_decision(blocked, missing);
    Theron_V1LaunchDecisionReceipt q =
        theron_v1_launch_decision(ok, ready);

    return !r.allowed &&
        !r.session_allocated &&
        !r.callback_invoked &&
        !r.visual_fallback_allowed &&
        q.capture_plan_prepared &&
        q.runtime_blocked &&
        q.bitmap_route_ready &&
        !q.level_route_ready &&
        !q.object_route_ready &&
        !q.session_allocated &&
        !q.callback_invoked ? 0 : 1;
}
