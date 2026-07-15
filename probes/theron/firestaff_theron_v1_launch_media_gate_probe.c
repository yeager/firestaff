#include "theron_v1_launch_media_gate.h"

#include <string.h>

int main(void) {
    Theron_V1ProfileMediaAudioStatus status = {
        "raw_track_required_end_variant",
        "format_mismatch",
        0,
        0
    };
    Theron_V1LaunchMediaGateReceipt receipt =
        theron_v1_launch_media_gate(status);

    return receipt.launch_blocked &&
        receipt.iso_end_blocked &&
        !receipt.visual_fallback_allowed &&
        !receipt.audio_fallback_allowed &&
        strcmp(receipt.status, "raw_track_required_end_variant") == 0 ? 0 : 1;
}
