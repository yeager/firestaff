#include "theron_v1_launch_decision.h"

Theron_V1LaunchDecisionReceipt theron_v1_launch_decision(
    Theron_V1LaunchMediaGateReceipt gate,
    Theron_V1ProfileMediaAudioStatus status) {
    Theron_V1LaunchDecisionReceipt receipt = {
        0, 0, 1, 0, 0,
        0, 0, 0, 0,
        status.raw_track_status
    };

    if (!gate.launch_blocked && !gate.raw_capture_required &&
        !gate.iso_end_blocked) {
        receipt.allowed = 1;
        receipt.capture_plan_prepared = 1;
        receipt.bitmap_route_ready = 1;
        receipt.level_route_ready = 1;
        receipt.object_route_ready = 1;
    }
    return receipt;
}
