#include "theron_v1_profile_media_availability.h"

#include <string.h>

Theron_V1ProfileMediaAvailability theron_v1_profile_media_availability(
    const Theron_V1MediaInventoryReceipt *receipt) {
    if (receipt && receipt->startup_eligible &&
        receipt->raw_track02_usable && receipt->bitmap_route_usable &&
        receipt->level_route_usable && receipt->object_route_usable) {
        return THERON_V1_PROFILE_MEDIA_RAW_READY;
    }
    if (receipt && receipt->diagnostic &&
        strcmp(receipt->diagnostic, "raw_track02_iso_end_variant") == 0) {
        return THERON_V1_PROFILE_MEDIA_END_VARIANT;
    }
    return THERON_V1_PROFILE_MEDIA_MISSING;
}

const char *theron_v1_profile_media_availability_name(
    Theron_V1ProfileMediaAvailability availability) {
    switch (availability) {
    case THERON_V1_PROFILE_MEDIA_RAW_READY:
        return "raw_track_required_ready";
    case THERON_V1_PROFILE_MEDIA_END_VARIANT:
        return "raw_track_required_end_variant";
    case THERON_V1_PROFILE_MEDIA_MISSING:
    default:
        return "raw_track_required_missing";
    }
}
