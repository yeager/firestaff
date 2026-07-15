#include "theron_v1_launch_media_gate.h"

#include <string.h>

Theron_V1LaunchMediaGateReceipt theron_v1_launch_media_gate(
    Theron_V1ProfileMediaAudioStatus status) {
    Theron_V1LaunchMediaGateReceipt receipt = {
        status.raw_track_status,
        1,
        0,
        0,
        1,
        0
    };

    if (status.raw_track_status &&
        strcmp(status.raw_track_status, "raw_track_required_ready") == 0) {
        receipt.launch_blocked = 0;
        receipt.raw_capture_required = 0;
    } else if (status.raw_track_status &&
               strcmp(status.raw_track_status,
                      "raw_track_required_end_variant") == 0) {
        receipt.iso_end_blocked = 1;
    }
    return receipt;
}
