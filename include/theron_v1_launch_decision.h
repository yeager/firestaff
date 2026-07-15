#ifndef THERON_V1_LAUNCH_DECISION_H
#define THERON_V1_LAUNCH_DECISION_H

#include "theron_v1_launch_media_gate.h"

typedef struct {
    int allowed;
    int capture_plan_prepared;
    int runtime_blocked;
    int session_allocated;
    int callback_invoked;
    int visual_fallback_allowed;
    int bitmap_route_ready;
    int level_route_ready;
    int object_route_ready;
    const char *status;
} Theron_V1LaunchDecisionReceipt;

Theron_V1LaunchDecisionReceipt theron_v1_launch_decision(
    Theron_V1LaunchMediaGateReceipt gate,
    Theron_V1ProfileMediaAudioStatus status);

#endif
