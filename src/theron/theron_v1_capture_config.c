#include "theron_v1_capture_config.h"

#include <string.h>

int theron_v1_capture_config_validate(
    const Theron_V1CaptureConfig *stored,
    const Theron_V1CaptureConfig *current) {
    return stored && current &&
        stored->valid && current->valid &&
        stored->runtime_blocked && current->runtime_blocked &&
        stored->version == 1u && current->version == 1u &&
        stored->track02_hash && current->track02_hash &&
        stored->system_card_hash && current->system_card_hash &&
        stored->status && current->status &&
        strcmp(stored->track02_hash, current->track02_hash) == 0 &&
        strcmp(stored->system_card_hash, current->system_card_hash) == 0 &&
        strcmp(stored->status, "raw_track_required_ready") == 0 &&
        strcmp(stored->status, current->status) == 0;
}
