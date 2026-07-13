#include "theron_v1_track_media_availability.h"

#include <string.h>

Theron_V1_TrackMediaAvailabilityReceipt theron_v1_track_media_availability(
    const char *raw_status, const Theron_V1IsoEndReceipt *end) {
    Theron_V1_TrackMediaAvailabilityReceipt receipt = {
        THERON_V1_TRACK_MEDIA_MISSING, 0
    };

    if (raw_status && strcmp(raw_status, "raw_track02_ready") == 0) {
        receipt.availability = THERON_V1_TRACK_MEDIA_RAW_READY;
        receipt.loader_usable = 1;
    } else if (end && end->valid && end->opaque_only) {
        receipt.availability = THERON_V1_TRACK_MEDIA_END_VARIANT;
    }
    return receipt;
}
