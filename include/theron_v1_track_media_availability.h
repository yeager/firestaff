#ifndef THERON_V1_TRACK_MEDIA_AVAILABILITY_H
#define THERON_V1_TRACK_MEDIA_AVAILABILITY_H

#include "theron_v1_iso_end_receipt.h"

typedef enum {
    THERON_V1_TRACK_MEDIA_MISSING = 0,
    THERON_V1_TRACK_MEDIA_END_VARIANT = 1,
    THERON_V1_TRACK_MEDIA_RAW_READY = 2
} Theron_V1TrackMediaAvailability;

typedef struct {
    Theron_V1TrackMediaAvailability availability;
    int loader_usable;
} Theron_V1_TrackMediaAvailabilityReceipt;

Theron_V1_TrackMediaAvailabilityReceipt theron_v1_track_media_availability(
    const char *raw_track_status,
    const Theron_V1IsoEndReceipt *iso_end);

const char *theron_v1_track_media_availability_name(
    Theron_V1TrackMediaAvailability availability);

#endif
