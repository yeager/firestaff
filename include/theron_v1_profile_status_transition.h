#ifndef THERON_V1_PROFILE_STATUS_TRANSITION_H
#define THERON_V1_PROFILE_STATUS_TRANSITION_H

#include "theron_v1_profile_launch_status.h"

typedef struct {
    const char *status;
    int startup_invoked;
    int session_allocated;
} Theron_V1ProfileStatusTransition;

Theron_V1ProfileStatusTransition theron_v1_profile_status_transition(
    const char *raw_status,
    const char *media_status,
    const Theron_V1ProfileLaunchStatusSnapshot *trace);

#endif
