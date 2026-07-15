#include "theron_v1_profile_media_availability.h"

#include <string.h>

int main(void) {
    Theron_V1MediaInventoryReceipt end_variant = {
        .diagnostic = "raw_track02_iso_end_variant"
    };
    Theron_V1MediaInventoryReceipt raw_ready = {
        .raw_track02_usable = 1,
        .startup_eligible = 1,
        .bitmap_route_usable = 1,
        .level_route_usable = 1,
        .object_route_usable = 1,
        .diagnostic = "raw_track02_ready"
    };

    return theron_v1_profile_media_availability(&end_variant) ==
            THERON_V1_PROFILE_MEDIA_END_VARIANT &&
        theron_v1_profile_media_availability(&raw_ready) ==
            THERON_V1_PROFILE_MEDIA_RAW_READY &&
        strcmp(theron_v1_profile_media_availability_name(
                   THERON_V1_PROFILE_MEDIA_MISSING),
               "raw_track_required_missing") == 0 ? 0 : 1;
}
