#ifndef THERON_V1_TRACK_MEDIA_AVAILABILITY_H
#define THERON_V1_TRACK_MEDIA_AVAILABILITY_H

#include "theron_v1_iso_end_receipt.h"

typedef enum {
    THERON_V1_TRACK_MEDIA_MISSING,
    THERON_V1_TRACK_MEDIA_END_VARIANT,
    THERON_V1_TRACK_MEDIA_RAW_READY
} Theron_V1TrackMediaAvailability;

typedef struct {
    Theron_V1TrackMediaAvailability availability;
    int loader_usable;
} Theron_V1_TrackMediaAvailabilityReceipt;

Theron_V1_TrackMediaAvailabilityReceipt theron_v1_track_media_availability(
    const char *raw_status, const Theron_V1IsoEndReceipt *end);
#endif
