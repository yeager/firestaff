#include "theron_v1_capture_config.h"

int main(void) {
    Theron_V1CaptureConfig a = {
        1u,
        "raw",
        "card",
        "raw_track_required_ready",
        1,
        1
    };
    Theron_V1CaptureConfig b = a;

    b.system_card_hash = "changed";
    return theron_v1_capture_config_validate(&a, &a) &&
        !theron_v1_capture_config_validate(&a, &b) ? 0 : 1;
}
