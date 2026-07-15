#include "theron_v1_track_media_availability.h"

int main(void) {
    Theron_V1IsoEndReceipt end = {
        .valid = 1,
        .opaque_only = 1
    };
    Theron_V1_TrackMediaAvailabilityReceipt a =
        theron_v1_track_media_availability("raw_track02_missing", &end);
    Theron_V1_TrackMediaAvailabilityReceipt b =
        theron_v1_track_media_availability("raw_track02_ready", &end);

    return a.availability == THERON_V1_TRACK_MEDIA_END_VARIANT &&
        !a.loader_usable &&
        b.availability == THERON_V1_TRACK_MEDIA_RAW_READY &&
        b.loader_usable ? 0 : 1;
}
