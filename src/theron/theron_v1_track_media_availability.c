#include "theron_v1_track_media_availability.h"

#include <string.h>

Theron_V1_TrackMediaAvailabilityReceipt theron_v1_track_media_availability(
    const char *raw_track_status,
    const Theron_V1IsoEndReceipt *iso_end) {
    Theron_V1_TrackMediaAvailabilityReceipt receipt = {
        THERON_V1_TRACK_MEDIA_MISSING,
        0
    };

    if (raw_track_status &&
        strcmp(raw_track_status, "raw_track02_ready") == 0) {
        receipt.availability = THERON_V1_TRACK_MEDIA_RAW_READY;
        receipt.loader_usable = 1;
        return receipt;
    }

    if (iso_end && iso_end->valid && iso_end->opaque_only &&
        !iso_end->loader_usable && !iso_end->bitmap_usable &&
        !iso_end->level_route_usable) {
        receipt.availability = THERON_V1_TRACK_MEDIA_END_VARIANT;
    }
    return receipt;
}

const char *theron_v1_track_media_availability_name(
    Theron_V1TrackMediaAvailability availability) {
    switch (availability) {
    case THERON_V1_TRACK_MEDIA_RAW_READY:
        return "raw_track02_ready";
    case THERON_V1_TRACK_MEDIA_END_VARIANT:
        return "raw_track02_iso_end_variant";
    case THERON_V1_TRACK_MEDIA_MISSING:
    default:
        return "raw_track02_missing";
    }
}
