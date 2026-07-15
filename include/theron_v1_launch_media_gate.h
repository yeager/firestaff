#ifndef THERON_V1_LAUNCH_MEDIA_GATE_H
#define THERON_V1_LAUNCH_MEDIA_GATE_H

#include "theron_v1_profile_media_audio_status.h"

typedef struct {
    const char *status;
    int launch_blocked;
    int visual_fallback_allowed;
    int audio_fallback_allowed;
    int raw_capture_required;
    int iso_end_blocked;
} Theron_V1LaunchMediaGateReceipt;

Theron_V1LaunchMediaGateReceipt theron_v1_launch_media_gate(
    Theron_V1ProfileMediaAudioStatus status);

#endif
